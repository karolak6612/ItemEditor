#!/bin/bash
# Build script for ItemEditor Qt6 on Linux/Ubuntu

echo "Building ItemEditor Qt6 for Linux..."

# Check for required dependencies
check_dependency() {
    if ! command -v $1 &> /dev/null; then
        echo "Error: $1 is not installed. Please install it first."
        echo "On Ubuntu/Debian: sudo apt install $2"
        exit 1
    fi
}

# Check for essential build tools
check_dependency "cmake" "cmake"
check_dependency "make" "build-essential"
check_dependency "g++" "build-essential"

# Check for Qt6 installation
if ! pkg-config --exists Qt6Core Qt6Widgets Qt6Gui; then
    echo "Error: Qt6 development packages not found."
    echo "Please install Qt6 development packages:"
    echo "  Ubuntu 22.04+: sudo apt install qt6-base-dev qt6-tools-dev cmake build-essential"
    echo "  Ubuntu 20.04:  Install Qt6 from official installer or snap"
    echo "  Or set CMAKE_PREFIX_PATH to your Qt6 installation directory"
    exit 1
fi

# Get Qt6 installation path
QT6_PATH=""
if pkg-config --exists Qt6Core; then
    QT6_LIBDIR=$(pkg-config --variable=libdir Qt6Core)
    QT6_PATH=$(dirname "$QT6_LIBDIR")
    echo "Found Qt6 at: $QT6_PATH"
else
    echo "Warning: Could not detect Qt6 path automatically"
    echo "You may need to set CMAKE_PREFIX_PATH manually"
fi

# Create build directory
if [ ! -d "build" ]; then
    mkdir build
fi
cd build

# Configure with CMake
echo "Configuring CMake..."
CMAKE_ARGS="-DCMAKE_BUILD_TYPE=Release"

if [ ! -z "$QT6_PATH" ]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_PREFIX_PATH=$QT6_PATH"
fi

# Add user-specified CMAKE_PREFIX_PATH if provided
if [ ! -z "$CMAKE_PREFIX_PATH" ]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH"
fi

cmake .. $CMAKE_ARGS

if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    cd ..
    exit 1
fi

# Build the project
echo "Building project..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "Build failed!"
    cd ..
    exit 1
fi

echo ""
echo "Build completed successfully!"
echo "Executable location: build/bin/itemeditor"
echo ""
echo "To install system-wide, run:"
echo "  sudo make install"
echo ""
echo "To run from build directory:"
echo "  cd build && ./bin/itemeditor"
echo ""

cd ..