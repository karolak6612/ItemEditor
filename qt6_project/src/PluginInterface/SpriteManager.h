/**
 * Item Editor Qt6 - Sprite Manager Header
 * Manages sprite loading and provides access to sprites for items
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_SPRITEMANAGER_H
#define ITEMEDITOR_SPRITEMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QMutex>
#include "OTLib/SPR/SpriteReader.h"
#include "IPlugin.h"

namespace ItemEditor {

// Forward declarations
class Sprite;
class ClientItem;

/**
 * SpriteManager Class
 * Central manager for sprite loading and access
 * Provides thread-safe sprite management for the application
 */
class SpriteManager : public QObject
{
    Q_OBJECT

public:
    explicit SpriteManager(QObject *parent = nullptr);
    virtual ~SpriteManager();

    // Sprite file management
    bool loadSpriteFile(const QString& filename, 
                       const SupportedClient& client, 
                       bool extended = false, 
                       bool transparency = false);
    
    void unloadSprites();
    bool isLoaded() const;
    
    // Sprite access
    Sprite* getSprite(quint32 id) const;
    bool hasSprite(quint32 id) const;
    QList<quint32> getSpriteIds() const;
    
    // Client item integration
    void loadSpritesForItem(ClientItem* item, const QList<quint32>& spriteIds);
    void clearItemSprites(ClientItem* item);
    
    // File information
    QString currentFile() const;
    quint32 spriteCount() const;
    
    // Cache management
    void clearCache();
    void setMaxCacheSize(int maxSize);

signals:
    void spritesLoaded(const QString& filename);
    void spritesUnloaded();
    void loadingProgress(int current, int total);
    void spriteLoadError(const QString& error);

private slots:
    void onSpriteReaderProgress(int current, int total);
    void onSpriteReaderFinished(bool success);

private:
    SpriteReader* m_spriteReader;
    QString m_currentFile;
    mutable QMutex m_mutex;
    
    // Loading state
    bool m_loaded;
    SupportedClient m_currentClient;
};

} // namespace ItemEditor

#endif // ITEMEDITOR_SPRITEMANAGER_H