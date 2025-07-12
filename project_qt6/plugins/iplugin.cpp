#include "iplugin.h"
#include <QDebug>

PluginManager::PluginManager(QObject* parent) : QObject(parent) {}

PluginManager::~PluginManager() {
    // If QPluginLoaders are used, they should be unloaded.
    // QPluginLoader instances are QObjects, if parented to PluginManager, they are auto-deleted.
    // Raw IPlugin pointers from static registration might need manual deletion if not parented elsewhere.
    qDeleteAll(m_staticPlugins); // Delete statically registered plugins if owned by manager
    m_staticPlugins.clear();

    for (QPluginLoader* loader : m_pluginLoaders) {
        loader->unload(); // Unload the plugin
        delete loader;    // Delete the loader itself
    }
    m_pluginLoaders.clear();
    m_loadedPluginsCache.clear();
}

void PluginManager::rebuildLoadedPluginsCache() {
    m_loadedPluginsCache.clear();
    m_loadedPluginsCache.append(m_staticPlugins);
    for (QPluginLoader* loader : m_pluginLoaders) {
        QObject *pluginObject = loader->instance();
        if (pluginObject) {
            IPlugin *plugin = qobject_cast<IPlugin*>(pluginObject);
            if (plugin) {
                m_loadedPluginsCache.append(plugin);
            }
        }
    }
}


void PluginManager::loadPlugins(const QString& pluginsPath) {
    QDir pluginsDir(pluginsPath);
    QStringList nameFilters;
#if defined(Q_OS_WIN)
    nameFilters << "*.dll";
#elif defined(Q_OS_LINUX)
    nameFilters << "*.so";
#elif defined(Q_OS_MACOS)
    nameFilters << "*.dylib";
#endif

    qDebug() << "Scanning for plugins in:" << pluginsDir.absolutePath();

    for (const QString &fileName : pluginsDir.entryList(nameFilters, QDir::Files)) {
        QString pluginFilePath = pluginsDir.absoluteFilePath(fileName);
        QPluginLoader *loader = new QPluginLoader(pluginFilePath, this); // Parent loader to PluginManager

        if (!loader->load()) {
            qWarning() << "Failed to load plugin" << pluginFilePath << ":" << loader->errorString();
            delete loader;
            continue;
        }

        QObject *pluginObject = loader->instance();
        if (pluginObject) {
            IPlugin *plugin = qobject_cast<IPlugin*>(pluginObject);
            if (plugin) {
                m_pluginLoaders.append(loader); // Store loader for management
                plugin->Initialize(); // Call Initialize on successfully loaded plugin
                qDebug() << "Successfully loaded plugin:" << plugin->pluginName() << "from" << pluginFilePath;
            } else {
                qWarning() << "Plugin" << pluginFilePath << "does not implement IPlugin interface correctly.";
                loader->unload();
                delete loader;
            }
        } else {
             qWarning() << "Could not get instance from plugin" << pluginFilePath << ":" << loader->errorString();
             loader->unload(); // Should not be necessary if instance() is null but good practice
             delete loader;
        }
    }
    rebuildLoadedPluginsCache();
}

void PluginManager::registerPlugin(IPlugin* plugin) {
    // For plugins compiled directly or added manually (like DummyPlugin or RealPlugin770 if not dynamic)
    if (plugin && !m_staticPlugins.contains(plugin)) {
        m_staticPlugins.append(plugin);
        qDebug() << "Statically registered plugin:" << plugin->pluginName();
        rebuildLoadedPluginsCache();
    }
}

QList<IPlugin*> PluginManager::availablePlugins() const {
    // Return a copy of the cache
    return m_loadedPluginsCache;
}

IPlugin* PluginManager::findPluginForOtbVersion(quint32 otbVersion) {
    for (IPlugin* plugin : m_loadedPluginsCache) { // Use the cache
        for (const OTB::SupportedClient& client : plugin->getSupportedClients()) {
            if (client.otbVersion == otbVersion) {
                return plugin;
            }
        }
    }
    return nullptr;
}

IPlugin* PluginManager::findPluginForClientVersion(quint32 clientVersion) {
     for (IPlugin* plugin : m_plugins) {
        for (const OTB::SupportedClient& client : plugin->getSupportedClients()) {
            if (client.version == clientVersion) {
                return plugin;
            }
        }
    }
    return nullptr;
}
