# C Language Tetris (Win32 API)

This is a simple Tetris game written in C using the native Windows API (Win32). It does not require any external third-party libraries (like SDL or Raylib), making it easy to compile on any Windows machine with a C compiler.

## Features
- Classic Tetris gameplay
- Score tracking
- Next piece preview
- Pause functionality (Press 'P')
- Arrow keys for control

## How to Compile

You need a C compiler installed on your system.

### Option 1: Using GCC (MinGW)
If you have MinGW installed:
1. Open a terminal in this folder.
2. Run `build.bat` OR execute manually:
   ```bash
   gcc main.c -o tetris.exe -lgdi32 -luser32
   ```

### Option 2: Using Visual Studio (MSVC)
1. Open "Developer Command Prompt for VS".
2. Navigate to this folder.
3. Run `build.bat` OR execute manually:
   ```cmd
   cl main.c user32.lib gdi32.lib /Fe:tetris.exe
   ```

## Controls
- **Left/Right Arrow**: Move piece
- **Up Arrow**: Rotate piece
- **Down Arrow**: Soft drop
- **P**: Pause/Resume
- **Enter**: Restart (when Game Over)
