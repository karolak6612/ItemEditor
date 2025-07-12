# ItemEditor Qt6 - Developer Guide

## Project Overview

ItemEditor Qt6 is a C++ application for editing Tibia OTB (Open Tibia Binary) files and managing game item data. This is a Qt6 port of the original C# WinForms ItemEditor application, designed to provide cross-platform compatibility and modern UI capabilities.

### Key Technologies
- **Language**: C++17
- **Framework**: Qt6 (Core, Widgets, Gui, Test)
- **Build System**: CMake 3.16+
- **Architecture**: Modular design with clear separation of concerns
- **Testing**: Qt Test framework with comprehensive test suite

## Project Structure

### Core Directories
- **`src/`** - Source code implementations
- **`include/`** - Header files and public interfaces
- **`resources/`** - Application resources (stylesheets, icons, QRC files)
- **`docs/`** - Documentation files
- **`tests/`** - Unit tests and test data
- **`plugins/`** - Plugin implementations (currently being refactored)
- **`plugins_backup/`** - Backup of removed plugin system

### Module Organization

#### Core Module (`src/core/`, `include/core/`)
- Application framework and lifecycle management
- Settings management and configuration
- Resource management and stylesheet handling
- Event system and command management
- Base classes and interfaces

#### UI Module (`src/ui/`, `include/ui/`)
- Main window and application interface
- Dialog windows (About, Find Item, Preferences, Compare OTB, Update OTB, Sprite Candidates)
- Custom widgets (ClientItemView)
- Qt-based user interface components

#### OTB Module (`src/otb/`, `include/otb/`)
- Binary tree data structure for OTB files
- OTB file reading and writing functionality
- Item data structures and management
- Comprehensive error handling and validation
- Performance optimization and caching
- Backup and recovery systems

#### TibiaData Module (`src/tibiadata/`, `include/tibiadata/`)
- DAT file parsing for client data
- SPR file parsing for sprite data
- Image similarity algorithms

#### Plugin System (`src/plugins/`, `include/plugins/`)
- **Status**: Currently being refactored (see Plugin_Removal_Documentation.md)
- Plugin interface definitions
- Plugin host and management system
- Dynamic loading and version compatibility
- Security and validation

## Coding Standards

### Naming Conventions
- **Classes**: PascalCase (e.g., `MainWindow`, `ServerItem`)
- **Methods/Functions**: camelCase (e.g., `loadFile()`, `saveData()`)
- **Variables**: camelCase (e.g., `itemCount`, `fileName`)
- **Member Variables**: m_camelCase (e.g., `m_itemList`, `m_currentFile`)
- **Constants**: UPPER_SNAKE_CASE (e.g., `MAX_ITEMS`, `DEFAULT_VERSION`)
- **Enums**: PascalCase for enum name and values (e.g., `ServerItemType::Ground`)
- **Files**: lowercase with underscores for headers, PascalCase for classes

### Namespace Organization
- **`OTB`** - OTB file handling and data structures
- **`Core`** - Core application functionality
- **`ItemEditor::Core`** - Application-specific core classes
- **`PluginInterface`** - Plugin system interfaces

### Code Organization
- Use forward declarations to minimize includes
- Header order: Related header, C++ standard library, Qt headers, Project headers
- Include guards: `#pragma once` preferred
- Comprehensive error handling with custom exception hierarchy
- Qt-style memory management with parent-child relationships

## Key Features

### OTB File Handling
- **Reading**: Comprehensive OTB file parsing with validation
- **Writing**: Safe OTB file writing with backup systems
- **Validation**: Multi-level validation and integrity checking
- **Performance**: Optimized reading/writing with caching and buffering
- **Error Recovery**: Robust error handling with recovery strategies

### Item Management
- **ServerItem Structure**: Complete item data representation
- **Flags System**: Comprehensive item flag management
- **Attributes**: Full support for all OTB item attributes
- **Validation**: Item consistency checking and validation

### Error Handling System
- **Comprehensive Error Classification**: Organized error codes (1000-1799)
- **Exception Hierarchy**: C#-compatible exception patterns
- **Recovery Strategies**: Automatic recovery, user intervention, abort
- **Centralized Management**: ErrorHandler singleton for error collection

