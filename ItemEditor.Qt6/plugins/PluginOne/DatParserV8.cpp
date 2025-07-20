#include "DatParserV8.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QMutexLocker>

DatParserV8::DatParserV8()
    : m_isLoaded(false)
    , m_datSignature(0)
    , m_itemCount(0)
{
}

DatParserV8::~DatParserV8()
{
    cleanup();
}

bool DatParserV8::parseFile(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);
    
    // Clean up any existing data
    cleanup();
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "DatParserV8: Failed to open DAT file:" << filePath;
        return false;
    }
    
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    try {
        // Read and validate DAT signature
        stream >> m_datSignature;
        qDebug() << "DatParserV8: Read DAT signature:" << QString::number(m_datSignature, 16).toUpper();
        
        // Validate signature against known Plugin One signatures
        if (!validateSignature(m_datSignature)) {
            qWarning() << "DatParserV8: Invalid DAT signature for Plugin One:" << QString::number(m_datSignature, 16).toUpper();
            // For now, continue anyway to allow testing with different versions
        }
        
        // Read item count and skip other counts (matching legacy LoadDat exactly)
        stream >> m_itemCount;
        
        quint16 outfitCount, effectCount, missileCount;
        stream >> outfitCount >> effectCount >> missileCount;
        
        qDebug() << "DatParserV8: DAT file has" << m_itemCount << "items";
        qDebug() << "DatParserV8: Outfits:" << outfitCount << "Effects:" << effectCount << "Missiles:" << missileCount;
        
        // Parse all items from the DAT file (starting from ID 100, matching legacy exactly)
        quint16 currentId = 100;
        int parsedItems = 0;
        
        while (currentId <= m_itemCount && !stream.atEnd()) {
            if (parseItemData(stream, currentId)) {
                parsedItems++;
                if (parsedItems % 100 == 0) {
                    qDebug() << "DatParserV8: Parsed" << parsedItems << "items...";
                }
            } else {
                qCritical() << "DatParserV8: Failed to parse item" << currentId << "- DAT file may be corrupted";
                cleanup();
                return false;
            }
            currentId++;
        }
        
        qDebug() << "DatParserV8: Successfully parsed" << parsedItems << "items from DAT file";
        
        m_isLoaded = true;
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "DatParserV8: Exception during parsing:" << e.what();
        cleanup();
        return false;
    }
}

