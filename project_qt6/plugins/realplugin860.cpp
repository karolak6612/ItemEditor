#include "realplugin860.h"
#include "otb/item.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>

RealPlugin860::RealPlugin860(QObject *parent)
    : QObject(parent), m_isClientLoaded(false)
{
    // Define supported clients for this plugin (e.g., 8.60)
    // Publicly known signatures for 8.60
    m_supportedClients.append(OTB::SupportedClient(860, "Tibia Client 8.60", 860, 0x50C69F87, 0x50C6A25A));
}

RealPlugin860::~RealPlugin860()
{
    unloadClient();
}

QString RealPlugin860::pluginName() const
{
    return "RealPlugin for Tibia 8.60";
}

QString RealPlugin860::pluginDescription() const
{
    return "Loads DAT and SPR files for Tibia client version 8.60.";
}

QList<OTB::SupportedClient> RealPlugin860::getSupportedClients() const
{
    return m_supportedClients;
}

void RealPlugin860::Initialize() {
    OTB::Sprite::createBlankSprite();
}


bool RealPlugin860::loadClient(
    const OTB::SupportedClient& clientToLoad,
    const QString& clientDirectoryPath,
    bool extended, bool frameDurations, bool transparency,
    QString& errorString)
{
    unloadClient();

    bool foundMatch = false;
    for(const auto& sc : m_supportedClients) {
        if (sc.version == clientToLoad.version) {
            m_currentlyLoadedClient = sc;
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

    m_currentlyLoadedClient.datPath = datPath;
    m_currentlyLoadedClient.sprPath = sprPath;

    // --- Load SPR File ---
    // For 8.60, extended is true.
    bool isSprExtended = extended || (m_currentlyLoadedClient.version >= 960);
    if (!m_sprParser.loadSpr(sprPath, isSprExtended, errorString)) {
        return false;
    }
    if (m_sprParser.getSignature() != m_currentlyLoadedClient.sprSignature) {
        errorString = tr("SPR file signature mismatch. Expected 0x%1, got 0x%2.")
                          .arg(m_currentlyLoadedClient.sprSignature, 8, 16, QChar('0'))
                          .arg(m_sprParser.getSignature(), 8, 16, QChar('0'));
        qWarning() << errorString;
    }

    // --- Load DAT File ---
    // For 8.60, DAT format is attribute-based (extended).
    bool isDatExtended = (m_currentlyLoadedClient.version >= 780);
    if (!m_datParser.loadDat(datPath, m_currentlyLoadedClient.version, errorString)) {
        return false;
    }
    if (m_datParser.getSignature() != m_currentlyLoadedClient.datSignature) {
        errorString = tr("DAT file signature mismatch. Expected 0x%1, got 0x%2.")
                          .arg(m_currentlyLoadedClient.datSignature, 8, 16, QChar('0'))
                          .arg(m_datParser.getSignature(), 8, 16, QChar('0'));
        qWarning() << errorString;
    }

    // --- Populate ClientItems from DAT and associate SPR data ---
    if (!m_datParser.getAllClientItems(m_clientItems, isDatExtended)) {
        errorString = tr("Failed to retrieve parsed client items from DatParser.");
        return false;
    }

    populateSpriteDataForClientItems();

    m_isClientLoaded = true;
    qDebug() << pluginName() << "loaded client" << m_currentlyLoadedClient.description
             << "with" << m_clientItems.count() << "items.";
    errorString.clear();
    return true;
}

bool RealPlugin860::isClientLoaded() const
{
    return m_isClientLoaded;
}

const OTB::SupportedClient& RealPlugin860::getCurrentLoadedClient() const
{
    if (!m_isClientLoaded) {
        static OTB::SupportedClient invalid;
        qWarning() << "getCurrentLoadedClient called when no client loaded in" << pluginName();
        return invalid;
    }
    return m_currentlyLoadedClient;
}

const QMap<quint16, OTB::ClientItem>& RealPlugin860::getClientItems() const
{
    return m_clientItems;
}

bool RealPlugin860::getClientItem(quint16 clientItemId, OTB::ClientItem& outItem) const
{
    if (!m_isClientLoaded) return false;
    return m_clientItems.contains(clientItemId) ? (outItem = m_clientItems.value(clientItemId), true) : false;
}

void RealPlugin860::unloadClient()
{
    m_clientItems.clear();
    m_isClientLoaded = false;
    m_currentlyLoadedClient = OTB::SupportedClient();
    qDebug() << pluginName() << "unloaded client data.";
}

void RealPlugin860::populateSpriteDataForClientItems() {
    if (!m_isClientLoaded || !m_sprParser.getSpriteCount()) {
        return;
    }
    bool useTransparency = true;

    for (auto it = m_clientItems.begin(); it != m_clientItems.end(); ++it) {
        OTB::ClientItem& clientItem = it.value();
        QList<OTB::Sprite> loadedSprites;
        for (const OTB::Sprite& placeholderSprite : clientItem.spriteList) {
            OTB::Sprite fullSpriteData;
            if (m_sprParser.getSprite(placeholderSprite.id, fullSpriteData, useTransparency)) {
                loadedSprites.append(fullSpriteData);
            } else {
                OTB::Sprite blankSprite;
                blankSprite.id = placeholderSprite.id;
                loadedSprites.append(blankSprite);
                qWarning() << pluginName() << ": Could not load sprite with ID" << placeholderSprite.id
                           << "for ClientItem" << clientItem.id;
            }
        }
        clientItem.spriteList = loadedSprites;
    }
}
