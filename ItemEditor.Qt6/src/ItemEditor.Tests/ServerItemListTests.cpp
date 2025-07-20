#include <QtTest>
#include <QObject>
#include "ServerItemList.h"
#include "ServerItem.h"
#include "ItemValidator.h"

class ServerItemListTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Basic collection operations
    void testConstruction();
    void testCopyConstructor();
    void testAssignment();
    void testAddItem();
    void testRemoveItem();
    void testUpdateItem();
    void testFindItem();
    void testClear();
    
    // Bulk operations
    void testAddItems();
    void testRemoveItems();
    void testUpdateItems();
    void testGetItems();
    
    // Search and filtering
    void testFindItemsByName();
    void testFindItemsByType();
    void testFindItemsByFlags();
    void testFindItemsInRange();
    void testFindItemsWithClientData();
    void testFindCustomItems();
    void testFindItemsWithPredicate();
    
    // Sorting
    void testSortById();
    void testSortByName();
    void testSortByType();
    void testSortByClientId();
    void testSortWithCustomComparator();
    
    // Statistics and counts
    void testGetItemCount();
    void testGetItemCountByType();
    void testGetCustomItemCount();
    void testGetItemsWithClientDataCount();
    void testGetItemCountsByType();
    
    // Range management
    void testUpdateItemRange();
    void testIsValidItemId();
    void testGetNextAvailableId();
    void testGetAvailableIds();
    void testGetUsedIds();
    void testGetUnusedIdsInRange();
    
    // Validation
    void testValidateCollection();
    void testGetValidationErrors();
    void testHasValidItemRange();
    void testHasDuplicateIds();
    void testGetDuplicateIds();
    
    // Advanced operations
    void testCompact();
    void testDefragment();
    void testSerialization();
    void testDeserialization();
    
    // Change tracking
    void testHasChanges();
    void testMarkAsModified();
    void testClearModified();
    void testGetModifiedItemIds();
    
    // Comparison and merging
    void testIsEqual();
    void testGetDifferences();
    void testMergeFrom();
    
    // Performance optimization
    void testBuildIndex();
    void testClearIndex();
    void testIndexedLookup();
    
    // Edge cases
    void testEmptyCollection();
    void testLargeCollection();
    void testDuplicateHandling();
    void testInvalidOperations();

private:
    ServerItem createTestItem(ItemId id, const QString& name = QString(), ServerItemType type = ServerItemType::Ground);
    ServerItemList createTestCollection(int itemCount = 10);
    void verifyCollectionIntegrity(const ServerItemList& list);
};

void ServerItemListTests::initTestCase()
{
    // Initialize test environment
}

void ServerItemListTests::cleanupTestCase()
{
    // Cleanup test environment
}

void ServerItemListTests::testConstruction()
{
    ServerItemList list;
    
    QCOMPARE(list.size(), 0);
    QVERIFY(list.isEmpty());
    QCOMPARE(list.versionInfo.majorVersion, static_cast<quint32>(0));
    QCOMPARE(list.versionInfo.minorVersion, static_cast<quint32>(0));
    QCOMPARE(list.versionInfo.buildNumber, static_cast<quint32>(0));
    QCOMPARE(list.versionInfo.clientVersion, static_cast<quint32>(0));
    QCOMPARE(list.itemRange.minId, static_cast<ItemId>(0));
    QCOMPARE(list.itemRange.maxId, static_cast<ItemId>(0));
    QVERIFY(!list.hasChanges());
    QCOMPARE(list.modifiedBy, QString("System"));
}

void ServerItemListTests::testCopyConstructor()
{
    ServerItemList original = createTestCollection(5);
    original.versionInfo.clientVersion = 1000;
    original.markAsModified();
    
    ServerItemList copy(original);
    
    QCOMPARE(copy.size(), original.size());
    QCOMPARE(copy.versionInfo.clientVersion, original.versionInfo.clientVersion);
    QVERIFY(copy.isEqual(original));
}

void ServerItemListTests::testAssignment()
{
    ServerItemList original = createTestCollection(5);
    ServerItemList assigned;
    
    assigned = original;
    
    QCOMPARE(assigned.size(), original.size());
    QVERIFY(assigned.isEqual(original));
}

