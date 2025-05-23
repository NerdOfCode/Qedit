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

        static constexpr const char* CONFIG_FILENAME = ".qeditrc";

        // Parse the config file
        void parse();
        
        // Get config values with type conversion
        [[nodiscard]] std::optional<std::string> getString(const std::string& key) const;
        [[nodiscard]] std::optional<int> getInt(const std::string& key) const;
        [[nodiscard]] std::optional<bool> getBool(const std::string& key) const;
        
        // Set config values
        void set(const std::string& key, const ConfigValue& value);
        
        // Check if a key exists
        [[nodiscard]] bool hasKey(const std::string& key) const;
        
        // Get the path to the config file
        static std::string getConfigFilePath();
        
    private:
        std::unordered_map<std::string, ConfigValue> values;
        
        // Trim whitespace from a string
        static std::string trim(const std::string& str);
    };
}

#endif //CONFIG_H
