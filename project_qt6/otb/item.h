#ifndef ITEM_H
#define ITEM_H

#include "otbtypes.h" // For ServerItemType, TileStackOrder etc.
#include <QString>
#include <QByteArray>
#include <QList>
#include <QImage> // For QImage/Bitmap equivalent
#include <QtGlobal>
#include <QVariantMap> // For SpriteSignature (double[,] -> QVariantMap of QVariantMaps or similar)

// Forward declaration for ImageSimilarity if Complex is defined elsewhere
// namespace ImageSimilarity { class Complex; }


namespace OTB {

// Corresponds to C# PluginInterface.Sprite
struct Sprite {
    static const quint8 DefaultSize = 32;
    static const quint16 RGBPixelsDataSize = 3072; // 32*32*3
    static const quint16 ARGBPixelsDataSize = 4096; // 32*32*4

    quint32 id = 0; // Sprite ID from .spr file
    quint32 size = 0; // Size of compressedPixels
    QByteArray compressedPixels;
    bool transparent = false;

    Sprite() = default;

    // Methods to be implemented later, matching C# Sprite.cs
    QByteArray getRGBData(quint8 transparentRgbColor = 0x11) const;
    QByteArray getPixelsARGB() const; // Returns ARGB
    QImage getBitmap() const;

    static QByteArray blankRGBSprite;
    static QByteArray blankARGBSprite;
    // static void createBlankSprite(); // Call once at app startup
};


// Base properties common to ServerItem and ClientItem, from C# Item.cs
// ServerItem in otbtypes.h already covers most of these.
// ClientItem will embed or inherit these.
struct ItemBase {
    quint16 id = 0; // Server ID for ServerItem, Client ID for ClientItem
    QString name;
    ServerItemType type = ServerItemType::None;
    TileStackOrder stackOrder = TileStackOrder::None;
    bool hasStackOrder = false; // Explicitly set if StackOrder attribute is present

    // Flags
    bool unpassable = false;
    bool blockMissiles = false;
    bool blockPathfinder = false;
    bool hasElevation = false;
    bool forceUse = false;
    bool multiUse = false;
    bool pickupable = false;
    bool movable = true; // Default true in C#
    bool stackable = false;
    bool readable = false;
    bool rotatable = false;
    bool hangable = false;
    bool hookSouth = false;
    bool hookEast = false;
    bool hasCharges = false; // Corresponds to ClientCharges flag in OTB
    bool ignoreLook = false;
    bool fullGround = false;
    bool allowDistanceRead = false;
    bool isAnimation = false; // From OTB flags or client DAT

    // Attributes
    quint16 groundSpeed = 0;
    quint16 lightLevel = 0;
    quint16 lightColor = 0;
    quint16 maxReadChars = 0;
    quint16 maxReadWriteChars = 0;
    quint16 minimapColor = 0;
    quint16 tradeAs = 0; // Ware ID

    QByteArray spriteHash; // 16 bytes. For ClientItem, this is calculated.

    ItemBase() : spriteHash(16,0) {}

    // virtual bool equals(const ItemBase& other) const; // For comparing properties
    // virtual void copyPropertiesFrom(const ItemBase& source);
};


// Corresponds to C# PluginInterface.ClientItem
struct ClientItem : public ItemBase {
    // Dimensions and animation properties from .dat file
    quint8 width = 1;
    quint8 height = 1;
    // quint8 exactSize = 0; // C# doesn't have this, seems like a leftover from my previous attempt
    quint8 layers = 0;
    quint8 patternX = 0;
    quint8 patternY = 0;
    quint8 patternZ = 0;
    quint8 frames = 0; // Number of animation frames
    quint32 numSprites = 0; // Total sprites making up this item (width*height*layers*frames)

    QList<Sprite> spriteList; // List of actual sprite data

    // SpriteSignature for image similarity (e.g., map of [x][y] to double)
    // C# uses double[,] which is a 2D array. Qt doesn't have a direct QArray2D.
    // A QList<QList<double>> or a QMap<int, QMap<int, double>> could work.
    // Or a flat QList with known dimensions.
    // For now, let's use QVariantMap representing a 2D structure.
    QVariantMap spriteSignature; // e.g. {"row_0": QVariantMap{{"col_0": 0.1}, ...}, ...}

    ClientItem() : ItemBase() {}

    // Override spriteHash calculation if needed (C# ClientItem calculates it on demand)
    const QByteArray& getSpriteHash(); // Calculates if not already set

    // Methods to be implemented later, matching C# ClientItem.cs
    QImage getBitmap() const;
    void generateSignature(); // Calculates SpriteSignature
};


// Corresponds to C# PluginInterface.SupportedClient
struct SupportedClient {
    quint32 version = 0;        // Numeric client version, e.g., 1098 for 10.98
    QString description;      // User-friendly string, e.g., "Tibia Client 10.98"
    quint32 otbVersion = 0;     // Corresponding OTB version, e.g., 770
    quint32 datSignature = 0;   // Expected signature of the .dat file
    quint32 sprSignature = 0;   // Expected signature of the .spr file

    // Store paths when a client is successfully loaded by a plugin instance
    QString clientDirectoryPath; // Path to the client files
    QString datPath;
    QString sprPath;

    SupportedClient() = default;
    SupportedClient(quint32 ver, const QString& desc, quint32 otbVer, quint32 datSig, quint32 sprSig)
        : version(ver), description(desc), otbVersion(otbVer), datSignature(datSig), sprSignature(sprSig) {}

    QString toString() const { return description; }
};


} // namespace OTB

#endif // ITEM_H
