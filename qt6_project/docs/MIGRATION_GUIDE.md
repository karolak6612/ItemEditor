# Item Editor Qt6 Migration Guide

## Overview

This document provides detailed guidance for migrating the Item Editor application from C# WinForms to Qt6, maintaining exact architectural fidelity while leveraging modern Qt6 patterns.

## Project Structure Mapping

### Directory Structure Comparison

| C# WinForms Structure | Qt6 Structure | Purpose |
|----------------------|---------------|---------|
| `Legacy_App/csharp/Source/` | `qt6_project/src/` | Main application source |
| `Controls/` | `src/Controls/` | Custom UI controls |
| `Dialogs/` | `src/Dialogs/` | Dialog windows |
| `Helpers/` | `src/Helpers/` | Utility classes |
| `Host/` | `src/Host/` | Plugin hosting services |
| `PluginInterface/` | `src/PluginInterface/` | Plugin system interface |
| `Properties/` | `src/Properties/` | Application metadata |
| `PluginOne/` | `plugins/PluginOne/` | Plugin implementation |
| `PluginTwo/` | `plugins/PluginTwo/` | Plugin implementation |
| `PluginThree/` | `plugins/PluginThree/` | Plugin implementation |

## Component Migration Details

### 1. Controls Migration

#### ClientItemView (Custom Paint Control)
**C# Original**: `Legacy_App/csharp/Source/Controls/ClientItemView.cs`
**Qt6 Target**: `src/Controls/ClientItemView.cpp/.h`

**Key Changes**:
- Inherit from `QWidget` instead of `UserControl`
- Override `paintEvent()` instead of `OnPaint()`
- Use `QPainter` instead of `Graphics`
- Use `QPixmap` instead of `Bitmap`

**Implementation Pattern**:
```cpp
class ClientItemView : public QWidget
{
    Q_OBJECT
    
public:
    explicit ClientItemView(QWidget *parent = nullptr);
    
    ClientItem* clientItem() const { return m_item; }
    void setClientItem(ClientItem* item);
    
protected:
    void paintEvent(QPaintEvent *event) override;
    
private:
    ClientItem* m_item;
    QRect m_destRect;
    QRect m_sourceRect;
};
```

#### ServerItemListBox (Custom List Control)
**C# Original**: `Legacy_App/csharp/Source/Controls/ServerItemListBox.cs`
**Qt6 Target**: `src/Controls/ServerItemListBox.cpp/.h`

**Key Changes**:
- Inherit from `QAbstractItemView` or `QListWidget`
- Implement custom item delegate for rendering
- Use Qt's Model/View architecture
- Handle selection and painting through Qt's framework

### 2. Dialogs Migration

#### Form to Dialog Conversion
**Pattern**: All C# Forms become Qt6 Dialogs

**C# Forms** → **Qt6 Dialogs**:
- `AboutForm.cs` → `AboutDialog.cpp/.h/.ui`
- `CompareOtbForm.cs` → `CompareOtbDialog.cpp/.h/.ui`
- `FindItemForm.cs` → `FindItemDialog.cpp/.h/.ui`
- etc.

**Implementation Pattern**:
```cpp
class AboutDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();
    
private slots:
    void onOkClicked();
    
private:
    Ui::AboutDialog *ui;
};
```

### 3. Plugin System Migration

#### Interface Definition
**C# Original**: `IPlugin` interface in `PluginInterface.cs`
**Qt6 Target**: `IPlugin.h` with Qt6 plugin system

**Key Changes**:
- Use `Q_DECLARE_INTERFACE` macro
- Implement Qt6 plugin loading with `QPluginLoader`
- Convert C# properties to Qt6 methods
- Use Qt6 data types (`QString`, `QList`, etc.)

**Implementation Pattern**:
```cpp
class IPlugin : public QObject
{
    Q_OBJECT
    
public:
    virtual ~IPlugin() = default;
    
    virtual ClientItems* items() const = 0;
    virtual quint16 minimumItemId() const = 0;
    virtual quint16 maximumItemId() const = 0;
    virtual QList<SupportedClient> supportedClients() const = 0;
    virtual bool isLoaded() const = 0;
    
    virtual bool loadClient(const SupportedClient& client, 
                           bool extended, bool frameDurations, 
                           bool transparency, const QString& datFullPath, 
                           const QString& sprFullPath) = 0;
    virtual void initialize() = 0;
    virtual SupportedClient getClientBySignatures(quint32 datSignature, quint32 sprSignature) = 0;
    virtual ClientItem* getClientItem(quint16 id) = 0;
};

Q_DECLARE_INTERFACE(IPlugin, "org.ottools.ItemEditor.IPlugin/1.0")
```

