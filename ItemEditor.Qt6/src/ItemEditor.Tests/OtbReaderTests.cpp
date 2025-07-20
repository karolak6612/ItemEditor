#include <QtTest>
#include <QObject>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include "OtbReader.h"
#include "OtbWriter.h"
#include "ServerItemList.h"
#include "ServerItem.h"

class OtbReaderTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Basic functionality tests
    void testConstruction();
    void testReadNonExistentFile();
    void testReadEmptyFile();
    void testReadInvalidFile();
    
    // File validation tests
    void testIsValidOtbFile();
    void testIsValidOtbData();
    void testReadVersionInfo();
    
    // Reading functionality tests
    void testReadValidFile();
    void testReadFromData();
    void testReadWithOptions();
    
    // Error handling tests
    void testErrorHandling();
    void testProgressCallback();
    
    // Statistics tests
    void testStatistics();
    
    // Round-trip tests (read -> write -> read)
    void testRoundTripCompatibility();
    void testByteIdenticalOutput();
    
    // Edge cases
    void testLargeFile();
    void testCorruptedFile();
    void testLegacyFormat();

private:
    QTemporaryDir* m_tempDir;
    QString createTestOtbFile(const ServerItemList& items);
    ServerItemList createTestItemList(int itemCount = 5);
    QByteArray createValidOtbHeader();
    QByteArray createInvalidOtbData();
    void verifyItemsEqual(const ServerItem& item1, const ServerItem& item2);
};

void OtbReaderTests::initTestCase()
{
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
}

void OtbReaderTests::cleanupTestCase()
{
    delete m_tempDir;
}

void OtbReaderTests::testConstruction()
{
    OtbReader reader;
    
    QVERIFY(!reader.hasError());
    QVERIFY(reader.getLastError().isEmpty());
    QVERIFY(reader.getAllErrors().isEmpty());
    QCOMPARE(reader.getItemsRead(), 0);
    QCOMPARE(reader.getItemsSkipped(), 0);
    QCOMPARE(reader.getInvalidItems(), 0);
    QCOMPARE(reader.getBytesRead(), static_cast<qint64>(0));
    
    ServerItemList items = reader.getItems();
    QVERIFY(items.isEmpty());
    
    VersionInfo versionInfo = reader.getVersionInfo();
    QCOMPARE(versionInfo.majorVersion, static_cast<quint32>(0));
    QCOMPARE(versionInfo.minorVersion, static_cast<quint32>(0));
    QCOMPARE(versionInfo.buildNumber, static_cast<quint32>(0));
    QCOMPARE(versionInfo.clientVersion, static_cast<quint32>(0));
}

void OtbReaderTests::testReadNonExistentFile()
{
    OtbReader reader;
    
    bool result = reader.readFile("non_existent_file.otb");
    
    QVERIFY(!result);
    QVERIFY(reader.hasError());
    QVERIFY(!reader.getLastError().isEmpty());
    QVERIFY(reader.getLastError().contains("Cannot open file"));
}

void OtbReaderTests::testReadEmptyFile()
{
    QString filePath = m_tempDir->filePath("empty.otb");
    
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();
    
    OtbReader reader;
    bool result = reader.readFile(filePath);
    
    QVERIFY(!result);
    QVERIFY(reader.hasError());
    QVERIFY(reader.getLastError().contains("empty"));
}

void OtbReaderTests::testReadInvalidFile()
{
    QString filePath = m_tempDir->filePath("invalid.otb");
    
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("This is not a valid OTB file");
    file.close();
    
    OtbReader reader;
    bool result = reader.readFile(filePath);
    
    QVERIFY(!result);
    QVERIFY(reader.hasError());
}

void OtbReaderTests::testIsValidOtbFile()
{
    // Test with non-existent file
    QVERIFY(!OtbReader::isValidOtbFile("non_existent.otb"));
    
    // Test with invalid file
    QString invalidPath = m_tempDir->filePath("invalid.otb");
    QFile invalidFile(invalidPath);
    QVERIFY(invalidFile.open(QIODevice::WriteOnly));
    invalidFile.write("invalid data");
    invalidFile.close();
    
    QVERIFY(!OtbReader::isValidOtbFile(invalidPath));
    
    // Test with valid file
    ServerItemList testItems = createTestItemList(3);
    QString validPath = createTestOtbFile(testItems);
    QVERIFY(OtbReader::isValidOtbFile(validPath));
}

