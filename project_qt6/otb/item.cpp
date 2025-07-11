#include "item.h"
#include <QDebug>
#include <QCryptographicHash> // For dummy sprite hash calculation

namespace OTB {

// --- Sprite ---
QByteArray Sprite::blankRGBSprite;
QByteArray Sprite::blankARGBSprite;

QByteArray Sprite::getRGBData(quint8 transparentRgbColor) const {
    // Stub implementation
    if (compressedPixels.isEmpty()) {
        // Return a copy of blankRGBSprite to avoid issues if caller modifies it
        return Sprite::blankRGBSprite;
    }
    // Dummy decompression: just return a small array indicating it's not blank
    // A real implementation would decompress 'compressedPixels'.
    QByteArray dummyData(96, transparentRgbColor); // 32*3 pixels
    dummyData[0] = id % 255; // Make it slightly unique for testing
    return dummyData;
}

QByteArray Sprite::getPixelsARGB() const {
    // Stub implementation
    if (compressedPixels.isEmpty()) {
        return Sprite::blankARGBSprite;
    }
    // Dummy:
    QByteArray dummyData(128, 0x00); // 32*4 pixels, fully transparent black
    if (id > 0) { // Make it slightly visible for testing
        dummyData[0] = id % 255; // B
        dummyData[1] = (id >> 8) % 255; // G
        dummyData[2] = (id >> 16) % 255; // R
        dummyData[3] = 0xFF; // Alpha
    }
    return dummyData;
}

QImage Sprite::getBitmap() const {
    // Stub implementation
    // A real implementation would use getPixelsARGB() and create a QImage.
    // For now, return a 32x32 image, possibly with a color based on ID.
    QImage image(DefaultSize, DefaultSize, QImage::Format_ARGB32);
    if (id == 0 && compressedPixels.isEmpty()) { // Blank sprite
        image.fill(QColor(0x11, 0x11, 0x11, 0xFF)); // C#'s transparent color, opaque
    } else {
        // Simple colored square based on ID or a hash of compressedPixels for visual distinction
        quint32 colorId = id;
        if (id == 0 && !compressedPixels.isEmpty()) { // Non-blank sprite with ID 0 (should not happen in real SPR)
            QByteArray hash = QCryptographicHash::hash(compressedPixels, QCryptographicHash::Md5).left(4);
            memcpy(&colorId, hash.constData(), 4);
        }
        image.fill(QColor((colorId & 0xFF0000) >> 16, (colorId & 0x00FF00) >> 8, colorId & 0x0000FF));
    }
    return image;
}

void Sprite::createBlankSprite() {
    blankRGBSprite.resize(RGBPixelsDataSize);
    blankRGBSprite.fill(0x11);

    blankARGBSprite.resize(ARGBPixelsDataSize);
    // For ARGB, alpha should be 0 for "blank" transparent, or 0xFF with magic color if that's how C# handles it.
    // C# Sprite.GetPixels() returns 00 for alpha on transparent sections.
    // Let's make it fully transparent black.
    blankARGBSprite.fill(0x00);
}


// --- ClientItem ---
const QByteArray& ClientItem::getSpriteHash() {
    // Stub: In C#, this calculates MD5 from all sprite layers if spriteHash member is null.
    // For now, if it's empty (default 16 zeros), "calculate" a dummy one.
    if (std::all_of(spriteHash.constBegin(), spriteHash.constEnd(), [](char c){ return c == 0; })) {
        if (id > 0) { // Make it somewhat unique for testing
            QCryptographicHash hasher(QCryptographicHash::Md5);
            hasher.addData(QByteArray::number(id));
            hasher.addData(name.toUtf8());
            spriteHash = hasher.result(); // MD5 is 16 bytes
        }
    }
    return spriteHash;
}

QImage ClientItem::getBitmap() const {
    // Stub: C# ClientItem::GetBitmap combines layers of sprites.
    // For now, if spriteList is not empty, return the bitmap of the first sprite.
    // Otherwise, return a default 32x32 image.
    if (!spriteList.isEmpty()) {
        return spriteList.first().getBitmap();
    }
    QImage dummyImage(Sprite::DefaultSize, Sprite::DefaultSize, QImage::Format_ARGB32);
    dummyImage.fill(Qt::gray); // Default placeholder
    return dummyImage;
}

void ClientItem::generateSignature() {
    // Stub: C# ClientItem::GenerateSignature does complex Fourier transform.
    // For now, just populate spriteSignature QVariantMap with some dummy values.
    spriteSignature.clear();
    QVariantMap row0, row1;
    row0["col0"] = QVariant(static_cast<double>(id) * 0.1);
    row0["col1"] = QVariant(static_cast<double>(id) * 0.2);
    row1["col0"] = QVariant(static_cast<double>(id) * 0.3);
    row1["col1"] = QVariant(static_cast<double>(id) * 0.4);
    spriteSignature["row0"] = row0;
    spriteSignature["row1"] = row1;
    qDebug() << "Dummy ClientItem::generateSignature() called for ID:" << id;
}

// --- ItemBase ---
// bool ItemBase::equals(const ItemBase& other) const { /* TODO */ return false; }
// void ItemBase::copyPropertiesFrom(const ItemBase& source) { /* TODO */ }


} // namespace OTB
