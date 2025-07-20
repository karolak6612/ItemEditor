#include <QtTest>
#include <QObject>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include "OtbWriter.h"
#include "OtbReader.h"
#include "ServerItemList.h"
#include "ServerItem.h"

class OtbWriterTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Basic functionality tests
    void testConstruction();
    void testWriteToInvalidPath();
    void testWriteEmptyList();
    void testWriteValidList();
    
    // File operations tests
    void testWriteToData();
    void testWriteWithOptions();
    void testBackupCreation();
    void testAtomicWrite();
    
    // Error handling tests
    void testErrorHandling();
    void testProgressCallback();
    void testValidation();
    
    // Statistics tests
    void testStatistics();
    
    // Path validation tests
    void testCanWriteToPath();
    void testValidateOutputPath();
    
    // Advanced features tests
    void testBackupManagement();
    void testWriteOptionsValidation();
    
    // Integration tests
    void testWriteReadRoundTrip();
    void testByteIdenticalOutput();
    
    // Edge cases
    void testLargeFile();
    void testInvalidItems();
    void testFilePermissions();

private:
    QTemporaryDir* m_tempDir;
    ServerItemList createTestItemList(int itemCount = 5);
    ServerItem createValidItem(ItemId id);
    ServerItem createInvalidItem(ItemId id);
    void verifyFileExists(const QString& filePath);
    void verifyFileContent(const QString& filePath, const ServerItemList& expectedItems);
};

void OtbWriterTests::initTestCase()
{
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
}

void OtbWriterTests::cleanupTestCase()
{
    delete m_tempDir;
}

void OtbWriterTests::testConstruction()
{
    OtbWriter writer;
    
    QVERIFY(!writer.hasError());
    QVERIFY(writer.getLastError().isEmpty());
    QVERIFY(writer.getAllErrors().isEmpty());
    QCOMPARE(writer.getItemsWritten(), 0);
    QCOMPARE(writer.getItemsSkipped(), 0);
    QCOMPARE(writer.getBytesWritten(), static_cast<qint64>(0));
}

void OtbWriterTests::testWriteToInvalidPath()
{
    OtbWriter writer;
    ServerItemList items = createTestItemList(3);
    
    // Test with empty path
    bool result = writer.writeFile("", items);
    QVERIFY(!result);
    QVERIFY(writer.hasError());
    
    writer.clearErrors();
    
    // Test with invalid extension
    result = writer.writeFile("test.txt", items);
    QVERIFY(!result);
    QVERIFY(writer.hasError());
    
    writer.clearErrors();
    
    // Test with non-existent directory (should create it)
    QString validPath = m_tempDir->filePath("subdir/test.otb");
    result = writer.writeFile(validPath, items);
    QVERIFY(result);
    QVERIFY(!writer.hasError());
    verifyFileExists(validPath);
}

void OtbWriterTests::testWriteEmptyList()
{
    OtbWriter writer;
    ServerItemList emptyItems;
    QString filePath = m_tempDir->filePath("empty.otb");
    
    bool result = writer.writeFile(filePath, emptyItems);
    
    QVERIFY(result);
    QVERIFY(!writer.hasError());
    QCOMPARE(writer.getItemsWritten(), 0);
    QCOMPARE(writer.getItemsSkipped(), 0);
    QVERIFY(writer.getBytesWritten() > 0); // Should still have header
    
    verifyFileExists(filePath);
    
    // Verify we can read it back
    OtbReader reader;
    QVERIFY(reader.readFile(filePath));
    ServerItemList readItems = reader.getItems();
    QVERIFY(readItems.isEmpty());
}

void OtbWriterTests::testWriteValidList()
{
    OtbWriter writer;
    ServerItemList items = createTestItemList(5);
    items.versionInfo.clientVersion = 1000;
    QString filePath = m_tempDir->filePath("valid.otb");
    
    bool result = writer.writeFile(filePath, items);
    
    QVERIFY(result);
    QVERIFY(!writer.hasError());
    QCOMPARE(writer.getItemsWritten(), 5);
    QCOMPARE(writer.getItemsSkipped(), 0);
    QVERIFY(writer.getBytesWritten() > 0);
    
    verifyFileExists(filePath);
    verifyFileContent(filePath, items);
}

