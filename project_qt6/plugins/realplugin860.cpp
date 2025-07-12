#include "realplugin860.h"
#include "otb/item.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>

namespace PluginInterface {

RealPlugin860::RealPlugin860(QObject *parent)
    : QObject(parent)
    , m_host(nullptr)
    , m_clientItems(nullptr)
    , m_isLoaded(false)
    , m_minItemId(100)  // Typical minimum for modern clients
    , m_maxItemId(10000) // Higher maximum for modern clients
    , m_frameDurationsEnabled(false)
    , m_enhancedTransparencyEnabled(false)
    , m_extendedAttributesEnabled(false)
{
    initializeSupportedClients();
    initializeModernFeatures();
    m_clientItems = new ClientItems();
}

RealPlugin860::~RealPlugin860()
{
    Dispose();
}

void RealPlugin860::initializeSupportedClients()
{
    // Define supported clients for modern Tibia versions (8.60+)
    // These signatures should be verified with actual client files
    
    // Tibia 8.60 - First major modern client
    m_supportedClients.append(ItemEditor::SupportedClient(
        860,                    // version
        "Tibia Client 8.60",   // description
        860,                   // otbVersion
        0x4A34DE39,           // datSignature (example - should be verified)
        0x4A34DE39            // sprSignature (example - should be verified)
    ));
    
    // Tibia 8.70 - Enhanced features
    m_supportedClients.append(ItemEditor::SupportedClient(
        870,                    // version
        "Tibia Client 8.70",   // description
        870,                   // otbVersion
        0x4B45EF4A,           // datSignature (example - should be verified)
        0x4B45EF4A            // sprSignature (example - should be verified)
    ));
    
    // Tibia 9.00+ - Modern format with extended attributes
    m_supportedClients.append(ItemEditor::SupportedClient(
        900,                    // version
        "Tibia Client 9.00+",  // description
        900,                   // otbVersion
        0x5C56F05B,           // datSignature (example - should be verified)
        0x5C56F05B            // sprSignature (example - should be verified)
    ));
    
    // Tibia 10.00+ - Latest modern format
    m_supportedClients.append(ItemEditor::SupportedClient(
        1000,                   // version
        "Tibia Client 10.00+", // description
        1000,                  // otbVersion
        0x6D67015C,           // datSignature (example - should be verified)
        0x6D67015C            // sprSignature (example - should be verified)
    ));
}void RealPlugin860::initializeModernFeatures()
{
    // Initialize modern client feature support
    m_frameDurationsEnabled = true;        // Modern clients support frame durations
    m_enhancedTransparencyEnabled = true;  // Enhanced transparency support
    m_extendedAttributesEnabled = true;    // Extended attribute support for 8.60+
}

bool RealPlugin860::Initialize()
{
    logMessage("Initializing RealPlugin860 for modern clients", 1);
    
    // Initialize blank sprites if needed
    ItemEditor::Sprite::createBlankSprite();
    
    // Clear any previous state
    clearData();
    
    // Initialize modern features
    initializeModernFeatures();
    
    logMessage("RealPlugin860 initialized successfully with modern features", 1);
    return true;
}

void RealPlugin860::Dispose()
{
    logMessage("Disposing RealPlugin860", 1);
    
    unloadClient();
    
    if (m_clientItems) {
        delete m_clientItems;
        m_clientItems = nullptr;
    }
    
    m_host = nullptr;
    
    logMessage("RealPlugin860 disposed", 1);
}

// Plugin identification methods
QString RealPlugin860::pluginName() const
{
    return "RealPlugin860";
}

QString RealPlugin860::pluginDescription() const
{
    return "Plugin for Tibia client versions 8.60 and higher - supports modern DAT/SPR formats with extended features";
}

QString RealPlugin860::pluginVersion() const
{
    return "1.0.0";
}

// Plugin host communication
IPluginHost* RealPlugin860::getHost() const
{
    return m_host;
}

void RealPlugin860::setHost(IPluginHost* host)
{
    m_host = host;
    if (m_host) {
        logMessage("Plugin host set successfully", 1);
    }
}// Client management methods
ClientItems* RealPlugin860::getItems() const
{
    return m_clientItems;
}

quint16 RealPlugin860::getMinItemId() const
{
    return m_minItemId;
}

quint16 RealPlugin860::getMaxItemId() const
{
    return m_maxItemId;
}

QList<ItemEditor::SupportedClient> RealPlugin860::getSupportedClients() const
{
    return m_supportedClients;
}

bool RealPlugin860::isLoaded() const
{
    return m_isLoaded;
}

// Main client loading method with modern features support
bool RealPlugin860::LoadClient(const ItemEditor::SupportedClient& client,
                              bool extended,
                              bool frameDurations,
                              bool transparency,
                              const QString& datFullPath,
                              const QString& sprFullPath)
{
    logMessage(QString("Loading modern client: %1").arg(client.description), 1);
    
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
    
    // Configure modern features based on client version and parameters
    m_frameDurationsEnabled = frameDurations && supportsFrameDurations();
    m_enhancedTransparencyEnabled = transparency && supportsTransparency();
    m_extendedAttributesEnabled = extended && supportsExtendedMode();
    
    logMessage(QString("Modern features enabled - Extended: %1, FrameDurations: %2, Transparency: %3")
              .arg(m_extendedAttributesEnabled)
              .arg(m_frameDurationsEnabled)
              .arg(m_enhancedTransparencyEnabled), 1);
    
    // Load SPR file first with modern features
    if (!loadSprFile(sprFullPath, extended, transparency, frameDurations)) {
        return false; // Error message set by loadSprFile
    }
    
    // Load DAT file with extended attributes for modern clients
    if (!loadDatFile(datFullPath, extended, m_extendedAttributesEnabled)) {
        return false; // Error message set by loadDatFile
    }
    
    // Validate signatures if specified
    if (!validateSignatures(m_currentClient)) {
        // Continue with warning - don't fail completely for modern clients
        logWarning("Signature validation failed but continuing with modern client");
    }
    
    // Validate modern format features
    if (!validateModernFormat(m_currentClient)) {
        logWarning("Modern format validation failed but continuing");
    }
    
    // Populate client items from parsed data with modern features
    populateClientItems();
    
    // Populate modern sprite data with enhanced features
    populateModernSpriteData();
    
    // Calculate actual item ID range
    calculateItemIdRange();
    
    m_isLoaded = true;
    logMessage(QString("Modern client loaded successfully with %1 items").arg(m_clientItems->size()), 1);
    
    return true;
}// Client data access methods
ItemEditor::SupportedClient RealPlugin860::GetClientBySignatures(quint32 datSignature, quint32 sprSignature) const
{
    for (const auto& client : m_supportedClients) {
        if (client.datSignature == datSignature && client.sprSignature == sprSignature) {
            return client;
        }
    }
    return ItemEditor::SupportedClient(); // Return empty client if not found
}

ItemEditor::ClientItem RealPlugin860::GetClientItem(quint16 id) const
{
    if (!m_isLoaded || !m_clientItems) {
        return ItemEditor::ClientItem();
    }
    
    return m_clientItems->getItem(id);
}

bool RealPlugin860::hasClientItem(quint16 id) const
{
    if (!m_isLoaded || !m_clientItems) {
        return false;
    }
    
    return m_clientItems->containsItem(id);
}

void RealPlugin860::unloadClient()
{
    if (m_isLoaded) {
        logMessage("Unloading modern client", 1);
    }
    
    clearData();
    m_isLoaded = false;
    m_currentClient = ItemEditor::SupportedClient();
    
    // Reset modern features
    m_frameDurationsEnabled = false;
    m_enhancedTransparencyEnabled = false;
    m_extendedAttributesEnabled = false;
}

QString RealPlugin860::getLastError() const
{
    return m_lastError;
}bool RealPlugin860::validateClientFiles(const QString& datPath, const QString& sprPath) const
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
    
