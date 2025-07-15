# Jules Coding Assistant - Complete Qt6 Migration Instructions

## Mission Statement

**Jules**: You are tasked with completing the migration of the Item Editor application from C# WinForms to Qt6 C++17. This is a specialized game development tool for editing Open Tibia Binary (OTB) item databases. Your objective is to achieve **100% functional parity** with the original C# application while leveraging modern Qt6 patterns and best practices.

## Project Overview

### What You're Migrating
- **Application Type**: Professional game development tool
- **Original Framework**: C# WinForms with .NET Framework
- **Target Framework**: Qt6 C++17 with cross-platform support
- **User Base**: Game developers working with Open Tibia-style MMORPG systems
- **File Formats**: OTB (server), DAT/SPR (client), XML (configuration)
- **Supported Versions**: Game client versions 8.00 through 10.77

### Critical Success Criteria
1. **Perfect Functional Parity**: Every feature works identically to C# version
2. **Zero Regressions**: No functionality loss or behavioral changes
3. **Production Quality**: Professional-grade code with comprehensive error handling
4. **Performance Standards**: Meet or exceed C# version performance
5. **Cross-Platform Support**: Windows (primary) and Linux compatibility

## Architectural Overview

### Source Code Structure Mapping
```
C# Reference Implementation          Qt6 Target Implementation
├── Legacy_App/csharp/Source/       ├── qt6_project/src/
│   ├── MainForm.cs                 │   ├── MainForm.cpp/.h/.ui
│   ├── Controls/                   │   ├── Controls/
│   │   ├── ServerItemListBox.cs    │   │   ├── ServerItemListBox.cpp/.h
│   │   ├── ClientItemView.cs       │   │   ├── ClientItemView.cpp/.h
│   │   ├── FlagCheckBox.cs         │   │   ├── FlagCheckBox.cpp/.h
│   │   └── ListBase.cs             │   │   └── ListBase.cpp/.h
│   ├── Dialogs/                    │   ├── Dialogs/
│   │   ├── AboutForm.cs            │   │   ├── AboutDialog.cpp/.h/.ui
│   │   ├── CompareOtbForm.cs       │   │   ├── CompareOtbDialog.cpp/.h/.ui
│   │   ├── FindItemForm.cs         │   │   ├── FindItemDialog.cpp/.h/.ui
│   │   └── [6 more dialogs]        │   │   └── [6 more dialogs]
│   ├── Helpers/                    │   ├── Helpers/
│   ├── Host/                       │   ├── Host/
│   ├── PluginInterface/            │   ├── PluginInterface/
│   └── Properties/                 │   └── Properties/
└── PluginOne/, PluginTwo/,         └── qt6_project/plugins/
    PluginThree/                        PluginOne/, PluginTwo/, PluginThree/
```

## Implementation Phases

### Phase 1: Foundation Components (CRITICAL PATH)
**Priority**: Highest - These enable all other functionality

#### 1.1 Plugin System Architecture
**Files to Implement:**
- `src/PluginInterface/IPlugin.h` - Core plugin interface
- `src/Host/PluginServices.cpp/.h` - Plugin discovery and loading
- `src/Host/Plugin.cpp/.h` - Plugin wrapper class
- `src/Host/PluginCollection.cpp/.h` - Plugin container

**Critical Requirements:**
```cpp
// IPlugin interface must exactly match C# functionality
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
    
    // Core methods (exact functional equivalents)
    virtual bool loadClient(const SupportedClient& client, 
                           bool extended, bool frameDurations, bool transparency,
                           const QString& datFullPath, const QString& sprFullPath) = 0;
    virtual void initialize() = 0;
    virtual SupportedClient getClientBySignatures(quint32 datSignature, quint32 sprSignature) = 0;
    virtual ClientItem* getClientItem(quint16 id) = 0;
};

Q_DECLARE_INTERFACE(IPlugin, "org.ottools.ItemEditor.IPlugin/1.0")
```

#### 1.2 MainForm Implementation
**Files to Implement:**
- `src/MainForm.cpp/.h/.ui` - Main application window

**Critical Features:**
- **Exact Layout**: Match pixel-perfect positioning from C# Designer
- **Complete Menu System**: All menu items with shortcuts and event handlers
- **Plugin Integration**: Full plugin lifecycle management
- **File Operations**: Open, save, create OTB files
- **Item Management**: Create, edit, delete, duplicate items

**Key Methods to Implement:**
```cpp
class MainForm : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainForm(QWidget* parent = nullptr);
    ~MainForm() override;

    // File operations (exact C# equivalents)
    bool openFile(const QString& filePath);
    bool saveFile(const QString& filePath = QString());
    bool saveAsFile();
    void createNewFile();

    // Item management (exact C# equivalents)
    void selectItem(ServerItem* item);
    void editItem(ServerItem* item);
    void createNewItem();
    void duplicateItem();
    void deleteItem();

    // UI updates (exact C# equivalents)
    void buildItemsList();
    void updateItemDisplay();
    void resetControls();

private slots:
    // All menu event handlers
    void onFileOpen();
    void onFileSave();
    void onEditCreateItem();
    void onEditFindItem();
    // ... (complete set matching C# version)

private:
    Ui::MainForm* ui;
    PluginServices* m_pluginServices;
    ServerItemList* m_serverItems;
    IPlugin* m_currentPlugin;
    QString m_currentFilePath;
    bool m_isModified;
};
```

