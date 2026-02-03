@echo off
echo Building Tetris...
if exist tetris.exe del tetris.exe

where gcc >nul 2>nul
if %errorlevel% equ 0 (
    echo Using GCC...
    gcc main.c -o tetris.exe -lgdi32 -luser32
    if %errorlevel% equ 0 (
        echo Build successful! Run tetris.exe to play.
    ) else (
        echo Build failed.
    )
    goto :eof
)

where cl >nul 2>nul
if %errorlevel% equ 0 (
    echo Using MSVC...
    cl main.c user32.lib gdi32.lib /Fe:tetris.exe
    if %errorlevel% equ 0 (
        echo Build successful! Run tetris.exe to play.
    ) else (
        echo Build failed.
    )
    goto :eof
)

echo No compiler found (gcc or cl). Please install MinGW or Visual Studio.
pause