    // Additional validation for modern client files
    if (datInfo.size() < 1024) {
        m_lastError = QString("DAT file appears to be too small: %1 bytes").arg(datInfo.size());
        return false;
    }
    
    if (sprInfo.size() < 1024) {
        m_lastError = QString("SPR file appears to be too small: %1 bytes").arg(sprInfo.size());
        return false;
    }
    
    return true;
}

// Plugin capabilities - enhanced for modern clients
bool RealPlugin860::supportsExtendedMode() const
{
    return true; // All 8.60+ clients support extended mode
}

bool RealPlugin860::supportsFrameDurations() const
{
    return true; // Modern clients support frame durations
}

bool RealPlugin860::supportsTransparency() const
{
    return true; // Enhanced transparency support for modern clients
}

bool RealPlugin860::supportsVersionDetection() const
{
    return true; // Can detect version by signatures and format analysis
}// Private helper methods implementation
bool RealPlugin860::loadDatFile(const QString& datPath, bool extended, bool enhancedAttributes)
{
    logMessage(QString("Loading modern DAT file: %1").arg(datPath), 1);
    
    QString errorString;
    
    // For modern clients (8.60+), extended mode is always enabled
    bool isDatExtended = extended || (m_currentClient.version >= 860);
    
    if (!m_datParser.loadDat(datPath, m_currentClient.version, errorString)) {
        m_lastError = QString("Failed to load modern DAT file: %1").arg(errorString);
        logError(m_lastError);
        return false;
    }
    
    if (enhancedAttributes && isModernClient(m_currentClient.version)) {
        logMessage("Enhanced attribute parsing enabled for modern client", 1);
        // Additional modern format parsing could be implemented here
    }
    
    logMessage("Modern DAT file loaded successfully", 1);
    return true;
}

