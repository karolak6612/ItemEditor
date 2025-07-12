// Helper methods for OtbWriter - separated for better organization
#include "otbwriter.h"
#include <QDataStream>

namespace OTB {

QByteArray OtbWriter::serializeServerID(quint16 serverID) {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << serverID;
    return data;
}

QByteArray OtbWriter::serializeClientID(quint16 clientID) {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << clientID;
    return data;
}

QByteArray OtbWriter::serializeGroundSpeed(quint16 groundSpeed) {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << groundSpeed;
    return data;
}

QByteArray OtbWriter::serializeName(const QString& name) {
    // C# writes name as char array - this is UTF-8 bytes in practice
    return name.toUtf8();
}

QByteArray OtbWriter::serializeSpriteHash(const QByteArray& spriteHash) {
    // Sprite hash is written as-is (16 bytes)
    return spriteHash;
}

QByteArray OtbWriter::serializeMinimapColor(quint16 minimapColor) {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << minimapColor;
    return data;
}

QByteArray OtbWriter::serializeMaxReadWriteChars(quint16 maxReadWriteChars) {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << maxReadWriteChars;
    return data;
}

QByteArray OtbWriter::serializeMaxReadChars(quint16 maxReadChars) {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << maxReadChars;
    return data;
}

QByteArray OtbWriter::serializeLight(quint16 lightLevel, quint16 lightColor) {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << lightLevel;
    stream << lightColor;
    return data;
}

QByteArray OtbWriter::serializeStackOrder(TileStackOrder stackOrder) {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << static_cast<quint8>(stackOrder);
    return data;
}

QByteArray OtbWriter::serializeTradeAs(quint16 tradeAs) {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << tradeAs;
    return data;
}

QList<ServerItemAttribute> OtbWriter::getAttributesToSave(const ServerItem& item) {
    QList<ServerItemAttribute> attributes;
    
    // ServerID is always saved (exactly like C# implementation)
    attributes.append(ServerItemAttribute::ServerID);
    
    // Only save other attributes for non-deprecated items
    if (item.type != ServerItemType::Deprecated) {
        attributes.append(ServerItemAttribute::ClientID);
        attributes.append(ServerItemAttribute::SpriteHash);
        
        if (item.minimapColor != 0) {
            attributes.append(ServerItemAttribute::MinimapColor);
        }
        
        if (item.maxReadWriteChars != 0) {
            attributes.append(ServerItemAttribute::MaxReadWriteChars);
        }
        
        if (item.maxReadChars != 0) {
            attributes.append(ServerItemAttribute::MaxReadChars);
        }
        
        if (item.lightLevel != 0 || item.lightColor != 0) {
            attributes.append(ServerItemAttribute::Light);
        }
        
        if (item.type == ServerItemType::Ground) {
            attributes.append(ServerItemAttribute::GroundSpeed);
        }
        
        if (item.stackOrder != TileStackOrder::None) {
            attributes.append(ServerItemAttribute::StackOrder);
        }
        
        if (item.tradeAs != 0) {
            attributes.append(ServerItemAttribute::TradeAs);
        }
        
        if (!item.name.isEmpty()) {
            attributes.append(ServerItemAttribute::Name);
        }
    }
    
    return attributes;
}

ServerItemGroup OtbWriter::getServerItemGroup(ServerItemType type) {
    // Convert ServerItemType to ServerItemGroup (exactly like C# implementation)
    switch (type) {
        case ServerItemType::Container:
            return ServerItemGroup::Container;
        case ServerItemType::Fluid:
            return ServerItemGroup::Fluid;
        case ServerItemType::Ground:
            return ServerItemGroup::Ground;
        case ServerItemType::Splash:
            return ServerItemGroup::Splash;
        case ServerItemType::Deprecated:
            return ServerItemGroup::Deprecated;
        default:
            return ServerItemGroup::None;
    }
}

quint32 OtbWriter::calculateItemFlags(const ServerItem& item) {
    // Calculate flags exactly like C# implementation
    quint32 flags = 0;
    
    if (item.unpassable) {
        flags |= ServerItemFlag::Unpassable;
    }
    
    if (item.blockMissiles) {
        flags |= ServerItemFlag::BlockMissiles;
    }
    
    if (item.blockPathfinder) {
        flags |= ServerItemFlag::BlockPathfinder;
    }
    
    if (item.hasElevation) {
        flags |= ServerItemFlag::HasElevation;
    }
    
    if (item.forceUse) {
        flags |= ServerItemFlag::ForceUse;
    }
    
    if (item.multiUse) {
        flags |= ServerItemFlag::MultiUse;
    }
    
    if (item.pickupable) {
        flags |= ServerItemFlag::Pickupable;
    }
    
    if (item.movable) {
        flags |= ServerItemFlag::Movable;
    }
    
    if (item.stackable) {
        flags |= ServerItemFlag::Stackable;
    }
    
    if (item.stackOrder != TileStackOrder::None) {
        flags |= ServerItemFlag::StackOrder;
    }
    
    if (item.readable) {
        flags |= ServerItemFlag::Readable;
    }
    
    if (item.rotatable) {
        flags |= ServerItemFlag::Rotatable;
    }
    
    if (item.hangable) {
        flags |= ServerItemFlag::Hangable;
    }
    
    if (item.hookSouth) {
        flags |= ServerItemFlag::HookSouth;
    }
    
    if (item.hookEast) {
        flags |= ServerItemFlag::HookEast;
    }
    
    if (item.hasCharges) {
        flags |= ServerItemFlag::ClientCharges;
    }
    
    if (item.ignoreLook) {
        flags |= ServerItemFlag::IgnoreLook;
    }
    
    if (item.allowDistanceRead) {
        flags |= ServerItemFlag::AllowDistanceRead;
    }
    
    if (item.isAnimation) {
        flags |= ServerItemFlag::IsAnimation;
    }
    
    if (item.fullGround) {
        flags |= ServerItemFlag::FullGround;
    }
    
    return flags;
}

} // namespace OTB