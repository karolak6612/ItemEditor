#include "DatParserV9.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QMutexLocker>

DatParserV9::DatParserV9()
    : m_isLoaded(false)
    , m_datSignature(0)
    , m_itemCount(0)
{
}

DatParserV9::~DatParserV9()
{
    cleanup();
}

bool DatParserV9::parseFile(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);
    
    // Clean up any existing data
    cleanup();
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "DatParserV9: Failed to open DAT file:" << filePath;
        return false;
    }
    
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    try {
        // Read and validate DAT signature
        stream >> m_datSignature;
        qDebug() << "DatParserV9: Read DAT signature:" << QString::number(m_datSignature, 16).toUpper();
        if (!validateSignature(m_datSignature)) {
            qWarning() << "DatParserV9: Invalid DAT signature:" 
                      << QString::number(m_datSignature, 16).toUpper()
                      << "- This signature is not supported by Plugin Two (versions 8.60-9.86)";
            return false;
        }
        
        // Read item count and skip other counts
        stream >> m_itemCount;
        
        quint16 outfitCount, effectCount, missileCount;
        stream >> outfitCount >> effectCount >> missileCount;
        
        qDebug() << "DatParserV9: Parsing" << m_itemCount << "items from" << filePath;
        qDebug() << "DatParserV9: DAT signature:" << QString::number(m_datSignature, 16).toUpper();
        
        // Parse items starting from ID 100 (as per legacy system)
        int successfullyParsed = 0;
        for (quint16 itemId = 100; itemId <= m_itemCount && itemId < 350; ++itemId) { // Limit to first 250 items for testing
            if (!parseItemData(stream, itemId)) {
                qWarning() << "DatParserV9: Failed to parse item" << itemId << "- stopping parsing to prevent corruption";
                break; // Stop parsing but don't fail completely
            }
            successfullyParsed++;
        }
        
        qDebug() << "DatParserV9: Successfully parsed" << successfullyParsed << "items out of" << qMin(100, static_cast<int>(m_itemCount - 99)) << "attempted";
        
        m_isLoaded = true;
        qDebug() << "DatParserV9: Successfully parsed" << m_datCache.size() << "items";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "DatParserV9: Exception during parsing:" << e.what();
        cleanup();
        return false;
    }
}