void OtbReaderTests::testIsValidOtbData()
{
    // Test with empty data
    QVERIFY(!OtbReader::isValidOtbData(QByteArray()));
    
    // Test with too small data
    QVERIFY(!OtbReader::isValidOtbData(QByteArray(8, 0)));
    
    // Test with invalid signature
    QByteArray invalidData = createInvalidOtbData();
    QVERIFY(!OtbReader::isValidOtbData(invalidData));
    
    // Test with valid header
    QByteArray validHeader = createValidOtbHeader();
    QVERIFY(OtbReader::isValidOtbData(validHeader));
}

void OtbReaderTests::testReadVersionInfo()
{
    ServerItemList testItems = createTestItemList(2);
    testItems.versionInfo.majorVersion = 1;
    testItems.versionInfo.minorVersion = 2;
    testItems.versionInfo.buildNumber = 3;
    testItems.versionInfo.clientVersion = 1000;
    
    QString filePath = createTestOtbFile(testItems);
    
    VersionInfo versionInfo = OtbReader::readVersionInfo(filePath);
    
    QCOMPARE(versionInfo.majorVersion, static_cast<quint32>(1));
    QCOMPARE(versionInfo.minorVersion, static_cast<quint32>(2));
    QCOMPARE(versionInfo.buildNumber, static_cast<quint32>(3));
    QCOMPARE(versionInfo.clientVersion, static_cast<quint32>(1000));
}

void OtbReaderTests::testReadValidFile()
{
    ServerItemList originalItems = createTestItemList(5);
    originalItems.versionInfo.clientVersion = 1000;
    
    QString filePath = createTestOtbFile(originalItems);
    
    OtbReader reader;
    bool result = reader.readFile(filePath);
    
    QVERIFY(result);
    QVERIFY(!reader.hasError());
    QCOMPARE(reader.getItemsRead(), 5);
    QCOMPARE(reader.getItemsSkipped(), 0);
    QCOMPARE(reader.getInvalidItems(), 0);
    QVERIFY(reader.getBytesRead() > 0);
    
    ServerItemList readItems = reader.getItems();
    QCOMPARE(readItems.size(), originalItems.size());
    QCOMPARE(readItems.versionInfo.clientVersion, originalItems.versionInfo.clientVersion);
    
    // Verify individual items
    for (int i = 0; i < readItems.size(); ++i) {
        const ServerItem* originalItem = originalItems.findItem(i + 1);
        const ServerItem* readItem = readItems.findItem(i + 1);
        
        QVERIFY(originalItem != nullptr);
        QVERIFY(readItem != nullptr);
        verifyItemsEqual(*originalItem, *readItem);
    }
}

void OtbReaderTests::testReadFromData()
{
    ServerItemList originalItems = createTestItemList(3);
    
    // Create OTB data using writer
    OtbWriter writer;
    QByteArray otbData;
    QVERIFY(writer.writeToData(otbData, originalItems));
    
    // Read from data
    OtbReader reader;
    bool result = reader.readFromData(otbData);
    
    QVERIFY(result);
    QVERIFY(!reader.hasError());
    
    ServerItemList readItems = reader.getItems();
    QCOMPARE(readItems.size(), originalItems.size());
}

void OtbReaderTests::testReadWithOptions()
{
    ServerItemList originalItems = createTestItemList(5);
    
    // Add an invalid item
    ServerItem invalidItem;
    invalidItem.id = 999;
    invalidItem.type = ServerItemType::None; // Invalid type
    invalidItem.name = ""; // Invalid name
    originalItems.append(invalidItem);
    
    QString filePath = createTestOtbFile(originalItems);
    
    OtbReader reader;
    OtbReader::ReadOptions options;
    options.validateItems = true;
    options.skipInvalidItems = true;
    
    bool result = reader.readFile(filePath, options);
    
    QVERIFY(result);
    QCOMPARE(reader.getItemsRead(), 5); // Should skip the invalid item
    QCOMPARE(reader.getItemsSkipped(), 1);
    QCOMPARE(reader.getInvalidItems(), 1);
}

void OtbReaderTests::testErrorHandling()
{
    OtbReader reader;
    
    // Test error accumulation
    reader.readFile("non_existent1.otb");
    reader.readFile("non_existent2.otb");
    
    QVERIFY(reader.hasError());
    QVERIFY(reader.getAllErrors().size() >= 2);
    
    // Test error clearing
    reader.clearErrors();
    QVERIFY(!reader.hasError());
    QVERIFY(reader.getAllErrors().isEmpty());
    QVERIFY(reader.getLastError().isEmpty());
}

