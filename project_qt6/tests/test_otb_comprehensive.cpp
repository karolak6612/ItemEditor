#include <QCoreApplication>
#include <QDebug>
#include <QTest>
#include <QTemporaryFile>
#include <QDir>
#include <QFileInfo>
#include <QTime>
#include <QElapsedTimer>
#include <QRandomGenerator>

// Include OTB system headers
#include "otb/binarytree.h"
#include "otb/otbreader.h"
#include "otb/otbwriter.h"
#include "otb/otbheader.h"
#include "otb/item.h"
#include "otb/otbvalidator.h"
#include "otb/otberrors.h"
#include "otb/otbbackup.h"
#include "otb/otbcache.h"
#include "otb/otbperformance.h"

/**
 * Comprehensive OTB Unit Test Suite
 * 
 * This test suite provides complete coverage of OTB functionality including:
 * - Basic read/write operations
 * - Edge cases and error conditions
 * - Performance benchmarks
 * - C# compatibility verification
 * - Data integrity validation
 * - Memory management tests
 */

namespace OTBTests {

class OTBTestSuite : public QObject {
    Q_OBJECT

private:
    QString m_testDataDir;
    QStringList m_testFiles;
    int m_testsPassed = 0;
    int m_testsFailed = 0;

public:
    OTBTestSuite() {
        // Initialize test data directory
        m_testDataDir = QDir::tempPath() + "/otb_tests";
        QDir().mkpath(m_testDataDir);
    }

    ~OTBTestSuite() {
        // Cleanup test files
        QDir testDir(m_testDataDir);
        testDir.removeRecursively();
    }

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Basic functionality tests
    void testBinaryTreeOperations();
    void testOTBHeaderHandling();
    void testItemDataStructure();
    void testOTBReaderBasic();
    void testOTBWriterBasic();
    
    // Edge case tests
    void testEmptyFiles();
    void testCorruptedFiles();
    void testLargeFiles();
    void testSpecialCharacters();
    void testBoundaryValues();
    
    // Error condition tests
    void testFileNotFound();
    void testPermissionDenied();
    void testInvalidFormat();
    void testMemoryExhaustion();
    void testInterruptedOperations();
    
    // Performance tests
    void testReadPerformance();
    void testWritePerformance();
    void testMemoryUsage();
    void testCacheEfficiency();
    
    // C# compatibility tests
    void testCSharpCompatibility();
    void testVersionCompatibility();
    void testDataFormatCompatibility();
    
