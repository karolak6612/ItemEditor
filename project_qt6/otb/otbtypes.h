#ifndef OTBTYPES_H
#define OTBTYPES_H

#include <QString>
#include <QList>
#include <QMap>
#include <QByteArray>
#include <QtGlobal>

// Forward declare ItemBase if preferred, or include item.h if direct use is needed
// For method signatures like equals(const ItemBase&), full definition of ItemBase is needed.
#include "item.h" // For ItemBase

namespace OTB {

// Equivalent to C# enum ServerItemType (Source/PluginInterface/Item.cs)
enum class ServerItemType : quint8 {
    None = 0,
    Ground = 1,
    Container = 2,
    Splash = 3,
    Fluid = 4,
    Deprecated = 5, // C# uses this, was also noted as itemGroup in OtbReader.cs
};

// Equivalent to C# enum TileStackOrder (Source/PluginInterface/OTLib/Server/Items/ServerItem.cs)
enum class TileStackOrder : quint8 {
    None = 0,       // Not specified, or default
    Border = 1,     // e.g. fences
    Ground = 2,     // e.g. ground tiles
    Bottom = 3,     // e.g. carpets
    Top = 4,        // e.g. tables
    Creature = 5,   // e.g. players, monsters
};

// Equivalent to C# enum ServerItemFlag (Source/PluginInterface/OTLib/Server/Items/ServerItem.cs)
// Stored as quint32 in OTB
namespace ServerItemFlag {
    const quint32 Unpassable        = 1 << 0;
    const quint32 BlockMissiles     = 1 << 1;
    const quint32 BlockPathfinder   = 1 << 2;
    const quint32 HasElevation      = 1 << 3;
    const quint32 ForceUse          = 1 << 4;
    const quint32 MultiUse          = 1 << 5;
    const quint32 Pickupable        = 1 << 6;
    const quint32 Movable           = 1 << 7;
    const quint32 Stackable         = 1 << 8;
    const quint32 StackOrder        = 1 << 9;  // This flag means "has stack order attribute"
    const quint32 Readable          = 1 << 10;
    const quint32 Rotatable         = 1 << 11;
    const quint32 Hangable          = 1 << 12;
    const quint32 HookSouth         = 1 << 13;
    const quint32 HookEast          = 1 << 14;
    const quint32 AllowDistanceRead = 1 << 15;
    const quint32 ClientCharges     = 1 << 16; // Deprecated, use HasCharges
    const quint32 IgnoreLook        = 1 << 17;
    const quint32 FullGround        = 1 << 18;
    const quint32 IsAnimation       = 1 << 19;
};

// Equivalent to C# enum ServerItemAttribute (Source/PluginInterface/OTLib/Server/Items/ServerItem.cs)
enum class ServerItemAttribute : quint8 {
    ServerID            = 0x10,
    ClientID            = 0x11,
    Name                = 0x12,
    Description         = 0x13, // Deprecated in later OTB versions, but C# ItemEditor might handle it for older ones
    GroundSpeed         = 0x14,
    SpriteHash          = 0x20,
    MinimapColor        = 0x21,
    MaxReadWriteChars   = 0x22,
    MaxReadChars        = 0x23,
    Light               = 0x2A,
    StackOrder          = 0x2C, // The actual stack order value if ServerItemFlag::StackOrder is set
    TradeAs             = 0x2D,
};

enum class RootAttribute : quint8 {
    Version = 0x01,
};

enum class ServerItemGroup : quint8 {
    None = 0,
    Ground = 1,
    Container = 2,
    Splash = 3,
    Fluid = 4,
    Deprecated = 5,
};


struct ServerItem {
    quint16 id = 0;
    quint16 clientId = 0;
    ServerItemType type = ServerItemType::None;
    QString name;
    quint32 flags = 0;

    quint16 groundSpeed = 0;
    QByteArray spriteHash;
    quint16 minimapColor = 0;
    quint16 maxReadWriteChars = 0;
    quint16 maxReadChars = 0;
    quint16 lightLevel = 0;
    quint16 lightColor = 0;
    TileStackOrder stackOrder = TileStackOrder::None;
    quint16 tradeAs = 0;

    // Boolean properties derived from flags for convenience (as in C# Item.cs)
    bool unpassable = false;
    bool blockMissiles = false;
    bool blockPathfinder = false;
    bool hasElevation = false;
    bool forceUse = false;
    bool multiUse = false;
    bool pickupable = false;
    bool movable = true; // Default true in C# Item.cs constructor
    bool stackable = false;
    bool hasStackOrder = false; // True if StackOrder attribute is present and ServerItemFlag::StackOrder is set
    bool readable = false;
    bool rotatable = false;
    bool hangable = false;
    bool hookSouth = false;
    bool hookEast = false;
    bool allowDistanceRead = false;
    bool hasCharges = false;
    bool ignoreLook = false;
    bool fullGround = false;
    bool isAnimation = false;

    bool isCustomCreated = false;

    quint16 previousClientId = 0;
    bool spriteAssigned = false;


    ServerItem() : spriteHash(16, 0) {}

    void setFlag(quint32 flag, bool on) {
        if (on) flags |= flag;
        else flags &= ~flag;
    }
    bool hasFlag(quint32 flagValue) const { // Renamed parameter to avoid conflict
        return (flags & flagValue) == flagValue;
    }

    void updatePropertiesFromFlags(); // Implementation in .cpp
    void updateFlagsFromProperties(); // Implementation in .cpp

    // Methods for property comparison and copying, similar to C# Item.cs
    bool equals(const ItemBase& other) const; // ItemBase is defined in item.h
    void copyPropertiesFrom(const ItemBase& source); // ItemBase is defined in item.h
};

struct ServerItemList {
    quint32 majorVersion = 3;
    quint32 minorVersion = 0;
    quint32 buildNumber = 1;
    quint32 clientVersion = 0;
    QString description;

    QList<ServerItem> items;

    quint16 minId = 0;
    quint16 maxId = 0;

    void clear() {
        items.clear();
        majorVersion = 3;
        minorVersion = 0;
        buildNumber = 1;
        clientVersion = 0;
        description.clear();
        minId = 0;
        maxId = 0;
    }

    void add(const ServerItem& item) {
        items.append(item);
        if (items.count() == 1) {
            minId = item.id;
            maxId = item.id;
        } else {
            if (item.id < minId) minId = item.id;
            if (item.id > maxId) maxId = item.id;
        }
    }
};

struct OtbVersionInfo {
    quint32 majorVersion = 0;
    quint32 minorVersion = 0;
    quint32 buildNumber = 0;
    QString csdVersion;
};

} // namespace OTB

#endif // OTBTYPES_H
