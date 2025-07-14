/**
 * Item Editor Qt6 - Server Item Flag Header
 * Exact mirror of Legacy_App/csharp/Source/PluginInterface/OTLib/Server/Items/ServerItemFlag.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef OTLIB_SERVER_ITEMS_SERVERITEMFLAG_H
#define OTLIB_SERVER_ITEMS_SERVERITEMFLAG_H

#include <QFlags>
#include <QMetaType>

namespace OTLib {
namespace Server {
namespace Items {

/**
 * Server Item Flag Enumeration
 * Exact mirror of C# [Flags] enum ServerItemFlag
 * 
 * Represents various flags that can be applied to server items
 * Uses QFlags for bitwise operations similar to C# [Flags] attribute
 */
enum class ServerItemFlag : quint32
{
    None                = 0,
    Unpassable          = 1 << 0,
    BlockMissiles       = 1 << 1,
    BlockPathfinder     = 1 << 2,
    HasElevation        = 1 << 3,
    MultiUse            = 1 << 4,
    Pickupable          = 1 << 5,
    Movable             = 1 << 6,
    Stackable           = 1 << 7,
    FloorChangeDown     = 1 << 8,
    FloorChangeNorth    = 1 << 9,
    FloorChangeEast     = 1 << 10,
    FloorChangeSouth    = 1 << 11,
    FloorChangeWest     = 1 << 12,
    StackOrder          = 1 << 13,
    Readable            = 1 << 14,
    Rotatable           = 1 << 15,
    Hangable            = 1 << 16,
    HookSouth           = 1 << 17,
    HookEast            = 1 << 18,
    CanNotDecay         = 1 << 19,
    AllowDistanceRead   = 1 << 20,
    Unused              = 1 << 21,
    ClientCharges       = 1 << 22,
    IgnoreLook          = 1 << 23,
    IsAnimation         = 1 << 24,
    FullGround          = 1 << 25,
    ForceUse            = 1 << 26
};

// Enable QFlags operations for ServerItemFlag
Q_DECLARE_FLAGS(ServerItemFlags, ServerItemFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(ServerItemFlags)

} // namespace Items
} // namespace Server
} // namespace OTLib

// Register with Qt meta-object system for Q_PROPERTY usage
Q_DECLARE_METATYPE(OTLib::Server::Items::ServerItemFlag)
Q_DECLARE_METATYPE(OTLib::Server::Items::ServerItemFlags)

#endif // OTLIB_SERVER_ITEMS_SERVERITEMFLAG_H