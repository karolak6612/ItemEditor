# ItemEditor OTLib Collections Library
# Exact mirror of Legacy_App/csharp/Source/PluginInterface/OTLib/Collections/

qt_add_library(ItemEditorOTLibCollections STATIC
    ServerItemList.cpp
    ServerItemList.h
)

# Link required Qt6 modules
target_link_libraries(ItemEditorOTLibCollections PUBLIC
    Qt6::Core
)

# Include directories
target_include_directories(ItemEditorOTLibCollections PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Set target properties
set_target_properties(ItemEditorOTLibCollections PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Enable Qt MOC for this library
set_target_properties(ItemEditorOTLibCollections PROPERTIES
    AUTOMOC ON
)