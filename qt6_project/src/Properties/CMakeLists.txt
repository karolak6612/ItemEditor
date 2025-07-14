# ItemEditor Properties Library
# Exact mirror of Legacy_App/csharp/Source/Properties/

qt_add_library(ItemEditorProperties STATIC
    version.cpp
    version.h
    settings.cpp
    settings.h
)

# Add resource file
qt_add_resources(ItemEditorProperties "application_resources"
    PREFIX "/"
    FILES
        application.qrc
)

# Link required Qt6 modules
target_link_libraries(ItemEditorProperties PUBLIC
    Qt6::Core
    Qt6::Widgets
)

# Include directories
target_include_directories(ItemEditorProperties PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Set target properties
set_target_properties(ItemEditorProperties PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Enable Qt MOC for this library (needed for Settings class)
set_target_properties(ItemEditorProperties PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)