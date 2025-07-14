# ItemEditor OTLib Server Library
# Exact mirror of Legacy_App/csharp/Source/PluginInterface/OTLib/Server/

# Add subdirectory
add_subdirectory(Items)

qt_add_library(ItemEditorOTLibServer STATIC
    # Main server files will be added here
)

# Link required Qt6 modules
target_link_libraries(ItemEditorOTLibServer PUBLIC
    Qt6::Core
    Qt6::Xml
)

# Link to Items sublibrary
target_link_libraries(ItemEditorOTLibServer PUBLIC
    ItemEditorOTLibServerItems
)

# Include directories
target_include_directories(ItemEditorOTLibServer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Set target properties
set_target_properties(ItemEditorOTLibServer PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Enable Qt MOC for this library
set_target_properties(ItemEditorOTLibServer PROPERTIES
    AUTOMOC ON
)