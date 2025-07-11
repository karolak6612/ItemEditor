#include "dummyplugin.h"
#include <QDebug>

DummyPlugin::DummyPlugin(QObject *parent) : QObject(parent), m_isClientLoaded(false)
{
    createDummyData();
}

DummyPlugin::~DummyPlugin()
{
    qDebug() << "DummyPlugin destroyed";
}

QString DummyPlugin::pluginName() const
{
    return "Dummy Plugin";
}

QString DummyPlugin::pluginDescription() const
{
    return "A stub plugin for testing purposes. Does not load real client data.";
}

QList<OTB::SupportedClient> DummyPlugin::getSupportedClients() const
{
    return m_supportedClients;
}

bool DummyPlugin::loadClient(const OTB::SupportedClient& client,
                           const QString& clientDirectoryPath,
                           bool extended, bool frameDurations, bool transparency,
                           QString& errorString)
{
    Q_UNUSED(clientDirectoryPath);
    Q_UNUSED(extended);
    Q_UNUSED(frameDurations);
    Q_UNUSED(transparency);

    bool found = false;
    for(const auto& sc : m_supportedClients) {
        if (sc.version == client.version && sc.otbVersion == client.otbVersion) {
            found = true;
            m_currentlyLoadedClient = sc; // Store a copy
            break;
        }
    }

    if (!found) {
        errorString = QString("DummyPlugin does not support client version %1 (OTB %2).").arg(client.version).arg(client.otbVersion);
        m_isClientLoaded = false;
        return false;
    }

    // Simulate loading by just setting the flag and populating a few dummy items
    // In a real plugin, this is where DAT/SPR parsing would happen.
    m_clientItems.clear();

    OTB::ClientItem item100;
    item100.id = 100; // Client ID
    item100.name = "Dummy Item (CID 100)";
    item100.type = OTB::ServerItemType::Ground; // Example
    item100.width = 1;
    item100.height = 1;
    item100.layers = 1;
    item100.frames = 1;
    item100.numSprites = 1;
    // item100.spriteHash will be default (zeros) or could be set
    m_clientItems.insert(item100.id, item100);

    OTB::ClientItem item101;
    item101.id = 101; // Client ID
    item101.name = "Another Dummy (CID 101)";
    item101.type = OTB::ServerItemType::Container;
    item101.width = 1;
    item101.height = 1;
    item101.layers = 1;
    item101.frames = 1;
    item101.numSprites = 1;
    m_clientItems.insert(item101.id, item101);

    // Example for an item that might be in OTB but not have a direct client counterpart initially
    // Or, an item that ServerItem refers to by ClientID
    OTB::ClientItem item3039; // Example from C# test OTB (stone tile)
    item3039.id = 3039;
    item3039.name = "Stone Tile (Dummy Client)";
    item3039.type = OTB::ServerItemType::Ground;
    m_clientItems.insert(item3039.id, item3039);


    qDebug() << "DummyPlugin: Simulated loading client" << client.description;
    m_isClientLoaded = true;
    errorString = "";
    return true;
}

bool DummyPlugin::isClientLoaded() const
{
    return m_isClientLoaded;
}

const OTB::SupportedClient& DummyPlugin::getCurrentLoadedClient() const
{
    if (!m_isClientLoaded) {
        // This should ideally not be called if not loaded, or return a default/invalid client
        // For now, throwing or returning a static invalid client might be options.
        // Let's return the first supported if any, or an empty one.
        static OTB::SupportedClient invalidClient; // Default constructed
        qWarning() << "DummyPlugin::getCurrentLoadedClient called when no client is loaded.";
        return invalidClient;
    }
    return m_currentlyLoadedClient;
}


const QMap<quint16, OTB::ClientItem>& DummyPlugin::getClientItems() const
{
    return m_clientItems;
}

bool DummyPlugin::getClientItem(quint16 clientItemId, OTB::ClientItem& outItem) const
{
    if (m_clientItems.contains(clientItemId)) {
        outItem = m_clientItems.value(clientItemId);
        return true;
    }
    return false;
}

void DummyPlugin::unloadClient()
{
    m_clientItems.clear();
    m_isClientLoaded = false;
    m_currentlyLoadedClient = OTB::SupportedClient(); // Reset
    qDebug() << "DummyPlugin: Simulated unloading client.";
}

void DummyPlugin::createDummyData()
{
    // Based on C# PluginOne/PluginOne.xml and PluginTwo/PluginTwo.xml examples
    m_supportedClients.append(OTB::SupportedClient(770, "Tibia Client 7.70 (Dummy)", 770, 0x4A34DE39, 0x4A34DE39)); // Signatures are examples
    m_supportedClients.append(OTB::SupportedClient(860, "Tibia Client 8.60 (Dummy)", 860, 0x50C69F87, 0x50C6A25A));
    m_supportedClients.append(OTB::SupportedClient(1098, "Tibia Client 10.98 (Dummy)", 1098, 0x0, 0x0)); // Signatures unknown for this example
}
