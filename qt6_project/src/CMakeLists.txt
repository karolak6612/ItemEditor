# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Gui
    Network
    Xml
)

# Main application executable - Linux only
qt_add_executable(ItemEditor
    main.cpp
    MainForm.cpp
    MainForm.h
    MainForm.ui
)

# Add subdirectories for component libraries (only existing ones)
add_subdirectory(Controls)
add_subdirectory(Dialogs)
add_subdirectory(Helpers)
add_subdirectory(Properties)
add_subdirectory(Host)         # Plugin system core components implemented
add_subdirectory(PluginInterface) # Plugin interface components implemented

# Link component libraries to main executable
target_link_libraries(ItemEditor PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Network
    Qt6::Xml
    ItemEditorControls
    ItemEditorDialogs
    ItemEditorHelpers
    ItemEditorProperties
    ItemEditorHost              # Plugin system core components implemented
    ItemEditorPluginInterface   # Plugin interface components implemented
)

# Include directories
target_include_directories(ItemEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Set target properties - Linux only
set_target_properties(ItemEditor PROPERTIES
    OUTPUT_NAME "ItemEditor"
)

# Installation
install(TARGETS ItemEditor
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)