#### Plugin Loading
**C# Original**: `PluginServices.cs` with `Assembly.LoadFrom()`
**Qt6 Target**: `PluginServices.cpp` with `QPluginLoader`

**Key Changes**:
- Replace `Assembly.LoadFrom()` with `QPluginLoader::load()`
- Use `QPluginLoader::instance()` to get plugin instance
- Handle plugin metadata through Qt6 system
- Implement proper error handling for plugin loading

### 4. Data Type Conversions

#### Basic Types
| C# Type | Qt6 Equivalent | Usage |
|---------|----------------|-------|
| `string` | `QString` | Text handling |
| `int` | `int` or `qint32` | 32-bit integers |
| `ushort` | `quint16` | 16-bit unsigned integers |
| `uint` | `quint32` | 32-bit unsigned integers |
| `bool` | `bool` | Boolean values |
| `Rectangle` | `QRect` | Geometry |
| `Point` | `QPoint` | 2D coordinates |
| `Size` | `QSize` | Dimensions |
| `Bitmap` | `QPixmap` | Images |
| `Graphics` | `QPainter` | Drawing operations |

#### Collections
| C# Collection | Qt6 Equivalent | Usage |
|---------------|----------------|-------|
| `List<T>` | `QList<T>` | Dynamic arrays |
| `Dictionary<K,V>` | `QMap<K,V>` | Key-value pairs |
| `HashSet<T>` | `QSet<T>` | Unique elements |
| `Queue<T>` | `QQueue<T>` | FIFO queue |
| `Stack<T>` | `QStack<T>` | LIFO stack |

### 5. Event Handling Migration

#### C# Events to Qt6 Signals/Slots
**C# Pattern**:
```csharp
public event EventHandler<ItemEventArgs> ItemSelected;
protected virtual void OnItemSelected(ItemEventArgs e)
{
    ItemSelected?.Invoke(this, e);
}
```

**Qt6 Pattern**:
```cpp
signals:
    void itemSelected(const ItemEventArgs& args);

// Emit signal
emit itemSelected(args);

// Connect signal to slot
connect(source, &SourceClass::itemSelected, 
        target, &TargetClass::onItemSelected);
```

### 6. Resource Management

#### C# Resources to Qt6 Resources
**C# Original**: `Properties/Resources.resx`
**Qt6 Target**: `Properties/application.qrc`

**Migration Steps**:
1. Convert `.resx` file to `.qrc` format
2. Update resource paths to Qt6 format (`:/path/to/resource`)
3. Use `QResource` system for accessing resources
4. Handle icons, images, and other assets through Qt6 resource system

### 7. Settings Management

#### C# Settings to Qt6 Settings
**C# Original**: `Properties/Settings.settings`
**Qt6 Target**: `QSettings` usage

**Key Changes**:
- Replace `Properties.Settings.Default` with `QSettings`
- Use `QSettings::setValue()` and `QSettings::value()`
- Handle different data types through QVariant
- Implement proper settings organization

**Implementation Pattern**:
```cpp
class Settings
{
public:
    static Settings& instance();
    
    void setValue(const QString& key, const QVariant& value);
    QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;
    
private:
    Settings();
    QSettings* m_settings;
};
```

## Build System Migration

### CMake Structure
The Qt6 project uses CMake with a hierarchical structure:

1. **Root CMakeLists.txt**: Project configuration and subdirectories
2. **src/CMakeLists.txt**: Main application and component libraries
3. **Component CMakeLists.txt**: Individual library configurations
4. **plugins/CMakeLists.txt**: Plugin build configuration

### Build Process
```bash
# Configure
cmake -B build -S qt6_project -DCMAKE_PREFIX_PATH=/path/to/qt6

# Build
cmake --build build

# Install
cmake --install build
```

## Testing Strategy

### Unit Tests
- Create tests for each migrated component
- Use Qt Test framework
- Maintain same test coverage as original
- Test plugin loading and functionality

### Integration Tests
- Test component interactions
- Verify plugin system functionality
- Test UI components and dialogs
- Cross-platform compatibility tests

## Deployment Considerations

### Cross-Platform Support
- Windows: Primary target (matching original)
- Linux: Secondary target
- macOS: Future consideration

### Plugin Deployment
- Plugins built as shared libraries
- Proper plugin discovery mechanism
- Version compatibility checking
- Error handling for missing plugins

## Migration Checklist

