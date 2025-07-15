# Task Execution Reminder - Item Editor Qt6 Migration

## Project Context

You are executing tasks for the **Item Editor Qt6 Migration** project. This is migrating a specialized game development tool from C# WinForms to Qt6 C++.

### What This Application Does
- **Purpose**: Edit game item databases for Open Tibia-style MMORPG systems
- **File Formats**: .otb (server), .dat/.spr (client), .xml (config)
- **Users**: Game developers working with Open Tibia item systems
- **Versions Supported**: Game client versions 8.00 - 10.77

### Migration Goal and Success Criteria
Create a Qt6 C++17 application that achieves **perfect functional parity** with the C# WinForms version:
- **Zero Feature Loss**: Every capability from C# version must work identically
- **Zero Regressions**: No functionality degradation or behavioral changes
- **Production Quality**: Professional-grade code with comprehensive error handling
- **Performance Parity**: Meet or exceed C# version performance benchmarks
- **Cross-Platform**: Support Windows (primary) and Linux platforms

## Source Code Locations
- **C# Reference**: `Legacy_App/csharp/Source/` (what to copy)
- **Qt6 Target**: `qt6_project/src/` (where to implement)

## Critical Implementation Requirements for Every Task

### 1. Functional Parity (ABSOLUTE REQUIREMENT)
**Every aspect must work identically to C# version:**
- **UI Behavior**: Button clicks, menu selections, keyboard shortcuts
- **Data Flow**: Item selection, property editing, file operations
- **Visual Appearance**: Layout, colors, fonts, icons, spacing
- **Error Handling**: Same error messages and recovery mechanisms
- **Performance**: Responsive UI with no blocking operations
- **File Operations**: Identical read/write results for all formats

### 2. Production Code Quality (MANDATORY STANDARD)
**Zero tolerance for incomplete implementations:**
- **Complete Methods**: Every function must be fully implemented
- **Error Handling**: Comprehensive exception handling and user feedback
- **Memory Management**: Proper Qt6 object lifecycle and resource cleanup
- **Thread Safety**: All UI updates on main thread, background operations properly managed
- **Input Validation**: Robust validation for all user inputs and file data
- **Documentation**: Clear code comments and API documentation

### 3. File Format Implementation (DATA INTEGRITY CRITICAL)
**Binary format parsing must be exact:**
- **OTB Files**: Complete Open Tibia Binary format support
  - All versions 8.00-10.77 supported
  - Proper endianness handling (little-endian)
  - Complete item property parsing
  - Robust error recovery for corrupted files
- **DAT Files**: Client data file parsing
  - Signature-based version detection
  - Complete item flag and property extraction
  - Animation frame and duration support
- **SPR Files**: Sprite file handling
  - Transparency support with proper alpha channels
  - Multiple sprite sizes and formats
  - Efficient memory usage for large sprite sets
- **XML Files**: Configuration and data exchange
  - Schema validation and error reporting
  - Unicode support for international characters

### 4. Plugin Architecture (SYSTEM FOUNDATION)
**Qt6 plugin system must replicate C# functionality:**
- **Plugin Discovery**: Automatic scanning and loading from plugins directory
- **Interface Compatibility**: Exact method signatures and behavior
- **Version Support**: PluginOne (8.00-10.77), PluginTwo (extended), PluginThree (latest)
- **Error Recovery**: Graceful handling of missing or incompatible plugins
- **Dynamic Loading**: QPluginLoader-based system with proper cleanup
- **Host Services**: Complete plugin host interface implementation

### 5. Qt6 Technical Standards (FRAMEWORK COMPLIANCE)
**Modern Qt6 patterns and best practices:**
- **Signal/Slot Connections**: Modern syntax with type safety
- **Model/View Architecture**: For complex data display (item lists)
- **Custom Painting**: QPainter for sprite rendering and custom controls
- **Resource Management**: Qt6 resource system for icons and stylesheets
- **Settings Management**: QSettings for user preferences and configuration
- **Threading**: QThread and QConcurrent for background operations

## Task Execution Checklist

Before marking ANY task complete:

✅ **Compare with C#**: Does Qt6 version behave exactly like C#?  
✅ **Test thoroughly**: All user workflows work correctly?  
✅ **No placeholders**: All code is production-ready?  
✅ **File formats**: Can load/save all required file types?  
✅ **Error handling**: Graceful handling of edge cases?  
✅ **Qt6 standards**: Follows Qt6 best practices?

## Remember
This is a **migration project**, not new development. The C# version is your specification - make the Qt6 version work exactly the same way.

---
**Execute your assigned task with these requirements in mind.**
CAL)
- .spr files must load/save correctly
- .dat files must load/save correctly  
- .otb files must load/save correctly
- .xml files must load/save correctly

### 4. Plugin System (IMPORTANT)
- Must support client version plugins (PluginOne, PluginTwo, PluginThree)
- Plugin loading must work like C# version

## Task Execution Checklist

Before marking ANY task complete:

✅ **Compare with C#**: Does Qt6 version behave exactly like C#?  
✅ **Test thoroughly**: All user workflows work correctly?  
✅ **No placeholders**: All code is production-ready?  
✅ **File formats**: Can load/save all required file types?  
✅ **Error handling**: Graceful handling of edge cases?  
✅ **Qt6 standards**: Follows Qt6 best practices?

## Remember
This is a **migration project**, not new development. The C# version is your specification - make the Qt6 version work exactly the same way.

---
**Execute your assigned task with these requirements in mind.**