void OtbReaderTests::testProgressCallback()
{
    ServerItemList testItems = createTestItemList(10);
    QString filePath = createTestOtbFile(testItems);
    
    QList<int> progressValues;
    QStringList statusMessages;
    
    OtbReader reader;
    reader.setProgressCallback([&](int current, int total, const QString& status) {
        Q_UNUSED(total)
        progressValues.append(current);
        statusMessages.append(status);
    });
    
    bool result = reader.readFile(filePath);
    
    QVERIFY(result);
    QVERIFY(!progressValues.isEmpty());
    QVERIFY(!statusMessages.isEmpty());
    
    // Verify progress increases
    for (int i = 1; i < progressValues.size(); ++i) {
        QVERIFY(progressValues[i] >= progressValues[i-1]);
    }
}

void OtbReaderTests::testStatistics()
{
    ServerItemList testItems = createTestItemList(7);
    QString filePath = createTestOtbFile(testItems);
    
    OtbReader reader;
    bool result = reader.readFile(filePath);
    
    QVERIFY(result);
    QCOMPARE(reader.getItemsRead(), 7);
    QCOMPARE(reader.getItemsSkipped(), 0);
    QCOMPARE(reader.getInvalidItems(), 0);
    
    QFileInfo fileInfo(filePath);
    QCOMPARE(reader.getBytesRead(), fileInfo.size());
}

void OtbReaderTests::testRoundTripCompatibility()
{
    ServerItemList originalItems = createTestItemList(5);
    originalItems.versionInfo.majorVersion = 1;
    originalItems.versionInfo.minorVersion = 0;
    originalItems.versionInfo.clientVersion = 1000;
    
    // Write -> Read -> Write -> Read
    QString filePath1 = m_tempDir->filePath("roundtrip1.otb");
    QString filePath2 = m_tempDir->filePath("roundtrip2.otb");
    
    // First write
    OtbWriter writer1;
    QVERIFY(writer1.writeFile(filePath1, originalItems));
    
    // First read
    OtbReader reader1;
    QVERIFY(reader1.readFile(filePath1));
    ServerItemList readItems1 = reader1.getItems();
    
    // Second write
    OtbWriter writer2;
    QVERIFY(writer2.writeFile(filePath2, readItems1));
    
    // Second read
    OtbReader reader2;
    QVERIFY(reader2.readFile(filePath2));
    ServerItemList readItems2 = reader2.getItems();
    
    // Verify consistency
    QCOMPARE(readItems1.size(), readItems2.size());
    QCOMPARE(readItems1.versionInfo.clientVersion, readItems2.versionInfo.clientVersion);
    
    for (int i = 0; i < readItems1.size(); ++i) {
        const ServerItem* item1 = readItems1.findItem(i + 1);
        const ServerItem* item2 = readItems2.findItem(i + 1);
        
        QVERIFY(item1 != nullptr);
        QVERIFY(item2 != nullptr);
        verifyItemsEqual(*item1, *item2);
    }
}

void OtbReaderTests::testByteIdenticalOutput()
{
    ServerItemList testItems = createTestItemList(3);
    
    QString filePath1 = m_tempDir->filePath("identical1.otb");
    QString filePath2 = m_tempDir->filePath("identical2.otb");
    
    // Write same data twice
    OtbWriter writer1;
    QVERIFY(writer1.writeFile(filePath1, testItems));
    
    OtbWriter writer2;
    QVERIFY(writer2.writeFile(filePath2, testItems));
    
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

void OtbReaderTests::testLargeFile()
{
    ServerItemList largeItems = createTestItemList(1000);
    QString filePath = createTestOtbFile(largeItems);
    
    OtbReader reader;
    bool result = reader.readFile(filePath);
    
    QVERIFY(result);
    QCOMPARE(reader.getItemsRead(), 1000);
    QVERIFY(reader.getBytesRead() > 10000); // Should be a reasonably large file
}

void OtbReaderTests::testCorruptedFile()
{
    ServerItemList testItems = createTestItemList(3);
    QString filePath = createTestOtbFile(testItems);
    
    // Corrupt the file by truncating it
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadWrite));
    qint64 originalSize = file.size();
    file.resize(originalSize / 2); // Truncate to half size
    file.close();
    
    OtbReader reader;
    bool result = reader.readFile(filePath);
    
    QVERIFY(!result);
    QVERIFY(reader.hasError());
}

