# ItemEditor OTLib Server Items Library
# Complete implementation of Legacy_App/csharp/Source/PluginInterface/OTLib/Server/Items/

qt_add_library(ItemEditorOTLibServerItems STATIC
    ItemsXmlReader.cpp
    ItemsXmlReader.h
    ServerItem.cpp
    ServerItem.h
    ServerItemAttribute.cpp
    ServerItemAttribute.h
    ServerItemFlag.cpp
    ServerItemFlag.h
)

# Link required Qt6 modules
target_link_libraries(ItemEditorOTLibServerItems PUBLIC
    Qt6::Core
    Qt6::Xml
)

# Include directories
target_include_directories(ItemEditorOTLibServerItems PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Set target properties
set_target_properties(ItemEditorOTLibServerItems PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Enable Qt MOC for this library
set_target_properties(ItemEditorOTLibServerItems PROPERTIES
    AUTOMOC ON
)