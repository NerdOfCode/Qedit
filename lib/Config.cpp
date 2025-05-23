//
// Created by W K on 4/24/25.
//

#include "Config.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <filesystem>

namespace QEditor {
    Config::Config() {
        parse();
    }

    Config::~Config() = default;
    
    std::string Config::getConfigFilePath() {
        const char* homeDir = std::getenv("HOME");
        if (!homeDir) {
            return "";
        }

        return std::string(homeDir) + "/" + CONFIG_FILENAME;
    }
    
    void Config::parse() {
        std::string configPath = getConfigFilePath();
        if (configPath.empty()) {
            return;
        }
        
        std::ifstream configFile(configPath);
        if (!configFile.is_open()) {
            // Config file doesn't exist or can't be opened
            return;
        }
        
        std::string line;
        while (std::getline(configFile, line)) {
            // Skip empty lines and comments
            line = trim(line);
            if (line.empty() || line[0] == '#') {
                continue;
            }
            
            // Parse key=value pair
            if (size_t pos = line.find('='); pos != std::string::npos) {
                std::string key = trim(line.substr(0, pos));
                std::string value = trim(line.substr(pos + 1));
                
                if (!key.empty()) {
                    // Try to determine the type of the value
                    if (value == "true" || value == "yes" || value == "1") {
                        set(key, true);
                    } else if (value == "false" || value == "no" || value == "0") {
                        set(key, false);
                    } else {
                        // Try to parse as an int
                        try {
                            int intValue = std::stoi(value);
                            set(key, intValue);
                        } catch (...) {
                            // Not an int, just store as string
                            set(key, value);
                        }
                    }
                }
            }
        }
    }
    
    void Config::set(const std::string& key, const ConfigValue& value) {
        values[key] = value;
    }
    
    bool Config::hasKey(const std::string& key) const {
        return values.find(key) != values.end();
    }
    
    std::optional<std::string> Config::getString(const std::string& key) const {
        const auto it = values.find(key);
        if (it == values.end()) {
            return std::nullopt;
        }
        
        const ConfigValue& value = it->second;
        if (std::holds_alternative<std::string>(value)) {
            return std::get<std::string>(value);
        }

        if (std::holds_alternative<int>(value)) {
            return std::to_string(std::get<int>(value));
        }

        if (std::holds_alternative<bool>(value)) {
            return std::get<bool>(value) ? "true" : "false";
        }

        return std::nullopt;
    }
    
    std::optional<int> Config::getInt(const std::string& key) const {
        const auto it = values.find(key);
        if (it == values.end()) {
            return std::nullopt;
        }
        
        const ConfigValue& value = it->second;
        if (std::holds_alternative<int>(value)) {
            return std::get<int>(value);
        }

        if (std::holds_alternative<std::string>(value)) {
            try {
                return std::stoi(std::get<std::string>(value));
            } catch (...) {
                return std::nullopt;
            }
        }

        if (std::holds_alternative<bool>(value)) {
            return std::get<bool>(value) ? 1 : 0;
        }
        
        return std::nullopt;
    }
    
    std::optional<bool> Config::getBool(const std::string& key) const {
        const auto it = values.find(key);
        if (it == values.end()) {
            return std::nullopt;
        }
        
        const ConfigValue& value = it->second;
        if (std::holds_alternative<bool>(value)) {
            return std::get<bool>(value);
        }

        if (std::holds_alternative<int>(value)) {
            return std::get<int>(value) != 0;
        }

        if (std::holds_alternative<std::string>(value)) {
            const auto& strValue = std::get<std::string>(value);
            if (strValue == "true" || strValue == "yes" || strValue == "1") {
                return true;
            }

            if (strValue == "false" || strValue == "no" || strValue == "0") {
                return false;
            }
        }
        
        return std::nullopt;
    }
    
    std::string Config::trim(const std::string& str) {
        const auto start = std::find_if_not(str.begin(), str.end(), [](const unsigned char c) {
            return std::isspace(c);
        });

        const auto end = std::find_if_not(str.rbegin(), str.rend(), [](const unsigned char c) {
            return std::isspace(c);
        }).base();
        
        return (start < end) ? std::string(start, end) : std::string();
    }
}
