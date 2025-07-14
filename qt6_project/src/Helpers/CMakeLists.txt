# ItemEditor Helpers Library
# Exact mirror of Legacy_App/csharp/Source/Helpers/

qt_add_library(ItemEditorHelpers STATIC
    FileNameHelper.cpp
    FileNameHelper.h
    MemoryManager.cpp
    MemoryManager.h
    PathHelper.cpp
    PathHelper.h
    Utils.cpp
    Utils.h
)

# Link required Qt6 modules
target_link_libraries(ItemEditorHelpers PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

# Include directories
target_include_directories(ItemEditorHelpers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Set target properties
set_target_properties(ItemEditorHelpers PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Enable Qt MOC for this library
set_target_properties(ItemEditorHelpers PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)