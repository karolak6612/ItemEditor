# Item Editor Qt6 - Complete Migration Project

## Project Overview

This is the Qt6 migration of the Item Editor application, maintaining **exact architectural fidelity** to the original C# WinForms codebase while leveraging modern Qt6 patterns.

### Key Information
- **Language**: C++17 with Qt6 framework
- **Original Source**: C# WinForms application in `../Legacy_App/csharp/`
- **Target Platform**: Cross-platform Qt6 application
- **Build System**: CMake with proper subdirectory organization
- **License**: MIT (matching original)

## Project Structure

```
qt6_project/
├── CMakeLists.txt              # Root CMake configuration
├── src/                        # Main application source
│   ├── CMakeLists.txt         # Source build configuration
│   ├── main.cpp               # Application entry point (Program.cs equivalent)
│   ├── MainForm.cpp/.h/.ui    # Main window (MainForm.cs equivalent)
│   ├── Controls/              # Custom Qt widgets (exact mirror of C# Controls/)
│   │   ├── CMakeLists.txt
│   │   ├── ClientItemView.cpp/.h      # Custom paint widget
│   │   ├── ServerItemListBox.cpp/.h   # Custom list widget
│   │   ├── FlagCheckBox.cpp/.h        # Custom checkbox
│   │   └── ListBase.cpp/.h            # Base list class
│   ├── Dialogs/               # Dialog windows (exact mirror of C# Dialogs/)
│   │   ├── CMakeLists.txt
│   │   ├── AboutForm.cpp/.h/.ui
│   │   ├── CompareOtbForm.cpp/.h/.ui
│   │   ├── FindItemForm.cpp/.h/.ui
│   │   ├── NewOtbFileForm.cpp/.h/.ui
│   │   ├── PreferencesForm.cpp/.h/.ui
│   │   ├── ProgressForm.cpp/.h/.ui
│   │   ├── UpdateForm.cpp/.h/.ui
│   │   └── UpdateSettingsForm.cpp/.h/.ui
│   ├── Helpers/               # Utility classes (exact mirror of C# Helpers/)
│   │   ├── CMakeLists.txt
│   │   ├── FileNameHelper.cpp/.h
│   │   ├── PathHelper.cpp/.h
│   │   └── Utils.cpp/.h
│   ├── Host/                  # Plugin hosting (exact mirror of C# Host/)
│   │   ├── CMakeLists.txt
│   │   ├── PluginServices.cpp/.h      # Plugin discovery and loading
│   │   ├── Plugin.cpp/.h              # Plugin wrapper class
│   │   └── PluginCollection.cpp/.h    # Plugin container
│   ├── PluginInterface/       # Plugin system (exact mirror of C# PluginInterface/)
│   │   ├── CMakeLists.txt
│   │   ├── IPlugin.h                  # Main plugin interface
│   │   ├── Item.cpp/.h                # Item data structures
│   │   ├── Sprite.cpp/.h              # Sprite handling
│   │   ├── Settings.cpp/.h            # Plugin settings
│   │   └── OTLib/                     # OTB/DAT/SPR file handling
│   │       ├── CMakeLists.txt
│   │       ├── Collections/
│   │       ├── OTB/
│   │       ├── Server/
│   │       └── Utils/
│   └── Properties/            # Application metadata (C# Properties/ equivalent)
│       ├── CMakeLists.txt
│       ├── application.qrc           # Resources (Resources.resx equivalent)
│       ├── version.h                 # Version info (AssemblyInfo.cs equivalent)
│       └── settings.h                # App settings (Settings.settings equivalent)
├── plugins/                   # Plugin implementations (exact mirror of C# plugins)
│   ├── CMakeLists.txt
│   ├── PluginOne/
│   │   ├── CMakeLists.txt
│   │   └── Plugin.cpp/.h
│   ├── PluginTwo/
│   │   ├── CMakeLists.txt
│   │   └── Plugin.cpp/.h
│   └── PluginThree/
│       ├── CMakeLists.txt
│       └── Plugin.cpp/.h
└── docs/                      # Documentation
    ├── MIGRATION_GUIDE.md     # Detailed migration documentation
    ├── BUILD_INSTRUCTIONS.md  # Build and setup guide
    └── COMPONENT_MAPPING.md   # C# to Qt6 component mappings
```

