#include "ClientDataTypes.h"
#include <QDebug>

QByteArray SpriteData::getRGBData() const
{
    if (!isValid()) {
        return QByteArray();
    }
    
    // For now, create a simple RGB representation
    // In a full implementation, this would decompress the sprite data
    QByteArray rgbData(RGBPixelsDataSize, 0x11); // Initialize with transparent color
    
    // Simple pattern generation for testing
    for (int y = 0; y < DefaultSize; ++y) {
        for (int x = 0; x < DefaultSize; ++x) {
            int offset = y * DefaultSize * 3 + x * 3;
            if (offset + 2 < rgbData.size()) {
                // Create a simple pattern based on sprite ID
                rgbData[offset + 0] = static_cast<char>((id * 17 + x) % 256); // Red
                rgbData[offset + 1] = static_cast<char>((id * 23 + y) % 256); // Green  
                rgbData[offset + 2] = static_cast<char>((id * 31 + x + y) % 256); // Blue
            }
        }
    }
    
    return rgbData;
}

QByteArray SpriteData::getPixels() const
{
    // For testing, return the same as RGB data
    return getRGBData();
}