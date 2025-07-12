#include "plugins/iplugin.h"
#include "plugins/plugindiscovery.h"
#include "plugins/pluginloader.h"
#include "plugins/versionmanager.h"
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include <QMutexLocker>
#include <QTimer>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

namespace PluginInterface {

PluginManager::PluginManager(QObject* parent)
    : QObject(parent)
    , IPluginHost()
    , m_applicationVersion("1.0.0")
    , m_applicationDirectory(QCoreApplication::applicationDirPath())
    , m_pluginsDirectory(QDir(m_applicationDirectory).absoluteFilePath("plugins"))
    , m_tempDirectory(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
{
    // Initialize subsystems
    m_discovery = new PluginDiscovery(this);
    m_loader = new PluginLoader(this);
    m_versionManager = new VersionManager(this);
    
    // Set up plugin loader with this as host
    m_loader->setPluginHost(this);
    
    // Connect signals
    connect(m_discovery, &PluginDiscovery::pluginFound, this, &PluginManager::onPluginDiscovered);
    connect(m_discovery, &PluginDiscovery::errorOccurred, this, &PluginManager::onDiscoveryError);
    
    connect(m_loader, &PluginLoader::pluginLoadFinished, this, &PluginManager::onPluginLoadFinished);
    connect(m_loader, &PluginLoader::pluginInitialized, this, &PluginManager::onPluginInitialized);
    connect(m_loader, &PluginLoader::pluginUnloaded, this, &PluginManager::onPluginUnloaded);
    connect(m_loader, &PluginLoader::errorOccurred, this, &PluginManager::onLoaderError);
    
    connect(m_versionManager, &VersionManager::versionDetected, this, &PluginManager::onVersionDetected);
    
    // Initialize default configuration
    initializeDefaultConfiguration();
    
    logMessage("PluginManager initialized", 1);
}

PluginManager::~PluginManager()
{
    unloadAllPlugins();
    logMessage("PluginManager destroyed", 1);
}

void PluginManager::loadPlugins(const QString& pluginsPath)
{
    logMessage(QString("Loading plugins from: %1").arg(pluginsPath), 1);
    
    if (pluginsPath.isEmpty()) {
        logError("Plugin path is empty");
        return;
    }
    
    QDir pluginDir(pluginsPath);
    if (!pluginDir.exists()) {
        logError(QString("Plugin directory does not exist: %1").arg(pluginsPath));
        return;
    }
    
    // Update plugins directory
    setPluginsDirectory(pluginsPath);
    
    // Discover plugins
    DiscoveryConfig discoveryConfig;
    discoveryConfig.validatePlugins = true;
    discoveryConfig.loadMetadata = true;
    discoveryConfig.checkDependencies = true;
    
    QList<DiscoveredPlugin> discoveredPlugins = m_discovery->scanDirectory(pluginsPath, discoveryConfig);
    
    if (discoveredPlugins.isEmpty()) {
        logWarning("No plugins found in the specified directory");
        return;
    }
    
    logMessage(QString("Found %1 plugins").arg(discoveredPlugins.size()), 1);
    
    // Filter compatible plugins
    QList<DiscoveredPlugin> compatiblePlugins;
    for (const DiscoveredPlugin& plugin : discoveredPlugins) {
        if (plugin.isValid && plugin.isCompatible) {
            compatiblePlugins.append(plugin);
        } else {
            logWarning(QString("Skipping incompatible plugin: %1 - %2")
                      .arg(plugin.fileName, plugin.errorMessage));
        }
    }
    
    if (compatiblePlugins.isEmpty()) {
        logWarning("No compatible plugins found");
        return;
    }
    
    // Load compatible plugins
    LoadConfig loadConfig;
    loadConfig.initializeAfterLoad = true;
    loadConfig.validateInterface = true;
    loadConfig.checkDependencies = true;
    
    QList<LoadResult> loadResults = m_loader->loadPlugins(compatiblePlugins, loadConfig);
    
    // Process load results
    int successCount = 0;
    for (const LoadResult& result : loadResults) {
        if (result.success && result.plugin) {
            registerLoadedPlugin(result.plugin, result.filePath);
            successCount++;
        } else {
            logError(QString("Failed to load plugin %1: %2")
                    .arg(QFileInfo(result.filePath).fileName(), result.errorMessage));
        }
    }
    
    logMessage(QString("Successfully loaded %1 out of %2 plugins").arg(successCount).arg(loadResults.size()), 1);
    rebuildLoadedPluginsCache();
}

void PluginManager::loadPlugin(const QString& pluginPath)
{
    logMessage(QString("Loading single plugin: %1").arg(pluginPath), 1);
    
    if (!QFile::exists(pluginPath)) {
        logError(QString("Plugin file does not exist: %1").arg(pluginPath));
        return;
    }
    
    // Analyze the plugin first
    DiscoveryConfig discoveryConfig;
    discoveryConfig.validatePlugins = true;
    discoveryConfig.loadMetadata = true;
    
    DiscoveredPlugin discoveredPlugin = m_discovery->analyzePlugin(pluginPath, discoveryConfig);
    
    if (!discoveredPlugin.isValid) {
        logError(QString("Plugin analysis failed: %1 - %2")
                .arg(QFileInfo(pluginPath).fileName(), discoveredPlugin.errorMessage));
        return;
    }
    
    if (!discoveredPlugin.isCompatible) {
        logWarning(QString("Plugin is not compatible: %1").arg(discoveredPlugin.fileName));
        return;
    }
    
    // Load the plugin
    LoadConfig loadConfig;
    loadConfig.initializeAfterLoad = true;
    loadConfig.validateInterface = true;
    
    LoadResult loadResult = m_loader->loadPlugin(discoveredPlugin, loadConfig);
    
    if (loadResult.success && loadResult.plugin) {
        registerLoadedPlugin(loadResult.plugin, loadResult.filePath);
        logMessage(QString("Successfully loaded plugin: %1").arg(loadResult.plugin->pluginName()), 1);
    } else {
        logError(QString("Failed to load plugin: %1").arg(loadResult.errorMessage));
    }
    
    rebuildLoadedPluginsCache();
}

void PluginManager::unloadPlugin(const QString& pluginName)
{
    logMessage(QString("Unloading plugin: %1").arg(pluginName), 1);
    
    IPlugin* plugin = findPlugin(pluginName);
    if (!plugin) {
        logWarning(QString("Plugin not found: %1").arg(pluginName));
        return;
    }
    
    // Find the plugin path
    QString pluginPath;
    for (auto it = m_pluginPaths.begin(); it != m_pluginPaths.end(); ++it) {
        if (it.value() == plugin) {
            pluginPath = it.key();
            break;
        }
    }
    
    if (pluginPath.isEmpty()) {
        logError(QString("Cannot find path for plugin: %1").arg(pluginName));
        return;
    }
    
    // Dispose the plugin
    try {
        plugin->Dispose();
    } catch (const std::exception& e) {
        logWarning(QString("Exception during plugin disposal: %1").arg(e.what()));
    }
    
    // Unload using the loader
    bool success = m_loader->unloadPlugin(pluginPath);
    if (success) {
        unregisterLoadedPlugin(plugin);
        logMessage(QString("Successfully unloaded plugin: %1").arg(pluginName), 1);
    } else {
        logError(QString("Failed to unload plugin: %1").arg(pluginName));
    }
    
    rebuildLoadedPluginsCache();
}

void PluginManager::unloadAllPlugins()
{
    logMessage("Unloading all plugins", 1);
    
    // Dispose all plugins first
    for (IPlugin* plugin : m_loadedPluginsCache) {
        if (plugin) {
            try {
                plugin->Dispose();
            } catch (const std::exception& e) {
                logWarning(QString("Exception during plugin disposal: %1").arg(e.what()));
            }
        }
    }
    
    // Unload all plugins using the loader
    m_loader->unloadAllPlugins();
    
    // Clear our tracking
    m_pluginPaths.clear();
    m_pluginMetadata.clear();
    m_staticPlugins.clear();
    m_loadedPluginsCache.clear();
    
    logMessage("All plugins unloaded", 1);
}

void PluginManager::registerStaticPlugin(IPlugin* plugin)
{
    if (!plugin) {
        logError("Cannot register null static plugin");
        return;
    }
    
    if (m_staticPlugins.contains(plugin)) {
        logWarning(QString("Static plugin already registered: %1").arg(plugin->pluginName()));
        return;
    }
    
    if (!validatePlugin(plugin)) {
        logError(QString("Static plugin validation failed: %1").arg(plugin->pluginName()));
        return;
    }
    
    m_staticPlugins.append(plugin);
    
    // Initialize the plugin
    plugin->setHost(this);
    if (plugin->Initialize()) {
        logMessage(QString("Static plugin registered and initialized: %1").arg(plugin->pluginName()), 1);
        emit pluginLoaded(plugin->pluginName());
    } else {
        logError(QString("Static plugin initialization failed: %1").arg(plugin->pluginName()));
        m_staticPlugins.removeAll(plugin);
    }
    
    rebuildLoadedPluginsCache();
}

QList<IPlugin*> PluginManager::getAvailablePlugins() const
{
    return m_loadedPluginsCache;
}

QList<IPlugin*> PluginManager::getLoadedPlugins() const
{
    return m_loadedPluginsCache;
}

IPlugin* PluginManager::findPlugin(const QString& pluginName) const
{
    for (IPlugin* plugin : m_loadedPluginsCache) {
        if (plugin && plugin->pluginName() == pluginName) {
            return plugin;
        }
    }
    return nullptr;
}

IPlugin* PluginManager::findPluginForOtbVersion(quint32 otbVersion) const
{
    for (IPlugin* plugin : m_loadedPluginsCache) {
        if (!plugin) continue;
        
        QList<ItemEditor::SupportedClient> clients = plugin->getSupportedClients();
        for (const ItemEditor::SupportedClient& client : clients) {
            if (client.otbVersion == otbVersion) {
                return plugin;
            }
        }
    }
    return nullptr;
}

IPlugin* PluginManager::findPluginForClientVersion(quint32 clientVersion) const
{
    for (IPlugin* plugin : m_loadedPluginsCache) {
        if (!plugin) continue;
        
        QList<ItemEditor::SupportedClient> clients = plugin->getSupportedClients();
        for (const ItemEditor::SupportedClient& client : clients) {
            if (client.version == clientVersion) {
                return plugin;
            }
        }
    }
    return nullptr;
}

IPlugin* PluginManager::findPluginBySignatures(quint32 datSignature, quint32 sprSignature) const
{
    for (IPlugin* plugin : m_loadedPluginsCache) {
        if (!plugin) continue;
        
        QList<ItemEditor::SupportedClient> clients = plugin->getSupportedClients();
        for (const ItemEditor::SupportedClient& client : clients) {
            if (client.datSignature == datSignature && client.sprSignature == sprSignature) {
                return plugin;
            }
        }
    }
    return nullptr;
}

QList<PluginMetadata> PluginManager::getPluginMetadata() const
{
    return m_pluginMetadata.values();
}

PluginMetadata PluginManager::getPluginMetadata(const QString& pluginName) const
{
    for (auto it = m_pluginMetadata.begin(); it != m_pluginMetadata.end(); ++it) {
        if (it.value().name == pluginName) {
            return it.value();
        }
    }
    return PluginMetadata();
}

// IPluginHost implementation

QString PluginManager::getApplicationVersion() const
{
    return m_applicationVersion;
}

QString PluginManager::getApplicationDirectory() const
{
    return m_applicationDirectory;
}

QString PluginManager::getPluginsDirectory() const
{
    return m_pluginsDirectory;
}

QString PluginManager::getTempDirectory() const
{
    return m_tempDirectory;
}

void PluginManager::logMessage(const QString& message, int level) const
{
    m_logMessages.append(QString("[%1] %2").arg(level).arg(message));
    emit logMessageEmitted(message, level);
    
    // Also log to Qt's logging system
    switch (level) {
        case 0: qDebug() << "PluginManager:" << message; break;
        case 1: qInfo() << "PluginManager:" << message; break;
        case 2: qWarning() << "PluginManager:" << message; break;
        case 3: qCritical() << "PluginManager:" << message; break;
        default: qDebug() << "PluginManager:" << message; break;
    }
}

void PluginManager::logError(const QString& error) const
{
    logMessage(QString("ERROR: %1").arg(error), 3);
}

void PluginManager::logWarning(const QString& warning) const
{
    logMessage(QString("WARNING: %1").arg(warning), 2);
}

void PluginManager::logDebug(const QString& debug) const
{
    logMessage(QString("DEBUG: %1").arg(debug), 0);
}

void PluginManager::reportProgress(int percentage, const QString& status) const
{
    emit progressChanged(percentage, status);
}

void PluginManager::setProgressVisible(bool visible) const
{
    Q_UNUSED(visible)
    // This could be implemented to show/hide a progress dialog
}

QVariant PluginManager::getConfigValue(const QString& key, const QVariant& defaultValue) const
{
    QSettings settings;
    return settings.value(QString("PluginManager/%1").arg(key), defaultValue);
}

void PluginManager::setConfigValue(const QString& key, const QVariant& value) const
{
    QSettings settings;
    settings.setValue(QString("PluginManager/%1").arg(key), value);
}

QByteArray PluginManager::loadResource(const QString& resourcePath) const
{
    QFile file(resourcePath);
    if (file.open(QIODevice::ReadOnly)) {
        return file.readAll();
    }
    
    logError(QString("Failed to load resource: %1").arg(resourcePath));
    return QByteArray();
}

bool PluginManager::saveResource(const QString& resourcePath, const QByteArray& data) const
{
    QFile file(resourcePath);
    if (file.open(QIODevice::WriteOnly)) {
        qint64 written = file.write(data);
        return written == data.size();
    }
    
    logError(QString("Failed to save resource: %1").arg(resourcePath));
    return false;
}

bool PluginManager::sendMessage(const QString& targetPlugin, const QString& message, const QVariant& data) const
{
    IPlugin* plugin = findPlugin(targetPlugin);
    if (!plugin) {
        logError(QString("Target plugin not found: %1").arg(targetPlugin));
        return false;
    }
    
    // This is a placeholder for inter-plugin communication
    // Real implementation would depend on how plugins handle messages
    Q_UNUSED(message)
    Q_UNUSED(data)
    
    logDebug(QString("Message sent to plugin %1: %2").arg(targetPlugin, message));
    return true;
}

// Configuration methods

void PluginManager::setApplicationVersion(const QString& version)
{
    m_applicationVersion = version;
    logDebug(QString("Application version set to: %1").arg(version));
}

void PluginManager::setApplicationDirectory(const QString& directory)
{
    m_applicationDirectory = directory;
    logDebug(QString("Application directory set to: %1").arg(directory));
}

void PluginManager::setPluginsDirectory(const QString& directory)
{
    m_pluginsDirectory = directory;
    logDebug(QString("Plugins directory set to: %1").arg(directory));
}

void PluginManager::setTempDirectory(const QString& directory)
{
    m_tempDirectory = directory;
    logDebug(QString("Temp directory set to: %1").arg(directory));
}

// Public slots

void PluginManager::refreshPlugins()
{
    logMessage("Refreshing plugins", 1);
    
    // Unload all current plugins
    unloadAllPlugins();
    
    // Reload plugins from the plugins directory
    loadPlugins(m_pluginsDirectory);
    
    logMessage("Plugin refresh completed", 1);
}

// Private slots

void PluginManager::onPluginDestroyed(QObject* plugin)
{
    IPlugin* iplugin = qobject_cast<IPlugin*>(plugin);
    if (iplugin) {
        unregisterLoadedPlugin(iplugin);
        rebuildLoadedPluginsCache();
        logMessage(QString("Plugin destroyed: %1").arg(iplugin->pluginName()), 1);
    }
}

void PluginManager::onPluginDiscovered(const QString& filePath)
{
    logDebug(QString("Plugin discovered: %1").arg(QFileInfo(filePath).fileName()));
}

void PluginManager::onDiscoveryError(const QString& error)
{
    logError(QString("Plugin discovery error: %1").arg(error));
}

void PluginManager::onPluginLoadFinished(const QString& filePath, bool success)
{
    QString fileName = QFileInfo(filePath).fileName();
    if (success) {
        logMessage(QString("Plugin loaded: %1").arg(fileName), 1);
    } else {
        logError(QString("Plugin load failed: %1").arg(fileName));
    }
}

void PluginManager::onPluginInitialized(const QString& pluginName, bool success)
{
    if (success) {
        logMessage(QString("Plugin initialized: %1").arg(pluginName), 1);
        emit pluginLoaded(pluginName);
    } else {
        logError(QString("Plugin initialization failed: %1").arg(pluginName));
        emit pluginError(pluginName, "Initialization failed");
    }
}

void PluginManager::onPluginUnloaded(const QString& filePath)
{
    QString fileName = QFileInfo(filePath).fileName();
    logMessage(QString("Plugin unloaded: %1").arg(fileName), 1);
}

void PluginManager::onLoaderError(const QString& error)
{
    logError(QString("Plugin loader error: %1").arg(error));
}

void PluginManager::onVersionDetected(const VersionDetectionResult& result)
{
    if (result.success) {
        logMessage(QString("Version detected: %1 (confidence: %2)")
                  .arg(result.detectedVersion.displayName)
                  .arg(result.confidence), 1);
    }
}

// Private implementation methods

void PluginManager::initializeDefaultConfiguration()
{
    // Set default configuration values if not already set
    if (getConfigValue("AutoLoadPlugins", true).toBool()) {
        // Auto-load plugins on startup if enabled
        QTimer::singleShot(100, this, [this]() {
            if (QDir(m_pluginsDirectory).exists()) {
                loadPlugins(m_pluginsDirectory);
            }
        });
    }
}

void PluginManager::registerLoadedPlugin(IPlugin* plugin, const QString& filePath)
{
    if (!plugin) return;
    
    m_pluginPaths[filePath] = plugin;
    
    // Load metadata if available
    PluginMetadata metadata = loadPluginMetadata(filePath);
    if (!metadata.name.isEmpty()) {
        m_pluginMetadata[filePath] = metadata;
    } else {
        // Create basic metadata from plugin
        metadata.name = plugin->pluginName();
        metadata.description = plugin->pluginDescription();
        metadata.version = plugin->pluginVersion();
        m_pluginMetadata[filePath] = metadata;
    }
    
    // Connect to plugin destruction
    connect(plugin, &QObject::destroyed, this, &PluginManager::onPluginDestroyed);
}

void PluginManager::unregisterLoadedPlugin(IPlugin* plugin)
{
    if (!plugin) return;
    
    // Find and remove from plugin paths
    for (auto it = m_pluginPaths.begin(); it != m_pluginPaths.end(); ++it) {
        if (it.value() == plugin) {
            QString filePath = it.key();
            m_pluginPaths.erase(it);
            m_pluginMetadata.remove(filePath);
            break;
        }
    }
    
    // Remove from static plugins if present
    m_staticPlugins.removeAll(plugin);
    
    emit pluginUnloaded(plugin->pluginName());
}

void PluginManager::rebuildLoadedPluginsCache()
{
    m_loadedPluginsCache.clear();
    
    // Add plugins from plugin paths (dynamic plugins)
    for (IPlugin* plugin : m_pluginPaths.values()) {
        if (plugin && !m_loadedPluginsCache.contains(plugin)) {
            m_loadedPluginsCache.append(plugin);
        }
    }
    
    // Add static plugins
    for (IPlugin* plugin : m_staticPlugins) {
        if (plugin && !m_loadedPluginsCache.contains(plugin)) {
            m_loadedPluginsCache.append(plugin);
        }
    }
    
    logDebug(QString("Plugin cache rebuilt: %1 plugins").arg(m_loadedPluginsCache.size()));
}

bool PluginManager::validatePlugin(IPlugin* plugin) const
{
    if (!plugin) {
        return false;
    }
    
    // Check plugin name and version
    if (plugin->pluginName().isEmpty() || plugin->pluginVersion().isEmpty()) {
        logError("Plugin has empty name or version");
        return false;
    }
    
    // Check supported clients
    const QList<ItemEditor::SupportedClient> clients = plugin->getSupportedClients();
    if (clients.isEmpty()) {
        logWarning(QString("Plugin supports no clients: %1").arg(plugin->pluginName()));
        return false;
    }
    
    // Validate client data
    for (const ItemEditor::SupportedClient& client : clients) {
        if (client.version == 0 || client.otbVersion == 0) {
            logError(QString("Plugin has invalid client data: %1").arg(plugin->pluginName()));
            return false;
        }
    }
    
    return true;
}

PluginMetadata PluginManager::loadPluginMetadata(const QString& pluginPath) const
{
    return m_discovery->loadPluginMetadata(pluginPath);
}

void PluginManager::cleanupPlugin(IPlugin* plugin)
{
    if (!plugin) return;
    
    try {
        plugin->Dispose();
    } catch (const std::exception& e) {
        logWarning(QString("Exception during plugin cleanup: %1").arg(e.what()));
    }
}

} // namespace PluginInterface