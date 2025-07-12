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
    if (compressedPixels.isNull() || compressedPixels.isEmpty() || size == 0) {
        return Sprite::blankARGBSprite; // Return a copy
    }

    QByteArray pixels(ARGBPixelsDataSize, 0x00); // Initialize with transparent black
    const quint8* rleData = reinterpret_cast<const quint8*>(compressedPixels.constData());
    int readPos = 0;
    int writePos = 0;
    quint8 bytesPerPixel = transparent ? 4 : 3;

    try {
        while (readPos < size && writePos < ARGBPixelsDataSize) {
            if (readPos + 1 >= size) break; // Not enough data for transparent/colored counts
            quint16 transparentPixels = static_cast<quint16>(rleData[readPos]) | (static_cast<quint16>(rleData[readPos + 1]) << 8);
            readPos += 2;

            // Write transparent pixels (alpha = 0)
            for (int i = 0; i < transparentPixels && writePos < ARGBPixelsDataSize; ++i) {
                pixels[writePos++] = 0x00; // Blue
                pixels[writePos++] = 0x00; // Green
                pixels[writePos++] = 0x00; // Red
                pixels[writePos++] = 0x00; // Alpha
            }
            if (writePos >= ARGBPixelsDataSize && readPos < size) break; // Output buffer full

            if (readPos + 1 >= size) break; // Not enough data for colored pixel count
            quint16 coloredPixels = static_cast<quint16>(rleData[readPos]) | (static_cast<quint16>(rleData[readPos + 1]) << 8);
            readPos += 2;

            for (int i = 0; i < coloredPixels && writePos < ARGBPixelsDataSize; ++i) {
                if (readPos + bytesPerPixel > size) { // Check if enough data for a full pixel
                     qWarning() << "SPR Decompression: Unexpected end of RLE data for sprite ID" << id
                               << "while reading colored pixel" << i+1 << "/" << coloredPixels
                               << ". ReadPos:" << readPos << "Size:" << size << "BytesPerPixel:" << bytesPerPixel;
                    // Fill remaining with transparent and return what we have
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
             if (writePos >= ARGBPixelsDataSize && readPos < size) break; // Output buffer full
        }
    } catch (const std::out_of_range& oor) {
        qWarning() << "SPR Decompression: Out of range error for sprite ID" << id << ":" << oor.what();
        return Sprite::blankARGBSprite; // Return blank on error
    } catch (...) {
        qWarning() << "SPR Decompression: Unknown error for sprite ID" << id;
        return Sprite::blankARGBSprite;
    }

    // If RLE data didn't fill the whole 32x32 area, the rest of 'pixels' is already transparent black.
    return pixels;
}

QImage Sprite::getBitmap() const {
    QByteArray argbData = getPixelsARGB();
    if (argbData.size() != ARGBPixelsDataSize) { // Should be blankARGBSprite or full size
        return QImage(DefaultSize, DefaultSize, QImage::Format_ARGB32); // Return blank image
    }

    // QImage expects ARGB format where data is (A,R,G,B) in memory for MSB systems,
    // or (B,G,R,A) for LSB systems if Format_ARGB32_Premultiplied or Format_ARGB32 is used.
    // Qt handles this internally. Our getPixelsARGB produces B,G,R,A.
    QImage image(reinterpret_cast<const uchar*>(argbData.constData()),
                 DefaultSize, DefaultSize, QImage::Format_ARGB32);

    // IMPORTANT: QImage created from existing data does not own the data.
    // We need to return a copy if argbData is temporary.
    // Since getPixelsARGB() returns by value (a QByteArray copy), argbData here is a local copy.
    // So, the QImage(data*, w, h, fmt) constructor will make a deep copy of the data if the data
    // is not const, or if we then call copy(). To be safe, explicitly copy.
    return image.copy();
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
    if (spriteList.isEmpty() || width == 0 || height == 0 || layers == 0) {
        QImage dummyImage(Sprite::DefaultSize, Sprite::DefaultSize, QImage::Format_ARGB32);
        dummyImage.fill(Qt::darkGray); // Default placeholder for empty/invalid item
        return dummyImage;
    }

    // Create a canvas for the full item image (width * 32, height * 32)
    // For now, we only render the first animation frame.
    QImage canvasImage(width * Sprite::DefaultSize,
                       height * Sprite::DefaultSize,
                       QImage::Format_ARGB32);
    canvasImage.fill(Qt::transparent); // Start with a transparent canvas

    QPainter painter(&canvasImage);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver); // Alpha blending

    // Iterate through layers, then height (y), then width (x) for one animation frame.
    // The C# ItemEditor.ClientItem.GetBitmap draws with origin at bottom-left for item parts,
    // then makes a specific color transparent.
    // Qt's QPainter draws with origin top-left.
    // SpriteList in C# is ordered: (all sprites for frame 0 layer 0), (all for frame 0 layer 1), ...
    // then (all for frame 1 layer 0), etc.
    // Sprite index: w + h * item.Width + l * item.Width * item.Height (for a single animation frame)

    int currentSpriteIndex = 0; // Assuming sprites for first frame are contiguous at the start of spriteList

    for (quint8 l = 0; l < layers; ++l) {
        for (quint8 h_idx = 0; h_idx < height; ++h_idx) { // y-coordinate in item grid
            for (quint8 w_idx = 0; w_idx < width; ++w_idx) { // x-coordinate in item grid
                if (currentSpriteIndex >= spriteList.size()) {
                    qWarning() << "ClientItem ID" << id << ": Not enough sprites in spriteList for its dimensions/layers. Expected at least"
                               << (l * width * height + h_idx * width + w_idx + 1) << "found" << spriteList.size();
                    painter.end();
                    return canvasImage.copy(); // Return what's drawn so far
                }

                const OTB::Sprite& sprite = spriteList.at(currentSpriteIndex);
                QImage spriteImage = sprite.getBitmap(); // This now returns a decompressed 32x32 QImage

                if (!spriteImage.isNull()) {
                    // C# rendering logic for item parts: px = (this.Width - w - 1) * Sprite.DefaultSize;
                    // py = (this.Height - h - 1) * Sprite.DefaultSize;
                    // This means sprite (0,0) of item is at bottom-left of canvas.
                    // Qt draws top-left. Let's adapt.
                    int drawX = w_idx * Sprite::DefaultSize;
                    int drawY = h_idx * Sprite::DefaultSize;

                    // The C# code for ClientItem.GetBitmap in Item.cs seems to draw sprites
                    // in a way that the final image is built up correctly for display.
                    // It uses `locker.CopyPixels(this.SpriteList[index].GetBitmap(), px, py);`
                    // where px, py are calculated to place parts correctly, then MakeTransparent.
                    // Let's try direct compositing with QPainter.
                    painter.drawImage(drawX, drawY, spriteImage);
                }
                currentSpriteIndex++;
            }
        }
    }
    painter.end();

    // C# code does bitmap.MakeTransparent(Color.FromArgb(0x11, 0x11, 0x11));
    // Our OTB::Sprite::getBitmap() should ideally return an ARGB image where transparency
    // is already handled by the alpha channel from RLE decompression.
    // If not, we might need to do a similar pass here on canvasImage.
    // Assuming OTB::Sprite::getBitmap() produces images with correct alpha.

    return canvasImage.copy(); // Return a copy
}