void OtbWriterTests::testWriteToData()
{
    OtbWriter writer;
    ServerItemList items = createTestItemList(3);
    QByteArray data;
    
    bool result = writer.writeToData(data, items);
    
    QVERIFY(result);
    QVERIFY(!writer.hasError());
    QVERIFY(!data.isEmpty());
    
    // Verify we can read the data back
    OtbReader reader;
    QVERIFY(reader.readFromData(data));
    ServerItemList readItems = reader.getItems();
    QCOMPARE(readItems.size(), items.size());
}

void OtbWriterTests::testWriteWithOptions()
{
    OtbWriter writer;
    ServerItemList items = createTestItemList(3);
    
    // Add an invalid item
    ServerItem invalidItem = createInvalidItem(999);
    items.append(invalidItem);
    
    QString filePath = m_tempDir->filePath("with_options.otb");
    
    OtbWriter::WriteOptions options;
    options.validateItems = true;
    options.skipInvalidItems = true;
    options.createBackup = false;
    
    bool result = writer.writeFile(filePath, items, options);
    
    QVERIFY(result);
    QCOMPARE(writer.getItemsWritten(), 3); // Should skip invalid item
    QCOMPARE(writer.getItemsSkipped(), 1);
    
    verifyFileExists(filePath);
}

void OtbWriterTests::testBackupCreation()
{
    QString filePath = m_tempDir->filePath("backup_test.otb");
    QString backupPath = OtbWriter::getBackupPath(filePath);
    
    // Create initial file
    OtbWriter writer1;
    ServerItemList items1 = createTestItemList(2);
    QVERIFY(writer1.writeFile(filePath, items1));
    verifyFileExists(filePath);
    
    // Write again with backup enabled
    OtbWriter writer2;
    ServerItemList items2 = createTestItemList(3);
    
    OtbWriter::WriteOptions options;
    options.createBackup = true;
    
    bool result = writer2.writeFile(filePath, items2, options);
    
    QVERIFY(result);
    verifyFileExists(filePath);
    verifyFileExists(backupPath);
    
    // Verify backup contains original data
    OtbReader reader;
    QVERIFY(reader.readFile(backupPath));
    ServerItemList backupItems = reader.getItems();
    QCOMPARE(backupItems.size(), 2); // Original size
}

void OtbWriterTests::testAtomicWrite()
{
    OtbWriter writer;
    ServerItemList items = createTestItemList(5);
    QString filePath = m_tempDir->filePath("atomic.otb");
    
    bool result = writer.writeFile(filePath, items);
    
    QVERIFY(result);
    verifyFileExists(filePath);
    
    // File should be complete and readable
    OtbReader reader;
    QVERIFY(reader.readFile(filePath));
    ServerItemList readItems = reader.getItems();
    QCOMPARE(readItems.size(), items.size());
}

void OtbWriterTests::testErrorHandling()
{
    OtbWriter writer;
    ServerItemList items = createTestItemList(3);
    
    // Test multiple errors
    writer.writeFile("", items); // Invalid path
    writer.writeFile("invalid.txt", items); // Invalid extension
    
    QVERIFY(writer.hasError());
    QVERIFY(writer.getAllErrors().size() >= 2);
    
    // Test error clearing
    writer.clearErrors();
    QVERIFY(!writer.hasError());
    QVERIFY(writer.getAllErrors().isEmpty());
    QVERIFY(writer.getLastError().isEmpty());
}

void OtbWriterTests::testProgressCallback()
{
    OtbWriter writer;
    ServerItemList items = createTestItemList(10);
    QString filePath = m_tempDir->filePath("progress.otb");
    
    QList<int> progressValues;
    QStringList statusMessages;
    
    writer.setProgressCallback([&](int current, int total, const QString& status) {
        Q_UNUSED(total)
        progressValues.append(current);
        statusMessages.append(status);
    });
    
    bool result = writer.writeFile(filePath, items);
    
    QVERIFY(result);
    QVERIFY(!progressValues.isEmpty());
    QVERIFY(!statusMessages.isEmpty());
    
    // Verify progress increases
    for (int i = 1; i < progressValues.size(); ++i) {
        QVERIFY(progressValues[i] >= progressValues[i-1]);
    }
    
    // Should reach 100%
    QVERIFY(progressValues.last() == 100);
}

