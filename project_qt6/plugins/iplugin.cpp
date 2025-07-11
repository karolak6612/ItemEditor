#include "iplugin.h"
#include <QDebug>

PluginManager::PluginManager(QObject* parent) : QObject(parent) {}

PluginManager::~PluginManager() {
    // If plugins were loaded with QPluginLoader, they should be unloaded.
    // If raw pointers are stored and ownership is here, delete them.
    // For DummyPlugin example, if MainWindow is parent, it's handled.
    // If plugins are QObjects and parented to PluginManager, they'd be auto-deleted.
    qDeleteAll(m_plugins); // Assuming m_plugins stores raw pointers it owns
    m_plugins.clear();
}

void PluginManager::loadPlugins(const QString& pluginsPath) {
    Q_UNUSED(pluginsPath);
    // TODO: Implement dynamic plugin loading using QDir to find files
    // and QPluginLoader to load them.
    // For each successfully loaded plugin, call registerPlugin(qobject_cast<IPlugin*>(loader->instance()));
    qWarning() << "Dynamic plugin loading from path not yet implemented.";
}

void PluginManager::registerPlugin(IPlugin* plugin) {
    if (plugin && !m_plugins.contains(plugin)) {
        m_plugins.append(plugin);
        qDebug() << "Plugin registered:" << plugin->pluginName();
    }
}

QList<IPlugin*> PluginManager::availablePlugins() const {
    return m_plugins;
}

IPlugin* PluginManager::findPluginForOtbVersion(quint32 otbVersion) {
    for (IPlugin* plugin : m_plugins) {
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
