# PluginOne
# Exact mirror of Legacy_App/csharp/Source/PluginOne/

qt_add_plugin(PluginOne)

target_sources(PluginOne PRIVATE
    Plugin.cpp
    Plugin.h
)

# Link required Qt6 modules
target_link_libraries(PluginOne PUBLIC
    Qt6::Core
    Qt6::Widgets
)

# Link to ItemEditor PluginInterface and OTLib
target_link_libraries(PluginOne PUBLIC
    ItemEditorPluginInterface
)

# Include directories
target_include_directories(PluginOne PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src/PluginInterface
)

# Set target properties
set_target_properties(PluginOne PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)

# Enable Qt MOC for this plugin
set_target_properties(PluginOne PROPERTIES
    AUTOMOC ON
)

# Copy plugin to application directory after build
add_custom_command(TARGET PluginOne POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:PluginOne>
        ${CMAKE_BINARY_DIR}/$<CONFIG>/plugins/$<TARGET_FILE_NAME:PluginOne>
    COMMENT "Copying PluginOne to application plugins directory"
)

# Ensure plugins directory exists in application directory
add_custom_command(TARGET PluginOne PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_BINARY_DIR}/$<CONFIG>/plugins
    COMMENT "Creating plugins directory in application directory"
)

# Install plugin to correct location
install(TARGETS PluginOne
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/ItemEditor/plugins
    BUNDLE DESTINATION ${CMAKE_INSTALL_LIBDIR}/ItemEditor/plugins
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/ItemEditor/plugins
)