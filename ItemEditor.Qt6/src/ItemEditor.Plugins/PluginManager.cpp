#include "PluginManager.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QLibrary>

PluginManager::PluginManager(QObject* parent)
    : QObject(parent)
{
}

PluginManager::~PluginManager()
{
    cleanup();
}

bool PluginManager::initialize(const QString& pluginDirectory)
{
    // Clear previous errors
    m_loadingErrors.clear();
    
    // Set default plugin directory if not specified
    if (pluginDirectory.isEmpty()) {
        m_pluginDirectory = QCoreApplication::applicationDirPath() + "/plugins";
    } else {
        m_pluginDirectory = pluginDirectory;
    }
    
    qDebug() << "Initializing PluginManager with directory:" << m_pluginDirectory;
    
    // Check if plugin directory exists
    QDir pluginDir(m_pluginDirectory);
    if (!pluginDir.exists()) {
        QString error = QString("Plugin directory does not exist: %1").arg(m_pluginDirectory);
        m_loadingErrors.append(error);
        emit errorOccurred(error);
        return false;
    }
    
    // Discover and load plugins
    discoverPlugins(m_pluginDirectory);
    
    emit pluginsLoaded(m_plugins.size());
    return true;
}

QList<IPlugin*> PluginManager::getAvailablePlugins() const
{
    return m_plugins;
}

IPlugin* PluginManager::getPlugin(const QString& name) const
{
    return m_pluginsByName.value(name, nullptr);
}

IPlugin* PluginManager::getPluginForVersion(const QString& version) const
{
    for (IPlugin* plugin : m_plugins) {
        if (plugin->supportedVersions().contains(version)) {
            return plugin;
        }
    }
    return nullptr;
}

bool PluginManager::hasPlugins() const
{
    return !m_plugins.isEmpty();
}

int PluginManager::getPluginCount() const
{
    return m_plugins.size();
}

bool PluginManager::reloadPlugins()
{
    cleanup();
    return initialize(m_pluginDirectory);
}

void PluginManager::cleanup()
{
    // Cleanup all plugins
    for (IPlugin* plugin : m_plugins) {
        if (plugin) {
            plugin->cleanup();
        }
    }
    
    // Unload all plugin libraries
    unloadAllPlugins();
    
    m_plugins.clear();
    m_pluginsByName.clear();
}

void PluginManager::onPluginError(const QString& error)
{
    emit errorOccurred(error);
}

void PluginManager::discoverPlugins(const QString& directory)
{
    QDir pluginDir(directory);
    
    // Get all dynamic library files
    QStringList filters;
#ifdef Q_OS_WIN
    filters << "*.dll";
#elif defined(Q_OS_MAC)
    filters << "*.dylib";
#else
    filters << "*.so";
#endif
    
    QStringList pluginFiles = pluginDir.entryList(filters, QDir::Files);
    
    emit loadingProgress(0, "Discovering plugins...");
    
    int totalFiles = pluginFiles.size();
    int currentFile = 0;
    
    for (const QString& fileName : pluginFiles) {
        QString filePath = pluginDir.absoluteFilePath(fileName);
        
        emit loadingProgress((currentFile * 100) / totalFiles, 
                           QString("Loading %1...").arg(fileName));
        
        if (!loadPlugin(filePath)) {
            qWarning() << "Failed to load plugin:" << filePath;
        }
        
        currentFile++;
    }
    
    emit loadingProgress(100, QString("Loaded %1 plugins").arg(m_plugins.size()));
}