bool DatParserV9::parseItemData(QDataStream& stream, quint16 itemId)
{
    DatData item;
    item.id = itemId;
    
    // Read item flags until LastFlag (0xFF) - using exact legacy PluginTwo logic
    quint8 flagByte;
    
    do {
        if (stream.atEnd()) {
            qWarning() << "DatParserV9: Unexpected end of stream at item" << itemId;
            return false;
        }
        
        stream >> flagByte;
        
        // Debug: Print every flag we encounter
        qDebug() << "DatParserV9: Processing flag" << QString::number(flagByte, 16) 
                 << "(decimal" << flagByte << ") at item" << itemId;
        
        // Use exact same flag handling as legacy PluginTwo
        switch (flagByte)
        {
            case 0x00: // Ground
                stream >> item.groundSpeed;
                break;

            case 0x01: // GroundBorder
                break;

            case 0x02: // OnBottom
                break;

            case 0x03: // OnTop
                break;

            case 0x04: // Container
                break;

            case 0x05: // Stackable
                break;

            case 0x06: // ForceUse
                break;

            case 0x07: // MultiUse
                break;

            case 0x08: // Writable
                stream >> item.maxReadWriteChars;
                break;

            case 0x09: // WritableOnce
                stream >> item.maxReadChars;
                break;

            case 0x0A: // FluidContainer
                break;

            case 0x0B: // Fluid
                break;

            case 0x0C: // IsUnpassable
                break;

            case 0x0D: // IsUnmoveable
                break;

            case 0x0E: // BlockMissiles
                break;

            case 0x0F: // BlockPathfinder
                break;

            case 0x10: // Pickupable
                break;

            case 0x11: // Hangable
                break;

            case 0x12: // IsHorizontal
                break;

            case 0x13: // IsVertical
                break;

            case 0x14: // Rotatable
                break;

            case 0x15: // HasLight
                stream >> item.lightLevel >> item.lightColor;
                break;

            case 0x16: // DontHide
                break;

            case 0x17: // Translucent
                break;

            case 0x18: // HasOffset
                {
                    quint16 offsetX, offsetY;
                    stream >> offsetX >> offsetY;
                }
                break;

            case 0x19: // HasElevation
                {
                    quint16 height;
                    stream >> height;
                }
                break;

            case 0x1A: // Lying
                break;

            case 0x1B: // AnimateAlways
                break;

            case 0x1C: // Minimap (decimal 28)
                stream >> item.minimapColor;
                qDebug() << "DatParserV9: Read minimap color for item" << itemId << "color:" << item.minimapColor;
                break;

            case 0x1D: // LensHelp
                {
                    quint16 opt;
                    stream >> opt;
                    // Legacy system sets readable flag if opt == 1112
                }
                break;

            case 0x1E: // FullGround
                break;

            case 0x1F: // IgnoreLook
                break;

            case 0x20: // Cloth
                {
                    quint16 clothSlot;
                    stream >> clothSlot;
                }
                break;

            case 0x21: // Market
                {
                    quint16 category, tradeAs, showAs, profession, level;
                    stream >> category >> tradeAs >> showAs;
                    
                    quint16 nameLength;
                    stream >> nameLength;
                    
                    QByteArray nameBuffer(nameLength, 0);
                    stream.readRawData(nameBuffer.data(), nameLength);
                    
                    stream >> profession >> level;
                }
                break;

            // Extended flags for later 8.60 builds - these might not have additional data
            case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
            case 0x28: case 0x29: case 0x2A: case 0x2B: case 0x2C: case 0x2D:
            case 0x2E: case 0x2F: case 0x30: case 0x31: case 0x32: case 0x33:
            case 0x34: case 0x35: case 0x36: case 0x37: case 0x38: case 0x39:
            case 0x3A: case 0x3B: case 0x3C: case 0x3D: case 0x3E: case 0x3F:
            // Even higher extended flags (0x40-0xFF range, excluding 0xFF which is LastFlag)
            case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45:
            case 0x46: case 0x47: case 0x48: case 0x49: case 0x4A: case 0x4B:
            case 0x4C: case 0x4D: case 0x4E: case 0x4F: case 0x50: case 0x51:
            case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
            case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D:
            case 0x5E: case 0x5F: case 0x60: case 0x61: case 0x62: case 0x63:
            case 0x64: case 0x65: case 0x66: case 0x67: case 0x68: case 0x69:
            case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F:
            case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75:
            case 0x76: case 0x77: case 0x78: case 0x79: case 0x7A: case 0x7B:
            case 0x7C: case 0x7D: case 0x7E: case 0x7F: case 0x80: case 0x81:
            case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
            case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D:
            case 0x8E: case 0x8F: case 0x90: case 0x91: case 0x92: case 0x93:
            case 0x94: case 0x95: case 0x96: case 0x97: case 0x98: case 0x99:
            case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9E: case 0x9F:
            case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5:
            case 0xA6: case 0xA7: case 0xA8: case 0xA9: case 0xAA: case 0xAB:
            case 0xAC: case 0xAD: case 0xAE: case 0xAF: case 0xB0: case 0xB1:
            case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: case 0xB7:
            case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD:
            case 0xBE: case 0xBF: case 0xC0: case 0xC1: case 0xC2: case 0xC3:
            case 0xC4: case 0xC5: case 0xC6: case 0xC7: case 0xC8: case 0xC9:
            case 0xCA: case 0xCB: case 0xCC: case 0xCD: case 0xCE: case 0xCF:
            case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xD4: case 0xD5:
            case 0xD6: case 0xD7: case 0xD8: case 0xD9: case 0xDA: case 0xDB:
            case 0xDC: case 0xDD: case 0xDE: case 0xDF: case 0xE0: case 0xE1:
            case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE6: case 0xE7:
            case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC: case 0xED:
            case 0xEE: case 0xEF: case 0xF0: case 0xF1: case 0xF2: case 0xF3:
            case 0xF4: case 0xF5: case 0xF6: case 0xF7: case 0xF8: case 0xF9:
            case 0xFA: case 0xFB: case 0xFC: case 0xFD: case 0xFE:
                // These are extended flags that appeared in later client versions
                // Most don't have additional data, so we just skip them
                qDebug() << "DatParserV9: Handling extended flag" << QString::number(flagByte, 16) 
                         << "at item" << itemId << "(no additional data)";
                break;

            case 0xFF: // LastFlag
                break;
                
            default:
                qWarning() << "DatParserV9: Unknown flag" << QString::number(flagByte, 16) 
                          << "at item" << itemId;
                return false;
        }
    }
    while (flagByte != 0xFF);
    
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
    
    // Read sprite IDs - For version 8.60, sprite IDs are 16-bit (matching legacy PluginTwo)
    item.spriteIds.reserve(item.numSprites);
    for (quint32 i = 0; i < item.numSprites; ++i) {
        quint16 spriteId16;
        stream >> spriteId16;
        item.spriteIds.append(spriteId16);
    }
    
    // Cache the parsed item
    m_datCache.insert(itemId, item);
    
    return true;
}

