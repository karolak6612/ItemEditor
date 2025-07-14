/**
 * Item Editor Qt6 - OTB Reader Implementation
 * Full implementation with ServerItem integration
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "OtbReader.h"
#include "../Collections/ServerItemList.h"
#include "../Server/Items/ServerItem.h"
#include "../Server/Items/ServerItemFlag.h"
#include "../Server/Items/ServerItemAttribute.h"
#include "../Utils/BinaryTreeReader.h"
#include <QDebug>
#include <QFileInfo>
#include <QDataStream>
#include <memory>

namespace OTLib {
namespace OTB {

OtbReader::OtbReader(QObject *parent)
    : QObject(parent)
    , m_items(std::make_unique<OTLib::Collections::ServerItemList>(this))
{
}

OtbReader::~OtbReader()
{
    clear();
}

bool OtbReader::read(const QString& filePath)
{
    clear();
    
    if (filePath.isEmpty()) {
        qWarning() << "OtbReader: File path is empty";
        return false;
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qWarning() << "OtbReader: File does not exist:" << filePath;
        return false;
    }
    
    m_filePath = filePath;
    
    try {
        // Use BinaryTreeReader for proper OTB parsing
        auto reader = std::make_unique<OTLib::Utils::BinaryTreeReader>(filePath, this);
        
        if (reader->isDisposed()) {
            qWarning() << "OtbReader: Failed to initialize BinaryTreeReader";
            return false;
        }
        
        // Get root node
        auto rootNode = reader->getRootNode();
        if (!rootNode) {
            qWarning() << "OtbReader: Failed to get root node";
            return false;
        }
        
        // Read version header
        if (!readVersionHeader(rootNode.get())) {
            qWarning() << "OtbReader: Failed to read version header";
            return false;
        }
        
        // Get first child node (first item)
        auto itemNode = reader->getChildNode();
        if (!itemNode) {
            qWarning() << "OtbReader: No items found in OTB file";
            return false;
        }
        
        // Read all items
        do {
            auto item = readItem(itemNode.get());
            if (item) {
                m_items->add(item);
            }
            itemNode = reader->getNextNode();
        } while (itemNode);
        
        qDebug() << "OtbReader: Successfully loaded" << m_items->count() << "items";
        return true;
        
    } catch (const std::exception& e) {
        qWarning() << "OtbReader: Exception during reading:" << e.what();
        return false;
    } catch (...) {
        qWarning() << "OtbReader: Unknown exception during reading";
        return false;
    }
}

void OtbReader::clear()
{
    if (m_items) {
        m_items->clear();
    }
    m_filePath.clear();
}

int OtbReader::count() const
{
    return m_items ? m_items->count() : 0;
}

bool OtbReader::readVersionHeader(QDataStream* node)
{
    if (!node) {
        return false;
    }
    
    try {
        // Read first byte (should be 0)
        quint8 firstByte;
        *node >> firstByte;
        
        // Read 4 bytes flags (unused)
        quint32 flags;
        *node >> flags;
        
        // Read attribute type
        quint8 attr;
        *node >> attr;
        
        if (static_cast<RootAttribute>(attr) == RootAttribute::Version) {
            quint16 dataLen;
            *node >> dataLen;
            
            if (dataLen != 140) { // 4 + 4 + 4 + 1 * 128
                qWarning() << "OtbReader: Invalid version header size:" << dataLen;
                return false;
            }
            
            quint32 majorVersion, minorVersion, buildNumber;
            *node >> majorVersion >> minorVersion >> buildNumber;
            
            m_items->setMajorVersion(majorVersion);
            m_items->setMinorVersion(minorVersion);
            m_items->setBuildNumber(buildNumber);
            
            // Skip 128 bytes
            node->skipRawData(128);
            
            qDebug() << "OtbReader: Version" << majorVersion << "." << minorVersion << "." << buildNumber;
            return true;
        }
        
        return false;
    } catch (...) {
        return false;
    }
}

OTLib::Server::Items::ServerItem* OtbReader::readItem(QDataStream* node)
{
    if (!node) {
        return nullptr;
    }
    
    try {
        auto item = new OTLib::Server::Items::ServerItem(this);
        
        // Read item group
        quint8 itemGroupByte;
        *node >> itemGroupByte;
        auto itemGroup = static_cast<OTLib::Server::Items::ServerItemGroup>(itemGroupByte);
        
        // Set item type based on group
        switch (itemGroup) {
            case OTLib::Server::Items::ServerItemGroup::None:
                item->setType(OTLib::Server::Items::ServerItemType::None);
                break;
            case OTLib::Server::Items::ServerItemGroup::Ground:
                item->setType(OTLib::Server::Items::ServerItemType::Ground);
                break;
            case OTLib::Server::Items::ServerItemGroup::Container:
                item->setType(OTLib::Server::Items::ServerItemType::Container);
                break;
            case OTLib::Server::Items::ServerItemGroup::Splash:
                item->setType(OTLib::Server::Items::ServerItemType::Splash);
                break;
            case OTLib::Server::Items::ServerItemGroup::Fluid:
                item->setType(OTLib::Server::Items::ServerItemType::Fluid);
                break;
            case OTLib::Server::Items::ServerItemGroup::Deprecated:
                item->setType(OTLib::Server::Items::ServerItemType::Deprecated);
                break;
            default:
                item->setType(OTLib::Server::Items::ServerItemType::None);
                break;
        }
        
        // Read flags
        quint32 flagsValue;
        *node >> flagsValue;
        auto flags = static_cast<OTLib::Server::Items::ServerItemFlags>(flagsValue);
        
        // Parse flags into item properties
        parseItemFlags(flags, item);
        
        // Parse attributes
        parseItemAttributes(node, item);
        
        // Set default sprite hash if not set and not deprecated
        if (item->spriteHash().isEmpty() && item->type() != OTLib::Server::Items::ServerItemType::Deprecated) {
            item->setSpriteHash(QByteArray(16, 0));
        }
        
        return item;
        
    } catch (...) {
        return nullptr;
    }
}

void OtbReader::parseItemFlags(OTLib::Server::Items::ServerItemFlags flags, OTLib::Server::Items::ServerItem* item)
{
    using namespace OTLib::Server::Items;
    
    item->setUnpassable(flags.testFlag(ServerItemFlag::Unpassable));
    item->setBlockMissiles(flags.testFlag(ServerItemFlag::BlockMissiles));
    item->setBlockPathfinder(flags.testFlag(ServerItemFlag::BlockPathfinder));
    item->setHasElevation(flags.testFlag(ServerItemFlag::HasElevation));
    item->setForceUse(flags.testFlag(ServerItemFlag::ForceUse));
    item->setMultiUse(flags.testFlag(ServerItemFlag::MultiUse));
    item->setPickupable(flags.testFlag(ServerItemFlag::Pickupable));
    item->setMovable(flags.testFlag(ServerItemFlag::Movable));
    item->setStackable(flags.testFlag(ServerItemFlag::Stackable));
    item->setHasStackOrder(flags.testFlag(ServerItemFlag::StackOrder));
    item->setReadable(flags.testFlag(ServerItemFlag::Readable));
    item->setRotatable(flags.testFlag(ServerItemFlag::Rotatable));
    item->setHangable(flags.testFlag(ServerItemFlag::Hangable));
    item->setHookSouth(flags.testFlag(ServerItemFlag::HookSouth));
    item->setHookEast(flags.testFlag(ServerItemFlag::HookEast));
    item->setAllowDistanceRead(flags.testFlag(ServerItemFlag::AllowDistanceRead));
    item->setHasCharges(flags.testFlag(ServerItemFlag::ClientCharges));
    item->setIgnoreLook(flags.testFlag(ServerItemFlag::IgnoreLook));
    item->setFullGround(flags.testFlag(ServerItemFlag::FullGround));
    item->setIsAnimation(flags.testFlag(ServerItemFlag::IsAnimation));
}

void OtbReader::parseItemAttributes(QDataStream* node, OTLib::Server::Items::ServerItem* item)
{
    using namespace OTLib::Server::Items;
    
    try {
        while (!node->atEnd()) {
            quint8 attrByte;
            if (node->readRawData(reinterpret_cast<char*>(&attrByte), 1) != 1) {
                break;
            }
            
            auto attribute = static_cast<ServerItemAttribute>(attrByte);
            
            quint16 dataLen;
            *node >> dataLen;
            
            switch (attribute) {
                case ServerItemAttribute::ServerID: {
                    quint16 serverId;
                    *node >> serverId;
                    item->setId(serverId);
                    break;
                }
                
                case ServerItemAttribute::ClientID: {
                    quint16 clientId;
                    *node >> clientId;
                    item->setClientId(clientId);
                    break;
                }
                
                case ServerItemAttribute::GroundSpeed: {
                    quint16 groundSpeed;
                    *node >> groundSpeed;
                    item->setGroundSpeed(groundSpeed);
                    break;
                }
                
                case ServerItemAttribute::Name: {
                    QByteArray nameData(dataLen, 0);
                    node->readRawData(nameData.data(), dataLen);
                    QString name = QString::fromUtf8(nameData);
                    item->setName(name);
                    break;
                }
                
                case ServerItemAttribute::SpriteHash: {
                    QByteArray spriteHash(dataLen, 0);
                    node->readRawData(spriteHash.data(), dataLen);
                    item->setSpriteHash(spriteHash);
                    break;
                }
                
                case ServerItemAttribute::MinimaColor: {
                    quint16 minimapColor;
                    *node >> minimapColor;
                    item->setMinimapColor(minimapColor);
                    break;
                }
                
                case ServerItemAttribute::MaxReadWriteChars: {
                    quint16 maxReadWriteChars;
                    *node >> maxReadWriteChars;
                    item->setMaxReadWriteChars(maxReadWriteChars);
                    break;
                }
                
                case ServerItemAttribute::MaxReadChars: {
                    quint16 maxReadChars;
                    *node >> maxReadChars;
                    item->setMaxReadChars(maxReadChars);
                    break;
                }
                
                case ServerItemAttribute::Light: {
                    quint16 lightLevel, lightColor;
                    *node >> lightLevel >> lightColor;
                    item->setLightLevel(lightLevel);
                    item->setLightColor(lightColor);
                    break;
                }
                
                case ServerItemAttribute::StackOrder: {
                    quint8 stackOrderByte;
                    *node >> stackOrderByte;
                    auto stackOrder = static_cast<TileStackOrder>(stackOrderByte);
                    item->setStackOrder(stackOrder);
                    break;
                }
                
                case ServerItemAttribute::TradeAs: {
                    quint16 tradeAs;
                    *node >> tradeAs;
                    item->setTradeAs(tradeAs);
                    break;
                }
                
                default:
                    // Skip unknown attributes
                    node->skipRawData(dataLen);
                    break;
            }
        }
    } catch (...) {
        // Continue parsing even if some attributes fail
    }
}

} // namespace OTB
} // namespace OTLib