### Phase 2: Custom Controls (CORE FUNCTIONALITY)

#### 2.1 ServerItemListBox Implementation
**Source Reference**: `Legacy_App/csharp/Source/Controls/ServerItemListBox.cs`
**Target**: `src/Controls/ServerItemListBox.cpp/.h`

**Critical Features:**
- **Virtual Scrolling**: Efficiently handle 10,000+ items
- **Sprite Thumbnails**: 32x32 pixel sprite rendering with caching
- **Selection Management**: Single-item selection with proper highlighting
- **Plugin Integration**: Access client sprites through plugin interface
- **Performance**: Smooth scrolling and responsive interaction

**Implementation Requirements:**
```cpp
class ServerItemListBox : public QAbstractItemView
{
    Q_OBJECT

public:
    explicit ServerItemListBox(QWidget* parent = nullptr);
    
    void setPlugin(IPlugin* plugin);
    void setItems(const QList<ServerItem*>& items);
    void addItem(ServerItem* item);
    void clear();
    
    ServerItem* selectedItem() const;
    void selectItem(ServerItem* item);

signals:
    void itemSelected(ServerItem* item);
    void itemDoubleClicked(ServerItem* item);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void paintItem(QPainter& painter, ServerItem* item, 
                   const QRect& rect, bool selected);
    QRect itemRect(int index) const;
    int itemAt(const QPoint& pos) const;
    void updateScrollBars();
    
    IPlugin* m_plugin;
    QList<ServerItem*> m_items;
    int m_selectedIndex;
    int m_itemHeight;
    QScrollBar* m_verticalScrollBar;
    mutable QPixmapCache m_spriteCache;
};
```

#### 2.2 ClientItemView Implementation
**Source Reference**: `Legacy_App/csharp/Source/Controls/ClientItemView.cs`
**Target**: `src/Controls/ClientItemView.cpp/.h`

**Critical Features:**
- **Centered Sprite Display**: Automatic centering within control bounds
- **Transparency Support**: Handle transparent sprite backgrounds correctly
- **Scaling Support**: Handle various sprite sizes (16x16, 32x32, 64x64)
- **Data Binding**: Connect to ClientItem objects

### Phase 3: Dialog System (COMPLETE FEATURE SET)

#### 3.1 FindItemDialog Implementation
**Source Reference**: `Legacy_App/csharp/Source/Dialogs/FindItemForm.cs`
**Target**: `src/Dialogs/FindItemDialog.cpp/.h/.ui`

**Critical Features:**
- **Multiple Search Modes**: By Server ID, Client ID, Properties
- **Property-Based Search**: Using FlagCheckBox controls for item flags
- **Embedded Results**: ServerItemListBox integration for result display
- **Real-Time Search**: As-you-type filtering and instant results

#### 3.2 CompareOtbDialog Implementation
**Source Reference**: `Legacy_App/csharp/Source/Dialogs/CompareOtbForm.cs`
**Target**: `src/Dialogs/CompareOtbDialog.cpp/.h/.ui`

**Critical Features:**
- **File Selection**: Browse and validate OTB files
- **Detailed Comparison**: Property-by-property analysis using reflection-like techniques
- **Difference Reporting**: Comprehensive change documentation
- **Progress Indication**: For large file comparisons

#### 3.3 Complete Dialog Set
**All dialogs must be implemented with full functionality:**
- AboutDialog - Application information and version details
- NewOtbFileDialog - OTB file creation wizard
- PreferencesDialog - Client configuration with file validation
- ProgressDialog - Operation progress with cancellation support
- UpdateDialog - OTB version migration tool
- UpdateSettingsDialog - Update configuration options

### Phase 4: File Format Implementation (DATA INTEGRITY CRITICAL)

#### 4.1 OTB File Handler
**Source Reference**: `Legacy_App/csharp/Source/PluginInterface/OTLib/OTB/`
**Target**: `src/PluginInterface/OTLib/OTB/`

**Critical Requirements:**
- **Binary Format Parsing**: Exact replication of C# parsing logic
- **Version Support**: All client versions 8.00-10.77
- **Endianness Handling**: Proper little-endian format support
- **Error Recovery**: Handle corrupted or invalid files gracefully
- **Progress Reporting**: For large file operations

#### 4.2 Plugin Implementations
**Source Reference**: `Legacy_App/csharp/Source/PluginOne/`, `PluginTwo/`, `PluginThree/`
**Target**: `qt6_project/plugins/PluginOne/`, `PluginTwo/`, `PluginThree/`

**Critical Features:**
- **DAT File Parsing**: Complete client data extraction
- **SPR File Handling**: Sprite extraction with transparency
- **Signature Detection**: Automatic client version identification
- **Animation Support**: Frame durations and sequences

