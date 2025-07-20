#include <QtTest>
#include <QObject>
#include "ServerItem.h"
#include "ClientItem.h"
#include "ItemValidator.h"

class ServerItemTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // ServerItem basic functionality tests
    void testServerItemConstruction();
    void testServerItemCopyConstructor();
    void testServerItemAssignment();
    void testServerItemSerialization();
    void testServerItemDeserialization();
    
    // Property validation tests
    void testPropertyValidation();
    void testPropertyGetSet();
    void testModificationTracking();
    
    // Validation tests
    void testBasicValidation();
    void testItemIdValidation();
    void testItemTypeValidation();
    void testItemNameValidation();
    void testDimensionValidation();
    void testFlagValidation();
    void testSpeedValidation();
    void testLightValidation();
    
    // Advanced validation tests
    void testWeaponValidation();
    void testContainerValidation();
    void testFluidValidation();
    void testItemConsistency();
    
    // ClientItem tests
    void testClientItemConstruction();
    void testClientItemValidation();
    void testSpriteSignatureCalculation();
    void testSpriteHashCalculation();
    void testClientItemCompatibility();
    
    // Edge cases and error conditions
    void testInvalidData();
    void testBoundaryValues();
    void testErrorMessages();

private:
    ServerItem createValidServerItem();
    ClientItem createValidClientItem();
    void verifyItemEquality(const ServerItem& item1, const ServerItem& item2);
};

void ServerItemTests::initTestCase()
{
    // Initialize test environment
}

void ServerItemTests::cleanupTestCase()
{
    // Cleanup test environment
}

void ServerItemTests::testServerItemConstruction()
{
    ServerItem item;
    
    // Test default values
    QCOMPARE(item.id, static_cast<ItemId>(0));
    QCOMPARE(item.clientId, static_cast<ClientId>(0));
    QCOMPARE(item.type, ServerItemType::None);
    QCOMPARE(item.stackOrder, TileStackOrder::None);
    QCOMPARE(item.name, QString("New Item"));
    QCOMPARE(item.width, static_cast<quint8>(1));
    QCOMPARE(item.height, static_cast<quint8>(1));
    QCOMPARE(item.flags, static_cast<quint32>(0));
    QCOMPARE(item.isCustomCreated, false);
    QCOMPARE(item.hasClientData, false);
    QCOMPARE(item.hasChanges(), false);
}

void ServerItemTests::testServerItemCopyConstructor()
{
    ServerItem original = createValidServerItem();
    original.markAsModified();
    
    ServerItem copy(original);
    
    verifyItemEquality(original, copy);
    QCOMPARE(copy.hasChanges(), original.hasChanges());
}

void ServerItemTests::testServerItemAssignment()
{
    ServerItem original = createValidServerItem();
    ServerItem assigned;
    
    assigned = original;
    
    verifyItemEquality(original, assigned);
}

void ServerItemTests::testServerItemSerialization()
{
    ServerItem original = createValidServerItem();
    
    QByteArray data = original.serialize();
    QVERIFY(!data.isEmpty());
    
    ServerItem deserialized;
    QVERIFY(deserialized.deserialize(data));
    
    verifyItemEquality(original, deserialized);
}

void ServerItemTests::testServerItemDeserialization()
{
    ServerItem item = createValidServerItem();
    QByteArray data = item.serialize();
    
    ServerItem newItem;
    bool result = newItem.deserialize(data);
    
    QVERIFY(result);
    verifyItemEquality(item, newItem);
}

void ServerItemTests::testPropertyValidation()
{
    ServerItem item;
    
    // Test valid property values
    QVERIFY(item.validateProperty("id", 100));
    QVERIFY(item.validateProperty("name", "Test Item"));
    QVERIFY(item.validateProperty("width", 2));
    QVERIFY(item.validateProperty("height", 3));
    
    // Test invalid property values
    QVERIFY(!item.validateProperty("id", 0));
    QVERIFY(!item.validateProperty("id", 70000));
    QVERIFY(!item.validateProperty("name", ""));
    QVERIFY(!item.validateProperty("width", 0));
    QVERIFY(!item.validateProperty("height", 15));
}

void ServerItemTests::testPropertyGetSet()
{
    ServerItem item;
    
    // Test property setting and getting
    QVERIFY(item.setProperty("id", 123));
    QCOMPARE(item.getProperty("id").toUInt(), static_cast<quint32>(123));
    QCOMPARE(item.id, static_cast<ItemId>(123));
    
    QVERIFY(item.setProperty("name", "Test Name"));
    QCOMPARE(item.getProperty("name").toString(), QString("Test Name"));
    QCOMPARE(item.name, QString("Test Name"));
    
    QVERIFY(item.setProperty("width", 5));
    QCOMPARE(item.getProperty("width").toUInt(), static_cast<quint32>(5));
    QCOMPARE(item.width, static_cast<quint8>(5));
    
    // Test invalid property setting
    QVERIFY(!item.setProperty("id", 0));
    QCOMPARE(item.id, static_cast<ItemId>(123)); // Should remain unchanged
}

