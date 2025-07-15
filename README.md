# Item Editor Qt6 - Complete Migration Project for Jules Coding Assistant

## Project Mission Statement

**Jules Coding Assistant**: You are tasked with completing the migration of the Item Editor application from C# WinForms to Qt6 C++17. This is a specialized game development tool for editing Open Tibia Binary (OTB) item databases. Your goal is to achieve **100% functional parity** with the original C# application while leveraging modern Qt6 patterns.

## Critical Success Criteria

### 1. Functional Parity (MANDATORY)
- **Every feature** from the C# version must work identically in Qt6
- **Every dialog, menu, button** must behave exactly the same
- **All file operations** (.otb, .dat, .spr, .xml) must produce identical results
- **Plugin system** must support all three client version plugins
- **UI layout and behavior** must match the original precisely

### 2. Production Quality (MANDATORY)
- **Zero placeholders** - No "TODO", "Not implemented", or stub functions
- **Zero crashes** - Robust error handling for all operations
- **Professional code** - Clean, maintainable, well-documented C++17/Qt6
- **Complete implementation** - Every method must be fully functional

### 3. Technical Requirements (MANDATORY)
- **C++17 standard** with modern best practices
- **Qt6 framework** (6.5+) using Widgets, Core, Gui, Network, Xml
- **CMake build system** with proper cross-platform support
- **Exact architectural mirroring** of C# codebase structure

## Project Structure Overview

```
Item Editor Migration Project/
├── Legacy_App/csharp/Source/          # C# REFERENCE IMPLEMENTATION
│   ├── MainForm.cs                    # → qt6_project/src/MainForm.cpp/.h/.ui
│   ├── Controls/                      # → qt6_project/src/Controls/
│   │   ├── ClientItemView.cs         # → ClientItemView.cpp/.h
│   │   ├── ServerItemListBox.cs      # → ServerItemListBox.cpp/.h
│   │   ├── FlagCheckBox.cs           # → FlagCheckBox.cpp/.h
│   │   └── ListBase.cs               # → ListBase.cpp/.h
│   ├── Dialogs/                      # → qt6_project/src/Dialogs/
│   │   ├── AboutForm.cs              # → AboutDialog.cpp/.h/.ui
│   │   ├── CompareOtbForm.cs         # → CompareOtbDialog.cpp/.h/.ui
│   │   ├── FindItemForm.cs           # → FindItemDialog.cpp/.h/.ui
│   │   ├── NewOtbFileForm.cs         # → NewOtbFileDialog.cpp/.h/.ui
│   │   ├── PreferencesForm.cs        # → PreferencesDialog.cpp/.h/.ui
│   │   ├── ProgressForm.cs           # → ProgressDialog.cpp/.h/.ui
│   │   ├── UpdateForm.cs             # → UpdateDialog.cpp/.h/.ui
│   │   └── UpdateSettingsForm.cs     # → UpdateSettingsDialog.cpp/.h/.ui
│   ├── Helpers/                      # → qt6_project/src/Helpers/
│   │   ├── FileNameHelper.cs         # → FileNameHelper.cpp/.h
│   │   ├── PathHelper.cs             # → PathHelper.cpp/.h
│   │   └── Utils.cs                  # → Utils.cpp/.h
│   ├── Host/                         # → qt6_project/src/Host/
│   │   ├── Plugin.cs                 # → Plugin.cpp/.h
│   │   ├── PluginCollection.cs       # → PluginCollection.cpp/.h
│   │   └── PluginServices.cs         # → PluginServices.cpp/.h
│   ├── PluginInterface/              # → qt6_project/src/PluginInterface/
│   │   ├── PluginInterface.cs        # → IPlugin.h
│   │   ├── Item.cs                   # → Item.cpp/.h
│   │   ├── Sprite.cs                 # → Sprite.cpp/.h
│   │   ├── Settings.cs               # → Settings.cpp/.h
│   │   └── OTLib/                    # → OTLib/ (complete migration)
│   ├── PluginOne/Plugin.cs           # → qt6_project/plugins/PluginOne/
│   ├── PluginTwo/Plugin.cs           # → qt6_project/plugins/PluginTwo/
│   ├── PluginThree/Plugin.cs         # → qt6_project/plugins/PluginThree/
│   └── Properties/                   # → qt6_project/src/Properties/
└── qt6_project/                      # QT6 TARGET IMPLEMENTATION
    ├── src/                          # Main application source
    ├── plugins/                      # Plugin implementations
    ├── docs/                         # Migration documentation
    └── CMakeLists.txt               # Build configuration
```

## Your Migration Tasks (Jules)

### Phase 1: Foundation Analysis (CRITICAL)
1. **Study the C# Reference**: Thoroughly analyze every file in `Legacy_App/csharp/Source/`
2. **Understand the Architecture**: Map out class relationships, data flow, and plugin interactions
3. **Identify Core Components**: MainForm, custom controls, dialogs, plugin system, file handlers
4. **Document Dependencies**: Note all external libraries, file formats, and system interactions

