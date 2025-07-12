#include "datparser.h"
#include <QDebug>

namespace TibiaData {

DatParser::DatParser() : m_signature(0), m_itemCount(0), m_outfitCount(0), m_effectCount(0), m_missileCount(0), m_clientVersion(0)
{
}

bool DatParser::loadDat(const QString& filePath, quint32 clientVersion, QString& errorString)
{
    m_clientVersion = clientVersion; // Store for parsing logic that might depend on it
    m_parsedClientItems.clear();

    m_file.setFileName(filePath);
    if (!m_file.open(QIODevice::ReadOnly)) {
        errorString = QObject::tr("Failed to open DAT file: %1 - %2").arg(filePath, m_file.errorString());
        return false;
    }

    QDataStream stream(&m_file);
    stream.setByteOrder(QDataStream::LittleEndian);

    // Read signature (4 bytes)
    stream >> m_signature;

    // Read counts
    stream >> m_itemCount;    // Max Item ID + 1 (or similar, usually 100-based)
    stream >> m_outfitCount;  // Max Outfit ID + 1
    stream >> m_effectCount;  // Max Effect ID + 1
    stream >> m_missileCount; // Max Missile ID + 1

    qDebug() << "DAT Signature:" << Qt::hex << m_signature;
    qDebug() << "DAT Item Count (max ID type):" << m_itemCount;
    qDebug() << "DAT Outfit Count:" << m_outfitCount;
    qDebug() << "DAT Effect Count:" << m_effectCount;
    qDebug() << "DAT Missile Count:" << m_missileCount;

    if (m_itemCount == 0 || m_itemCount > 0xFFFF) { // Sanity check for typical item ID range
        errorString = QObject::tr("Invalid item count in DAT file: %1").arg(m_itemCount);
        m_file.close();
        return false;
    }

    // The DAT file contains data for items, outfits, effects, missiles.
    // Item Editor is primarily concerned with items. Client IDs for items start from 100.
    // We need to loop from Client ID 100 up to itemCount - 1 (or itemCount if it's max actual ID).
    // C# ItemEditor.PluginInterface.IPlugin has MinItemId/MaxItemId.
    // For now, let's assume items are from 100 up to m_itemCount (exclusive or inclusive needs checking against format).
    // Typically, loop is for (id = 100; id < itemCount; ++id)

    // For now, we'll just read the headers.
    // The actual parsing of each "thing" will be done in getAllClientItems or a similar method
    // that iterates from ClientID 100 up to m_itemCount.
    // The file position after reading headers is the start of the first item's (ID 100) attributes.

    // We can choose to parse all items now and store them, or parse on demand.
    // Given ItemEditor usually loads all client items, let's parse them here.

    bool extendedFormat = (m_clientVersion >= 960); // Example, C# uses this logic for SPR options

    for (quint16 clientId = 100; clientId < m_itemCount; ++clientId) {
        OTB::ClientItem currentClientItem;
        currentClientItem.id = clientId; // Set the ID before parsing attributes for it

        if (!parseThing(stream, clientId, currentClientItem, extendedFormat)) {
            // errorString = QObject::tr("Failed to parse item with ClientID %1.").arg(clientId);
            // It's common for DATs to have "gaps" or unparseable sections for unused IDs.
            // We might just log a warning and continue, not adding this item.
            qWarning() << "DAT Parser: Failed or skipped parsing for ClientID" << clientId << "at stream pos" << stream.device()->pos();
            // To skip to the next item, we need to know how DAT indicates end of attributes or if it's fixed size.
            // DAT format: loop until 0xFF (end of attributes for current item).
            // The parseThing should consume until 0xFF. If it fails mid-way, stream might be mispositioned.
            // This needs robust handling of the DAT format's attribute list termination.
            // For now, if parseThing fails, we might be stuck.
            // A robust parser would try to find the next 0xFF or next valid item start.
            // Let's assume parseThing correctly consumes its section or reports error if stream ends.
            if (stream.atEnd()) {
                 errorString = QObject::tr("Unexpected end of DAT file while parsing item %1.").arg(clientId);
                 m_file.close();
                 return false;
            }
            // If parseThing failed but didn't consume to FF, we need to advance stream.
            // This is complex without knowing exact DAT structure variations.
            // Simplification: Assume parseThing handles its segment.
        } else {
             m_parsedClientItems.insert(clientId, currentClientItem);
        }
        if(stream.atEnd() && (clientId < m_itemCount -1) ){
             errorString = QObject::tr("DAT file ended prematurely after item %1.").arg(clientId);
             m_file.close();
             return false;
        }
    }

    qDebug() << "DAT file" << filePath << "loaded and parsed. Total client items stored:" << m_parsedClientItems.count();
    m_file.close(); // Close after parsing all into memory
    return true;
}

bool DatParser::getAllClientItems(QMap<quint16, OTB::ClientItem>& outItemsMap, bool extendedFormat) const
{
    Q_UNUSED(extendedFormat); // extendedFormat was used during initial parsing in loadDat
    if (m_parsedClientItems.isEmpty() && m_itemCount > 0) {
        // This implies loadDat didn't parse items, or failed.
        // Or, if we change loadDat to not parse all upfront, this method would trigger parsing.
        qWarning() << "DatParser::getAllClientItems: No items were parsed during loadDat, or itemCount is 0.";
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

// Parses attributes for a single "thing" (item) starting at the current stream position
// Consumes bytes until 0xFF (end of attributes marker) is found.
bool DatParser::parseThing(QDataStream& stream, quint16 clientId, OTB::ClientItem& outItem, bool extendedFormat) const
{
    Q_UNUSED(extendedFormat); // May be needed for version-specific attribute handling

    // Reset relevant ClientItem fields before parsing
    outItem = OTB::ClientItem(); // Default construct
    outItem.id = clientId;       // Set its ID

    bool hasMoreAttributes = true;
    while(hasMoreAttributes && !stream.atEnd()) {
        quint8 attributeCode;
        stream >> attributeCode;

        if (attributeCode == static_cast<quint8>(DatAttribute::Unknown) || attributeCode == 0xFF) { // End of attributes for this item
            hasMoreAttributes = false;
            break;
        }

        DatAttribute attr = static_cast<DatAttribute>(attributeCode);
        quint16 dataLen; // For attributes that have a length field (like Light, MinimapColor)

        // This is a simplified version. Real DAT parsing is much more complex
        // and attribute-dependent for how data is read.
        // Many attributes are just flags (presence means true).
        // Some (like Light, MinimapColor) have subsequent data bytes.
        // Others (like GroundSpeed, MaxTextLen) are often part of item type definition in server
        // or derived, not direct simple attributes for all client versions.

        switch(attr) {
            case DatAttribute::Unpassable: outItem.unpassable = true; break; // Flag-like
            case DatAttribute::BlockSolid: outItem.unpassable = true; break; // Often equivalent
            case DatAttribute::BlockProjectile: outItem.blockMissiles = true; break;
            case DatAttribute::BlockPath: outItem.blockPathfinder = true; break;
            case DatAttribute::Height: outItem.hasElevation = true; break; // This means "has elevation value"
                // For HasElevation, DAT often stores an actual elevation value (e.g. 1 byte)
                // stream >> elevation_value; outItem.elevation = elevation_value; (if ClientItem had elevation field)
                // This is highly client version dependent.
                // For now, just treating as a flag.
                break;
            case DatAttribute::IsUsable: // Generic usable, might map to MultiUse or ForceUse
            case DatAttribute::Usable:
            case DatAttribute::ForceUse:
            case DatAttribute::MultiUse:
                 outItem.multiUse = true; // Or map to a specific one based on client version
                 break;
            case DatAttribute::Take: outItem.pickupable = true; break;
            case DatAttribute::Unmovable: outItem.movable = false; break; // Note: sets to false
            case DatAttribute::Stackable: outItem.stackable = true; break;
            case DatAttribute::Look: /* Is look-through-able */ break;
            case DatAttribute::Rotatable: outItem.rotatable = true; break;
            case DatAttribute::Hangable: outItem.hangable = true; break;
            case DatAttribute::HookSouth: outItem.hookSouth = true; break;
            case DatAttribute::HookEast: outItem.hookEast = true; break;
            case DatAttribute::IgnoreLook: outItem.ignoreLook = true; break; // This is from OTB flags, might be different in DAT
            case DatAttribute::FullGround: outItem.fullGround = true; break;
            case DatAttribute::AnimateAlways: outItem.isAnimation = true; break; // This is a guess, DAT has more complex animation data

            case DatAttribute::Light: // Has 2 bytes data: LightLevel, LightColor
                stream >> dataLen; // Should be 2 or 4 depending on version for Light (level+color)
                                   // OTLib suggests 2 bytes for level, 2 for color for some attributes
                                   // C# Item.cs has LightLevel, LightColor as ushorts
                if (dataLen == 2 && m_clientVersion < 840) { // Older clients might just have level
                     stream >> outItem.lightLevel;
                     outItem.lightColor = 0; // No color data
                } else if (dataLen == 4 || (dataLen == 2 && m_clientVersion >= 840) ) { // Newer often 2 bytes for light (level+color combined or just level)
                                                                    // This is very version specific.
                                                                    // Assuming 2 bytes for combined level+color for now if dataLen is 2 for newer
                                                                    // Or if dataLen is 4, then 2 for level, 2 for color.
                     // This part of DAT parsing is notoriously tricky and version-dependent.
                     // C# ItemEditor's DAT parsing (inside plugins) would have the precise logic.
                     // For now, let's assume if dataLen is 2, it's level, if 4, it's level+color.
                     // This is a major simplification.
                    if (dataLen == 2) { // Simplified: assume this is level, color is default or not set
                        stream >> outItem.lightLevel;
                        // outItem.lightColor might remain 0 or be derived.
                    } else if (dataLen == 4) { // Common for newer clients
                        stream >> outItem.lightLevel; // WORD
                        stream >> outItem.lightColor; // WORD
                    } else {
                         qWarning() << "DAT: Unexpected dataLen" << dataLen << "for Light attribute, ClientID" << clientId;
                         stream.skipRawData(dataLen);
                    }
                } else {
                     qWarning() << "DAT: Unexpected dataLen" << dataLen << "for Light attribute, ClientID" << clientId;
                     stream.skipRawData(dataLen);
                }
                break;

            case DatAttribute::MinimapColor: // Has 2 bytes data
                stream >> dataLen; // Should be 2
                if (dataLen == 2) {
                    stream >> outItem.minimapColor;
                } else {
                    qWarning() << "DAT: Unexpected dataLen" << dataLen << "for MinimapColor, ClientID" << clientId;
                    stream.skipRawData(dataLen);
                }
                break;

            // Writable, WritableOnce, MaxTextLen etc. are often not simple flags/attributes in newer DATs
            // but part of item type definitions or implied.
            // For example, readable might be a separate flag or derived from MaxTextLen > 0.
            // The C# Item.cs properties (Readable, MaxReadChars, MaxReadWriteChars) are what we need to populate.
            // These are often set by server OTB or scripts, not directly from simple DAT flags for all versions.
            // The current ClientItem struct reflects OTB/ServerItem properties more.
            // True DAT parsing would map raw DAT attributes to these concepts.

            // Placeholder for other attributes that consume data
            // case DatAttribute::Writable: stream >> len; stream.skipRawData(len); break;
            // case DatAttribute::Shift: stream >> len; stream.skipRawData(len); break; // x,y offset
            // case DatAttribute::Height: stream >> len; stream.skipRawData(len); break; // elevation value

            default:
                // This is where detailed knowledge of each attribute for each client version is needed.
                // Many attributes are just 1 byte (the code itself), some are followed by data.
                // The C# ItemEditor plugins contain this detailed parsing logic.
                // For now, we assume unknown attributes are just flags or we misinterpret them.
                // A robust DAT parser needs a table of (AttributeCode, DataSizeToRead) for each client version.
                // qWarning() << "DAT: Unhandled attribute code" << Qt::hex << attributeCode << "for ClientID" << clientId;
                // If an attribute has data, we MUST skip it correctly.
                // This part is highly simplified and likely incorrect for many attributes.
                break;
        }
        if(stream.atEnd() && hasMoreAttributes) { // Check stream status after each attribute read
            qWarning() << "DAT: Stream ended prematurely while parsing attributes for ClientID" << clientId;
            return false; // Critical error, can't continue parsing this item
        }
    }
    // After loop, stream should be at 0xFF (or EOF if it was the last item)
    // If hasMoreAttributes is false, it means 0xFF was correctly found and consumed by `stream >> attributeCode;`
    return !hasMoreAttributes; // True if 0xFF was found.
}


} // namespace TibiaData
