# ItemEditor Qt6

A Qt6 C++ port of the ItemEditor application, originally written in C# WinForms. This application is designed for editing Tibia OTB (Open Tibia Binary) files and managing game item data.

## Project Structure

The project follows a modular architecture with clear separation of concerns:

- **src/**: Source code implementations
- **include/**: Header files and public interfaces  
- **resources/**: Application resources (stylesheets, icons)
- **docs/**: Documentation files
- **tests/**: Unit tests and test data
- **build/**: Build output directory

For detailed structure information, see [docs/PROJECT_STRUCTURE.md](docs/PROJECT_STRUCTURE.md).

## Features

- OTB file reading and writing
- Item data management and editing
- Plugin system for client version compatibility
- Sprite and image handling
- Modern Qt6 user interface

## Building

### Prerequisites

- Qt6 (6.0 or later)
- CMake (3.16 or later)
- C++17 compatible compiler

### Build Instructions

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Windows with Visual Studio

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
```

## Plugin System

The application supports plugins for different Tibia client versions:
- Version 7.70 support
- Version 8.60+ support
- Extensible plugin interface

## Contributing

1. Follow the established project structure
2. Maintain C++17 compatibility
3. Use Qt6 coding conventions
4. Add tests for new functionality

## License

See LICENSE file for details.