/**
 * Item Editor Qt6 - OTB Writer Implementation
 * Full implementation with ServerItem integration
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "OtbWriter.h"
#include "../Collections/ServerItemList.h"
#include "../Server/Items/ServerItem.h"
#include "../Server/Items/ServerItemFlag.h"
#include "../Server/Items/ServerItemAttribute.h"
#include "../Utils/BinaryTreeWriter.h"
#include "../Utils/SpecialChar.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QVariant>
#include <QBuffer>
#include <memory>

namespace OTLib {
namespace OTB {

OtbWriter::OtbWriter(QObject *parent)
    : QObject(parent)
{
}

bool OtbWriter::write(const QString& filePath, OTLib::Collections::ServerItemList* items)
{
    return write(filePath, items, m_versionInfo);
}

bool OtbWriter::write(const QString& filePath, OTLib::Collections::ServerItemList* items, const OtbVersionInfo& versionInfo)
{
    m_lastError.clear();
    
    if (filePath.isEmpty()) {
        setError("File path is empty");
        return false;
    }
    
    if (!items || items->count() == 0) {
        setError("No items to write");
        return false;
    }
    
    try {
        emit statusChanged("Creating OTB file...");
        
        // Use BinaryTreeWriter for proper OTB structure
        auto writer = std::make_unique<OTLib::Utils::BinaryTreeWriter>(filePath, this);
        
        if (writer->isDisposed()) {
            setError("Failed to initialize BinaryTreeWriter");
            return false;
        }
        
        emit statusChanged("Writing OTB header...");
        
        // Create root node with version header
        writer->createNode(0); // Root node type
        if (!writeVersionHeader(writer.get(), versionInfo)) {
            setError("Failed to write version header");
            return false;
        }
        
        emit statusChanged("Writing items...");
        
        // Write all items
        int itemCount = 0;
        for (auto item : items->items()) {
            if (!item) continue;
            
            // Create child node for each item
            writer->createNode(static_cast<quint8>(getItemGroup(item)));
            
            if (!writeItem(writer.get(), item)) {
                setError(QString("Failed to write item %1").arg(item->id()));
                return false;
            }
            
            writer->closeNode();
            
            itemCount++;
            int progress = (itemCount * 100) / items->count();
            emit progressChanged(progress);
        }
        
        // Close root node
        writer->closeNode();
        
        emit statusChanged("OTB file written successfully");
        emit progressChanged(100);
        
        return true;
        
    } catch (const std::exception& e) {
        setError(QString("Exception during writing: %1").arg(e.what()));
        return false;
    } catch (...) {
        setError("Unknown exception during writing");
        return false;
    }
}

bool OtbWriter::writeVersionHeader(OTLib::Utils::BinaryTreeWriter* writer, const OtbVersionInfo& versionInfo)
{
    if (!writer) {
        return false;
    }
    
    try {
        // Write first byte (0)
        writer->writeByte(0);
        
        // Write 4 bytes flags (unused)
        writer->writeUInt32(0);
        
        // Create version attribute data
        QByteArray versionData;
        QDataStream versionStream(&versionData, QIODevice::WriteOnly);
        versionStream.setByteOrder(QDataStream::LittleEndian);
        
        // Write version info to stream
        versionStream << static_cast<quint32>(versionInfo.getMajorVersion());
        versionStream << static_cast<quint32>(versionInfo.getMinorVersion());
        versionStream << static_cast<quint32>(versionInfo.getBuildNumber());
        
        // Write 128 bytes of padding/description
        QByteArray padding(128, 0);
        QByteArray descBytes = versionInfo.getDescription().toUtf8();
        if (descBytes.size() < 128) {
            padding.replace(0, descBytes.size(), descBytes);
        }
        versionStream.writeRawData(padding.data(), 128);
        
        // Write version attribute
        writer->writeByte(static_cast<quint8>(OTLib::Utils::RootAttribute::Version));
        writer->writeUInt16(static_cast<quint16>(versionData.size()));
        writer->writeBytes(versionData, true);
        
        return true;
    } catch (...) {
        return false;
    }
}

bool OtbWriter::writeItem(OTLib::Utils::BinaryTreeWriter* writer, OTLib::Server::Items::ServerItem* item)
{
    if (!writer || !item) {
        return false;
    }
    
    try {
        // Write item flags
        auto flags = getItemFlags(item);
        writer->writeUInt32(static_cast<quint32>(flags));
        
        // Write item attributes
        writeItemAttributes(writer, item);
        
        return true;
    } catch (...) {
        return false;
    }
}

OTLib::Server::Items::ServerItemGroup OtbWriter::getItemGroup(OTLib::Server::Items::ServerItem* item)
{
    using namespace OTLib::Server::Items;
    
    switch (item->type()) {
        case ServerItemType::Ground:
            return ServerItemGroup::Ground;
        case ServerItemType::Container:
            return ServerItemGroup::Container;
        case ServerItemType::Splash:
            return ServerItemGroup::Splash;
        case ServerItemType::Fluid:
            return ServerItemGroup::Fluid;
        case ServerItemType::Deprecated:
            return ServerItemGroup::Deprecated;
        default:
            return ServerItemGroup::None;
    }
}

OTLib::Server::Items::ServerItemFlags OtbWriter::getItemFlags(OTLib::Server::Items::ServerItem* item)
{
    using namespace OTLib::Server::Items;
    
    ServerItemFlags flags = ServerItemFlag::None;
    
    if (item->unpassable()) flags |= ServerItemFlag::Unpassable;
    if (item->blockMissiles()) flags |= ServerItemFlag::BlockMissiles;
    if (item->blockPathfinder()) flags |= ServerItemFlag::BlockPathfinder;
    if (item->hasElevation()) flags |= ServerItemFlag::HasElevation;
    if (item->forceUse()) flags |= ServerItemFlag::ForceUse;
    if (item->multiUse()) flags |= ServerItemFlag::MultiUse;
    if (item->pickupable()) flags |= ServerItemFlag::Pickupable;
    if (item->movable()) flags |= ServerItemFlag::Movable;
    if (item->stackable()) flags |= ServerItemFlag::Stackable;
    if (item->hasStackOrder()) flags |= ServerItemFlag::StackOrder;
    if (item->readable()) flags |= ServerItemFlag::Readable;
    if (item->rotatable()) flags |= ServerItemFlag::Rotatable;
    if (item->hangable()) flags |= ServerItemFlag::Hangable;
    if (item->hookSouth()) flags |= ServerItemFlag::HookSouth;
    if (item->hookEast()) flags |= ServerItemFlag::HookEast;
    if (item->allowDistanceRead()) flags |= ServerItemFlag::AllowDistanceRead;
    if (item->hasCharges()) flags |= ServerItemFlag::ClientCharges;
    if (item->ignoreLook()) flags |= ServerItemFlag::IgnoreLook;
    if (item->fullGround()) flags |= ServerItemFlag::FullGround;
    if (item->isAnimation()) flags |= ServerItemFlag::IsAnimation;
    
    return flags;
}

void OtbWriter::writeItemAttributes(OTLib::Utils::BinaryTreeWriter* writer, OTLib::Server::Items::ServerItem* item)
{
    using namespace OTLib::Server::Items;
    
    // Write Server ID
    QByteArray serverIdData;
    QDataStream serverIdStream(&serverIdData, QIODevice::WriteOnly);
    serverIdStream.setByteOrder(QDataStream::LittleEndian);
    serverIdStream << static_cast<quint16>(item->id());
    writer->writeByte(static_cast<quint8>(ServerItemAttribute::ServerID));
    writer->writeUInt16(static_cast<quint16>(serverIdData.size()));
    writer->writeBytes(serverIdData, true);
    
    // Write Client ID
    QByteArray clientIdData;
    QDataStream clientIdStream(&clientIdData, QIODevice::WriteOnly);
    clientIdStream.setByteOrder(QDataStream::LittleEndian);
    clientIdStream << static_cast<quint16>(item->clientId());
    writer->writeByte(static_cast<quint8>(ServerItemAttribute::ClientID));
    writer->writeUInt16(static_cast<quint16>(clientIdData.size()));
    writer->writeBytes(clientIdData, true);
    
    // Write Ground Speed if set
    if (item->groundSpeed() > 0) {
        QByteArray groundSpeedData;
        QDataStream groundSpeedStream(&groundSpeedData, QIODevice::WriteOnly);
        groundSpeedStream.setByteOrder(QDataStream::LittleEndian);
        groundSpeedStream << static_cast<quint16>(item->groundSpeed());
        writer->writeByte(static_cast<quint8>(ServerItemAttribute::GroundSpeed));
        writer->writeUInt16(static_cast<quint16>(groundSpeedData.size()));
        writer->writeBytes(groundSpeedData, true);
    }
    
    // Write Name if set
    if (!item->name().isEmpty()) {
        QByteArray nameBytes = item->name().toUtf8();
        writer->writeByte(static_cast<quint8>(ServerItemAttribute::Name));
        writer->writeUInt16(static_cast<quint16>(nameBytes.size()));
        writer->writeBytes(nameBytes, true);
    }
    
    // Write Sprite Hash if set
    if (!item->spriteHash().isEmpty()) {
        writer->writeByte(static_cast<quint8>(ServerItemAttribute::SpriteHash));
        writer->writeUInt16(static_cast<quint16>(item->spriteHash().size()));
        writer->writeBytes(item->spriteHash(), true);
    }
    
    // Write Minimap Color if set
    if (item->minimapColor() > 0) {
        QByteArray minimapColorData;
        QDataStream minimapColorStream(&minimapColorData, QIODevice::WriteOnly);
        minimapColorStream.setByteOrder(QDataStream::LittleEndian);
        minimapColorStream << static_cast<quint16>(item->minimapColor());
        writer->writeByte(static_cast<quint8>(ServerItemAttribute::MinimaColor));
        writer->writeUInt16(static_cast<quint16>(minimapColorData.size()));
        writer->writeBytes(minimapColorData, true);
    }
    
    // Write Max Read Write Chars if set
    if (item->maxReadWriteChars() > 0) {
        QByteArray maxReadWriteCharsData;
        QDataStream maxReadWriteCharsStream(&maxReadWriteCharsData, QIODevice::WriteOnly);
        maxReadWriteCharsStream.setByteOrder(QDataStream::LittleEndian);
        maxReadWriteCharsStream << static_cast<quint16>(item->maxReadWriteChars());
        writer->writeByte(static_cast<quint8>(ServerItemAttribute::MaxReadWriteChars));
        writer->writeUInt16(static_cast<quint16>(maxReadWriteCharsData.size()));
        writer->writeBytes(maxReadWriteCharsData, true);
    }
    
    // Write Max Read Chars if set
    if (item->maxReadChars() > 0) {
        QByteArray maxReadCharsData;
        QDataStream maxReadCharsStream(&maxReadCharsData, QIODevice::WriteOnly);
        maxReadCharsStream.setByteOrder(QDataStream::LittleEndian);
        maxReadCharsStream << static_cast<quint16>(item->maxReadChars());
        writer->writeByte(static_cast<quint8>(ServerItemAttribute::MaxReadChars));
        writer->writeUInt16(static_cast<quint16>(maxReadCharsData.size()));
        writer->writeBytes(maxReadCharsData, true);
    }
    
    // Write Light if set
    if (item->lightLevel() > 0 || item->lightColor() > 0) {
        QByteArray lightData;
        QDataStream lightStream(&lightData, QIODevice::WriteOnly);
        lightStream.setByteOrder(QDataStream::LittleEndian);
        lightStream << static_cast<quint16>(item->lightLevel());
        lightStream << static_cast<quint16>(item->lightColor());
        writer->writeByte(static_cast<quint8>(ServerItemAttribute::Light));
        writer->writeUInt16(static_cast<quint16>(lightData.size()));
        writer->writeBytes(lightData, true);
    }
    
    // Write Stack Order if set
    if (item->hasStackOrder()) {
        QByteArray stackOrderData;
        QDataStream stackOrderStream(&stackOrderData, QIODevice::WriteOnly);
        stackOrderStream.setByteOrder(QDataStream::LittleEndian);
        stackOrderStream << static_cast<quint8>(item->stackOrder());
        writer->writeByte(static_cast<quint8>(ServerItemAttribute::StackOrder));
        writer->writeUInt16(static_cast<quint16>(stackOrderData.size()));
        writer->writeBytes(stackOrderData, true);
    }
    
    // Write Trade As if set
    if (item->tradeAs() > 0) {
        QByteArray tradeAsData;
        QDataStream tradeAsStream(&tradeAsData, QIODevice::WriteOnly);
        tradeAsStream.setByteOrder(QDataStream::LittleEndian);
        tradeAsStream << static_cast<quint16>(item->tradeAs());
        writer->writeByte(static_cast<quint8>(ServerItemAttribute::TradeAs));
        writer->writeUInt16(static_cast<quint16>(tradeAsData.size()));
        writer->writeBytes(tradeAsData, true);
    }
}

void OtbWriter::setError(const QString& error)
{
    m_lastError = error;
    qWarning() << "OtbWriter error:" << error;
}

} // namespace OTB
} // namespace OTLib