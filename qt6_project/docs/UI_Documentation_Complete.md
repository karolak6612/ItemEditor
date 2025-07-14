# Legacy Item Editor - Complete UI Documentation

## Table of Contents
1. [Application Overview](#application-overview)
2. [Main Interface Structure](#main-interface-structure)
3. [Menu System](#menu-system)
4. [Dialog Windows](#dialog-windows)
5. [Custom Controls](#custom-controls)
6. [Plugin Architecture](#plugin-architecture)
7. [User Interaction Flows](#user-interaction-flows)
8. [Code Mappings](#code-mappings)
9. [Maintenance Guide](#maintenance-guide)

## Application Overview

### Purpose
The Legacy Item Editor is a specialized C# WinForms application designed for editing game item databases in OTB (Open Tibia Binary) format. It serves as a comprehensive tool for game developers working with Open Tibia-style MMORPG item systems.

### Architecture
- **Framework**: .NET Framework with WinForms
- **UI Theme**: DarkUI library for consistent dark interface
- **Plugin System**: Modular architecture supporting multiple client versions
- **File Formats**: OTB (server), DAT/SPR (client)

### Key Features
- Visual item editing with sprite preview
- Plugin-based client version support
- Advanced search and filtering capabilities
- File comparison tools
- Batch operations for item management

## Main Interface Structure

### MainForm Layout
**File**: `Legacy_App\csharp\Source\MainForm.cs` and `MainForm.Designer.cs`

The main window consists of four primary areas:

#### 1. Item List Panel (Left Side)
- **Component**: `ServerItemListBox serverItemListBox`
- **Size**: 232x440 pixels
- **Location**: (10, 59)
- **Purpose**: Displays scrollable list of server items with sprite thumbnails

#### 2. Appearance Panel (Center)
- **Component**: `DarkGroupBox appearanceGroupBox`
- **Size**: 89x309 pixels  
- **Location**: (252, 53)
- **Contents**:
  - Previous sprite display (`previousPictureBox`)
  - Current sprite display (`pictureBox`)
  - Server ID label (`serverIdLbl`)
  - Client ID numeric control (`clientIdUpDown`)
  - Candidates button (`candidatesButton`)

#### 3. Attributes Panel (Right Side)
- **Component**: `DarkGroupBox optionsGroupBox`
- **Size**: 425x309 pixels
- **Location**: (349, 53)
- **Contents**: Item properties and configuration controls

#### 4. Output Panel (Bottom)
- **Component**: `DarkTextBox outputTextBox`
- **Size**: 525x160 pixels
- **Location**: (251, 371)
- **Purpose**: Application log and status messages

## Menu System

### File Menu (`fileMenuItem`)

| Menu Item | Shortcut | Event Handler | Target Method | Purpose |
|-----------|----------|---------------|---------------|---------|
| New | Ctrl+N | `FileNewMenuItem_Click` | Opens `NewOtbFileForm` | Create new OTB file |
| Open | Ctrl+O | `FileOpenMenuItem_Click` | `MainForm.Open()` | Load existing OTB file |
| Save | Ctrl+S | `FileSaveMenuItem_Click` | `MainForm.Save()` | Save current file |
| Save As | Ctrl+Shift+S | `FileSaveAsMenuItem_Click` | `MainForm.SaveAs()` | Save with new name |
| Preferences | Ctrl+P | `FilePreferencesMenuItem_Click` | Opens `PreferencesForm` | Configure settings |
| Exit | - | `FileExitMenuItem_Click` | `this.Close()` | Close application |

### Edit Menu (`editMenuItem`)

| Menu Item | Shortcut | Event Handler | Target Method | Purpose |
|-----------|----------|---------------|---------------|---------|
| Create Item | Ctrl+I | `EditCreateItemMenuItem_Click` | `MainForm.AddNewItem()` | Add new item |
| Duplicate Item | Ctrl+D | `EditDuplicateItemMenuItem_Click` | `MainForm.DuplicateItem()` | Copy selected item |
| Reload Item | Ctrl+R | `EditReloadItemMenuItem_Click` | `MainForm.ReloadSelectedItem()` | Refresh from client |
| Find Item | Ctrl+F | `FindItemButton_Click` | Opens `FindItemForm` | Search items |
| Create Missing Items | - | `EditCreateMissingItemsMenu_Click` | Auto-generation | Fill gaps in ID range |

### View Menu (`viewMenuItem`)

| Menu Item | Event Handler | Target Method | Purpose |
|-----------|---------------|---------------|---------|
| Show Mismatched Items | `ShowOnlyUnmatchedToolStripMenuItem_Click` | `BuildItemsListBox()` | Filter mismatched items |
| Show Deprecated Items | `ViewShowDecaptedItemsMenuItem_Click` | `BuildItemsListBox()` | Show/hide deprecated |
| Update Items List | `ViewUpdateItemsListMenuItem_Click` | `BuildItemsListBox()` | Refresh item list |

### Tools Menu (`toolsMenuItem`)

| Menu Item | Event Handler | Target Method | Purpose |
|-----------|---------------|---------------|---------|
| Reload Item Attributes | `ToolsReloadItemAttributesMenuItem_Click` | `ReloadItems()` | Sync all items with client |
| Compare OTB Files | `ToolsCompareOtbFilesMenuItem_Click` | Opens `CompareOtbForm` | Compare two OTB files |
| Update OTB Version | `ToolsUpdateVersionMenuItem_Click` | Opens `UpdateForm` | Migrate to new version |

### Help Menu (`helpMenuItem`)

| Menu Item | Event Handler | Target Method | Purpose |
|-----------|---------------|---------------|---------|
| About | `HelpAboutMenuItem_Click` | Opens `AboutForm` | Show application info |

## Dialog Windows

### 1. Find Item Dialog
**File**: `Legacy_App\csharp\Source\Dialogs\FindItemForm.cs`

#### Purpose
Advanced search functionality for locating items by various criteria.

#### Components
- **Search Mode Selection**:
  - `findBySidButton` - Search by Server ID
  - `findByCidButton` - Search by Client ID  
  - `findByPropertiesButton` - Search by item properties

- **Input Controls**:
  - `findIdNumericUpDown` - ID input field
  - Property checkboxes (using `FlagCheckBox` controls)

- **Results Display**:
  - `serverItemList` - Embedded `ServerItemListBox`

#### Key Methods
```csharp
private void UpdateProperties()    // Updates UI based on search mode
private void StartFind()          // Executes search operation
```

#### User Interaction Flow
1. User selects search mode (ID or Properties)
2. UI updates to show relevant input controls
3. User enters search criteria
4. Click Find button or press Enter
5. Results populate in embedded list
6. Clicking result selects item in main form

### 2. Compare OTB Files Dialog
**File**: `Legacy_App\csharp\Source\Dialogs\CompareOtbForm.cs`

#### Purpose
Compare two OTB files and display detailed differences.

#### Components
- **File Selection**:
  - `file1Text` / `browseButton1` - First file selection
  - `file2Text` / `browseButton2` - Second file selection

- **Results Display**:
  - `resultTextBox` - Detailed comparison output

#### Key Methods
```csharp
private bool CompareItems()       // Performs detailed comparison using reflection
```

#### Comparison Features
- Item count differences
- Sprite changes (client ID modifications)
- Sprite hash updates
- Property-by-property comparison using reflection
- Detailed change reporting

### 3. Preferences Dialog
**File**: `Legacy_App\csharp\Source\Dialogs\PreferencesForm.cs`

#### Purpose
Configure client directory and file loading options.

#### Components
- **Client Configuration**:
  - `directoryPathTextBox` - Client files directory
  - `browseButton` - Directory browser

- **Loading Options**:
  - `extendedCheckBox` - Extended format support
  - `frameDurationsCheckBox` - Animation frame data
  - `transparencyCheckBox` - Transparency handling

#### Key Methods
```csharp
private void OnSelectFiles(string directory)  // Validates client files
private uint GetSignature(string fileName)    // Reads file signatures
```

#### Validation Process
1. Directory validation
2. DAT/SPR file detection
3. Signature reading and plugin matching
4. Version compatibility checking
5. Settings persistence

### 4. About Dialog
**File**: `Legacy_App\csharp\Source\Dialogs\AboutForm.cs`

#### Purpose
Display application information, version, and credits.

#### Components
- Background image display
- Version information
- Clickable links to project resources

## Custom Controls

### 1. ServerItemListBox
**File**: `Legacy_App\csharp\Source\Controls\ServerItemListBox.cs`

#### Purpose
High-performance list control for displaying server items with sprite thumbnails.

#### Key Features
- **Virtual Scrolling**: Handles large item lists efficiently
- **Sprite Rendering**: Displays 32x32 sprite thumbnails
- **Selection Management**: Single-item selection with highlighting
- **Plugin Integration**: Accesses client sprites through plugin interface

#### Key Properties
```csharp
public IPlugin Plugin { get; set; }           // Plugin for sprite access
public ushort MinimumID { get; private set; } // ID range tracking
public ushort MaximumID { get; private set; }
```

#### Key Methods
```csharp
public void Add(List<ServerItem> items)       // Bulk item addition
protected override void PaintContent(Graphics graphics) // Custom rendering
```

#### Rendering Process
1. Calculate visible item range
2. For each visible item:
   - Draw selection background if selected
   - Render sprite thumbnail (32x32)
   - Draw item text (ID and name)
   - Apply borders and styling

### 2. ClientItemView
**File**: `Legacy_App\csharp\Source\Controls\ClientItemView.cs`

#### Purpose
Display control for individual client item sprites with automatic centering.

#### Key Features
- **Automatic Centering**: Centers sprite within control bounds
- **Scaling Support**: Handles various sprite sizes
- **Transparency**: Supports transparent sprite backgrounds

#### Key Properties
```csharp
public ClientItem ClientItem { get; set; }    // Item to display
```

#### Key Methods
```csharp
protected override void OnPaint(PaintEventArgs e) // Custom sprite rendering
```

#### Rendering Logic
1. Get sprite bitmap from ClientItem
2. Calculate centered position within control
3. Set up source and destination rectangles
4. Draw sprite with proper scaling

### 3. FlagCheckBox
**File**: `Legacy_App\csharp\Source\Controls\FlagCheckBox.cs`

#### Purpose
Specialized checkbox control for ServerItemFlag enumeration binding.

#### Key Features
- **Flag Binding**: Automatically binds to specific ServerItemFlag values
- **Event Integration**: Supports property-based searching
- **Type Safety**: Ensures correct flag handling

## Plugin Architecture

### Core Interface (IPlugin)
**File**: `Legacy_App\csharp\Source\PluginInterface\PluginInterface.cs`

#### Interface Definition
```csharp
public interface IPlugin : IDisposable
{
    IPluginHost Host { get; set; }
    ClientItems Items { get; }
    ushort MinItemId { get; }
    ushort MaxItemId { get; }
    List<SupportedClient> SupportedClients { get; }
    bool Loaded { get; }
    
    bool LoadClient(SupportedClient client, bool extended, 
                   bool frameDurations, bool transparency, 
                   string datFullPath, string sprFullPath);
    void Initialize();
    SupportedClient GetClientBySignatures(uint datSignature, uint sprSignature);
    ClientItem GetClientItem(ushort id);
}
```

#### Key Responsibilities
- **Client Loading**: Parse DAT/SPR files for specific client versions
- **Item Access**: Provide client item data by ID
- **Version Detection**: Match client versions by file signatures
- **Resource Management**: Handle sprite and item data lifecycle

### Plugin Implementation (PluginOne)
**File**: `Legacy_App\csharp\Source\PluginOne\Plugin.cs`

#### Supported Features
- Multiple client version support
- Extended format handling (32-bit sprite IDs)
- Frame duration support for animations
- Transparency options

#### Key Methods
```csharp
public bool LoadDat(string filename, SupportedClient client, 
                   bool extended, bool frameDurations)
public bool LoadSprites(string filename, SupportedClient client, 
                       bool extended, bool transparency)
```

#### DAT File Parsing Process
1. Read and validate file signature
2. Read item count and ranges
3. For each item:
   - Parse item flags and properties
   - Read sprite dimensions and animation data
   - Load sprite ID references
   - Create ClientItem with complete data

## User Interaction Flows

### Opening an OTB File
```
User Action: File → Open
    ↓
FileOpenMenuItem_Click()
    ↓
MainForm.Open()
    ↓
File Dialog Selection
    ↓
OtbReader.Read(path)
    ↓
Plugin Detection (by OTB version)
    ↓
LoadClient() - Parse DAT/SPR files
    ↓
UI Controls Enabled
    ↓
BuildItemsListBox() - Populate item list
```

### Editing an Item
```
User Action: Click item in list
    ↓
ItemsListBox_SelectedIndexChanged()
    ↓
MainForm.SelectItem(item)
    ↓
EditItem(item)
    ↓
- Load client item data
- Setup data bindings
- Update sprite displays
- Enable editing controls
    ↓
User modifies properties
    ↓
Changes saved via data binding
```

### Searching for Items
```
User Action: Edit → Find Item
    ↓
FindItemButton_Click()
    ↓
FindItemForm.Show()
    ↓
User selects search criteria
    ↓
StartFind() method
    ↓
Search execution based on criteria
    ↓
Results displayed in embedded list
    ↓
User clicks result
    ↓
Main form selects corresponding item
```

## Code Mappings

### UI Control to Code Mappings

#### Main Form Controls
| UI Control | Variable Name | Event Handler | Data Binding Target |
|------------|---------------|---------------|-------------------|
| Server ID Label | `serverIdLbl` | - | `item.ID` |
| Client ID Numeric | `clientIdUpDown` | `ClientIdUpDown_ValueChanged` | `clientItem.ID` |
| Type Combo | `typeCombo` | `TypeCombo_SelectedIndexChanged` | `item.Type` |
| Stack Order Combo | `stackOrderComboBox` | `StackOrderComboBox_SelectedIndexChanged` | `item.StackOrder` |

#### Boolean Attributes
| UI Control | Variable Name | Data Binding Target |
|------------|---------------|-------------------|
| Unpassable | `unpassableCheck` | `item.Unpassable` |
| Movable | `movableCheck` | `item.Movable` |
| Block Missiles | `blockMissilesCheck` | `item.BlockMissiles` |
| Block Pathfinder | `blockPathfinderCheck` | `item.BlockPathfinder` |
| Force Use | `forceUseCheckBox` | `item.ForceUse` |
| Multi Use | `useableCheck` | `item.MultiUse` |
| Pickupable | `pickupableCheck` | `item.Pickupable` |
| Stackable | `stackableCheck` | `item.Stackable` |
| Readable | `readableCheck` | `item.Readable` |
| Rotatable | `rotatableCheck` | `item.Rotatable` |
| Hangable | `hangableCheck` | `item.Hangable` |
| Hook South | `hookSouthCheck` | `item.HookSouth` |
| Hook East | `hookEastCheck` | `item.HookEast` |
| Has Elevation | `hasElevationCheck` | `item.HasElevation` |
| Ignore Look | `ignoreLookCheck` | `item.IgnoreLook` |
| Full Ground | `fullGroundCheck` | `item.FullGround` |

#### Numeric Attributes
| UI Control | Variable Name | Data Binding Target | Validation |
|------------|---------------|-------------------|------------|
| Ground Speed | `groundSpeedText` | `item.GroundSpeed` | Numeric only |
| Light Level | `lightLevelText` | `item.LightLevel` | Numeric only |
| Light Color | `lightColorText` | `item.LightColor` | Numeric only |
| Minimap Color | `minimapColorText` | `item.MinimapColor` | Numeric only |
| Max Read Chars | `maxReadCharsText` | `item.MaxReadChars` | Numeric only |
| Max Read/Write Chars | `maxReadWriteCharsText` | `item.MaxReadWriteChars` | Numeric only |
| Ware ID | `wareIdText` | `item.TradeAs` | Numeric only |
| Name | `nameText` | `item.Name` | Text |

### Data Binding Implementation
The application uses WinForms data binding to connect UI controls directly to item properties:

```csharp
// Example from EditItem() method
this.serverIdLbl.DataBindings.Add("Text", item, "ID");
this.clientIdUpDown.DataBindings.Add("Value", clientItem, "ID");
this.AddBinding(this.unpassableCheck, "Checked", item, "Unpassable", 
                item.Unpassable, clientItem.Unpassable);
```

### Color Coding System
The application uses color coding to indicate data consistency:
- **Normal Color** (`Colors.LightText`): Property matches between server and client
- **Red Color** (`Color.Red`): Property differs between server and client
- **Background Colors**:
  - Normal (`Colors.DarkBackground`): Sprite hash matches
  - Red (`Color.Red`): Sprite hash differs

## Maintenance Guide

### Adding New UI Controls
1. **Designer Changes**: Add control in `MainForm.Designer.cs`
2. **Event Handlers**: Implement in `MainForm.cs`
3. **Data Binding**: Add binding setup in `EditItem()` method
4. **Reset Logic**: Update `ResetControls()` method
5. **Validation**: Add validation in appropriate event handlers

### Adding New Dialog Windows
1. **Create Form**: New form class inheriting from `DarkForm`
2. **Designer File**: Layout controls using DarkUI components
3. **Event Handlers**: Implement user interaction logic
4. **Integration**: Add menu item and event handler in MainForm
5. **Data Exchange**: Implement properties for data passing

### Adding New Custom Controls
1. **Base Class**: Inherit from appropriate WinForms control
2. **Custom Properties**: Define control-specific properties
3. **Paint Override**: Implement custom rendering if needed
4. **Event Handling**: Add custom events as needed
5. **Integration**: Use in forms and handle events

### Plugin Development
1. **Interface Implementation**: Implement `IPlugin` interface
2. **Client Support**: Define supported client versions
3. **File Parsing**: Implement DAT/SPR file parsing
4. **Registration**: Add plugin to application plugin collection
5. **Testing**: Verify with target client versions

### Common Maintenance Tasks

#### Adding New Item Properties
1. Add property to `Item` class
2. Add UI control to attributes panel
3. Update data binding in `EditItem()`
4. Add to comparison logic in `CompareItems()`
5. Update plugin parsing if needed

#### Supporting New Client Versions
1. Create new plugin or extend existing
2. Add client version to `SupportedClient` list
3. Implement version-specific parsing logic
4. Test with target client files
5. Update documentation

#### UI Theme Updates
1. Update DarkUI library reference
2. Verify control styling consistency
3. Test color schemes and accessibility
4. Update custom control rendering if needed

### Debugging Tips

#### Common Issues
- **Data Binding Errors**: Check property names and types
- **Plugin Loading**: Verify file signatures and paths
- **UI Performance**: Monitor item list rendering with large datasets
- **Memory Usage**: Ensure proper disposal of graphics resources

#### Diagnostic Tools
- **Output Panel**: Monitor trace messages and errors
- **Visual Studio Debugger**: Step through event handlers
- **Plugin Validation**: Test with known good client files
- **UI Inspector**: Verify control states and bindings

This comprehensive documentation provides the foundation for understanding, maintaining, and extending the Legacy Item Editor application's user interface and underlying architecture.