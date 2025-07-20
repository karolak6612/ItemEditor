#include "ServerItemList.h"
#include <QDataStream>
#include <QIODevice>
#include <QDebug>
#include <algorithm>

ServerItemList::ServerItemList()
    : m_indexValid(false)
    , m_hasChanges(false)
{
    // Initialize version info
    versionInfo.majorVersion = 0;
    versionInfo.minorVersion = 0;
    versionInfo.buildNumber = 0;
    versionInfo.clientVersion = 0;
    
    // Initialize item range
    itemRange.minId = 0;
    itemRange.maxId = 0;
    
    lastModified = QDateTime::currentDateTime();
    modifiedBy = "System";
}

ServerItemList::ServerItemList(const ServerItemList& other)
    : QList<ServerItem>(other)
    , m_indexValid(false)
    , m_hasChanges(false)
{
    copyFrom(other);
}

ServerItemList& ServerItemList::operator=(const ServerItemList& other)
{
    if (this != &other) {
        QList<ServerItem>::operator=(other);
        copyFrom(other);
    }
    return *this;
}

ServerItemList::~ServerItemList()
{
}

bool ServerItemList::addItem(const ServerItem& item)
{
    // Check for duplicate ID
    if (findItem(item.id) != nullptr) {
        return false;
    }
    
    append(item);
    invalidateIndex();
    markAsModified();
    updateItemRange();
    
    return true;
}

bool ServerItemList::removeItem(ItemId id)
{
    int index = findItemIndex(id);
    if (index == -1) {
        return false;
    }
    
    removeAt(index);
    invalidateIndex();
    markAsModified();
    updateItemRange();
    
    return true;
}

bool ServerItemList::updateItem(const ServerItem& item)
{
    int index = findItemIndex(item.id);
    if (index == -1) {
        return false;
    }
    
    (*this)[index] = item;
    invalidateIndex();
    markAsModified();
    
    return true;
}

ServerItem* ServerItemList::findItem(ItemId id)
{
    int index = findItemIndex(id);
    return (index != -1) ? &(*this)[index] : nullptr;
}

const ServerItem* ServerItemList::findItem(ItemId id) const
{
    int index = findItemIndex(id);
    return (index != -1) ? &(*this)[index] : nullptr;
}

int ServerItemList::findItemIndex(ItemId id) const
{
    if (m_indexValid) {
        auto it = m_itemIndex.find(id);
        return (it != m_itemIndex.end()) ? it.value() : -1;
    }
    
    return findItemIndexInternal(id);
}

void ServerItemList::addItems(const QList<ServerItem>& items)
{
    for (const ServerItem& item : items) {
        if (findItem(item.id) == nullptr) {
            append(item);
        }
    }
    
    invalidateIndex();
    markAsModified();
    updateItemRange();
}

void ServerItemList::removeItems(const QList<ItemId>& ids)
{
    // Sort IDs in descending order to remove from end first
    QList<ItemId> sortedIds = ids;
    std::sort(sortedIds.begin(), sortedIds.end(), std::greater<ItemId>());
    
    for (ItemId id : sortedIds) {
        removeItem(id);
    }
}

void ServerItemList::updateItems(const QList<ServerItem>& items)
{
    for (const ServerItem& item : items) {
        updateItem(item);
    }
}

QList<ServerItem> ServerItemList::getItems(const QList<ItemId>& ids) const
{
    QList<ServerItem> result;
    for (ItemId id : ids) {
        const ServerItem* item = findItem(id);
        if (item) {
            result.append(*item);
        }
    }
    return result;
}

QList<ServerItem> ServerItemList::findItemsByName(const QString& name, bool exactMatch) const
{
    return findItems([name, exactMatch](const ServerItem& item) {
        if (exactMatch) {
            return item.name.compare(name, Qt::CaseInsensitive) == 0;
        } else {
            return item.name.contains(name, Qt::CaseInsensitive);
        }
    });
}

QList<ServerItem> ServerItemList::findItemsByType(ServerItemType type) const
{
    return findItems([type](const ServerItem& item) {
        return item.type == type;
    });
}

QList<ServerItem> ServerItemList::findItemsByFlags(quint32 flags, bool allFlags) const
{
    return findItems([flags, allFlags](const ServerItem& item) {
        if (allFlags) {
            return (item.flags & flags) == flags;
        } else {
            return (item.flags & flags) != 0;
        }
    });
}

QList<ServerItem> ServerItemList::findItemsInRange(ItemId minId, ItemId maxId) const
{
    return findItems([minId, maxId](const ServerItem& item) {
        return item.id >= minId && item.id <= maxId;
    });
}

QList<ServerItem> ServerItemList::findItemsWithClientData() const
{
    return findItems([](const ServerItem& item) {
        return item.hasClientData;
    });
}

