#include "QEditor.h"

#include <fstream>
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <sys/ioctl.h>

#include "EditorCommands.h"

namespace {
    termios orig_termios;
}

Editor::Editor() {
    enableRawMode();
    updateWindowSize();

    // Load configuration
    config.parse();
    
    // Apply configuration values
    if (const auto tabWidth = config.getInt("tab_width")) {
        TAB_WIDTH = *tabWidth;
    }
    
    if (const auto lineNums = config.getBool("show_line_numbers")) {
        showLineNumbers = *lineNums;
    }

    filename = "";
    commandBuffer = "";

    // Save terminal state and switch to alternate screen
    std::cout << "\x1b[?1049h" << std::flush;
    // Clear the alternate screen completely
    std::cout << "\x1b[2J" << std::flush;
    // Move to home position
    std::cout << "\x1b[H" << std::flush;
    // Disable line wrapping
    std::cout << "\x1b[?7l" << std::flush;
}

Editor::~Editor() {
    // Re-enable line wrapping
    std::cout << "\x1b[?7h" << std::flush;
    // Show cursor
    std::cout << "\x1b[?25h" << std::flush;
    // Switch back to main screen
    std::cout << "\x1b[?1049l" << std::flush;

    disableRawMode();
}

Editor &Editor::getInstance() {
    static Editor instance;
    return instance;
}

void Editor::enableRawMode() {
    termios raw{};
    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;

    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void Editor::disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void Editor::run() {
    running = true;

    while (running) {
        // Wait up to 500ms for input
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);

        timeval timeout{};
        timeout.tv_sec = 1;

        const int ready = select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout);

        if (ready == -1 && errno != EINTR) {
            perror("select");
            break;
        }

        if (ready > 0) {
            processKeypress();
        }
    }
}

void Editor::stop() {
    running = false;
}

void Editor::processKeypress() {
    char c;
    ssize_t nread = read(STDIN_FILENO, &c, 1);

    if (nread == -1) {
        if (errno == EINTR) {
            return;
        }
    }

    if (nread == 1) {
        if (mode == VIEW) {
            if (c == 'i') {
                editMode();
            } else if (c == ':') {
                commandBuffer.clear();

                mode = COMMAND;
                if (cur_y >= buffer.size()) {
                    // Ensure at least one line exists
                    buffer.emplace_back("");
                }

                if (buffer[cur_y].size() < 2) {
                    // Pad with spaces to at least 2 chars for cur x positioning
                    buffer[cur_y].resize(2, ' ');
                }

                cur_x = 1;

                commandBuffer += c;
            } else if (c == 'x') {
                deleteChar();
            } else if (c == 'o') {
                cur_x = buffer[cur_y].length();

                insertNewline();

                editMode();
            } else if (c == 'a') {
                ++cur_x;
                editMode();
            } else if (c == 'd') {
                commandBuffer += c;

                if (commandBuffer == "dd") {
                    commandBuffer.clear();
                    deleteLine();
                }
            } else if (c == 'A') {
                editMode();

                jumpToEnd();
            } else if (c == 'D') {
                deleteToEol();
            } else if (c == 'w') {
                jumpWord();
            } else if (c == '0') {
                cur_x = 0;
            } else {
                // Handle multibyte input by ignoring them
                if (c == '\x1b') {
                    // Possible escape sequence (i.e., arrow key)
                    char seq[2];
                    if (read(STDIN_FILENO, &seq[0], 1) == 1 &&
                        read(STDIN_FILENO, &seq[1], 1) == 1) {
                        // Check for arrow key sequence: [A, B, C, D]
                        if (seq[0] == '[' && (seq[1] >= 'A' && seq[1] <= 'D')) {
                            return;
                        }
                    }
                }

                moveCursor(c);
            }
        } else if (mode == EDIT) {
            if (c == 27) { // esc
                mode = VIEW;

                // Move cursor back
                if (cur_x > 0) {
                    --cur_x;
                }

                setCursorShapeNormal();
            } else if (c == 127) { // backspace
                deleteChar();
            } else if (c == '\n') {
                insertNewline();
            } else if (c == '\t') {
                buffer[cur_y].insert(cur_x, 1, '\t');
                buffer[cur_y] = expandTabs(buffer[cur_y]);

                cur_x += TAB_WIDTH;
            } else { // insert
                insertText(c);
            }
        } else if (mode == COMMAND) {
            // Move cursor right one position, unless delete key
            if (c != 127) {
                ++cur_x;
            } else {
                if (cur_x > 0) --cur_x;
            }

            if (c == '\n') {
                if (!commandBuffer.empty())
                    processCommand();

                mode = VIEW;
            } else if (c == 127) { // backspace
                if (!commandBuffer.empty()) commandBuffer.pop_back();
                if (commandBuffer.empty()) mode = VIEW;
            } else if (c == 27) { // esc
                mode = VIEW;
                commandBuffer.clear();
            } else {
                commandBuffer.push_back(c);
            }
        }
    }

    drawScreen();
}

