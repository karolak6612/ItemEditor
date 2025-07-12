#include "plugins/iplugin.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QSettings>
#include <QMutex>
#include <QMutexLocker>

namespace PluginInterface {

// Static mutex for thread-safe operations
static QMutex s_pluginManagerMutex;

//=============================================================================
// ClientItems Implementation
//=============================================================================

void ClientItems::clear()
{
    QMap<quint16, ItemEditor::ClientItem>::clear();
    signatureCalculated = false;
    validationErrors.clear();
}

bool ClientItems::containsItem(quint16 id) const
{
    return contains(id);
}

ItemEditor::ClientItem ClientItems::getItem(quint16 id) const
{
    return value(id);
}

bool ClientItems::setItem(quint16 id, const ItemEditor::ClientItem& item)
{
    if (id == 0) {
        return false; // Invalid ID
    }
    
    insert(id, item);
    return true;
}

bool ClientItems::removeItem(quint16 id)
{
    return remove(id) > 0;
}

QList<quint16> ClientItems::getItemIds() const
{
    return keys();
}

QList<ItemEditor::ClientItem> ClientItems::getItems() const
{
    return values();
}

int ClientItems::getItemCount() const
{
    return size();
}

QList<quint16> ClientItems::findItemsByName(const QString& name) const
{
    QList<quint16> result;
    for (auto it = begin(); it != end(); ++it) {
        if (it.value().Name.contains(name, Qt::CaseInsensitive)) {
            result.append(it.key());
        }
    }
    return result;
}

QList<quint16> ClientItems::findItemsByType(quint8 type) const
{
    QList<quint16> result;
    for (auto it = begin(); it != end(); ++it) {
        if (static_cast<quint8>(it.value().Type) == type) {
            result.append(it.key());
        }
    }
    return result;
}QList<quint16> ClientItems::findItemsWithProperty(quint32 property) const
{
    QList<quint16> result;
    for (auto it = begin(); it != end(); ++it) {
        if (it.value().HasProperties(property)) {
            result.append(it.key());
        }
    }
    return result;
}

bool ClientItems::validateItems() const
{
    validationErrors.clear();
    bool isValid = true;

    for (auto it = begin(); it != end(); ++it) {
        const quint16 id = it.key();
        const ItemEditor::ClientItem& item = it.value();

        // Validate item ID consistency
        if (item.ID != id) {
            validationErrors.append(QString("Item ID mismatch: key=%1, item.ID=%2").arg(id).arg(item.ID));
            isValid = false;
        }

        // Validate sprite list consistency
        if (item.NumSprites != static_cast<quint32>(item.SpriteList.size())) {
            validationErrors.append(QString("Item %1: NumSprites mismatch: expected=%2, actual=%3")
                                  .arg(id).arg(item.NumSprites).arg(item.SpriteList.size()));
            isValid = false;
        }

        // Validate dimensions
        if (item.Width == 0 || item.Height == 0) {
            validationErrors.append(QString("Item %1: Invalid dimensions: %2x%3")
                                  .arg(id).arg(item.Width).arg(item.Height));
            isValid = false;
        }

        // Validate sprite count matches dimensions
        quint32 expectedSprites = item.Width * item.Height * qMax(item.Layers, quint8(1));
        if (item.NumSprites != expectedSprites) {
            validationErrors.append(QString("Item %1: Sprite count mismatch: expected=%2, actual=%3")
                                  .arg(id).arg(expectedSprites).arg(item.NumSprites));
            isValid = false;
        }
    }

    return isValid;
}

QStringList ClientItems::getValidationErrors() const
{
    return validationErrors;
}

//=============================================================================
// PluginManager Implementation
//=============================================================================

PluginManager::PluginManager(QObject* parent) 
    : QObject(parent)
    , m_applicationVersion("1.0.0")
{
    // Set default directories
    m_applicationDirectory = QCoreApplication::applicationDirPath();
    m_pluginsDirectory = QDir(m_applicationDirectory).absoluteFilePath("plugins");
    m_tempDirectory = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
}

PluginManager::~PluginManager()
{
    unloadAllPlugins();
}void PluginManager::loadPlugins(const QString& pluginsPath)
{
    QMutexLocker locker(&s_pluginManagerMutex);
    
    QDir pluginsDir(pluginsPath);
    if (!pluginsDir.exists()) {
        logError(QString("Plugins directory does not exist: %1").arg(pluginsPath));
        return;
    }

    QStringList nameFilters;
#if defined(Q_OS_WIN)
    nameFilters << "*.dll";
#elif defined(Q_OS_LINUX)
    nameFilters << "*.so";
#elif defined(Q_OS_MACOS)
    nameFilters << "*.dylib";
#endif

    logMessage(QString("Scanning for plugins in: %1").arg(pluginsDir.absolutePath()));

    const QStringList files = pluginsDir.entryList(nameFilters, QDir::Files);
    for (const QString& fileName : files) {
        QString pluginFilePath = pluginsDir.absoluteFilePath(fileName);
        loadPlugin(pluginFilePath);
    }

    rebuildLoadedPluginsCache();
}

void PluginManager::loadPlugin(const QString& pluginPath)
{
    QFileInfo fileInfo(pluginPath);
    if (!fileInfo.exists()) {
        logError(QString("Plugin file does not exist: %1").arg(pluginPath));
        return;
    }

    QPluginLoader* loader = new QPluginLoader(pluginPath, this);
    
    if (!loader->load()) {
        logError(QString("Failed to load plugin %1: %2").arg(pluginPath).arg(loader->errorString()));
        delete loader;
        return;
    }

    QObject* pluginObject = loader->instance();
    if (!pluginObject) {
        logError(QString("Could not get instance from plugin %1: %2").arg(pluginPath).arg(loader->errorString()));
        loader->unload();
        delete loader;
        return;
    }

    IPlugin* plugin = qobject_cast<IPlugin*>(pluginObject);
    if (!plugin) {
        logError(QString("Plugin %1 does not implement IPlugin interface correctly").arg(pluginPath));
        loader->unload();
        delete loader;
        return;
    }

    // Validate plugin before initialization
    if (!validatePlugin(plugin)) {
        logError(QString("Plugin %1 failed validation").arg(pluginPath));
        loader->unload();
        delete loader;
        return;
    }

    // Set host and initialize
    plugin->setHost(this);
    if (!plugin->Initialize()) {
        logError(QString("Plugin %1 failed to initialize").arg(plugin->pluginName()));
        loader->unload();
        delete loader;
        return;
    }

    // Store loader and metadata
    m_pluginLoaders.append(loader);
    m_pluginMetadata[plugin->pluginName()] = loadPluginMetadata(pluginPath);

    // Connect destruction signal for cleanup
    connect(pluginObject, &QObject::destroyed, this, &PluginManager::onPluginDestroyed);

    logMessage(QString("Successfully loaded plugin: %1 v%2").arg(plugin->pluginName()).arg(plugin->pluginVersion()));
    emit pluginLoaded(plugin->pluginName());
}void PluginManager::unloadPlugin(const QString& pluginName)
{
    QMutexLocker locker(&s_pluginManagerMutex);
    
    IPlugin* plugin = findPlugin(pluginName);
    if (!plugin) {
        logWarning(QString("Plugin not found for unloading: %1").arg(pluginName));
        return;
    }

    // Find and remove the corresponding loader
    for (int i = 0; i < m_pluginLoaders.size(); ++i) {
        QPluginLoader* loader = m_pluginLoaders[i];
        QObject* pluginObject = loader->instance();
        IPlugin* loaderPlugin = qobject_cast<IPlugin*>(pluginObject);
        
        if (loaderPlugin && loaderPlugin->pluginName() == pluginName) {
            cleanupPlugin(loaderPlugin);
            loader->unload();
            m_pluginLoaders.removeAt(i);
            delete loader;
            break;
        }
    }

    m_pluginMetadata.remove(pluginName);
    rebuildLoadedPluginsCache();
    
    logMessage(QString("Unloaded plugin: %1").arg(pluginName));
    emit pluginUnloaded(pluginName);
}

void PluginManager::unloadAllPlugins()
{
    QMutexLocker locker(&s_pluginManagerMutex);
    
    // Cleanup static plugins
    for (IPlugin* plugin : m_staticPlugins) {
        cleanupPlugin(plugin);
        delete plugin;
    }
    m_staticPlugins.clear();

    // Cleanup dynamic plugins
    for (QPluginLoader* loader : m_pluginLoaders) {
        QObject* pluginObject = loader->instance();
        IPlugin* plugin = qobject_cast<IPlugin*>(pluginObject);
        if (plugin) {
            cleanupPlugin(plugin);
        }
        loader->unload();
        delete loader;
    }
    m_pluginLoaders.clear();

    m_loadedPluginsCache.clear();
    m_pluginMetadata.clear();
}

void PluginManager::registerStaticPlugin(IPlugin* plugin)
{
    if (!plugin) {
        logError("Attempted to register null plugin");
        return;
    }

    QMutexLocker locker(&s_pluginManagerMutex);
    
    if (m_staticPlugins.contains(plugin)) {
        logWarning(QString("Plugin already registered: %1").arg(plugin->pluginName()));
        return;
    }

    if (!validatePlugin(plugin)) {
        logError(QString("Static plugin failed validation: %1").arg(plugin->pluginName()));
        return;
    }

    plugin->setHost(this);
    if (!plugin->Initialize()) {
        logError(QString("Static plugin failed to initialize: %1").arg(plugin->pluginName()));
        return;
    }

    m_staticPlugins.append(plugin);
    rebuildLoadedPluginsCache();
    
    logMessage(QString("Registered static plugin: %1 v%2").arg(plugin->pluginName()).arg(plugin->pluginVersion()));
    emit pluginLoaded(plugin->pluginName());
}QList<IPlugin*> PluginManager::getAvailablePlugins() const
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
        if (plugin->pluginName() == pluginName) {
            return plugin;
        }
    }
    return nullptr;
}