QList<ServerItem> ServerItemList::findCustomItems() const
{
    return findItems([](const ServerItem& item) {
        return item.isCustomCreated;
    });
}

QList<ServerItem> ServerItemList::findItems(std::function<bool(const ServerItem&)> predicate) const
{
    QList<ServerItem> result;
    for (const ServerItem& item : *this) {
        if (predicate(item)) {
            result.append(item);
        }
    }
    return result;
}

QList<ItemId> ServerItemList::findItemIds(std::function<bool(const ServerItem&)> predicate) const
{
    QList<ItemId> result;
    for (const ServerItem& item : *this) {
        if (predicate(item)) {
            result.append(item.id);
        }
    }
    return result;
}

void ServerItemList::sortItems(SortBy sortBy, SortOrder order)
{
    switch (sortBy) {
        case SortBy::Id:
            std::sort(begin(), end(), [order](const ServerItem& a, const ServerItem& b) {
                return compareById(a, b, order);
            });
            break;
        case SortBy::Name:
            std::sort(begin(), end(), [order](const ServerItem& a, const ServerItem& b) {
                return compareByName(a, b, order);
            });
            break;
        case SortBy::Type:
            std::sort(begin(), end(), [order](const ServerItem& a, const ServerItem& b) {
                return compareByType(a, b, order);
            });
            break;
        case SortBy::ClientId:
            std::sort(begin(), end(), [order](const ServerItem& a, const ServerItem& b) {
                return compareByClientId(a, b, order);
            });
            break;
        case SortBy::LastModified:
            std::sort(begin(), end(), [order](const ServerItem& a, const ServerItem& b) {
                return compareByLastModified(a, b, order);
            });
            break;
    }
    
    invalidateIndex();
    markAsModified();
}

void ServerItemList::sortItems(std::function<bool(const ServerItem&, const ServerItem&)> comparator)
{
    std::sort(begin(), end(), comparator);
    invalidateIndex();
    markAsModified();
}

int ServerItemList::getItemCount() const
{
    return size();
}

int ServerItemList::getItemCountByType(ServerItemType type) const
{
    return std::count_if(begin(), end(), [type](const ServerItem& item) {
        return item.type == type;
    });
}

int ServerItemList::getCustomItemCount() const
{
    return std::count_if(begin(), end(), [](const ServerItem& item) {
        return item.isCustomCreated;
    });
}

int ServerItemList::getItemsWithClientDataCount() const
{
    return std::count_if(begin(), end(), [](const ServerItem& item) {
        return item.hasClientData;
    });
}

QHash<ServerItemType, int> ServerItemList::getItemCountsByType() const
{
    QHash<ServerItemType, int> counts;
    for (const ServerItem& item : *this) {
        counts[item.type]++;
    }
    return counts;
}

void ServerItemList::updateItemRange()
{
    if (isEmpty()) {
        itemRange.minId = 0;
        itemRange.maxId = 0;
        return;
    }
    
    ItemId minId = std::numeric_limits<ItemId>::max();
    ItemId maxId = 0;
    
    for (const ServerItem& item : *this) {
        if (item.id < minId) minId = item.id;
        if (item.id > maxId) maxId = item.id;
    }
    
    itemRange.minId = minId;
    itemRange.maxId = maxId;
}

bool ServerItemList::isValidItemId(ItemId id) const
{
    return id > 0 && id <= 65535;
}

ItemId ServerItemList::getNextAvailableId() const
{
    return getNextAvailableId(1);
}

ItemId ServerItemList::getNextAvailableId(ItemId startId) const
{
    for (ItemId id = startId; id <= 65535; ++id) {
        if (findItem(id) == nullptr) {
            return id;
        }
    }
    return 0; // No available ID found
}

QList<ItemId> ServerItemList::getAvailableIds(int count) const
{
    QList<ItemId> result;
    ItemId currentId = 1;
    
    while (result.size() < count && currentId <= 65535) {
        if (findItem(currentId) == nullptr) {
            result.append(currentId);
        }
        currentId++;
    }
    
    return result;
}

QList<ItemId> ServerItemList::getUsedIds() const
{
    QList<ItemId> result;
    for (const ServerItem& item : *this) {
        result.append(item.id);
    }
    std::sort(result.begin(), result.end());
    return result;
}

QList<ItemId> ServerItemList::getUnusedIdsInRange() const
{
    QList<ItemId> result;
    if (itemRange.minId == 0 || itemRange.maxId == 0) {
        return result;
    }
    
    for (ItemId id = itemRange.minId; id <= itemRange.maxId; ++id) {
        if (findItem(id) == nullptr) {
            result.append(id);
        }
    }
    
    return result;
}

