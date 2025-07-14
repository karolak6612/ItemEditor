# ItemEditor PluginInterface Library
# Exact mirror of Legacy_App/csharp/Source/PluginInterface/

# Add OTLib subdirectory first
add_subdirectory(OTLib)

qt_add_library(ItemEditorPluginInterface STATIC
    IPlugin.h
    Item.cpp
    Item.h
    Settings.cpp
    Settings.h
    Sprite.cpp
    Sprite.h
    SpriteManager.cpp
    SpriteManager.h
    SupportedClient.cpp
    SupportedClient.h
)

# Link required Qt6 modules
target_link_libraries(ItemEditorPluginInterface PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Xml
)

# Link to OTLib components
target_link_libraries(ItemEditorPluginInterface PUBLIC
    ItemEditorOTLib
)

# Include directories
target_include_directories(ItemEditorPluginInterface PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Set target properties
set_target_properties(ItemEditorPluginInterface PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Enable Qt MOC for this library
set_target_properties(ItemEditorPluginInterface PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)