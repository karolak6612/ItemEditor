#include "sprparser.h"
#include <QDebug>

namespace TibiaData {

SprParser::SprParser() : m_signature(0), m_spriteCount(0)
{
}

bool SprParser::loadSpr(const QString& filePath, bool isExtended, QString& errorString)
{
    m_file.setFileName(filePath);
    if (!m_file.open(QIODevice::ReadOnly)) {
        errorString = QObject::tr("Failed to open SPR file: %1 - %2").arg(filePath, m_file.errorString());
        return false;
    }

    QDataStream stream(&m_file);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream >> m_signature;

    // Handle extended (uint32) vs non-extended (uint16) sprite count
    if (isExtended) {
        stream >> m_spriteCount;
    } else {
        quint16 spriteCount16;
        stream >> spriteCount16;
        m_spriteCount = spriteCount16;
    }

    qDebug() << "SPR Signature:" << Qt::hex << m_signature;
    qDebug() << "SPR Sprite Count:" << m_spriteCount << "(isExtended:" << isExtended << ")";

    if (m_spriteCount == 0 || m_spriteCount > 100000) { // Sanity check
        errorString = QObject::tr("Invalid sprite count in SPR file: %1").arg(m_spriteCount);
        m_file.close();
        return false;
    }

    m_spriteAddresses.resize(static_cast<int>(m_spriteCount + 1)); // +1 because IDs are 1-based

    for (quint32 i = 1; i <= m_spriteCount; ++i) { // Sprite IDs are typically 1-based
        quint32 address;
        stream >> address;
        if (address == 0) { // Address 0 means sprite does not exist (transparent)
            m_spriteAddresses[static_cast<int>(i)] = 0;
            // qDebug() << "Sprite ID" << i << "has address 0 (non-existent)";
        } else {
            m_spriteAddresses[static_cast<int>(i)] = address;
        }
        if (stream.atEnd() && i < m_spriteCount) {
            errorString = QObject::tr("Unexpected end of file while reading sprite addresses in SPR file.");
            m_file.close();
            return false;
        }
    }

    // File is kept open by m_file for on-demand sprite reading
    // m_file.close(); // Or close and reopen in getSprite
    qDebug() << "SPR file" << filePath << "loaded headers. Sprite count:" << m_spriteCount;
    return true;
}

quint32 SprParser::getSpriteCount() const
{
    return m_spriteCount;
}

quint32 SprParser::getSignature() const
{
    return m_signature;
}


// Placeholder for actual sprite data parsing
bool SprParser::getSprite(quint32 spriteId, OTB::Sprite& outSprite, bool transparent) const
{
    if (spriteId == 0 || spriteId > m_spriteCount) {
        qWarning() << "Requested spriteId" << spriteId << "is out of bounds (1 to" << m_spriteCount << ")";
        return false; // Sprite ID 0 is invalid or means "no sprite"
    }

    quint32 address = m_spriteAddresses.value(static_cast<int>(spriteId), 0);
    if (address == 0) { // Sprite does not exist (fully transparent placeholder)
        outSprite.id = spriteId;
        outSprite.size = 0;
        outSprite.compressedPixels.clear();
        outSprite.transparent = true; // Assumed for non-existent sprites
        // qDebug() << "Sprite ID" << spriteId << "does not exist (address 0). Returning blank sprite.";
        return true; // Successfully "got" a blank sprite
    }

    if (!m_file.isOpen()) {
        // This case should ideally not happen if loadSpr keeps file open,
        // or getSprite re-opens it. For now, assume it's open.
        qWarning() << "SPR file is not open in getSprite";
        return false;
    }

    // Cast away const for seek (QFile is not const-correct for seek with internal QDataStream)
    QFile* nonConstFile = const_cast<QFile*>(&m_file);
    if (!nonConstFile->seek(address)) {
        qWarning() << "Failed to seek to sprite address" << address << "for sprite ID" << spriteId;
        return false;
    }

    QDataStream stream(nonConstFile); // Use the existing QFile object
    stream.setByteOrder(QDataStream::LittleEndian);

    return parseSpriteData(spriteId, stream, outSprite, transparent);
}


// This is where the core SPR RLE-like decompression logic from C# Sprite.cs needs to be ported.
// byte[] Sprite.GetRGBData(byte transparentColor) or byte[] Sprite.GetPixels()
bool SprParser::parseSpriteData(quint32 spriteId, QDataStream& stream, OTB::Sprite& outSprite, bool isTransparentByDefault) const
{
    // Read color key (RGB, 3 bytes) - C# does not seem to use this for ItemEditor's purposes.
    // It's usually 0xFF00FF (magenta) for transparency in some contexts, but ItemEditor's
    // Sprite.cs GetRGBData/GetPixels uses a fixed transparent color (0x11) or alpha=0.
    // stream.skipRawData(3); // Skip 3 bytes of color key.

    quint16 pixelDataSize; // This is the size of the *compressed* pixel data that follows.
    stream >> pixelDataSize;

    if (pixelDataSize == 0 || pixelDataSize > 4096) { // Sanity check, max typical compressed size for 32x32
        qWarning() << "Invalid or unexpectedly large pixelDataSize" << pixelDataSize << "for sprite ID" << spriteId;
        // This might be a non-existent sprite if size is 0 after address.
        if (pixelDataSize == 0) {
            outSprite.id = spriteId;
            outSprite.size = 0;
            outSprite.compressedPixels.clear();
            outSprite.transparent = true;
            return true;
        }
        return false;
    }

    outSprite.id = spriteId;
    outSprite.size = pixelDataSize;
    outSprite.compressedPixels.resize(pixelDataSize);
    int bytesRead = stream.readRawData(outSprite.compressedPixels.data(), pixelDataSize);

    if (bytesRead != pixelDataSize) {
        qWarning() << "Failed to read full pixel data for sprite ID" << spriteId
                   << ". Expected" << pixelDataSize << "got" << bytesRead;
        outSprite.compressedPixels.clear();
        outSprite.size = 0;
        return false;
    }

    // The 'transparent' flag in OTB::Sprite is set by the C# Sprite.LoadSprites
    // based on a parameter passed in, which itself comes from client settings or detection.
    // For now, we pass it into this function.
    outSprite.transparent = isTransparentByDefault;

    // Decompression to raw pixels (e.g., ARGB) would happen here or in OTB::Sprite methods.
    // The OTB::Sprite class will store this compressedPixels array.
    // Its methods like getBitmap() will perform the actual decompression.
    // This SprParser::getSprite just extracts the compressed data.

    return true;
}


} // namespace TibiaData
