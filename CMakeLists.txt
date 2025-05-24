cmake_minimum_required(VERSION 3.10)
project(Qedit)

set(CMAKE_CXX_STANDARD 17)

# Platform-specific filesystem library
if(APPLE)
    # On macOS, filesystem is part of the standard library
    set(FILESYSTEM_LIB "")
else()
    # On Linux, we need to link against stdc++fs
    set(FILESYSTEM_LIB stdc++fs)
endif()

# Include FetchContent for downloading Catch2
include(FetchContent)

# Download and configure Catch2
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.3
)
FetchContent_MakeAvailable(Catch2)

# Add source files (excluding test files)
file(GLOB SOURCES "lib/*.cpp" "src/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*_test\\.cpp$")

# Main executable
add_executable(Qedit
        main.cpp
        ${SOURCES}
        src/EditorCommands.h
)
if(FILESYSTEM_LIB)
    target_link_libraries(Qedit PRIVATE ${FILESYSTEM_LIB})
endif()

# Config test executable
add_executable(config_test
        lib/config_test.cpp
        lib/Config.cpp
)
if(FILESYSTEM_LIB)
    target_link_libraries(config_test PRIVATE ${FILESYSTEM_LIB})
endif()

# Editor test executable
add_executable(editor_test
        tests/editor_test.cpp
        ${SOURCES}
)
target_link_libraries(editor_test PRIVATE Catch2::Catch2WithMain)
if(FILESYSTEM_LIB)
    target_link_libraries(editor_test PRIVATE ${FILESYSTEM_LIB})
endif()
target_compile_definitions(editor_test PRIVATE CATCH_CONFIG_FAST_COMPILE)

# Enable testing
enable_testing()
add_test(NAME editor_test COMMAND editor_test)
add_test(NAME config_test COMMAND config_test)