### Phase 2: Core Framework Implementation
1. **MainForm Migration**: Convert `MainForm.cs` to Qt6 QMainWindow with identical layout and functionality
2. **Menu System**: Implement complete menu structure with all shortcuts and event handlers
3. **Plugin Architecture**: Convert C# plugin system to Qt6 QPluginLoader-based system
4. **Settings Management**: Replace .NET Settings with QSettings implementation

### Phase 3: Custom Controls Migration
1. **ServerItemListBox**: High-performance list widget with sprite thumbnails and virtual scrolling
2. **ClientItemView**: Custom paint widget for sprite display with automatic centering
3. **FlagCheckBox**: Specialized checkbox for ServerItemFlag enumeration binding
4. **ListBase**: Base class for custom list controls with common functionality

### Phase 4: Dialog System Implementation
1. **AboutDialog**: Application information with version details and links
2. **CompareOtbDialog**: File comparison tool with detailed difference reporting
3. **FindItemDialog**: Advanced search with multiple criteria and embedded results
4. **NewOtbFileDialog**: OTB file creation wizard with version selection
5. **PreferencesDialog**: Client configuration with file validation
6. **ProgressDialog**: Operation progress with cancellation support
7. **UpdateDialog**: OTB version migration tool
8. **UpdateSettingsDialog**: Update configuration options

### Phase 5: File Format Implementation
1. **OTB Files**: Complete Open Tibia Binary format support (read/write)
2. **DAT Files**: Client data file parsing with all item properties
3. **SPR Files**: Sprite file handling with transparency and animation support
4. **XML Files**: Configuration and data exchange format support

### Phase 6: Plugin System Completion
1. **PluginOne**: Client version 8.00-10.77 support with complete DAT/SPR parsing
2. **PluginTwo**: Extended format support for newer clients
3. **PluginThree**: Advanced features and optimization for latest versions
4. **Plugin Loading**: Robust discovery, validation, and error handling

### Phase 7: Integration and Testing
1. **End-to-End Testing**: Verify all user workflows work identically to C# version
2. **File Format Validation**: Test with real OTB/DAT/SPR files from various client versions
3. **Plugin Compatibility**: Ensure all plugins load and function correctly
4. **Performance Optimization**: Optimize for large item databases and sprite rendering

## Critical Implementation Guidelines

### Code Quality Standards
```cpp
// Example: Proper Qt6 class structure
class MainForm : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainForm(QWidget *parent = nullptr);
    ~MainForm();

    // Public interface matching C# version
    bool openFile(const QString &filePath);
    bool saveFile(const QString &filePath = QString());
    void selectItem(ServerItem *item);

private slots:
    // Event handlers for all UI interactions
    void onFileOpen();
    void onFileSave();
    void onEditCreateItem();
    void onItemSelectionChanged();

private:
    // Private implementation
    Ui::MainForm *ui;
    PluginServices *m_pluginServices;
    ServerItemList *m_serverItems;
    IPlugin *m_currentPlugin;
    
    // Helper methods
    void setupMenus();
    void setupConnections();
    void updateUI();
    void buildItemsList();
};
```

### Data Type Conversions (MANDATORY)
| C# Type | Qt6 Equivalent | Usage Notes |
|---------|----------------|-------------|
| `string` | `QString` | Use QString::fromStdString() for std::string conversion |
| `ushort` | `quint16` | Exact match for item IDs |
| `uint` | `quint32` | File signatures and large values |
| `Rectangle` | `QRect` | UI geometry and sprite bounds |
| `Bitmap` | `QPixmap` | Sprite and image handling |
| `Graphics` | `QPainter` | Custom drawing operations |
| `List<T>` | `QList<T>` | Dynamic collections |
| `Dictionary<K,V>` | `QMap<K,V>` | Key-value storage |

### Plugin Interface Migration (CRITICAL)
```cpp
// Convert C# IPlugin to Qt6 interface
class IPlugin
{
public:
    virtual ~IPlugin() = default;
    
    // Properties (convert C# properties to getter/setter methods)
    virtual IPluginHost* host() const = 0;
    virtual void setHost(IPluginHost* host) = 0;
    virtual ClientItems* items() const = 0;
    virtual quint16 minItemId() const = 0;
    virtual quint16 maxItemId() const = 0;
    virtual QList<SupportedClient> supportedClients() const = 0;
    virtual bool loaded() const = 0;
    
    // Methods (exact functional equivalents)
    virtual bool loadClient(const SupportedClient& client, 
                           bool extended, bool frameDurations, bool transparency,
                           const QString& datFullPath, const QString& sprFullPath) = 0;
    virtual void initialize() = 0;
    virtual SupportedClient getClientBySignatures(quint32 datSignature, quint32 sprSignature) = 0;
    virtual ClientItem* getClientItem(quint16 id) = 0;
};

Q_DECLARE_INTERFACE(IPlugin, "org.ottools.ItemEditor.IPlugin/1.0")
```