bool ServerItemList::validateCollection() const
{
    return getValidationErrors().isEmpty();
}

QStringList ServerItemList::getValidationErrors() const
{
    QStringList errors;
    
    // Check for duplicate IDs
    QList<ItemId> duplicates = getDuplicateIds();
    if (!duplicates.isEmpty()) {
        errors << QString("Duplicate item IDs found: %1").arg(duplicates.size());
    }
    
    // Validate individual items
    for (const ServerItem& item : *this) {
        if (!item.isValid()) {
            errors << QString("Invalid item with ID %1").arg(item.id);
        }
    }
    
    // Check item range consistency
    if (!hasValidItemRange()) {
        errors << "Item range is inconsistent with actual items";
    }
    
    return errors;
}

bool ServerItemList::hasValidItemRange() const
{
    if (isEmpty()) {
        return itemRange.minId == 0 && itemRange.maxId == 0;
    }
    
    ItemId actualMin = std::numeric_limits<ItemId>::max();
    ItemId actualMax = 0;
    
    for (const ServerItem& item : *this) {
        if (item.id < actualMin) actualMin = item.id;
        if (item.id > actualMax) actualMax = item.id;
    }
    
    return itemRange.minId == actualMin && itemRange.maxId == actualMax;
}

bool ServerItemList::hasDuplicateIds() const
{
    return !getDuplicateIds().isEmpty();
}

QList<ItemId> ServerItemList::getDuplicateIds() const
{
    QHash<ItemId, int> idCounts;
    QList<ItemId> duplicates;
    
    for (const ServerItem& item : *this) {
        idCounts[item.id]++;
    }
    
    for (auto it = idCounts.begin(); it != idCounts.end(); ++it) {
        if (it.value() > 1) {
            duplicates.append(it.key());
        }
    }
    
    return duplicates;
}

void ServerItemList::clear()
{
    QList<ServerItem>::clear();
    m_itemIndex.clear();
    m_indexValid = false;
    itemRange.minId = 0;
    itemRange.maxId = 0;
    markAsModified();
}

void ServerItemList::reserve(int size)
{
    QList<ServerItem>::reserve(size);
    m_itemIndex.reserve(size);
}

void ServerItemList::compact()
{
    // Remove gaps by reassigning IDs sequentially
    sortItems(SortBy::Id, SortOrder::Ascending);
    
    ItemId newId = 1;
    for (ServerItem& item : *this) {
        item.id = newId++;
        item.markAsModified();
    }
    
    invalidateIndex();
    updateItemRange();
    markAsModified();
}

void ServerItemList::defragment()
{
    // Sort by ID for optimal access patterns
    sortItems(SortBy::Id, SortOrder::Ascending);
    buildIndex();
}

QByteArray ServerItemList::serialize() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    
    // Serialize version info
    stream << versionInfo.majorVersion << versionInfo.minorVersion 
           << versionInfo.buildNumber << versionInfo.clientVersion;
    
    // Serialize item range
    stream << itemRange.minId << itemRange.maxId;
    
    // Serialize metadata
    stream << lastModified << modifiedBy;
    
    // Serialize items
    stream << size();
    for (const ServerItem& item : *this) {
        stream << item.serialize();
    }
    
    return data;
}

bool ServerItemList::deserialize(const QByteArray& data)
{
    QDataStream stream(data);
    
    // Deserialize version info
    stream >> versionInfo.majorVersion >> versionInfo.minorVersion 
           >> versionInfo.buildNumber >> versionInfo.clientVersion;
    
    // Deserialize item range
    stream >> itemRange.minId >> itemRange.maxId;
    
    // Deserialize metadata
    stream >> lastModified >> modifiedBy;
    
    // Deserialize items
    int itemCount;
    stream >> itemCount;
    
    clear();
    reserve(itemCount);
    
    for (int i = 0; i < itemCount; ++i) {
        QByteArray itemData;
        stream >> itemData;
        
        ServerItem item;
        if (item.deserialize(itemData)) {
            append(item);
        } else {
            return false;
        }
    }
    
    invalidateIndex();
    m_hasChanges = false;
    
    return stream.status() == QDataStream::Ok;
}

bool ServerItemList::hasChanges() const
{
    if (m_hasChanges) {
        return true;
    }
    
    // Check if any individual item has changes
    for (const ServerItem& item : *this) {
        if (item.hasChanges()) {
            return true;
        }
    }
    
    return false;
}

void ServerItemList::markAsModified()
{
    m_hasChanges = true;
    lastModified = QDateTime::currentDateTime();
}

void ServerItemList::clearModified()
{
    m_hasChanges = false;
    for (ServerItem& item : *this) {
        item.clearModified();
    }
}

