#!/bin/bash

# Exit on error
set -e

# Default build type
BUILD_TYPE="Debug"
BUILD_DIR="build"
CLEAN=false
RUN_TESTS=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --test)
            RUN_TESTS=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--release] [--clean] [--build-dir <directory>] [--test]"
            echo "Options:"
            echo "  --release     Build in Release mode"
            echo "  --clean       Clean build directory before building"
            echo "  --build-dir   Specify build directory (default: build)"
            echo "  --test        Build and run tests"
            exit 1
            ;;
    esac
done

# Clean build directory if requested
if [ "$CLEAN" = true ]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory: $BUILD_DIR"
    mkdir -p "$BUILD_DIR"
fi

# Navigate to build directory
cd "$BUILD_DIR"

# Configure CMake
echo "Configuring CMake with build type: $BUILD_TYPE"
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" ..

# Build the project
echo "Building project..."
cmake --build . -- -j$(nproc)

if [ "$RUN_TESTS" = true ]; then
    echo "Running tests..."
    ./editor_test --reporter console -s
    ./config_test --reporter console -s
fi

echo "Build completed successfully!" 