void ServerItemListTests::testAddItem()
{
    ServerItemList list;
    ServerItem item = createTestItem(100, "Test Item");
    
    QVERIFY(list.addItem(item));
    QCOMPARE(list.size(), 1);
    QVERIFY(list.hasChanges());
    QCOMPARE(list.itemRange.minId, static_cast<ItemId>(100));
    QCOMPARE(list.itemRange.maxId, static_cast<ItemId>(100));
    
    // Test duplicate ID rejection
    ServerItem duplicate = createTestItem(100, "Duplicate");
    QVERIFY(!list.addItem(duplicate));
    QCOMPARE(list.size(), 1);
}

void ServerItemListTests::testRemoveItem()
{
    ServerItemList list = createTestCollection(5);
    int originalSize = list.size();
    
    QVERIFY(list.removeItem(1));
    QCOMPARE(list.size(), originalSize - 1);
    QVERIFY(list.findItem(1) == nullptr);
    QVERIFY(list.hasChanges());
    
    // Test removing non-existent item
    QVERIFY(!list.removeItem(999));
    QCOMPARE(list.size(), originalSize - 1);
}

void ServerItemListTests::testUpdateItem()
{
    ServerItemList list = createTestCollection(5);
    ServerItem updatedItem = createTestItem(1, "Updated Item", ServerItemType::Weapon);
    
    QVERIFY(list.updateItem(updatedItem));
    
    const ServerItem* found = list.findItem(1);
    QVERIFY(found != nullptr);
    QCOMPARE(found->name, QString("Updated Item"));
    QCOMPARE(found->type, ServerItemType::Weapon);
    QVERIFY(list.hasChanges());
    
    // Test updating non-existent item
    ServerItem nonExistent = createTestItem(999, "Non-existent");
    QVERIFY(!list.updateItem(nonExistent));
}

void ServerItemListTests::testFindItem()
{
    ServerItemList list = createTestCollection(5);
    
    // Test finding existing item
    const ServerItem* found = list.findItem(3);
    QVERIFY(found != nullptr);
    QCOMPARE(found->id, static_cast<ItemId>(3));
    
    // Test finding non-existent item
    const ServerItem* notFound = list.findItem(999);
    QVERIFY(notFound == nullptr);
    
    // Test mutable version
    ServerItem* mutableFound = list.findItem(3);
    QVERIFY(mutableFound != nullptr);
    mutableFound->name = "Modified";
    QCOMPARE(list.findItem(3)->name, QString("Modified"));
}

void ServerItemListTests::testClear()
{
    ServerItemList list = createTestCollection(10);
    
    list.clear();
    
    QCOMPARE(list.size(), 0);
    QVERIFY(list.isEmpty());
    QCOMPARE(list.itemRange.minId, static_cast<ItemId>(0));
    QCOMPARE(list.itemRange.maxId, static_cast<ItemId>(0));
    QVERIFY(list.hasChanges());
}

void ServerItemListTests::testAddItems()
{
    ServerItemList list;
    QList<ServerItem> items;
    
    for (int i = 1; i <= 5; ++i) {
        items.append(createTestItem(i, QString("Item %1").arg(i)));
    }
    
    list.addItems(items);
    
    QCOMPARE(list.size(), 5);
    QVERIFY(list.hasChanges());
    
    for (int i = 1; i <= 5; ++i) {
        QVERIFY(list.findItem(i) != nullptr);
    }
}

void ServerItemListTests::testRemoveItems()
{
    ServerItemList list = createTestCollection(10);
    QList<ItemId> idsToRemove = {2, 4, 6, 8};
    
    list.removeItems(idsToRemove);
    
    QCOMPARE(list.size(), 6);
    
    for (ItemId id : idsToRemove) {
        QVERIFY(list.findItem(id) == nullptr);
    }
}

void ServerItemListTests::testUpdateItems()
{
    ServerItemList list = createTestCollection(5);
    QList<ServerItem> updatedItems;
    
    for (int i = 1; i <= 3; ++i) {
        ServerItem item = createTestItem(i, QString("Updated %1").arg(i), ServerItemType::Weapon);
        updatedItems.append(item);
    }
    
    list.updateItems(updatedItems);
    
    for (int i = 1; i <= 3; ++i) {
        const ServerItem* item = list.findItem(i);
        QVERIFY(item != nullptr);
        QCOMPARE(item->name, QString("Updated %1").arg(i));
        QCOMPARE(item->type, ServerItemType::Weapon);
    }
}

