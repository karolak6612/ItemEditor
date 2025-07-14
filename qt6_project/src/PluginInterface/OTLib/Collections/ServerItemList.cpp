/**
 * Item Editor Qt6 - Server Item List Implementation
 * Exact mirror of Legacy_App/csharp/Source/PluginInterface/OTLib/Collections/ServerItemList.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "ServerItemList.h"
#include "../Server/Items/ServerItem.h"
#include "../Server/Items/ServerItemFlag.h"

namespace OTLib {
namespace Collections {

ServerItemList::ServerItemList(QObject *parent)
    : QObject(parent)
    , m_minId(100)
    , m_maxId(100)
    , m_majorVersion(0)
    , m_minorVersion(0)
    , m_buildNumber(0)
    , m_clientVersion(0)
{
    // Initialize empty list - equivalent to C# constructor
}

ServerItemList::~ServerItemList()
{
    // Qt6 parent-child ownership will handle cleanup
    // Items will be deleted when their parent is deleted
    clear();
}

void ServerItemList::add(OTLib::Server::Items::ServerItem* item)
{
    if (!item) {
        return; // Null check
    }
    
    // Check if item already exists (equivalent to C# Contains)
    if (m_items.contains(item)) {
        return;
    }
    
    // Add item to collection
    m_items.append(item);
    
    // Set this object as parent for Qt6 memory management
    if (item->parent() == nullptr) {
        item->setParent(this);
    }
    
    // Update MaxId if necessary (equivalent to C# logic)
    if (m_maxId < item->id()) {
        m_maxId = item->id();
    }
    
    // Emit signals for change notification
    emit itemAdded(item);
    emit collectionChanged();
}

void ServerItemList::clear()
{
    // Clear all items (Qt6 parent-child will handle deletion)
    m_items.clear();
    
    // Reset MaxId to default value (equivalent to C# logic)
    m_maxId = 100;
    
    // Emit signals for change notification
    emit collectionCleared();
    emit collectionChanged();
}

QList<OTLib::Server::Items::ServerItem*> ServerItemList::findByServerId(quint16 sid) const
{
    QList<OTLib::Server::Items::ServerItem*> result;
    
    // Equivalent to C# Items.FindAll(i => i.ID == sid)
    for (auto* item : m_items) {
        if (item && item->id() == sid) {
            result.append(item);
        }
    }
    
    return result;
}

QList<OTLib::Server::Items::ServerItem*> ServerItemList::findByClientId(quint16 cid) const
{
    QList<OTLib::Server::Items::ServerItem*> result;
    
    // Equivalent to C# Items.FindAll(i => i.ClientId == cid)
    for (auto* item : m_items) {
        if (item && item->clientId() == cid) {
            result.append(item);
        }
    }
    
    return result;
}

QList<OTLib::Server::Items::ServerItem*> ServerItemList::findByProperties(
    OTLib::Server::Items::ServerItemFlags properties) const
{
    QList<OTLib::Server::Items::ServerItem*> result;
    
    // Equivalent to C# Items.FindAll(i => i.HasProperties(properties))
    for (auto* item : m_items) {
        if (item && item->hasProperties(properties)) {
            result.append(item);
        }
    }
    
    return result;
}

bool ServerItemList::tryGetValue(quint16 sid, OTLib::Server::Items::ServerItem*& item) const
{
    // Equivalent to C# Items.FirstOrDefault(i => i.ID == sid)
    for (auto* currentItem : m_items) {
        if (currentItem && currentItem->id() == sid) {
            item = currentItem;
            return true;
        }
    }
    
    item = nullptr;
    return false;
}

} // namespace Collections
} // namespace OTLib