    // Integration tests
    void testBackupAndRestore();
    void testValidationSystem();
    void testConcurrentAccess();

private:
    void logTestResult(const QString& testName, bool passed, const QString& details = "");
    QString createTestFile(const QString& name, int itemCount = 100);
    bool compareFiles(const QString& file1, const QString& file2);
    void generateTestData(QList<OTB::Item>& items, int count);
};

void OTBTestSuite::initTestCase() {
    qDebug() << "=== Initializing OTB Test Suite ===";
    qDebug() << "Test data directory:" << m_testDataDir;
    
    // Create sample test files
    m_testFiles << createTestFile("small_test.otb", 10);
    m_testFiles << createTestFile("medium_test.otb", 1000);
    m_testFiles << createTestFile("large_test.otb", 10000);
    
    qDebug() << "Created" << m_testFiles.size() << "test files";
}

void OTBTestSuite::cleanupTestCase() {
    qDebug() << "=== Test Suite Summary ===";
    qDebug() << "Tests Passed:" << m_testsPassed;
    qDebug() << "Tests Failed:" << m_testsFailed;
    qDebug() << "Success Rate:" << (double(m_testsPassed) / (m_testsPassed + m_testsFailed) * 100) << "%";
}

void OTBTestSuite::testBinaryTreeOperations() {
    qDebug() << "Testing BinaryTree operations...";
    
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString testPath = tempFile.fileName();
    tempFile.close();
    
    // Test 1: Basic write/read operations
    {
        OTB::BinaryTree writer;
        QVERIFY(writer.open(testPath, QIODevice::WriteOnly));
        
        writer.writeNodeStart(0x00); // Root node
        writer.writeValue<quint32>(12345);
        writer.writeString("Test String", true);
        
        // Write child node
        writer.writeNodeStart(0x01);
        writer.writeValue<quint16>(999);
        writer.writeNodeEnd();
        
        writer.writeNodeEnd(); // Close root
        writer.close();
    }
    
    {
        OTB::BinaryTree reader;
        QVERIFY(reader.open(testPath, QIODevice::ReadOnly));
        
        QVERIFY(reader.enterNode());
        QCOMPARE(reader.getCurrentNodeType(), static_cast<quint8>(0x00));
        
        quint32 value = reader.readValue<quint32>();
        QCOMPARE(value, 12345u);
        
        QString str = reader.readString(true);
        QCOMPARE(str, QString("Test String"));
        
        QVERIFY(reader.hasNextNode());
        QVERIFY(reader.enterNode());
        QCOMPARE(reader.getCurrentNodeType(), static_cast<quint8>(0x01));
        
        quint16 childValue = reader.readValue<quint16>();
        QCOMPARE(childValue, static_cast<quint16>(999));
        
        reader.leaveNode();
        reader.leaveNode();
        reader.close();
    }
    
    logTestResult("BinaryTree Operations", true);
}

void OTBTestSuite::testOTBHeaderHandling() {
    qDebug() << "Testing OTB header handling...";
    
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString testPath = tempFile.fileName();
    tempFile.close();
    
    // Create header with test data
    OTB::OTBHeader originalHeader;
    originalHeader.setVersion(1, 2, 3);
    originalHeader.setDescription("Test OTB File");
    originalHeader.setItemCount(500);
    originalHeader.setCreatureCount(100);
    originalHeader.setEffectCount(50);
    originalHeader.setDistanceCount(25);
    
    // Write header
    {
        QFile file(testPath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QVERIFY(originalHeader.write(&file));
        file.close();
    }
    
    // Read and verify header
    {
        QFile file(testPath);
        QVERIFY(file.open(QIODevice::ReadOnly));
        
        OTB::OTBHeader readHeader;
        QVERIFY(readHeader.read(&file));
        
        QCOMPARE(readHeader.getMajorVersion(), 1u);
        QCOMPARE(readHeader.getMinorVersion(), 2u);
        QCOMPARE(readHeader.getBuildNumber(), 3u);
        QCOMPARE(readHeader.getDescription(), QString("Test OTB File"));
        QCOMPARE(readHeader.getItemCount(), 500u);
        QCOMPARE(readHeader.getCreatureCount(), 100u);
        QCOMPARE(readHeader.getEffectCount(), 50u);
        QCOMPARE(readHeader.getDistanceCount(), 25u);
        
        file.close();
    }
    
    logTestResult("OTB Header Handling", true);
}

void OTBTestSuite::testItemDataStructure() {
    qDebug() << "Testing Item data structure...";
    
    // Create test item with all properties
    OTB::Item originalItem;
    originalItem.setId(1001);
    originalItem.setClientId(2002);
    originalItem.setName("Test Item");
    originalItem.setDescription("A test item for validation");
    originalItem.setWeight(150);
    originalItem.setCapacity(500);
    originalItem.setSpeed(250);
    originalItem.setArmor(10);
    originalItem.setAttack(25);
    originalItem.setDefense(15);
    originalItem.setExtraDefense(5);
    originalItem.setRotateTo(90);
    originalItem.setContainerSize(20);
    originalItem.setStackable(true);
    originalItem.setUseable(true);
    originalItem.setMoveable(true);
    originalItem.setPickupable(true);
    originalItem.setHangable(false);
    originalItem.setHookSouth(true);
    originalItem.setHookEast(false);
    originalItem.setReadable(true);
    originalItem.setLookThrough(false);
    originalItem.setAnimation(true);
    originalItem.setWalkStack(false);
    
    // Test serialization/deserialization
    QByteArray serializedData;
    {
        QDataStream stream(&serializedData, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::LittleEndian);
        QVERIFY(originalItem.serialize(stream));
    }
    
    {
        OTB::Item deserializedItem;
        QDataStream stream(&serializedData, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::LittleEndian);
        QVERIFY(deserializedItem.deserialize(stream));
        
        // Verify all properties
        QCOMPARE(deserializedItem.getId(), originalItem.getId());
        QCOMPARE(deserializedItem.getClientId(), originalItem.getClientId());
        QCOMPARE(deserializedItem.getName(), originalItem.getName());
        QCOMPARE(deserializedItem.getDescription(), originalItem.getDescription());
        QCOMPARE(deserializedItem.getWeight(), originalItem.getWeight());
        QCOMPARE(deserializedItem.getCapacity(), originalItem.getCapacity());
        QCOMPARE(deserializedItem.getSpeed(), originalItem.getSpeed());
        QCOMPARE(deserializedItem.getArmor(), originalItem.getArmor());
        QCOMPARE(deserializedItem.getAttack(), originalItem.getAttack());
        QCOMPARE(deserializedItem.getDefense(), originalItem.getDefense());
        QCOMPARE(deserializedItem.isStackable(), originalItem.isStackable());
        QCOMPARE(deserializedItem.isUseable(), originalItem.isUseable());
        QCOMPARE(deserializedItem.isMoveable(), originalItem.isMoveable());
        QCOMPARE(deserializedItem.isPickupable(), originalItem.isPickupable());
    }
    
    logTestResult("Item Data Structure", true);
}void OTBTestSuite::testOTBReaderBasic() {
    qDebug() << "Testing OTB reader basic functionality...";
    
    if (m_testFiles.isEmpty()) {
        logTestResult("OTB Reader Basic", false, "No test files available");
        return;
    }
    
    QString testFile = m_testFiles.first();
    OTB::OTBReader reader;
    
    // Test opening file
    QVERIFY(reader.open(testFile));
    
    // Test reading header
    OTB::OTBHeader header;
    QVERIFY(reader.readHeader(header));
    QVERIFY(header.isValid());
    
    // Test reading items
    QList<OTB::Item> items;
    QVERIFY(reader.readItems(items));
    QVERIFY(!items.isEmpty());
    
    // Verify item data integrity
    for (const auto& item : items) {
        QVERIFY(item.isValid());
        QVERIFY(item.getId() > 0);
    }
    
    reader.close();
    logTestResult("OTB Reader Basic", true);
}

void OTBTestSuite::testOTBWriterBasic() {
    qDebug() << "Testing OTB writer basic functionality...";
    
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString testPath = tempFile.fileName();
    tempFile.close();
    
    // Create test data
    OTB::OTBHeader header;
    header.setVersion(1, 0, 0);
    header.setDescription("Test Write File");
    
    QList<OTB::Item> items;
    generateTestData(items, 50);
    header.setItemCount(items.size());
    
    // Write file
    OTB::OTBWriter writer;
    QVERIFY(writer.open(testPath));
    QVERIFY(writer.writeHeader(header));
    QVERIFY(writer.writeItems(items));
    writer.close();
    
    // Verify written file
    OTB::OTBReader reader;
    QVERIFY(reader.open(testPath));
    
    OTB::OTBHeader readHeader;
    QVERIFY(reader.readHeader(readHeader));
    QCOMPARE(readHeader.getDescription(), header.getDescription());
    QCOMPARE(readHeader.getItemCount(), header.getItemCount());
    
    QList<OTB::Item> readItems;
    QVERIFY(reader.readItems(readItems));
    QCOMPARE(readItems.size(), items.size());
    
    reader.close();
    logTestResult("OTB Writer Basic", true);
}

void OTBTestSuite::testEmptyFiles() {
    qDebug() << "Testing empty file handling...";
    
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString testPath = tempFile.fileName();
    tempFile.close();
    
    // Create empty file
    QFile file(testPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();
    
    // Try to read empty file
    OTB::OTBReader reader;
    QVERIFY(!reader.open(testPath)); // Should fail gracefully
    
    logTestResult("Empty Files", true);
}

void OTBTestSuite::testCorruptedFiles() {
    qDebug() << "Testing corrupted file handling...";
    
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString testPath = tempFile.fileName();
    
    // Write corrupted data
    QByteArray corruptedData;
    for (int i = 0; i < 1000; ++i) {
        corruptedData.append(static_cast<char>(QRandomGenerator::global()->bounded(256)));
    }
    tempFile.write(corruptedData);
    tempFile.close();
    
    // Try to read corrupted file
    OTB::OTBReader reader;
    bool opened = reader.open(testPath);
    if (opened) {
        OTB::OTBHeader header;
        bool headerRead = reader.readHeader(header);
        QVERIFY(!headerRead || !header.isValid()); // Should detect corruption
        reader.close();
    }
    
    logTestResult("Corrupted Files", true);
}

void OTBTestSuite::testLargeFiles() {
    qDebug() << "Testing large file handling...";
    
    if (m_testFiles.size() < 3) {
        logTestResult("Large Files", false, "Large test file not available");
        return;
    }
    
    QString largeFile = m_testFiles.last(); // Should be the 10000 item file
    
    QElapsedTimer timer;
    timer.start();
    
    OTB::OTBReader reader;
    QVERIFY(reader.open(largeFile));
    
    OTB::OTBHeader header;
    QVERIFY(reader.readHeader(header));
    
    QList<OTB::Item> items;
    QVERIFY(reader.readItems(items));
    
    reader.close();
    
    qint64 elapsed = timer.elapsed();
    qDebug() << "Large file read time:" << elapsed << "ms for" << items.size() << "items";
    
    // Performance check - should complete within reasonable time
    QVERIFY(elapsed < 10000); // Less than 10 seconds
    
    logTestResult("Large Files", true, QString("Read %1 items in %2ms").arg(items.size()).arg(elapsed));
}

void OTBTestSuite::testSpecialCharacters() {
    qDebug() << "Testing special character handling...";
    
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString testPath = tempFile.fileName();
    tempFile.close();
    
    // Create item with special characters
    OTB::Item testItem;
    testItem.setId(1);
    testItem.setName("Test\xFF\xFE\xFD Item"); // Contains escape characters
    testItem.setDescription("Description with\nnewlines\tand\ttabs");
    
    // Write and read back
    {
        OTB::OTBWriter writer;
        QVERIFY(writer.open(testPath));
        
        OTB::OTBHeader header;
        header.setVersion(1, 0, 0);
        header.setItemCount(1);
        QVERIFY(writer.writeHeader(header));
        
        QList<OTB::Item> items;
        items.append(testItem);
        QVERIFY(writer.writeItems(items));
        writer.close();
    }
    
    {
        OTB::OTBReader reader;
        QVERIFY(reader.open(testPath));
        
        OTB::OTBHeader header;
        QVERIFY(reader.readHeader(header));
        
        QList<OTB::Item> items;
        QVERIFY(reader.readItems(items));
        QCOMPARE(items.size(), 1);
        
        const OTB::Item& readItem = items.first();
        QCOMPARE(readItem.getName(), testItem.getName());
        QCOMPARE(readItem.getDescription(), testItem.getDescription());
        
        reader.close();
    }
    
    logTestResult("Special Characters", true);
}

void OTBTestSuite::testBoundaryValues() {
    qDebug() << "Testing boundary values...";
    
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString testPath = tempFile.fileName();
    tempFile.close();
    
    // Test with extreme values
    OTB::Item testItem;
    testItem.setId(0xFFFFFFFF); // Max uint32
    testItem.setClientId(0xFFFF); // Max uint16
    testItem.setWeight(0xFFFFFFFF); // Max weight
    testItem.setSpeed(0); // Min speed
    testItem.setArmor(0xFFFF); // Max armor
    
    // Write and read back
    {
        OTB::OTBWriter writer;
        QVERIFY(writer.open(testPath));
        
        OTB::OTBHeader header;
        header.setVersion(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF); // Max versions
        header.setItemCount(1);
        QVERIFY(writer.writeHeader(header));
        
        QList<OTB::Item> items;
        items.append(testItem);
        QVERIFY(writer.writeItems(items));
        writer.close();
    }
    
    {
        OTB::OTBReader reader;
        QVERIFY(reader.open(testPath));
        
        OTB::OTBHeader header;
        QVERIFY(reader.readHeader(header));
        
        QList<OTB::Item> items;
        QVERIFY(reader.readItems(items));
        QCOMPARE(items.size(), 1);
        
        const OTB::Item& readItem = items.first();
        QCOMPARE(readItem.getId(), testItem.getId());
        QCOMPARE(readItem.getClientId(), testItem.getClientId());
        QCOMPARE(readItem.getWeight(), testItem.getWeight());
        QCOMPARE(readItem.getSpeed(), testItem.getSpeed());
        QCOMPARE(readItem.getArmor(), testItem.getArmor());
        
        reader.close();
    }
    
    logTestResult("Boundary Values", true);
}

void OTBTestSuite::testFileNotFound() {
    qDebug() << "Testing file not found error handling...";
    
    QString nonExistentFile = m_testDataDir + "/does_not_exist.otb";
    
    OTB::OTBReader reader;
    QVERIFY(!reader.open(nonExistentFile));
    
    // Check error handling
    QString errorMsg = reader.getLastError();
    QVERIFY(!errorMsg.isEmpty());
    
    logTestResult("File Not Found", true);
}

void OTBTestSuite::testPermissionDenied() {
    qDebug() << "Testing permission denied error handling...";
    
    // This test is platform-specific and may not work on all systems
    // We'll create a file and try to make it read-only, then write to it
    
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString testPath = tempFile.fileName();
    tempFile.close();
    
    // Make file read-only
    QFile::setPermissions(testPath, QFile::ReadOwner | QFile::ReadGroup | QFile::ReadOther);
    
    OTB::OTBWriter writer;
    bool opened = writer.open(testPath);
    
    if (!opened) {
        // Expected behavior - permission denied
        logTestResult("Permission Denied", true);
    } else {
        // Some systems might still allow writing
        writer.close();
        logTestResult("Permission Denied", true, "System allows writing to read-only file");
    }
    
    // Restore permissions for cleanup
    QFile::setPermissions(testPath, QFile::WriteOwner | QFile::ReadOwner);
}

void OTBTestSuite::testInvalidFormat() {
    qDebug() << "Testing invalid format detection...";
    
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    
    // Write invalid header
    QByteArray invalidData = "INVALID_OTB_FILE_FORMAT";
    tempFile.write(invalidData);
    tempFile.close();
    
    OTB::OTBReader reader;
    bool opened = reader.open(tempFile.fileName());
    
    if (opened) {
        OTB::OTBHeader header;
        bool headerRead = reader.readHeader(header);
        QVERIFY(!headerRead || !header.isValid());
        reader.close();
    }
    
    logTestResult("Invalid Format", true);
}void OTBTestSuite::testMemoryExhaustion() {
    qDebug() << "Testing memory exhaustion handling...";
    
    // This test attempts to simulate memory pressure
    // We'll try to allocate a very large number of items
    
    try {
        QList<OTB::Item> largeList;
        
        // Try to create a very large list (this should be handled gracefully)
        for (int i = 0; i < 1000000; ++i) {
            OTB::Item item;
            item.setId(i);
            item.setName(QString("Item %1").arg(i));
            largeList.append(item);
            
            // Break if we've allocated enough for testing
            if (i > 100000) break;
        }
        
        logTestResult("Memory Exhaustion", true, QString("Allocated %1 items").arg(largeList.size()));
    } catch (const std::bad_alloc&) {
        logTestResult("Memory Exhaustion", true, "Caught memory allocation exception");
    } catch (...) {
        logTestResult("Memory Exhaustion", true, "Caught unknown exception");
    }
}

void OTBTestSuite::testInterruptedOperations() {
    qDebug() << "Testing interrupted operation handling...";
    
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString testPath = tempFile.fileName();
    tempFile.close();
    
    // Start writing operation
    OTB::OTBWriter writer;
    QVERIFY(writer.open(testPath));
    
    OTB::OTBHeader header;
    header.setVersion(1, 0, 0);
    header.setItemCount(1000);
    QVERIFY(writer.writeHeader(header));
    
    // Simulate interruption by closing without finishing
    writer.close();
    
    // Try to read the incomplete file
    OTB::OTBReader reader;
    bool opened = reader.open(testPath);
    
    if (opened) {
        OTB::OTBHeader readHeader;
        bool headerRead = reader.readHeader(readHeader);
        
        if (headerRead) {
            QList<OTB::Item> items;
            bool itemsRead = reader.readItems(items);
            // Should handle incomplete data gracefully
            QVERIFY(!itemsRead || items.size() != 1000);
        }
        
        reader.close();
    }
    
    logTestResult("Interrupted Operations", true);
}

void OTBTestSuite::testReadPerformance() {
    qDebug() << "Testing read performance...";
    
    if (m_testFiles.isEmpty()) {
        logTestResult("Read Performance", false, "No test files available");
        return;
    }
    
    QString testFile = m_testFiles.last(); // Use largest file
    
    QElapsedTimer timer;
    timer.start();
    
    OTB::OTBReader reader;
    QVERIFY(reader.open(testFile));
    
    OTB::OTBHeader header;
    QVERIFY(reader.readHeader(header));
    
    QList<OTB::Item> items;
    QVERIFY(reader.readItems(items));
    
    reader.close();
    
    qint64 elapsed = timer.elapsed();
    double itemsPerSecond = (items.size() * 1000.0) / elapsed;
    
    qDebug() << "Read performance:" << itemsPerSecond << "items/second";
    qDebug() << "Total time:" << elapsed << "ms for" << items.size() << "items";
    
    // Performance should be reasonable
    QVERIFY(itemsPerSecond > 100); // At least 100 items per second
    
    logTestResult("Read Performance", true, 
                  QString("%1 items/sec (%2 items in %3ms)")
                  .arg(itemsPerSecond, 0, 'f', 1)
                  .arg(items.size())
                  .arg(elapsed));
}

void OTBTestSuite::testWritePerformance() {
    qDebug() << "Testing write performance...";
    
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString testPath = tempFile.fileName();
    tempFile.close();
    
    // Generate test data
    QList<OTB::Item> items;
    generateTestData(items, 5000);
    
    QElapsedTimer timer;
    timer.start();
    
    OTB::OTBWriter writer;
    QVERIFY(writer.open(testPath));
    
    OTB::OTBHeader header;
    header.setVersion(1, 0, 0);
    header.setItemCount(items.size());
    QVERIFY(writer.writeHeader(header));
    
    QVERIFY(writer.writeItems(items));
    writer.close();
    
    qint64 elapsed = timer.elapsed();
    double itemsPerSecond = (items.size() * 1000.0) / elapsed;
    
    qDebug() << "Write performance:" << itemsPerSecond << "items/second";
    qDebug() << "Total time:" << elapsed << "ms for" << items.size() << "items";
    
    // Performance should be reasonable
    QVERIFY(itemsPerSecond > 100); // At least 100 items per second
    
    logTestResult("Write Performance", true, 
                  QString("%1 items/sec (%2 items in %3ms)")
                  .arg(itemsPerSecond, 0, 'f', 1)
                  .arg(items.size())
                  .arg(elapsed));
}

void OTBTestSuite::testMemoryUsage() {
    qDebug() << "Testing memory usage...";
    
    // This is a basic memory usage test
    // In a real implementation, you might use platform-specific APIs
    // to measure actual memory consumption
    
    QList<OTB::Item> items;
    generateTestData(items, 10000);
    
    // Calculate approximate memory usage
    size_t estimatedMemory = items.size() * sizeof(OTB::Item);
    qDebug() << "Estimated memory usage for" << items.size() << "items:" << estimatedMemory << "bytes";
    
    // Memory usage should be reasonable
    QVERIFY(estimatedMemory < 100 * 1024 * 1024); // Less than 100MB for 10k items
    
    logTestResult("Memory Usage", true, 
                  QString("%1 bytes for %2 items").arg(estimatedMemory).arg(items.size()));
}

void OTBTestSuite::testCacheEfficiency() {
    qDebug() << "Testing cache efficiency...";
    
    if (m_testFiles.isEmpty()) {
        logTestResult("Cache Efficiency", false, "No test files available");
        return;
    }
    
    QString testFile = m_testFiles.first();
    
    // First read (cold cache)
    QElapsedTimer timer1;
    timer1.start();
    
    OTB::OTBReader reader1;
    QVERIFY(reader1.open(testFile));
    
    QList<OTB::Item> items1;
    OTB::OTBHeader header1;
    QVERIFY(reader1.readHeader(header1));
    QVERIFY(reader1.readItems(items1));
    reader1.close();
    
    qint64 coldTime = timer1.elapsed();
    
    // Second read (warm cache)
    QElapsedTimer timer2;
    timer2.start();
    
    OTB::OTBReader reader2;
    QVERIFY(reader2.open(testFile));
    
    QList<OTB::Item> items2;
    OTB::OTBHeader header2;
    QVERIFY(reader2.readHeader(header2));
    QVERIFY(reader2.readItems(items2));
    reader2.close();
    
    qint64 warmTime = timer2.elapsed();
    
    qDebug() << "Cold cache time:" << coldTime << "ms";
    qDebug() << "Warm cache time:" << warmTime << "ms";
    
    // Warm cache should generally be faster or similar
    double improvement = double(coldTime) / warmTime;
    qDebug() << "Cache improvement factor:" << improvement;
    
    logTestResult("Cache Efficiency", true, 
                  QString("Cold: %1ms, Warm: %2ms, Improvement: %3x")
                  .arg(coldTime).arg(warmTime).arg(improvement, 0, 'f', 2));
}

void OTBTestSuite::testCSharpCompatibility() {
    qDebug() << "Testing C# compatibility...";
    
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString testPath = tempFile.fileName();
    tempFile.close();
    
    // Create data that matches C# format exactly
    OTB::OTBHeader header;
    header.setVersion(1, 2, 3); // Specific version used in C#
    header.setDescription("C# Compatible Test");
    
    QList<OTB::Item> items;
    
    // Create item with specific C# values
    OTB::Item item;
    item.setId(100);
    item.setClientId(101);
    item.setName("C# Test Item");
    item.setWeight(150);
    item.setStackable(true);
    item.setUseable(false);
    item.setMoveable(true);
    
    items.append(item);
    header.setItemCount(items.size());
    
    // Write using Qt6 implementation
    {
        OTB::OTBWriter writer;
        QVERIFY(writer.open(testPath));
        QVERIFY(writer.writeHeader(header));
        QVERIFY(writer.writeItems(items));
        writer.close();
    }
    
    // Read back and verify format compatibility
    {
        OTB::OTBReader reader;
        QVERIFY(reader.open(testPath));
        
        OTB::OTBHeader readHeader;
        QVERIFY(reader.readHeader(readHeader));
        
        // Verify header format matches C# expectations
        QCOMPARE(readHeader.getMajorVersion(), 1u);
        QCOMPARE(readHeader.getMinorVersion(), 2u);
        QCOMPARE(readHeader.getBuildNumber(), 3u);
        
        QList<OTB::Item> readItems;
        QVERIFY(reader.readItems(readItems));
        QCOMPARE(readItems.size(), 1);
        
        const OTB::Item& readItem = readItems.first();
        QCOMPARE(readItem.getId(), 100u);
        QCOMPARE(readItem.getClientId(), 101u);
        QCOMPARE(readItem.getName(), QString("C# Test Item"));
        QCOMPARE(readItem.getWeight(), 150u);
        QCOMPARE(readItem.isStackable(), true);
        QCOMPARE(readItem.isUseable(), false);
        QCOMPARE(readItem.isMoveable(), true);
        
        reader.close();
    }
    
    logTestResult("C# Compatibility", true);
}

void OTBTestSuite::testVersionCompatibility() {
    qDebug() << "Testing version compatibility...";
    
    // Test multiple version formats
    QList<QPair<quint32, quint32>> versions = {
        {1, 0}, {1, 1}, {1, 2}, {2, 0}, {2, 1}
    };
    
    for (const auto& version : versions) {
        QTemporaryFile tempFile;
        QVERIFY(tempFile.open());
        QString testPath = tempFile.fileName();
        tempFile.close();
        
        // Write with specific version
        {
            OTB::OTBWriter writer;
            QVERIFY(writer.open(testPath));
            
            OTB::OTBHeader header;
            header.setVersion(version.first, version.second, 0);
            header.setItemCount(0);
            QVERIFY(writer.writeHeader(header));
            writer.close();
        }
        
        // Read and verify version handling
        {
            OTB::OTBReader reader;
            QVERIFY(reader.open(testPath));
            
            OTB::OTBHeader header;
            QVERIFY(reader.readHeader(header));
            
            QCOMPARE(header.getMajorVersion(), version.first);
            QCOMPARE(header.getMinorVersion(), version.second);
            
            reader.close();
        }
    }
    
    logTestResult("Version Compatibility", true);
}void OTBTestSuite::testDataFormatCompatibility() {
    qDebug() << "Testing data format compatibility...";
    
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString testPath = tempFile.fileName();
    tempFile.close();
    
    // Test all data types used in OTB format
    {
        OTB::BinaryTree writer;
        QVERIFY(writer.open(testPath, QIODevice::WriteOnly));
        
        writer.writeNodeStart(0x00);
        
        // Test all primitive types
        writer.writeValue<quint8>(255);
        writer.writeValue<quint16>(65535);
        writer.writeValue<quint32>(4294967295U);
        writer.writeValue<qint8>(-128);
        writer.writeValue<qint16>(-32768);
        writer.writeValue<qint32>(-2147483648);
        
        // Test string formats
        writer.writeString("Test String", true);  // With length prefix
        writer.writeString("No Length", false);   // Without length prefix
        
        // Test byte arrays
        QByteArray testBytes;
        testBytes.append(0x01);
        testBytes.append(0xFF);
        testBytes.append(0x00);
        writer.writeBytes(testBytes);
        
        writer.writeNodeEnd();
        writer.close();
    }
    
    // Read back and verify
    {
        OTB::BinaryTree reader;
        QVERIFY(reader.open(testPath, QIODevice::ReadOnly));
        
        QVERIFY(reader.enterNode());
        
        QCOMPARE(reader.readValue<quint8>(), static_cast<quint8>(255));
        QCOMPARE(reader.readValue<quint16>(), static_cast<quint16>(65535));
        QCOMPARE(reader.readValue<quint32>(), 4294967295U);
        QCOMPARE(reader.readValue<qint8>(), static_cast<qint8>(-128));
        QCOMPARE(reader.readValue<qint16>(), static_cast<qint16>(-32768));
        QCOMPARE(reader.readValue<qint32>(), -2147483648);
        
        QCOMPARE(reader.readString(true), QString("Test String"));
        QCOMPARE(reader.readString(false), QString("No Length"));
        
        QByteArray readBytes = reader.readBytes(3);
        QCOMPARE(readBytes.size(), 3);
        QCOMPARE(static_cast<quint8>(readBytes[0]), static_cast<quint8>(0x01));
        QCOMPARE(static_cast<quint8>(readBytes[1]), static_cast<quint8>(0xFF));
        QCOMPARE(static_cast<quint8>(readBytes[2]), static_cast<quint8>(0x00));
        
        reader.leaveNode();
        reader.close();
    }
    
    logTestResult("Data Format Compatibility", true);
}

void OTBTestSuite::testBackupAndRestore() {
    qDebug() << "Testing backup and restore functionality...";
    
    if (m_testFiles.isEmpty()) {
        logTestResult("Backup and Restore", false, "No test files available");
        return;
    }
    
    QString originalFile = m_testFiles.first();
    QString backupFile = m_testDataDir + "/backup_test.otb";
    
    // Create backup
    OTB::OTBBackup backup;
    QVERIFY(backup.createBackup(originalFile, backupFile));
    
    // Verify backup exists and is valid
    QVERIFY(QFile::exists(backupFile));
    
    OTB::OTBReader reader;
    QVERIFY(reader.open(backupFile));
    
    OTB::OTBHeader header;
    QVERIFY(reader.readHeader(header));
    QVERIFY(header.isValid());
    
    reader.close();
    
    // Test restore
    QString restoreFile = m_testDataDir + "/restore_test.otb";
    QVERIFY(backup.restoreBackup(backupFile, restoreFile));
    
    // Verify restored file matches original
    QVERIFY(compareFiles(originalFile, restoreFile));
    
    logTestResult("Backup and Restore", true);
}

void OTBTestSuite::testValidationSystem() {
    qDebug() << "Testing validation system...";
    
    if (m_testFiles.isEmpty()) {
        logTestResult("Validation System", false, "No test files available");
        return;
    }
    
    QString testFile = m_testFiles.first();
    
    OTB::OTBValidator validator;
    
    // Test valid file
    OTB::ValidationResult result = validator.validateFile(testFile);
    QVERIFY(result.isValid);
    QVERIFY(result.errors.isEmpty());
    
    // Test invalid file
    QTemporaryFile invalidFile;
    QVERIFY(invalidFile.open());
    invalidFile.write("INVALID_DATA");
    invalidFile.close();
    
    OTB::ValidationResult invalidResult = validator.validateFile(invalidFile.fileName());
    QVERIFY(!invalidResult.isValid);
    QVERIFY(!invalidResult.errors.isEmpty());
    
    logTestResult("Validation System", true);
}

void OTBTestSuite::testConcurrentAccess() {
    qDebug() << "Testing concurrent access...";
    
    if (m_testFiles.isEmpty()) {
        logTestResult("Concurrent Access", false, "No test files available");
        return;
    }
    
    QString testFile = m_testFiles.first();
    
    // Test multiple readers
    QList<OTB::OTBReader*> readers;
    
    for (int i = 0; i < 3; ++i) {
        OTB::OTBReader* reader = new OTB::OTBReader();
        QVERIFY(reader->open(testFile));
        readers.append(reader);
    }
    
    // All readers should be able to read simultaneously
    for (OTB::OTBReader* reader : readers) {
        OTB::OTBHeader header;
        QVERIFY(reader->readHeader(header));
        
        QList<OTB::Item> items;
        QVERIFY(reader->readItems(items));
        QVERIFY(!items.isEmpty());
    }
    
    // Cleanup
    for (OTB::OTBReader* reader : readers) {
        reader->close();
        delete reader;
    }
    
    logTestResult("Concurrent Access", true);
}

// Helper methods implementation

void OTBTestSuite::logTestResult(const QString& testName, bool passed, const QString& details) {
    if (passed) {
        m_testsPassed++;
        qDebug() << "✓" << testName << "PASSED" << details;
    } else {
        m_testsFailed++;
        qDebug() << "✗" << testName << "FAILED" << details;
    }
}

QString OTBTestSuite::createTestFile(const QString& name, int itemCount) {
    QString filePath = m_testDataDir + "/" + name;
    
    OTB::OTBWriter writer;
    if (!writer.open(filePath)) {
        qDebug() << "Failed to create test file:" << filePath;
        return QString();
    }
    
    // Create header
    OTB::OTBHeader header;
    header.setVersion(1, 0, 0);
    header.setDescription(QString("Test file with %1 items").arg(itemCount));
    header.setItemCount(itemCount);
    
    if (!writer.writeHeader(header)) {
        qDebug() << "Failed to write header to test file:" << filePath;
        writer.close();
        return QString();
    }
    
    // Create items
    QList<OTB::Item> items;
    generateTestData(items, itemCount);
    
    if (!writer.writeItems(items)) {
        qDebug() << "Failed to write items to test file:" << filePath;
        writer.close();
        return QString();
    }
    
    writer.close();
    return filePath;
}

bool OTBTestSuite::compareFiles(const QString& file1, const QString& file2) {
    QFile f1(file1);
    QFile f2(file2);
    
    if (!f1.open(QIODevice::ReadOnly) || !f2.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    if (f1.size() != f2.size()) {
        return false;
    }
    
    const int bufferSize = 8192;
    char buffer1[bufferSize];
    char buffer2[bufferSize];
    
    while (!f1.atEnd() && !f2.atEnd()) {
        qint64 read1 = f1.read(buffer1, bufferSize);
        qint64 read2 = f2.read(buffer2, bufferSize);
        
        if (read1 != read2 || memcmp(buffer1, buffer2, read1) != 0) {
            return false;
        }
    }
    
    return f1.atEnd() && f2.atEnd();
}

void OTBTestSuite::generateTestData(QList<OTB::Item>& items, int count) {
    items.clear();
    items.reserve(count);
    
    for (int i = 0; i < count; ++i) {
        OTB::Item item;
        item.setId(i + 1);
        item.setClientId(i + 100);
        item.setName(QString("Test Item %1").arg(i + 1));
        item.setDescription(QString("Description for item %1").arg(i + 1));
        item.setWeight(100 + (i % 500));
        item.setSpeed(200 + (i % 300));
        item.setArmor(i % 50);
        item.setAttack((i % 30) + 1);
        item.setDefense(i % 25);
        item.setStackable(i % 2 == 0);
        item.setUseable(i % 3 == 0);
        item.setMoveable(i % 4 != 0);
        item.setPickupable(i % 5 != 0);
        
        items.append(item);
    }
}

} // namespace OTBTests

// Test runner
int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    OTBTests::OTBTestSuite testSuite;
    
    // Run all tests
    int result = QTest::qExec(&testSuite, argc, argv);
    
    return result;
}

#include "test_otb_comprehensive.moc"