void ServerItemListTests::testGetItems()
{
    ServerItemList list = createTestCollection(10);
    QList<ItemId> requestedIds = {2, 4, 6, 999}; // Include non-existent ID
    
    QList<ServerItem> result = list.getItems(requestedIds);
    
    QCOMPARE(result.size(), 3); // Should only return existing items
    QCOMPARE(result[0].id, static_cast<ItemId>(2));
    QCOMPARE(result[1].id, static_cast<ItemId>(4));
    QCOMPARE(result[2].id, static_cast<ItemId>(6));
}

void ServerItemListTests::testFindItemsByName()
{
    ServerItemList list;
    list.addItem(createTestItem(1, "Test Item"));
    list.addItem(createTestItem(2, "Another Test"));
    list.addItem(createTestItem(3, "Different Name"));
    
    // Test partial match
    QList<ServerItem> results = list.findItemsByName("Test", false);
    QCOMPARE(results.size(), 2);
    
    // Test exact match
    results = list.findItemsByName("Test Item", true);
    QCOMPARE(results.size(), 1);
    QCOMPARE(results[0].id, static_cast<ItemId>(1));
}

void ServerItemListTests::testFindItemsByType()
{
    ServerItemList list;
    list.addItem(createTestItem(1, "Ground", ServerItemType::Ground));
    list.addItem(createTestItem(2, "Weapon", ServerItemType::Weapon));
    list.addItem(createTestItem(3, "Another Ground", ServerItemType::Ground));
    
    QList<ServerItem> results = list.findItemsByType(ServerItemType::Ground);
    QCOMPARE(results.size(), 2);
    
    results = list.findItemsByType(ServerItemType::Weapon);
    QCOMPARE(results.size(), 1);
    QCOMPARE(results[0].id, static_cast<ItemId>(2));
}

void ServerItemListTests::testFindItemsByFlags()
{
    ServerItemList list;
    
    ServerItem item1 = createTestItem(1, "Pickupable");
    item1.flags = static_cast<quint32>(ItemFlag::Pickupable);
    list.addItem(item1);
    
    ServerItem item2 = createTestItem(2, "Stackable");
    item2.flags = static_cast<quint32>(ItemFlag::Stackable);
    list.addItem(item2);
    
    ServerItem item3 = createTestItem(3, "Both");
    item3.flags = static_cast<quint32>(ItemFlag::Pickupable) | static_cast<quint32>(ItemFlag::Stackable);
    list.addItem(item3);
    
    // Test any flag match
    QList<ServerItem> results = list.findItemsByFlags(static_cast<quint32>(ItemFlag::Pickupable), false);
    QCOMPARE(results.size(), 2);
    
    // Test all flags match
    results = list.findItemsByFlags(static_cast<quint32>(ItemFlag::Pickupable) | static_cast<quint32>(ItemFlag::Stackable), true);
    QCOMPARE(results.size(), 1);
    QCOMPARE(results[0].id, static_cast<ItemId>(3));
}

void ServerItemListTests::testFindItemsInRange()
{
    ServerItemList list = createTestCollection(10);
    
    QList<ServerItem> results = list.findItemsInRange(3, 7);
    QCOMPARE(results.size(), 5);
    
    for (const ServerItem& item : results) {
        QVERIFY(item.id >= 3 && item.id <= 7);
    }
}

void ServerItemListTests::testFindItemsWithClientData()
{
    ServerItemList list;
    
    ServerItem item1 = createTestItem(1, "With Client Data");
    item1.hasClientData = true;
    list.addItem(item1);
    
    ServerItem item2 = createTestItem(2, "Without Client Data");
    item2.hasClientData = false;
    list.addItem(item2);
    
    QList<ServerItem> results = list.findItemsWithClientData();
    QCOMPARE(results.size(), 1);
    QCOMPARE(results[0].id, static_cast<ItemId>(1));
}

void ServerItemListTests::testFindCustomItems()
{
    ServerItemList list;
    
    ServerItem item1 = createTestItem(1, "Custom Item");
    item1.isCustomCreated = true;
    list.addItem(item1);
    
    ServerItem item2 = createTestItem(2, "Regular Item");
    item2.isCustomCreated = false;
    list.addItem(item2);
    
    QList<ServerItem> results = list.findCustomItems();
    QCOMPARE(results.size(), 1);
    QCOMPARE(results[0].id, static_cast<ItemId>(1));
}

