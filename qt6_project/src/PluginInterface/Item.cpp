/**
 * Item Editor Qt6 - Item Classes Implementation
 * Exact mirror of Legacy_App/csharp/Source/PluginInterface/Item.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "Item.h"
#include "Sprite.h"
#include <QCryptographicHash>
#include <QBuffer>
#include <QPainter>

namespace ItemEditor {

// Item base class implementation
Item::Item(QObject *parent)
    : QObject(parent)
    , m_id(0)
    , m_type(ServerItemType::None)
    , m_hasStackOrder(false)
    , m_stackOrder(TileStackOrder::None)
    , m_unpassable(false)
    , m_blockMissiles(false)
    , m_blockPathfinder(false)
    , m_hasElevation(false)
    , m_forceUse(false)
    , m_multiUse(false)
    , m_pickupable(false)
    , m_movable(true)
    , m_stackable(false)
    , m_readable(false)
    , m_rotatable(false)
    , m_hangable(false)
    , m_hookSouth(false)
    , m_hookEast(false)
    , m_hasCharges(false)
    , m_ignoreLook(false)
    , m_fullGround(false)
    , m_allowDistanceRead(false)
    , m_isAnimation(false)
    , m_groundSpeed(0)
    , m_lightLevel(0)
    , m_lightColor(0)
    , m_maxReadChars(0)
    , m_maxReadWriteChars(0)
    , m_minimapColor(0)
    , m_tradeAs(0)
    , m_name()
{
}

bool Item::equals(const Item& item) const
{
    return m_type == item.m_type &&
           m_stackOrder == item.m_stackOrder &&
           m_unpassable == item.m_unpassable &&
           m_blockMissiles == item.m_blockMissiles &&
           m_blockPathfinder == item.m_blockPathfinder &&
           m_hasElevation == item.m_hasElevation &&
           m_forceUse == item.m_forceUse &&
           m_multiUse == item.m_multiUse &&
           m_pickupable == item.m_pickupable &&
           m_movable == item.m_movable &&
           m_stackable == item.m_stackable &&
           m_readable == item.m_readable &&
           m_rotatable == item.m_rotatable &&
           m_hangable == item.m_hangable &&
           m_hookSouth == item.m_hookSouth &&
           m_hookEast == item.m_hookEast &&
           m_hasCharges == item.m_hasCharges &&
           m_ignoreLook == item.m_ignoreLook &&
           m_fullGround == item.m_fullGround &&
           m_allowDistanceRead == item.m_allowDistanceRead &&
           m_isAnimation == item.m_isAnimation &&
           m_groundSpeed == item.m_groundSpeed &&
           m_lightLevel == item.m_lightLevel &&
           m_lightColor == item.m_lightColor &&
           m_maxReadChars == item.m_maxReadChars &&
           m_maxReadWriteChars == item.m_maxReadWriteChars &&
           m_minimapColor == item.m_minimapColor &&
           m_tradeAs == item.m_tradeAs &&
           m_name == item.m_name;
}

QString Item::toString() const
{
    if (!m_name.isEmpty()) {
        return QString("%1 - %2").arg(m_id).arg(m_name);
    } else {
        return QString("Item %1").arg(m_id);
    }
}

// ClientItem implementation
ClientItem::ClientItem(QObject *parent)
    : Item(parent)
    , m_width(1)
    , m_height(1)
    , m_layers(1)
    , m_patternX(1)
    , m_patternY(1)
    , m_patternZ(1)
    , m_frames(1)
    , m_numSprites(0)
    , m_spriteList()
    , m_bitmap()
{
}

QPixmap ClientItem::getBitmap() const
{
    // Auto-generate bitmap if not available and sprites exist
    if (m_bitmap.isNull() && !m_spriteList.isEmpty()) {
        const_cast<ClientItem*>(this)->generateBitmap();
    }
    return m_bitmap;
}

void ClientItem::setBitmap(const QPixmap& bitmap)
{
    if (m_bitmap.cacheKey() != bitmap.cacheKey()) {
        m_bitmap = bitmap;
        emit itemChanged();
    }
}

void ClientItem::generateBitmap()
{
    // Generate bitmap from sprite list - exact mirror of C# implementation
    if (m_spriteList.isEmpty()) {
        qDebug() << "ClientItem::generateBitmap: No sprites available for item" << id();
        m_bitmap = QPixmap();
        return;
    }
    
    // Calculate total bitmap size
    int totalWidth = m_width * Sprite::DEFAULT_SIZE;
    int totalHeight = m_height * Sprite::DEFAULT_SIZE;
    
    // Validate dimensions to prevent excessive memory usage
    if (totalWidth > 1024 || totalHeight > 1024) {
        qWarning() << QString("ClientItem %1: Bitmap size too large (%2x%3), skipping generation")
                      .arg(id()).arg(totalWidth).arg(totalHeight);
        m_bitmap = QPixmap();
        return;
    }
    
    if (totalWidth <= 0 || totalHeight <= 0) {
        qWarning() << QString("ClientItem %1: Invalid bitmap dimensions (%2x%3)")
                      .arg(id()).arg(totalWidth).arg(totalHeight);
        m_bitmap = QPixmap();
        return;
    }
    
    // Create composite bitmap
    QPixmap compositeBitmap(totalWidth, totalHeight);
    compositeBitmap.fill(Qt::transparent);
    
    QPainter painter(&compositeBitmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false); // Disable for performance
    
    // Render each layer with enhanced error handling
    int renderedSprites = 0;
    int totalExpectedSprites = m_layers * m_width * m_height;
    
    for (quint8 l = 0; l < m_layers; l++) {
        for (quint8 h = 0; h < m_height; h++) {
            for (quint8 w = 0; w < m_width; w++) {
                int index = w + h * m_width + l * m_width * m_height;
                if (index < m_spriteList.size()) {
                    Sprite* sprite = m_spriteList[index];
                    if (sprite && sprite->size() > 0) {
                        try {
                            QPixmap spritePixmap = sprite->getPixmap();
                            if (!spritePixmap.isNull() && spritePixmap.width() > 0 && spritePixmap.height() > 0) {
                                int x = w * Sprite::DEFAULT_SIZE;
                                int y = h * Sprite::DEFAULT_SIZE;
                                
                                // Ensure we don't draw outside bounds
                                if (x + spritePixmap.width() <= totalWidth && y + spritePixmap.height() <= totalHeight) {
                                    painter.drawPixmap(x, y, spritePixmap);
                                    renderedSprites++;
                                } else {
                                    qWarning() << QString("ClientItem %1: Sprite %2 would exceed bitmap bounds")
                                                  .arg(id()).arg(index);
                                }
                            } else {
                                qDebug() << QString("ClientItem %1: Sprite %2 has null or invalid pixmap")
                                            .arg(id()).arg(index);
                            }
                        }
                        catch (const std::exception& e) {
                            qWarning() << QString("ClientItem %1: Exception rendering sprite %2: %3")
                                          .arg(id()).arg(index).arg(e.what());
                        }
                        catch (...) {
                            qWarning() << QString("ClientItem %1: Unknown exception rendering sprite %2")
                                          .arg(id()).arg(index);
                        }
                    } else {
                        qDebug() << QString("ClientItem %1: Sprite %2 is null or has zero size")
                                    .arg(id()).arg(index);
                    }
                } else {
                    qDebug() << QString("ClientItem %1: Sprite index %2 out of range (have %3 sprites)")
                                .arg(id()).arg(index).arg(m_spriteList.size());
                }
            }
        }
    }
    
    painter.end();
    
    // Only set bitmap if we successfully rendered at least one sprite
    if (renderedSprites > 0) {
        setBitmap(compositeBitmap);
        qDebug() << QString("ClientItem %1: Successfully generated bitmap with %2/%3 sprites (%4x%5)")
                    .arg(id()).arg(renderedSprites).arg(totalExpectedSprites)
                    .arg(totalWidth).arg(totalHeight);
    } else {
        qWarning() << QString("ClientItem %1: Failed to render any sprites (0/%2)")
                      .arg(id()).arg(totalExpectedSprites);
        m_bitmap = QPixmap();
    }
}

QByteArray ClientItem::spriteHash() const
{
    if (m_spriteHash.isEmpty() && !m_spriteList.isEmpty()) {
        // Calculate sprite hash similar to C# implementation
        QCryptographicHash hash(QCryptographicHash::Md5);
        QByteArray rgbaData(Sprite::ARGB_PIXELS_DATA_SIZE, 0);
        
        int spriteBase = 0;
        for (quint8 l = 0; l < m_layers; l++) {
            for (quint8 h = 0; h < m_height; h++) {
                for (quint8 w = 0; w < m_width; w++) {
                    int index = spriteBase + w + h * m_width + l * m_width * m_height;
                    if (index < m_spriteList.size()) {
                        Sprite* sprite = m_spriteList[index];
                        if (sprite) {
                            QByteArray rgbData = sprite->getRGBData();
                            
                            // Convert RGB to RGBA with vertical flip (like C# implementation)
                            for (int y = 0; y < Sprite::DEFAULT_SIZE; ++y) {
                                for (int x = 0; x < Sprite::DEFAULT_SIZE; ++x) {
                                    int rgbaIndex = 128 * y + x * 4;
                                    int rgbIndex = (32 - y - 1) * 96 + x * 3;
                                    
                                    if (rgbIndex + 2 < rgbData.size() && rgbaIndex + 3 < rgbaData.size()) {
                                        rgbaData[rgbaIndex + 0] = rgbData[rgbIndex + 2]; // blue
                                        rgbaData[rgbaIndex + 1] = rgbData[rgbIndex + 1]; // green
                                        rgbaData[rgbaIndex + 2] = rgbData[rgbIndex + 0]; // red
                                        rgbaData[rgbaIndex + 3] = 0; // alpha
                                    }
                                }
                            }
                            
                            hash.addData(rgbaData);
                        }
                    }
                }
            }
        }
        
        const_cast<ClientItem*>(this)->m_spriteHash = hash.result();
    }
    
    return m_spriteHash;
}

void ClientItem::setSpriteHash(const QByteArray& spriteHash)
{
    m_spriteHash = spriteHash;
}

// ServerItem implementation
ServerItem::ServerItem(QObject *parent)
    : Item(parent)
    , m_clientId(0)
    , m_description()
{
}

} // namespace ItemEditor