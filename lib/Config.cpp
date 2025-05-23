#include "Config.h"
#include "EditorError.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <pwd.h>
#include <unistd.h>

namespace QEditor {
    Config::Config() = default;

    Config::~Config() = default;
    
    std::string Config::getConfigFilePath() {
        const char* home = getenv("HOME");
        if (!home) {
            // Fallback to getpwuid if HOME is not set
            if (const passwd* pw = getpwuid(getuid())) {
                home = pw->pw_dir;
            } else {
                throw ConfigError("Could not determine home directory");
            }
        }
        return std::string(home) + "/" + CONFIG_FILENAME;
    }
    
    void Config::parse() {
        const std::string configPath = getConfigFilePath();
        
        // Check if config file exists
        if (!std::filesystem::exists(configPath)) {
            return; // No config file is not an error
        }

        // Check file permissions
        if (access(configPath.c_str(), R_OK) != 0) {
            throw FilePermissionError(configPath);
        }

        std::ifstream file(configPath);
        if (!file) {
            throw FileOpenError(configPath);
        }

        std::string line;
        int lineNum = 0;
        while (std::getline(file, line)) {
            ++lineNum;
            
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') {
                continue;
            }

            // Parse key=value pair
            size_t pos = line.find('=');
            if (pos == std::string::npos) {
                throw ConfigParseError("Invalid format at line " + std::to_string(lineNum) + 
                                     ": missing '=' in '" + line + "'");
            }

            std::string key = trim(line.substr(0, pos));
            std::string value = trim(line.substr(pos + 1));

            if (key.empty()) {
                throw ConfigParseError("Empty key at line " + std::to_string(lineNum));
            }

            // Convert and store the value
            try {
                if (value == "true" || value == "yes" || value == "1") {
                    set(key, true);
                } else if (value == "false" || value == "no" || value == "0") {
                    set(key, false);
                } else if (std::all_of(value.begin(), value.end(), ::isdigit)) {
                    set(key, std::stoi(value));
                } else {
                    set(key, value);
                }
            } catch (const std::invalid_argument&) {
                throw ConfigValueError(key, "valid value");
            } catch (const std::out_of_range&) {
                throw ConfigValueError(key, "value within valid range");
            }
        }
    }
    
    void Config::set(const std::string& key, const ConfigValue& value) {
        if (key.empty()) {
            throw ConfigError("Cannot set empty key");
        }
        values[key] = value;
    }
    
    bool Config::hasKey(const std::string& key) const {
        return values.find(key) != values.end();
    }
    
    std::optional<std::string> Config::getString(const std::string& key) const {
        if (auto it = values.find(key); it != values.end()) {
            if (auto* str = std::get_if<std::string>(&it->second)) {
                return *str;
            }
            throw ConfigValueError(key, "string");
        }
        return std::nullopt;
    }
    
    std::optional<int> Config::getInt(const std::string& key) const {
        if (auto it = values.find(key); it != values.end()) {
            if (auto* num = std::get_if<int>(&it->second)) {
                return *num;
            }
            throw ConfigValueError(key, "integer");
        }
        return std::nullopt;
    }
    
    std::optional<bool> Config::getBool(const std::string& key) const {
        if (auto it = values.find(key); it != values.end()) {
            if (auto* b = std::get_if<bool>(&it->second)) {
                return *b;
            }
            throw ConfigValueError(key, "boolean");
        }
        return std::nullopt;
    }
    
    std::string Config::trim(const std::string& str) {
        const auto first = str.find_first_not_of(" \t");
        if (first == std::string::npos) {
            return "";
        }
        const auto last = str.find_last_not_of(" \t");
        return str.substr(first, (last - first + 1));
    }
}