bool DatParserV8::parseItemData(QDataStream& stream, quint16 itemId)
{
    // FIX 7: Add comprehensive bounds checking and error handling
    DatData item;
    item.id = itemId;
    
    qDebug() << "DatParserV8::parseItemData: Parsing item" << itemId;
    
    // FIX 8: Add stream validation before each read
    ItemFlag flag;
    quint8 flagByte;
    int flagCount = 0;
    const int maxFlags = 50; // Prevent infinite loops
    
    do {
        // FIX 9: Check stream status before every read
        if (stream.atEnd() || stream.status() != QDataStream::Ok) {
            qCritical() << "DatParserV8: Stream error at item" << itemId << "flag" << flagCount;
            return false;
        }
        
        if (flagCount++ > maxFlags) {
            qCritical() << "DatParserV8: Too many flags at item" << itemId;
            return false;
        }
        
        stream >> flagByte;
        flag = static_cast<ItemFlag>(flagByte);
        
        qDebug() << "DatParserV8: Item" << itemId << "flag" << flagCount << ":" << QString::number(flagByte, 16);
        
        // FIX 10: Validate stream after each read operation
        auto validateStream = [&]() -> bool {
            if (stream.status() != QDataStream::Ok) {
                qCritical() << "DatParserV8: Stream error after reading flag data at item" << itemId;
                return false;
            }
            return true;
        };
        
        // Process flags with stream validation
        switch (flag) {
            case ItemFlag::Ground:
                stream >> item.groundSpeed;
                if (!validateStream()) return false;
                break;
                
            case ItemFlag::GroundBorder:
            case ItemFlag::OnBottom:
            case ItemFlag::OnTop:
            case ItemFlag::Container:
            case ItemFlag::Stackable:
            case ItemFlag::ForceUse:
            case ItemFlag::MultiUse:
            case ItemFlag::HasCharges:
            case ItemFlag::FluidContainer:
            case ItemFlag::Fluid:
            case ItemFlag::IsUnpassable:
            case ItemFlag::IsUnmoveable:
            case ItemFlag::BlockMissiles:
            case ItemFlag::BlockPathfinder:
            case ItemFlag::Pickupable:
            case ItemFlag::Hangable:
            case ItemFlag::IsHorizontal:
            case ItemFlag::IsVertical:
            case ItemFlag::Rotatable:
            case ItemFlag::DontHide:
            case ItemFlag::FloorChange:
            case ItemFlag::Lying:
            case ItemFlag::AnimateAlways:
            case ItemFlag::FullGround:
            case ItemFlag::IgnoreLook:
                // No additional data
                break;
                
            case ItemFlag::Writable:
                stream >> item.maxReadWriteChars;
                if (!validateStream()) return false;
                break;
                
            case ItemFlag::WritableOnce:
                stream >> item.maxReadChars;
                if (!validateStream()) return false;
                break;
                
            case ItemFlag::HasLight:
                stream >> item.lightLevel >> item.lightColor;
                if (!validateStream()) return false;
                break;
                
            case ItemFlag::HasOffset:
                {
                    quint16 offsetX, offsetY;
                    stream >> offsetX >> offsetY;
                    if (!validateStream()) return false;
                }
                break;
                
            case ItemFlag::HasElevation:
                {
                    quint16 height;
                    stream >> height;
                    if (!validateStream()) return false;
                }
                break;
                
            case ItemFlag::Minimap:
                stream >> item.minimapColor;
                if (!validateStream()) return false;
                break;
                
            case ItemFlag::LensHelp:
                {
                    quint16 opt;
                    stream >> opt;
                    if (!validateStream()) return false;
                }
                break;
                
            case ItemFlag::LastFlag:
                // End of flags
                break;
                
            default:
                qCritical() << "DatParserV8: Unknown flag" << QString::number(flagByte, 16) 
                           << "at item" << itemId;
                return false;
        }
    } while (flag != ItemFlag::LastFlag);
    
    qDebug() << "DatParserV8: Item" << itemId << "processed" << flagCount << "flags";
    
    // Read sprite dimensions with validation
    stream >> item.width >> item.height;
    if (stream.status() != QDataStream::Ok) {
        qCritical() << "DatParserV8: Failed to read dimensions for item" << itemId;
        return false;
    }
    
    // Skip unknown byte if width or height > 1
    if (item.width > 1 || item.height > 1) {
        quint8 skipByte;
        stream >> skipByte;
        if (stream.status() != QDataStream::Ok) {
            qCritical() << "DatParserV8: Failed to skip byte for item" << itemId;
            return false;
        }
    }
    
    stream >> item.layers >> item.patternX >> item.patternY >> item.patternZ >> item.frames;
    if (stream.status() != QDataStream::Ok) {
        qCritical() << "DatParserV8: Failed to read patterns/frames for item" << itemId;
        return false;
    }
    
    // Calculate total number of sprites with bounds checking
    item.numSprites = static_cast<quint32>(item.width) * item.height * item.layers * 
                     item.patternX * item.patternY * item.patternZ * item.frames;
    
    if (item.numSprites > 10000) { // Sanity check
        qCritical() << "DatParserV8: Unreasonable sprite count" << item.numSprites << "for item" << itemId;
        return false;
    }
    
    // Skip frame durations if this is an animation
    if (item.frames > 1) {
        int skipBytes = 6 + 8 * item.frames;
        for (int i = 0; i < skipBytes; ++i) {
            quint8 dummy;
            stream >> dummy;
            if (stream.status() != QDataStream::Ok) {
                qCritical() << "DatParserV8: Failed to skip frame data for item" << itemId;
                return false;
            }
        }
    }
    
    // Read sprite IDs with validation
    item.spriteIds.reserve(item.numSprites);
    for (quint32 i = 0; i < item.numSprites; ++i) {
        quint16 spriteId;
        stream >> spriteId;
        if (stream.status() != QDataStream::Ok) {
            qCritical() << "DatParserV8: Failed to read sprite ID" << i << "for item" << itemId;
            return false;
        }
        item.spriteIds.append(spriteId);
    }
    
    qDebug() << "DatParserV8: Successfully parsed item" << itemId 
             << "size:" << item.width << "x" << item.height 
             << "sprites:" << item.spriteIds.size();
    
    // Cache the parsed item
    m_datCache.insert(itemId, item);
    
    return true;
}

