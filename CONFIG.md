# Qedit Configuration

Qedit can be configured using a configuration file located at `~/.qedit.rc`. The file uses a simple key-value format with each setting on its own line.

## Configuration Format

The configuration file uses a simple key-value format:
```
key=value
```

- Lines beginning with `#` are treated as comments and ignored
- Empty lines are ignored
- Boolean values can be specified as `true`/`false`, `yes`/`no`, or `1`/`0`

## Available Settings

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| tab_width | Integer | 4 | Width of tab characters in spaces |
| default_filename | String | test.txt | Default filename when saving without specifying a name |
| show_line_numbers | Boolean | false | Show line numbers in the editor |

## Sample Configuration

```
# Qedit Configuration File
# If you want to configure QEditor, place this file in your home directory: ~/.qedit.rc

# Editor settings
tab_width=8  # Tab width in spaces
default_filename=test.txt  # Default filename for saving

# Appearance
show_line_numbers=true  # Show line numbers
```

## How to Apply Settings

The config is loaded when QEdit starts. To apply changes to your configuration:

1. Edit `~/.qedit.rc` with your preferred settings
2. Restart QEdit

Some settings may be changed at runtime through the editor's command mode.
These changes will not be persisted to the configuration file.