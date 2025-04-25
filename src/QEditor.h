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

    void processKeypress();
    void moveCursor(char direction);
    void insertText(char c);
    void deleteText();
    void loadFile(const std::string &filename);
    void saveFile(const std::string &filename);
    void insertNewline();
    void deleteLine();
    void processCommand();
    void setStatusMessage(const std::string& msg);

    void drawScreen() const;
    void updateWindowSize();
    static void clearScreen();

    bool needsRedrawn = false;
    std::string filename;
    int screenRows{};
    int screenCols{};

    volatile sig_atomic_t resumed = 0;

    static void enableRawMode();
    static void disableRawMode();
private:
    std::string expandTabs(const std::string& line) const;
    int getRenderX(const std::string& line, int cur_x) const;

    void jumpWord();
    void deleteToEol();

    void editMode();
    void normalMode();

    static void setCursorShapeNormal();
    static void setCursorShapeInsert();

    // Start in View Mode
    Mode mode = VIEW;

    // Configuration
    QEditor::Config config;
    int TAB_WIDTH = 4; // Now can be configured
    bool showLineNumbers = false; // Default to not showing line numbers
    bool running = true;

    std::string commandBuffer;
    std::vector<std::string> buffer;

    std::string statusMessage;
    std::chrono::time_point<std::chrono::steady_clock> statusMessageTime;

    std::vector<std::string> history;

    int cur_x{}, cur_y = 0;
};
