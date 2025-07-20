# Plugin Two - Client versions 8.60-9.86
add_library(PluginTwo SHARED
    PluginTwo.cpp
    PluginTwo.h
    DatParserV9.cpp
    DatParserV9.h
    SprParserV9.cpp
    SprParserV9.h
    plugintwo.json
)

target_link_libraries(PluginTwo 
    Qt6::Core
    ItemEditor.Core
    ItemEditor.Plugins
)

# Set plugin properties
set_target_properties(PluginTwo PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)

# Platform-specific plugin configuration
if(UNIX AND NOT APPLE)
    # Linux-specific plugin settings
    set_target_properties(PluginTwo PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        INSTALL_RPATH_USE_LINK_PATH TRUE
        INSTALL_RPATH "\$ORIGIN/../lib"
    )
    
    # Link math library on Linux
    target_link_libraries(PluginTwo m)
endif()

# Install plugin
install(TARGETS PluginTwo
    LIBRARY DESTINATION plugins
    RUNTIME DESTINATION plugins
)