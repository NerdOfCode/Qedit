#include <csignal>
#include <iostream>
#include <unistd.h>
#include <cstdlib>

#include "src/QEditor.h"
#include "lib/EditorError.h"

void handleSigTSTP(int);
void handleSigCONT(int);
void handleSigWINCH(int);
void setupSigTSTP();
void setupSigCONT();
void cleanupTerminal();

int main(const int argc, char *argv[]) {
    try {
        const std::string filename = (argc >= 2) ? argv[1] : "";

        // Set up signal handlers
        std::signal(SIGTSTP, handleSigTSTP);
        std::signal(SIGCONT, handleSigCONT);
        std::signal(SIGWINCH, handleSigWINCH);
        setupSigCONT();
        setupSigTSTP();

        // Initialize editor
        Editor editor;

        // Load file if specified
        if (!filename.empty()) {
            try {
                editor.loadFile(filename);
            } catch (const QEditor::FileOpenError& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                std::cerr << "Starting with empty buffer." << std::endl;
                editor.setStatusMessage("New file: " + filename);
            } catch (const QEditor::FilePermissionError& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                std::cerr << "Please check file permissions." << std::endl;
                return EXIT_FAILURE;
            }
        } else {
            editor.setStatusMessage("No file selected");
        }

        // Start editor
        editor.drawScreen();
        editor.run();

        return EXIT_SUCCESS;
    } catch (const QEditor::TerminalError& e) {
        std::cerr << "Terminal error: " << e.what() << std::endl;
        std::cerr << "Please ensure you're running in a valid terminal." << std::endl;
        cleanupTerminal();
        return EXIT_FAILURE;
    } catch (const QEditor::ConfigError& e) {
        std::cerr << "Configuration error: " << e.what() << std::endl;
        std::cerr << "Please check your ~/.qeditrc file." << std::endl;
        cleanupTerminal();
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        cleanupTerminal();
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown error occurred." << std::endl;
        cleanupTerminal();
        return EXIT_FAILURE;
    }
}

void cleanupTerminal() {
    // Restore terminal settings
    Editor::disableRawMode();
    
    // Show cursor
    std::cout << "\x1b[?25h";
    
    // Restore line wrapping
    std::cout << "\x1b[?7h";
    
    // Switch back to main screen
    std::cout << "\x1b[?1049l";
    
    std::cout.flush();
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
    sa.sa_flags = 0; // Don't use SA_RESTART
    sigaction(SIGCONT, &sa, nullptr);
}

void handleSigCONT(int) {
    try {
        Editor& editor = Editor::getInstance();
        
        // Restore raw mode
        Editor::enableRawMode();
        
        // Mark for redraw
        editor.needsRedrawn = true;
        editor.resumed = 1;
    } catch (const std::exception& e) {
        std::cerr << "Error restoring editor after suspend: " << e.what() << std::endl;
        cleanupTerminal();
        std::exit(EXIT_FAILURE);
    }
}

void handleSigWINCH(int) {
    try {
        Editor& editor = Editor::getInstance();
        editor.updateWindowSize();
        editor.needsRedrawn = true;
    } catch (const QEditor::TerminalError& e) {
        std::cerr << "Error handling window resize: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error during window resize: " << e.what() << std::endl;
    }
}