QList<ItemId> ServerItemList::getModifiedItemIds() const
{
    QList<ItemId> result;
    for (const ServerItem& item : *this) {
        if (item.hasChanges()) {
            result.append(item.id);
        }
    }
    return result;
}

bool ServerItemList::isEqual(const ServerItemList& other) const
{
    if (size() != other.size()) {
        return false;
    }
    
    if (versionInfo.majorVersion != other.versionInfo.majorVersion ||
        versionInfo.minorVersion != other.versionInfo.minorVersion ||
        versionInfo.buildNumber != other.versionInfo.buildNumber ||
        versionInfo.clientVersion != other.versionInfo.clientVersion) {
        return false;
    }
    
    // Compare all items (order independent)
    for (const ServerItem& item : *this) {
        const ServerItem* otherItem = other.findItem(item.id);
        if (!otherItem || otherItem->serialize() != item.serialize()) {
            return false;
        }
    }
    
    return true;
}

QList<ItemId> ServerItemList::getDifferences(const ServerItemList& other) const
{
    QList<ItemId> differences;
    
    // Find items that are different or missing in other
    for (const ServerItem& item : *this) {
        const ServerItem* otherItem = other.findItem(item.id);
        if (!otherItem || otherItem->serialize() != item.serialize()) {
            differences.append(item.id);
        }
    }
    
    // Find items that exist in other but not in this
    for (const ServerItem& otherItem : other) {
        if (findItem(otherItem.id) == nullptr) {
            differences.append(otherItem.id);
        }
    }
    
    // Remove duplicates and sort
    std::sort(differences.begin(), differences.end());
    differences.erase(std::unique(differences.begin(), differences.end()), differences.end());
    
    return differences;
}

void ServerItemList::mergeFrom(const ServerItemList& other, bool overwriteExisting)
{
    for (const ServerItem& otherItem : other) {
        ServerItem* existingItem = findItem(otherItem.id);
        if (existingItem) {
            if (overwriteExisting) {
                *existingItem = otherItem;
            }
        } else {
            addItem(otherItem);
        }
    }
    
    // Update version info if other is newer
    if (other.versionInfo.clientVersion > versionInfo.clientVersion) {
        versionInfo = other.versionInfo;
    }
}

void ServerItemList::buildIndex()
{
    m_itemIndex.clear();
    m_itemIndex.reserve(size());
    
    for (int i = 0; i < size(); ++i) {
        m_itemIndex[at(i).id] = i;
    }
    
    m_indexValid = true;
}

void ServerItemList::clearIndex()
{
    m_itemIndex.clear();
    m_indexValid = false;
}

bool ServerItemList::isIndexed() const
{
    return m_indexValid;
}

void ServerItemList::invalidateIndex()
{
    m_indexValid = false;
}

void ServerItemList::ensureIndexValid() const
{
    if (!m_indexValid) {
        const_cast<ServerItemList*>(this)->buildIndex();
    }
}

int ServerItemList::findItemIndexInternal(ItemId id) const
{
    for (int i = 0; i < size(); ++i) {
        if (at(i).id == id) {
            return i;
        }
    }
    return -1;
}

void ServerItemList::copyFrom(const ServerItemList& other)
{
    versionInfo = other.versionInfo;
    itemRange = other.itemRange;
    lastModified = other.lastModified;
    modifiedBy = other.modifiedBy;
    m_hasChanges = other.m_hasChanges;
    m_indexValid = false; // Force rebuild of index
}

void ServerItemList::updateStatistics()
{
    updateItemRange();
}

bool ServerItemList::compareById(const ServerItem& a, const ServerItem& b, SortOrder order)
{
    return (order == SortOrder::Ascending) ? (a.id < b.id) : (a.id > b.id);
}

bool ServerItemList::compareByName(const ServerItem& a, const ServerItem& b, SortOrder order)
{
    int result = a.name.compare(b.name, Qt::CaseInsensitive);
    return (order == SortOrder::Ascending) ? (result < 0) : (result > 0);
}

bool ServerItemList::compareByType(const ServerItem& a, const ServerItem& b, SortOrder order)
{
    return (order == SortOrder::Ascending) ? (a.type < b.type) : (a.type > b.type);
}

bool ServerItemList::compareByClientId(const ServerItem& a, const ServerItem& b, SortOrder order)
{
    return (order == SortOrder::Ascending) ? (a.clientId < b.clientId) : (a.clientId > b.clientId);
}

bool ServerItemList::compareByLastModified(const ServerItem& a, const ServerItem& b, SortOrder order)
{
    return (order == SortOrder::Ascending) ? (a.lastModified < b.lastModified) : (a.lastModified > b.lastModified);
}