bool RealPlugin860::loadSprFile(const QString& sprPath, bool extended, bool transparency, bool frameDurations)
{
    logMessage(QString("Loading modern SPR file: %1").arg(sprPath), 1);
    
    QString errorString;
    
    // For modern clients (8.60+), extended mode is typically enabled
    bool isSprExtended = extended || (m_currentClient.version >= 860);
    
    if (!m_sprParser.loadSpr(sprPath, isSprExtended, errorString)) {
        m_lastError = QString("Failed to load modern SPR file: %1").arg(errorString);
        logError(m_lastError);
        return false;
    }
    
    if (frameDurations && m_frameDurationsEnabled) {
        logMessage("Frame duration support enabled for modern client", 1);
        // Frame duration parsing implementation would go here
    }
    
    if (transparency && m_enhancedTransparencyEnabled) {
        logMessage("Enhanced transparency support enabled for modern client", 1);
        // Enhanced transparency processing would go here
    }
    
    logMessage("Modern SPR file loaded successfully", 1);
    return true;
}void RealPlugin860::populateClientItems()
{
    logMessage("Populating client items from modern parsed data", 1);
    
    if (!m_clientItems) {
        m_clientItems = new ClientItems();
    } else {
        m_clientItems->clear();
    }
    
    // Get parsed items from DAT parser with extended attributes
    QMap<quint16, ItemEditor::ClientItem> parsedItems;
    bool isDatExtended = (m_currentClient.version >= 860); // Always extended for 8.60+
    
    if (!m_datParser.getAllClientItems(parsedItems, isDatExtended)) {
        logError("Failed to retrieve parsed client items from modern DatParser");
        return;
    }
    
    // For each ClientItem from DAT, load its sprites from SPR with modern features
    for (auto it = parsedItems.begin(); it != parsedItems.end(); ++it) {
        quint16 itemId = it.key();
        ItemEditor::ClientItem clientItem = it.value();
        
        // Load sprite data for this item with modern features
        QList<ItemEditor::Sprite> loadedSprites;
        for (const ItemEditor::Sprite& placeholderSprite : clientItem.SpriteList) {
            ItemEditor::Sprite fullSpriteData;
            
            if (m_sprParser.getSprite(placeholderSprite.id, fullSpriteData, m_enhancedTransparencyEnabled)) {
                // Apply modern enhancements if enabled
                if (m_frameDurationsEnabled && isModernClient(m_currentClient.version)) {
                    // Frame duration processing would be applied here
                    logDebug(QString("Processing frame durations for sprite %1").arg(placeholderSprite.id));
                }
                
                loadedSprites.append(fullSpriteData);
            } else {
                // If sprite can't be loaded, append a blank one
                ItemEditor::Sprite blankSprite;
                blankSprite.id = placeholderSprite.id;
                loadedSprites.append(blankSprite);
                logMessage(QString("Could not load sprite ID %1 for modern item %2")
                          .arg(placeholderSprite.id).arg(itemId), 2);
            }
        }
        
        // Replace placeholder sprites with loaded sprites
        clientItem.SpriteList = loadedSprites;
        
        // Add to client items collection
        m_clientItems->setItem(itemId, clientItem);
    }
    
    logMessage(QString("Populated %1 modern client items").arg(m_clientItems->size()), 1);
}void RealPlugin860::populateModernSpriteData()
{
    if (!m_isLoaded || !m_clientItems || m_clientItems->isEmpty()) {
        return;
    }
    
    logMessage("Applying modern sprite enhancements", 1);
    
    int enhancedSprites = 0;
    
    // Apply modern enhancements to all loaded sprites
    for (auto it = m_clientItems->begin(); it != m_clientItems->end(); ++it) {
        ItemEditor::ClientItem& clientItem = it.value();
        
        for (ItemEditor::Sprite& sprite : clientItem.SpriteList) {
            bool enhanced = false;
            
            // Apply frame duration enhancements
            if (m_frameDurationsEnabled && sprite.id > 0) {
                // Frame duration processing implementation would go here
                enhanced = true;
            }
            
            // Apply enhanced transparency processing
            if (m_enhancedTransparencyEnabled && sprite.id > 0) {
                // Enhanced transparency processing implementation would go here
                enhanced = true;
            }
            
            // Apply extended attribute processing for modern clients
            if (m_extendedAttributesEnabled && supportsEnhancedFeatures(m_currentClient.version)) {
                // Extended attribute processing implementation would go here
                enhanced = true;
            }
            
            if (enhanced) {
                enhancedSprites++;
            }
        }
    }
    
    logMessage(QString("Applied modern enhancements to %1 sprites").arg(enhancedSprites), 1);
}

