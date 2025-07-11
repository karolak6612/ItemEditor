#ifndef OTBTYPES_H
#define OTBTYPES_H

#include <QString>
#include <QList>
#include <QMap>
#include <QByteArray>
#include <QtGlobal>

namespace OTB {

// Equivalent to C# enum ServerItemType (Source/PluginInterface/Item.cs)
enum class ServerItemType : quint8 {
    None = 0,
    Ground = 1,
    Container = 2,
    Splash = 3,
    Fluid = 4,
    Deprecated = 5, // C# uses this, was also noted as itemGroup in OtbReader
    // Other types if they exist, based on ServerItemGroup in OtbReader.cs
};

// Equivalent to C# enum TileStackOrder (Source/PluginInterface/OTLib/Server/Items/ServerItem.cs)
enum class TileStackOrder : quint8 {
    None = 0,       // Not specified, or default
    Border = 1,     // e.g. fences
    Ground = 2,     // e.g. ground tiles
    Bottom = 3,     // e.g. carpets
    Top = 4,        // e.g. tables
    Creature = 5,   // e.g. players, monsters
    // Values might need adjustment based on actual usage
};

// Equivalent to C# enum ServerItemFlag (Source/PluginInterface/OTLib/Server/Items/ServerItem.cs)
// Stored as quint32 in OTB
namespace ServerItemFlag {
    const quint32 Unpassable        = 1 << 0;  // walkable attribute (default true)
    const quint32 BlockMissiles     = 1 << 1;
    const quint32 BlockPathfinder   = 1 << 2;
    const quint32 HasElevation      = 1 << 3;
    const quint32 ForceUse          = 1 << 4;  // use with -> charges/subtype
    const quint32 MultiUse          = 1 << 5;  // use with -> charges/subtype
    const quint32 Pickupable        = 1 << 6;
    const quint32 Movable           = 1 << 7;
    const quint32 Stackable         = 1 << 8;  // stackable -> amount / subtype
    const quint32 StackOrder        = 1 << 9;  // alternative stack order
    const quint32 Readable          = 1 << 10; // readable -> max text length
    const quint32 Rotatable         = 1 << 11;
    const quint32 Hangable          = 1 << 12;
    const quint32 HookSouth         = 1 << 13;
    const quint32 HookEast          = 1 << 14;
    const quint32 AllowDistanceRead = 1 << 15; // Can be read from far away
    const quint32 ClientCharges     = 1 << 16; // Deprecated
    const quint32 IgnoreLook        = 1 << 17; // Look (true) (usually transparent items)
    const quint32 FullGround        = 1 << 18; // Is full ground
    const quint32 IsAnimation       = 1 << 19; // Is an animation
    // Add any other flags if present in C#
};

// Equivalent to C# enum ServerItemAttribute (Source/PluginInterface/OTLib/Server/Items/ServerItem.cs)
enum class ServerItemAttribute : quint8 {
    ServerID            = 0x10,
    ClientID            = 0x11,
    Name                = 0x12, // deprecated
    Description         = 0x13, // deprecated
    GroundSpeed         = 0x14,
    SpriteHash          = 0x20,
    MinimapColor        = 0x21, // Changed from MinimaColor in C#
    MaxReadWriteChars   = 0x22,
    MaxReadChars        = 0x23,
    Light               = 0x2A,
    StackOrder          = 0x2C,
    TradeAs             = 0x2D,
    // Add any other attributes
};

// From OtbReader.cs / OtbWriter.cs for root node properties
enum class RootAttribute : quint8 {
    Version = 0x01,
};

// From OtbReader.cs for item groups (seems to map to ServerItemType)
enum class ServerItemGroup : quint8 {
    None = 0,
    Ground = 1,
    Container = 2,
    Splash = 3,
    Fluid = 4,
    Deprecated = 5, // Matches ServerItemType::Deprecated
};


struct ServerItem {
    quint16 id = 0; // Server ID
    quint16 clientId = 0;
    ServerItemType type = ServerItemType::None;
    QString name;
    quint32 flags = 0; // Combination of ServerItemFlag values

    // Attributes (can be stored in a map or as individual members if fixed)
    quint16 groundSpeed = 0;
    QByteArray spriteHash; // Should be 16 bytes
    quint16 minimapColor = 0;
    quint16 maxReadWriteChars = 0;
    quint16 maxReadChars = 0;
    quint16 lightLevel = 0;
    quint16 lightColor = 0;
    TileStackOrder stackOrder = TileStackOrder::None;
    quint16 tradeAs = 0; // Ware ID

