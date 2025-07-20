#include <QtTest>
#include "ServerItem.h"
#include "ServerItemList.h"

/**
 * @brief Core functionality tests
 * 
 * Placeholder implementation - will be fully implemented in later tasks
 */
class CoreTests : public QObject
{
    Q_OBJECT

private slots:
    void testServerItemCreation();
    void testServerItemSerialization();
    void testServerItemListOperations();
};

void CoreTests::testServerItemCreation()
{
    ServerItem item;
    QCOMPARE(item.id, static_cast<ItemId>(0));
    QCOMPARE(item.type, ServerItemType::None);
    QVERIFY(!item.isValid());
}

void CoreTests::testServerItemSerialization()
{
    ServerItem item;
    item.id = 100;
    item.name = "Test Item";
    
    QByteArray data = item.serialize();
    QVERIFY(!data.isEmpty());
    
    ServerItem item2;
    QVERIFY(item2.deserialize(data));
    QCOMPARE(item2.id, item.id);
    QCOMPARE(item2.name, item.name);
}

void CoreTests::testServerItemListOperations()
{
    ServerItemList list;
    QCOMPARE(list.size(), 0);
    
    ServerItem item;
    item.id = 1;
    list.append(item);
    
    QCOMPARE(list.size(), 1);
}

QTEST_MAIN(CoreTests)
#include "CoreTests.moc"