# Plugin Three - Client versions 10.00-10.77
add_library(PluginThree SHARED
    PluginThree.cpp
    PluginThree.h
    DatParserV10.cpp
    DatParserV10.h
    SprParserV10.cpp
    SprParserV10.h
    pluginthree.json
)

target_link_libraries(PluginThree 
    Qt6::Core
    ItemEditor.Core
    ItemEditor.Plugins
)

# Set plugin properties
set_target_properties(PluginThree PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)

# Platform-specific plugin configuration
if(UNIX AND NOT APPLE)
    # Linux-specific plugin settings
    set_target_properties(PluginThree PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        INSTALL_RPATH_USE_LINK_PATH TRUE
        INSTALL_RPATH "\$ORIGIN/../lib"
    )
    
    # Link math library on Linux
    target_link_libraries(PluginThree m)
endif()

# Install plugin
install(TARGETS PluginThree
    LIBRARY DESTINATION plugins
    RUNTIME DESTINATION plugins
)