IPlugin* PluginManager::findPluginForOtbVersion(quint32 otbVersion) const
{
    for (IPlugin* plugin : m_loadedPluginsCache) {
        const QList<ItemEditor::SupportedClient> clients = plugin->getSupportedClients();
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
        const QList<ItemEditor::SupportedClient> clients = plugin->getSupportedClients();
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
        try {
            ItemEditor::SupportedClient client = plugin->GetClientBySignatures(datSignature, sprSignature);
            if (client.datSignature == datSignature && client.sprSignature == sprSignature) {
                return plugin;
            }
        } catch (...) {
            // Plugin doesn't support this signature combination
            continue;
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
    return m_pluginMetadata.value(pluginName);
}// IPluginHost implementation
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
    qDebug() << "Plugin:" << message;
    emit const_cast<PluginManager*>(this)->logMessageEmitted(message, level);
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
    emit const_cast<PluginManager*>(this)->progressChanged(percentage, status);
}

void PluginManager::setProgressVisible(bool visible) const
{
    // Implementation depends on UI framework integration
    Q_UNUSED(visible)
}

QVariant PluginManager::getConfigValue(const QString& key, const QVariant& defaultValue) const
{
    QSettings settings;
    return settings.value(QString("plugins/%1").arg(key), defaultValue);
}

void PluginManager::setConfigValue(const QString& key, const QVariant& value) const
{
    QSettings settings;
    settings.setValue(QString("plugins/%1").arg(key), value);
}

QByteArray PluginManager::loadResource(const QString& resourcePath) const
{
    QFile file(resourcePath);
    if (file.open(QIODevice::ReadOnly)) {
        return file.readAll();
    }
    return QByteArray();
}bool PluginManager::saveResource(const QString& resourcePath, const QByteArray& data) const
{
    QFile file(resourcePath);
    if (file.open(QIODevice::WriteOnly)) {
        return file.write(data) == data.size();
    }
    return false;
}

bool PluginManager::sendMessage(const QString& targetPlugin, const QString& message, const QVariant& data) const
{
    // Inter-plugin communication implementation
    // This could be extended with a proper message bus system
    Q_UNUSED(targetPlugin)
    Q_UNUSED(message)
    Q_UNUSED(data)
    return false;
}

// Configuration setters
void PluginManager::setApplicationVersion(const QString& version)
{
    m_applicationVersion = version;
}

void PluginManager::setApplicationDirectory(const QString& directory)
{
    m_applicationDirectory = directory;
}

void PluginManager::setPluginsDirectory(const QString& directory)
{
    m_pluginsDirectory = directory;
}

void PluginManager::setTempDirectory(const QString& directory)
{
    m_tempDirectory = directory;
}

// Public slots
void PluginManager::refreshPlugins()
{
    logMessage("Refreshing plugins...");
    loadPlugins(m_pluginsDirectory);
}

// Private slots
void PluginManager::onPluginDestroyed(QObject* plugin)
{
    // Handle plugin destruction cleanup
    Q_UNUSED(plugin)
    rebuildLoadedPluginsCache();
}

// Private methods
void PluginManager::rebuildLoadedPluginsCache()
{
    m_loadedPluginsCache.clear();
    
    // Add static plugins
    m_loadedPluginsCache.append(m_staticPlugins);
    
    // Add dynamic plugins
    for (QPluginLoader* loader : m_pluginLoaders) {
        QObject* pluginObject = loader->instance();
        if (pluginObject) {
            IPlugin* plugin = qobject_cast<IPlugin*>(pluginObject);
            if (plugin) {
                m_loadedPluginsCache.append(plugin);
            }
        }
    }
}bool PluginManager::validatePlugin(IPlugin* plugin) const
{
    if (!plugin) {
        return false;
    }

    // Check required methods return valid values
    if (plugin->pluginName().isEmpty()) {
        logError("Plugin has empty name");
        return false;
    }

    if (plugin->pluginVersion().isEmpty()) {
        logError(QString("Plugin %1 has empty version").arg(plugin->pluginName()));
        return false;
    }

    // Validate supported clients
    const QList<ItemEditor::SupportedClient> clients = plugin->getSupportedClients();
    if (clients.isEmpty()) {
        logWarning(QString("Plugin %1 supports no clients").arg(plugin->pluginName()));
    }

    for (const ItemEditor::SupportedClient& client : clients) {
        if (client.version == 0 || client.otbVersion == 0) {
            logError(QString("Plugin %1 has invalid client version data").arg(plugin->pluginName()));
            return false;
        }
    }

    return true;
}

PluginMetadata PluginManager::loadPluginMetadata(const QString& pluginPath) const
{
    PluginMetadata metadata;
    
    // Try to load metadata from JSON file alongside the plugin
    QFileInfo pluginInfo(pluginPath);
    QString metadataPath = pluginInfo.absolutePath() + "/" + pluginInfo.baseName() + ".json";
    
    QFile metadataFile(metadataPath);
    if (metadataFile.open(QIODevice::ReadOnly)) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(metadataFile.readAll(), &error);
        
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            metadata.name = obj["name"].toString();
            metadata.description = obj["description"].toString();
            metadata.version = obj["version"].toString();
            metadata.author = obj["author"].toString();
            metadata.website = obj["website"].toString();
            metadata.license = obj["license"].toString();
            metadata.apiVersion = obj["apiVersion"].toInt(1);
            
            QJsonArray deps = obj["dependencies"].toArray();
            for (const QJsonValue& dep : deps) {
                metadata.dependencies.append(dep.toString());
            }
        }
    }
    
    return metadata;
}

void PluginManager::cleanupPlugin(IPlugin* plugin)
{
    if (!plugin) {
        return;
    }

    try {
        plugin->Dispose();
    } catch (...) {
        logError(QString("Exception during plugin disposal: %1").arg(plugin->pluginName()));
    }
}

} // namespace PluginInterface

#include "iplugin.moc"