#!/bin/bash

cd build

# Run CMake and build the config_test target
cmake ..
cmake --build . --target config_test

# Check if build was successful
if [ $? -eq 0 ]; then
    echo -e "\nBuild successful. Running config_test...\n"

    ./config_test
else
    echo -e "\nBuild failed."
fi