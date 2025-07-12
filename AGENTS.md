# ItemEditor C# to Qt6 Migration Guide

This document outlines the plan and strategy for migrating the ItemEditor application from C# WinForms to C++/Qt6.

## 1. C# Project Structure

The ItemEditor solution is organized as follows:

*   **`ItemEditor.sln`**: The main solution file.
*   **`Source/`**: Contains the primary source code for the application.
    *   **`ItemEditor/`**: The main Windows Forms application project.
        *   **`Controls/`**: Custom WinForms controls (e.g., `ClientItemView`, `ServerItemListBox`).
        *   **`Dialogs/`**: Various dialog windows used in the application (e.g., `AboutForm`, `FindItemForm`, `PreferencesForm`).
        *   **`Helpers/`**: Utility classes.
        *   **`Host/`**: Plugin hosting infrastructure.
        *   **`MainForm.cs`**: The main application window and core logic.
        *   **`Program.cs`**: Application entry point.
    *   **`PluginInterface/`**: Defines the interface for plugins and includes OTLib.
        *   **`OTLib/`**: Library for handling OTB (OpenTibia Binary) files.
        *   **`ImageSimilarity/`**: Utilities for comparing image signatures.
    *   **`PluginOne/`**, **`PluginTwo/`**, etc.: Example plugin implementations.
*   **`ThirdParty/`**: Contains third-party libraries like `DarkUI`.

## 2. Feature List

The ItemEditor application provides the following key features:

*   **OTB File Management:** Create, Open, Save, Save As.
*   **Item Editing:** View and edit a wide range of server item properties and flags.
*   **Item Operations:** Create, duplicate, reload attributes, and create missing items from client data.
*   **Client Version Management:** A plugin system allows loading different client versions (.dat/.spr files) to provide sprites and comparative data.
*   **Tools:** Find items, compare OTB files, and update an OTB to a new client version.

## 3. Dependencies

*   **.NET Framework**
*   **Windows Forms**
*   **DarkUI:** Third-party library for UI theming.

## 4. C# to Qt6 Migration Strategy

The migration involves translating the C# codebase to C++/Qt6, focusing on one component at a time, prioritizing core logic (OTB handling) first, then UI, then advanced features.

## 5. Qt6 Migrated Architecture

The migrated application in `project_qt6/` is built using C++ and the Qt6 framework. Its architecture is as follows:

*   **`ItemEditorQt` (Main Application):**
    *   **`main.cpp`:** Entry point. Creates the `QApplication` and `MainWindow`, loads the dark theme stylesheet (`dark.qss`) from resources, and initializes the `Sprite` singleton data.
    *   **`MainWindow` (`mainwindow.h/.cpp`):** The core UI class, inheriting from `QMainWindow`. It's responsible for:
        *   The main window layout, menus, toolbars, and status bar.
        *   Hosting all primary UI elements (item list, attribute editors, sprite views).
        *   Managing application state (loaded OTB data, current plugin, modified status).
        *   Containing all slots for user actions (File I/O, Item Operations, Tools).
        *   Coordinating interactions between the data models, UI, and plugin system.
    *   **Dialogs (`dialogs/`):** Each dialog (`AboutDialog`, `FindItemDialog`, `PreferencesDialog`, `CompareOtbDialog`, `UpdateOtbDialog`, `SpriteCandidatesDialog`) is a self-contained `QDialog` subclass responsible for a specific user interaction.
    *   **Custom Widgets (`widgets/`):**
        *   **`ClientItemView`:** A `QWidget` for displaying item sprites. It takes a `ClientItem` and uses its (decompressed) sprite data for rendering.

*   **Data Models & OTB Handling (`otb/`):**
    *   **`otbtypes.h/.cpp`:** Defines core data structures like `ServerItem`, `ServerItemList`, and all related OTB enums (`ServerItemType`, `ServerItemFlag`, etc.). `ServerItem` contains methods for property management (`equals`, `copyPropertiesFrom`, `updateFlagsFromProperties`).
    *   **`item.h/.cpp`:** Defines client-side data structures like `ItemBase`, `ClientItem`, and `Sprite`. These classes contain the logic for handling sprite data (decompression, bitmap generation) and image signatures (stubs).
    *   **`binarytree.h/.cpp`:** A low-level utility class for reading and writing the OTB file's node-based binary structure, including handling of escaped bytes.
    *   **`otbreader.h/.cpp` & `otbwriter.h/.cpp`:** High-level classes that use `BinaryTree` to parse OTB files into `ServerItemList` objects and serialize them back to disk.

*   **Plugin System (`plugins/`):**
    *   **`iplugin.h/.cpp`:** Defines the plugin architecture:
        *   **`IPlugin`:** The abstract interface that all plugins must implement. It defines the contract for loading client data and providing `ClientItem`s.
        *   **`PluginManager`:** A class responsible for discovering and loading plugins. It uses `QPluginLoader` to find and load dynamic libraries from a `./plugins` subdirectory. It can also manage statically registered plugins.
    *   **`DummyPlugin` & `RealPlugin770`:** Example plugin implementations. `RealPlugin770` is the primary "real" plugin that uses the `tibiadata` library to parse client files.

*   **Tibia Data Parsing (`tibiadata/`):**
    *   **`sprparser.h/.cpp`:** A parser for Tibia `.spr` files. Reads the header and sprite addresses, and extracts raw, compressed sprite data.
    *   **`datparser.h/.cpp`:** A (currently stubbed) parser for Tibia `.dat` files. Reads the header and contains placeholder logic for parsing item attributes. **This is a known incomplete module requiring significant work.**
    *   **`imagesimilarity.h/.cpp`:** Contains stubs for porting the C# image similarity logic (FFT, signature calculation). **This is a known incomplete module.**

*   **Build System:**
    *   **`CMakeLists.txt`:** Manages the entire build. It finds Qt6, configures AUTOMOC/AUTORCC/AUTOUIC, includes all source and header directories, and links the necessary Qt libraries. It also handles the Qt resource file (`resources.qrc`).
    *   **`resources.qrc`:** Embeds application assets like the stylesheet (`dark.qss`) and application icon (`ItemEditor.ico`) into the executable.
