#include "tibiadata/datparser.h"
#include <QDebug>

namespace TibiaData {

DatParser::DatParser() : m_signature(0), m_itemCount(0), m_outfitCount(0), m_effectCount(0), m_missileCount(0), m_clientVersion(0)
{
}

bool DatParser::loadDat(const QString& filePath, quint32 clientVersion, QString& errorString)
{
    m_clientVersion = clientVersion;
    m_parsedClientItems.clear();

    m_file.setFileName(filePath);
    if (!m_file.open(QIODevice::ReadOnly)) {
        errorString = QObject::tr("Failed to open DAT file: %1 - %2").arg(filePath, m_file.errorString());
        return false;
    }

    QDataStream stream(&m_file);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream >> m_signature;
    stream >> m_itemCount;
    stream >> m_outfitCount;
    stream >> m_effectCount;
    stream >> m_missileCount;

    qDebug() << "DAT Signature:" << Qt::hex << m_signature;
    qDebug() << "DAT Item Count:" << m_itemCount;
    qDebug() << "DAT Outfit Count:" << m_outfitCount;
    qDebug() << "DAT Effect Count:" << m_effectCount;
    qDebug() << "DAT Missile Count:" << m_missileCount;

    if (m_itemCount == 0 || m_itemCount > 0xFFFF) {
        errorString = QObject::tr("Invalid item count in DAT file: %1").arg(m_itemCount);
        m_file.close();
        return false;
    }

    // This flag helps select the parsing path inside parseThing
    bool isExtended = (m_clientVersion >= 780);

    for (quint16 clientId = 100; clientId < m_itemCount; ++clientId) {
        ItemEditor::ClientItem currentClientItem;
        currentClientItem.ID = clientId; // Pass the ID to the parser

        if (!parseThing(stream, currentClientItem, isExtended)) {
            if (stream.atEnd()) {
                errorString = QObject::tr("Unexpected end of DAT file while parsing item %1.").arg(clientId);
                m_file.close();
                return false;
            }
        } else {
             m_parsedClientItems.insert(clientId, currentClientItem);
        }
    }

    qDebug() << "DAT file" << filePath << "loaded and parsed. Total client items stored:" << m_parsedClientItems.count();
    m_file.close();
    return true;
}

bool DatParser::getAllClientItems(QMap<quint16, ItemEditor::ClientItem>& outItemsMap, bool extendedFormat) const
{
    Q_UNUSED(extendedFormat);
    if (m_parsedClientItems.isEmpty()) {
        qWarning() << "DatParser::getAllClientItems: No items were parsed from DAT file.";
        return false;
    }
    outItemsMap = m_parsedClientItems;
    return true;
}


quint32 DatParser::getSignature() const { return m_signature; }
quint16 DatParser::getItemCount() const { return m_itemCount; }
quint16 DatParser::getOutfitCount() const { return m_outfitCount; }
quint16 DatParser::getEffectCount() const { return m_effectCount; }
quint16 DatParser::getMissileCount() const { return m_missileCount; }


bool TibiaData::DatParser::parseThing(QDataStream& stream, ItemEditor::ClientItem& outItem, bool isExtended) const
{
    if (!isExtended) {
        qWarning() << "DAT Parser: Fixed-structure DAT format for clients < 7.80 not yet supported. Skipping item" << outItem.ID;
        // Without knowing the fixed size, we cannot reliably skip. This will likely fail.
        return false;
    }

    // --- Attribute-based parsing (clients >= 7.8) ---
    bool hasMoreAttributes = true;
    while(hasMoreAttributes && !stream.atEnd()) {
        quint8 attributeCode;
        stream >> attributeCode;

        if (attributeCode == 0xFF) { // End of attributes for this item
            hasMoreAttributes = false;
            break;
        }

        DatAttribute attr = static_cast<DatAttribute>(attributeCode);

        // This switch handles attributes that have data following them.
        // If an attribute is just a flag, its presence is handled by the default case.
        switch(attributeCode) { // Use the raw attribute code instead of enum
            case static_cast<quint8>(TibiaData::DatAttribute::Ground):              stream >> outItem.GroundSpeed; break;
            case static_cast<quint8>(TibiaData::DatAttribute::MinimapColor):        stream >> outItem.MinimapColor; break;
            case static_cast<quint8>(TibiaData::DatAttribute::Light): {
                quint16 lightLevel, lightColor;
                stream >> lightLevel >> lightColor;
                outItem.LightLevel = lightLevel;
                outItem.LightColor = lightColor;
                break;
            }
            case static_cast<quint8>(TibiaData::DatAttribute::Height): {
                quint16 elevation;
                stream >> elevation; // Read the data
                break;
            }
            // Add cases for other attributes that have data...
            default:
                // This is a flag-like attribute. We set the property and continue.
                break;
        }

        // Set boolean properties based on flags
        switch(attributeCode) { // Use the raw attribute code instead of enum
            case static_cast<quint8>(TibiaData::DatAttribute::Unpassable):          outItem.Unpassable = true; break;
            // Note: BlockProjectile, BlockPath, IsUsable, IgnoreLook are in DatItemFlag namespace, not DatAttribute enum
            case static_cast<quint8>(TibiaData::DatAttribute::Take):                outItem.Pickupable = true; break;
            case static_cast<quint8>(TibiaData::DatAttribute::Unmovable):           outItem.Movable = false; break;
            case static_cast<quint8>(TibiaData::DatAttribute::Stackable):           outItem.Stackable = true; break;
            case static_cast<quint8>(TibiaData::DatAttribute::Rotatable):           outItem.Rotatable = true; break;
            case static_cast<quint8>(TibiaData::DatAttribute::Hangable):            outItem.Hangable = true; break;
            case static_cast<quint8>(TibiaData::DatAttribute::HookSouth):           outItem.HookSouth = true; break;
            case static_cast<quint8>(TibiaData::DatAttribute::HookEast):            outItem.HookEast = true; break;
            case static_cast<quint8>(TibiaData::DatAttribute::FullGround):          outItem.FullGround = true; break;
            case static_cast<quint8>(TibiaData::DatAttribute::AnimateAlways):       outItem.IsAnimation = true; break;
            case static_cast<quint8>(TibiaData::DatAttribute::ForceUse):            outItem.ForceUse = true; break;
            case static_cast<quint8>(TibiaData::DatAttribute::MultiUse):            outItem.MultiUse = true; break;
            case static_cast<quint8>(TibiaData::DatAttribute::Usable):              outItem.MultiUse = true; break;
            case static_cast<quint8>(TibiaData::DatAttribute::Writable):            outItem.Readable = true; break;
            case static_cast<quint8>(TibiaData::DatAttribute::Height):              outItem.HasElevation = true; break;
            default: break; // Unknown flags are ignored for now
        }
    }

    if (stream.atEnd()) {
        qWarning() << "DAT: Stream ended before sprite information for ClientID" << outItem.ID;
        return false;
    }

    // After attributes, sprite info follows
    stream >> outItem.Width;
    stream >> outItem.Height;
    if (outItem.Width > 1 || outItem.Height > 1) {
        stream.skipRawData(1); // Skip "extra size" byte
    }
    stream >> outItem.Layers;
    stream >> outItem.PatternX;
    stream >> outItem.PatternY;

    // patternZ and frame count reading varies by version
    if (this->m_clientVersion >= 820) {
        stream >> outItem.PatternZ;
    }
    stream >> outItem.Frames;

    outItem.NumSprites = outItem.Width * outItem.Height * outItem.Layers * outItem.Frames;

    if (outItem.NumSprites > 4096) {
        qWarning() << "DAT: Item" << outItem.ID << "has an excessive number of sprites:" << outItem.NumSprites;
        return false;
    }

    outItem.SpriteList.clear();
    for (quint32 i = 0; i < outItem.NumSprites; ++i) {
        if (stream.atEnd()) {
            qWarning() << "DAT: Stream ended while reading sprite IDs for ClientID" << outItem.ID;
            return false;
        }
        quint16 spriteId;
        stream >> spriteId;
        ItemEditor::Sprite placeholderSprite;
        placeholderSprite.id = spriteId;
        outItem.SpriteList.append(placeholderSprite);
    }

    return true;
}

} // namespace TibiaData