#include "tibiadata/imagesimilarity.h" // For ImageSimilarity functions

void ClientItem::generateSignature() {
    // This method should use the actual sprite data to generate a signature.
    // 1. Get the composite bitmap for the item (e.g., first animation frame, all layers).
    //    The C# version creates a 32x32 or 64x64 canvas, draws sprites, then FFTs that.
    QImage itemAppearanceImage; // This should be the representative image of the item.
                                // For now, let's use its own getBitmap() which gets the first frame.
                                // C# ClientItem.GenerateSignature draws the item onto a new canvas,
                                // potentially scaling it if it's > 1x1 sprite dimensions.
                                // For simplicity, let's use the 32x32 (or WxH) output of getBitmap().

    if (this->width == 1 && this->height == 1) { // Simple 1x1 item
        itemAppearanceImage = getBitmap(); // Gets the 32x32 image
    } else {
        // For larger items, C# ItemEditor.Item.ClientItem.GenerateSignature uses a canvas of
        // 32x32 or 64x64 based on item dimensions, then draws the item onto it.
        // Let's create a 64x64 canvas for items > 1x1 for now, and draw the item centered or scaled.
        // This part needs to align with how C# prepares the image for FFT.
        // For now, just use the (potentially W*32 x H*32) image from getBitmap().
        // The FFT and signature logic should handle varying image sizes if designed generally,
        // or images should be normalized to a fixed size before FFT.
        // C# Fourier.fft2dRGB takes the canvas, not the direct item bitmap if it's multi-sprite.
        // Let's assume for now getBitmap() gives us the relevant "visual" to fingerprint.
        itemAppearanceImage = getBitmap();
    }


    if (itemAppearanceImage.isNull()) {
        qWarning() << "ClientItem::generateSignature: Cannot generate signature, item bitmap is null for ID:" << id;
        spriteSignature.clear();
        return;
    }

    // 2. Perform FFT on the image (e.g., on its RGB channels or grayscale version).
    //    C# uses Fourier.fft2dRGB(canvas, false) which returns a Bitmap of FFT magnitudes.
    QImage fftMagnitudeImage = TibiaData::ImageSimilarity::Fourier::FFT2D_RGB(itemAppearanceImage, false);
    if (fftMagnitudeImage.isNull()) {
        qWarning() << "ClientItem::generateSignature: FFT magnitude image is null for ID:" << id;
        spriteSignature.clear();
        return;
    }

    // 3. Calculate the Euclidean distance signature from the FFT magnitude image.
    //    C# ImageUtils.CalculateEuclideanDistance(ff2dBmp, 1) with regions=4 (implicitly by block size calc).
    spriteSignature = TibiaData::ImageSimilarity::Utils::CalculateEuclideanDistanceSignature(fftMagnitudeImage, 4); // 4x4 regions

    qDebug() << "ClientItem::generateSignature called for ID:" << id << "Signature isEmpty:" << spriteSignature.isEmpty();
}

// --- ItemBase ---
// bool ItemBase::equals(const ItemBase& other) const { /* TODO */ return false; }
// void ItemBase::copyPropertiesFrom(const ItemBase& source) { /* TODO */ }


} // namespace OTB
