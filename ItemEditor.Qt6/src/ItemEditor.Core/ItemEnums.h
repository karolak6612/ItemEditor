#pragma once

#include <QtGlobal>

/**
 * @brief Enumerations for ItemEditor
 * 
 * Defines all enumerations used in the legacy system
 * to maintain exact compatibility.
 */

// Server item types
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

// Tile stack order
enum class TileStackOrder : quint8
{
    None = 0,
    Ground = 1,
    Border = 2,
    Bottom = 3,
    Top = 4
};

// Item flags (bit flags)
enum class ItemFlag : quint32
{
    None = 0,
    Unpassable = 1 << 0,
    BlockMissiles = 1 << 1,
    BlockPathfinder = 1 << 2,
    HasElevation = 1 << 3,
    MultiUse = 1 << 4,
    Pickupable = 1 << 5,
    Moveable = 1 << 6,
    Stackable = 1 << 7,
    FloorChangeDown = 1 << 8,
    FloorChangeNorth = 1 << 9,
    FloorChangeEast = 1 << 10,
    FloorChangeSouth = 1 << 11,
    FloorChangeWest = 1 << 12,
    AlwaysOnTop = 1 << 13,
    Readable = 1 << 14,
    Rotatable = 1 << 15,
    Hangable = 1 << 16,
    Vertical = 1 << 17,
    Horizontal = 1 << 18,
    CannotDecay = 1 << 19,
    AllowDistRead = 1 << 20,
    Unused = 1 << 21,
    ClientCharges = 1 << 22,
    IgnoreLook = 1 << 23
};

// Sprite directions
enum class SpriteDirection : quint8
{
    North = 0,
    East = 1,
    South = 2,
    West = 3
};

// Animation types
enum class AnimationType : quint8
{
    None = 0,
    Loop = 1,
    PingPong = 2
};