void ServerItemTests::testModificationTracking()
{
    ServerItem item;
    
    QVERIFY(!item.hasChanges());
    
    item.markAsModified();
    QVERIFY(item.hasChanges());
    
    item.clearModified();
    QVERIFY(!item.hasChanges());
    
    // Test automatic modification tracking
    item.setProperty("name", "Modified Name");
    QVERIFY(item.hasChanges());
}

void ServerItemTests::testBasicValidation()
{
    ServerItem item;
    
    // Invalid item (default state)
    QVERIFY(!ItemValidator::validateItem(item));
    
    // Make item valid
    item.id = 100;
    item.type = ServerItemType::Ground;
    item.name = "Valid Item";
    
    QVERIFY(ItemValidator::validateItem(item));
    QVERIFY(item.isValid());
}

void ServerItemTests::testItemIdValidation()
{
    // Test valid IDs
    QVERIFY(ItemValidator::validateItemId(1));
    QVERIFY(ItemValidator::validateItemId(100));
    QVERIFY(ItemValidator::validateItemId(65535));
    
    // Test invalid IDs
    QVERIFY(!ItemValidator::validateItemId(0));
    QVERIFY(!ItemValidator::validateItemId(65536));
}

void ServerItemTests::testItemTypeValidation()
{
    QVERIFY(ItemValidator::validateItemType(ServerItemType::Ground));
    QVERIFY(ItemValidator::validateItemType(ServerItemType::Weapon));
    QVERIFY(ItemValidator::validateItemType(ServerItemType::Container));
    QVERIFY(!ItemValidator::validateItemType(static_cast<ServerItemType>(255)));
}

void ServerItemTests::testItemNameValidation()
{
    QVERIFY(ItemValidator::validateItemName("Valid Name"));
    QVERIFY(ItemValidator::validateItemName("A"));
    
    QVERIFY(!ItemValidator::validateItemName(""));
    
    // Test maximum length
    QString longName(256, 'A');
    QVERIFY(!ItemValidator::validateItemName(longName));
    
    QString maxName(255, 'A');
    QVERIFY(ItemValidator::validateItemName(maxName));
}

void ServerItemTests::testDimensionValidation()
{
    QVERIFY(ItemValidator::validateDimensions(1, 1));
    QVERIFY(ItemValidator::validateDimensions(5, 3));
    QVERIFY(ItemValidator::validateDimensions(10, 10));
    
    QVERIFY(!ItemValidator::validateDimensions(0, 1));
    QVERIFY(!ItemValidator::validateDimensions(1, 0));
    QVERIFY(!ItemValidator::validateDimensions(11, 5));
    QVERIFY(!ItemValidator::validateDimensions(5, 11));
}

void ServerItemTests::testFlagValidation()
{
    // Test valid flag combinations
    QVERIFY(ItemValidator::validateFlags(0));
    QVERIFY(ItemValidator::validateFlags(static_cast<quint32>(ItemFlag::Pickupable)));
    QVERIFY(ItemValidator::validateFlags(static_cast<quint32>(ItemFlag::Stackable)));
    
    // Test invalid flag combinations
    quint32 conflictingFlags = static_cast<quint32>(ItemFlag::Unpassable) | 
                              static_cast<quint32>(ItemFlag::Pickupable);
    QVERIFY(!ItemValidator::validateFlags(conflictingFlags));
    
    quint32 conflictingFlags2 = static_cast<quint32>(ItemFlag::Stackable) | 
                               static_cast<quint32>(ItemFlag::MultiUse);
    QVERIFY(!ItemValidator::validateFlags(conflictingFlags2));
}

void ServerItemTests::testSpeedValidation()
{
    QVERIFY(ItemValidator::validateSpeed(0));
    QVERIFY(ItemValidator::validateSpeed(1000));
    QVERIFY(ItemValidator::validateSpeed(65535));
    
    // Speed validation should always pass for valid range
    QVERIFY(ItemValidator::validateSpeed(65536)); // This might fail depending on implementation
}

void ServerItemTests::testLightValidation()
{
    QVERIFY(ItemValidator::validateLight(0, 0));
    QVERIFY(ItemValidator::validateLight(255, 65535));
    QVERIFY(ItemValidator::validateLight(100, 1000));
    
    QVERIFY(!ItemValidator::validateLight(256, 0));
}

