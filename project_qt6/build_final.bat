@echo off
setlocal EnableDelayedExpansion

echo ===============================================
echo ItemEditor Qt6 - Final Release Build Script
echo ===============================================
echo.

:: Set build configuration
set BUILD_TYPE=Release
set BUILD_DIR=build-release
set PROJECT_DIR=%~dp0

echo Project Directory: %PROJECT_DIR%
echo Build Directory: %BUILD_DIR%
echo Build Type: %BUILD_TYPE%
echo.

:: Navigate to project directory
cd /d "%PROJECT_DIR%"

:: Create build directory if it doesn't exist
if not exist "%BUILD_DIR%" (
    echo Creating build directory...
    mkdir "%BUILD_DIR%"
)

:: Navigate to build directory
cd "%BUILD_DIR%"

echo ===============================================
echo Step 1: Configuring CMake for Release build
echo ===============================================

:: Configure CMake for Release build
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_CXX_FLAGS_RELEASE="/O2 /Ob2 /DNDEBUG" ^
    -DCMAKE_C_FLAGS_RELEASE="/O2 /Ob2 /DNDEBUG"

if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo ===============================================
echo Step 2: Building Release executable
echo ===============================================

:: Build the project
cmake --build . --config Release --parallel

if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo ===============================================
echo Step 3: Copying final executable
echo ===============================================

:: Copy the executable to project root
if exist "Release\ItemEditorQt.exe" (
    copy "Release\ItemEditorQt.exe" "..\ItemEditorQt_Final.exe"
    echo Final executable created: ItemEditorQt_Final.exe
) else (
    echo ERROR: Executable not found!
    pause
    exit /b 1
)

echo.
echo ===============================================
echo Build completed successfully!
echo ===============================================
echo.
echo Final executable: ItemEditorQt_Final.exe
echo Location: %PROJECT_DIR%
echo.

:: Ask user if they want to run the application
set /p choice="Do you want to run the application now? (y/n): "
if /i "%choice%"=="y" (
    echo Starting ItemEditor Qt6...
    cd /d "%PROJECT_DIR%"
    start ItemEditorQt_Final.exe
)

echo.
echo Build script completed.
pause