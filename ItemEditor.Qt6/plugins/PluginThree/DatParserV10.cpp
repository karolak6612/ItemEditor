#include "DatParserV10.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QMutexLocker>

DatParserV10::DatParserV10()
    : m_isLoaded(false)
    , m_datSignature(0)
    , m_itemCount(0)
{
}

DatParserV10::~DatParserV10()
{
    cleanup();
}

bool DatParserV10::parseFile(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);
    
    // Clean up any existing data
    cleanup();
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "DatParserV10: Failed to open DAT file:" << filePath;
        return false;
    }
    
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    try {
        // Read and validate DAT signature
        stream >> m_datSignature;
        qDebug() << "DatParserV10: Read DAT signature:" << QString::number(m_datSignature, 16).toUpper();
        if (!validateSignature(m_datSignature)) {
            qWarning() << "DatParserV10: Invalid DAT signature:" 
                      << QString::number(m_datSignature, 16).toUpper()
                      << "- This signature is not supported by Plugin Three (versions 10.00-10.77)";
            return false;
        }
        
        // Read item count and skip other counts
        stream >> m_itemCount;
        
        quint16 outfitCount, effectCount, missileCount;
        stream >> outfitCount >> effectCount >> missileCount;
        
        qDebug() << "DatParserV10: Parsing" << m_itemCount << "items from" << filePath;
        qDebug() << "DatParserV10: DAT signature:" << QString::number(m_datSignature, 16).toUpper();
        
        // Parse items starting from ID 100 (as per legacy system)
        int successfullyParsed = 0;
        for (quint16 itemId = 100; itemId <= m_itemCount && itemId < 200; ++itemId) { // Limit to first 100 items for testing
            if (!parseItemData(stream, itemId)) {
                qWarning() << "DatParserV10: Failed to parse item" << itemId << "- stopping parsing to prevent corruption";
                break; // Stop parsing but don't fail completely
            }
            successfullyParsed++;
        }
        
        qDebug() << "DatParserV10: Successfully parsed" << successfullyParsed << "items out of" << qMin(100, static_cast<int>(m_itemCount - 99)) << "attempted";
        
        m_isLoaded = true;
        qDebug() << "DatParserV10: Successfully parsed" << m_datCache.size() << "items";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "DatParserV10: Exception during parsing:" << e.what();
        cleanup();
        return false;
    }
}

