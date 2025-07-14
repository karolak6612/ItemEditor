# Plugin System Documentation

## Overview

The Item Editor Qt6 plugin system provides a complete migration from the original C# plugin architecture to Qt6's QPluginLoader-based system. The architecture maintains exact functional parity while leveraging Qt6's modern plugin capabilities.

## Architecture

### Core Components

#### IPlugin Interface
**Location**: `src/PluginInterface/IPlugin.h`
**Purpose**: Defines the contract that all plugins must implement

```cpp
class IPlugin
{
public:
    virtual ~IPlugin() = default;
    
    // Properties
    virtual IPluginHost* host() const = 0;
    virtual void setHost(IPluginHost* host) = 0;
    virtual ClientItems* items() const = 0;
    virtual quint16 minItemId() const = 0;
    virtual quint16 maxItemId() const = 0;
    virtual QList<SupportedClient> supportedClients() const = 0;
    virtual bool loaded() const = 0;
    
    // Methods
    virtual bool loadClient(const SupportedClient& client, 
                           bool extended, bool frameDurations, bool transparency,
                           const QString& datFullPath, const QString& sprFullPath) = 0;
    virtual void initialize() = 0;
    virtual SupportedClient getClientBySignatures(quint32 datSignature, quint32 sprSignature) = 0;
    virtual ClientItem* getClientItem(quint16 id) = 0;
};

Q_DECLARE_INTERFACE(ItemEditor::IPlugin, "org.ottools.ItemEditor.IPlugin/1.0")
```

#### PluginServices
**Location**: `src/Host/PluginServices.h`
**Purpose**: Manages plugin discovery, loading, and lifecycle

**Key Features**:
- Automatic plugin discovery in plugins directory
- Thread-safe plugin loading with QPluginLoader
- Plugin validation and error handling
- Memory management for plugin instances

#### Plugin Implementation
**Locations**: `plugins/PluginOne/`, `plugins/PluginTwo/`, `plugins/PluginThree/`
**Purpose**: Concrete implementations for different Tibia client versions

## Current Implementation Status
## Plugin Development Guide

### Creating a New Plugin

#### 1. Plugin Class Structure
```cpp
class Plugin : public QObject, public ItemEditor::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ottools.ItemEditor.IPlugin/1.0" FILE "PluginName.json")
    Q_INTERFACES(ItemEditor::IPlugin)

public:
    explicit Plugin(QObject *parent = nullptr);
    virtual ~Plugin();

    // Implement all IPlugin methods
    // ...
};
```

#### 2. Plugin Metadata (PluginName.json)
```json
{
    "IID": "org.ottools.ItemEditor.IPlugin/1.0",
    "MetaData": {
        "name": "Plugin Name",
        "version": "1.0.0",
        "description": "Plugin description",
        "supportedClients": ["10.77", "10.78"]
    }
}
```

#### 3. CMake Integration
```cmake
qt_add_plugin(PluginName
    Plugin.cpp
    Plugin.h
    PluginName.json
)

target_link_libraries(PluginName PRIVATE
    Qt6::Core
    ItemEditorPluginInterface
)
```

### Plugin Loading Process

#### 1. Discovery Phase
- PluginServices scans plugins directory
- Validates plugin files (.dll/.so/.dylib)
- Checks plugin metadata

#### 2. Loading Phase
- QPluginLoader attempts to load plugin
- Validates plugin interface compatibility
- Calls plugin initialize() method

#### 3. Integration Phase
- Plugin registers supported clients
- Host provides services to plugin
- Plugin becomes available for use

## File Format Support

### Supported Formats

#### OTB Files (Open Tibia Binary)
- **Purpose**: Server item database
- **Status**: Framework complete, parsing partial
- **Plugin Role**: Read/write item definitions

#### SPR Files (Sprite Files)
- **Purpose**: Client sprite data
- **Status**: Framework complete, extraction needed
- **Plugin Role**: Extract sprites for UI display

#### DAT Files (Client Data)
- **Purpose**: Client item definitions
- **Status**: Framework complete, parsing partial
- **Plugin Role**: Read item properties and metadat


## Conclusion

The plugin system provides a robust, extensible architecture that successfully migrates from C# to Qt6 while maintaining full compatibility. The core infrastructure is production-ready, with remaining work focused on completing the file format parsing implementations within the plugins themselves.
