# ItemEditor Host Library
# Exact mirror of Legacy_App/csharp/Source/Host/

qt_add_library(ItemEditorHost STATIC
    PluginServices.cpp
    PluginServices.h
    Plugin.cpp
    Plugin.h
    PluginCollection.cpp
    PluginCollection.h
)

# Link required Qt6 modules
target_link_libraries(ItemEditorHost PUBLIC
    Qt6::Core
    Qt6::Widgets
)

# Link to PluginInterface and Helpers
target_link_libraries(ItemEditorHost PUBLIC
    ItemEditorPluginInterface
    ItemEditorHelpers
)

# Include directories
target_include_directories(ItemEditorHost PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Set target properties
set_target_properties(ItemEditorHost PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Enable Qt MOC for this library
set_target_properties(ItemEditorHost PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)