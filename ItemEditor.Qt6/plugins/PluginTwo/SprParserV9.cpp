#include "SprParserV9.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QMutexLocker>

SprParserV9::SprParserV9()
    : m_isLoaded(false)
    , m_sprSignature(0)
    , m_totalSprites(0)
    , m_transparency(false)
{
}

SprParserV9::~SprParserV9()
{
    cleanup();
}

bool SprParserV9::parseFile(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);
    
    // Clean up any existing data
    cleanup();
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "SprParserV9: Failed to open SPR file:" << filePath;
        return false;
    }
    
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    try {
        // Read and validate SPR signature
        stream >> m_sprSignature;
        if (!validateSignature(m_sprSignature)) {
            qWarning() << "SprParserV9: Invalid SPR signature:" 
                      << QString::number(m_sprSignature, 16).toUpper();
            return false;
        }
        
        // For versions 8.60-9.86, sprite count handling
        // Version 8.60 uses 16-bit sprite count, later versions use 32-bit
        QString clientVersion = getClientVersion();
        if (clientVersion.startsWith("8.6")) {
            // For 8.60-8.62, sprite count is 16-bit
            quint16 spriteCount16;
            stream >> spriteCount16;
            m_totalSprites = spriteCount16;
        } else {
            // For 8.70+, sprite count is 32-bit
            stream >> m_totalSprites;
        }
        
        qDebug() << "SprParserV9: Parsing" << m_totalSprites << "sprites from" << filePath;
        qDebug() << "SprParserV9: SPR signature:" << QString::number(m_sprSignature, 16).toUpper();
        
        // Read sprite indexes
        m_spriteIndexes.clear();
        m_spriteIndexes.reserve(m_totalSprites);
        
        for (quint32 i = 0; i < m_totalSprites; ++i) {
            quint32 index;
            stream >> index;
            m_spriteIndexes.append(index);
        }
        
        // Load sprite data
        if (!loadSpriteData(filePath)) {
            qWarning() << "SprParserV9: Failed to load sprite data";
            cleanup();
            return false;
        }
        
        m_isLoaded = true;
        qDebug() << "SprParserV9: Successfully parsed" << m_spriteCache.size() << "sprites";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "SprParserV9: Exception during parsing:" << e.what();
        cleanup();
        return false;
    }
}

bool SprParserV9::loadSpriteData(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    quint32 spriteId = 1;
    for (quint32 index : m_spriteIndexes) {
        // Seek to sprite data position (index + 3 bytes offset)
        quint32 position = index + 3;
        if (!file.seek(position)) {
            qWarning() << "SprParserV9: Failed to seek to sprite" << spriteId << "at position" << position;
            continue;
        }
        
        // Read sprite size
        quint16 size;
        stream >> size;
        
        if (size > 0) {
            ::SpriteData sprite;
            sprite.id = spriteId;
            sprite.size = size;
            sprite.transparent = m_transparency;
            
            // Read compressed pixel data
            sprite.compressedPixels = file.read(size);
            if (sprite.compressedPixels.size() != size) {
                qWarning() << "SprParserV9: Failed to read sprite data for sprite" << spriteId;
                continue;
            }
            
            // Cache the sprite
            m_spriteCache.insert(spriteId, sprite);
        }
        
        ++spriteId;
    }
    
    return true;
}

::SpriteData SprParserV9::getSpriteData(quint16 id) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isLoaded || !m_spriteCache.contains(id)) {
        return ::SpriteData{};
    }
    
    return m_spriteCache.value(id);
}

bool SprParserV9::isLoaded() const
{
    QMutexLocker locker(&m_mutex);
    return m_isLoaded;
}

void SprParserV9::cleanup()
{
    m_spriteCache.clear();
    m_spriteIndexes.clear();
    m_sprSignature = 0;
    m_totalSprites = 0;
    m_transparency = false;
    m_isLoaded = false;
}

bool SprParserV9::validateSignature(quint32 signature) const
{
    // Known SPR signatures for client versions 8.60-9.86 (Plugin Two range)
    // These signatures are from PluginTwo.xml
    QList<quint32> validSignatures = {
        0x4C220594, // 8.60 v1 & v2
        0x4C63F145, // 8.61 & 8.62
        0x4CFD078A, // 8.70
        0x4D3D65D0, // 8.71
        0x4DAD1A32, // 8.72, 8.73, 9.00
        0x4E12DB27, // 9.10
        0x4E807C23, // 9.20
        0x4EE71E06, // 9.40
        0x4F0EEFEF, // 9.44 (old)
        0x4F1051D7, // 9.44 v1 & v2
        0x4F3131F6, // 9.44 v3
        0x4F5DCEF7, // 9.46
        0x4F75B7CD, // 9.50
        0x4F857F8E, // 9.52
        0x4FA11282, // 9.53
        0x4FD595B7, // 9.54
        0x4FFA74F9, // 9.60
        0x50226FBD, // 9.61
        0x503CB954, // 9.63
        0x5072A567, // 9.70
        0x50C70753, // 9.80
        0x50D1C685, // 9.81
        0x512CAD68, // 9.82
        0x51407BC7, // 9.83
        0x51641A84, // 9.85
        0x5170E96F  // 9.86
    };
    
    bool isValid = validSignatures.contains(signature);
    if (!isValid) {
        qDebug() << "SprParserV9: Unknown signature" << QString::number(signature, 16).toUpper() 
                 << "- this signature may belong to a different plugin";
    }
    
    return isValid;
}

QString SprParserV9::getClientVersion() const
{
    return determineClientVersion(m_sprSignature);
}

QString SprParserV9::determineClientVersion(quint32 signature) const
{
    // Map signatures to version strings based on PluginTwo.xml
    QHash<quint32, QString> signatureToVersion = {
        {0x4C220594, "8.60"}, // v1 & v2
        {0x4C63F145, "8.61"}, // Also used for 8.62
        {0x4CFD078A, "8.70"},
        {0x4D3D65D0, "8.71"},
        {0x4DAD1A32, "8.72"}, // Also used for 8.73 and 9.00
        {0x4E12DB27, "9.10"},
        {0x4E807C23, "9.20"},
        {0x4EE71E06, "9.40"},
        {0x4F0EEFEF, "9.44"}, // old
        {0x4F1051D7, "9.44"}, // v1 & v2
        {0x4F3131F6, "9.44"}, // v3
        {0x4F5DCEF7, "9.46"},
        {0x4F75B7CD, "9.50"},
        {0x4F857F8E, "9.52"},
        {0x4FA11282, "9.53"},
        {0x4FD595B7, "9.54"},
        {0x4FFA74F9, "9.60"},
        {0x50226FBD, "9.61"},
        {0x503CB954, "9.63"},
        {0x5072A567, "9.70"},
        {0x50C70753, "9.80"},
        {0x50D1C685, "9.81"},
        {0x512CAD68, "9.82"},
        {0x51407BC7, "9.83"},
        {0x51641A84, "9.85"},
        {0x5170E96F, "9.86"}
    };
    
    return signatureToVersion.value(signature, "Unknown");
}

quint32 SprParserV9::getSprSignature() const
{
    return m_sprSignature;
}