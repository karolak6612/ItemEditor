#include "plugins/exampleplugin.h"

namespace PluginInterface {

ExamplePlugin::ExamplePlugin(QObject* parent)
    : BasePlugin(parent)
    , m_initialized(false)
{
    // Set plugin information
    setPluginName("ExamplePlugin");
    setPluginDescription("Example plugin demonstrating the plugin interface");
    setPluginVersion("1.0.0");
    
    // Initialize supported clients
    initializeSupportedClients();
}

ExamplePlugin::~ExamplePlugin()
{
    // Cleanup is handled by BasePlugin destructor
}

QList<ItemEditor::SupportedClient> ExamplePlugin::getSupportedClients() const
{
    return m_supportedClients;
}

bool ExamplePlugin::LoadClient(const ItemEditor::SupportedClient& client,
                              bool extended,
                              bool frameDurations,
                              bool transparency,
                              const QString& datFullPath,
                              const QString& sprFullPath)
{
    logMessage(QString("Loading client: %1").arg(client.description));
    
    // Validate client is supported
    bool clientSupported = false;
    for (const ItemEditor::SupportedClient& supportedClient : m_supportedClients) {
        if (supportedClient.version == client.version &&
            supportedClient.otbVersion == client.otbVersion) {
            clientSupported = true;
            break;
        }
    }
    
    if (!clientSupported) {
        setLastError(QString("Unsupported client version: %1").arg(client.version));
        return false;
    }
    
    // Validate file paths
    if (!validateClientFiles(datFullPath, sprFullPath)) {
        return false;
    }
    
    // Call base implementation
    return doLoadClient(client, extended, frameDurations, transparency, datFullPath, sprFullPath);
}

bool ExamplePlugin::doInitialize()
{
    logMessage("Initializing ExamplePlugin");
    
    if (m_supportedClients.isEmpty()) {
        setLastError("No supported clients configured");
        return false;
    }
    
    // Set item ID range (example values)
    setMinItemId(100);
    setMaxItemId(999);
    
    m_initialized = true;
    logMessage("ExamplePlugin initialized successfully");
    return true;
}void ExamplePlugin::doDispose()
{
    logMessage("Disposing ExamplePlugin");
    m_initialized = false;
}

bool ExamplePlugin::doLoadClient(const ItemEditor::SupportedClient& client,
                                bool extended, bool frameDurations, bool transparency,
                                const QString& datPath, const QString& sprPath)
{
    logMessage(QString("Loading client files: %1, %2").arg(datPath).arg(sprPath));
    
    // Parse client files
    if (!parseClientFiles(datPath, sprPath)) {
        return false;
    }
    
    // Populate client items
    populateClientItems();
    
    // Mark as loaded
    setLoaded(true);
    
    logMessage(QString("Client loaded successfully: %1 items").arg(getItems()->size()));
    return true;
}

void ExamplePlugin::doUnloadClient()
{
    logMessage("Unloading client");
    
    // Clear items
    if (getItems()) {
        getItems()->clear();
    }
    
    setLoaded(false);
}

void ExamplePlugin::initializeSupportedClients()
{
    // Add example supported clients
    m_supportedClients.clear();
    
    // Example client 1
    ItemEditor::SupportedClient client1;
    client1.version = 1098;
    client1.description = "Example Client 10.98";
    client1.otbVersion = 770;
    client1.datSignature = 0x12345678;
    client1.sprSignature = 0x87654321;
    m_supportedClients.append(client1);
    
    // Example client 2
    ItemEditor::SupportedClient client2;
    client2.version = 1100;
    client2.description = "Example Client 11.00";
    client2.otbVersion = 860;
    client2.datSignature = 0x11223344;
    client2.sprSignature = 0x44332211;
    m_supportedClients.append(client2);
    
    logDebug(QString("Initialized %1 supported clients").arg(m_supportedClients.size()));
}bool ExamplePlugin::parseClientFiles(const QString& datPath, const QString& sprPath)
{
    logDebug(QString("Parsing DAT file: %1").arg(datPath));
    logDebug(QString("Parsing SPR file: %1").arg(sprPath));
    
    // This is a placeholder implementation
    // Real plugins would implement actual file parsing here
    
    // Simulate file parsing delay
    QThread::msleep(100);
    
    // For demonstration, just validate files exist and are readable
    QFileInfo datInfo(datPath);
    QFileInfo sprInfo(sprPath);
    
    if (!datInfo.exists() || !datInfo.isReadable()) {
        setLastError(QString("Cannot read DAT file: %1").arg(datPath));
        return false;
    }
    
    if (!sprInfo.exists() || !sprInfo.isReadable()) {
        setLastError(QString("Cannot read SPR file: %1").arg(sprPath));
        return false;
    }
    
    logDebug("Client files parsed successfully");
    return true;
}

void ExamplePlugin::populateClientItems()
{
    logDebug("Populating client items");
    
    ClientItems* items = getItems();
    if (!items) {
        logError("Items collection is null");
        return;
    }
    
    items->clear();
    
    // Create example items
    for (quint16 id = getMinItemId(); id <= getMaxItemId() && id < getMinItemId() + 10; ++id) {
        ItemEditor::ClientItem item;
        item.ID = id;
        item.Name = QString("Example Item %1").arg(id);
        item.Width = 1;
        item.Height = 1;
        item.Layers = 1;
        item.NumSprites = 1;
        
        // Create a dummy sprite
        ItemEditor::Sprite sprite;
        sprite.id = id;
        sprite.size = 0;
        sprite.transparent = true;
        item.SpriteList.append(sprite);
        
        items->setItem(id, item);
    }
    
    items->setSignatureCalculated(true);
    logDebug(QString("Populated %1 example items").arg(items->size()));
}

} // namespace PluginInterface

#include "exampleplugin.moc"