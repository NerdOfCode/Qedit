#include "QEditor.h"

#include <fstream>
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <sys/ioctl.h>

static termios orig_termios;

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
                mode = COMMAND;
                commandBuffer.clear();
                commandBuffer += c;
                std::cout << "\033[" << screenRows << ";1H";
            } else if (c == 'x') {
                deleteText();
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
            } else if (c == 'D') {
                deleteToEol();
            } else if (c == 'w') {
                jumpWord();
            } else if (c == '0') {
                cur_x = 0;
            } else {
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
                deleteText();
            } else if (c == '\n') {
                insertNewline();
            } else if (c == '\t') {
                buffer[cur_y].insert(cur_x, 1, '\t');
                ++cur_x;
            } else { // insert
                insertText(c);
            }
        } else if (mode == COMMAND) {
            if (c == '\n') {
                processCommand();

                mode = VIEW;
            } else if (c == 127) { // backspace
                if (!commandBuffer.empty()) commandBuffer.pop_back();
                if (commandBuffer.empty()) mode = VIEW;
            } else if (c == 27) { // esc
                mode = VIEW;
                commandBuffer.clear();
            } else {
                commandBuffer += c;
            }
        }
    }

    drawScreen();
}

void Editor::moveCursor(char direction) {
    if (direction == 'h' || direction == 127) { // Move left
        if (cur_x > 0) {
            --cur_x;
        }
    } else if (direction == 'l') { // Move right
        if (cur_x + 1 < buffer[cur_y].size()) {
            ++cur_x;
        }
    } else if (direction == 'j') { // Move down
        if (cur_y + 1 < buffer.size()) {
            ++cur_y;

            if (cur_x > buffer[cur_y].size()) {
                cur_x = buffer[cur_y].size() - 1;
            }
        }
    } else if (direction == 'k') { // Move up
        if (cur_y > 0) {
            --cur_y;

            if (cur_x > buffer[cur_y].size()) {
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

void Editor::deleteText() {
    if (cur_x >= 0) {
        buffer[cur_y].erase(cur_x, 1);
        --cur_x;
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

void Editor::saveFile(const std::string& filename) {
    std::ofstream file(filename);

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
    int lineNumWidth = 0;
    if (showLineNumbers) {
        lineNumWidth = std::to_string(buffer.size()).length() + 1; // +1 for the space after
    }

    // Draw content line by line with explicit positioning
    for (int i = 0; i < screenRows - 1; ++i) {
        // Position cursor at start of each line
        std::cout << "\x1b[" << (i+1) << ";1H";

        // Clear the entire line first
        std::cout << "\x1b[2K";

        // Draw line number if enabled
        if (showLineNumbers) {
            if (i < buffer.size()) {
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
        } else {
            std::cout << "~";
        }
    }

    // Draw status/command line
    std::cout << "\x1b[" << screenRows << ";1H\x1b[2K";

    if (mode == COMMAND && !commandBuffer.empty()) {
        std::cout << ":" << commandBuffer.substr(1);
    }

    // if (
    //     const auto now = std::chrono::steady_clock::now(); !statusMessage.empty()
    //     && std::chrono::duration_cast<std::chrono::seconds>(now - statusMessageTime).count() < 3
    // ) {
        // const int row = screenRows; // last row
        // int col = screenCols - static_cast<int>(statusMessage.length()) + 1;
        // if (col < 1) col = 1; // protect against overflow
        //
        // std::cout << "\x1b[" << row << ";" << col << "H" << statusMessage;
    // }

    // Show status message if any
    if (!statusMessage.empty()) {
        int col = screenCols - static_cast<int>(statusMessage.length()) + 1;
        if (col < 1) col = 1;
        std::cout << "\x1b[" << screenRows << ";" << col << "H" << statusMessage;
    }

    // Position cursor at edit location
    const int renderX = getRenderX(buffer[cur_y], cur_x);
    if (showLineNumbers) {
        lineNumWidth = std::to_string(buffer.size()).length() + 1; // +1 for the space after
    }
    std::cout << "\x1b[" << (cur_y + 1) << ";" << (renderX + lineNumWidth + 1) << "H";

    // Show cursor again
    std::cout << "\x1b[?25h";

    std::cout.flush();
}

void Editor::processCommand() {
    if (commandBuffer == ":w") {
        std::string file = this->filename;

        if (file.empty()) {
            const auto defaultFilename = config.getString("default_filename");

            file = *defaultFilename;
        }

        saveFile(file);
        setStatusMessage("wrote: \"" + file + "\"");
    } else if (commandBuffer == ":q") {
        running = false;
    } else if (commandBuffer.substr(0, 3) == ":w ") {
        // Save to specified file
        std::string saveFilename = commandBuffer.substr(3);
        saveFile(saveFilename);
        filename = saveFilename; // Update current filename
        setStatusMessage("wrote: " + saveFilename);
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

    buffer.insert(buffer.begin() + cur_y + 1, new_line);

    ++cur_y;
    cur_x = 0;
}

void Editor::deleteLine() {
    if (cur_y < 0 || cur_y >= buffer.size()) return;

    buffer.erase(buffer.begin() + cur_y);

    // Move cursor up to start of previous line
    --cur_y;
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
            const int spaces = TAB_WIDTH - (result.length() % TAB_WIDTH);
            result += std::string(spaces, ' ');
        } else {
            result += ch;
        }
    }

    return result;
}

int Editor::getRenderX(const std::string &line, int cur_x) const {
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

void Editor::editMode() {
    mode = EDIT;
    setCursorShapeInsert();
}

void Editor::normalMode() {
    mode = VIEW;
    setCursorShapeNormal();
}

void Editor::setStatusMessage(const std::string &msg) {
    statusMessage = msg;
    statusMessageTime = std::chrono::steady_clock::now();
}

void Editor::setCursorShapeNormal() {
    std::cout << "\033[1 q"; // Blinking block
}

void Editor::setCursorShapeInsert() {
    std::cout << "\033[5 q"; // Blinking bar
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
            cur_x = static_cast<int>(i);

            break;
        }

        // If we've scanned the whole line and found no jump-to point,
        // just move to eol
        if (i + 1 >= len) {
            cur_x = static_cast<int>(len) - 1;
            break;
        }

        ++i;
    }
}