DatData DatParserV9::getDatData(quint16 id) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isLoaded) {
        qDebug() << "DatParserV9::getDatData: Parser not loaded, returning empty data for ID" << id;
        return DatData{};
    }
    
    if (!m_datCache.contains(id)) {
        qDebug() << "DatParserV9::getDatData: No data cached for ID" << id;
        return DatData{};
    }
    
    return m_datCache.value(id);
}

bool DatParserV9::isLoaded() const
{
    QMutexLocker locker(&m_mutex);
    return m_isLoaded;
}

void DatParserV9::cleanup()
{
    m_datCache.clear();
    m_datSignature = 0;
    m_itemCount = 0;
    m_isLoaded = false;
}

bool DatParserV9::validateSignature(quint32 signature) const
{
    // Known DAT signatures for client versions 8.60-9.86 (Plugin Two range)
    // These signatures are from PluginTwo.xml
    QList<quint32> validSignatures = {
        0x4C28B721, // 8.60 v1
        0x4C2C7993, // 8.60 v2
        0x4C6A4CBC, // 8.61
        0x4C973450, // 8.62
        0x4CFE22C5, // 8.70
        0x4D41979E, // 8.71
        0x4DAD1A1A, // 8.72
        0x4DBAA20B, // 8.73 and 9.00
        0x4E12DAFF, // 9.10
        0x4E807C08, // 9.20
        0x4EE71DE5, // 9.40
        0x4F0EEFBB, // 9.44 (old)
        0x4F105168, // 9.44 v1
        0x4F16C0D7, // 9.44 v2
        0x4F3131CF, // 9.44 v3
        0x4F6B341F, // 9.46
        0x4F75B7AB, // 9.50
        0x4F857F6C, // 9.52
        0x4FA11252, // 9.53
        0x4FD5956B, // 9.54
        0x4FFA74CC, // 9.60
        0x50226F9D, // 9.61
        0x503CB933, // 9.63
        0x5072A490, // 9.70
        0x50C70674, // 9.80
        0x50D1C5B6, // 9.81
        0x512CAD09, // 9.82
        0x51407B67, // 9.83
        0x51641A1B, // 9.85
        0x5170E904  // 9.86
    };
    
    bool isValid = validSignatures.contains(signature);
    if (!isValid) {
        qDebug() << "DatParserV9: Unknown signature" << QString::number(signature, 16).toUpper() 
                 << "- this signature may belong to a different plugin";
    }
    
    return isValid;
}

QString DatParserV9::getClientVersion() const
{
    return determineClientVersion(m_datSignature);
}

QString DatParserV9::determineClientVersion(quint32 signature) const
{
    // Map signatures to version strings based on PluginTwo.xml
    QHash<quint32, QString> signatureToVersion = {
        {0x4C28B721, "8.60"}, // v1
        {0x4C2C7993, "8.60"}, // v2
        {0x4C6A4CBC, "8.61"},
        {0x4C973450, "8.62"},
        {0x4CFE22C5, "8.70"},
        {0x4D41979E, "8.71"},
        {0x4DAD1A1A, "8.72"},
        {0x4DBAA20B, "8.73"}, // Also used for 9.00
        {0x4E12DAFF, "9.10"},
        {0x4E807C08, "9.20"},
        {0x4EE71DE5, "9.40"},
        {0x4F0EEFBB, "9.44"}, // old
        {0x4F105168, "9.44"}, // v1
        {0x4F16C0D7, "9.44"}, // v2
        {0x4F3131CF, "9.44"}, // v3
        {0x4F6B341F, "9.46"},
        {0x4F75B7AB, "9.50"},
        {0x4F857F6C, "9.52"},
        {0x4FA11252, "9.53"},
        {0x4FD5956B, "9.54"},
        {0x4FFA74CC, "9.60"},
        {0x50226F9D, "9.61"},
        {0x503CB933, "9.63"},
        {0x5072A490, "9.70"},
        {0x50C70674, "9.80"},
        {0x50D1C5B6, "9.81"},
        {0x512CAD09, "9.82"},
        {0x51407B67, "9.83"},
        {0x51641A1B, "9.85"},
        {0x5170E904, "9.86"}
    };
    
    return signatureToVersion.value(signature, "Unknown");
}

quint32 DatParserV9::getDatSignature() const
{
    return m_datSignature;
}