bool DatParserV10::parseItemData(QDataStream& stream, quint16 itemId)
{
    DatData item;
    item.id = itemId;
    
    // Read item flags until LastFlag (0xFF)
    ItemFlag flag;
    quint8 flagByte;
    int unknownFlagCount = 0;
    const int maxUnknownFlags = 10; // Limit unknown flags to prevent infinite loops
    
    do {
        if (stream.atEnd()) {
            qWarning() << "DatParserV10: Unexpected end of stream at item" << itemId;
            return false;
        }
        
        stream >> flagByte;
        flag = static_cast<ItemFlag>(flagByte);
        
        switch (flag) {
            case ItemFlag::Ground:
                item.flags |= (1 << 0);
                stream >> item.groundSpeed;
                break;
                
            case ItemFlag::GroundBorder:
                item.flags |= (1 << 1);
                break;
                
            case ItemFlag::OnBottom:
                item.flags |= (1 << 2);
                break;
                
            case ItemFlag::OnTop:
                item.flags |= (1 << 3);
                break;
                
            case ItemFlag::Container:
                item.flags |= (1 << 4);
                break;
                
            case ItemFlag::Stackable:
                item.flags |= (1 << 5);
                break;
                
            case ItemFlag::ForceUse:
                item.flags |= (1 << 6);
                break;
                
            case ItemFlag::MultiUse:
                item.flags |= (1 << 7);
                break;
                
            case ItemFlag::HasCharges:
                item.flags |= (1 << 8);
                break;
                
            case ItemFlag::Writable:
                item.flags |= (1 << 9);
                stream >> item.maxReadWriteChars;
                break;
                
            case ItemFlag::WritableOnce:
                item.flags |= (1 << 10);
                stream >> item.maxReadChars;
                break;
                
            case ItemFlag::FluidContainer:
                item.flags |= (1 << 11);
                break;
                
            case ItemFlag::Fluid:
                item.flags |= (1 << 12);
                break;
                
            case ItemFlag::IsUnpassable:
                item.flags |= (1 << 13);
                break;
                
            case ItemFlag::IsUnmoveable:
                item.flags |= (1 << 14);
                break;
                
            case ItemFlag::BlockMissiles: // Same as BlockPathfinder (0x0F)
                item.flags |= (1 << 15);
                break;
                
            case ItemFlag::Pickupable:
                item.flags |= (1 << 17);
                break;
                
            case ItemFlag::Hangable:
                item.flags |= (1 << 18);
                break;
                
            case ItemFlag::IsHorizontal:
                item.flags |= (1 << 19);
                break;
                
            case ItemFlag::IsVertical:
                item.flags |= (1 << 20);
                break;
                
            case ItemFlag::Rotatable:
                item.flags |= (1 << 21);
                break;
                
            case ItemFlag::HasLight:
                item.flags |= (1 << 22);
                stream >> item.lightLevel >> item.lightColor;
                break;
                
            case ItemFlag::DontHide:
                item.flags |= (1 << 23);
                break;
                
            case ItemFlag::Translucent:
                item.flags |= (1 << 24);
                break;
                
            case ItemFlag::HasOffset:
                item.flags |= (1 << 25);
                {
                    quint16 offsetX, offsetY;
                    stream >> offsetX >> offsetY;
                }
                break;
                
            case ItemFlag::HasElevation:
                item.flags |= (1 << 26);
                {
                    quint16 height;
                    stream >> height;
                }
                break;
                
            case ItemFlag::Lying:
                item.flags |= (1 << 27);
                break;
                
            case ItemFlag::AnimateAlways:
                item.flags |= (1 << 28);
                break;
                
            case ItemFlag::Minimap:
                item.flags |= (1 << 29);
                stream >> item.minimapColor;
                break;
                
            case ItemFlag::LensHelp:
                item.flags |= (1 << 30);
                {
                    quint16 opt;
                    stream >> opt;
                    if (opt == 1112) {
                        item.flags |= (1 << 9); // Set readable flag
                    }
                }
                break;
                
            case ItemFlag::FullGround:
                item.flags |= (1U << 31);
                break;
                
            case ItemFlag::IgnoreLook:
                // This would require a 64-bit flag field, but legacy system handles it
                break;
                
            case ItemFlag::Cloth:
                // Version 10+ specific flag
                {
                    quint16 clothSlot;
                    stream >> clothSlot;
                }
                break;
                
            case ItemFlag::Market:
                // Version 10+ specific flag
                {
                    quint16 category, tradeAs, showAs;
                    QString name, restrictVocation;
                    quint16 restrictLevel;
                    stream >> category >> tradeAs >> showAs;
                    // Read name string (null-terminated)
                    QByteArray nameBytes;
                    char c;
                    while (stream.readRawData(&c, 1) == 1 && c != 0) {
                        nameBytes.append(c);
                    }
                    name = QString::fromUtf8(nameBytes);
                    stream >> restrictVocation >> restrictLevel;
                }
                break;
                
            case ItemFlag::LastFlag:
                // End of flags
                break;
                
            default:
                unknownFlagCount++;
                if (unknownFlagCount > maxUnknownFlags) {
                    qCritical() << "DatParserV10: Too many unknown flags at item" << itemId 
                               << "- file may be corrupted or from unsupported version";
                    return false;
                }
                qWarning() << "DatParserV10: Unknown flag" << QString::number(flagByte, 16) 
                          << "at item" << itemId << "(" << unknownFlagCount << "/" << maxUnknownFlags << ")";
                // For unknown flags, we can't know if they have additional data, so we must stop
                return false;
        }
    } while (flag != ItemFlag::LastFlag);
    
    // Read sprite dimensions and properties
    stream >> item.width >> item.height;
    
    // Skip unknown byte if width or height > 1
    if (item.width > 1 || item.height > 1) {
        quint8 skipByte;
        stream >> skipByte;
    }
    
    stream >> item.layers >> item.patternX >> item.patternY >> item.patternZ >> item.frames;
    
    // Calculate total number of sprites
    item.numSprites = static_cast<quint32>(item.width) * item.height * item.layers * 
                     item.patternX * item.patternY * item.patternZ * item.frames;
    
    // Skip frame durations if this is an animation (frames > 1)
    if (item.frames > 1) {
        // Skip frame duration data (6 + 8 * frames bytes)
        stream.skipRawData(6 + 8 * item.frames);
    }
    
    // Read sprite IDs - For versions 10.00+, sprite IDs are 32-bit
    item.spriteIds.reserve(item.numSprites);
    for (quint32 i = 0; i < item.numSprites; ++i) {
        quint32 spriteId; // For versions 10.00+, sprite IDs are 32-bit
        stream >> spriteId;
        item.spriteIds.append(spriteId);
    }
    
    // Cache the parsed item
    m_datCache.insert(itemId, item);
    
    return true;
}

DatData DatParserV10::getDatData(quint16 id) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isLoaded || !m_datCache.contains(id)) {
        return DatData{};
    }
    
    return m_datCache.value(id);
}

bool DatParserV10::isLoaded() const
{
    QMutexLocker locker(&m_mutex);
    return m_isLoaded;
}

void DatParserV10::cleanup()
{
    m_datCache.clear();
    m_datSignature = 0;
    m_itemCount = 0;
    m_isLoaded = false;
}

bool DatParserV10::validateSignature(quint32 signature) const
{
    // Known DAT signatures for client versions 10.00-10.77 (Plugin Three range)
    // These signatures are from PluginThree.xml
    QList<quint32> validSignatures = {
        0x51E3F8C3, // 10.10
        0x5236F129, // 10.20
        0x526A5068, // 10.21
        0x52A59036, // 10.30
        0x52AED581, // 10.31
        0x5383504E, // 10.41
        0x38DE,     // 10.77 (16-bit signature)
        0x42A3      // 10.98 (16-bit signature)
    };
    
    bool isValid = validSignatures.contains(signature);
    if (!isValid) {
        qDebug() << "DatParserV10: Unknown signature" << QString::number(signature, 16).toUpper() 
                 << "- this signature may belong to a different plugin";
    }
    
    return isValid;
}

QString DatParserV10::getClientVersion() const
{
    return determineClientVersion(m_datSignature);
}

QString DatParserV10::determineClientVersion(quint32 signature) const
{
    // Map signatures to version strings based on PluginThree.xml
    QHash<quint32, QString> signatureToVersion = {
        {0x51E3F8C3, "10.10"},
        {0x5236F129, "10.20"},
        {0x526A5068, "10.21"},
        {0x52A59036, "10.30"},
        {0x52AED581, "10.31"},
        {0x5383504E, "10.41"},
        {0x38DE, "10.77"},
        {0x42A3, "10.98"}
    };
    
    return signatureToVersion.value(signature, "Unknown");
}