#include "plugins/plugins.h"
#include <QCoreApplication>
#include <QDebug>

namespace PluginInterface {

// Global plugin manager instance
static PluginManager* g_pluginManager = nullptr;

void initializePluginSystem()
{
    qDebug() << "Initializing plugin system version" << PluginSystemInfo::VERSION;
    
    if (g_pluginManager) {
        qWarning() << "Plugin system already initialized";
        return;
    }
    
    // Create global plugin manager
    g_pluginManager = new PluginManager(QCoreApplication::instance());
    
    // Set application information
    g_pluginManager->setApplicationVersion(QCoreApplication::applicationVersion());
    g_pluginManager->setApplicationDirectory(QCoreApplication::applicationDirPath());
    
    // Register built-in plugin types
    PluginFactory::instance().registerBuiltInPlugins();
    
    qDebug() << "Plugin system initialized successfully";
}

void shutdownPluginSystem()
{
    qDebug() << "Shutting down plugin system";
    
    if (g_pluginManager) {
        g_pluginManager->unloadAllPlugins();
        g_pluginManager->deleteLater();
        g_pluginManager = nullptr;
    }
    
    qDebug() << "Plugin system shutdown complete";
}

PluginSystemInfo getPluginSystemInfo()
{
    return PluginSystemInfo{};
}

bool validatePluginCompatibility(IPlugin* plugin)
{
    if (!plugin) {
        return false;
    }
    
    // Check plugin name and version
    if (plugin->pluginName().isEmpty() || plugin->pluginVersion().isEmpty()) {
        qWarning() << "Plugin has empty name or version";
        return false;
    }
    
    // Check supported clients
    const QList<ItemEditor::SupportedClient> clients = plugin->getSupportedClients();
    if (clients.isEmpty()) {
        qWarning() << "Plugin supports no clients:" << plugin->pluginName();
        return false;
    }
    
    // Validate client data
    for (const ItemEditor::SupportedClient& client : clients) {
        if (client.version == 0 || client.otbVersion == 0) {
            qWarning() << "Plugin has invalid client data:" << plugin->pluginName();
            return false;
        }
    }
    
    return true;
}

PluginManager* getGlobalPluginManager()
{
    return g_pluginManager;
}

} // namespace PluginInterface