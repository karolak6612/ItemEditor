# ItemEditor Controls Library
# Exact mirror of Legacy_App/csharp/Source/Controls/

qt_add_library(ItemEditorControls STATIC
    ClientItemView.cpp
    ClientItemView.h
    FlagCheckBox.cpp
    FlagCheckBox.h
    ListBase.cpp
    ListBase.h
    ServerItemListBox.cpp
    ServerItemListBox.h
)

# Link required Qt6 modules
target_link_libraries(ItemEditorControls PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
)

# Link to PluginInterface for data types
target_link_libraries(ItemEditorControls PUBLIC
    ItemEditorPluginInterface   # Now implemented
)

# Include directories
target_include_directories(ItemEditorControls PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../PluginInterface
)

# Set target properties
set_target_properties(ItemEditorControls PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Enable Qt MOC for this library
set_target_properties(ItemEditorControls PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)