void OtbWriterTests::testValidation()
{
    OtbWriter writer;
    ServerItemList items;
    
    // Add valid and invalid items
    items.addItem(createValidItem(1));
    items.addItem(createValidItem(2));
    items.append(createInvalidItem(3)); // Invalid item
    
    QString filePath = m_tempDir->filePath("validation.otb");
    
    // Test with validation enabled, skip invalid disabled
    OtbWriter::WriteOptions options;
    options.validateItems = true;
    options.skipInvalidItems = false;
    
    bool result = writer.writeFile(filePath, items, options);
    
    QVERIFY(!result); // Should fail due to invalid item
    QVERIFY(writer.hasError());
    
    // Test with validation enabled, skip invalid enabled
    writer.clearErrors();
    options.skipInvalidItems = true;
    
    result = writer.writeFile(filePath, items, options);
    
    QVERIFY(result); // Should succeed, skipping invalid item
    QCOMPARE(writer.getItemsWritten(), 2);
    QCOMPARE(writer.getItemsSkipped(), 1);
}

void OtbWriterTests::testStatistics()
{
    OtbWriter writer;
    ServerItemList items = createTestItemList(7);
    QString filePath = m_tempDir->filePath("statistics.otb");
    
    bool result = writer.writeFile(filePath, items);
    
    QVERIFY(result);
    QCOMPARE(writer.getItemsWritten(), 7);
    QCOMPARE(writer.getItemsSkipped(), 0);
    
    QFileInfo fileInfo(filePath);
    QCOMPARE(writer.getBytesWritten(), fileInfo.size());
}

void OtbWriterTests::testCanWriteToPath()
{
    // Test with valid directory
    QString validPath = m_tempDir->filePath("test.otb");
    QVERIFY(OtbWriter::canWriteToPath(validPath));
    
    // Test with non-existent directory (should be creatable)
    QString newDirPath = m_tempDir->filePath("newdir/test.otb");
    QVERIFY(OtbWriter::canWriteToPath(newDirPath));
    
    // Test with existing writable file
    QFile existingFile(validPath);
    QVERIFY(existingFile.open(QIODevice::WriteOnly));
    existingFile.write("test");
    existingFile.close();
    
    QVERIFY(OtbWriter::canWriteToPath(validPath));
}

void OtbWriterTests::testValidateOutputPath()
{
    // Test valid paths
    QVERIFY(OtbWriter::validateOutputPath(m_tempDir->filePath("test.otb")));
    QVERIFY(OtbWriter::validateOutputPath(m_tempDir->filePath("subdir/test.otb")));
    
    // Test invalid paths
    QVERIFY(!OtbWriter::validateOutputPath(""));
    QVERIFY(!OtbWriter::validateOutputPath("test.txt"));
    QVERIFY(!OtbWriter::validateOutputPath("test"));
}

void OtbWriterTests::testBackupManagement()
{
    QString filePath = m_tempDir->filePath("backup_mgmt.otb");
    QString backupPath = OtbWriter::getBackupPath(filePath, ".backup");
    
    // Create original file
    OtbWriter writer;
    ServerItemList originalItems = createTestItemList(2);
    QVERIFY(writer.writeFile(filePath, originalItems));
    
    // Create backup
    QVERIFY(writer.createBackup(filePath, backupPath));
    verifyFileExists(backupPath);
    
    // Modify original file
    ServerItemList modifiedItems = createTestItemList(3);
    QVERIFY(writer.writeFile(filePath, modifiedItems));
    
    // Restore from backup
    QVERIFY(writer.restoreFromBackup(filePath, backupPath));
    
    // Verify restoration
    OtbReader reader;
    QVERIFY(reader.readFile(filePath));
    ServerItemList restoredItems = reader.getItems();
    QCOMPARE(restoredItems.size(), 2); // Should match original
}

void OtbWriterTests::testWriteOptionsValidation()
{
    OtbWriter writer;
    ServerItemList items = createTestItemList(3);
    QString filePath = m_tempDir->filePath("options.otb");
    
    // Test all options combinations
    OtbWriter::WriteOptions options;
    options.validateItems = false;
    options.skipInvalidItems = false;
    options.preserveModificationInfo = true;
    options.compressOutput = false;
    options.createBackup = false;
    
    bool result = writer.writeFile(filePath, items, options);
    QVERIFY(result);
    
    // Test with compression (if implemented)
    options.compressOutput = true;
    QString compressedPath = m_tempDir->filePath("compressed.otb");
    result = writer.writeFile(compressedPath, items, options);
    QVERIFY(result);
}