bool PluginManager::loadPlugin(const QString& filePath)
{
    QPluginLoader* loader = new QPluginLoader(filePath, this);
    
    // Check if plugin can be loaded
    if (!loader->load()) {
        QString error = QString("Failed to load plugin library %1: %2").arg(filePath, loader->errorString());
        m_loadingErrors.append(error);
        qWarning() << error;
        delete loader;
        return false;
    }
    
    // Get plugin instance
    QObject* pluginObject = loader->instance();
    if (!pluginObject) {
        QString error = QString("Failed to get plugin instance from: %1").arg(filePath);
        m_loadingErrors.append(error);
        qWarning() << error;
        loader->unload();
        delete loader;
        return false;
    }
    
    // Cast to IPlugin interface
    IPlugin* plugin = qobject_cast<IPlugin*>(pluginObject);
    if (!plugin) {
        QString error = QString("Plugin does not implement IPlugin interface: %1").arg(filePath);
        m_loadingErrors.append(error);
        qWarning() << error;
        loader->unload();
        delete loader;
        return false;
    }
    
    // Validate plugin compatibility
    if (!validatePlugin(plugin)) {
        QString error = QString("Plugin validation failed: %1").arg(filePath);
        m_loadingErrors.append(error);
        qWarning() << error;
        loader->unload();
        delete loader;
        return false;
    }
    
    // Initialize plugin
    if (!plugin->initialize()) {
        QString error = QString("Failed to initialize plugin: %1").arg(filePath);
        m_loadingErrors.append(error);
        qWarning() << error;
        loader->unload();
        delete loader;
        return false;
    }
    
    // Connect plugin signals
    connect(plugin, &IPlugin::errorOccurred, this, &PluginManager::onPluginError);
    
    // Store plugin references
    m_pluginLoaders.append(loader);
    m_plugins.append(plugin);
    m_pluginsByName.insert(plugin->name(), plugin);
    
    qDebug() << "Successfully loaded plugin:" << plugin->name() << "v" << plugin->version();
    
    return true;
}

void PluginManager::unloadAllPlugins()
{
    for (QPluginLoader* loader : m_pluginLoaders) {
        if (loader) {
            loader->unload();
            delete loader;
        }
    }
    m_pluginLoaders.clear();
}

bool PluginManager::validatePlugin(IPlugin* plugin) const
{
    if (!plugin) {
        return false;
    }
    
    // Check if plugin has a valid name
    if (plugin->name().isEmpty()) {
        qWarning() << "Plugin has empty name";
        return false;
    }
    
    // Check if plugin has a valid version
    if (plugin->version().isEmpty()) {
        qWarning() << "Plugin has empty version";
        return false;
    }
    
    // Check version compatibility
    if (!isVersionCompatible(plugin->version())) {
        qWarning() << "Plugin version not compatible:" << plugin->version();
        return false;
    }
    
    // Check if plugin supports at least one client version
    if (plugin->supportedVersions().isEmpty()) {
        qWarning() << "Plugin supports no client versions";
        return false;
    }
    
    // Check for duplicate plugin names
    if (m_pluginsByName.contains(plugin->name())) {
        qWarning() << "Plugin with name already exists:" << plugin->name();
        return false;
    }
    
    return true;
}

bool PluginManager::isVersionCompatible(const QString& pluginVersion) const
{
    // For now, accept any non-empty version string
    // In the future, this could implement semantic versioning checks
    if (pluginVersion.isEmpty()) {
        return false;
    }
    
    // Basic version format validation (e.g., "1.0.0")
    QStringList versionParts = pluginVersion.split('.');
    if (versionParts.size() < 2 || versionParts.size() > 3) {
        return false;
    }
    
    // Check if all parts are numeric
    for (const QString& part : versionParts) {
        bool ok;
        part.toInt(&ok);
        if (!ok) {
            return false;
        }
    }
    
    return true;
}

QString PluginManager::getPluginInfo(IPlugin* plugin) const
{
    if (!plugin) {
        return QString();
    }
    
    QString info;
    info += QString("Name: %1\n").arg(plugin->name());
    info += QString("Version: %1\n").arg(plugin->version());
    info += QString("Supported Versions: %1\n").arg(plugin->supportedVersions().join(", "));
    info += QString("Client Loaded: %1\n").arg(plugin->isClientLoaded() ? "Yes" : "No");
    if (plugin->isClientLoaded()) {
        info += QString("Client Version: %1\n").arg(plugin->getClientVersion());
    }
    
    return info;
}

