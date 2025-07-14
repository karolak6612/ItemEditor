/**
 * Item Editor Qt6 - Sprite Class Implementation
 * Exact mirror of Legacy_App/csharp/Source/PluginInterface/Sprite.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "Sprite.h"
#include "IPlugin.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QRect>
#include <QImage>

namespace ItemEditor {

// Static member initialization
QByteArray Sprite::s_blankRGBSprite;
QByteArray Sprite::s_blankARGBSprite;

Sprite::Sprite()
    : m_id(0)
    , m_size(0)
    , m_transparent(false)
{
    // Constructor - exact mirror of C# constructor
}

QByteArray Sprite::getRGBData()
{
    // Exact mirror of C# GetRGBData() method
    try {
        const quint8 transparentColor = 0x11;
        return getRGBData(transparentColor);
    }
    catch (...) {
        qWarning() << QString("Failed to get sprite id %1").arg(m_id);
        return s_blankRGBSprite;
    }
}

QByteArray Sprite::getRGBData(quint8 transparentColor)
{
    // Exact mirror of C# GetRGBData(byte transparentColor) method
    if (m_compressedPixels.isEmpty() || static_cast<quint32>(m_compressedPixels.size()) != m_size) {
        return s_blankRGBSprite;
    }

    QByteArray rgb32x32x3(RGB_PIXELS_DATA_SIZE, 0);
    quint32 bytes = 0;
    quint32 x = 0;
    quint32 y = 0;
    qint32 chunkSize;
    quint8 bitPerPixel = m_transparent ? 4 : 3;

    while (bytes < m_size) {
        // Read transparent pixel chunk size
        if (bytes + 1 >= m_size) break;
        chunkSize = static_cast<quint8>(m_compressedPixels[bytes]) | 
                   (static_cast<quint8>(m_compressedPixels[bytes + 1]) << 8);
        bytes += 2;

        // Fill transparent pixels
        for (int i = 0; i < chunkSize; ++i) {
            if (y < DEFAULT_SIZE && x < DEFAULT_SIZE) {
                rgb32x32x3[96 * y + x * 3 + 0] = transparentColor;
                rgb32x32x3[96 * y + x * 3 + 1] = transparentColor;
                rgb32x32x3[96 * y + x * 3 + 2] = transparentColor;
            }
            x++;
            if (x >= DEFAULT_SIZE) {
                x = 0;
                ++y;
            }
        }

        // Check if we're done
        if (bytes >= m_size) break;

        // Read colored pixel chunk size
        if (bytes + 1 >= m_size) break;
        chunkSize = static_cast<quint8>(m_compressedPixels[bytes]) | 
                   (static_cast<quint8>(m_compressedPixels[bytes + 1]) << 8);
        bytes += 2;

        // Fill colored pixels
        for (int i = 0; i < chunkSize; ++i) {
            if (bytes + bitPerPixel > m_size) break;
            
            quint8 red = static_cast<quint8>(m_compressedPixels[bytes + 0]);
            quint8 green = static_cast<quint8>(m_compressedPixels[bytes + 1]);
            quint8 blue = static_cast<quint8>(m_compressedPixels[bytes + 2]);

            if (y < DEFAULT_SIZE && x < DEFAULT_SIZE) {
                rgb32x32x3[96 * y + x * 3 + 0] = red;
                rgb32x32x3[96 * y + x * 3 + 1] = green;
                rgb32x32x3[96 * y + x * 3 + 2] = blue;
            }

            bytes += bitPerPixel;
            x++;
            if (x >= DEFAULT_SIZE) {
                x = 0;
                ++y;
            }
        }
    }

    // Fill remaining pixels with transparent color
    while (y < DEFAULT_SIZE) {
        while (x < DEFAULT_SIZE) {
            rgb32x32x3[96 * y + x * 3 + 0] = transparentColor;
            rgb32x32x3[96 * y + x * 3 + 1] = transparentColor;
            rgb32x32x3[96 * y + x * 3 + 2] = transparentColor;
            x++;
        }
        x = 0;
        ++y;
    }

    return rgb32x32x3;
}

QByteArray Sprite::getPixels()
{
    // Exact mirror of C# GetPixels() method
    if (m_compressedPixels.isEmpty() || static_cast<quint32>(m_compressedPixels.size()) != m_size) {
        return s_blankARGBSprite;
    }

    qint32 read = 0;
    qint32 write = 0;
    qint32 pos = 0;
    qint32 transparentPixels = 0;
    qint32 coloredPixels = 0;
    qint32 length = m_compressedPixels.length();
    quint8 bitPerPixel = m_transparent ? 4 : 3;
    QByteArray pixels(ARGB_PIXELS_DATA_SIZE, 0);

    for (read = 0; read < length; read += 4 + (bitPerPixel * coloredPixels)) {
        if (pos + 1 >= length) break;
        transparentPixels = static_cast<quint8>(m_compressedPixels[pos++]) | 
                           (static_cast<quint8>(m_compressedPixels[pos++]) << 8);
        
        if (pos + 1 >= length) break;
        coloredPixels = static_cast<quint8>(m_compressedPixels[pos++]) | 
                       (static_cast<quint8>(m_compressedPixels[pos++]) << 8);

        // Fill transparent pixels
        for (int i = 0; i < transparentPixels && write < ARGB_PIXELS_DATA_SIZE - 3; i++) {
            pixels[write++] = 0x00; // Blue
            pixels[write++] = 0x00; // Green
            pixels[write++] = 0x00; // Red
            pixels[write++] = 0x00; // Alpha
        }

        // Fill colored pixels
        for (int i = 0; i < coloredPixels && write < ARGB_PIXELS_DATA_SIZE - 3; i++) {
            if (pos + 2 >= length) break;
            
            quint8 red = static_cast<quint8>(m_compressedPixels[pos++]);
            quint8 green = static_cast<quint8>(m_compressedPixels[pos++]);
            quint8 blue = static_cast<quint8>(m_compressedPixels[pos++]);
            quint8 alpha = m_transparent ? (pos < length ? static_cast<quint8>(m_compressedPixels[pos++]) : 0xFF) : 0xFF;

            pixels[write++] = blue;
            pixels[write++] = green;
            pixels[write++] = red;
            pixels[write++] = alpha;
        }
    }

    // Fill remaining pixels with transparent
    while (write < ARGB_PIXELS_DATA_SIZE) {
        pixels[write++] = 0x00; // Blue
        pixels[write++] = 0x00; // Green
        pixels[write++] = 0x00; // Red
        pixels[write++] = 0x00; // Alpha
    }

    return pixels;
}

QPixmap Sprite::getPixmap()
{
    // Qt6 equivalent of C# GetBitmap() method
    QByteArray pixels = getPixels();
    if (pixels.isEmpty()) {
        QPixmap pixmap(DEFAULT_SIZE, DEFAULT_SIZE);
        pixmap.fill(Qt::transparent);
        return pixmap;
    }
    
    // Create QImage from ARGB pixel data
    // Qt expects ARGB32 format: 0xAARRGGBB in memory as BBGGRRAA (little-endian)
    // Our pixel data is already in BGRA format from getPixels()
    QImage image(reinterpret_cast<const uchar*>(pixels.constData()), 
                 DEFAULT_SIZE, DEFAULT_SIZE, 
                 QImage::Format_ARGB32);
    
    // Convert to QPixmap
    return QPixmap::fromImage(image);
}

void Sprite::createBlankSprite()
{
    // Exact mirror of C# CreateBlankSprite() static method
    s_blankRGBSprite.resize(RGB_PIXELS_DATA_SIZE);
    s_blankARGBSprite.resize(ARGB_PIXELS_DATA_SIZE);
    
    for (int i = 0; i < RGB_PIXELS_DATA_SIZE; i++) {
        s_blankRGBSprite[i] = 0x11;
    }
    
    for (int i = 0; i < ARGB_PIXELS_DATA_SIZE; i++) {
        s_blankARGBSprite[i] = 0x11;
    }
}

bool Sprite::loadSprites(const QString& filename, 
                        QMap<quint32, Sprite*>& sprites, 
                        const SupportedClient& client, 
                        bool extended, 
                        bool transparency)
{
    // Exact mirror of C# LoadSprites() static method
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    try {
        quint32 sprSignature;
        stream >> sprSignature;
        
        if (client.sprSignature() != sprSignature) {
            qWarning() << QString("Bad spr signature. Expected signature is %1 and loaded signature is %2")
                         .arg(client.sprSignature(), 0, 16)
                         .arg(sprSignature, 0, 16);
            return false;
        }
        
        quint32 totalPics;
        if (extended) {
            stream >> totalPics;
        } else {
            quint16 pics;
            stream >> pics;
            totalPics = pics;
        }
        
        // Read sprite indexes
        QList<quint32> spriteIndexes;
        for (quint32 i = 0; i < totalPics; ++i) {
            quint32 index;
            stream >> index;
            spriteIndexes.append(index);
        }
        
        // Load sprite data
        quint32 id = 1;
        for (quint32 element : spriteIndexes) {
            quint32 index = element + 3;
            if (!file.seek(index)) {
                continue;
            }
            
            quint16 size;
            stream >> size;
            
            if (sprites.contains(id)) {
                Sprite* sprite = sprites[id];
                if (sprite != nullptr && size > 0) {
                    if (sprite->m_size > 0) {
                        // Generate warning - sprite already loaded
                        qWarning() << QString("Sprite %1 already loaded, skipping").arg(id);
                    } else {
                        sprite->m_id = id;
                        sprite->m_size = size;
                        sprite->m_compressedPixels = stream.device()->read(size);
                        sprite->m_transparent = transparency;
                    }
                }
            } else {
                // Skip sprite data if not in map
                stream.skipRawData(size);
            }
            
            ++id;
        }
        
        return true;
    }
    catch (...) {
        return false;
    }
}

} // namespace ItemEditor