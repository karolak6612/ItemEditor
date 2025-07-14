# ItemEditor OTLib Utils Library
# Exact mirror of Legacy_App/csharp/Source/PluginInterface/OTLib/Utils/

qt_add_library(ItemEditorOTLibUtils STATIC
    BinaryTreeReader.cpp
    BinaryTreeReader.h
    BinaryTreeWriter.cpp
    BinaryTreeWriter.h
    BitmapLocker.cpp
    BitmapLocker.h
    SpecialChar.cpp
    SpecialChar.h
)

# Link required Qt6 modules
target_link_libraries(ItemEditorOTLibUtils PUBLIC
    Qt6::Core
    Qt6::Gui
)

# Include directories
target_include_directories(ItemEditorOTLibUtils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Set target properties
set_target_properties(ItemEditorOTLibUtils PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Enable Qt MOC for this library
set_target_properties(ItemEditorOTLibUtils PROPERTIES
    AUTOMOC ON
)