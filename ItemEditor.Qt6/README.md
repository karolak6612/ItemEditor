# ItemEditor Qt6

A modern Qt6-based migration of the legacy ItemEditor application for editing OTB (Open Tibia Binary) data files.

## Project Structure

```
ItemEditor.Qt6/
├── src/
│   ├── ItemEditor.Core/        # Business logic and data models
│   ├── ItemEditor.UI/          # Qt6 UI application
│   ├── ItemEditor.Plugins/     # Plugin interfaces and base classes
│   └── ItemEditor.Tests/       # Test projects
├── plugins/
│   ├── PluginOne/             # Client versions 8.00-8.57
│   ├── PluginTwo/             # Client versions 8.60-9.86
│   └── PluginThree/           # Client versions 10.00-10.77
└── assets/
    ├── icons/                 # Application icons
    ├── themes/                # UI themes and styles
    └── resources/             # Embedded resources
```

## Build Requirements

- Qt6 (6.5+ LTS recommended)
- CMake 3.16+
- C++17 compatible compiler
- Windows SDK (for Windows builds)

## Building

1. Create build directory:
   ```bash
   mkdir build
   cd build
   ```

2. Configure with CMake:
   ```bash
   cmake .. -DCMAKE_PREFIX_PATH=path/to/qt6
   ```

3. Build:
   ```bash
   cmake --build . --config Release
   ```

## Features

- 100% functional parity with legacy Windows Forms application
- Native Qt6 plugin system with rewritten plugins
- Dark UI theme matching legacy system
- Cross-platform compatibility (Windows primary target)
- Modern C++ implementation for improved performance

## Status

This is the initial project structure setup. Individual components will be implemented in subsequent development phases.