void Editor::moveCursor(const char direction) {
    if (direction == 'h' || direction == 127) { // Move left for "h" key or delete/backspace
        if (cur_x > 0) {
            --cur_x;
        }
    } else if (direction == 'l') { // Move right
        if (cur_y < buffer.size()) {
            if (const std::string& line = buffer[cur_y]; cur_x < line.size() - 1) {
                ++cur_x;
            }
        }
    } else if (direction == 'j') { // Move down
        if (cur_y + 1 < buffer.size()) {
            ++cur_y;

            if (const std::string& line = buffer[cur_y]; cur_x >= line.length()) {
                // clamp cur_x to end of line (or 0 if empty)
                cur_x = line.empty() ? 0 : line.size() - 1;
            }
        }
    } else if (direction == 'k') { // Move up
        if (cur_y > 0) {
            --cur_y;

            if (cur_x >= buffer[cur_y].size()) {
                cur_x = buffer[cur_y].size() - 1;
            }
        }
    }
}

void Editor::insertText(const char c) {
    if (cur_y >= buffer.size()) buffer.resize(cur_y + 1);

    std::string& line = buffer[cur_y];
    if (cur_x <= line.size()) {
        line.insert(cur_x, 1, c);
    } else {
        line += c;
    }

    ++cur_x;
}

void Editor::deleteChar() {
    if (cur_y >= buffer.size()) {
        // nothing to delete
        return;
    }

    // if empty line, delete it
    if (buffer[cur_y].empty()) {
        const auto it = buffer.begin() + static_cast<std::vector<std::string>::difference_type>(cur_y);
        buffer.erase(it);

        if (cur_y > 0) {
            cur_y = buffer.empty() ? 0 : cur_y - 1;
            cur_x = buffer[cur_y].length() - 1;
        }
    } else {
        const bool deleted = cur_x >= buffer[cur_y].size();

        buffer[cur_y].erase(cur_x, 1);

        if (deleted) --cur_x;
    }
}

void Editor::loadFile(const std::string& filename) {
    std::ifstream file;

    this->filename = filename;

    file.open(filename, std::ios::in | std::ios::out);

    // File does not exist, create it...
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        buffer.emplace_back("");

        return;
    }

    buffer.clear();
    std::string line;
    while (std::getline(file, line)) {
        buffer.push_back(line);
    }

    file.close();
}

void Editor::saveFile(const std::string& filename) const {
    // Trim whitespaces on the ends of the filename
    const std::string trimmedFilename = trimWhitespace(filename);

    if (trimmedFilename.empty()) {
        setStatusMessage(EditorCommands::INVALID_FILENAME_MSG);

        return;
    }

    std::ofstream file(trimmedFilename);

    if (!file) {
        std::ofstream newFile(filename);

        newFile.close();

        file.open(filename, std::ios::in | std::ios::out);

        if (!file) {
            std::cerr << "Failed to open" + filename + " after creation\n";
            return;
        }
    }

    for (const std::string& line : buffer) {
        file << line << std::endl;
    }

    file.close();
}

