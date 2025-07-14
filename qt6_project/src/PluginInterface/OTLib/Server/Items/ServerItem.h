/**
 * Item Editor Qt6 - Server Item Header
 * Complete implementation based on Legacy_App/csharp/Source/PluginInterface/OTLib/Server/Items/ServerItem.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef OTLIB_SERVER_ITEMS_SERVERITEM_H
#define OTLIB_SERVER_ITEMS_SERVERITEM_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include "ServerItemFlag.h"

namespace OTLib {
namespace Server {
namespace Items {

/**
 * Server Item Group enumeration
 * Exact mirror of C# ServerItemGroup enum
 */
enum class ServerItemGroup : quint8
{
    None        = 0,
    Ground      = 1,
    Container   = 2,
    Weapon      = 3,
    Ammunition  = 4,
    Armor       = 5,
    Changes     = 6,
    Teleport    = 7,
    MagicField  = 8,
    Writable    = 9,
    Key         = 10,
    Splash      = 11,
    Fluid       = 12,
    Door        = 13,
    Deprecated  = 14
};

/**
 * Server Item Type enumeration
 * Exact mirror of C# ServerItemType enum
 */
enum class ServerItemType : quint8
{
    None        = 0,
    Ground      = 1,
    Container   = 2,
    Fluid       = 3,
    Splash      = 4,
    Deprecated  = 5
};

/**
 * Tile Stack Order enumeration
 * Exact mirror of C# TileStackOrder enum
 */
enum class TileStackOrder : quint8
{
    None    = 0,
    Border  = 1,
    Bottom  = 2,
    Top     = 3
};

/**
 * Base Item class
 * Exact mirror of C# Item class with all properties and methods
 */
class Item : public QObject
{
    Q_OBJECT
    Q_PROPERTY(quint16 id READ id WRITE setId NOTIFY itemChanged)
    Q_PROPERTY(ServerItemType type READ type WRITE setType NOTIFY itemChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY itemChanged)

public:
    explicit Item(QObject *parent = nullptr);
    virtual ~Item() = default;

    // Properties - exact mirror of C# Item properties
    quint16 id() const { return m_id; }
    void setId(quint16 id) { m_id = id; emit itemChanged(); }
    
    ServerItemType type() const { return m_type; }
    void setType(ServerItemType type) { m_type = type; emit itemChanged(); }
    
    bool hasStackOrder() const { return m_hasStackOrder; }
    void setHasStackOrder(bool hasStackOrder) { m_hasStackOrder = hasStackOrder; emit itemChanged(); }
    
    TileStackOrder stackOrder() const { return m_stackOrder; }
    void setStackOrder(TileStackOrder stackOrder) { m_stackOrder = stackOrder; emit itemChanged(); }
    
    bool unpassable() const { return m_unpassable; }
    void setUnpassable(bool unpassable) { m_unpassable = unpassable; emit itemChanged(); }
    
    bool blockMissiles() const { return m_blockMissiles; }
    void setBlockMissiles(bool blockMissiles) { m_blockMissiles = blockMissiles; emit itemChanged(); }
    
    bool blockPathfinder() const { return m_blockPathfinder; }
    void setBlockPathfinder(bool blockPathfinder) { m_blockPathfinder = blockPathfinder; emit itemChanged(); }
    
    bool hasElevation() const { return m_hasElevation; }
    void setHasElevation(bool hasElevation) { m_hasElevation = hasElevation; emit itemChanged(); }
    
    bool forceUse() const { return m_forceUse; }
    void setForceUse(bool forceUse) { m_forceUse = forceUse; emit itemChanged(); }
    
    bool multiUse() const { return m_multiUse; }
    void setMultiUse(bool multiUse) { m_multiUse = multiUse; emit itemChanged(); }
    
    bool pickupable() const { return m_pickupable; }
    void setPickupable(bool pickupable) { m_pickupable = pickupable; emit itemChanged(); }
    
    bool movable() const { return m_movable; }
    void setMovable(bool movable) { m_movable = movable; emit itemChanged(); }
    
    bool stackable() const { return m_stackable; }
    void setStackable(bool stackable) { m_stackable = stackable; emit itemChanged(); }
    
    bool readable() const { return m_readable; }
    void setReadable(bool readable) { m_readable = readable; emit itemChanged(); }
    
    bool rotatable() const { return m_rotatable; }
    void setRotatable(bool rotatable) { m_rotatable = rotatable; emit itemChanged(); }
    
    bool hangable() const { return m_hangable; }
    void setHangable(bool hangable) { m_hangable = hangable; emit itemChanged(); }
    
    bool hookSouth() const { return m_hookSouth; }
    void setHookSouth(bool hookSouth) { m_hookSouth = hookSouth; emit itemChanged(); }
    
    bool hookEast() const { return m_hookEast; }
    void setHookEast(bool hookEast) { m_hookEast = hookEast; emit itemChanged(); }
    
    bool hasCharges() const { return m_hasCharges; }
    void setHasCharges(bool hasCharges) { m_hasCharges = hasCharges; emit itemChanged(); }
    