### Plugin Architecture
- **Current Status**: Being refactored (broken system removed)
- **Target**: C#-compatible plugin interface
- **Features**: Dynamic loading, version compatibility, security validation

## Build Configuration

### Prerequisites
- Qt6 (6.0 or later) with Core, Widgets, Gui, Test components
- CMake 3.16 or later
- C++17 compatible compiler

### Build Commands
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### CMake Structure
- **Root CMakeLists.txt**: Main project configuration
- **Modular Organization**: Separate source file groups
- **Qt Integration**: Automatic MOC, RCC, UIC processing
- **Platform Support**: Windows, macOS, Linux

## Testing Framework

### Test Organization
- **Unit Tests**: Individual component testing
- **Integration Tests**: Cross-component testing
- **Performance Tests**: Optimization and benchmarking
- **Plugin Tests**: Plugin system validation

### Test Categories
- **OTB Tests**: Comprehensive OTB file handling tests
- **Core Tests**: Application framework tests
- **UI Tests**: User interface component tests
- **Plugin Tests**: Plugin system tests (when reimplemented)

### Running Tests
```bash
cd build
ctest
```

## Development Guidelines

### Memory Management
- Use Qt's parent-child ownership model for QObjects
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) for non-QObject classes
- Prefer Qt containers (`QList`, `QMap`) for Qt integration

### Qt6 Best Practices
- Use new signal/slot syntax: `connect(sender, &Class::signal, receiver, &Class::slot)`
- Leverage Qt resource system for assets: `:/path/to/resource`
- Use `QSettings` for configuration persistence
- Prefer `QXmlStreamReader` for XML processing

### Error Handling
- Use the comprehensive OTB error handling system
- Implement proper exception handling with custom exception types
- Provide meaningful error messages and recovery options
- Log errors appropriately for debugging

### Performance Considerations
- Use caching systems for frequently accessed data
- Implement buffered I/O for large file operations
- Monitor performance with built-in metrics
- Optimize memory usage for large datasets


## Important Files

### Configuration
- **CMakeLists.txt** - Main build configuration
- **resources/resources.qrc** - Qt resource definitions
- **resources/dark.qss** - Application stylesheet

### Documentation
- **docs/PROJECT_STRUCTURE.md** - Detailed project structure
- **docs/OTB_Error_Handling_System.md** - Error handling documentation
- **Plugin_Removal_Documentation.md** - Plugin system refactoring notes

### Core Headers
- **include/core/application.h** - Main application class
- **include/otb/otbtypes.h** - OTB data structures
- **include/otb/otbreader.h** - OTB file reading
- **include/core/interfaces.h** - Core interfaces

## Migration Notes

This project is a C# to Qt6 migration. Key considerations:

### C# to Qt6 Mappings
- **String** → **QString**
- **List<T>** → **QList<T>**
- **Dictionary<K,V>** → **QMap<K,V>**
- **IDisposable** → **Core::IDisposable**
- **Exception hierarchy** → **OTB exception system**

### Preserved Patterns
- Plugin interface structure
- OTB handling logic
- Item data structures and enums
- Error handling patterns

## Troubleshooting

### Common Issues
- **Qt6 not found**: Ensure Qt6 is properly installed and CMAKE_PREFIX_PATH is set
- **MOC errors**: Check Q_OBJECT macro placement and include statements
- **Plugin loading**: Currently disabled during refactoring
- **Resource loading**: Verify QRC file compilation and resource paths

### Debug Mode
- Use `-DCMAKE_BUILD_TYPE=Debug` for debug builds
- Enable detailed logging in OTB components
- Use Qt Creator for integrated debugging

## Contributing

1. Follow established project structure and naming conventions
2. Maintain C++17 compatibility
3. Use Qt6 coding conventions and best practices
4. Add comprehensive tests for new functionality
5. Update documentation for significant changes
6. Respect the modular architecture design

## Resources

- **Qt6 Documentation**: https://doc.qt.io/qt-6/
- **CMake Documentation**: https://cmake.org/documentation/
- **Project Structure**: docs/PROJECT_STRUCTURE.md
- **Error Handling**: docs/OTB_Error_Handling_System.md