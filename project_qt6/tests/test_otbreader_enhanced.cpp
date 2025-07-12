#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>
#include "otbreader.h"
#include "otbwriter.h"
#include "otbtypes.h"

using namespace OTB;

class OtbReaderTester {
public:
    static bool runAllTests() {
        qDebug() << "=== Enhanced OTB Reader Tests ===";
        
        bool allPassed = true;
        allPassed &= testBasicDeserialization();
        allPassed &= testErrorHandling();
        allPassed &= testDataValidation();
        allPassed &= testCorruptionDetection();
        allPassed &= testPerformanceMonitoring();
        allPassed &= testCompatibilityWithCSharp();
        
        qDebug() << "=== Test Results ===";
        qDebug() << (allPassed ? "ALL TESTS PASSED" : "SOME TESTS FAILED");
        
        return allPassed;
    }

private:
    static bool testBasicDeserialization() {
        qDebug() << "\n--- Testing Basic Deserialization ---";
        
        // Create test data
        ServerItemList testItems = createTestData();
        
        // Write test file
        QTemporaryFile tempFile;
        if (!tempFile.open()) {
            qDebug() << "FAIL: Could not create temporary file";
            return false;
        }
        
        OtbWriter writer;
        QString writeError;
        if (!writer.write(tempFile.fileName(), testItems, writeError)) {
            qDebug() << "FAIL: Could not write test file:" << writeError;
            return false;
        }
        
        // Read back with enhanced reader
        OtbReader reader;
        reader.setDetailedLogging(true);
        reader.setStrictValidation(true);
        
        ServerItemList readItems;
        QString readError;
        
        if (!reader.read(tempFile.fileName(), readItems, readError)) {
            qDebug() << "FAIL: Could not read test file:" << readError;
            return false;
        }
        
        // Verify data integrity
        if (!compareItemLists(testItems, readItems)) {
            qDebug() << "FAIL: Data integrity check failed";
            return false;
        }
        
        // Check statistics
        const ReadingStats& stats = reader.getLastReadingStats();
        qDebug() << "Reading stats:";
        qDebug() << "  Items processed:" << stats.itemsProcessed;
        qDebug() << "  Attributes processed:" << stats.attributesProcessed;
        qDebug() << "  Reading time:" << stats.readingTimeMs << "ms";
        qDebug() << "  Warnings:" << stats.warnings.size();
        
        if (stats.itemsProcessed != testItems.items.size()) {
            qDebug() << "FAIL: Item count mismatch";
            return false;
        }
        
        qDebug() << "PASS: Basic deserialization test";
        return true;
    }
    
    static bool testErrorHandling() {
        qDebug() << "\n--- Testing Error Handling ---";
        
        OtbReader reader;
        reader.setStrictValidation(true);
        
        ServerItemList items;
        QString errorString;
        
        // Test non-existent file
        if (reader.read("non_existent_file.otb", items, errorString)) {
            qDebug() << "FAIL: Should have failed for non-existent file";
            return false;
        }
        
        if (reader.getLastErrorCode() != OtbReadError::FileNotFound) {
            qDebug() << "FAIL: Wrong error code for non-existent file";
            return false;
        }
        
        // Test empty file
        QTemporaryFile emptyFile;
        if (emptyFile.open()) {
            emptyFile.close(); // Create empty file
            
            if (reader.validateFileIntegrity(emptyFile.fileName(), errorString)) {
                qDebug() << "FAIL: Should have failed for empty file";
                return false;
            }
            
            if (reader.getLastErrorCode() != OtbReadError::CorruptedData) {
                qDebug() << "FAIL: Wrong error code for empty file";
                return false;
            }
        }
        
        qDebug() << "PASS: Error handling test";
        return true;
    }
    