    bool ignoreLook() const { return m_ignoreLook; }
    void setIgnoreLook(bool ignoreLook) { m_ignoreLook = ignoreLook; emit itemChanged(); }
    
    bool fullGround() const { return m_fullGround; }
    void setFullGround(bool fullGround) { m_fullGround = fullGround; emit itemChanged(); }
    
    bool allowDistanceRead() const { return m_allowDistanceRead; }
    void setAllowDistanceRead(bool allowDistanceRead) { m_allowDistanceRead = allowDistanceRead; emit itemChanged(); }
    
    bool isAnimation() const { return m_isAnimation; }
    void setIsAnimation(bool isAnimation) { m_isAnimation = isAnimation; emit itemChanged(); }
    
    quint16 groundSpeed() const { return m_groundSpeed; }
    void setGroundSpeed(quint16 groundSpeed) { m_groundSpeed = groundSpeed; emit itemChanged(); }
    
    quint16 lightLevel() const { return m_lightLevel; }
    void setLightLevel(quint16 lightLevel) { m_lightLevel = lightLevel; emit itemChanged(); }
    
    quint16 lightColor() const { return m_lightColor; }
    void setLightColor(quint16 lightColor) { m_lightColor = lightColor; emit itemChanged(); }
    
    quint16 maxReadChars() const { return m_maxReadChars; }
    void setMaxReadChars(quint16 maxReadChars) { m_maxReadChars = maxReadChars; emit itemChanged(); }
    
    quint16 maxReadWriteChars() const { return m_maxReadWriteChars; }
    void setMaxReadWriteChars(quint16 maxReadWriteChars) { m_maxReadWriteChars = maxReadWriteChars; emit itemChanged(); }
    
    quint16 minimapColor() const { return m_minimapColor; }
    void setMinimapColor(quint16 minimapColor) { m_minimapColor = minimapColor; emit itemChanged(); }
    
    quint16 tradeAs() const { return m_tradeAs; }
    void setTradeAs(quint16 tradeAs) { m_tradeAs = tradeAs; emit itemChanged(); }
    
    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; emit itemChanged(); }
    
    virtual QByteArray spriteHash() const { return m_spriteHash; }
    virtual void setSpriteHash(const QByteArray& spriteHash) { m_spriteHash = spriteHash; emit itemChanged(); }

    // Methods - exact mirror of C# Item methods
    bool equals(const Item* item) const;
    bool hasProperties(ServerItemFlags flags) const;
    void copyPropertiesFrom(const Item* item);

signals:
    void itemChanged();

protected:
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
    QByteArray m_spriteHash;
};

/**
 * ServerItem class
 * Exact mirror of C# ServerItem class extending Item
 */
class ServerItem : public Item
{
    Q_OBJECT
    Q_PROPERTY(QString nameXml READ nameXml WRITE setNameXml NOTIFY itemChanged)
    Q_PROPERTY(quint16 clientId READ clientId WRITE setClientId NOTIFY itemChanged)
    Q_PROPERTY(quint16 previousClientId READ previousClientId WRITE setPreviousClientId NOTIFY itemChanged)
    Q_PROPERTY(bool spriteAssigned READ spriteAssigned WRITE setSpriteAssigned NOTIFY itemChanged)
    Q_PROPERTY(bool isCustomCreated READ isCustomCreated WRITE setIsCustomCreated NOTIFY itemChanged)

public:
    explicit ServerItem(QObject *parent = nullptr);
    explicit ServerItem(const Item* item, QObject *parent = nullptr);
    virtual ~ServerItem() = default;

    // Additional properties specific to ServerItem
    QString nameXml() const { return m_nameXml; }
    void setNameXml(const QString& nameXml) { m_nameXml = nameXml; emit itemChanged(); }
    
    quint16 clientId() const { return m_clientId; }
    void setClientId(quint16 clientId) { m_clientId = clientId; emit itemChanged(); }
    
    quint16 previousClientId() const { return m_previousClientId; }
    void setPreviousClientId(quint16 previousClientId) { m_previousClientId = previousClientId; emit itemChanged(); }
    
    bool spriteAssigned() const { return m_spriteAssigned; }
    void setSpriteAssigned(bool spriteAssigned) { m_spriteAssigned = spriteAssigned; emit itemChanged(); }
    
    bool isCustomCreated() const { return m_isCustomCreated; }
    void setIsCustomCreated(bool isCustomCreated) { m_isCustomCreated = isCustomCreated; emit itemChanged(); }

    // Methods
    QString toString() const;

private:
    QString m_nameXml;
    quint16 m_clientId;
    quint16 m_previousClientId;
    bool m_spriteAssigned;
    bool m_isCustomCreated;
};

} // namespace Items
} // namespace Server
} // namespace OTLib

Q_DECLARE_METATYPE(OTLib::Server::Items::ServerItem*)
Q_DECLARE_METATYPE(OTLib::Server::Items::ServerItemGroup)
Q_DECLARE_METATYPE(OTLib::Server::Items::ServerItemType)
Q_DECLARE_METATYPE(OTLib::Server::Items::TileStackOrder)

#endif // OTLIB_SERVER_ITEMS_SERVERITEM_H