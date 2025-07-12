#include "realplugin770.h"
#include "otb/item.h" // For OTB::ClientItem, OTB::Sprite
#include <QFileInfo>
#include <QDir>
#include <QDebug>

RealPlugin770::RealPlugin770(QObject *parent)
    : QObject(parent), m_isClientLoaded(false)
{
    // Define supported clients for this plugin (e.g., 7.70)
    // Signatures for 7.70 (example, actual values should be verified if possible)
    // From a quick search, tibia.dat 7.70 might be around 0x4A34DE39
    // tibia.spr 7.70 might be around 0x4A34DE39
    // These are often the same for older clients.
    // OTB version for 7.70 is typically also 770.
    m_supportedClients.append(OTB::SupportedClient(770, "Tibia Client 7.70", 770, 0x4A34DE39, 0x4A34DE39));
    // Could add other clients this plugin format might support if DAT/SPR structure is identical.
}

RealPlugin770::~RealPlugin770()
{
    unloadClient();
}

QString RealPlugin770::pluginName() const
{
    return "RealPlugin for Tibia 7.70";
}

QString RealPlugin770::pluginDescription() const
{
    return "Loads DAT and SPR files for Tibia client version 7.70.";
}

QList<OTB::SupportedClient> RealPlugin770::getSupportedClients() const
{
    return m_supportedClients;
}

void RealPlugin770::Initialize() {
    // C# IPlugin has this. Perform any one-time setup if needed.
    OTB::Sprite::createBlankSprite(); // Ensure blank sprites are initialized if not done globally
}


bool RealPlugin770::loadClient(
    const OTB::SupportedClient& clientToLoad,
    const QString& clientDirectoryPath,
    bool extended, bool frameDurations, bool transparency, // Options from preferences
    QString& errorString)
{
    unloadClient(); // Ensure clean state

    bool foundMatch = false;
    for(const auto& sc : m_supportedClients) {
        if (sc.version == clientToLoad.version) { // Match based on version primarily
            m_currentlyLoadedClient = sc; // Use the profile defined by the plugin
            foundMatch = true;
            break;
        }
    }
    if (!foundMatch) {
        errorString = tr("This plugin does not support client version %1.").arg(clientToLoad.version);
        return false;
    }

    QDir clientDir(clientDirectoryPath);
    if (!clientDir.exists()) {
        errorString = tr("Client directory does not exist: %1").arg(clientDirectoryPath);
        return false;
    }

    // Construct full paths (assuming standard Tibia.dat, Tibia.spr names)
    // C# ItemEditor.Helpers.Utils.FindClientFile might have more robust finding logic
    QString datPath = clientDir.filePath("Tibia.dat");
    QString sprPath = clientDir.filePath("Tibia.spr");

    if (!QFileInfo::exists(datPath)) {
        errorString = tr("Tibia.dat not found in: %1").arg(datPath);
        return false;
    }
    if (!QFileInfo::exists(sprPath)) {
        errorString = tr("Tibia.spr not found in: %1").arg(sprPath);
        return false;
    }

    m_currentlyLoadedClient.datPath = datPath; // Store actual paths used
    m_currentlyLoadedClient.sprPath = sprPath;


    // --- Load SPR File ---
    // The 'extended' flag for SPR count (uint16 vs uint32) depends on client version.
    // For 7.70, it's typically uint16 (not extended).
    // The C# `extended` parameter passed to `LoadSprites` is a combination of user preference and client version.
    // `extended = extended || client.Version >= 960;`
    // We will use the same logic here.
    bool isSprExtended = extended || (m_currentlyLoadedClient.version >= 960);

    if (!m_sprParser.loadSpr(sprPath, isSprExtended, errorString)) {
        // errorString is set by sprParser
        return false;
    }
    if (m_sprParser.getSignature() != m_currentlyLoadedClient.sprSignature) {
        errorString = tr("SPR file signature mismatch. Expected 0x%1, got 0x%2.")
                          .arg(m_currentlyLoadedClient.sprSignature, 8, 16, QChar('0'))
                          .arg(m_sprParser.getSignature(), 8, 16, QChar('0'));
        // For now, treat as warning, C# ItemEditor continues.
        qWarning() << errorString;
    }

    // --- Load DAT File ---
    if (!m_datParser.loadDat(datPath, m_currentlyLoadedClient.version, errorString)) {
        // errorString is set by datParser
        return false;
    }
    if (m_datParser.getSignature() != m_currentlyLoadedClient.datSignature) {
        errorString = tr("DAT file signature mismatch. Expected 0x%1, got 0x%2.")
                          .arg(m_currentlyLoadedClient.datSignature, 8, 16, QChar('0'))
                          .arg(m_datParser.getSignature(), 8, 16, QChar('0'));
        qWarning() << errorString; // Also treat as warning for now
    }

    // --- Populate ClientItems from DAT and associate SPR data ---
    // DatParser's loadDat now parses all items into m_parsedClientItems.
    // We need to get that map.
    QMap<quint16, OTB::ClientItem> parsedItems;
    // The 'extendedFormat' for DAT parsing (affecting attribute reading) also depends on client version.
    // DatParser::loadDat should ideally take this as a parameter or deduce it.
    // For 7.70, extended is typically false.
    bool isDatExtended = (m_currentlyLoadedClient.version >= 780); // Example threshold for DAT structure changes
                                                               // This is complex and version dependent.
                                                               // DatParser.parseThing needs this.

    if (!m_datParser.getAllClientItems(m_clientItems, isDatExtended)) { // Pass extended flag
        errorString = tr("Failed to retrieve parsed client items from DatParser.");
        return false;
    }

    // Now, for each ClientItem from DAT, load its sprites from SPR
    populateSpriteDataForClientItems();


    m_isClientLoaded = true;
    qDebug() << pluginName() << "loaded client" << m_currentlyLoadedClient.description
             << "with" << m_clientItems.count() << "items.";
    errorString.clear();
    return true;
}

