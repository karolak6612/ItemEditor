/**
 * Item Editor Qt6 - Sprite Reader Implementation
 * Handles .spr file format loading and sprite management
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "SpriteReader.h"
#include "../../Sprite.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QMutexLocker>

namespace ItemEditor {

SpriteReader::SpriteReader(QObject *parent)
    : QObject(parent)
    , m_filename()
    , m_fileData()
    , m_spriteOffsets()
    , m_spriteSizes()
    , m_spriteCache(DEFAULT_CACHE_SIZE)
    , m_loaded(false)
    , m_spriteCount(0)
    , m_client()
    , m_extended(false)
    , m_transparency(false)
{
    // Initialize sprite cache
    m_spriteCache.setMaxCost(DEFAULT_CACHE_SIZE);
}

SpriteReader::~SpriteReader()
{
    clearCache();
}bool SpriteReader::loadSprites(const QString& filename, 
                              const SupportedClient& client, 
                              bool extended, 
                              bool transparency)
{
    QMutexLocker locker(&m_mutex);
    
    // Clear previous data
    clearCache();
    m_spriteOffsets.clear();
    m_spriteSizes.clear();
    m_fileData.clear();
    m_loaded = false;
    m_spriteCount = 0;
    
    // Store parameters
    m_filename = filename;
    m_client = client;
    m_extended = extended;
    m_transparency = transparency;
    
    // Load sprite data
    bool success = loadSpriteData(filename, client, extended, transparency);
    if (success) {
        m_loaded = true;
        emit loadingFinished(true);
    } else {
        emit loadingFinished(false);
    }
    
    return success;
}

bool SpriteReader::loadSpriteData(const QString& filename, 
                                 const SupportedClient& client, 
                                 bool extended, 
                                 bool transparency)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open sprite file:" << filename;
        return false;
    }
    
    // Read entire file into memory for performance
    m_fileData = file.readAll();
    file.close();
    
    if (m_fileData.isEmpty()) {
        qWarning() << "Empty sprite file:" << filename;
        return false;
    }
    
    QDataStream stream(m_fileData);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    try {
        // Read and verify signature
        quint32 sprSignature;
        stream >> sprSignature;
        
        if (client.sprSignature() != sprSignature) {
            qWarning() << QString("Bad spr signature. Expected %1, got %2")
                         .arg(client.sprSignature(), 0, 16)
                         .arg(sprSignature, 0, 16);
            return false;
        }
        
        // Read sprite count
        quint32 totalPics;
        if (extended) {
            stream >> totalPics;
        } else {
            quint16 pics;
            stream >> pics;
            totalPics = pics;
        }
        
        m_spriteCount = totalPics;
        emit loadingProgress(0, totalPics);
        
        // Read sprite index table
        QList<quint32> spriteIndexes;
        spriteIndexes.reserve(totalPics);
        
        for (quint32 i = 0; i < totalPics; ++i) {
            quint32 index;
            stream >> index;
            spriteIndexes.append(index);
        }
        
        // Process sprite data offsets and sizes
        quint32 id = 1;
        for (quint32 element : spriteIndexes) {
            quint32 dataOffset = element + 3; // Add 3 bytes offset as per C# implementation
            
            if (dataOffset + 2 >= static_cast<quint32>(m_fileData.size())) {
                qWarning() << QString("Invalid sprite offset for ID %1: %2").arg(id).arg(dataOffset);
                continue;
            }
            
            // Read sprite size from file data
            QDataStream sizeStream(m_fileData.mid(dataOffset, 2));
            sizeStream.setByteOrder(QDataStream::LittleEndian);
            quint16 size;
            sizeStream >> size;
            
            if (size > 0 && dataOffset + 2 + size <= static_cast<quint32>(m_fileData.size())) {
                m_spriteOffsets[id] = dataOffset + 2; // Skip size bytes
                m_spriteSizes[id] = size;
            } else {
                qWarning() << QString("Invalid sprite size for ID %1: %2").arg(id).arg(size);
            }
            
            ++id;
            
            // Emit progress
            if (id % 100 == 0) {
                emit loadingProgress(id - 1, totalPics);
            }
        }
        
        emit loadingProgress(totalPics, totalPics);
        return true;
    }
    catch (...) {
        qWarning() << "Exception occurred while loading sprite file:" << filename;
        return false;
    }
}

Sprite* SpriteReader::getSprite(quint32 id) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_loaded || !m_spriteOffsets.contains(id)) {
        return nullptr;
    }
    
    // Check cache first
    Sprite* cachedSprite = m_spriteCache.object(id);
    if (cachedSprite) {
        return cachedSprite;
    }
    
    // Create sprite from file data
    quint32 offset = m_spriteOffsets[id];
    quint16 size = m_spriteSizes[id];
    
    if (offset + size > static_cast<quint32>(m_fileData.size())) {
        qWarning() << QString("Invalid sprite data range for ID %1").arg(id);
        return nullptr;
    }
    
    // Create new sprite
    Sprite* sprite = new Sprite();
    sprite->setId(id);
    sprite->setSize(size);
    sprite->setTransparent(m_transparency);
    
    // Extract compressed pixel data
    QByteArray compressedPixels = m_fileData.mid(offset, size);
    sprite->setCompressedPixels(compressedPixels);
    
    // Cache the sprite
    const_cast<SpriteReader*>(this)->cacheSprite(id, sprite);
    
    emit const_cast<SpriteReader*>(this)->spriteLoaded(id);
    return sprite;
}

bool SpriteReader::hasSprite(quint32 id) const
{
    QMutexLocker locker(&m_mutex);
    return m_loaded && m_spriteOffsets.contains(id);
}

QList<quint32> SpriteReader::getSpriteIds() const
{
    QMutexLocker locker(&m_mutex);
    return m_spriteOffsets.keys();
}

void SpriteReader::clearCache()
{
    QMutexLocker locker(&m_mutex);
    m_spriteCache.clear();
}

void SpriteReader::setMaxCacheSize(int maxSize)
{
    QMutexLocker locker(&m_mutex);
    m_spriteCache.setMaxCost(maxSize);
}

int SpriteReader::cacheSize() const
{
    QMutexLocker locker(&m_mutex);
    return m_spriteCache.totalCost();
}

void SpriteReader::cacheSprite(quint32 id, Sprite* sprite)
{
    // Cache takes ownership of the sprite
    m_spriteCache.insert(id, sprite, 1);
}

} // namespace ItemEditor