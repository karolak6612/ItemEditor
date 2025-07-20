@echo off
REM Build script for ItemEditor Qt6 on Windows

echo Building ItemEditor Qt6...

REM Check if Qt6 is in PATH or set common locations
set QT6_PATH=C:\Qt\6.9.1\msvc2022_64
if exist "C:\Qt\6.9.1\msvc2022_64" set QT6_PATH=C:\Qt\6.9.1\msvc2022_64
if exist "C:\Qt\6.5.0\msvc2019" set QT6_PATH=C:\Qt\6.5.0\msvc2019
if exist "C:\Qt\6.6.0\msvc2019" set QT6_PATH=C:\Qt\6.6.0\msvc2019
if exist "C:\Qt\6.7.0\msvc2019" set QT6_PATH=C:\Qt\6.7.0\msvc2019

REM Check for Qt6 in environment
if "%Qt6_DIR%" NEQ "" set QT6_PATH=%Qt6_DIR%

if "%QT6_PATH%" == "" (
    echo Error: Qt6 not found. Please set Qt6_DIR environment variable or install Qt6 to a standard location.
    echo Example: set Qt6_DIR=C:\Qt\6.9.1\msvc2022_64
    pause
    exit /b 1
)

echo Using Qt6 from: %QT6_PATH%

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="%QT6_PATH%" -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

REM Build the project
echo Building project...
cmake --build . --config Release --parallel

if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    cd ..
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo Executable location: build\src\ItemEditor.UI\Release\ItemEditor.exe
echo.

cd ..
pause