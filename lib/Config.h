//
// Created by W K on 4/24/25.
//

#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>
#include <variant>
#include <optional>

namespace QEditor {
    class Config {
    public:
        // Supported config value types
        using ConfigValue = std::variant<std::string, int, bool>;
        
        Config();
        ~Config();

        // Parse the config file
        void parse();
        
        // Get config values with type conversion
        std::optional<std::string> getString(const std::string& key) const;
        std::optional<int> getInt(const std::string& key) const;
        std::optional<bool> getBool(const std::string& key) const;
        
        // Set config values
        void set(const std::string& key, const ConfigValue& value);
        
        // Check if a key exists
        bool hasKey(const std::string& key) const;
        
        // Get the path to the config file
        static std::string getConfigFilePath();
        
    private:
        std::unordered_map<std::string, ConfigValue> values;
        
        // Trim whitespace from a string
        static std::string trim(const std::string& str);
    };
}

#endif //CONFIG_H