void ServerItemListTests::testFindItemsWithPredicate()
{
    ServerItemList list = createTestCollection(10);
    
    // Find items with even IDs
    QList<ServerItem> results = list.findItems([](const ServerItem& item) {
        return item.id % 2 == 0;
    });
    
    QCOMPARE(results.size(), 5);
    for (const ServerItem& item : results) {
        QVERIFY(item.id % 2 == 0);
    }
    
    // Find item IDs only
    QList<ItemId> ids = list.findItemIds([](const ServerItem& item) {
        return item.id > 5;
    });
    
    QCOMPARE(ids.size(), 5);
    for (ItemId id : ids) {
        QVERIFY(id > 5);
    }
}

void ServerItemListTests::testSortById()
{
    ServerItemList list;
    list.addItem(createTestItem(5, "Five"));
    list.addItem(createTestItem(2, "Two"));
    list.addItem(createTestItem(8, "Eight"));
    list.addItem(createTestItem(1, "One"));
    
    list.sortItems(ServerItemList::SortBy::Id, ServerItemList::SortOrder::Ascending);
    
    QCOMPARE(list[0].id, static_cast<ItemId>(1));
    QCOMPARE(list[1].id, static_cast<ItemId>(2));
    QCOMPARE(list[2].id, static_cast<ItemId>(5));
    QCOMPARE(list[3].id, static_cast<ItemId>(8));
    
    list.sortItems(ServerItemList::SortBy::Id, ServerItemList::SortOrder::Descending);
    
    QCOMPARE(list[0].id, static_cast<ItemId>(8));
    QCOMPARE(list[1].id, static_cast<ItemId>(5));
    QCOMPARE(list[2].id, static_cast<ItemId>(2));
    QCOMPARE(list[3].id, static_cast<ItemId>(1));
}

void ServerItemListTests::testSortByName()
{
    ServerItemList list;
    list.addItem(createTestItem(1, "Zebra"));
    list.addItem(createTestItem(2, "Apple"));
    list.addItem(createTestItem(3, "Banana"));
    
    list.sortItems(ServerItemList::SortBy::Name, ServerItemList::SortOrder::Ascending);
    
    QCOMPARE(list[0].name, QString("Apple"));
    QCOMPARE(list[1].name, QString("Banana"));
    QCOMPARE(list[2].name, QString("Zebra"));
}

void ServerItemListTests::testSortByType()
{
    ServerItemList list;
    list.addItem(createTestItem(1, "Weapon", ServerItemType::Weapon));
    list.addItem(createTestItem(2, "Ground", ServerItemType::Ground));
    list.addItem(createTestItem(3, "Container", ServerItemType::Container));
    
    list.sortItems(ServerItemList::SortBy::Type, ServerItemList::SortOrder::Ascending);
    
    QCOMPARE(list[0].type, ServerItemType::Ground);
    QCOMPARE(list[1].type, ServerItemType::Container);
    QCOMPARE(list[2].type, ServerItemType::Weapon);
}

void ServerItemListTests::testSortByClientId()
{
    ServerItemList list;
    
    ServerItem item1 = createTestItem(1, "Item1");
    item1.clientId = 300;
    list.addItem(item1);
    
    ServerItem item2 = createTestItem(2, "Item2");
    item2.clientId = 100;
    list.addItem(item2);
    
    ServerItem item3 = createTestItem(3, "Item3");
    item3.clientId = 200;
    list.addItem(item3);
    
    list.sortItems(ServerItemList::SortBy::ClientId, ServerItemList::SortOrder::Ascending);
    
    QCOMPARE(list[0].clientId, static_cast<ClientId>(100));
    QCOMPARE(list[1].clientId, static_cast<ClientId>(200));
    QCOMPARE(list[2].clientId, static_cast<ClientId>(300));
}

void ServerItemListTests::testSortWithCustomComparator()
{
    ServerItemList list = createTestCollection(5);
    
    // Sort by ID descending using custom comparator
    list.sortItems([](const ServerItem& a, const ServerItem& b) {
        return a.id > b.id;
    });
    
    for (int i = 0; i < list.size() - 1; ++i) {
        QVERIFY(list[i].id > list[i + 1].id);
    }
}

void ServerItemListTests::testGetItemCount()
{
    ServerItemList list = createTestCollection(15);
    QCOMPARE(list.getItemCount(), 15);
    QCOMPARE(list.getItemCount(), list.size());
}

void ServerItemListTests::testGetItemCountByType()
{
    ServerItemList list;
    list.addItem(createTestItem(1, "Ground1", ServerItemType::Ground));
    list.addItem(createTestItem(2, "Ground2", ServerItemType::Ground));
    list.addItem(createTestItem(3, "Weapon1", ServerItemType::Weapon));
    
    QCOMPARE(list.getItemCountByType(ServerItemType::Ground), 2);
    QCOMPARE(list.getItemCountByType(ServerItemType::Weapon), 1);
    QCOMPARE(list.getItemCountByType(ServerItemType::Container), 0);
}

