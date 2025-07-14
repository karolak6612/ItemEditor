# Item Editor Qt6 Migration - AI Assistant Context Prompt

## Project Overview

You are working on migrating the **Item Editor** application from C# WinForms (located in `Legacy_App/csharp/`) to Qt6 C++ framework (located in `qt6_project/`). This is a specialized tool for editing game item databases in OTB (Open Tibia Binary) format for Open Tibia-style MMORPG systems.

## Migration Status & Context

**IMPORTANT**: Before starting any work, you MUST:
1. **Research current state** - Examine both C# source and Qt6 implementation to understand what's been migrated
2. **Compare implementations** - Verify migration quality and identify gaps
3. **Assess production readiness** - Check for placeholders, TODOs, "not implemented" messages, or bugs
4. **Plan systematically** - Use task management to organize remaining work

## Migration Requirements

### Core Principles
- **1:1 Migration**: Exact functional parity with C# application
- **No New Features**: Unless explicitly requested
- **Production Ready**: No placeholders, TODOs, or "not implemented" messages in final code
- **User Experience**: Application must work seamlessly from end-user perspective

### File Format Support (Critical)
The Qt6 application MUST handle these file formats exactly like the C# version:
- **.spr files** - Sprite data files
- **.dat files** - Client data files  
- **.otb files** - Open Tibia Binary server files
- **.xml files** - Configuration and metadata files

### Key Components to Migrate
- **Main Window** - Complete UI functionality, menus, toolbars
- **Item List Display** - Visual item browsing with sprites
- **Item Property Editing** - All item attributes and flags
- **File Operations** - Loading, saving, importing, exporting
- **Plugin System** - Client version support plugins
- **Preferences/Settings** - Application configuration
- **Dialog Windows** - All modal dialogs and forms
- **Rendering System** - Sprite display and graphics


### Research Protocol (MANDATORY FIRST STEP)

Before any coding work, you MUST:

1. **Examine C# Implementation**
   ```
   - Read Legacy_App/csharp/Source/ thoroughly
   - Understand UI structure, event handlers, data flow
   - Document key functionalities and user workflows
   ```

2. **Assess Qt6 Implementation**
   ```
   - Review qt6_project/src/ current state
   - Identify what's been migrated vs. what's missing
   - Check for placeholders, TODOs, incomplete implementations
   ```

3. **Gap Analysis**
   ```
   - Create detailed list of missing functionalities
   - Prioritize based on user impact
   - Plan implementation approach
   ```

4. **Quality Assessment**
   ```
   - Test existing Qt6 code for bugs
   - Verify file format handling works correctly
   - Ensure UI behaves like C# version
   ```

## Project Structure Reference

### C# Source (Reference Implementation)
```
Legacy_App/csharp/Source/
├── MainForm.cs/.Designer.cs     # Main application window
├── Controls/                    # Custom UI controls
├── Dialogs/                     # Modal dialog windows
├── Helpers/                     # Utility classes
├── Host/                        # Plugin hosting system
├── PluginInterface/             # Plugin system interface
└── Properties/                  # Application metadata
```

### Qt6 Target (Migration Destination)
```
qt6_project/src/
├── main.cpp                     # Application entry point
├── MainForm.cpp/.h/.ui         # Main window (MainForm.cs equivalent)
├── Controls/                    # Custom Qt widgets
├── Dialogs/                     # Dialog windows
├── Helpers/                     # Utility classes
├── Host/                        # Plugin hosting
├── PluginInterface/             # Plugin system
└── Properties/                  # Application metadata
```

## Development Standards

### Code Quality Requirements
- **No Placeholders**: Remove all "TODO", "FIXME", "Not implemented" comments
- **Error Handling**: Implement proper error handling for all operations
- **Memory Management**: Use Qt's parent-child ownership model correctly
- **Performance**: Ensure responsive UI and efficient file operations

### Qt6 Best Practices
- Use modern Qt6 signal/slot syntax
- Implement proper Model/View architecture where applicable
- Use Qt's resource system for assets
- Follow Qt naming conventions and patterns

### Testing Requirements
- Test all migrated functionality thoroughly
- Verify file format compatibility
- Ensure UI responsiveness and correct behavior
- Test plugin loading and functionality

## Typical Workflow

1. **Context Restoration**
   - Examine current Qt6 implementation state
   - Identify immediate priorities

2. **Research Phase**
   - understand Qt6 patterns
   - Study C# implementation for specific functionality
   - Plan migration approach

3. **Implementation Phase**
   - Migrate C# code to Qt6 equivalent
   - Ensure 1:1 functional parity
   - Test thoroughly before considering complete

4. **Validation Phase**
   - Test from user perspective
   - Verify file operations work correctly
   - Ensure no production issues remain

## Success Criteria

The migration is successful when:
- ✅ Qt6 application has identical functionality to C# version
- ✅ All file formats (.spr, .dat, .otb, .xml) load and save correctly
- ✅ UI behaves exactly like the original application
- ✅ No error messages, placeholders, or incomplete features
- ✅ Application is ready for production use
- ✅ Plugin system works with all supported client versions

## Remember

- **Always start with research** - Understand current state before coding
- **Think like an end user** - Will they encounter bugs or missing features?
- **Maintain quality** - Production-ready code only
- **Document decisions** - Keep track of migration choices and rationale

---

**Use this prompt every time you work on the Item Editor migration to maintain context and ensure systematic, high-quality progress.**