void OtbWriterTests::testWriteReadRoundTrip()
{
    ServerItemList originalItems = createTestItemList(5);
    originalItems.versionInfo.majorVersion = 1;
    originalItems.versionInfo.minorVersion = 2;
    originalItems.versionInfo.clientVersion = 1000;
    
    QString filePath = m_tempDir->filePath("roundtrip.otb");
    
    // Write
    OtbWriter writer;
    bool writeResult = writer.writeFile(filePath, originalItems);
    QVERIFY(writeResult);
    QVERIFY(!writer.hasError());
    
    // Read
    OtbReader reader;
    bool readResult = reader.readFile(filePath);
    QVERIFY(readResult);
    QVERIFY(!reader.hasError());
    
    ServerItemList readItems = reader.getItems();
    
    // Verify data integrity
    QCOMPARE(readItems.size(), originalItems.size());
    QCOMPARE(readItems.versionInfo.majorVersion, originalItems.versionInfo.majorVersion);
    QCOMPARE(readItems.versionInfo.minorVersion, originalItems.versionInfo.minorVersion);
    QCOMPARE(readItems.versionInfo.clientVersion, originalItems.versionInfo.clientVersion);
    
    for (int i = 0; i < originalItems.size(); ++i) {
        const ServerItem* originalItem = originalItems.findItem(i + 1);
        const ServerItem* readItem = readItems.findItem(i + 1);
        
        QVERIFY(originalItem != nullptr);
        QVERIFY(readItem != nullptr);
        QCOMPARE(readItem->id, originalItem->id);
        QCOMPARE(readItem->name, originalItem->name);
        QCOMPARE(readItem->type, originalItem->type);
    }
}

void OtbWriterTests::testByteIdenticalOutput()
{
    ServerItemList items = createTestItemList(3);
    
    QString filePath1 = m_tempDir->filePath("identical1.otb");
    QString filePath2 = m_tempDir->filePath("identical2.otb");
    
    // Write same data with two different writer instances
    OtbWriter writer1;
    QVERIFY(writer1.writeFile(filePath1, items));
    
    OtbWriter writer2;
    QVERIFY(writer2.writeFile(filePath2, items));
    
    // Read both files as binary data
    QFile file1(filePath1);
    QVERIFY(file1.open(QIODevice::ReadOnly));
    QByteArray data1 = file1.readAll();
    file1.close();
    
    QFile file2(filePath2);
    QVERIFY(file2.open(QIODevice::ReadOnly));
    QByteArray data2 = file2.readAll();
    file2.close();
    
    // Verify byte-identical output
    QCOMPARE(data1.size(), data2.size());
    QCOMPARE(data1, data2);
}

void OtbWriterTests::testLargeFile()
{
    ServerItemList largeItems = createTestItemList(1000);
    QString filePath = m_tempDir->filePath("large.otb");
    
    OtbWriter writer;
    bool result = writer.writeFile(filePath, largeItems);
    
    QVERIFY(result);
    QCOMPARE(writer.getItemsWritten(), 1000);
    QVERIFY(writer.getBytesWritten() > 50000); // Should be a large file
    
    verifyFileExists(filePath);
    
    // Verify we can read it back
    OtbReader reader;
    QVERIFY(reader.readFile(filePath));
    ServerItemList readItems = reader.getItems();
    QCOMPARE(readItems.size(), 1000);
}

void OtbWriterTests::testInvalidItems()
{
    OtbWriter writer;
    ServerItemList items;
    
    // Add mix of valid and invalid items
    items.addItem(createValidItem(1));
    items.append(createInvalidItem(2));
    items.addItem(createValidItem(3));
    items.append(createInvalidItem(4));
    
    QString filePath = m_tempDir->filePath("invalid_items.otb");
    
    OtbWriter::WriteOptions options;
    options.validateItems = true;
    options.skipInvalidItems = true;
    
    bool result = writer.writeFile(filePath, items, options);
    
    QVERIFY(result);
    QCOMPARE(writer.getItemsWritten(), 2); // Only valid items
    QCOMPARE(writer.getItemsSkipped(), 2); // Invalid items skipped
    
    // Verify file contains only valid items
    OtbReader reader;
    QVERIFY(reader.readFile(filePath));
    ServerItemList readItems = reader.getItems();
    QCOMPARE(readItems.size(), 2);
}

