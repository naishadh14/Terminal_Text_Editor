# Text Editor

This program is a simple terminal-based text editor that enables users to create, edit, and save text files.

## Installation

Make sure you have the `ncurses` library installed on your system.

```sudo apt-get install libncurses5-dev libncursesw5-dev```

## Usage

Compile the program by running the make command:

```bash
make
```

Alternatively, the precompiled executable is provided in case manual compilation is not required.

To run the text editor:

```bash
./editor [filename]
```

Replace `[filename]` with the name of the text file you want to open or create. If the file doesn't exist, a new one will be created with that name.

## Key Bindings

- **Arrow keys** - Navigate through the text.
- **Backspace (KEY_BACKSPACE)** - Delete a character from the text.
- **Delete (KEY_DC)** - Remove the character at the cursor.
- **Enter (Return)** - Add a new line.
- **F2** - Save the changes and exit.
- **F3** - Exit the program without saving.

## Features

- **Basic Text Editing:** Insert, delete, and navigate text.
- **File Operations:** Save changes made in the editor.
- **Supports Terminal Resizing:** Adjusts to terminal size changes dynamically.

## Architecture

The text content is stored in a 2D character array. Each row represents a line of text, allowing the program to manage the text content efficiently. The editor manipulates this array for different text operations.
