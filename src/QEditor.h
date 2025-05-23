//
// Created by Nathan Wander
//

#pragma once
#include <string>
#include <vector>
#include <chrono>
#include "../lib/Config.h"

enum Mode { VIEW, EDIT, COMMAND };

class Editor {
public:
    Editor();
    ~Editor();

    static Editor& getInstance();

    void run();
    void stop();
    void loadFile(const std::string& filename);
    void saveFile(const std::string& filename) const;
    void clearScreen();
    void updateWindowSize();
    void setStatusMessage(const std::string& msg) const;
    void setCursorShapeNormal();
    void setCursorShapeInsert();
    void editMode();
    void normalMode();

    void processKeypress();
    void moveCursor(char direction);
    void insertText(char c);
    void deleteChar();
    void insertNewline();
    void deleteLine();
    void processCommand();

    void drawScreen() const;

    bool needsRedrawn = false;
    std::string filename;

    size_t screenRows{}, screenCols{};

    volatile sig_atomic_t resumed = 0;

    static void enableRawMode();
    static void disableRawMode();

#ifdef TESTING
    // Test helper methods
    [[nodiscard]] bool isInEditMode() const { return mode == EDIT; }
    [[nodiscard]] bool isInNormalMode() const { return mode == VIEW; }
    [[nodiscard]] bool isInCommandMode() const { return mode == COMMAND; }
    [[nodiscard]] Mode getMode() const { return mode; }
    [[nodiscard]] const std::vector<std::string>& getBuffer() const { return buffer; }
    [[nodiscard]] size_t getCursorX() const { return cur_x; }
    [[nodiscard]] size_t getCursorY() const { return cur_y; }
    [[nodiscard]] const std::string& getCommandBuffer() const { return commandBuffer; }
    [[nodiscard]] const std::string& getStatusMessage() const { return statusMessage; }
    [[nodiscard]] const std::string& getFilename() const { return filename; }
    [[nodiscard]] bool isRunning() const { return running; }
    [[nodiscard]] bool isShowLineNumbers() const { return showLineNumbers; }
    [[nodiscard]] int getTabWidth() const { return TAB_WIDTH; }
    [[nodiscard]] size_t getScreenRows() const { return screenRows; }
    [[nodiscard]] size_t getScreenCols() const { return screenCols; }

    // Test-only methods
    void setCursorPosition(size_t x, size_t y) {
        cur_x = x;
        cur_y = y;
    }
    void clearBuffer() {
        buffer.clear();
        cur_x = 0;
        cur_y = 0;
    }
#endif

private:
    [[nodiscard]] std::string expandTabs(const std::string& line) const;
    [[nodiscard]] int getRenderX(const std::string& line, size_t cur_x) const;

    void jumpWord();
    void jumpToEnd();
    void jumpBack();

    void deleteToEol();

    static void trimWhitespace(std::string& line);
    static std::string trimWhitespace(const std::string &line);

    // Default is View Mode
    Mode mode = VIEW;

    // Configuration
    QEditor::Config config;
    size_t TAB_WIDTH = 4; // Now can be configured
    bool showLineNumbers = false; // Default to not showing line numbers
    bool running = true;

    std::string commandBuffer;
    std::vector<std::string> buffer;

    mutable std::string statusMessage;
    mutable std::chrono::time_point<std::chrono::steady_clock> statusMessageTime;

    std::vector<std::string> history;

    size_t cur_x = 0, cur_y = cur_x;
};