bool RealPlugin770::isClientLoaded() const
{
    return m_isClientLoaded;
}

const OTB::SupportedClient& RealPlugin770::getCurrentLoadedClient() const
{
    // Should check m_isClientLoaded before calling, or this could return invalid data.
    // For robustness, return a static invalid if not loaded.
    if (!m_isClientLoaded) {
        static OTB::SupportedClient invalid;
        qWarning() << "getCurrentLoadedClient called when no client loaded in" << pluginName();
        return invalid;
    }
    return m_currentlyLoadedClient;
}

const QMap<quint16, OTB::ClientItem>& RealPlugin770::getClientItems() const
{
    return m_clientItems;
}

bool RealPlugin770::getClientItem(quint16 clientItemId, OTB::ClientItem& outItem) const
{
    if (!m_isClientLoaded) return false;
    if (m_clientItems.contains(clientItemId)) {
        outItem = m_clientItems.value(clientItemId);
        return true;
    }
    return false;
}

void RealPlugin770::unloadClient()
{
    m_clientItems.clear();
    // m_sprParser.closeFile(); // If sprParser keeps file open
    // m_datParser.closeFile(); // If datParser keeps file open
    m_isClientLoaded = false;
    m_currentlyLoadedClient = OTB::SupportedClient(); // Reset
    qDebug() << pluginName() << "unloaded client data.";
}

void RealPlugin770::populateSpriteDataForClientItems() {
    if (!m_isClientLoaded || !m_sprParser.getSpriteCount()) { // Check if SPR was loaded
        return;
    }

    // The `transparency` option from preferences should be used here.
    // For now, hardcoding true. This should be passed from loadClient.
    bool useTransparency = true;

    for (auto it = m_clientItems.begin(); it != m_clientItems.end(); ++it) {
        OTB::ClientItem& clientItem = it.value(); // Get a modifiable reference

        // DatParser should have populated the spriteList with placeholder Sprites containing only their IDs.
        // Now we need to load the actual compressed pixel data for each of those sprites.

        QList<OTB::Sprite> loadedSprites;
        for (const OTB::Sprite& placeholderSprite : clientItem.spriteList) {
            OTB::Sprite fullSpriteData;
            // Get the full sprite data from the parser using the ID from the placeholder
            if (m_sprParser.getSprite(placeholderSprite.id, fullSpriteData, useTransparency)) {
                loadedSprites.append(fullSpriteData);
            } else {
                // If a sprite can't be loaded, append a blank one to keep indices correct.
                OTB::Sprite blankSprite;
                blankSprite.id = placeholderSprite.id; // Keep the original requested ID
                loadedSprites.append(blankSprite);
                qWarning() << "RealPlugin770: Could not load sprite with ID" << placeholderSprite.id
                           << "for ClientItem" << clientItem.id;
            }
        }
        // Replace the list of placeholder sprites with the list of fully loaded sprites.
        clientItem.spriteList = loadedSprites;

        // After populating spriteList, ClientItem::getSpriteHash() and ClientItem::getBitmap()
        // can now work with the actual data.
    }
}
