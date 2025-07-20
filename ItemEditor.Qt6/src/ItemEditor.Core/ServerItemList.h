#pragma once

#include <QList>
#include <QHash>
#include <QStringList>
#include <QDateTime>
#include <functional>
#include "ServerItem.h"
#include "ItemTypes.h"

/**
 * @brief Collection of server items with advanced management capabilities
 * 
 * Provides identical functionality to the legacy system including
 * filtering, searching, sorting, and collection operations.
 */
class ServerItemList : public QList<ServerItem>
{
public:
    ServerItemList();
    ServerItemList(const ServerItemList& other);
    ServerItemList& operator=(const ServerItemList& other);
    ~ServerItemList();

    // Version information
    VersionInfo versionInfo;
    ItemRange itemRange;
    QDateTime lastModified;
    QString modifiedBy;
    
    // Collection management
    bool addItem(const ServerItem& item);
    bool removeItem(ItemId id);
    bool updateItem(const ServerItem& item);
    ServerItem* findItem(ItemId id);
    const ServerItem* findItem(ItemId id) const;
    int findItemIndex(ItemId id) const;
    
    // Bulk operations
    void addItems(const QList<ServerItem>& items);
    void removeItems(const QList<ItemId>& ids);
    void updateItems(const QList<ServerItem>& items);
    QList<ServerItem> getItems(const QList<ItemId>& ids) const;
    
    // Search and filtering
    QList<ServerItem> findItemsByName(const QString& name, bool exactMatch = false) const;
    QList<ServerItem> findItemsByType(ServerItemType type) const;
    QList<ServerItem> findItemsByFlags(quint32 flags, bool allFlags = true) const;
    QList<ServerItem> findItemsInRange(ItemId minId, ItemId maxId) const;
    QList<ServerItem> findItemsWithClientData() const;
    QList<ServerItem> findCustomItems() const;
    
    // Advanced search with predicates
    QList<ServerItem> findItems(std::function<bool(const ServerItem&)> predicate) const;
    QList<ItemId> findItemIds(std::function<bool(const ServerItem&)> predicate) const;
    
    // Sorting
    enum class SortOrder {
        Ascending,
        Descending
    };
    
    enum class SortBy {
        Id,
        Name,
        Type,
        ClientId,
        LastModified
    };
    
    void sortItems(SortBy sortBy, SortOrder order = SortOrder::Ascending);
    void sortItems(std::function<bool(const ServerItem&, const ServerItem&)> comparator);
    
    // Collection statistics
    int getItemCount() const;
    int getItemCountByType(ServerItemType type) const;
    int getCustomItemCount() const;
    int getItemsWithClientDataCount() const;
    QHash<ServerItemType, int> getItemCountsByType() const;
    
    // Range management
    void updateItemRange();
    bool isValidItemId(ItemId id) const;
    ItemId getNextAvailableId() const;
    ItemId getNextAvailableId(ItemId startId) const;
    QList<ItemId> getAvailableIds(int count) const;
    QList<ItemId> getUsedIds() const;
    QList<ItemId> getUnusedIdsInRange() const;
    
    // Validation
    bool validateCollection() const;
    QStringList getValidationErrors() const;
    bool hasValidItemRange() const;
    bool hasDuplicateIds() const;
    QList<ItemId> getDuplicateIds() const;
    
    // Collection operations
    void clear();
    void reserve(int size);
    void compact(); // Remove gaps in item IDs
    void defragment(); // Reorganize items for optimal access
    
    // Import/Export helpers
    QByteArray serialize() const;
    bool deserialize(const QByteArray& data);
    
    // Change tracking
    bool hasChanges() const;
    void markAsModified();
    void clearModified();
    QList<ItemId> getModifiedItemIds() const;
    
    // Comparison and merging
    bool isEqual(const ServerItemList& other) const;
    QList<ItemId> getDifferences(const ServerItemList& other) const;
    void mergeFrom(const ServerItemList& other, bool overwriteExisting = false);
    
    // Performance optimization
    void buildIndex(); // Build internal hash index for fast lookups
    void clearIndex(); // Clear internal index to save memory
    bool isIndexed() const;

private:
    // Internal data structures for performance
    mutable QHash<ItemId, int> m_itemIndex; // Fast ID to index mapping
    mutable bool m_indexValid;
    bool m_hasChanges;
    
    // Internal helper methods
    void invalidateIndex();
    void ensureIndexValid() const;
    int findItemIndexInternal(ItemId id) const;
    void copyFrom(const ServerItemList& other);
    void updateStatistics();
    
    // Sorting helpers
    static bool compareById(const ServerItem& a, const ServerItem& b, SortOrder order);
    static bool compareByName(const ServerItem& a, const ServerItem& b, SortOrder order);
    static bool compareByType(const ServerItem& a, const ServerItem& b, SortOrder order);
    static bool compareByClientId(const ServerItem& a, const ServerItem& b, SortOrder order);
    static bool compareByLastModified(const ServerItem& a, const ServerItem& b, SortOrder order);
};