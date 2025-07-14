# ItemEditor OTLib Library
# Exact mirror of Legacy_App/csharp/Source/PluginInterface/OTLib/

# Add subdirectories
add_subdirectory(Collections)
add_subdirectory(OTB)        # Now enabled
add_subdirectory(Server)
add_subdirectory(SPR)        # Sprite file support
add_subdirectory(Utils)

qt_add_library(ItemEditorOTLib STATIC
    # Main OTLib files will be added here
)

# Link required Qt6 modules
target_link_libraries(ItemEditorOTLib PUBLIC
    Qt6::Core
    Qt6::Network
    Qt6::Xml
)

# Link to OTLib component libraries
target_link_libraries(ItemEditorOTLib PUBLIC
    ItemEditorOTLibCollections
    ItemEditorOTLibOTB         # Now enabled
    ItemEditorOTLibServer
    ItemEditorOTLibSPR         # Sprite file support
    ItemEditorOTLibUtils
)

# Include directories
target_include_directories(ItemEditorOTLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Set target properties
set_target_properties(ItemEditorOTLib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Enable Qt MOC for this library
set_target_properties(ItemEditorOTLib PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)