void ServerItemTests::testWeaponValidation()
{
    ServerItem weapon;
    weapon.id = 100;
    weapon.type = ServerItemType::Weapon;
    weapon.name = "Test Weapon";
    weapon.attack = 50;
    weapon.flags = static_cast<quint32>(ItemFlag::Pickupable);
    
    QVERIFY(ItemValidator::validateItem(weapon));
    
    // Test invalid weapon (no attack)
    weapon.attack = 0;
    QVERIFY(!ItemValidator::validateItem(weapon));
    
    // Test invalid weapon (not pickupable)
    weapon.attack = 50;
    weapon.flags = 0;
    QVERIFY(!ItemValidator::validateItem(weapon));
}

void ServerItemTests::testContainerValidation()
{
    ServerItem container;
    container.id = 200;
    container.type = ServerItemType::Container;
    container.name = "Test Container";
    container.containerSize = 20;
    container.flags = static_cast<quint32>(ItemFlag::Pickupable);
    
    QVERIFY(ItemValidator::validateItem(container));
    
    // Test invalid container (no size)
    container.containerSize = 0;
    QVERIFY(!ItemValidator::validateItem(container));
    
    // Test invalid container (wrong flags)
    container.containerSize = 20;
    container.flags = 0;
    QVERIFY(!ItemValidator::validateItem(container));
}

void ServerItemTests::testFluidValidation()
{
    ServerItem fluid;
    fluid.id = 300;
    fluid.type = ServerItemType::Fluid;
    fluid.name = "Test Fluid";
    fluid.fluidSource = 1;
    
    QVERIFY(ItemValidator::validateItem(fluid));
    
    // Test invalid fluid (no source)
    fluid.fluidSource = 0;
    QVERIFY(!ItemValidator::validateItem(fluid));
}

void ServerItemTests::testItemConsistency()
{
    ServerItem item = createValidServerItem();
    QVERIFY(ItemValidator::validateItemConsistency(item));
    
    // Test inconsistent item
    item.type = ServerItemType::Weapon;
    item.attack = 0; // Weapons should have attack
    QVERIFY(!ItemValidator::validateItemConsistency(item));
}

void ServerItemTests::testClientItemConstruction()
{
    ClientItem item;
    
    // Test client-specific defaults
    QCOMPARE(item.animationPhases, static_cast<quint8>(1));
    QCOMPARE(item.xDiv, static_cast<quint8>(1));
    QCOMPARE(item.yDiv, static_cast<quint8>(1));
    QCOMPARE(item.zDiv, static_cast<quint8>(1));
    QCOMPARE(item.animationSpeed, static_cast<quint16>(0));
    QCOMPARE(item.animationType, AnimationType::None);
    QCOMPARE(item.hasClientData, true);
}

void ServerItemTests::testClientItemValidation()
{
    ClientItem item = createValidClientItem();
    QVERIFY(ItemValidator::validateClientItem(item));
    
    // Test invalid client item
    item.animationPhases = 0;
    QVERIFY(!ItemValidator::validateClientItem(item));
    
    item.animationPhases = 1;
    item.xDiv = 0;
    QVERIFY(!ItemValidator::validateClientItem(item));
}

void ServerItemTests::testSpriteSignatureCalculation()
{
    ClientItem item = createValidClientItem();
    
    // Add some sprite data
    item.spriteList.append(QByteArray(1024, 0x55)); // Test pattern
    item.spriteList.append(QByteArray(1024, 0xAA)); // Different pattern
    
    item.calculateSpriteSignature();
    
    QCOMPARE(item.spriteSignature.size(), 2);
    QVERIFY(!item.spriteSignature[0].isEmpty());
    QVERIFY(!item.spriteSignature[1].isEmpty());
    
    // Test signature comparison
    ClientItem item2 = item;
    QVERIFY(item.compareSignature(item2, 0.95));
    
    // Modify sprite data and test difference
    item2.spriteList[0] = QByteArray(1024, 0x00);
    item2.calculateSpriteSignature();
    QVERIFY(!item.compareSignature(item2, 0.95));
}

void ServerItemTests::testSpriteHashCalculation()
{
    ClientItem item = createValidClientItem();
    
    // Add sprite data
    item.spriteList.append(QByteArray(1024, 0x55));
    
    item.calculateSpriteHash();
    QVERIFY(!item.spriteHash.isEmpty());
    QVERIFY(item.verifySpriteHash());
    
    // Modify sprite data and verify hash changes
    QByteArray originalHash = item.spriteHash;
    item.spriteList[0] = QByteArray(1024, 0xAA);
    item.calculateSpriteHash();
    QVERIFY(item.spriteHash != originalHash);
}