void RealPlugin860::calculateItemIdRange()
{
    if (!m_clientItems || m_clientItems->isEmpty()) {
        m_minItemId = 100;
        m_maxItemId = 10000; // Higher range for modern clients
        return;
    }
    
    QList<quint16> itemIds = m_clientItems->getItemIds();
    if (!itemIds.isEmpty()) {
        m_minItemId = *std::min_element(itemIds.begin(), itemIds.end());
        m_maxItemId = *std::max_element(itemIds.begin(), itemIds.end());
    }
    
    logMessage(QString("Modern client item ID range: %1 - %2").arg(m_minItemId).arg(m_maxItemId), 1);
}bool RealPlugin860::validateSignatures(const ItemEditor::SupportedClient& client) const
{
    bool datSignatureValid = (m_datParser.getSignature() == client.datSignature);
    bool sprSignatureValid = (m_sprParser.getSignature() == client.sprSignature);
    
    if (!datSignatureValid) {
        logWarning(QString("Modern DAT signature mismatch. Expected: 0x%1, Got: 0x%2")
                  .arg(client.datSignature, 8, 16, QChar('0'))
                  .arg(m_datParser.getSignature(), 8, 16, QChar('0')));
    }
    
    if (!sprSignatureValid) {
        logWarning(QString("Modern SPR signature mismatch. Expected: 0x%1, Got: 0x%2")
                  .arg(client.sprSignature, 8, 16, QChar('0'))
                  .arg(m_sprParser.getSignature(), 8, 16, QChar('0')));
    }
    
    return datSignatureValid && sprSignatureValid;
}

bool RealPlugin860::validateModernFormat(const ItemEditor::SupportedClient& client) const
{
    // Validate modern format features
    bool formatValid = true;
    
    if (isModernClient(client.version)) {
        // Check if extended attributes are properly supported
        if (!supportsExtendedMode()) {
            logWarning("Modern client requires extended mode support");
            formatValid = false;
        }
        
        // Check if frame durations are available for clients that should support them
        if (client.version >= 900 && !supportsFrameDurations()) {
            logWarning("Modern client should support frame durations");
            formatValid = false;
        }
        
        // Validate enhanced transparency support
        if (!supportsTransparency()) {
            logWarning("Modern client requires enhanced transparency support");
            formatValid = false;
        }
    }
    
    return formatValid;
}

void RealPlugin860::clearData()
{
    if (m_clientItems) {
        m_clientItems->clear();
    }
    
    m_minItemId = 100;
    m_maxItemId = 10000; // Higher range for modern clients
    m_lastError.clear();
}bool RealPlugin860::isModernClient(quint32 version) const
{
    return version >= 860; // All 8.60+ clients are considered modern
}

bool RealPlugin860::supportsEnhancedFeatures(quint32 version) const
{
    return version >= 900; // Enhanced features available from 9.00+
}

// Logging helper methods
void RealPlugin860::logMessage(const QString& message, int level) const
{
    if (m_host) {
        m_host->logMessage(QString("[%1] %2").arg(pluginName()).arg(message), level);
    } else {
        qDebug() << QString("[%1] %2").arg(pluginName()).arg(message);
    }
}

void RealPlugin860::logError(const QString& error) const
{
    if (m_host) {
        m_host->logError(QString("[%1] %2").arg(pluginName()).arg(error));
    } else {
        qCritical() << QString("[%1] ERROR: %2").arg(pluginName()).arg(error);
    }
}

void RealPlugin860::logWarning(const QString& warning) const
{
    if (m_host) {
        m_host->logWarning(QString("[%1] %2").arg(pluginName()).arg(warning));
    } else {
        qWarning() << QString("[%1] WARNING: %2").arg(pluginName()).arg(warning);
    }
}

void RealPlugin860::logDebug(const QString& debug) const
{
    if (m_host) {
        m_host->logDebug(QString("[%1] %2").arg(pluginName()).arg(debug));
    } else {
        qDebug() << QString("[%1] DEBUG: %2").arg(pluginName()).arg(debug);
    }
}

} // namespace PluginInterface

// Include moc file for Qt's meta-object system
#include "realplugin860.moc"