## Technical Implementation Guidelines

### Data Type Conversions (MANDATORY MAPPINGS)
| C# Type | Qt6 Equivalent | Usage Notes |
|---------|----------------|-------------|
| `string` | `QString` | Use QString::fromStdString() for std::string conversion |
| `ushort` | `quint16` | Exact match for item IDs and sprite IDs |
| `uint` | `quint32` | File signatures and large numeric values |
| `Rectangle` | `QRect` | UI geometry and sprite bounds |
| `Point` | `QPoint` | 2D coordinates and positioning |
| `Size` | `QSize` | Dimensions and sizing |
| `Bitmap` | `QPixmap` | Sprite and image handling |
| `Graphics` | `QPainter` | Custom drawing operations |
| `List<T>` | `QList<T>` | Dynamic collections and arrays |
| `Dictionary<K,V>` | `QMap<K,V>` | Key-value storage and lookup |

### Event Handling Migration (CRITICAL PATTERN)
```cpp
// C# Event Pattern:
// public event EventHandler<ItemEventArgs> ItemSelected;
// protected virtual void OnItemSelected(ItemEventArgs e)

// Qt6 Signal/Slot Pattern:
signals:
    void itemSelected(const ItemEventArgs& args);

// Connection:
connect(serverItemListBox, &ServerItemListBox::itemSelected,
        this, &MainForm::onItemSelected);
```

### Memory Management (QT6 BEST PRACTICES)
```cpp
// Use Qt6 parent-child ownership model
class MainForm : public QMainWindow
{
private:
    // Qt objects with parent ownership
    ServerItemListBox* m_itemListBox;  // parent manages lifetime
    
    // Non-Qt objects with smart pointers
    std::unique_ptr<PluginServices> m_pluginServices;
    
    // Raw pointers for Qt plugin system
    IPlugin* m_currentPlugin;  // managed by plugin system
};
```

## Quality Assurance Requirements

### Testing Checklist (MANDATORY VALIDATION)
- [ ] **Functional Parity**: Every C# feature works identically in Qt6
- [ ] **File Compatibility**: Perfect interchange of files between versions
- [ ] **Plugin System**: All three plugins load and function correctly
- [ ] **Performance**: Meets or exceeds C# version benchmarks
- [ ] **Error Handling**: Graceful recovery from all error conditions
- [ ] **Memory Management**: No leaks during extended usage
- [ ] **Cross-Platform**: Builds and runs on Windows and Linux

### Performance Benchmarks (MINIMUM REQUIREMENTS)
- **Startup Time**: < 3 seconds on modern hardware
- **File Loading**: 10,000 item OTB file loads within 10 seconds
- **UI Responsiveness**: No blocking operations on main thread
- **Memory Usage**: Efficient sprite caching with automatic cleanup
- **Plugin Loading**: Fast discovery and initialization

### Code Quality Standards (MANDATORY)
```cpp
// Example of production-ready Qt6 code
class ServerItemListBox : public QAbstractItemView
{
    Q_OBJECT

public:
    explicit ServerItemListBox(QWidget* parent = nullptr);
    ~ServerItemListBox() override;

    /// Sets the plugin for sprite access
    /// @param plugin Valid plugin instance or nullptr to clear
    void setPlugin(IPlugin* plugin);
    
    /// Adds items to the list with efficient batch processing
    /// @param items List of server items to add
    void setItems(const QList<ServerItem*>& items);

signals:
    /// Emitted when item selection changes
    /// @param item Selected item or nullptr if none selected
    void itemSelected(ServerItem* item);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    /// Renders individual item with sprite and text
    void paintItem(QPainter& painter, const ServerItem* item, 
                   const QRect& rect, bool selected) const;
    
    /// Calculates item rectangle for given index
    QRect itemRect(int index) const;
    
    // Member variables with clear naming
    IPlugin* m_plugin;
    QList<ServerItem*> m_items;
    int m_selectedIndex;
    int m_itemHeight;
    mutable QPixmapCache m_spriteCache;
};
```

## Success Criteria and Completion

### Definition of Done
A task is complete when:
1. **Functional Parity**: Qt6 implementation behaves identically to C# reference
2. **Complete Implementation**: No placeholder code, TODOs, or stub methods
3. **Error Handling**: Comprehensive exception handling with user feedback
4. **Performance**: Meets or exceeds C# version performance benchmarks
5. **Testing**: All functionality verified through comprehensive testing
6. **Documentation**: Clear code comments and API documentation

### Final Validation
Before considering the migration complete:
- [ ] Every menu item and shortcut works identically to C# version
- [ ] All dialogs function exactly as in C# version
- [ ] File operations produce identical results
- [ ] Plugin system maintains full compatibility
- [ ] Performance meets or exceeds C# version
- [ ] Error handling provides equivalent user experience
- [ ] Cross-platform compatibility verified
- [ ] Memory usage optimized and leak-free

**Jules**: Your mission is to create a Qt6 application that is indistinguishable from the original C# version in terms of functionality, behavior, and user experience. The C# implementation is your complete specification - replicate it exactly with no compromises on features or quality.