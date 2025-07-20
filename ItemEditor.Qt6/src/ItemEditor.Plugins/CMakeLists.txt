# ItemEditor.Plugins - Plugin interfaces and base classes
add_library(ItemEditor.Plugins STATIC
    # Plugin Interface
    IPlugin.h
    PluginManager.cpp
    PluginManager.h
    
    # Plugin Base Classes
    BasePlugin.cpp
    BasePlugin.h
    
    # Client Data Types
    ClientDataTypes.h
    SpriteData.cpp
    SpriteData.h
    
    # File Parsers Base
    DatParser.h
    SprParser.h
)

target_link_libraries(ItemEditor.Plugins 
    Qt6::Core
    ItemEditor.Core
)

# Platform-specific linking
if(UNIX AND NOT APPLE)
    # Link additional libraries required on Linux
    target_link_libraries(ItemEditor.Plugins 
        ${CMAKE_DL_LIBS}  # For dynamic loading
    )
    
    # Set position independent code for shared libraries
    set_target_properties(ItemEditor.Plugins PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
endif()

# Set include directories
target_include_directories(ItemEditor.Plugins PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)