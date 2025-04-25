#include <iostream>
#include "Config.h"

int main() {
    // Create a config instance
    QEditor::Config config;
    
    // Parse config file
    config.parse();
    
    // Display configuration values
    std::cout << "Configuration from " << QEditor::Config::getConfigFilePath() << ":\n";
    std::cout << "-------------------------\n";
    
    // Check for tab_width
    if (const auto tabWidth = config.getInt("tab_width")) {
        std::cout << "tab_width = " << *tabWidth << "\n";
    } else {
        std::cout << "tab_width not set, using default (4)\n";
    }
    
    // Check for default_filename
    if (const auto defaultFilename = config.getString("default_filename")) {
        std::cout << "default_filename = " << *defaultFilename << "\n";
    } else {
        std::cout << "default_filename not set, using default (test.txt)\n";
    }
    
    // Check for show_line_numbers
    if (const auto showLineNumbers = config.getBool("show_line_numbers")) {
        std::cout << "show_line_numbers = " << (*showLineNumbers ? "true" : "false") << "\n";
    } else {
        std::cout << "show_line_numbers not set, using default (false)\n";
    }
    
    // Check for highlight_current_line
    if (const auto highlightCurrentLine = config.getBool("highlight_current_line")) {
        std::cout << "highlight_current_line = " << (*highlightCurrentLine ? "true" : "false") << "\n";
    } else {
        std::cout << "highlight_current_line not set, using default (false)\n";
    }
    
    // Test setting and getting a value
    std::cout << "\nTesting setting and getting values:\n";
    config.set("new_string_value", std::string("test string"));
    config.set("new_int_value", 42);
    config.set("new_bool_value", true);
    
    std::cout << "new_string_value = " << *config.getString("new_string_value") << "\n";
    std::cout << "new_int_value = " << *config.getInt("new_int_value") << "\n";
    std::cout << "new_bool_value = " << (*config.getBool("new_bool_value") ? "true" : "false") << "\n";
    
    return 0;
}