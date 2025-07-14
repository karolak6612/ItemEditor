# PluginTwo
# Exact mirror of Legacy_App/csharp/Source/PluginTwo/

qt_add_plugin(PluginTwo)

target_sources(PluginTwo PRIVATE
    Plugin.cpp
    Plugin.h
)

# Link required Qt6 modules
target_link_libraries(PluginTwo PUBLIC
    Qt6::Core
    Qt6::Widgets
)

# Link to ItemEditor PluginInterface and OTLib
target_link_libraries(PluginTwo PUBLIC
    ItemEditorPluginInterface
)

# Include directories
target_include_directories(PluginTwo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src/PluginInterface
)

# Set target properties
set_target_properties(PluginTwo PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)

# Enable Qt MOC for this plugin
set_target_properties(PluginTwo PROPERTIES
    AUTOMOC ON
)

# Copy plugin to application directory after build
add_custom_command(TARGET PluginTwo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:PluginTwo>
        ${CMAKE_BINARY_DIR}/$<CONFIG>/plugins/$<TARGET_FILE_NAME:PluginTwo>
    COMMENT "Copying PluginTwo to application plugins directory"
)

# Ensure plugins directory exists in application directory
add_custom_command(TARGET PluginTwo PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_BINARY_DIR}/$<CONFIG>/plugins
    COMMENT "Creating plugins directory in application directory"
)

# Install plugin to correct location
install(TARGETS PluginTwo
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/ItemEditor/plugins
    BUNDLE DESTINATION ${CMAKE_INSTALL_LIBDIR}/ItemEditor/plugins
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/ItemEditor/plugins
)