    static bool testDataValidation() {
        qDebug() << "\n--- Testing Data Validation ---";
        
        // Create test data with edge cases
        ServerItemList testItems;
        testItems.majorVersion = 3;
        testItems.minorVersion = 770;
        testItems.buildNumber = 1;
        testItems.description = "Validation Test";
        
        // Add item with edge case values
        ServerItem item;
        item.id = 65000; // High ID
        item.clientId = 65000; // High client ID
        item.type = ServerItemType::Ground;
        item.name = QString("A").repeated(200); // Long name
        item.groundSpeed = 999; // High speed
        item.spriteHash.fill(0xFF, 16);
        item.updateFlagsFromProperties();
        
        testItems.add(item);
        
        // Write and read with validation
        QTemporaryFile tempFile;
        if (!tempFile.open()) {
            qDebug() << "FAIL: Could not create temporary file";
            return false;
        }
        
        OtbWriter writer;
        QString writeError;
        if (!writer.write(tempFile.fileName(), testItems, writeError)) {
            qDebug() << "FAIL: Could not write validation test file:" << writeError;
            return false;
        }
        
        OtbReader reader;
        reader.setStrictValidation(true);
        reader.setDetailedLogging(true);
        
        ServerItemList readItems;
        QString readError;
        
        if (!reader.read(tempFile.fileName(), readItems, readError)) {
            qDebug() << "FAIL: Could not read validation test file:" << readError;
            return false;
        }
        
        // Check for warnings about edge case values
        const ReadingStats& stats = reader.getLastReadingStats();
        if (stats.warnings.isEmpty()) {
            qDebug() << "WARN: Expected validation warnings for edge case values";
        }
        
        qDebug() << "PASS: Data validation test";
        return true;
    }
    
    static bool testCorruptionDetection() {
        qDebug() << "\n--- Testing Corruption Detection ---";
        
        // Create a valid file first
        ServerItemList testItems = createTestData();
        
        QTemporaryFile tempFile;
        if (!tempFile.open()) {
            qDebug() << "FAIL: Could not create temporary file";
            return false;
        }
        
        OtbWriter writer;
        QString writeError;
        if (!writer.write(tempFile.fileName(), testItems, writeError)) {
            qDebug() << "FAIL: Could not write test file:" << writeError;
            return false;
        }
        
        tempFile.close();
        
        // Corrupt the file by truncating it
        QFile corruptFile(tempFile.fileName());
        if (corruptFile.open(QIODevice::ReadWrite)) {
            qint64 size = corruptFile.size();
            corruptFile.resize(size / 2); // Truncate to half size
            corruptFile.close();
        }
        
        // Try to read corrupted file
        OtbReader reader;
        reader.setStrictValidation(true);
        
        ServerItemList items;
        QString errorString;
        
        if (reader.read(tempFile.fileName(), items, errorString)) {
            qDebug() << "FAIL: Should have failed for corrupted file";
            return false;
        }
        
        qDebug() << "Corruption detected correctly:" << errorString;
        qDebug() << "PASS: Corruption detection test";
        return true;
    }
    
    static bool testPerformanceMonitoring() {
        qDebug() << "\n--- Testing Performance Monitoring ---";
        
        // Create larger test data
        ServerItemList testItems;
        testItems.majorVersion = 3;
        testItems.minorVersion = 860;
        testItems.buildNumber = 1;
        testItems.description = "Performance Test";
        
        // Add multiple items
        for (int i = 1; i <= 100; ++i) {
            ServerItem item;
            item.id = i;
            item.clientId = i + 1000;
            item.type = ServerItemType::Ground;
            item.name = QString("Test Item %1").arg(i);
            item.groundSpeed = 150;
            item.spriteHash.fill(static_cast<char>(i % 256), 16);
            item.updateFlagsFromProperties();
            testItems.add(item);
        }
        
        QTemporaryFile tempFile;
        if (!tempFile.open()) {
            qDebug() << "FAIL: Could not create temporary file";
            return false;
        }
        
        OtbWriter writer;
        QString writeError;
        if (!writer.write(tempFile.fileName(), testItems, writeError)) {
            qDebug() << "FAIL: Could not write performance test file:" << writeError;
            return false;
        }
        
        // Read with performance monitoring
        OtbReader reader;
        reader.setDetailedLogging(true);
        
        ServerItemList readItems;
        QString readError;
        
        if (!reader.read(tempFile.fileName(), readItems, readError)) {
            qDebug() << "FAIL: Could not read performance test file:" << readError;
            return false;
        }
        
        const ReadingStats& stats = reader.getLastReadingStats();
        qDebug() << "Performance stats for 100 items:";
        qDebug() << "  Reading time:" << stats.readingTimeMs << "ms";
        qDebug() << "  Bytes read:" << stats.bytesRead;
        qDebug() << "  Items/second:" << (stats.itemsProcessed * 1000.0 / qMax(stats.readingTimeMs, 1LL));
        
        if (stats.readingTimeMs > 5000) { // Should not take more than 5 seconds
            qDebug() << "WARN: Reading took longer than expected";
        }
        
        qDebug() << "PASS: Performance monitoring test";
        return true;
    }
    