### Phase 1: Foundation
- [ ] Set up Qt6 project structure
- [ ] Create CMake build system
- [ ] Implement basic data types
- [ ] Set up plugin interface

### Phase 2: Core Components
- [ ] Migrate PluginInterface library
- [ ] Migrate Helpers library
- [ ] Migrate Host library
- [ ] Implement basic plugin loading

### Phase 3: UI Components
- [ ] Migrate Controls library
- [ ] Migrate Dialogs library
- [ ] Implement main window
- [ ] Test UI functionality

### Phase 4: Plugins
- [ ] Migrate PluginOne
- [ ] Migrate PluginTwo
- [ ] Migrate PluginThree
- [ ] Test plugin functionality

### Phase 5: Integration
- [ ] Integration testing
- [ ] Cross-platform testing
- [ ] Performance optimization
- [ ] Documentation completion

## Common Pitfalls and Solutions

### 1. Memory Management
**Issue**: C# automatic garbage collection vs C++ manual memory management
**Solution**: Use Qt6's parent-child ownership model and smart pointers

### 2. String Handling
**Issue**: C# string vs Qt6 QString differences
**Solution**: Use QString consistently, convert when interfacing with C++ standard library

### 3. Plugin Loading
**Issue**: Different plugin systems between C# and Qt6
**Solution**: Implement proper Qt6 plugin interface and loading mechanism

### 4. UI Layout
**Issue**: WinForms layout vs Qt6 layout differences
**Solution**: Use Qt6 layout managers and size policies properly

### 5. Event Handling
**Issue**: C# events vs Qt6 signals/slots
**Solution**: Convert all events to proper signal/slot connections

## Performance Considerations

### Optimization Areas
1. **Plugin Loading**: Lazy loading and caching
2. **UI Rendering**: Efficient paint operations
3. **Memory Usage**: Proper resource management
4. **File I/O**: Asynchronous operations where appropriate

### Profiling Tools
- Qt Creator's profiler
- Valgrind (Linux)
- Application Verifier (Windows)
- Instruments (macOS)

## Conclusion

This migration maintains complete architectural fidelity while modernizing the codebase with Qt6. The modular approach ensures maintainability and allows for incremental migration and testing.

## Current Implementation Status (Updated)

### ✅ Fully Implemented Components
- **Helper Classes**: PathHelper, FileNameHelper, Utils, MemoryManager
- **Plugin Interface**: Complete Qt6 interface definition with Q_DECLARE_INTERFACE
- **Plugin Host System**: QPluginLoader-based architecture with PluginServices
- **Core UI Controls**: ClientItemView with full sprite rendering and caching
- **Dialog Forms**: Complete UI structure and basic functionality
- **Performance Optimizations**: Memory management and UI rendering optimizations
- **Build System**: CMake configuration with cross-platform support

### 🔄 Partially Implemented (Has Placeholders)
- **ServerItemListBox**: 
  - ✅ Complete UI structure and performance optimizations
  - ❌ Uses placeholder rendering (shows item ID instead of sprite)
  - **Location**: `src/Controls/ServerItemListBox.cpp:239-244`
  - **Impact**: Users cannot visually identify items by sprite
  
- **Plugin System**: 
  - ✅ Complete architecture and loading mechanism
  - ❌ Client detection returns placeholder implementations
  - **Location**: `plugins/*/Plugin.cpp:152` (all three plugins)
  - **Impact**: Cannot detect client versions from .spr/.dat files
  
- **NewOtbFileForm**: 
  - ✅ Complete dialog structure and validation
  - ❌ Uses hardcoded placeholder client data
  - **Location**: `src/Dialogs/NewOtbFileForm.cpp:86-88`
  - **Impact**: Limited to single client version for file creation

### ❌ Critical Missing Implementations
1. **Sprite Loading System**: Complete .spr file parsing and sprite extraction
2. **DAT File Parsing**: Full .dat file structure parsing for item definitions  
3. **Plugin-UI Integration**: Connect loaded plugin data to UI components
4. **File Format Validation**: Proper .otb/.spr/.dat file format verification

## Production Readiness Assessment

### Blocking Issues for Production Use
1. **Visual Item Identification**: ServerItemListBox placeholder prevents item recognition
2. **File Format Support**: Incomplete plugin file parsing limits functionality
3. **Client Version Detection**: Cannot automatically detect client versions

### Ready for Development Use
- Complete build system and project structure
- Full plugin architecture for extensibility
- Performance-optimized UI components
- Comprehensive memory management
- Cross-platform compatibility

For specific implementation details, refer to the individual component documentation and example code in the project structure.