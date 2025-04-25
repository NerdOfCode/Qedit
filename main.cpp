#include <csignal>
#include <iostream>
#include <unistd.h>

#include "src/QEditor.h"

void handleSigTSTP(int);
void handleSigCONT(int);
void handleSigWINCH(int);

void setupSigTSTP();
void setupSigCONT();

int main(const int argc, char *argv[]) {
    const std::string filename = (argc >= 2) ? argv[1] : "";

    Editor editor;

    std::signal(SIGTSTP, handleSigTSTP);
    std::signal(SIGCONT, handleSigCONT);
    std::signal(SIGWINCH, handleSigWINCH);

    setupSigCONT();
    setupSigTSTP();

    if (!filename.empty()) {
        editor.loadFile(filename);
        editor.setStatusMessage("\"" + filename + "\"");
    } else {
        editor.setStatusMessage("No file selected");
    }

    editor.drawScreen();
    editor.run();

    return 0;
}

void handleSigTSTP(int) {
    // Restore terminal before suspending
    Editor::disableRawMode();
    std::signal(SIGTSTP, SIG_DFL); // Reset to default
    raise(SIGTSTP); // Re-send signal
}

void setupSigTSTP() {
    struct sigaction sa{};
    sa.sa_handler = handleSigTSTP;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTSTP, &sa, nullptr);
}

void setupSigCONT() {
    struct sigaction sa{};
    sa.sa_handler = handleSigCONT;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // <--- Don't use SA_RESTART
    sigaction(SIGCONT, &sa, nullptr);
}

void handleSigCONT(int) {
    Editor& editor = Editor::getInstance();

    // Restore raw mode
    Editor::enableRawMode();

    // Mark for redraw but don't draw yet - main loop will handle it
    editor.needsRedrawn = true;
    editor.resumed = 1;

    // No need to call drawScreen() here, the main loop will do it
}

void handleSigWINCH(int) {
    Editor& editor = Editor::getInstance();

    editor.updateWindowSize();
    editor.needsRedrawn = true;
}