void ServerItemListTests::testGetCustomItemCount()
{
    ServerItemList list;
    
    ServerItem custom1 = createTestItem(1, "Custom1");
    custom1.isCustomCreated = true;
    list.addItem(custom1);
    
    ServerItem custom2 = createTestItem(2, "Custom2");
    custom2.isCustomCreated = true;
    list.addItem(custom2);
    
    list.addItem(createTestItem(3, "Regular"));
    
    QCOMPARE(list.getCustomItemCount(), 2);
}

void ServerItemListTests::testGetItemsWithClientDataCount()
{
    ServerItemList list;
    
    ServerItem withData1 = createTestItem(1, "WithData1");
    withData1.hasClientData = true;
    list.addItem(withData1);
    
    ServerItem withData2 = createTestItem(2, "WithData2");
    withData2.hasClientData = true;
    list.addItem(withData2);
    
    list.addItem(createTestItem(3, "WithoutData"));
    
    QCOMPARE(list.getItemsWithClientDataCount(), 2);
}

void ServerItemListTests::testGetItemCountsByType()
{
    ServerItemList list;
    list.addItem(createTestItem(1, "Ground1", ServerItemType::Ground));
    list.addItem(createTestItem(2, "Ground2", ServerItemType::Ground));
    list.addItem(createTestItem(3, "Weapon1", ServerItemType::Weapon));
    list.addItem(createTestItem(4, "Container1", ServerItemType::Container));
    
    QHash<ServerItemType, int> counts = list.getItemCountsByType();
    
    QCOMPARE(counts[ServerItemType::Ground], 2);
    QCOMPARE(counts[ServerItemType::Weapon], 1);
    QCOMPARE(counts[ServerItemType::Container], 1);
    QVERIFY(!counts.contains(ServerItemType::Armor));
}

void ServerItemListTests::testUpdateItemRange()
{
    ServerItemList list;
    
    // Empty list
    list.updateItemRange();
    QCOMPARE(list.itemRange.minId, static_cast<ItemId>(0));
    QCOMPARE(list.itemRange.maxId, static_cast<ItemId>(0));
    
    // Add items
    list.addItem(createTestItem(5, "Five"));
    list.addItem(createTestItem(2, "Two"));
    list.addItem(createTestItem(8, "Eight"));
    
    QCOMPARE(list.itemRange.minId, static_cast<ItemId>(2));
    QCOMPARE(list.itemRange.maxId, static_cast<ItemId>(8));
}

void ServerItemListTests::testIsValidItemId()
{
    ServerItemList list;
    
    QVERIFY(list.isValidItemId(1));
    QVERIFY(list.isValidItemId(100));
    QVERIFY(list.isValidItemId(65535));
    
    QVERIFY(!list.isValidItemId(0));
    QVERIFY(!list.isValidItemId(65536));
}

void ServerItemListTests::testGetNextAvailableId()
{
    ServerItemList list;
    list.addItem(createTestItem(1, "One"));
    list.addItem(createTestItem(3, "Three"));
    list.addItem(createTestItem(5, "Five"));
    
    QCOMPARE(list.getNextAvailableId(), static_cast<ItemId>(2));
    QCOMPARE(list.getNextAvailableId(4), static_cast<ItemId>(4));
    QCOMPARE(list.getNextAvailableId(6), static_cast<ItemId>(6));
}

void ServerItemListTests::testGetAvailableIds()
{
    ServerItemList list;
    list.addItem(createTestItem(2, "Two"));
    list.addItem(createTestItem(4, "Four"));
    
    QList<ItemId> available = list.getAvailableIds(3);
    QCOMPARE(available.size(), 3);
    QCOMPARE(available[0], static_cast<ItemId>(1));
    QCOMPARE(available[1], static_cast<ItemId>(3));
    QCOMPARE(available[2], static_cast<ItemId>(5));
}

void ServerItemListTests::testGetUsedIds()
{
    ServerItemList list;
    list.addItem(createTestItem(5, "Five"));
    list.addItem(createTestItem(2, "Two"));
    list.addItem(createTestItem(8, "Eight"));
    
    QList<ItemId> used = list.getUsedIds();
    QCOMPARE(used.size(), 3);
    QCOMPARE(used[0], static_cast<ItemId>(2)); // Should be sorted
    QCOMPARE(used[1], static_cast<ItemId>(5));
    QCOMPARE(used[2], static_cast<ItemId>(8));
}

