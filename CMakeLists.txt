cmake_minimum_required(VERSION 3.10)
project(Qedit)

set(CMAKE_CXX_STANDARD 17)

# Add source files (excluding test files)
file(GLOB SOURCES "lib/*.cpp" "src/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*_test\\.cpp$")

# Main executable
add_executable(Qedit
        main.cpp
        ${SOURCES}
)

# Config test executable
add_executable(config_test
        lib/config_test.cpp
        lib/Config.cpp
)
