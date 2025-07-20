cmake_minimum_required(VERSION 3.16)
project(ItemEditor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific configurations
if(WIN32)
    set(CMAKE_GENERATOR_PLATFORM x64)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
elseif(UNIX AND NOT APPLE)
    # Linux-specific settings
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    add_definitions(-D_GNU_SOURCE)
    
    # Enable debug symbols in debug builds
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    endif()
endif()

# Find Qt6 components with flexible path detection
if(WIN32)
    # Windows default paths
    if(NOT DEFINED CMAKE_PREFIX_PATH)
        set(CMAKE_PREFIX_PATH "C:/Qt/6.9.1/msvc2022_64")
    endif()
elseif(UNIX AND NOT APPLE)
    # Linux: Try common Qt6 installation paths
    if(NOT DEFINED CMAKE_PREFIX_PATH)
        find_path(QT6_CMAKE_PATH
            NAMES Qt6Config.cmake
            PATHS
                /usr/lib/x86_64-linux-gnu/cmake/Qt6
                /usr/lib/cmake/Qt6
                /usr/local/lib/cmake/Qt6
                /opt/Qt/6.*/gcc_64/lib/cmake/Qt6
                /home/$ENV{USER}/Qt/6.*/gcc_64/lib/cmake/Qt6
            NO_DEFAULT_PATH
        )
        if(QT6_CMAKE_PATH)
            get_filename_component(CMAKE_PREFIX_PATH ${QT6_CMAKE_PATH}/../../.. ABSOLUTE)
        endif()
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

# Enable Qt6 standard project setup
qt6_standard_project_setup()

# Add subdirectories
add_subdirectory(src/ItemEditor.Core)
add_subdirectory(src/ItemEditor.UI)
add_subdirectory(src/ItemEditor.Plugins)
add_subdirectory(plugins)

# Add tests if enabled
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(src/ItemEditor.Tests)
endif()

# Platform-specific IDE configurations
if(WIN32)
    # Set startup project for Visual Studio
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ItemEditor)
endif()

# Add test plugin loading executable
add_executable(TestPluginLoading test_plugin_loading.cpp)
target_link_libraries(TestPluginLoading Qt6::Core Qt6::Widgets ItemEditor.Plugins)

# Add DAT/SPR parsing test executable
add_executable(TestDatSprParsing test_dat_spr_parsing.cpp)
target_link_libraries(TestDatSprParsing Qt6::Core Qt6::Widgets ItemEditor.Plugins)

# Add sprite processing test executable
add_executable(TestSpriteProcessing test_sprite_processing.cpp)
target_link_libraries(TestSpriteProcessing Qt6::Core Qt6::Widgets ItemEditor.Plugins)

# Add Plugin Two specific test executable
add_executable(TestPluginTwo test_plugin_two.cpp)
target_link_libraries(TestPluginTwo Qt6::Core Qt6::Widgets ItemEditor.Plugins)

# Add item editing workflow test executable
add_executable(TestItemEditingWorkflow test_item_editing_workflow.cpp)
target_link_libraries(TestItemEditingWorkflow Qt6::Core Qt6::Widgets ItemEditor.Core ItemEditor.UI)

# Add item editing core test executable (no UI dependencies)
add_executable(TestItemEditingCore test_item_editing_core.cpp)
target_link_libraries(TestItemEditingCore Qt6::Core ItemEditor.Core)

# Add item comparison test executable
add_executable(TestItemComparison test_item_comparison.cpp)
target_link_libraries(TestItemComparison Qt6::Core ItemEditor.Core)

# Add item creation and duplication test executable
add_executable(TestItemCreationDuplication test_item_creation_duplication.cpp)
target_link_libraries(TestItemCreationDuplication Qt6::Core ItemEditor.Core)

# Add Plugin Two sprite processing test executable
add_executable(TestPluginTwoSpriteProcessing test_plugin_two_sprite_processing.cpp)
target_link_libraries(TestPluginTwoSpriteProcessing Qt6::Core Qt6::Widgets ItemEditor.Plugins)

# Add Plugin Two simple sprite test executable
add_executable(TestPluginTwoSpriteSimple test_plugin_two_sprite_simple.cpp)
target_link_libraries(TestPluginTwoSpriteSimple Qt6::Core Qt6::Widgets ItemEditor.Plugins)

# Add Plugin Two minimal test executable
add_executable(TestPluginTwoMinimal test_plugin_two_minimal.cpp)
target_link_libraries(TestPluginTwoMinimal Qt6::Core Qt6::Widgets ItemEditor.Plugins)

# Add Plugin Three minimal test executable
add_executable(TestPluginThreeMinimal test_plugin_three_minimal.cpp)
target_link_libraries(TestPluginThreeMinimal Qt6::Core Qt6::Widgets ItemEditor.Plugins)

# Add Property Editor test executable
add_executable(TestPropertyEditor test_property_editor.cpp)
target_link_libraries(TestPropertyEditor Qt6::Core Qt6::Widgets ItemEditor.Core ItemEditor.UI)

# Add Property Editor minimal test executable
add_executable(TestPropertyEditorMinimal test_property_editor_minimal.cpp)
target_link_libraries(TestPropertyEditorMinimal Qt6::Core Qt6::Widgets ItemEditor.Core ItemEditor.UI)

# Add Property Editor standalone test executable
add_executable(TestPropertyEditorStandalone test_property_editor_standalone.cpp)
target_link_libraries(TestPropertyEditorStandalone Qt6::Core Qt6::Widgets ItemEditor.Core)

# Add File Operations test executable
add_executable(TestFileOperations test_file_operations.cpp)
target_link_libraries(TestFileOperations Qt6::Core Qt6::Widgets ItemEditor.Core)

# Add Plugin One integration test executable
add_executable(TestPluginOneIntegration test_plugin_one_integration.cpp)
target_link_libraries(TestPluginOneIntegration Qt6::Core Qt6::Widgets ItemEditor.Plugins)