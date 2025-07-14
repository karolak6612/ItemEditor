/**
 * Item Editor Qt6 - Sprite Manager Implementation
 * Manages sprite loading and provides access to sprites for items
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "SpriteManager.h"
#include "Sprite.h"
#include "Item.h"
#include <QMutexLocker>
#include <QDebug>

namespace ItemEditor {

SpriteManager::SpriteManager(QObject *parent)
    : QObject(parent)
    , m_spriteReader(new SpriteReader(this))
    , m_currentFile()
    , m_loaded(false)
    , m_currentClient()
{
    // Connect sprite reader signals
    connect(m_spriteReader, &SpriteReader::loadingProgress,
            this, &SpriteManager::onSpriteReaderProgress);
    connect(m_spriteReader, &SpriteReader::loadingFinished,
            this, &SpriteManager::onSpriteReaderFinished);
}

SpriteManager::~SpriteManager()
{
    unloadSprites();
}

bool SpriteManager::loadSpriteFile(const QString& filename, 
                                  const SupportedClient& client, 
                                  bool extended, 
                                  bool transparency)
{
    QMutexLocker locker(&m_mutex);
    
    // Unload previous sprites
    unloadSprites();
    
    // Store current parameters
    m_currentFile = filename;
    m_currentClient = client;
    
    // Load sprites using sprite reader
    bool success = m_spriteReader->loadSprites(filename, client, extended, transparency);
    
    if (success) {
        m_loaded = true;
        qDebug() << "Successfully loaded sprites from:" << filename;
    } else {
        m_loaded = false;
        m_currentFile.clear();
        emit spriteLoadError(QString("Failed to load sprite file: %1").arg(filename));
    }
    
    return success;
}

void SpriteManager::unloadSprites()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_loaded) {
        m_spriteReader->clearCache();
        m_loaded = false;
        m_currentFile.clear();
        emit spritesUnloaded();
    }
}

bool SpriteManager::isLoaded() const
{
    QMutexLocker locker(&m_mutex);
    return m_loaded && m_spriteReader->isLoaded();
}

Sprite* SpriteManager::getSprite(quint32 id) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_loaded) {
        return nullptr;
    }
    
    return m_spriteReader->getSprite(id);
}

bool SpriteManager::hasSprite(quint32 id) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_loaded) {
        return false;
    }
    
    return m_spriteReader->hasSprite(id);
}

QList<quint32> SpriteManager::getSpriteIds() const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_loaded) {
        return QList<quint32>();
    }
    
    return m_spriteReader->getSpriteIds();
}

void SpriteManager::loadSpritesForItem(ClientItem* item, const QList<quint32>& spriteIds)
{
    if (!item || !m_loaded) {
        return;
    }
    
    // Clear existing sprites
    item->clearSprites();
    
    // Load new sprites
    for (quint32 spriteId : spriteIds) {
        Sprite* sprite = getSprite(spriteId);
        if (sprite) {
            item->addSprite(sprite);
        }
    }
    
    // Update sprite count
    item->setNumSprites(spriteIds.size());
    
    // Generate bitmap from sprites
    item->generateBitmap();
}

void SpriteManager::clearItemSprites(ClientItem* item)
{
    if (!item) {
        return;
    }
    
    item->clearSprites();
    item->setNumSprites(0);
    item->setBitmap(QPixmap()); // Clear bitmap
}

QString SpriteManager::currentFile() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentFile;
}

quint32 SpriteManager::spriteCount() const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_loaded) {
        return 0;
    }
    
    return m_spriteReader->spriteCount();
}

void SpriteManager::clearCache()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_spriteReader) {
        m_spriteReader->clearCache();
    }
}

void SpriteManager::setMaxCacheSize(int maxSize)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_spriteReader) {
        m_spriteReader->setMaxCacheSize(maxSize);
    }
}

void SpriteManager::onSpriteReaderProgress(int current, int total)
{
    emit loadingProgress(current, total);
}

void SpriteManager::onSpriteReaderFinished(bool success)
{
    if (success) {
        emit spritesLoaded(m_currentFile);
    } else {
        m_loaded = false;
        m_currentFile.clear();
        emit spriteLoadError("Sprite loading failed");
    }
}

} // namespace ItemEditor