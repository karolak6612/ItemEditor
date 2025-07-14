# Task Execution Reminder - Item Editor Qt6 Migration

## Project Context

You are executing tasks for the **Item Editor Qt6 Migration** project. This is migrating a specialized game development tool from C# WinForms to Qt6 C++.

### What This Application Does
- **Purpose**: Edit game item databases for Open Tibia-style MMORPG systems
- **File Formats**: .otb (server), .dat/.spr (client), .xml (config)
- **Users**: Game developers working with Open Tibia item systems
- **Versions Supported**: Game client versions 8.00 - 10.77

### Migration Goal
Create a Qt6 C++ application with **exact functional parity** to the C# version. No new features, no missing functionality.

## Source Code Locations
- **C# Reference**: `Legacy_App/csharp/Source/` (what to copy)
- **Qt6 Target**: `qt6_project/src/` (where to implement)

## Critical Requirements When Executing Tasks

### 1. Functional Parity (MANDATORY)
- Qt6 app must behave **exactly** like C# version
- Every button, menu, dialog must work the same way
- File operations must produce identical results

### 2. Production Ready (MANDATORY)
- **NO** placeholders, TODOs, or "not implemented" messages
- **NO** crashes or obvious bugs
- Proper error handling for all operations

### 3. File Format Support (CRITICAL)
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
