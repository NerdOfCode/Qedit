#ifndef EDITORCOMMANDS_H
#define EDITORCOMMANDS_H

#pragma once

namespace EditorCommands {
    // Commands
    static const std::string WRITE = ":w";
    static const std::string QUIT = ":q";
    static const std::string WRITE_QUIT = ":wq";

    // Responses
    static const std::string WROTE_TO = "wrote: ";
    static const std::string INVALID_FILENAME_MSG = "Invalid filename.";
}

#endif //EDITORCOMMANDS_H