    // Helper booleans from C# ServerItem.cs, derived from flags or specific attributes
    // These can be methods or set during OTB parsing
    bool unpassable = false;
    bool blockMissiles = false;
    bool blockPathfinder = false;
    bool hasElevation = false;
    bool forceUse = false;
    bool multiUse = false;
    bool pickupable = false;
    bool movable = false;
    bool stackable = false;
    bool hasStackOrder = false; // True if stackOrder attribute is present
    bool readable = false;
    bool rotatable = false;
    bool hangable = false;
    bool hookSouth = false;
    bool hookEast = false;
    bool allowDistanceRead = false;
    bool hasCharges = false; // From ClientCharges flag
    bool ignoreLook = false;
    bool fullGround = false;
    bool isAnimation = false;
    bool isCustomCreated = false; // Not directly in OTB, but used in C# logic

    // For updates
    quint16 previousClientId = 0;
    bool spriteAssigned = false;


    ServerItem() : spriteHash(16, 0) {} // Initialize spriteHash to 16 zero bytes

    // Helper methods to set/check flags
    void setFlag(quint32 flag, bool on) {
        if (on) flags |= flag;
        else flags &= ~flag;
    }
    bool hasFlag(quint32 flag) const {
        return (flags & flag) == flag;
    }

    void updatePropertiesFromFlags() {
        unpassable = hasFlag(ServerItemFlag::Unpassable);
        blockMissiles = hasFlag(ServerItemFlag::BlockMissiles);
        blockPathfinder = hasFlag(ServerItemFlag::BlockPathfinder);
        hasElevation = hasFlag(ServerItemFlag::HasElevation);
        forceUse = hasFlag(ServerItemFlag::ForceUse);
        multiUse = hasFlag(ServerItemFlag::MultiUse);
        pickupable = hasFlag(ServerItemFlag::Pickupable);
        movable = hasFlag(ServerItemFlag::Movable);
        stackable = hasFlag(ServerItemFlag::Stackable);
        // hasStackOrder is set when StackOrder attribute is read
        readable = hasFlag(ServerItemFlag::Readable);
        rotatable = hasFlag(ServerItemFlag::Rotatable);
        hangable = hasFlag(ServerItemFlag::Hangable);
        hookSouth = hasFlag(ServerItemFlag::HookSouth);
        hookEast = hasFlag(ServerItemFlag::HookEast);
        allowDistanceRead = hasFlag(ServerItemFlag::AllowDistanceRead);
        hasCharges = hasFlag(ServerItemFlag::ClientCharges);
        ignoreLook = hasFlag(ServerItemFlag::IgnoreLook);
        fullGround = hasFlag(ServerItemFlag::FullGround);
        isAnimation = hasFlag(ServerItemFlag::IsAnimation);
    }

    void updateFlagsFromProperties() {
        flags = 0;
        setFlag(ServerItemFlag::Unpassable, unpassable);
        setFlag(ServerItemFlag::BlockMissiles, blockMissiles);
        setFlag(ServerItemFlag::BlockPathfinder, blockPathfinder);
        setFlag(ServerItemFlag::HasElevation, hasElevation);
        setFlag(ServerItemFlag::ForceUse, forceUse);
        setFlag(ServerItemFlag::MultiUse, multiUse);
        setFlag(ServerItemFlag::Pickupable, pickupable);
        setFlag(ServerItemFlag::Movable, movable);
        setFlag(ServerItemFlag::Stackable, stackable);
        setFlag(ServerItemFlag::StackOrder, hasStackOrder); // Set if stackOrder attribute is written
        setFlag(ServerItemFlag::Readable, readable);
        setFlag(ServerItemFlag::Rotatable, rotatable);
        setFlag(ServerItemFlag::Hangable, hangable);
        setFlag(ServerItemFlag::HookSouth, hookSouth);
        setFlag(ServerItemFlag::HookEast, hookEast);
        setFlag(ServerItemFlag::AllowDistanceRead, allowDistanceRead);
        setFlag(ServerItemFlag::ClientCharges, hasCharges);
        setFlag(ServerItemFlag::IgnoreLook, ignoreLook);
        setFlag(ServerItemFlag::FullGround, fullGround);
        setFlag(ServerItemFlag::IsAnimation, isAnimation);
    }
};

struct ServerItemList {
    quint32 majorVersion = 3; // OTB File version
    quint32 minorVersion = 0; // Client version (e.g., 770 -> 7.70)
    quint32 buildNumber = 1;  // Revision
    quint32 clientVersion = 0; // Stored as raw number e.g. 770 for 7.70
    QString description; // From CSDVersion in OtbWriter

    QList<ServerItem> items;
    // For quick lookup if needed, similar to C# ServerItems.Items.Find(id)
    // QMap<quint16, ServerItem*> itemMap; // Or just iterate QList for now

    quint16 minId = 0;
    quint16 maxId = 0;

    void clear() {
        items.clear();
        // itemMap.clear();
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
        // itemMap.insert(item.id, &items.last()); // If using map
    }
};

// For OtbWriter version info
struct OtbVersionInfo {
    quint32 majorVersion = 0;
    quint32 minorVersion = 0;
    quint32 buildNumber = 0;
    QString csdVersion; // e.g. "OTB 3.7.70-1.1"
};

} // namespace OTB

#endif // OTBTYPES_H