## Quick Start

### Prerequisites
- Qt6 (6.5+) with Widgets, Core, Gui, Network, Xml modules
- CMake (3.16+)
- C++17 compatible compiler
- Git for version control

### Build Instructions
```bash
# Configure the build
cmake -B build -S qt6_project -DCMAKE_PREFIX_PATH=/path/to/qt6

# Build the project
cmake --build build

# Run the application
./build/src/ItemEditor
```

## Architecture Principles

### 1. Exact Structural Mirroring
- Every C# directory has a corresponding Qt6 directory
- Every C# class has a corresponding Qt6 class with same name
- Same architectural patterns and responsibilities

### 2. Modern Qt6 Patterns
- CMake build system with proper subdirectory organization
- Qt6 plugin system using `Q_DECLARE_INTERFACE`
- Proper Qt6 widget inheritance and signal/slot connections
- Qt6 resource system for assets and translations

### 3. Cross-Platform Compatibility
- Windows (primary target, matching original)
- Linux (secondary target)
- macOS (future consideration)

## Component Migration Strategy

### UI Framework Migration
| C# WinForms | Qt6 Equivalent | Implementation |
|-------------|----------------|----------------|
| `UserControl` | `QWidget` | Custom widget base class |
| `Form` | `QDialog`/`QMainWindow` | Dialog or main window |
| `ListBox` | `QListWidget` | Standard list widget |
| `CheckBox` | `QCheckBox` | Standard checkbox |
| `Graphics.DrawImage()` | `QPainter::drawPixmap()` | Custom painting in `paintEvent()` |
| `Graphics.DrawString()` | `QPainter::drawText()` | Text rendering |

### Plugin System Migration
| C# Plugin System | Qt6 Equivalent | Implementation |
|------------------|----------------|----------------|
| `IPlugin` interface | `Q_DECLARE_INTERFACE` | Qt6 plugin interface |
| `Assembly.LoadFrom()` | `QPluginLoader` | Dynamic plugin loading |
| `PluginServices` | `PluginServices` | Same class, Qt6 implementation |
| `.dll` plugins | `.so`/`.dylib`/`.dll` | Platform-specific libraries |

### Data Types Migration
| C# Type | Qt6 Equivalent | Notes |
|---------|----------------|-------|
| `Rectangle` | `QRect` | Geometry handling |
| `Bitmap` | `QPixmap` | Image handling |
| `Graphics` | `QPainter` | Drawing operations |
| `List<T>` | `QList<T>` | Container classes |
| `Dictionary<K,V>` | `QMap<K,V>` | Key-value containers |
| `string` | `QString` | String handling |

## Development Guidelines

### Naming Conventions
- **Classes**: PascalCase (e.g., `ClientItemView`, `ServerItemListBox`)
- **Methods**: camelCase (e.g., `getClientItem()`, `loadPlugin()`)
- **Variables**: camelCase (e.g., `clientItem`, `pluginPath`)
- **Files**: Match class names (e.g., `ClientItemView.cpp`, `ClientItemView.h`)

### Code Organization
- Each directory has its own `CMakeLists.txt`
- Header files (`.h`) contain class declarations
- Source files (`.cpp`) contain implementations
- UI files (`.ui`) for complex dialog layouts
- Maintain same class responsibilities as C# version

### Plugin Development
- Inherit from `IPlugin` interface
- Implement all required virtual methods
- Use `Q_OBJECT` macro for Qt meta-object system
- Export plugin using `Q_PLUGIN_METADATA`

## Documentation
- Complete API documentation using Doxygen
- Migration guides for each component
- Build instructions for all platforms
- Plugin development tutorials

---


For detailed implementation guidance, see the documentation in the `docs/` directory.
