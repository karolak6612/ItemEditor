# Qt6 ItemEditor Project Structure

This document describes the organized project structure for the Qt6 ItemEditor application, designed to match C# architecture patterns.

## Directory Structure

```
project_qt6/
├── CMakeLists.txt              # Main build configuration
├── build/                      # Build output directory
├── docs/                       # Documentation files
├── resources/                  # Application resources
│   ├── resources.qrc          # Qt resource file
│   └── dark.qss               # Dark theme stylesheet
├── include/                    # Header files (public interfaces)
│   ├── core/                  # Core application headers
│   ├── ui/                    # User interface headers
│   │   ├── mainwindow.h
│   │   ├── aboutdialog.h
│   │   ├── compareotbdialog.h
│   │   ├── finditemdialog.h
│   │   ├── preferencesdialog.h
│   │   ├── spritecandidatesdialog.h
│   │   ├── updateotbdialog.h
│   │   └── clientitemview.h
│   ├── plugins/               # Plugin system headers
│   │   ├── interface/         # Plugin interface definitions
│   │   └── host/              # Plugin host system headers
│   ├── otb/                   # OTB file handling headers
│   │   ├── binarytree.h
│   │   ├── item.h
│   │   ├── otbreader.h
│   │   ├── otbtypes.h
│   │   └── otbwriter.h
│   ├── tibiadata/             # Tibia data format headers
│   │   ├── datparser.h
│   │   ├── imagesimilarity.h
│   │   └── sprparser.h
│   └── helpers/               # Utility and helper headers
├── src/                       # Source files (implementations)
│   ├── main.cpp               # Application entry point
│   ├── core/                  # Core application logic
│   ├── ui/                    # User interface implementations
│   │   ├── mainwindow.cpp
│   │   ├── dialogs/           # Dialog implementations
│   │   └── widgets/           # Custom widget implementations
│   │       └── clientitemview.cpp
│   ├── plugins/               # Plugin system implementations
│   │   ├── interface/         # Plugin interface implementations
│   │   └── host/              # Plugin host system
│   ├── otb/                   # OTB file handling implementations
│   │   ├── binarytree.cpp
│   │   ├── item.cpp
│   │   ├── otbreader.cpp
│   │   ├── otbtypes.cpp
│   │   └── otbwriter.cpp
│   ├── tibiadata/             # Tibia data format implementations
│   │   ├── datparser.cpp
│   │   ├── imagesimilarity.cpp
│   │   └── sprparser.cpp
│   └── helpers/               # Utility and helper implementations
└── tests/                     # Unit tests and test files
```

## Module Organization

### Core Module
- Application initialization and main logic
- Application-wide utilities and base classes

### UI Module
- Main window and application interface
- Dialog windows for various functions
- Custom widgets and controls

### OTB Module
- Binary tree data structure for OTB files
- OTB file reading and writing functionality
- Item data structures and management

### TibiaData Module
- DAT file parsing for client data
- SPR file parsing for sprite data
- Image similarity algorithms

### Plugin System
- Plugin interface definitions
- Plugin host and management system
- Dynamic loading and version compatibility

### Helpers Module
- Utility functions and helper classes
- Common functionality shared across modules

## Naming Conventions

Following C# patterns adapted for C++:

1. **Classes**: PascalCase (e.g., `MainWindow`, `ItemData`)
2. **Methods**: camelCase (e.g., `loadFile()`, `saveData()`)
3. **Variables**: camelCase (e.g., `itemCount`, `fileName`)
4. **Constants**: UPPER_SNAKE_CASE (e.g., `MAX_ITEMS`, `DEFAULT_VERSION`)
5. **Files**: lowercase with underscores (e.g., `main_window.cpp`, `item_data.h`)

## Build Configuration

The CMakeLists.txt is organized with:
- Modular source file organization
- Proper include directory setup
- Qt6 integration and linking
- Platform-specific configurations
- Installation rules

## Future Extensions

The structure supports:
- Plugin system implementation
- Unit testing framework
- Documentation generation
- Cross-platform builds
- Package management