void Editor::drawScreen() const {
    // First, hide cursor while drawing
    std::cout << "\x1b[?25l";

    // Position at home
    std::cout << "\x1b[H";
    
    // Calculate line number width
    size_t lineNumWidth = 0;
    if (showLineNumbers) {
        lineNumWidth = std::to_string(buffer.size()).length() + 1; // +1 for the space after
    }

    // Draw content line by line with explicit positioning
    for (size_t i = 0; i < screenRows - 1; ++i) {
        // Position cursor at start of each line
        std::cout << "\x1b[" << (i + 1) << ";1H";

        // Clear the entire line first
        std::cout << "\x1b[2K";

        // Draw line numbers if config'd
        if (showLineNumbers) {
            if (i < buffer.size() || i == 0) {
                // Right-align the line number
                std::string lineNum = std::to_string(i + 1);
                std::string padding(lineNumWidth - lineNum.length() - 1, ' ');
                std::cout << padding << lineNum << " ";
            } else {
                // Print spaces for line number column on empty lines
                std::cout << std::string(lineNumWidth, ' ');
            }
        }

        // Draw content if available
        if (i < buffer.size()) {
            std::cout << expandTabs(buffer[i]);
        } else if (i != 0) {
            std::cout << "~";
        }
    }

    // Draw status/command line
    std::cout << "\x1b[" << screenRows << ";1H\x1b[2K";

    if (mode == COMMAND && !commandBuffer.empty()) {
        std::cout << ":" << commandBuffer.substr(1);
    }

    // Show status message if any
    if (!statusMessage.empty()) {
        size_t col = screenCols - statusMessage.length() + 1;

        if (col < 1) col = 1;

        std::cout << "\x1b[" << screenRows << ";" << col << "H" << statusMessage;
    }

    // Position cursor at edit location
    if (mode != COMMAND) {
        const int renderX = getRenderX(buffer[cur_y], cur_x);
        if (showLineNumbers) {
            lineNumWidth = std::to_string(buffer.size()).length() + 1; // +1 for the space after
        }
        std::cout << "\x1b[" << (cur_y + 1) << ";" << (renderX + lineNumWidth + 1) << "H";
    } else {
        // Move cursor to command bar
        std::cout << "\033[" << screenRows << ";" << cur_x + 1 << "H" << std::flush;
    }

    // Show cursor again
    std::cout << "\x1b[?25h";

    std::cout.flush();
}

void Editor::processCommand() {
    if (commandBuffer == EditorCommands::WRITE ||
        commandBuffer == EditorCommands::WRITE_QUIT) {
        std::string file = this->filename;

        if (file.empty()) {
            const std::optional<std::string> defaultFilename = config.getString("default_filename");

            if (defaultFilename && !defaultFilename->empty()) {
                file = *defaultFilename;
            } else {
                setStatusMessage(EditorCommands::INVALID_FILENAME_MSG);

                return;
            }
        }

        saveFile(file);
        setStatusMessage(EditorCommands::WROTE_TO + file);
    }

    if (commandBuffer == EditorCommands::QUIT ||
        commandBuffer == EditorCommands::WRITE_QUIT) {
        running = false;
    }

    if (commandBuffer.substr(0, 3) == EditorCommands::WRITE + " ") {
        // Save to specified file
        const std::string saveFilename = trimWhitespace(commandBuffer.substr(3));

        if (saveFilename.empty()) {
            setStatusMessage(EditorCommands::INVALID_FILENAME_MSG);

            return;
        }

        // Update current filename
        filename = saveFilename;

        saveFile(saveFilename);

        setStatusMessage(EditorCommands::WROTE_TO + saveFilename);
    }

    commandBuffer.clear();
}

void Editor::clearScreen() {
    // Save cursor position
    std::cout << "\x1b[s";

    // Clear screen
    std::cout << "\x1b[H\x1b[J";

    // Restore cursor position
    std::cout << "\x1b[u";

    std::cout.flush();
}

void Editor::insertNewline() {
    if (cur_y >= buffer.size()) {
        buffer.emplace_back("");
        cur_y = buffer.size() - 1;
    }

    std::string& current_line = buffer[cur_y];
    if (cur_x > current_line.length()) {
        cur_x = current_line.length();
    }

    const std::string new_line = current_line.substr(cur_x);
    current_line = current_line.substr(0, cur_x);

    const auto it = buffer.begin() + static_cast<std::vector<std::string>::difference_type>(cur_y);

    buffer.insert(it + 1, new_line);

    ++cur_y;
    cur_x = 0;
}