void OtbWriterTests::testFilePermissions()
{
    // This test is platform-specific and may not work on all systems
    QString filePath = m_tempDir->filePath("permissions.otb");
    
    // Create file first
    OtbWriter writer;
    ServerItemList items = createTestItemList(2);
    QVERIFY(writer.writeFile(filePath, items));
    
    // Test overwriting existing file
    ServerItemList newItems = createTestItemList(3);
    bool result = writer.writeFile(filePath, newItems);
    
    QVERIFY(result);
    QCOMPARE(writer.getItemsWritten(), 3);
    
    // Verify overwrite worked
    OtbReader reader;
    QVERIFY(reader.readFile(filePath));
    ServerItemList readItems = reader.getItems();
    QCOMPARE(readItems.size(), 3);
}

ServerItemList OtbWriterTests::createTestItemList(int itemCount)
{
    ServerItemList items;
    items.versionInfo.majorVersion = 1;
    items.versionInfo.minorVersion = 0;
    items.versionInfo.buildNumber = 0;
    items.versionInfo.clientVersion = 800;
    
    for (int i = 1; i <= itemCount; ++i) {
        items.addItem(createValidItem(i));
    }
    
    items.clearModified();
    return items;
}

ServerItem OtbWriterTests::createValidItem(ItemId id)
{
    ServerItem item;
    item.id = id;
    item.clientId = id;
    item.type = ServerItemType::Ground;
    item.stackOrder = TileStackOrder::Ground;
    item.name = QString("Test Item %1").arg(id);
    item.description = QString("Description for item %1").arg(id);
    item.article = "a";
    item.plural = QString("Test Items %1").arg(id);
    item.width = 1;
    item.height = 1;
    item.layers = 1;
    item.patternX = 1;
    item.patternY = 1;
    item.patternZ = 1;
    item.frames = 1;
    item.flags = 0;
    item.speed = 100 + id;
    item.lightLevel = 0;
    item.lightColor = 0;
    item.minimapColor = 0;
    item.elevation = 0;
    item.tradeAs = 0;
    item.showAs = false;
    item.weaponType = 0;
    item.ammoType = 0;
    item.shootType = 0;
    item.effect = 0;
    item.distanceEffect = 0;
    item.armor = 0;
    item.defense = 0;
    item.extraDefense = 0;
    item.attack = 0;
    item.rotateTo = 0;
    item.containerSize = 0;
    item.fluidSource = 0;
    item.maxReadWriteChars = 0;
    item.maxReadChars = 0;
    item.maxWriteChars = 0;
    item.isCustomCreated = false;
    item.hasClientData = false;
    item.lastModified = QDateTime::currentDateTime();
    item.modifiedBy = "Test";
    
    return item;
}

ServerItem OtbWriterTests::createInvalidItem(ItemId id)
{
    ServerItem item;
    item.id = id;
    item.clientId = id;
    item.type = ServerItemType::None; // Invalid type
    item.stackOrder = TileStackOrder::None;
    item.name = ""; // Invalid empty name
    item.description = "";
    item.width = 0; // Invalid dimension
    item.height = 0; // Invalid dimension
    // Leave other fields with default/invalid values
    
    return item;
}

void OtbWriterTests::verifyFileExists(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    QVERIFY(fileInfo.exists());
    QVERIFY(fileInfo.isFile());
    QVERIFY(fileInfo.size() > 0);
}

void OtbWriterTests::verifyFileContent(const QString& filePath, const ServerItemList& expectedItems)
{
    OtbReader reader;
    bool result = reader.readFile(filePath);
    
    QVERIFY(result);
    QVERIFY(!reader.hasError());
    
    ServerItemList readItems = reader.getItems();
    QCOMPARE(readItems.size(), expectedItems.size());
    QCOMPARE(readItems.versionInfo.clientVersion, expectedItems.versionInfo.clientVersion);
    
    // Verify each item
    for (int i = 0; i < expectedItems.size(); ++i) {
        const ServerItem* expectedItem = expectedItems.findItem(i + 1);
        const ServerItem* readItem = readItems.findItem(i + 1);
        
        QVERIFY(expectedItem != nullptr);
        QVERIFY(readItem != nullptr);
        QCOMPARE(readItem->id, expectedItem->id);
        QCOMPARE(readItem->name, expectedItem->name);
        QCOMPARE(readItem->type, expectedItem->type);
    }
}

QTEST_APPLESS_MAIN(OtbWriterTests)

#include "OtbWriterTests.moc"