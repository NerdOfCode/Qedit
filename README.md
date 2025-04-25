# Qedit - A Stupid, Simple Text Editor

Qedit is a lightweight text editor built in C++ with a simple and intuitive interface. It provides essential text editing features.

*Note:* This editor was created as a personal project for learning C++

## Configuring

Qedit includes a configuration system that parses `~/.qedit.rc`. The configuration file uses a simple `key=value` format, allowing you to customize the editor's behavior.

### Example Config

Create a file at `~/.qedit.rc` with content like:

```
# Qedit Configuration
tab_width=4
default_filename=test.txt
show_line_numbers=true
```

For more information, see the [CONFIG.md](CONFIG.md) file.

## Building and Testing

To build Qedit:

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

To run the configuration system test:

```bash
./test_config.sh
```

## Usage

To run Qedit:

```bash
./Qedit [filename]
```

### Basic Commands

- `:w` - Save the file
- `:q` - Quit the editor
- `:w filename` - Save as a new filename