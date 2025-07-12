#include "realplugin770.h"
#include "otb/item.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>

namespace PluginInterface {

RealPlugin770::RealPlugin770(QObject *parent)
    : QObject(parent)
    , m_host(nullptr)
    , m_clientItems(nullptr)
    , m_isLoaded(false)
    , m_minItemId(100)  // Typical minimum for 7.70
    , m_maxItemId(2000) // Typical maximum for 7.70
{
    initializeSupportedClients();
    m_clientItems = new ClientItems();
}

RealPlugin770::~RealPlugin770()
{
    Dispose();
}

void RealPlugin770::initializeSupportedClients()
{
    // Define supported clients for Tibia 7.70
    // Signatures for 7.70 (these should be verified with actual 7.70 files)
    m_supportedClients.append(ItemEditor::SupportedClient(
        770,                    // version
        "Tibia Client 7.70",   // description
        770,                   // otbVersion
        0x4A34DE39,           // datSignature (example - should be verified)
        0x4A34DE39            // sprSignature (example - should be verified)
    ));
}

bool RealPlugin770::Initialize()
{
    logMessage("Initializing RealPlugin770", 1);
    
    // Initialize blank sprites if needed
    ItemEditor::Sprite::createBlankSprite();
    
    // Clear any previous state
    clearData();
    
    logMessage("RealPlugin770 initialized successfully", 1);
    return true;
}void RealPlugin770::Dispose()
{
    logMessage("Disposing RealPlugin770", 1);
    
    unloadClient();
    
    if (m_clientItems) {
        delete m_clientItems;
        m_clientItems = nullptr;
    }
    
    m_host = nullptr;
    
    logMessage("RealPlugin770 disposed", 1);
}

// Plugin identification methods
QString RealPlugin770::pluginName() const
{
    return "RealPlugin770";
}

QString RealPlugin770::pluginDescription() const
{
    return "Plugin for Tibia client version 7.70 - handles DAT and SPR file formats";
}

QString RealPlugin770::pluginVersion() const
{
    return "1.0.0";
}

// Plugin host communication
IPluginHost* RealPlugin770::getHost() const
{
    return m_host;
}void RealPlugin770::setHost(IPluginHost* host)
{
    m_host = host;
    if (m_host) {
        logMessage("Plugin host set successfully", 1);
    }
}

// Client management methods
ClientItems* RealPlugin770::getItems() const
{
    return m_clientItems;
}

quint16 RealPlugin770::getMinItemId() const
{
    return m_minItemId;
}

quint16 RealPlugin770::getMaxItemId() const
{
    return m_maxItemId;
}

QList<ItemEditor::SupportedClient> RealPlugin770::getSupportedClients() const
{
    return m_supportedClients;
}

bool RealPlugin770::isLoaded() const
{
    return m_isLoaded;
}// Main client loading method
bool RealPlugin770::LoadClient(const ItemEditor::SupportedClient& client,
                              bool extended,
                              bool frameDurations,
                              bool transparency,
                              const QString& datFullPath,
                              const QString& sprFullPath)
{
    logMessage(QString("Loading client: %1").arg(client.description), 1);
    
    // Clear previous state
    unloadClient();
    
    // Validate that we support this client
    bool foundMatch = false;
    for (const auto& supportedClient : m_supportedClients) {
        if (supportedClient.version == client.version) {
            m_currentClient = supportedClient;
            foundMatch = true;
            break;
        }
    }
    
    if (!foundMatch) {
        m_lastError = QString("Plugin does not support client version %1").arg(client.version);
        logError(m_lastError);
        return false;
    }
    
    // Validate file paths
    if (!validateClientFiles(datFullPath, sprFullPath)) {
        return false; // Error message set by validateClientFiles
    }    // Store actual paths used
    m_currentClient.datPath = datFullPath;
    m_currentClient.sprPath = sprFullPath;
    
    // Load SPR file first
    if (!loadSprFile(sprFullPath, extended, transparency)) {
        return false; // Error message set by loadSprFile
    }
    
    // Load DAT file
    if (!loadDatFile(datFullPath, extended)) {
        return false; // Error message set by loadDatFile
    }
    
    // Validate signatures if specified
    if (!validateSignatures(m_currentClient)) {
        // Continue with warning - don't fail completely
        logMessage("Signature validation failed but continuing", 2);
    }
    
    // Populate client items from parsed data
    populateClientItems();
    
    // Calculate actual item ID range
    calculateItemIdRange();
    
    m_isLoaded = true;
    logMessage(QString("Client loaded successfully with %1 items").arg(m_clientItems->size()), 1);
    
    return true;
}// Client data access methods
ItemEditor::SupportedClient RealPlugin770::GetClientBySignatures(quint32 datSignature, quint32 sprSignature) const
{
    for (const auto& client : m_supportedClients) {
        if (client.datSignature == datSignature && client.sprSignature == sprSignature) {
            return client;
        }
    }
    return ItemEditor::SupportedClient(); // Return empty client if not found
}

ItemEditor::ClientItem RealPlugin770::GetClientItem(quint16 id) const
{
    if (!m_isLoaded || !m_clientItems) {
        return ItemEditor::ClientItem();
    }
    
    return m_clientItems->getItem(id);
}

bool RealPlugin770::hasClientItem(quint16 id) const
{
    if (!m_isLoaded || !m_clientItems) {
        return false;
    }
    
    return m_clientItems->containsItem(id);
}

void RealPlugin770::unloadClient()
{
    if (m_isLoaded) {
        logMessage("Unloading client", 1);
    }
    
    clearData();
    m_isLoaded = false;
    m_currentClient = ItemEditor::SupportedClient();
}QString RealPlugin770::getLastError() const
{
    return m_lastError;
}

bool RealPlugin770::validateClientFiles(const QString& datPath, const QString& sprPath) const
{
    QFileInfo datInfo(datPath);
    QFileInfo sprInfo(sprPath);
    
    if (!datInfo.exists()) {
        m_lastError = QString("DAT file not found: %1").arg(datPath);
        return false;
    }
    
    if (!sprInfo.exists()) {
        m_lastError = QString("SPR file not found: %1").arg(sprPath);
        return false;
    }
    
    if (!datInfo.isReadable()) {
        m_lastError = QString("DAT file is not readable: %1").arg(datPath);
        return false;
    }
    
    if (!sprInfo.isReadable()) {
        m_lastError = QString("SPR file is not readable: %1").arg(sprPath);
        return false;
    }
    
    return true;
}

// Plugin capabilities
bool RealPlugin770::supportsExtendedMode() const
{
    return false; // 7.70 doesn't support extended mode
}bool RealPlugin770::supportsFrameDurations() const
{
    return false; // 7.70 doesn't support frame durations
}

bool RealPlugin770::supportsTransparency() const
{
    return true; // 7.70 supports transparency
}

bool RealPlugin770::supportsVersionDetection() const
{
    return true; // Can detect version by signatures
}

// Private helper methods implementation
bool RealPlugin770::loadDatFile(const QString& datPath, bool extended)
{
    logMessage(QString("Loading DAT file: %1").arg(datPath), 1);
    
    QString errorString;
    bool isDatExtended = extended || (m_currentClient.version >= 780);
    
    if (!m_datParser.loadDat(datPath, m_currentClient.version, errorString)) {
        m_lastError = QString("Failed to load DAT file: %1").arg(errorString);
        logError(m_lastError);
        return false;
    }
    
    logMessage("DAT file loaded successfully", 1);
    return true;
}

bool RealPlugin770::loadSprFile(const QString& sprPath, bool extended, bool transparency)
{
    logMessage(QString("Loading SPR file: %1").arg(sprPath), 1);
    
    QString errorString;
    bool isSprExtended = extended || (m_currentClient.version >= 960);
    
    if (!m_sprParser.loadSpr(sprPath, isSprExtended, errorString)) {
        m_lastError = QString("Failed to load SPR file: %1").arg(errorString);
        logError(m_lastError);
        return false;
    }
    
    logMessage("SPR file loaded successfully", 1);
    return true;
}void RealPlugin770::populateClientItems()
{
    logMessage("Populating client items from parsed data", 1);
    
    if (!m_clientItems) {
        m_clientItems = new ClientItems();
    } else {
        m_clientItems->clear();
    }
    
    // Get parsed items from DAT parser
    QMap<quint16, ItemEditor::ClientItem> parsedItems;
    bool isDatExtended = (m_currentClient.version >= 780);
    
    if (!m_datParser.getAllClientItems(parsedItems, isDatExtended)) {
        logError("Failed to retrieve parsed client items from DatParser");
        return;
    }
    
    // For each ClientItem from DAT, load its sprites from SPR
    bool useTransparency = true; // Should be configurable
    
    for (auto it = parsedItems.begin(); it != parsedItems.end(); ++it) {
        quint16 itemId = it.key();
        ItemEditor::ClientItem clientItem = it.value();
        
        // Load sprite data for this item
        QList<ItemEditor::Sprite> loadedSprites;
        for (const ItemEditor::Sprite& placeholderSprite : clientItem.SpriteList) {
            ItemEditor::Sprite fullSpriteData;
            
            if (m_sprParser.getSprite(placeholderSprite.id, fullSpriteData, useTransparency)) {
                loadedSprites.append(fullSpriteData);
            } else {
                // If sprite can't be loaded, append a blank one
                ItemEditor::Sprite blankSprite;
                blankSprite.id = placeholderSprite.id;
                loadedSprites.append(blankSprite);
                logMessage(QString("Could not load sprite ID %1 for item %2")
                          .arg(placeholderSprite.id).arg(itemId), 2);
            }
        }
        
        // Replace placeholder sprites with loaded sprites
        clientItem.SpriteList = loadedSprites;
        
        // Add to client items collection
        m_clientItems->setItem(itemId, clientItem);
    }
    
    logMessage(QString("Populated %1 client items").arg(m_clientItems->size()), 1);
}void RealPlugin770::calculateItemIdRange()
{
    if (!m_clientItems || m_clientItems->isEmpty()) {
        m_minItemId = 100;
        m_maxItemId = 2000;
        return;
    }
    
    QList<quint16> itemIds = m_clientItems->getItemIds();
    if (!itemIds.isEmpty()) {
        m_minItemId = *std::min_element(itemIds.begin(), itemIds.end());
        m_maxItemId = *std::max_element(itemIds.begin(), itemIds.end());
    }
    
    logMessage(QString("Item ID range: %1 - %2").arg(m_minItemId).arg(m_maxItemId), 1);
}

bool RealPlugin770::validateSignatures(const ItemEditor::SupportedClient& client) const
{
    bool datSignatureValid = (m_datParser.getSignature() == client.datSignature);
    bool sprSignatureValid = (m_sprParser.getSignature() == client.sprSignature);
    
    if (!datSignatureValid) {
        logMessage(QString("DAT signature mismatch. Expected: 0x%1, Got: 0x%2")
                  .arg(client.datSignature, 8, 16, QChar('0'))
                  .arg(m_datParser.getSignature(), 8, 16, QChar('0')), 2);
    }
    
    if (!sprSignatureValid) {
        logMessage(QString("SPR signature mismatch. Expected: 0x%1, Got: 0x%2")
                  .arg(client.sprSignature, 8, 16, QChar('0'))
                  .arg(m_sprParser.getSignature(), 8, 16, QChar('0')), 2);
    }
    
    return datSignatureValid && sprSignatureValid;
}void RealPlugin770::clearData()
{
    if (m_clientItems) {
        m_clientItems->clear();
    }
    
    m_minItemId = 100;
    m_maxItemId = 2000;
    m_lastError.clear();
}

// Logging helper methods
void RealPlugin770::logMessage(const QString& message, int level) const
{
    if (m_host) {
        m_host->logMessage(QString("[%1] %2").arg(pluginName()).arg(message), level);
    } else {
        qDebug() << QString("[%1] %2").arg(pluginName()).arg(message);
    }
}

void RealPlugin770::logError(const QString& error) const
{
    if (m_host) {
        m_host->logError(QString("[%1] %2").arg(pluginName()).arg(error));
    } else {
        qCritical() << QString("[%1] ERROR: %2").arg(pluginName()).arg(error);
    }
}

} // namespace PluginInterface

// Include moc file for Qt's meta-object system
#include "realplugin770.moc"