void ServerItemListTests::testGetUnusedIdsInRange()
{
    ServerItemList list;
    list.addItem(createTestItem(2, "Two"));
    list.addItem(createTestItem(4, "Four"));
    list.addItem(createTestItem(6, "Six"));
    list.updateItemRange(); // Range should be 2-6
    
    QList<ItemId> unused = list.getUnusedIdsInRange();
    QCOMPARE(unused.size(), 2);
    QCOMPARE(unused[0], static_cast<ItemId>(3));
    QCOMPARE(unused[1], static_cast<ItemId>(5));
}

void ServerItemListTests::testValidateCollection()
{
    ServerItemList list = createTestCollection(5);
    QVERIFY(list.validateCollection());
    
    // Add duplicate ID
    ServerItem duplicate = createTestItem(1, "Duplicate");
    list.append(duplicate); // Bypass addItem validation
    QVERIFY(!list.validateCollection());
}

void ServerItemListTests::testGetValidationErrors()
{
    ServerItemList list = createTestCollection(3);
    
    // Valid collection
    QStringList errors = list.getValidationErrors();
    QVERIFY(errors.isEmpty());
    
    // Add duplicate
    ServerItem duplicate = createTestItem(1, "Duplicate");
    list.append(duplicate);
    
    errors = list.getValidationErrors();
    QVERIFY(!errors.isEmpty());
    QVERIFY(errors.first().contains("Duplicate"));
}

void ServerItemListTests::testHasValidItemRange()
{
    ServerItemList list = createTestCollection(5);
    QVERIFY(list.hasValidItemRange());
    
    // Manually corrupt the range
    list.itemRange.minId = 999;
    QVERIFY(!list.hasValidItemRange());
}

void ServerItemListTests::testHasDuplicateIds()
{
    ServerItemList list = createTestCollection(3);
    QVERIFY(!list.hasDuplicateIds());
    
    // Add duplicate
    ServerItem duplicate = createTestItem(1, "Duplicate");
    list.append(duplicate);
    QVERIFY(list.hasDuplicateIds());
}

void ServerItemListTests::testGetDuplicateIds()
{
    ServerItemList list = createTestCollection(3);
    
    // Add duplicates
    list.append(createTestItem(1, "Duplicate1"));
    list.append(createTestItem(2, "Duplicate2"));
    
    QList<ItemId> duplicates = list.getDuplicateIds();
    QCOMPARE(duplicates.size(), 2);
    QVERIFY(duplicates.contains(1));
    QVERIFY(duplicates.contains(2));
}

void ServerItemListTests::testCompact()
{
    ServerItemList list;
    list.addItem(createTestItem(5, "Five"));
    list.addItem(createTestItem(2, "Two"));
    list.addItem(createTestItem(8, "Eight"));
    
    list.compact();
    
    // IDs should be reassigned sequentially
    QCOMPARE(list[0].id, static_cast<ItemId>(1));
    QCOMPARE(list[1].id, static_cast<ItemId>(2));
    QCOMPARE(list[2].id, static_cast<ItemId>(3));
    QCOMPARE(list.itemRange.minId, static_cast<ItemId>(1));
    QCOMPARE(list.itemRange.maxId, static_cast<ItemId>(3));
}

void ServerItemListTests::testDefragment()
{
    ServerItemList list;
    list.addItem(createTestItem(5, "Five"));
    list.addItem(createTestItem(2, "Two"));
    list.addItem(createTestItem(8, "Eight"));
    
    list.defragment();
    
    // Should be sorted by ID
    QCOMPARE(list[0].id, static_cast<ItemId>(2));
    QCOMPARE(list[1].id, static_cast<ItemId>(5));
    QCOMPARE(list[2].id, static_cast<ItemId>(8));
    QVERIFY(list.isIndexed());
}

void ServerItemListTests::testSerialization()
{
    ServerItemList list = createTestCollection(5);
    list.versionInfo.clientVersion = 1000;
    list.modifiedBy = "Test User";
    
    QByteArray data = list.serialize();
    QVERIFY(!data.isEmpty());
}

