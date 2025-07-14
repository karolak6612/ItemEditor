/**
 * Item Editor Qt6 - Item Classes Header
 * Exact mirror of Legacy_App/csharp/Source/PluginInterface/Item.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_ITEM_H
#define ITEMEDITOR_ITEM_H

#include <QObject>
#include <QString>
#include <QPixmap>
#include <QList>
#include <QByteArray>

namespace ItemEditor {

// Forward declarations
class Sprite;

/**
 * ServerItemType enumeration
 * Exact mirror of C# ServerItemType enum
 */
enum class ServerItemType : quint8
{
    None = 0,
    Ground = 1,
    Container = 2,
    Weapon = 3,
    Ammunition = 4,
    Armor = 5,
    Charges = 6,
    Teleport = 7,
    MagicField = 8,
    Writable = 9,
    Key = 10,
    Splash = 11,
    Fluid = 12,
    Door = 13,
    Deprecated = 14
};

/**
 * TileStackOrder enumeration
 * Exact mirror of C# TileStackOrder enum
 */
enum class TileStackOrder : quint8
{
    None = 0,
    Ground = 1,
    Border = 2,
    Bottom = 3,
    Top = 4
};

/**
 * Item base class
 * Exact mirror of C# Item class
 * 
 * Base class for all item types with common properties
 */
class Item : public QObject
{
    Q_OBJECT

public:
    explicit Item(QObject *parent = nullptr);
    virtual ~Item() = default;

    // Properties - exact mirror of C# Item properties
    quint16 id() const { return m_id; }
    void setId(quint16 id) { m_id = id; }
    
    ServerItemType type() const { return m_type; }
    void setType(ServerItemType type) { m_type = type; }
    
    bool hasStackOrder() const { return m_hasStackOrder; }
    void setHasStackOrder(bool hasStackOrder) { m_hasStackOrder = hasStackOrder; }
    
    TileStackOrder stackOrder() const { return m_stackOrder; }
    void setStackOrder(TileStackOrder stackOrder) { m_stackOrder = stackOrder; }
    
    bool unpassable() const { return m_unpassable; }
    void setUnpassable(bool unpassable) { m_unpassable = unpassable; }
    
    bool blockMissiles() const { return m_blockMissiles; }
    void setBlockMissiles(bool blockMissiles) { m_blockMissiles = blockMissiles; }
    
    bool blockPathfinder() const { return m_blockPathfinder; }
    void setBlockPathfinder(bool blockPathfinder) { m_blockPathfinder = blockPathfinder; }
    
    bool hasElevation() const { return m_hasElevation; }
    void setHasElevation(bool hasElevation) { m_hasElevation = hasElevation; }
    
    bool forceUse() const { return m_forceUse; }
    void setForceUse(bool forceUse) { m_forceUse = forceUse; }
    
    bool multiUse() const { return m_multiUse; }
    void setMultiUse(bool multiUse) { m_multiUse = multiUse; }
    
    bool pickupable() const { return m_pickupable; }
    void setPickupable(bool pickupable) { m_pickupable = pickupable; }
    
    bool movable() const { return m_movable; }
    void setMovable(bool movable) { m_movable = movable; }
    
    bool stackable() const { return m_stackable; }
    void setStackable(bool stackable) { m_stackable = stackable; }
    
    bool readable() const { return m_readable; }
    void setReadable(bool readable) { m_readable = readable; }
    
    bool rotatable() const { return m_rotatable; }
    void setRotatable(bool rotatable) { m_rotatable = rotatable; }
    
    bool hangable() const { return m_hangable; }
    void setHangable(bool hangable) { m_hangable = hangable; }
    
    bool hookSouth() const { return m_hookSouth; }
    void setHookSouth(bool hookSouth) { m_hookSouth = hookSouth; }
    
    bool hookEast() const { return m_hookEast; }
    void setHookEast(bool hookEast) { m_hookEast = hookEast; }
    
    bool hasCharges() const { return m_hasCharges; }
    void setHasCharges(bool hasCharges) { m_hasCharges = hasCharges; }
    
    bool ignoreLook() const { return m_ignoreLook; }
    void setIgnoreLook(bool ignoreLook) { m_ignoreLook = ignoreLook; }
    
    bool fullGround() const { return m_fullGround; }
    void setFullGround(bool fullGround) { m_fullGround = fullGround; }
    
    bool allowDistanceRead() const { return m_allowDistanceRead; }
    void setAllowDistanceRead(bool allowDistanceRead) { m_allowDistanceRead = allowDistanceRead; }
    
    bool isAnimation() const { return m_isAnimation; }
    void setIsAnimation(bool isAnimation) { m_isAnimation = isAnimation; }
    
    quint16 groundSpeed() const { return m_groundSpeed; }
    void setGroundSpeed(quint16 groundSpeed) { m_groundSpeed = groundSpeed; }
    
    quint16 lightLevel() const { return m_lightLevel; }
    void setLightLevel(quint16 lightLevel) { m_lightLevel = lightLevel; }
    
    quint16 lightColor() const { return m_lightColor; }
    void setLightColor(quint16 lightColor) { m_lightColor = lightColor; }
    
