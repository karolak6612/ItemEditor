#ifndef IPLUGIN_H
#define IPLUGIN_H

#include "otb/item.h" // For ClientItem, SupportedClient
#include <QString>
#include <QList>
#include <QMap>
#include <QObject> // For Q_DECLARE_INTERFACE, QPluginLoader parent
#include <QPluginLoader> // For dynamic plugin loading
#include <QDir>        // For scanning plugin directory

namespace OTB {
    class ServerItemList; // Forward declaration
}

// Define an interface class for plugins
class IPlugin
{
public:
    virtual ~IPlugin() = default;

    // Initialize method that plugins must implement
    virtual bool Initialize() = 0;

    virtual QString pluginName() const = 0;
    virtual QString pluginDescription() const = 0;
    virtual QList<OTB::SupportedClient> getSupportedClients() const = 0;

    // Attempts to load client data for the specified client version.
    // clientDirectoryPath: Path to the Tibia client directory.
    // Returns true on success, false on failure.
    // Error details can be retrieved via a separate method if needed.
    virtual bool loadClient(const OTB::SupportedClient& client,
                            const QString& clientDirectoryPath,
                            bool extended, bool frameDurations, bool transparency, // Options from C#
                            QString& errorString) = 0;

    virtual bool isClientLoaded() const = 0;
    virtual const OTB::SupportedClient& getCurrentLoadedClient() const = 0; // Throws if not loaded

    // Retrieves all client items for the currently loaded client.
    // Key is Client ID.
    virtual const QMap<quint16, OTB::ClientItem>& getClientItems() const = 0;

    // Optional: Get a specific client item
    virtual bool getClientItem(quint16 clientItemId, OTB::ClientItem& outItem) const = 0;

    // Unloads any currently loaded client data.
    virtual void unloadClient() = 0;

    // Other potential methods:
    // - getSpriteData(quint16 spriteId)
    // - getRawItemData(quint16 itemId) // For .dat parsing details
};

// Declare the interface using Q_DECLARE_INTERFACE
// This is necessary if we want to use Qt's plugin system (QPluginLoader) later.
// The string identifier should be unique.
#define IPlugin_iid "com.ItemEditorQt.IPlugin/1.0"
Q_DECLARE_INTERFACE(IPlugin, IPlugin_iid)


// For managing multiple plugins later
class PluginManager : public QObject {
    Q_OBJECT
public:
    explicit PluginManager(QObject* parent = nullptr);
    ~PluginManager();

    void loadPlugins(const QString& pluginsPath); // For dynamic loading later
    void registerPlugin(IPlugin* plugin); // For static/stub plugins

    QList<IPlugin*> availablePlugins() const;
    IPlugin* findPluginForOtbVersion(quint32 otbVersion); // Helper
    IPlugin* findPluginForClientVersion(quint32 clientVersion); // Helper

private:
    QList<IPlugin*> m_staticPlugins; // For plugins compiled in or manually registered
    QList<QPluginLoader*> m_pluginLoaders; // For dynamically loaded plugins
    QList<IPlugin*> m_loadedPluginsCache; // Combined list, rebuild when needed
    void rebuildLoadedPluginsCache();
};


#endif // IPLUGIN_H
