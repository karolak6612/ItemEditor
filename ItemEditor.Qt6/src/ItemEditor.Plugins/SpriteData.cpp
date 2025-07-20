#include "SpriteData.h"
#include <QDebug>

SpriteDataManager::SpriteDataManager(QObject* parent)
    : QObject(parent)
{
}

SpriteDataManager::~SpriteDataManager()
{
}

// Implementation of SpriteData methods from ClientDataTypes.h
QByteArray SpriteData::getRGBData() const
{
    if (compressedPixels.isEmpty() || compressedPixels.size() != static_cast<int>(size)) {
        // Return blank sprite data
        QByteArray blankSprite(RGBPixelsDataSize, 0x11);
        return blankSprite;
    }

    const quint8 transparentColor = 0x11;
    QByteArray rgb32x32x3(RGBPixelsDataSize, 0);
    
    quint32 bytes = 0;
    quint32 x = 0;
    quint32 y = 0;
    qint32 chunkSize;
    quint8 bitPerPixel = transparent ? 4 : 3;

    while (bytes < size) {
        // Read transparent pixel count
        if (bytes + 1 >= size) break;
        chunkSize = static_cast<quint8>(compressedPixels[bytes]) | 
                   (static_cast<quint8>(compressedPixels[bytes + 1]) << 8);
        bytes += 2;

        // Fill transparent pixels
        for (int i = 0; i < chunkSize; ++i) {
            if (y >= DefaultSize) break;
            
            rgb32x32x3[96 * y + x * 3 + 0] = transparentColor;
            rgb32x32x3[96 * y + x * 3 + 1] = transparentColor;
            rgb32x32x3[96 * y + x * 3 + 2] = transparentColor;
            
            x++;
            if (x >= DefaultSize) {
                x = 0;
                ++y;
            }
        }

        // Check if we're done
        if (bytes >= size) break;

        // Read colored pixel count
        if (bytes + 1 >= size) break;
        chunkSize = static_cast<quint8>(compressedPixels[bytes]) | 
                   (static_cast<quint8>(compressedPixels[bytes + 1]) << 8);
        bytes += 2;

        // Fill colored pixels
        for (int i = 0; i < chunkSize; ++i) {
            if (bytes + bitPerPixel > size || y >= DefaultSize) break;
            
            quint8 red = static_cast<quint8>(compressedPixels[bytes + 0]);
            quint8 green = static_cast<quint8>(compressedPixels[bytes + 1]);
            quint8 blue = static_cast<quint8>(compressedPixels[bytes + 2]);

            rgb32x32x3[96 * y + x * 3 + 0] = red;
            rgb32x32x3[96 * y + x * 3 + 1] = green;
            rgb32x32x3[96 * y + x * 3 + 2] = blue;

            bytes += bitPerPixel;

            x++;
            if (x >= DefaultSize) {
                x = 0;
                ++y;
            }
        }
    }

    // Fill remaining pixels with transparent color
    while (y < DefaultSize) {
        while (x < DefaultSize) {
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

QByteArray SpriteData::getPixels() const
{
    if (compressedPixels.isEmpty() || compressedPixels.size() != static_cast<int>(size)) {
        // Return blank ARGB sprite data
        QByteArray blankSprite(ARGBPixelsDataSize, 0x00);
        return blankSprite;
    }

    int read = 0;
    int write = 0;
    int pos = 0;
    int transparentPixels = 0;
    int coloredPixels = 0;
    int length = compressedPixels.size();
    quint8 bitPerPixel = transparent ? 4 : 3;
    QByteArray pixels(ARGBPixelsDataSize, 0);

    for (read = 0; read < length; read += 4 + (bitPerPixel * coloredPixels)) {
        if (pos + 3 >= length) break;
        
        transparentPixels = static_cast<quint8>(compressedPixels[pos++]) | 
                           (static_cast<quint8>(compressedPixels[pos++]) << 8);
        coloredPixels = static_cast<quint8>(compressedPixels[pos++]) | 
                       (static_cast<quint8>(compressedPixels[pos++]) << 8);

        // Fill transparent pixels (BGRA format)
        for (int i = 0; i < transparentPixels && write < ARGBPixelsDataSize; i++) {
            pixels[write++] = 0x00; // Blue
            pixels[write++] = 0x00; // Green
            pixels[write++] = 0x00; // Red
            pixels[write++] = 0x00; // Alpha
        }

        // Fill colored pixels (BGRA format)
        for (int i = 0; i < coloredPixels && write < ARGBPixelsDataSize; i++) {
            if (pos + bitPerPixel > length) break;
            
            quint8 red = static_cast<quint8>(compressedPixels[pos++]);
            quint8 green = static_cast<quint8>(compressedPixels[pos++]);
            quint8 blue = static_cast<quint8>(compressedPixels[pos++]);
            quint8 alpha = transparent ? static_cast<quint8>(compressedPixels[pos++]) : 0xFF;

            pixels[write++] = blue;
            pixels[write++] = green;
            pixels[write++] = red;
            pixels[write++] = alpha;
        }
    }

    // Fill remaining pixels with transparent
    while (write < ARGBPixelsDataSize) {
        pixels[write++] = 0x00; // Blue
        pixels[write++] = 0x00; // Green
        pixels[write++] = 0x00; // Red
        pixels[write++] = 0x00; // Alpha
    }

    return pixels;
}