    quint16 maxReadChars() const { return m_maxReadChars; }
    void setMaxReadChars(quint16 maxReadChars) { m_maxReadChars = maxReadChars; }
    
    quint16 maxReadWriteChars() const { return m_maxReadWriteChars; }
    void setMaxReadWriteChars(quint16 maxReadWriteChars) { m_maxReadWriteChars = maxReadWriteChars; }
    
    quint16 minimapColor() const { return m_minimapColor; }
    void setMinimapColor(quint16 minimapColor) { m_minimapColor = minimapColor; }
    
    quint16 tradeAs() const { return m_tradeAs; }
    void setTradeAs(quint16 tradeAs) { m_tradeAs = tradeAs; }
    
    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }
    
    // Virtual sprite hash property
    virtual QByteArray spriteHash() const { return m_spriteHash; }
    virtual void setSpriteHash(const QByteArray& spriteHash) { m_spriteHash = spriteHash; }

    // Methods
    bool equals(const Item& item) const;
    QString toString() const;

signals:
    void itemChanged();

protected:
    QByteArray m_spriteHash;

private:
    quint16 m_id;
    ServerItemType m_type;
    bool m_hasStackOrder;
    TileStackOrder m_stackOrder;
    bool m_unpassable;
    bool m_blockMissiles;
    bool m_blockPathfinder;
    bool m_hasElevation;
    bool m_forceUse;
    bool m_multiUse;
    bool m_pickupable;
    bool m_movable;
    bool m_stackable;
    bool m_readable;
    bool m_rotatable;
    bool m_hangable;
    bool m_hookSouth;
    bool m_hookEast;
    bool m_hasCharges;
    bool m_ignoreLook;
    bool m_fullGround;
    bool m_allowDistanceRead;
    bool m_isAnimation;
    quint16 m_groundSpeed;
    quint16 m_lightLevel;
    quint16 m_lightColor;
    quint16 m_maxReadChars;
    quint16 m_maxReadWriteChars;
    quint16 m_minimapColor;
    quint16 m_tradeAs;
    QString m_name;
};

/**
 * ClientItem class
 * Exact mirror of C# ClientItem class
 * 
 * Represents a client-side item with sprite data
 */
class ClientItem : public Item
{
    Q_OBJECT

public:
    explicit ClientItem(QObject *parent = nullptr);
    virtual ~ClientItem() = default;

    // Additional ClientItem properties - exact mirror of C# ClientItem properties
    quint8 width() const { return m_width; }
    void setWidth(quint8 width) { m_width = width; }
    
    quint8 height() const { return m_height; }
    void setHeight(quint8 height) { m_height = height; }
    
    quint8 layers() const { return m_layers; }
    void setLayers(quint8 layers) { m_layers = layers; }
    
    quint8 patternX() const { return m_patternX; }
    void setPatternX(quint8 patternX) { m_patternX = patternX; }
    
    quint8 patternY() const { return m_patternY; }
    void setPatternY(quint8 patternY) { m_patternY = patternY; }
    
    quint8 patternZ() const { return m_patternZ; }
    void setPatternZ(quint8 patternZ) { m_patternZ = patternZ; }
    
    quint8 frames() const { return m_frames; }
    void setFrames(quint8 frames) { m_frames = frames; }
    
    quint32 numSprites() const { return m_numSprites; }
    void setNumSprites(quint32 numSprites) { m_numSprites = numSprites; }
    
    QList<Sprite*> spriteList() const { return m_spriteList; }
    void setSpriteList(const QList<Sprite*>& spriteList) { m_spriteList = spriteList; }
    void addSprite(Sprite* sprite) { m_spriteList.append(sprite); }
    void clearSprites() { m_spriteList.clear(); }

    // Methods - exact mirror of C# ClientItem methods
    QPixmap getBitmap() const;
    void setBitmap(const QPixmap& bitmap);
    void generateBitmap(); // Generate bitmap from sprite list
    
    bool isValid() const { return !m_bitmap.isNull(); }
    
    // Override sprite hash calculation
    QByteArray spriteHash() const override;
    void setSpriteHash(const QByteArray& spriteHash) override;

private:
    quint8 m_width;
    quint8 m_height;
    quint8 m_layers;
    quint8 m_patternX;
    quint8 m_patternY;
    quint8 m_patternZ;
    quint8 m_frames;
    quint32 m_numSprites;
    QList<Sprite*> m_spriteList;
    QPixmap m_bitmap;
};

/**
 * ServerItem class
 * Exact mirror of C# ServerItem class
 * 
 * Represents a server-side item with properties and attributes
 */
class ServerItem : public Item
{
    Q_OBJECT

public:
    explicit ServerItem(QObject *parent = nullptr);
    virtual ~ServerItem() = default;

    // Additional ServerItem properties
    quint16 clientId() const { return m_clientId; }
    void setClientId(quint16 clientId) { m_clientId = clientId; }
    
    QString description() const { return m_description; }
    void setDescription(const QString& description) { m_description = description; }

private:
    quint16 m_clientId;
    QString m_description;
};

} // namespace ItemEditor

// Forward declaration for Qt MOC
class ClientItem;
// ServerItem forward declaration removed to avoid conflict with using alias

#endif // ITEMEDITOR_ITEM_H