#include "SprParserV8.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QMutexLocker>

SprParserV8::SprParserV8()
    : m_isLoaded(false)
    , m_sprSignature(0)
    , m_totalSprites(0)
    , m_transparency(false)
{
}

SprParserV8::~SprParserV8()
{
    cleanup();
}

bool SprParserV8::parseFile(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);
    
    // Clean up any existing data
    cleanup();
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "SprParserV8: Failed to open SPR file:" << filePath;
        return false;
    }
    
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    try {
        // Read and validate SPR signature (matching legacy LoadSprites exactly)
        stream >> m_sprSignature;
        qDebug() << "SprParserV8: Read SPR signature:" << QString::number(m_sprSignature, 16).toUpper();
        
        // Validate signature against known Plugin One signatures
        if (!validateSignature(m_sprSignature)) {
            qWarning() << "SprParserV8: Invalid SPR signature for Plugin One:" << QString::number(m_sprSignature, 16).toUpper();
            // For now, continue anyway to allow testing with different versions
        }
        
        // For versions 8.00-8.57, sprite count is 16-bit (extended = false)
        quint16 spriteCount16;
        stream >> spriteCount16;
        m_totalSprites = spriteCount16;
        
        qDebug() << "SprParserV8: SPR file has" << m_totalSprites << "sprites";
        
        // Read sprite indexes (matching legacy LoadSprites exactly)
        m_spriteIndexes.clear();
        m_spriteIndexes.reserve(m_totalSprites);
        
        for (quint32 i = 0; i < m_totalSprites; ++i) {
            quint32 index;
            stream >> index;
            m_spriteIndexes.append(index);
        }
        
        qDebug() << "SprParserV8: Read" << m_spriteIndexes.size() << "sprite indexes";
        
        // Load sprite data using the indexes (matching legacy LoadSprites exactly)
        if (!loadSpriteData(filePath)) {
            qCritical() << "SprParserV8: Failed to load sprite data";
            cleanup();
            return false;
        }
        
        m_isLoaded = true;
        qDebug() << "SprParserV8: Successfully loaded" << m_spriteCache.size() << "sprites";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "SprParserV8: Exception during parsing:" << e.what();
        cleanup();
        return false;
    }
}

bool SprParserV8::loadSpriteData(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "SprParserV8: Failed to open SPR file for sprite data loading";
        return false;
    }
    
    // Load sprite data exactly as in legacy Sprite.LoadSprites method
    quint32 spriteId = 1;
    int loadedSprites = 0;
    
    for (quint32 element : m_spriteIndexes) {
        // Calculate position: index + 3 bytes offset (matching legacy exactly)
        quint32 position = element + 3;
        
        if (!file.seek(position)) {
            qWarning() << "SprParserV8: Failed to seek to sprite" << spriteId << "at position" << position;
            ++spriteId;
            continue;
        }
        
        // Read sprite size (matching legacy exactly)
        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::LittleEndian);
        
        quint16 size;
        stream >> size;
        
        if (size > 0) {
            ::SpriteData sprite;
            sprite.id = spriteId;
            sprite.size = size;
            sprite.transparent = m_transparency; // Set based on client version
            
            // Read compressed pixel data (matching legacy exactly)
            sprite.compressedPixels = file.read(size);
            if (sprite.compressedPixels.size() != static_cast<int>(size)) {
                qWarning() << "SprParserV8: Failed to read complete sprite data for sprite" << spriteId 
                          << "- expected" << size << "bytes, got" << sprite.compressedPixels.size();
                ++spriteId;
                continue;
            }
            
            // Cache the sprite (matching legacy exactly)
            m_spriteCache.insert(spriteId, sprite);
            loadedSprites++;
            
            if (loadedSprites % 1000 == 0) {
                qDebug() << "SprParserV8: Loaded" << loadedSprites << "sprites...";
            }
        }
        
        ++spriteId;
    }
    
    qDebug() << "SprParserV8: Successfully loaded" << loadedSprites << "sprites with data";
    return true;
}

::SpriteData SprParserV8::getSpriteData(quint16 id) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isLoaded || !m_spriteCache.contains(id)) {
        return ::SpriteData{};
    }
    
    return m_spriteCache.value(id);
}

bool SprParserV8::isLoaded() const
{
    QMutexLocker locker(&m_mutex);
    return m_isLoaded;
}

void SprParserV8::cleanup()
{
    m_spriteCache.clear();
    m_spriteIndexes.clear();
    m_sprSignature = 0;
    m_totalSprites = 0;
    m_transparency = false;
    m_isLoaded = false;
}

bool SprParserV8::validateSignature(quint32 signature) const
{
    // Known SPR signatures for client versions 8.00-8.55 (Plugin One range)
    // These signatures are from the legacy system's PluginOne.xml
    QList<quint32> validSignatures = {
        0x467F9E74, // 8.00
        0x475D0B01, // 8.10
        0x47EBB9B2, // 8.11
        0x4868ECC9, // 8.20
        0x48C8E712, // 8.30
        0x493D4E7C, // 8.40
        0x49B140EA, // 8.41
        0x49B140EA, // 8.42 (same as 8.41)
        0x4A44FD4E, // 8.50 v1 & v2
        0x4ACB5230, // 8.50 v3
        0x4B1E2C87, // 8.54 v1 & v3
        0x4B0D3AFF, // 8.54 v2
        0x4B913871  // 8.55
    };
    
    bool isValid = validSignatures.contains(signature);
    if (!isValid) {
        qDebug() << "SprParserV8: Unknown signature" << QString::number(signature, 16).toUpper() 
                 << "- this signature may belong to a different plugin (Plugin Two handles 8.60+)";
    }
    
    return isValid;
}

QString SprParserV8::getClientVersion() const
{
    return determineClientVersion(m_sprSignature);
}

QString SprParserV8::determineClientVersion(quint32 signature) const
{
    // Map signatures to version strings based on PluginOne.xml
    QHash<quint32, QString> signatureToVersion = {
        {0x467F9E74, "8.00"},
        {0x475D0B01, "8.10"},
        {0x47EBB9B2, "8.11"},
        {0x4868ECC9, "8.20"},
        {0x48C8E712, "8.30"},
        {0x493D4E7C, "8.40"},
        {0x49B140EA, "8.41"}, // Also used for 8.42
        {0x4A44FD4E, "8.50"}, // v1 & v2
        {0x4ACB5230, "8.50"}, // v3
        {0x4B1E2C87, "8.54"}, // v1 & v3
        {0x4B0D3AFF, "8.54"}, // v2
        {0x4B913871, "8.55"}
    };
    
    return signatureToVersion.value(signature, "Unknown");
}