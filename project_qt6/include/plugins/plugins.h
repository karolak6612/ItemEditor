#ifndef PLUGINS_H
#define PLUGINS_H

/**
 * @file plugins.h
 * @brief Comprehensive plugin system for ItemEditor Qt6
 * 
 * This header provides a complete plugin interface system that matches
 * the functionality of the C# PluginInterface.cs while being adapted
 * for Qt6 and C++ best practices.
 * 
 * Key Components:
 * - IPlugin: Main plugin interface matching C# IPlugin
 * - IPluginHost: Host interface for plugin-application communication
 * - ClientItems: Collection class for managing client items
 * - PluginManager: Manages plugin lifecycle and discovery
 * - BasePlugin: Base implementation for easier plugin development
 * - PluginFactory: Factory for creating and registering plugin types
 * 
 * Usage:
 * 1. Include this header in your plugin implementations
 * 2. Inherit from BasePlugin for easier development
 * 3. Use PluginManager to load and manage plugins
 * 4. Register plugin types with PluginFactory
 * 
 * @author ItemEditor Development Team
 * @version 1.0.0
 */

// Core plugin interfaces
#include "plugins/iplugin.h"

// Base implementation for plugin development
#include "plugins/baseplugin.h"

// Plugin factory and discovery
#include "plugins/pluginfactory.h"

// Example plugin implementation
#include "plugins/exampleplugin.h"

namespace PluginInterface {

/**
 * @brief Plugin system version information
 */
struct PluginSystemInfo {
    static constexpr const char* VERSION = "1.0.0";
    static constexpr const char* API_VERSION = "1.0";
    static constexpr int API_VERSION_NUMBER = 1;
    static constexpr const char* COMPATIBLE_ITEMEDITOR_VERSION = "2.0.0";
};

/**
 * @brief Initialize the plugin system
 * 
 * Call this function once at application startup to initialize
 * the plugin system and register built-in plugin types.
 */
void initializePluginSystem();

/**
 * @brief Shutdown the plugin system
 * 
 * Call this function at application shutdown to properly cleanup
 * all loaded plugins and release resources.
 */
void shutdownPluginSystem();

/**
 * @brief Get plugin system information
 */
PluginSystemInfo getPluginSystemInfo();

/**
 * @brief Validate plugin compatibility
 * 
 * @param plugin The plugin to validate
 * @return true if the plugin is compatible with the current system
 */
bool validatePluginCompatibility(IPlugin* plugin);

/**
 * @brief Get global plugin manager instance
 * 
 * @return Pointer to the global plugin manager
 */
PluginManager* getGlobalPluginManager();

} // namespace PluginInterface

#endif // PLUGINS_H