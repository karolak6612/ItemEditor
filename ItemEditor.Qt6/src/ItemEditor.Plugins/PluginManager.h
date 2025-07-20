#pragma once

#include <QObject>
#include <QList>
#include <QPluginLoader>
#include <QDir>
#include <QHash>
#include "IPlugin.h"

/**
 * @brief Manages plugin loading and lifecycle
 * 
 * Provides plugin discovery, loading, and management functionality
 * using Qt's native plugin system. Maintains compatibility with
 * the legacy plugin architecture.
 */
class PluginManager : public QObject
{
    Q_OBJECT
    
public:
    explicit PluginManager(QObject* parent = nullptr);
    ~PluginManager();
    
    /**
     * @brief Initialize plugin manager and discover plugins
     * @param pluginDirectory Directory to search for plugins
     * @return true if initialization successful
     */
    bool initialize(const QString& pluginDirectory = QString());
    
    /**
     * @brief Get list of available plugins
     * @return List of plugin interfaces
     */
    QList<IPlugin*> getAvailablePlugins() const;
    
    /**
     * @brief Get plugin by name
     * @param name Plugin name
     * @return Plugin interface or nullptr if not found
     */
    IPlugin* getPlugin(const QString& name) const;
    
    /**
     * @brief Get plugin supporting specific client version
     * @param version Client version string
     * @return Plugin interface or nullptr if not found
     */
    IPlugin* getPluginForVersion(const QString& version) const;
    
    /**
     * @brief Check if any plugins are loaded
     * @return true if plugins are available
     */
    bool hasPlugins() const;
    
    /**
     * @brief Get plugin count
     * @return Number of loaded plugins
     */
    int getPluginCount() const;
    
    /**
     * @brief Reload all plugins
     * @return true if reload successful
     */
    bool reloadPlugins();
    
    /**
     * @brief Cleanup all plugins
     */
    void cleanup();
    
    /**
     * @brief Validate plugin compatibility
     * @param plugin Plugin to validate
     * @return true if plugin is valid and compatible
     */
    bool validatePlugin(IPlugin* plugin) const;
    
    /**
     * @brief Check plugin version compatibility
     * @param pluginVersion Plugin version string
     * @return true if version is compatible
     */
    bool isVersionCompatible(const QString& pluginVersion) const;
    
    /**
     * @brief Get detailed plugin information
     * @param plugin Plugin to get information for
     * @return Plugin information as formatted string
     */
    QString getPluginInfo(IPlugin* plugin) const;
    
    /**
     * @brief Get all supported client versions from all plugins
     * @return List of all supported client versions
     */
    QStringList getAllSupportedVersions() const;
    
    /**
     * @brief Check if a specific client version is supported
     * @param version Client version to check
     * @return true if version is supported by any plugin
     */
    bool isClientVersionSupported(const QString& version) const;
    
    /**
     * @brief Get plugin loading errors
     * @return List of error messages from plugin loading
     */
    QStringList getLoadingErrors() const;
    
    /**
     * @brief Get detailed plugin statistics
     * @return Plugin statistics as formatted string
     */
    QString getPluginStatistics() const;
    
    /**
     * @brief Validate all loaded plugins
     * @return true if all plugins are valid and functional
     */
    bool validateAllPlugins() const;
    
    /**
     * @brief Get plugin by supported version range
     * @param minVersion Minimum supported version
     * @param maxVersion Maximum supported version
     * @return List of plugins supporting the version range
     */
    QList<IPlugin*> getPluginsForVersionRange(const QString& minVersion, const QString& maxVersion) const;
    
    /**
     * @brief Check plugin health status
     * @param plugin Plugin to check
     * @return Health status information
     */
    QString getPluginHealthStatus(IPlugin* plugin) const;

signals:
    /**
     * @brief Emitted when plugin loading progress changes
     * @param progress Progress percentage (0-100)
     * @param message Status message
     */
    void loadingProgress(int progress, const QString& message);
    
    /**
     * @brief Emitted when plugin loading completes
     * @param pluginCount Number of plugins loaded
     */
    void pluginsLoaded(int pluginCount);
    
    /**
     * @brief Emitted when an error occurs
     * @param error Error message
     */
    void errorOccurred(const QString& error);

private slots:
    void onPluginError(const QString& error);

private:
    void discoverPlugins(const QString& directory);
    bool loadPlugin(const QString& filePath);
    void unloadAllPlugins();
    
    QList<QPluginLoader*> m_pluginLoaders;
    QList<IPlugin*> m_plugins;
    QHash<QString, IPlugin*> m_pluginsByName;
    QString m_pluginDirectory;
    QStringList m_loadingErrors;
};