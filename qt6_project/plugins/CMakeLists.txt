# ItemEditor Plugins
# Exact mirror of Legacy_App/csharp/Source/ plugin projects

# Add plugin subdirectories
add_subdirectory(PluginOne)
add_subdirectory(PluginTwo)
add_subdirectory(PluginThree)

# Set global plugin installation directory
set(PLUGIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/plugins" CACHE PATH "Plugin installation directory")

# Create plugins directory during build
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

# Message about plugin location
message(STATUS "Plugins will be installed to: ${PLUGIN_INSTALL_DIR}")
message(STATUS "Plugins will be built to: ${CMAKE_BINARY_DIR}/plugins")