QStringList PluginManager::getAllSupportedVersions() const
{
    QStringList allVersions;
    
    for (IPlugin* plugin : m_plugins) {
        if (plugin) {
            QStringList pluginVersions = plugin->supportedVersions();
            for (const QString& version : pluginVersions) {
                if (!allVersions.contains(version)) {
                    allVersions.append(version);
                }
            }
        }
    }
    
    // Sort versions for consistent output
    allVersions.sort();
    return allVersions;
}

bool PluginManager::isClientVersionSupported(const QString& version) const
{
    for (IPlugin* plugin : m_plugins) {
        if (plugin && plugin->supportedVersions().contains(version)) {
            return true;
        }
    }
    return false;
}

QStringList PluginManager::getLoadingErrors() const
{
    return m_loadingErrors;
}

QString PluginManager::getPluginStatistics() const
{
    QString stats;
    stats += QString("Plugin Manager Statistics\n");
    stats += QString("========================\n");
    stats += QString("Total Plugins Loaded: %1\n").arg(m_plugins.size());
    stats += QString("Plugin Directory: %1\n").arg(m_pluginDirectory);
    stats += QString("Loading Errors: %1\n").arg(m_loadingErrors.size());
    
    if (!m_loadingErrors.isEmpty()) {
        stats += QString("\nLoading Errors:\n");
        for (int i = 0; i < m_loadingErrors.size(); ++i) {
            stats += QString("  %1. %2\n").arg(i + 1).arg(m_loadingErrors.at(i));
        }
    }
    
    stats += QString("\nPlugin Details:\n");
    for (int i = 0; i < m_plugins.size(); ++i) {
        IPlugin* plugin = m_plugins.at(i);
        if (plugin) {
            stats += QString("  %1. %2 v%3\n").arg(i + 1).arg(plugin->name()).arg(plugin->version());
            stats += QString("     Supported Versions: %1\n").arg(plugin->supportedVersions().join(", "));
            stats += QString("     Client Loaded: %1\n").arg(plugin->isClientLoaded() ? "Yes" : "No");
            if (plugin->isClientLoaded()) {
                stats += QString("     Client Version: %1\n").arg(plugin->getClientVersion());
            }
        }
    }
    
    QStringList allVersions = getAllSupportedVersions();
    stats += QString("\nSupported Client Versions (%1 total):\n").arg(allVersions.size());
    stats += QString("  %1\n").arg(allVersions.join(", "));
    
    return stats;
}

bool PluginManager::validateAllPlugins() const
{
    if (m_plugins.isEmpty()) {
        return false;
    }
    
    for (IPlugin* plugin : m_plugins) {
        if (!plugin) {
            qWarning() << "Null plugin found in plugin list";
            return false;
        }
        
        // Check basic plugin properties
        if (plugin->name().isEmpty()) {
            qWarning() << "Plugin has empty name";
            return false;
        }
        
        if (plugin->version().isEmpty()) {
            qWarning() << "Plugin has empty version:" << plugin->name();
            return false;
        }
        
        if (plugin->supportedVersions().isEmpty()) {
            qWarning() << "Plugin supports no client versions:" << plugin->name();
            return false;
        }
        
        // Validate version format
        if (!isVersionCompatible(plugin->version())) {
            qWarning() << "Plugin has invalid version format:" << plugin->name() << plugin->version();
            return false;
        }
        
        // Check for duplicate supported versions across plugins
        QStringList pluginVersions = plugin->supportedVersions();
        for (const QString& version : pluginVersions) {
            int supportCount = 0;
            for (IPlugin* otherPlugin : m_plugins) {
                if (otherPlugin && otherPlugin->supportedVersions().contains(version)) {
                    supportCount++;
                }
            }
            if (supportCount > 1) {
                qWarning() << "Multiple plugins support the same client version:" << version;
                // This is actually acceptable - multiple plugins can support the same version
                // Just log it as a warning, don't fail validation
            }
        }
    }
    
    return true;
}

