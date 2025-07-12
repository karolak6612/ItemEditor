#ifndef ITEM_H
#define ITEM_H

#include <QString>
#include <QByteArray>
#include <QList>
#include <QImage>
#include <QtGlobal>
#include <QVariantMap>
#include <QMetaType>

// Forward declarations to avoid circular dependencies
namespace OTB {
    enum class ServerItemType : quint8;
    enum class TileStackOrder : quint8;
    namespace ServerItemFlag {
        extern const quint32 Unpassable;
        extern const quint32 BlockMissiles;
        extern const quint32 BlockPathfinder;
        extern const quint32 HasElevation;
        extern const quint32 ForceUse;
        extern const quint32 MultiUse;
        extern const quint32 Pickupable;
        extern const quint32 Movable;
        extern const quint32 Stackable;
        extern const quint32 Readable;
        extern const quint32 Rotatable;
        extern const quint32 Hangable;
        extern const quint32 HookSouth;
        extern const quint32 HookEast;
        extern const quint32 AllowDistanceRead;
        extern const quint32 IgnoreLook;
        extern const quint32 FullGround;
        extern const quint32 IsAnimation;
    }
}

namespace ItemEditor {

// Corresponds to C# PluginInterface.Sprite
class Sprite {
public:
    static const quint8 DefaultSize = 32;
    static const quint16 RGBPixelsDataSize = 3072; // 32*32*3
    static const quint16 ARGBPixelsDataSize = 4096; // 32*32*4

    quint32 id = 0; // Sprite ID from .spr file
    quint32 size = 0; // Size of compressedPixels
    QByteArray compressedPixels;
    bool transparent = false;

    Sprite() = default;

    // Methods matching C# Sprite.cs
    QByteArray getRGBData(quint8 transparentRgbColor = 0x11) const;
    QByteArray getPixelsARGB() const; // Returns ARGB
    QImage getBitmap() const;

    static QByteArray blankRGBSprite;
    static QByteArray blankARGBSprite;
    static void createBlankSprite(); // Call once at app startup
};

// Base Item class matching C# Item.cs exactly
class Item {
protected:
    QByteArray spriteHash; // Protected member like in C#

public:
    // Constructor matching C# Item constructor
    Item();
    virtual ~Item() = default;

    // Copy constructor and assignment operator
    Item(const Item& other);
    Item& operator=(const Item& other);

    // Public Properties matching C# Item.cs exactly
    quint16 ID = 0;
    OTB::ServerItemType Type;
    bool HasStackOrder = false;
    OTB::TileStackOrder StackOrder;
    bool Unpassable = false;
    bool BlockMissiles = false;
    bool BlockPathfinder = false;
    bool HasElevation = false;
    bool ForceUse = false;
    bool MultiUse = false;
    bool Pickupable = false;
    bool Movable = true; // Default true in C#
    bool Stackable = false;
    bool Readable = false;
    bool Rotatable = false;
    bool Hangable = false;
    bool HookSouth = false;
    bool HookEast = false;
    bool HasCharges = false;
    bool IgnoreLook = false;
    bool FullGround = false;
    bool AllowDistanceRead = false;
    bool IsAnimation = false;
    quint16 GroundSpeed = 0;
    quint16 LightLevel = 0;
    quint16 LightColor = 0;
    quint16 MaxReadChars = 0;
    quint16 MaxReadWriteChars = 0;
    quint16 MinimapColor = 0;
    quint16 TradeAs = 0;
    QString Name;

    // Virtual SpriteHash property matching C# implementation
    virtual QByteArray getSpriteHash() const;
    virtual void setSpriteHash(const QByteArray& value);

    // Public Methods matching C# Item.cs exactly
    bool Equals(const Item& item) const;
    bool HasProperties(quint32 properties) const;
    Item& CopyPropertiesFrom(const Item& item);
};

// ClientItem class matching C# ClientItem exactly
class ClientItem : public Item {
private:
    static QRect Rect; // Static rectangle for drawing operations

public:
    // Constructor matching C# ClientItem constructor
    ClientItem();

    // Copy constructor and assignment operator
    ClientItem(const ClientItem& other);
    ClientItem& operator=(const ClientItem& other);

    // Public Properties matching C# ClientItem.cs exactly
    quint8 Width = 1;
    quint8 Height = 1;
    quint8 Layers = 0;
    quint8 PatternX = 0;
    quint8 PatternY = 0;
    quint8 PatternZ = 0;
    quint8 Frames = 0;
    quint32 NumSprites = 0;
    QList<Sprite> SpriteList;

    // SpriteSignature for image similarity - using QVariantMap to represent double[,]
    QVariantMap SpriteSignature;

    // Override SpriteHash calculation like in C# ClientItem
    QByteArray getSpriteHash() const override;
    void setSpriteHash(const QByteArray& value) override;

    // Public Methods matching C# ClientItem.cs exactly
    QImage GetBitmap() const;
    void GenerateSignature();
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

} // namespace ItemEditor

// Register types for Qt's meta-object system
Q_DECLARE_METATYPE(ItemEditor::Item)
Q_DECLARE_METATYPE(ItemEditor::ClientItem)
Q_DECLARE_METATYPE(ItemEditor::Sprite)

#endif // ITEM_H