void ServerItemListTests::testDeserialization()
{
    ServerItemList original = createTestCollection(5);
    original.versionInfo.clientVersion = 1000;
    original.modifiedBy = "Test User";
    
    QByteArray data = original.serialize();
    
    ServerItemList deserialized;
    QVERIFY(deserialized.deserialize(data));
    
    QVERIFY(deserialized.isEqual(original));
    QCOMPARE(deserialized.versionInfo.clientVersion, original.versionInfo.clientVersion);
    QCOMPARE(deserialized.modifiedBy, original.modifiedBy);
}

void ServerItemListTests::testHasChanges()
{
    ServerItemList list = createTestCollection(3);
    list.clearModified();
    
    QVERIFY(!list.hasChanges());
    
    list.markAsModified();
    QVERIFY(list.hasChanges());
    
    list.clearModified();
    QVERIFY(!list.hasChanges());
    
    // Modify an item
    ServerItem* item = list.findItem(1);
    QVERIFY(item != nullptr);
    item->markAsModified();
    QVERIFY(list.hasChanges());
}

void ServerItemListTests::testMarkAsModified()
{
    ServerItemList list = createTestCollection(3);
    list.clearModified();
    
    QDateTime beforeModification = QDateTime::currentDateTime();
    list.markAsModified();
    
    QVERIFY(list.hasChanges());
    QVERIFY(list.lastModified >= beforeModification);
}

void ServerItemListTests::testClearModified()
{
    ServerItemList list = createTestCollection(3);
    list.markAsModified();
    
    // Mark individual items as modified
    for (ServerItem& item : list) {
        item.markAsModified();
    }
    
    QVERIFY(list.hasChanges());
    
    list.clearModified();
    
    QVERIFY(!list.hasChanges());
    for (const ServerItem& item : list) {
        QVERIFY(!item.hasChanges());
    }
}

void ServerItemListTests::testGetModifiedItemIds()
{
    ServerItemList list = createTestCollection(5);
    list.clearModified();
    
    // Modify specific items
    list.findItem(2)->markAsModified();
    list.findItem(4)->markAsModified();
    
    QList<ItemId> modifiedIds = list.getModifiedItemIds();
    QCOMPARE(modifiedIds.size(), 2);
    QVERIFY(modifiedIds.contains(2));
    QVERIFY(modifiedIds.contains(4));
}

void ServerItemListTests::testIsEqual()
{
    ServerItemList list1 = createTestCollection(5);
    ServerItemList list2 = createTestCollection(5);
    
    QVERIFY(list1.isEqual(list2));
    
    // Modify one item
    list2.findItem(3)->name = "Modified";
    QVERIFY(!list1.isEqual(list2));
    
    // Different sizes
    list2.removeItem(3);
    QVERIFY(!list1.isEqual(list2));
}

void ServerItemListTests::testGetDifferences()
{
    ServerItemList list1 = createTestCollection(5);
    ServerItemList list2 = createTestCollection(5);
    
    // Modify item in list2
    list2.findItem(3)->name = "Modified";
    
    // Add item to list2
    list2.addItem(createTestItem(10, "New Item"));
    
    // Remove item from list1
    list1.removeItem(5);
    
    QList<ItemId> differences = list1.getDifferences(list2);
    
    QVERIFY(differences.contains(3)); // Modified
    QVERIFY(differences.contains(5)); // Removed from list1
    QVERIFY(differences.contains(10)); // Added to list2
}

void ServerItemListTests::testMergeFrom()
{
    ServerItemList list1 = createTestCollection(3);
    ServerItemList list2;
    
    // Add overlapping and new items to list2
    list2.addItem(createTestItem(2, "Modified Item 2")); // Overlapping
    list2.addItem(createTestItem(4, "New Item 4")); // New
    list2.addItem(createTestItem(5, "New Item 5")); // New
    
    int originalSize = list1.size();
    
    // Merge without overwriting
    list1.mergeFrom(list2, false);
    
    QCOMPARE(list1.size(), originalSize + 2); // Should add 2 new items
    QCOMPARE(list1.findItem(2)->name, QString("Item 2")); // Should not be overwritten
    QVERIFY(list1.findItem(4) != nullptr);
    QVERIFY(list1.findItem(5) != nullptr);
    
    // Merge with overwriting
    list1.mergeFrom(list2, true);
    
    QCOMPARE(list1.findItem(2)->name, QString("Modified Item 2")); // Should be overwritten
}

void ServerItemListTests::testBuildIndex()
{
    ServerItemList list = createTestCollection(100);
    
    QVERIFY(!list.isIndexed());
    
    list.buildIndex();
    
    QVERIFY(list.isIndexed());
    
    // Test that indexed lookup works
    const ServerItem* item = list.findItem(50);
    QVERIFY(item != nullptr);
    QCOMPARE(item->id, static_cast<ItemId>(50));
}