void Editor::deleteLine() {
    if (cur_y < 0 || cur_y >= buffer.size()) return;

    const auto it = buffer.begin() + static_cast<std::vector<std::string>::difference_type>(cur_y);

    buffer.erase(it);

    // Move cursor up to start of previous line
    cur_x = 0;
    if (cur_y > 0) {
        --cur_y;
    }
}

void Editor::updateWindowSize() {
    cur_x = 0;
    cur_y = 0;
    winsize ws{};

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        screenRows = 24;
        screenCols = 80;
    } else {
        screenRows = ws.ws_row;
        screenCols = ws.ws_col;
    }
}

std::string Editor::expandTabs(const std::string &line) const {
    std::string result;

    for (const char ch : line) {
        if (ch == '\t') {
            const size_t spaces = TAB_WIDTH;// - (result.length() % TAB_WIDTH);

            result += std::string(spaces, ' ');
        } else {
            result += ch;
        }
    }

    return result;
}

int Editor::getRenderX(const std::string &line, const size_t cur_x) const {
    int rx = 0;

    for (int i = 0; i < cur_x && i < line.size(); ++i) {
        if (line[i] == '\t') {
            rx += TAB_WIDTH - (rx % TAB_WIDTH);
        } else {
            rx += 1;
        }
    }

    return rx;
}

void Editor::setStatusMessage(const std::string &msg) const {
    statusMessage = msg;
    statusMessageTime = std::chrono::steady_clock::now();
}

void Editor::setCursorShapeNormal() {
    std::cout << "\033[1 q"; // Blinking block
}

void Editor::setCursorShapeInsert() {
    std::cout << "\033[5 q"; // Blinking bar
}

void Editor::editMode() {
    mode = EDIT;
    setCursorShapeInsert();
}

void Editor::normalMode() {
    mode = VIEW;
    setCursorShapeNormal();
}

void Editor::deleteToEol() {
    if (cur_y >= buffer.size()) return;

    if (std::string& line = buffer[cur_y]; cur_x < line.length()) {
        line.erase(cur_x);
    }
}

void Editor::jumpWord() {
    if (cur_y >= buffer.size()) return;

    const std::string& line = buffer[cur_y];
    const size_t len = line.length();

    if (cur_x + 1 >= len) return;

    size_t i = cur_x + 1;

    while (i < len) {
        if (!std::isalnum(line[i]) || line[i] == ' ') {
            cur_x = i;

            break;
        }

        // If we've scanned the whole line and found no jump-to point,
        // just move to eol
        if (i + 1 >= len) {
            cur_x = len - 1;
            break;
        }

        ++i;
    }
}

void Editor::jumpBack() {
    if (cur_x <= 0) return;

    const std::string& line = buffer[cur_y];
    const size_t len = line.length();

}

void Editor::jumpToEnd() {
    if (cur_x >= buffer[cur_y].size()) return;

    buffer[cur_y].resize(buffer[cur_y].size(), ' ');

    cur_x = buffer[cur_y].length();
}

void Editor::trimWhitespace(std::string &line) {
    const auto trimCharacters = " \t";

    if (const size_t firstNonWhitespace = line.find_first_not_of(trimCharacters);
        firstNonWhitespace != std::string::npos) {
        line = line.substr(firstNonWhitespace);
    }

    if (const size_t lastNonWhitespace = line.find_last_not_of(trimCharacters);
        lastNonWhitespace != std::string::npos) {
        line = line.substr(lastNonWhitespace + 1, line.length());
    }
}

std::string Editor::trimWhitespace(const std::string &line) {
    const auto trimCharacters = " ";

    const size_t first = line.find_first_not_of(trimCharacters);
    if (first == std::string::npos)
        return ""; // All spaces

    const size_t last = line.find_last_not_of(trimCharacters);

    return line.substr(first, last - first + 1);
}