QList<IPlugin*> PluginManager::getPluginsForVersionRange(const QString& minVersion, const QString& maxVersion) const
{
    QList<IPlugin*> matchingPlugins;
    
    // Parse version strings for comparison
    auto parseVersion = [](const QString& versionStr) -> QPair<int, int> {
        QStringList parts = versionStr.split('.');
        if (parts.size() >= 2) {
            bool ok1, ok2;
            int major = parts[0].toInt(&ok1);
            int minor = parts[1].toInt(&ok2);
            if (ok1 && ok2) {
                return qMakePair(major, minor);
            }
        }
        return qMakePair(-1, -1);
    };
    
    QPair<int, int> minVer = parseVersion(minVersion);
    QPair<int, int> maxVer = parseVersion(maxVersion);
    
    if (minVer.first == -1 || maxVer.first == -1) {
        qWarning() << "Invalid version format for range query:" << minVersion << "to" << maxVersion;
        return matchingPlugins;
    }
    
    for (IPlugin* plugin : m_plugins) {
        if (!plugin) continue;
        
        QStringList supportedVersions = plugin->supportedVersions();
        bool hasVersionInRange = false;
        
        for (const QString& version : supportedVersions) {
            QPair<int, int> ver = parseVersion(version);
            if (ver.first == -1) continue;
            
            // Check if version is within range
            bool isInRange = false;
            if (ver.first > minVer.first || (ver.first == minVer.first && ver.second >= minVer.second)) {
                if (ver.first < maxVer.first || (ver.first == maxVer.first && ver.second <= maxVer.second)) {
                    isInRange = true;
                }
            }
            
            if (isInRange) {
                hasVersionInRange = true;
                break;
            }
        }
        
        if (hasVersionInRange && !matchingPlugins.contains(plugin)) {
            matchingPlugins.append(plugin);
        }
    }
    
    return matchingPlugins;
}

QString PluginManager::getPluginHealthStatus(IPlugin* plugin) const
{
    if (!plugin) {
        return "ERROR: Null plugin";
    }
    
    QString status;
    status += QString("Plugin Health Status: %1\n").arg(plugin->name());
    status += QString("====================\n");
    
    // Basic validation checks
    bool isHealthy = true;
    QStringList issues;
    
    if (plugin->name().isEmpty()) {
        issues.append("Empty plugin name");
        isHealthy = false;
    }
    
    if (plugin->version().isEmpty()) {
        issues.append("Empty plugin version");
        isHealthy = false;
    } else if (!isVersionCompatible(plugin->version())) {
        issues.append("Invalid version format: " + plugin->version());
        isHealthy = false;
    }
    
    if (plugin->supportedVersions().isEmpty()) {
        issues.append("No supported client versions");
        isHealthy = false;
    }
    
    // Check if plugin is in our managed list
    if (!m_plugins.contains(plugin)) {
        issues.append("Plugin not managed by this PluginManager");
        isHealthy = false;
    }
    
    // Check if plugin name conflicts with others
    int nameCount = 0;
    for (IPlugin* otherPlugin : m_plugins) {
        if (otherPlugin && otherPlugin->name() == plugin->name()) {
            nameCount++;
        }
    }
    if (nameCount > 1) {
        issues.append("Duplicate plugin name detected");
        isHealthy = false;
    }
    
    status += QString("Overall Status: %1\n").arg(isHealthy ? "HEALTHY" : "UNHEALTHY");
    status += QString("Plugin Name: %1\n").arg(plugin->name());
    status += QString("Plugin Version: %1\n").arg(plugin->version());
    status += QString("Supported Versions: %1\n").arg(plugin->supportedVersions().join(", "));
    status += QString("Client Loaded: %1\n").arg(plugin->isClientLoaded() ? "Yes" : "No");
    
    if (plugin->isClientLoaded()) {
        status += QString("Client Version: %1\n").arg(plugin->getClientVersion());
    }
    
    if (!issues.isEmpty()) {
        status += QString("\nIssues Found:\n");
        for (int i = 0; i < issues.size(); ++i) {
            status += QString("  %1. %2\n").arg(i + 1).arg(issues.at(i));
        }
    }
    
    return status;
}