void ServerItemListTests::testClearIndex()
{
    ServerItemList list = createTestCollection(10);
    list.buildIndex();
    
    QVERIFY(list.isIndexed());
    
    list.clearIndex();
    
    QVERIFY(!list.isIndexed());
}

void ServerItemListTests::testIndexedLookup()
{
    ServerItemList list = createTestCollection(1000);
    list.buildIndex();
    
    // Test multiple lookups with index
    for (int i = 1; i <= 1000; i += 100) {
        const ServerItem* item = list.findItem(i);
        QVERIFY(item != nullptr);
        QCOMPARE(item->id, static_cast<ItemId>(i));
    }
}

void ServerItemListTests::testEmptyCollection()
{
    ServerItemList list;
    
    QVERIFY(list.isEmpty());
    QCOMPARE(list.getItemCount(), 0);
    QVERIFY(list.getUsedIds().isEmpty());
    QVERIFY(list.getValidationErrors().isEmpty());
    QVERIFY(list.hasValidItemRange());
    QVERIFY(!list.hasDuplicateIds());
    
    // Operations on empty collection
    QVERIFY(!list.removeItem(1));
    QVERIFY(!list.updateItem(createTestItem(1, "Test")));
    QVERIFY(list.findItem(1) == nullptr);
}

void ServerItemListTests::testLargeCollection()
{
    const int itemCount = 10000;
    ServerItemList list = createTestCollection(itemCount);
    
    QCOMPARE(list.size(), itemCount);
    
    // Test performance operations
    list.buildIndex();
    QVERIFY(list.isIndexed());
    
    // Test lookup performance
    const ServerItem* item = list.findItem(itemCount / 2);
    QVERIFY(item != nullptr);
    
    // Test sorting
    list.sortItems(ServerItemList::SortBy::Id, ServerItemList::SortOrder::Descending);
    QCOMPARE(list[0].id, static_cast<ItemId>(itemCount));
    QCOMPARE(list[itemCount - 1].id, static_cast<ItemId>(1));
}

void ServerItemListTests::testDuplicateHandling()
{
    ServerItemList list;
    ServerItem item = createTestItem(1, "Test Item");
    
    QVERIFY(list.addItem(item));
    QVERIFY(!list.addItem(item)); // Should reject duplicate
    
    QCOMPARE(list.size(), 1);
}

void ServerItemListTests::testInvalidOperations()
{
    ServerItemList list;
    
    // Operations on non-existent items
    QVERIFY(!list.removeItem(999));
    QVERIFY(!list.updateItem(createTestItem(999, "Non-existent")));
    QVERIFY(list.findItem(999) == nullptr);
    
    // Invalid serialization data
    QByteArray invalidData("invalid");
    QVERIFY(!list.deserialize(invalidData));
    
    // Empty serialization data
    QVERIFY(!list.deserialize(QByteArray()));
}

ServerItem ServerItemListTests::createTestItem(ItemId id, const QString& name, ServerItemType type)
{
    ServerItem item;
    item.id = id;
    item.clientId = id;
    item.type = type;
    item.stackOrder = TileStackOrder::Ground;
    item.name = name.isEmpty() ? QString("Item %1").arg(id) : name;
    item.description = QString("Description for item %1").arg(id);
    item.width = 1;
    item.height = 1;
    item.layers = 1;
    item.patternX = 1;
    item.patternY = 1;
    item.patternZ = 1;
    item.frames = 1;
    item.flags = 0;
    item.speed = 100;
    item.lightLevel = 0;
    item.lightColor = 0;
    return item;
}

ServerItemList ServerItemListTests::createTestCollection(int itemCount)
{
    ServerItemList list;
    for (int i = 1; i <= itemCount; ++i) {
        list.addItem(createTestItem(i));
    }
    list.clearModified(); // Clear modification flags for clean testing
    return list;
}

void ServerItemListTests::verifyCollectionIntegrity(const ServerItemList& list)
{
    QVERIFY(list.validateCollection());
    QVERIFY(list.hasValidItemRange());
    QVERIFY(!list.hasDuplicateIds());
    
    // Verify all items are valid
    for (const ServerItem& item : list) {
        QVERIFY(item.isValid());
    }
}

QTEST_APPLESS_MAIN(ServerItemListTests)

#include "ServerItemListTests.moc"