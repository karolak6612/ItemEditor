# Plugin One - Client versions 8.00-8.57
add_library(PluginOne SHARED
    PluginOne.cpp
    PluginOne.h
    DatParserV8.cpp
    DatParserV8.h
    SprParserV8.cpp
    SprParserV8.h
    pluginone.json
)

target_link_libraries(PluginOne 
    Qt6::Core
    ItemEditor.Core
    ItemEditor.Plugins
)

# Set plugin properties
set_target_properties(PluginOne PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)

# Platform-specific plugin configuration
if(UNIX AND NOT APPLE)
    # Linux-specific plugin settings
    set_target_properties(PluginOne PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        INSTALL_RPATH_USE_LINK_PATH TRUE
        INSTALL_RPATH "\$ORIGIN/../lib"
    )
    
    # Link math library on Linux
    target_link_libraries(PluginOne m)
endif()

# Install plugin
install(TARGETS PluginOne
    LIBRARY DESTINATION plugins
    RUNTIME DESTINATION plugins
)