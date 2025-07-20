#include "SprParserV10.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QMutexLocker>

SprParserV10::SprParserV10()
    : m_isLoaded(false)
    , m_sprSignature(0)
    , m_totalSprites(0)
    , m_transparency(false)
{
}

SprParserV10::~SprParserV10()
{
    cleanup();
}

bool SprParserV10::parseFile(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);
    
    // Clean up any existing data
    cleanup();
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "SprParserV10: Failed to open SPR file:" << filePath;
        return false;
    }
    
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    try {
        // Read and validate SPR signature
        stream >> m_sprSignature;
        if (!validateSignature(m_sprSignature)) {
            qWarning() << "SprParserV10: Invalid SPR signature:" 
                      << QString::number(m_sprSignature, 16).toUpper();
            return false;
        }
        
        // For versions 10.00+, sprite count is 32-bit
        stream >> m_totalSprites;
        
        qDebug() << "SprParserV10: Parsing" << m_totalSprites << "sprites from" << filePath;
        qDebug() << "SprParserV10: SPR signature:" << QString::number(m_sprSignature, 16).toUpper();
        
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
            qWarning() << "SprParserV10: Failed to load sprite data";
            cleanup();
            return false;
        }
        
        m_isLoaded = true;
        qDebug() << "SprParserV10: Successfully parsed" << m_spriteCache.size() << "sprites";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "SprParserV10: Exception during parsing:" << e.what();
        cleanup();
        return false;
    }
}

bool SprParserV10::loadSpriteData(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    quint32 spriteId = 1;
    for (quint32 index : m_spriteIndexes) {
        // Seek to sprite data position (index + 3 bytes offset for version 10+)
        quint32 position = index + 3;
        if (!file.seek(position)) {
            qWarning() << "SprParserV10: Failed to seek to sprite" << spriteId << "at position" << position;
            continue;
        }
        
        // Read sprite size (32-bit for version 10+)
        quint32 size;
        stream >> size;
        
        if (size > 0) {
            ::SpriteData sprite;
            sprite.id = spriteId;
            sprite.size = size;
            sprite.transparent = m_transparency;
            
            // Read compressed pixel data
            sprite.compressedPixels = file.read(size);
            if (sprite.compressedPixels.size() != static_cast<int>(size)) {
                qWarning() << "SprParserV10: Failed to read sprite data for sprite" << spriteId;
                continue;
            }
            
            // Cache the sprite
            m_spriteCache.insert(spriteId, sprite);
        }
        
        ++spriteId;
    }
    
    return true;
}

::SpriteData SprParserV10::getSpriteData(quint16 id) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isLoaded || !m_spriteCache.contains(id)) {
        return ::SpriteData{};
    }
    
    return m_spriteCache.value(id);
}

bool SprParserV10::isLoaded() const
{
    QMutexLocker locker(&m_mutex);
    return m_isLoaded;
}

void SprParserV10::cleanup()
{
    m_spriteCache.clear();
    m_spriteIndexes.clear();
    m_sprSignature = 0;
    m_totalSprites = 0;
    m_transparency = false;
    m_isLoaded = false;
}

bool SprParserV10::validateSignature(quint32 signature) const
{
    // Known SPR signatures for client versions 10.00-10.77 (Plugin Three range)
    // These signatures are from PluginThree.xml
    QList<quint32> validSignatures = {
        0x51E3F8E9, // 10.10
        0x5236F14F, // 10.20
        0x526A5090, // 10.21
        0x52A5905F, // 10.30
        0x52AED5A7, // 10.31
        0x53835077, // 10.41
        0x5525213D, // 10.77
        0x57BBD603  // 10.98
    };
    
    bool isValid = validSignatures.contains(signature);
    if (!isValid) {
        qDebug() << "SprParserV10: Unknown signature" << QString::number(signature, 16).toUpper() 
                 << "- this signature may belong to a different plugin";
    }
    
    return isValid;
}

QString SprParserV10::getClientVersion() const
{
    return determineClientVersion(m_sprSignature);
}

QString SprParserV10::determineClientVersion(quint32 signature) const
{
    // Map signatures to version strings based on PluginThree.xml
    QHash<quint32, QString> signatureToVersion = {
        {0x51E3F8E9, "10.10"},
        {0x5236F14F, "10.20"},
        {0x526A5090, "10.21"},
        {0x52A5905F, "10.30"},
        {0x52AED5A7, "10.31"},
        {0x53835077, "10.41"},
        {0x5525213D, "10.77"},
        {0x57BBD603, "10.98"}
    };
    
    return signatureToVersion.value(signature, "Unknown");
}