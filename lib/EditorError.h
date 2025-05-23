#pragma once

#include <stdexcept>
#include <string>

namespace QEditor {

// Base exception class for all editor errors
class EditorError : public std::runtime_error {
public:
    explicit EditorError(const std::string& message) : std::runtime_error(message) {}
};

// File operation errors
class FileError : public EditorError {
public:
    explicit FileError(const std::string& message) : EditorError("File error: " + message) {}
};

class FileOpenError : public FileError {
public:
    explicit FileOpenError(const std::string& filename) 
        : FileError("Could not open file: " + filename) {}
};

class FileSaveError : public FileError {
public:
    explicit FileSaveError(const std::string& filename) 
        : FileError("Could not save file: " + filename) {}
};

class FilePermissionError : public FileError {
public:
    explicit FilePermissionError(const std::string& filename) 
        : FileError("Permission denied: " + filename) {}
};

// Configuration errors
class ConfigError : public EditorError {
public:
    explicit ConfigError(const std::string& message) : EditorError("Configuration error: " + message) {}
};

class ConfigParseError : public ConfigError {
public:
    explicit ConfigParseError(const std::string& message) 
        : ConfigError("Failed to parse configuration: " + message) {}
};

class ConfigValueError : public ConfigError {
public:
    explicit ConfigValueError(const std::string& key, const std::string& expected_type) 
        : ConfigError("Invalid value for '" + key + "', expected " + expected_type) {}
};

// Terminal/UI errors
class TerminalError : public EditorError {
public:
    explicit TerminalError(const std::string& message) : EditorError("Terminal error: " + message) {}
};

class TerminalSizeError : public TerminalError {
public:
    TerminalSizeError(int rows, int cols) 
        : TerminalError("Invalid terminal size: " + std::to_string(rows) + "x" + std::to_string(cols)) {}
};

// Buffer operation errors
class BufferError : public EditorError {
public:
    explicit BufferError(const std::string& message) : EditorError("Buffer error: " + message) {}
};

class BufferBoundsError : public BufferError {
public:
    BufferBoundsError(size_t x, size_t y, size_t max_x, size_t max_y)
        : BufferError("Cursor position out of bounds: (" + 
                     std::to_string(x) + "," + std::to_string(y) + 
                     ") exceeds (" + std::to_string(max_x) + "," + 
                     std::to_string(max_y) + ")") {}
};

// Command errors
class CommandError : public EditorError {
public:
    explicit CommandError(const std::string& message) : EditorError("Command error: " + message) {}
};

class InvalidCommandError : public CommandError {
public:
    explicit InvalidCommandError(const std::string& command) 
        : CommandError("Invalid command: " + command) {}
};

} // namespace QEditor 