DatData DatParserV8::getDatData(quint16 id) const
{
    // FIX 6: Remove mutex from getter to avoid deadlock
    if (!m_isLoaded) {
        qDebug() << "DatParserV8::getDatData: Parser not loaded";
        return DatData{};
    }
    
    if (!m_datCache.contains(id)) {
        qDebug() << "DatParserV8::getDatData: No data for item" << id;
        return DatData{};
    }
    
    DatData data = m_datCache.value(id);
    qDebug() << "DatParserV8::getDatData: Found data for item" << id 
             << "sprites:" << data.spriteIds.size();
    return data;
}

bool DatParserV8::isLoaded() const
{
    QMutexLocker locker(&m_mutex);
    return m_isLoaded;
}

void DatParserV8::cleanup()
{
    m_datCache.clear();
    m_datSignature = 0;
    m_itemCount = 0;
    m_isLoaded = false;
}

bool DatParserV8::validateSignature(quint32 signature) const
{
    // Known DAT signatures for client versions 8.00-8.55 (Plugin One range)
    // These signatures are from the legacy system's PluginOne.xml
    QList<quint32> validSignatures = {
        0x467FD7E6, // 8.00
        0x475D3747, // 8.10
        0x47F60E37, // 8.11
        0x486905AA, // 8.20
        0x48DA1FB6, // 8.30
        0x493D607A, // 8.40
        0x49B7CC19, // 8.41
        0x49C233C9, // 8.42
        0x4A49C5EB, // 8.50 v1
        0x4A4CC0DC, // 8.50 v2
        0x4AE97492, // 8.50 v3
        0x4B1E2CAA, // 8.54 v1
        0x4B0D46A9, // 8.54 v2
        0x4B28B89E, // 8.54 v3
        0x4B98FF53  // 8.55
    };
    
    bool isValid = validSignatures.contains(signature);
    if (!isValid) {
        qDebug() << "DatParserV8: Unknown signature" << QString::number(signature, 16).toUpper() 
                 << "- this signature may belong to a different plugin (Plugin Two handles 8.60+)";
    }
    
    return isValid;
}

QString DatParserV8::getClientVersion() const
{
    return determineClientVersion(m_datSignature);
}

QString DatParserV8::determineClientVersion(quint32 signature) const
{
    // Map signatures to version strings based on PluginOne.xml
    QHash<quint32, QString> signatureToVersion = {
        {0x467FD7E6, "8.00"},
        {0x475D3747, "8.10"},
        {0x47F60E37, "8.11"},
        {0x486905AA, "8.20"},
        {0x48DA1FB6, "8.30"},
        {0x493D607A, "8.40"},
        {0x49B7CC19, "8.41"},
        {0x49C233C9, "8.42"},
        {0x4A49C5EB, "8.50"}, // v1
        {0x4A4CC0DC, "8.50"}, // v2
        {0x4AE97492, "8.50"}, // v3
        {0x4B1E2CAA, "8.54"}, // v1
        {0x4B0D46A9, "8.54"}, // v2
        {0x4B28B89E, "8.54"}, // v3
        {0x4B98FF53, "8.55"}
    };
    
    return signatureToVersion.value(signature, "Unknown");
}