    static bool testCompatibilityWithCSharp() {
        qDebug() << "\n--- Testing C# Compatibility ---";
        
        // Create data that matches C# format exactly
        ServerItemList testItems;
        testItems.majorVersion = 3;
        testItems.minorVersion = 770;
        testItems.buildNumber = 1;
        testItems.clientVersion = 1098;
        testItems.description = "C# Compatibility Test";
        
        // Add item with all attributes
        ServerItem item;
        item.id = 100;
        item.clientId = 100;
        item.type = ServerItemType::Ground;
        item.name = "Test Ground Item";
        item.flags = ServerItemFlag::Unpassable | ServerItemFlag::BlockMissiles;
        item.groundSpeed = 150;
        item.spriteHash.fill(0x42, 16);
        item.minimapColor = 255;
        item.lightLevel = 5;
        item.lightColor = 0xFF00;
        item.stackOrder = TileStackOrder::Ground;
        item.hasStackOrder = true;
        item.tradeAs = 50;
        item.maxReadWriteChars = 100;
        item.maxReadChars = 50;
        item.updatePropertiesFromFlags();
        
        testItems.add(item);
        
        QTemporaryFile tempFile;
        if (!tempFile.open()) {
            qDebug() << "FAIL: Could not create temporary file";
            return false;
        }
        
        OtbWriter writer;
        QString writeError;
        if (!writer.write(tempFile.fileName(), testItems, writeError)) {
            qDebug() << "FAIL: Could not write C# compatibility test file:" << writeError;
            return false;
        }
        
        // Read back and verify all attributes
        OtbReader reader;
        reader.setStrictValidation(true);
        reader.setDetailedLogging(true);
        
        ServerItemList readItems;
        QString readError;
        
        if (!reader.read(tempFile.fileName(), readItems, readError)) {
            qDebug() << "FAIL: Could not read C# compatibility test file:" << readError;
            return false;
        }
        
        if (readItems.items.size() != 1) {
            qDebug() << "FAIL: Expected 1 item, got" << readItems.items.size();
            return false;
        }
        
        const ServerItem& readItem = readItems.items.first();
        
        // Verify all attributes match
        if (readItem.id != item.id ||
            readItem.clientId != item.clientId ||
            readItem.type != item.type ||
            readItem.name != item.name ||
            readItem.flags != item.flags ||
            readItem.groundSpeed != item.groundSpeed ||
            readItem.spriteHash != item.spriteHash ||
            readItem.minimapColor != item.minimapColor ||
            readItem.lightLevel != item.lightLevel ||
            readItem.lightColor != item.lightColor ||
            readItem.stackOrder != item.stackOrder ||
            readItem.tradeAs != item.tradeAs ||
            readItem.maxReadWriteChars != item.maxReadWriteChars ||
            readItem.maxReadChars != item.maxReadChars) {
            
            qDebug() << "FAIL: Attribute mismatch in C# compatibility test";
            return false;
        }
        
        qDebug() << "PASS: C# compatibility test";
        return true;
    }
    
    static ServerItemList createTestData() {
        ServerItemList testItems;
        testItems.majorVersion = 3;
        testItems.minorVersion = 770;
        testItems.buildNumber = 1;
        testItems.clientVersion = 1098;
        testItems.description = "Enhanced Reader Test";
        
        // Add a test item
        ServerItem testItem;
        testItem.id = 100;
        testItem.clientId = 100;
        testItem.type = ServerItemType::Ground;
        testItem.name = "Test Ground Item";
        testItem.flags = ServerItemFlag::Unpassable | ServerItemFlag::BlockMissiles;
        testItem.groundSpeed = 150;
        testItem.spriteHash.fill(0x42, 16);
        testItem.updatePropertiesFromFlags();
        
        testItems.add(testItem);
        
        return testItems;
    }
    
    static bool compareItemLists(const ServerItemList& list1, const ServerItemList& list2) {
        if (list1.majorVersion != list2.majorVersion ||
            list1.minorVersion != list2.minorVersion ||
            list1.buildNumber != list2.buildNumber ||
            list1.description != list2.description ||
            list1.items.size() != list2.items.size()) {
            return false;
        }
        
        for (int i = 0; i < list1.items.size(); ++i) {
            const ServerItem& item1 = list1.items[i];
            const ServerItem& item2 = list2.items[i];
            
            if (item1.id != item2.id ||
                item1.clientId != item2.clientId ||
                item1.type != item2.type ||
                item1.name != item2.name ||
                item1.flags != item2.flags ||
                item1.groundSpeed != item2.groundSpeed ||
                item1.spriteHash != item2.spriteHash) {
                return false;
            }
        }
        
        return true;
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    return OtbReaderTester::runAllTests() ? 0 : 1;
}