void ServerItemTests::testClientItemCompatibility()
{
    ServerItem serverItem = createValidServerItem();
    ClientItem clientItem = createValidClientItem();
    
    // Make items compatible
    serverItem.clientId = clientItem.id;
    serverItem.width = clientItem.width;
    serverItem.height = clientItem.height;
    serverItem.spriteHash = clientItem.spriteHash;
    
    QVERIFY(ItemValidator::validateItemCompatibility(serverItem, clientItem));
    
    // Test incompatible items
    serverItem.clientId = clientItem.id + 1;
    QVERIFY(!ItemValidator::validateItemCompatibility(serverItem, clientItem));
}

void ServerItemTests::testInvalidData()
{
    ServerItem item;
    
    // Test with completely invalid data
    QByteArray invalidData("invalid data");
    QVERIFY(!item.deserialize(invalidData));
    
    // Test with empty data
    QVERIFY(!item.deserialize(QByteArray()));
}

void ServerItemTests::testBoundaryValues()
{
    ServerItem item;
    
    // Test minimum values
    item.id = ItemValidator::getMinItemId();
    item.type = ServerItemType::Ground;
    item.name = "A";
    item.width = 1;
    item.height = 1;
    
    QVERIFY(ItemValidator::validateItem(item));
    
    // Test maximum values
    item.id = ItemValidator::getMaxItemId();
    item.name = QString(ItemValidator::getMaxNameLength(), 'A');
    item.width = ItemValidator::getMaxDimension();
    item.height = ItemValidator::getMaxDimension();
    item.speed = ItemValidator::getMaxSpeed();
    item.lightLevel = ItemValidator::getMaxLightLevel();
    
    QVERIFY(ItemValidator::validateItem(item));
}

void ServerItemTests::testErrorMessages()
{
    ServerItem item; // Invalid item
    
    QStringList errors = ItemValidator::getAllValidationErrors(item);
    QVERIFY(!errors.isEmpty());
    
    QString firstError = ItemValidator::getValidationError(item);
    QVERIFY(!firstError.isEmpty());
    QVERIFY(firstError.contains("[ERROR]"));
}

ServerItem ServerItemTests::createValidServerItem()
{
    ServerItem item;
    item.id = 100;
    item.clientId = 100;
    item.type = ServerItemType::Ground;
    item.stackOrder = TileStackOrder::Ground;
    item.name = "Test Item";
    item.description = "Test Description";
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

ClientItem ServerItemTests::createValidClientItem()
{
    ClientItem item;
    item.id = 100;
    item.clientId = 100;
    item.type = ServerItemType::Ground;
    item.stackOrder = TileStackOrder::Ground;
    item.name = "Test Client Item";
    item.description = "Test Description";
    item.width = 1;
    item.height = 1;
    item.layers = 1;
    item.patternX = 1;
    item.patternY = 1;
    item.patternZ = 1;
    item.frames = 1;
    item.animationPhases = 1;
    item.xDiv = 1;
    item.yDiv = 1;
    item.zDiv = 1;
    item.animationSpeed = 0;
    item.animationType = AnimationType::None;
    return item;
}

void ServerItemTests::verifyItemEquality(const ServerItem& item1, const ServerItem& item2)
{
    QCOMPARE(item1.id, item2.id);
    QCOMPARE(item1.clientId, item2.clientId);
    QCOMPARE(item1.previousClientId, item2.previousClientId);
    QCOMPARE(item1.type, item2.type);
    QCOMPARE(item1.stackOrder, item2.stackOrder);
    QCOMPARE(item1.name, item2.name);
    QCOMPARE(item1.description, item2.description);
    QCOMPARE(item1.article, item2.article);
    QCOMPARE(item1.plural, item2.plural);
    QCOMPARE(item1.spriteHash, item2.spriteHash);
    QCOMPARE(item1.width, item2.width);
    QCOMPARE(item1.height, item2.height);
    QCOMPARE(item1.layers, item2.layers);
    QCOMPARE(item1.patternX, item2.patternX);
    QCOMPARE(item1.patternY, item2.patternY);
    QCOMPARE(item1.patternZ, item2.patternZ);
    QCOMPARE(item1.frames, item2.frames);
    QCOMPARE(item1.flags, item2.flags);
    QCOMPARE(item1.speed, item2.speed);
    QCOMPARE(item1.lightLevel, item2.lightLevel);
    QCOMPARE(item1.lightColor, item2.lightColor);
    QCOMPARE(item1.isCustomCreated, item2.isCustomCreated);
    QCOMPARE(item1.hasClientData, item2.hasClientData);
}

QTEST_APPLESS_MAIN(ServerItemTests)

#include "ServerItemTests.moc"