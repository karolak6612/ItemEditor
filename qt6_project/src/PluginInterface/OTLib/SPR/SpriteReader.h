/**
 * Item Editor Qt6 - Sprite Reader Header
 * Handles .spr file format loading and sprite management
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_SPRITEREADER_H
#define ITEMEDITOR_SPRITEREADER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QMutex>
#include <QCache>
#include "../../IPlugin.h"

namespace ItemEditor {

// Forward declarations
class Sprite;

/**
 * SpriteReader Class
 * Handles loading and caching of .spr sprite files
 * Provides thread-safe sprite access and memory management
 */
class SpriteReader : public QObject
{
    Q_OBJECT

public:
    explicit SpriteReader(QObject *parent = nullptr);
    virtual ~SpriteReader();

    // Main loading method
    bool loadSprites(const QString& filename, 
                    const SupportedClient& client, 
                    bool extended = false, 
                    bool transparency = false);

    // Sprite access methods
    Sprite* getSprite(quint32 id) const;
    bool hasSprite(quint32 id) const;
    QList<quint32> getSpriteIds() const;
    
    // Cache management
    void clearCache();
    void setMaxCacheSize(int maxSize);
    int cacheSize() const;
    
    // File information
    QString filename() const { return m_filename; }
    quint32 spriteCount() const { return m_spriteCount; }
    bool isLoaded() const { return m_loaded; }

signals:
    void loadingProgress(int current, int total);
    void loadingFinished(bool success);
    void spriteLoaded(quint32 id);

private:
    // Internal loading methods
    bool loadSpriteData(const QString& filename, 
                       const SupportedClient& client, 
                       bool extended, 
                       bool transparency);
    
    // Sprite creation and caching
    Sprite* createSprite(quint32 id, quint32 dataOffset, quint16 size);
    void cacheSprite(quint32 id, Sprite* sprite);
    
    // Thread safety
    mutable QMutex m_mutex;
    
    // File data
    QString m_filename;
    QByteArray m_fileData;
    QMap<quint32, quint32> m_spriteOffsets;  // id -> file offset
    QMap<quint32, quint16> m_spriteSizes;    // id -> data size
    
    // Cache management
    mutable QCache<quint32, Sprite> m_spriteCache;
    static const int DEFAULT_CACHE_SIZE = 1000;
    
    // Loading state
    bool m_loaded;
    quint32 m_spriteCount;
    SupportedClient m_client;
    bool m_extended;
    bool m_transparency;
};

} // namespace ItemEditor

#endif // ITEMEDITOR_SPRITEREADER_H