void OtbReaderTests::testLegacyFormat()
{
    // This test would verify handling of legacy OTB formats
    // For now, we'll just test that the reader handles unknown versions gracefully
    
    ServerItemList testItems = createTestItemList(2);
    testItems.versionInfo.majorVersion = 99; // Unsupported version
    
    QString filePath = createTestOtbFile(testItems);
    
    OtbReader reader;
    bool result = reader.readFile(filePath);
    
    // Should still read successfully but with warnings
    QVERIFY(result);
    QVERIFY(!reader.getAllErrors().isEmpty()); // Should have warnings
}

QString OtbReaderTests::createTestOtbFile(const ServerItemList& items)
{
    QString filePath = m_tempDir->filePath(QString("test_%1.otb").arg(QDateTime::currentMSecsSinceEpoch()));
    
    OtbWriter writer;
    bool result = writer.writeFile(filePath, items);
    Q_ASSERT(result);
    
    return filePath;
}

ServerItemList OtbReaderTests::createTestItemList(int itemCount)
{
    ServerItemList items;
    items.versionInfo.majorVersion = 1;
    items.versionInfo.minorVersion = 0;
    items.versionInfo.buildNumber = 0;
    items.versionInfo.clientVersion = 800;
    
    for (int i = 1; i <= itemCount; ++i) {
        ServerItem item;
        item.id = i;
        item.clientId = i;
        item.type = ServerItemType::Ground;
        item.stackOrder = TileStackOrder::Ground;
        item.name = QString("Test Item %1").arg(i);
        item.description = QString("Description for item %1").arg(i);
        item.article = "a";
        item.plural = QString("Test Items %1").arg(i);
        item.width = 1;
        item.height = 1;
        item.layers = 1;
        item.patternX = 1;
        item.patternY = 1;
        item.patternZ = 1;
        item.frames = 1;
        item.flags = 0;
        item.speed = 100 + i;
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
        
        items.addItem(item);
    }
    
    items.clearModified();
    return items;
}

QByteArray OtbReaderTests::createValidOtbHeader()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    stream << static_cast<quint32>(0x00000000); // OTB_SIGNATURE
    stream << static_cast<quint32>(1); // Major version
    stream << static_cast<quint32>(0); // Minor version
    stream << static_cast<quint32>(0); // Build number
    stream << static_cast<quint32>(800); // Client version
    
    return data;
}

QByteArray OtbReaderTests::createInvalidOtbData()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    stream << static_cast<quint32>(0xDEADBEEF); // Invalid signature
    stream << static_cast<quint32>(1);
    stream << static_cast<quint32>(0);
    stream << static_cast<quint32>(0);
    stream << static_cast<quint32>(800);
    
    return data;
}

void OtbReaderTests::verifyItemsEqual(const ServerItem& item1, const ServerItem& item2)
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
    QCOMPARE(item1.minimapColor, item2.minimapColor);
    QCOMPARE(item1.elevation, item2.elevation);
    QCOMPARE(item1.tradeAs, item2.tradeAs);
    QCOMPARE(item1.showAs, item2.showAs);
    QCOMPARE(item1.weaponType, item2.weaponType);
    QCOMPARE(item1.ammoType, item2.ammoType);
    QCOMPARE(item1.shootType, item2.shootType);
    QCOMPARE(item1.effect, item2.effect);
    QCOMPARE(item1.distanceEffect, item2.distanceEffect);
    QCOMPARE(item1.armor, item2.armor);
    QCOMPARE(item1.defense, item2.defense);
    QCOMPARE(item1.extraDefense, item2.extraDefense);
    QCOMPARE(item1.attack, item2.attack);
    QCOMPARE(item1.rotateTo, item2.rotateTo);
    QCOMPARE(item1.containerSize, item2.containerSize);
    QCOMPARE(item1.fluidSource, item2.fluidSource);
    QCOMPARE(item1.maxReadWriteChars, item2.maxReadWriteChars);
    QCOMPARE(item1.maxReadChars, item2.maxReadChars);
    QCOMPARE(item1.maxWriteChars, item2.maxWriteChars);
    QCOMPARE(item1.isCustomCreated, item2.isCustomCreated);
    QCOMPARE(item1.hasClientData, item2.hasClientData);
    QCOMPARE(item1.modifiedBy, item2.modifiedBy);
    // Note: lastModified might differ slightly due to serialization precision
}

QTEST_APPLESS_MAIN(OtbReaderTests)

#include "OtbReaderTests.moc"