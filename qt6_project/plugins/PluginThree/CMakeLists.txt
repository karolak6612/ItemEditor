# PluginThree
# Exact mirror of Legacy_App/csharp/Source/PluginThree/

qt_add_plugin(PluginThree)

target_sources(PluginThree PRIVATE
    Plugin.cpp
    Plugin.h
)

# Link required Qt6 modules
target_link_libraries(PluginThree PUBLIC
    Qt6::Core
    Qt6::Widgets
)

# Link to ItemEditor PluginInterface and OTLib
target_link_libraries(PluginThree PUBLIC
    ItemEditorPluginInterface
)

# Include directories
target_include_directories(PluginThree PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src/PluginInterface
)

# Set target properties
set_target_properties(PluginThree PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)

# Enable Qt MOC for this plugin
set_target_properties(PluginThree PROPERTIES
    AUTOMOC ON
)

# Copy plugin to application directory after build
add_custom_command(TARGET PluginThree POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:PluginThree>
        ${CMAKE_BINARY_DIR}/$<CONFIG>/plugins/$<TARGET_FILE_NAME:PluginThree>
    COMMENT "Copying PluginThree to application plugins directory"
)

# Ensure plugins directory exists in application directory
add_custom_command(TARGET PluginThree PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_BINARY_DIR}/$<CONFIG>/plugins
    COMMENT "Creating plugins directory in application directory"
)

# Install plugin to correct location
install(TARGETS PluginThree
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/ItemEditor/plugins
    BUNDLE DESTINATION ${CMAKE_INSTALL_LIBDIR}/ItemEditor/plugins
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/ItemEditor/plugins
)