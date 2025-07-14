/**
 * Item Editor Qt6 - Server Item Attribute Header
 * Exact mirror of Legacy_App/csharp/Source/PluginInterface/OTLib/Server/Items/ServerItemAttribute.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef OTLIB_SERVER_ITEMS_SERVERITEMATTRIBUTE_H
#define OTLIB_SERVER_ITEMS_SERVERITEMATTRIBUTE_H

#include <QtGlobal>

namespace OTLib {
namespace Server {
namespace Items {

/**
 * Server Item Attribute enumeration
 * Exact mirror of C# ServerItemAttribute enum
 */
enum class ServerItemAttribute : quint8
{
    ServerID = 0x10,
    ClientID = 0x11,
    Name = 0x12,
    GroundSpeed = 0x14,
    SpriteHash = 0x20,
    MinimaColor = 0x21,
    MaxReadWriteChars = 0x22,
    MaxReadChars = 0x23,
    Light = 0x2A,
    StackOrder = 0x2B,
    TradeAs = 0x2D
};

} // namespace Items
} // namespace Server
} // namespace OTLib

#endif // OTLIB_SERVER_ITEMS_SERVERITEMATTRIBUTE_H