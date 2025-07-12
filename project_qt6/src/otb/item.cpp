#include "otb/item.h"
#include "otb/otbtypes.h"
#include <QDebug>
#include <QCryptographicHash>
#include <QPainter>
#include <QRect>

namespace ItemEditor {

// --- Sprite Implementation ---
QByteArray Sprite::blankRGBSprite;
QByteArray Sprite::blankARGBSprite;

QByteArray Sprite::getRGBData(quint8 transparentRgbColor) const {
    // Stub implementation - to be replaced with actual decompression
    if (compressedPixels.isEmpty()) {
        return Sprite::blankRGBSprite;
    }
    // Dummy decompression: just return a small array indicating it's not blank
    QByteArray dummyData(RGBPixelsDataSize, transparentRgbColor);
    dummyData[0] = id % 255; // Make it slightly unique for testing
    return dummyData;
}

QByteArray Sprite::getPixelsARGB() const {
    if (compressedPixels.isNull() || compressedPixels.isEmpty() || size == 0) {
        return Sprite::blankARGBSprite;
    }

    QByteArray pixels(ARGBPixelsDataSize, 0x00); // Initialize with transparent black
    const quint8* rleData = reinterpret_cast<const quint8*>(compressedPixels.constData());
    int readPos = 0;
    int writePos = 0;
    quint8 bytesPerPixel = transparent ? 4 : 3;

    try {
        while (readPos < size && writePos < ARGBPixelsDataSize) {
            if (readPos + 1 >= size) break;
            quint16 transparentPixels = static_cast<quint16>(rleData[readPos]) | (static_cast<quint16>(rleData[readPos + 1]) << 8);
            readPos += 2;

            // Write transparent pixels (alpha = 0)
            for (int i = 0; i < transparentPixels && writePos < ARGBPixelsDataSize; ++i) {
                pixels[writePos++] = 0x00; // Blue
                pixels[writePos++] = 0x00; // Green
                pixels[writePos++] = 0x00; // Red
                pixels[writePos++] = 0x00; // Alpha
            }
            if (writePos >= ARGBPixelsDataSize && readPos < size) break;

            if (readPos + 1 >= size) break;
            quint16 coloredPixels = static_cast<quint16>(rleData[readPos]) | (static_cast<quint16>(rleData[readPos + 1]) << 8);
            readPos += 2;

            for (int i = 0; i < coloredPixels && writePos < ARGBPixelsDataSize; ++i) {
                if (readPos + bytesPerPixel > size) {
                    qWarning() << "SPR Decompression: Unexpected end of RLE data for sprite ID" << id;
                    while(writePos < ARGBPixelsDataSize) pixels[writePos++] = 0x00;
                    return pixels;
                }
                quint8 red   = rleData[readPos + 0];
                quint8 green = rleData[readPos + 1];
                quint8 blue  = rleData[readPos + 2];
                quint8 alpha = transparent ? rleData[readPos + 3] : 0xFF;
                readPos += bytesPerPixel;

                pixels[writePos++] = blue;
                pixels[writePos++] = green;
                pixels[writePos++] = red;
                pixels[writePos++] = alpha;
            }
            if (writePos >= ARGBPixelsDataSize && readPos < size) break;
        }
    } catch (const std::exception& e) {
        qWarning() << "SPR Decompression: Error for sprite ID" << id << ":" << e.what();
        return Sprite::blankARGBSprite;
    }

    return pixels;
}

QImage Sprite::getBitmap() const {
    QByteArray argbData = getPixelsARGB();
    if (argbData.size() != ARGBPixelsDataSize) {
        return QImage(DefaultSize, DefaultSize, QImage::Format_ARGB32);
    }

    QImage image(reinterpret_cast<const uchar*>(argbData.constData()),
                 DefaultSize, DefaultSize, QImage::Format_ARGB32);
    return image.copy();
}

void Sprite::createBlankSprite() {
    Sprite::blankRGBSprite.resize(Sprite::RGBPixelsDataSize);
    Sprite::blankRGBSprite.fill(0x11);

    Sprite::blankARGBSprite.resize(Sprite::ARGBPixelsDataSize);
    Sprite::blankARGBSprite.fill(0x00);
}

// --- Item Implementation ---
Item::Item() {
    Type = OTB::ServerItemType::None;
    StackOrder = OTB::TileStackOrder::None;
    Movable = true;
    Name = QString();
    spriteHash = QByteArray(16, 0); // Initialize with 16 zero bytes
}

Item::Item(const Item& other) {
    *this = other;
}

Item& Item::operator=(const Item& other) {
    if (this != &other) {
        ID = other.ID;
        Type = other.Type;
        HasStackOrder = other.HasStackOrder;
        StackOrder = other.StackOrder;
        Unpassable = other.Unpassable;
        BlockMissiles = other.BlockMissiles;
        BlockPathfinder = other.BlockPathfinder;
        HasElevation = other.HasElevation;
        ForceUse = other.ForceUse;
        MultiUse = other.MultiUse;
        Pickupable = other.Pickupable;
        Movable = other.Movable;
        Stackable = other.Stackable;
        Readable = other.Readable;
        Rotatable = other.Rotatable;
        Hangable = other.Hangable;
        HookSouth = other.HookSouth;
        HookEast = other.HookEast;
        HasCharges = other.HasCharges;
        IgnoreLook = other.IgnoreLook;
        FullGround = other.FullGround;
        AllowDistanceRead = other.AllowDistanceRead;
        IsAnimation = other.IsAnimation;
        GroundSpeed = other.GroundSpeed;
        LightLevel = other.LightLevel;
        LightColor = other.LightColor;
        MaxReadChars = other.MaxReadChars;
        MaxReadWriteChars = other.MaxReadWriteChars;
        MinimapColor = other.MinimapColor;
        TradeAs = other.TradeAs;
        Name = other.Name;
        spriteHash = other.spriteHash;
    }
    return *this;
}

QByteArray Item::getSpriteHash() const {
    return spriteHash;
}

void Item::setSpriteHash(const QByteArray& value) {
    spriteHash = value;
}

bool Item::Equals(const Item& item) const {
    // Exact match of C# Item.Equals method
    if (Type != item.Type ||
        StackOrder != item.StackOrder ||
        Unpassable != item.Unpassable ||
        BlockMissiles != item.BlockMissiles ||
        BlockPathfinder != item.BlockPathfinder ||
        HasElevation != item.HasElevation ||
        ForceUse != item.ForceUse ||
        MultiUse != item.MultiUse ||
        Pickupable != item.Pickupable ||
        Movable != item.Movable ||
        Stackable != item.Stackable ||
        Readable != item.Readable ||
        Rotatable != item.Rotatable ||
        Hangable != item.Hangable ||
        HookSouth != item.HookSouth ||
        HookEast != item.HookEast ||
        IgnoreLook != item.IgnoreLook ||
        FullGround != item.FullGround ||
        IsAnimation != item.IsAnimation ||
        GroundSpeed != item.GroundSpeed ||
        LightLevel != item.LightLevel ||
        LightColor != item.LightColor ||
        MaxReadChars != item.MaxReadChars ||
        MaxReadWriteChars != item.MaxReadWriteChars ||
        MinimapColor != item.MinimapColor ||
        TradeAs != item.TradeAs) {
        return false;
    }

    if (Name.compare(item.Name) != 0) {
        return false;
    }

    return true;
}

bool Item::HasProperties(quint32 properties) const {
    // Exact match of C# Item.HasProperties method
    if (properties == 0) return false; // ServerItemFlag::None
    if ((properties & OTB::ServerItemFlag::Unpassable) && !Unpassable) return false;
    if ((properties & OTB::ServerItemFlag::BlockMissiles) && !BlockMissiles) return false;
    if ((properties & OTB::ServerItemFlag::BlockPathfinder) && !BlockPathfinder) return false;
    if ((properties & OTB::ServerItemFlag::HasElevation) && !HasElevation) return false;
    if ((properties & OTB::ServerItemFlag::ForceUse) && !ForceUse) return false;
    if ((properties & OTB::ServerItemFlag::MultiUse) && !MultiUse) return false;
    if ((properties & OTB::ServerItemFlag::Pickupable) && !Pickupable) return false;
    if ((properties & OTB::ServerItemFlag::Movable) && !Movable) return false;
    if ((properties & OTB::ServerItemFlag::Stackable) && !Stackable) return false;
    if ((properties & OTB::ServerItemFlag::Readable) && !Readable) return false;
    if ((properties & OTB::ServerItemFlag::Rotatable) && !Rotatable) return false;
    if ((properties & OTB::ServerItemFlag::Hangable) && !Hangable) return false;
    if ((properties & OTB::ServerItemFlag::HookSouth) && !HookSouth) return false;
    if ((properties & OTB::ServerItemFlag::HookEast) && !HookEast) return false;
    if ((properties & OTB::ServerItemFlag::AllowDistanceRead) && !AllowDistanceRead) return false;
    if ((properties & OTB::ServerItemFlag::IgnoreLook) && !IgnoreLook) return false;
    if ((properties & OTB::ServerItemFlag::FullGround) && !FullGround) return false;
    if ((properties & OTB::ServerItemFlag::IsAnimation) && !IsAnimation) return false;
    return true;
}

Item& Item::CopyPropertiesFrom(const Item& item) {
    // Simple property copying without using meta-object system for now
    // Skip SpriteHash like in C#
    ID = item.ID;
    Type = item.Type;
    HasStackOrder = item.HasStackOrder;
    StackOrder = item.StackOrder;
    Unpassable = item.Unpassable;
    BlockMissiles = item.BlockMissiles;
    BlockPathfinder = item.BlockPathfinder;
    HasElevation = item.HasElevation;
    ForceUse = item.ForceUse;
    MultiUse = item.MultiUse;
    Pickupable = item.Pickupable;
    Movable = item.Movable;
    Stackable = item.Stackable;
    Readable = item.Readable;
    Rotatable = item.Rotatable;
    Hangable = item.Hangable;
    HookSouth = item.HookSouth;
    HookEast = item.HookEast;
    HasCharges = item.HasCharges;
    IgnoreLook = item.IgnoreLook;
    FullGround = item.FullGround;
    AllowDistanceRead = item.AllowDistanceRead;
    IsAnimation = item.IsAnimation;
    GroundSpeed = item.GroundSpeed;
    LightLevel = item.LightLevel;
    LightColor = item.LightColor;
    MaxReadChars = item.MaxReadChars;
    MaxReadWriteChars = item.MaxReadWriteChars;
    MinimapColor = item.MinimapColor;
    TradeAs = item.TradeAs;
    Name = item.Name;

    return *this;
}

// --- ClientItem Implementation ---
QRect ClientItem::Rect;

ClientItem::ClientItem() : Item() {
    SpriteList.clear();
}

ClientItem::ClientItem(const ClientItem& other) : Item(other) {
    Width = other.Width;
    Height = other.Height;
    Layers = other.Layers;
    PatternX = other.PatternX;
    PatternY = other.PatternY;
    PatternZ = other.PatternZ;
    Frames = other.Frames;
    NumSprites = other.NumSprites;
    SpriteList = other.SpriteList;
    SpriteSignature = other.SpriteSignature;
}

ClientItem& ClientItem::operator=(const ClientItem& other) {
    if (this != &other) {
        Item::operator=(other);
        Width = other.Width;
        Height = other.Height;
        Layers = other.Layers;
        PatternX = other.PatternX;
        PatternY = other.PatternY;
        PatternZ = other.PatternZ;
        Frames = other.Frames;
        NumSprites = other.NumSprites;
        SpriteList = other.SpriteList;
        SpriteSignature = other.SpriteSignature;
    }
    return *this;
}

QByteArray ClientItem::getSpriteHash() const {
    // Match C# ClientItem.SpriteHash getter - calculate on demand if null
    if (spriteHash.isEmpty() || spriteHash == QByteArray(16, 0)) {
        QCryptographicHash md5(QCryptographicHash::Md5);
        int spriteBase = 0;
        QByteArray rgbaData(Sprite::ARGBPixelsDataSize, 0);

        for (quint8 l = 0; l < Layers; l++) {
            for (quint8 h = 0; h < Height; h++) {
                for (quint8 w = 0; w < Width; w++) {
                    int index = spriteBase + w + h * Width + l * Width * Height;
                    if (index < SpriteList.size()) {
                        const Sprite& sprite = SpriteList[index];
                        QByteArray rgbData = sprite.getRGBData();

                        // Reverse RGB like in C# implementation
                        for (int y = 0; y < Sprite::DefaultSize; ++y) {
                            for (int x = 0; x < Sprite::DefaultSize; ++x) {
                                int srcIdx = (32 - y - 1) * 96 + x * 3;
                                int dstIdx = 128 * y + x * 4;
                                if (srcIdx + 2 < rgbData.size() && dstIdx + 3 < rgbaData.size()) {
                                    rgbaData[dstIdx + 0] = rgbData[srcIdx + 2]; // blue
                                    rgbaData[dstIdx + 1] = rgbData[srcIdx + 1]; // green
                                    rgbaData[dstIdx + 2] = rgbData[srcIdx + 0]; // red
                                    rgbaData[dstIdx + 3] = 0;
                                }
                            }
                        }
                    }
                    md5.addData(rgbaData);
                }
            }
        }

        // Cast away const to modify the mutable cache
        const_cast<ClientItem*>(this)->spriteHash = md5.result();
    }

    return spriteHash;
}

void ClientItem::setSpriteHash(const QByteArray& value) {
    spriteHash = value;
}

QImage ClientItem::GetBitmap() const {
    // Match C# ClientItem.GetBitmap method exactly
    if (SpriteList.isEmpty() || Width == 0 || Height == 0 || Layers == 0) {
        return QImage(Sprite::DefaultSize, Sprite::DefaultSize, QImage::Format_ARGB32);
    }

    QImage bitmap(Width * Sprite::DefaultSize, Height * Sprite::DefaultSize, QImage::Format_ARGB32);
    bitmap.fill(Qt::transparent);

    QPainter painter(&bitmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    try {
        for (quint8 l = 0; l < Layers; l++) {
            for (quint8 w = 0; w < Width; w++) {
                for (quint8 h = 0; h < Height; h++) {
                    int index = w + h * Width + l * Width * Height;
                    if (index < SpriteList.size()) {
                        int px = (Width - w - 1) * Sprite::DefaultSize;
                        int py = (Height - h - 1) * Sprite::DefaultSize;

                        QImage spriteImage = SpriteList[index].getBitmap();
                        painter.drawImage(px, py, spriteImage);
                    }
                }
            }
        }

        painter.end();

        // Make transparent color like in C# (Color.FromArgb(0x11, 0x11, 0x11))
        // This is handled by the sprite decompression, but we can ensure it here
        return bitmap;
    } catch (...) {
        qDebug() << QString("Failed to get image for client id %1. Check the transparency option.").arg(ID);
        return QImage();
    }
}

void ClientItem::GenerateSignature() {
    // Match C# ClientItem.GenerateSignature method
    int width = Sprite::DefaultSize;
    int height = Sprite::DefaultSize;

    if (Width > 1 || Height > 1) {
        width = Sprite::DefaultSize * 2;
        height = Sprite::DefaultSize * 2;
    }

    QImage canvas(width, height, QImage::Format_RGB888);
    QPainter g(&canvas);

    // Draw sprite like in C#
    for (int l = 0; l < Layers; l++) {
        for (int h = 0; h < Height; ++h) {
            for (int w = 0; w < Width; ++w) {
                int index = w + h * Width + l * Width * Height;
                if (index < SpriteList.size()) {
                    QByteArray rgbData = SpriteList[index].getRGBData();
                    
                    // Create QImage from RGB data
                    if (rgbData.size() >= Sprite::RGBPixelsDataSize) {
                        QImage bitmap(reinterpret_cast<const uchar*>(rgbData.constData()), 
                                    Sprite::DefaultSize, Sprite::DefaultSize, QImage::Format_RGB888);

                        if (canvas.width() == Sprite::DefaultSize) {
                            Rect.setX(0);
                            Rect.setY(0);
                            Rect.setWidth(bitmap.width());
                            Rect.setHeight(bitmap.height());
                        } else {
                            Rect.setX(qMax(Sprite::DefaultSize - w * Sprite::DefaultSize, 0));
                            Rect.setY(qMax(Sprite::DefaultSize - h * Sprite::DefaultSize, 0));
                            Rect.setWidth(bitmap.width());
                            Rect.setHeight(bitmap.height());
                        }

                        g.drawImage(Rect, bitmap);
                    }
                }
            }
        }
    }

    g.end();

    // Use FFT and calculate Euclidean distance like in C#
    // This requires the ImageSimilarity implementation
    // For now, just initialize an empty signature
    SpriteSignature.clear();
    
    // TODO: Implement when ImageSimilarity::Fourier is available
    // QImage ff2dBmp = TibiaData::ImageSimilarity::Fourier::fft2dRGB(canvas, false);
    // SpriteSignature = TibiaData::ImageSimilarity::Utils::CalculateEuclideanDistance(ff2dBmp, 1);
}

} // namespace ItemEditor
