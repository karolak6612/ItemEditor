# ItemEditor OTLib SPR Library
# Sprite file format support

qt_add_library(ItemEditorOTLibSPR STATIC
    SpriteReader.h
    SpriteReader.cpp
)

# Link required Qt6 modules
target_link_libraries(ItemEditorOTLibSPR PUBLIC
    Qt6::Core
    Qt6::Gui
)

# Include directories
target_include_directories(ItemEditorOTLibSPR PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Set target properties
set_target_properties(ItemEditorOTLibSPR PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Enable Qt MOC for this library
set_target_properties(ItemEditorOTLibSPR PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)