### UI Layout Fidelity (MANDATORY)
- **Exact pixel positioning**: Match C# Designer coordinates precisely
- **Control sizing**: Maintain identical dimensions and proportions
- **Color schemes**: Implement dark theme matching DarkUI library
- **Font rendering**: Use system fonts with proper sizing
- **Icon resources**: Convert all .resx resources to Qt6 .qrc format

### Event Handling Migration (CRITICAL)
```cpp
// Convert C# events to Qt6 signals/slots
// C# Pattern:
// public event EventHandler<ItemEventArgs> ItemSelected;

// Qt6 Pattern:
signals:
    void itemSelected(const ItemEventArgs& args);

// Connection:
connect(serverItemListBox, &ServerItemListBox::itemSelected,
        this, &MainForm::onItemSelected);
```

### File I/O Implementation (MANDATORY)
- **Binary file handling**: Use QDataStream for .otb/.dat/.spr files
- **Endianness**: Handle little-endian format correctly
- **Error handling**: Robust exception handling for corrupted files
- **Progress reporting**: Implement progress callbacks for large operations
- **Memory management**: Efficient handling of large sprite datasets

### Build System Requirements
```cmake
# CMakeLists.txt structure for each component
qt_add_library(ItemEditorControls STATIC
    ClientItemView.cpp ClientItemView.h
    ServerItemListBox.cpp ServerItemListBox.h
    FlagCheckBox.cpp FlagCheckBox.h
    ListBase.cpp ListBase.h
)

target_link_libraries(ItemEditorControls PRIVATE
    Qt6::Core Qt6::Widgets Qt6::Gui
    ItemEditorPluginInterface
)

# Plugin build configuration
qt_add_plugin(PluginOne
    Plugin.cpp Plugin.h
    PluginOne.json
)

target_link_libraries(PluginOne PRIVATE
    Qt6::Core
    ItemEditorPluginInterface
)
```

## Testing and Validation Requirements

### Functional Testing Checklist
- [ ] **File Operations**: Open, save, create new OTB files
- [ ] **Item Editing**: All property modifications work correctly
- [ ] **Search Functionality**: Find items by ID and properties
- [ ] **Plugin Loading**: All three plugins load and function
- [ ] **Sprite Display**: Correct rendering in all contexts
- [ ] **Menu Operations**: All menu items and shortcuts work
- [ ] **Dialog Interactions**: All dialogs open, function, and close properly
- [ ] **Error Handling**: Graceful handling of invalid files and operations

### Performance Requirements
- **Startup Time**: < 3 seconds on modern hardware
- **File Loading**: Large OTB files (10,000+ items) load within 10 seconds
- **UI Responsiveness**: No blocking operations on main thread
- **Memory Usage**: Efficient sprite caching and cleanup
- **Plugin Loading**: Fast discovery and initialization

### Compatibility Requirements
- **Windows 10/11**: Primary target platform
- **Linux**: Ubuntu 20.04+ with Qt6 packages
- **File Formats**: Support client versions 8.00 through 10.77
- **Plugins**: Backward compatibility with existing plugin architecture

## Success Metrics

### Completion Criteria
1. **100% Feature Parity**: Every C# feature works in Qt6 version
2. **Zero Regressions**: No functionality lost in migration
3. **Production Ready**: No placeholders, TODOs, or incomplete implementations
4. **Professional Quality**: Clean, maintainable, well-documented code
5. **Cross-Platform**: Builds and runs on Windows and Linux
6. **Performance**: Meets or exceeds C# version performance

### Deliverables
1. **Complete Qt6 Application**: Fully functional Item Editor
2. **Plugin System**: All three plugins working correctly
3. **Build System**: CMake configuration for all platforms
4. **Documentation**: Updated guides and API documentation
5. **Test Suite**: Comprehensive testing coverage
6. **Installation Package**: Ready-to-deploy application

## Getting Started (Jules)

### Step 1: Environment Setup
1. Examine the complete C# codebase in `Legacy_App/csharp/Source/`
2. Review existing Qt6 implementation in `qt6_project/src/`
3. Study the migration documentation in `qt6_project/docs/`
4. Understand the build system and dependencies

### Step 2: Analysis Phase
1. Create detailed component mapping between C# and Qt6
2. Identify missing implementations and incomplete code
3. Plan the migration sequence and dependencies
4. Document architectural decisions and patterns

### Step 3: Implementation Phase
1. Start with core framework (MainForm, plugin system)
2. Implement custom controls with exact functionality
3. Migrate all dialogs with complete feature sets
4. Complete file format handling and plugin implementations

### Step 4: Integration Phase
1. Connect all components and test interactions
2. Implement comprehensive error handling
3. Optimize performance and memory usage
4. Validate against C# reference implementation

**Remember**: This is a migration project, not new development. The C# version is your complete specification. Make the Qt6 version work exactly the same way, with no missing features and no compromises on functionality.

---

**Jules**: Begin your migration work by thoroughly studying the C# reference implementation, then systematically convert each component to Qt6 while maintaining perfect functional parity. Your success is measured by how closely the Qt6 version matches the original C# application's behavior and capabilities.