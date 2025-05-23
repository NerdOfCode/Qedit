# Qedit - A Stupid and Semi-functioning Text Editor

Qedit is a lightweight text editor built in C++ with a simple and intuitive interface. It provides essential text editing features.


## !! Warning !!
This editor was created as a personal project for learning C++.

That being said, it *will* find a way to break so don't edit anything you actually need with it.

## Configuring

Qedit includes a configuration system that parses `~/.qeditrc` by default.

The configuration file uses a simple `key=value` format, allowing you to customize some of the editor's behavior.

### Example Config

Create a file at `~/.qeditrc` with content like:

```
# Qedit Configuration
tab_width=4
default_filename=test.txt
show_line_numbers=true
```

For more information, see the [CONFIG.md](CONFIG.md) file.

## Building

To build Qedit manually:

```bash
cd build
cmake ..
cmake --build .
```

Or, use the build script via:

```bash
./build.sh --release
```

## Testing

Qedit uses the Catch2 Testing Framework for unit tests. There are two ways to run the tests:

1. Using CMake's test runner:
```bash
cd build
cmake ..
cmake --build .

ctest
ctest --output-on-failure
```

2. Running individual test executables:
```bash
# Run all editor tests
./build/editor_test -d yes

# Run specific test cases
./build/editor_test "[editor]"  # Run all editor tests
./build/editor_test "[editor][cursor]"  # Run only cursor movement tests

# Run configuration tests
./build/config_test
```

## Usage

To run Qedit:

```bash
./Qedit [filename]
```

### Basic Commands

- `:w` - Save the file
- `:q` - Quit the editor
- `:wq` - Save and quit
- `:w filename` - Save as a new filename
