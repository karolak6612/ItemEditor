#include "otbperformance.h"
#include "otbcache.h"
#include "otbreader.h"
#include "otbwriter.h"
#include "item.h"
#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QDir>
#include <QTemporaryFile>
#include <iostream>

using namespace OTB;

class PerformanceTest {
public:
    static bool runAllTests() {
        std::cout << "=== OTB Performance Optimization Tests ===" << std::endl;
        
        bool allPassed = true;
        allPassed &= testCacheSystem();
        allPassed &= testIOBuffer();
        allPassed &= testPerformanceMonitor();
        allPassed &= testPerformanceOptimizer();
        allPassed &= testIntegratedPerformance();
        
        std::cout << "\n=== Test Results ===" << std::endl;
        std::cout << "All tests " << (allPassed ? "PASSED" : "FAILED") << std::endl;
        
        return allPassed;
    }

private:
    static bool testCacheSystem() {
        std::cout << "\n--- Testing Cache System ---" << std::endl;
        
        try {
            // Test cache creation and configuration
            auto cache = CacheFactory::createCache(CacheFactory::Balanced);
            if (!cache) {
                std::cout << "FAIL: Failed to create cache" << std::endl;
                return false;
            }
            
            // Test item caching
            ServerItem testItem;
            testItem.id = 100;
            testItem.name = "Test Item";
            testItem.clientId = 200;
            testItem.type = ServerItemType::Ground;
            
            bool cached = cache->cacheItem(testItem.id, testItem, OtbCache::L2_Medium);
            if (!cached) {
                std::cout << "FAIL: Failed to cache item" << std::endl;
                return false;
            }
            
            // Test item retrieval
            ServerItem retrievedItem;
            bool retrieved = cache->getCachedItem(testItem.id, retrievedItem, OtbCache::L2_Medium);
            if (!retrieved) {
                std::cout << "FAIL: Failed to retrieve cached item" << std::endl;
                return false;
            }
            
            if (retrievedItem.id != testItem.id || retrievedItem.name != testItem.name) {
                std::cout << "FAIL: Retrieved item doesn't match cached item" << std::endl;
                return false;
            }
            
            // Test cache statistics
            const CacheStats& stats = cache->getStats(OtbCache::L2_Medium);
            if (stats.hits == 0) {
                std::cout << "FAIL: Cache statistics not updated" << std::endl;
                return false;
            }
            
            // Test cache eviction
            cache->setCacheSize(OtbCache::L2_Medium, 1);
            ServerItem testItem2;
            testItem2.id = 101;
            testItem2.name = "Test Item 2";
            cache->cacheItem(testItem2.id, testItem2, OtbCache::L2_Medium);
            
            // Test cache report
            QStringList report = cache->getCacheReport();
            if (report.isEmpty()) {
                std::cout << "FAIL: Cache report is empty" << std::endl;
                return false;
            }
            
            std::cout << "PASS: Cache system tests completed successfully" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "FAIL: Exception in cache system test: " << e.what() << std::endl;
            return false;
        }
    }
    
    static bool testIOBuffer() {
        std::cout << "\n--- Testing I/O Buffer ---" << std::endl;
        
        try {
            IOBuffer buffer(1024);
            
            // Test buffer properties
            if (buffer.size() != 1024) {
                std::cout << "FAIL: Buffer size not set correctly" << std::endl;
                return false;
            }
            
            // Test buffer with temporary file
            QTemporaryFile tempFile;
            if (!tempFile.open()) {
                std::cout << "FAIL: Failed to create temporary file" << std::endl;
                return false;
            }
            
            // Test writing to buffer
            QByteArray testData = "Hello, World! This is a test of the I/O buffer system.";
            bool written = buffer.write(&tempFile, testData);
            if (!written) {
                std::cout << "FAIL: Failed to write to buffer" << std::endl;
                return false;
            }
            
            // Test buffer flush
            buffer.flush(&tempFile);
            tempFile.close();
            
            // Test reading from file
            if (!tempFile.open()) {
                std::cout << "FAIL: Failed to reopen temporary file" << std::endl;
                return false;
            }
            
            QByteArray readData = tempFile.readAll();
            if (readData != testData) {
                std::cout << "FAIL: Read data doesn't match written data" << std::endl;
                return false;
            }
            
            // Test buffer statistics
            if (buffer.getBytesWritten() == 0) {
                std::cout << "FAIL: Buffer statistics not updated" << std::endl;
                return false;
            }
            
            std::cout << "PASS: I/O Buffer tests completed successfully" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "FAIL: Exception in I/O buffer test: " << e.what() << std::endl;
            return false;
        }
    }
    
    static bool testPerformanceMonitor() {
        std::cout << "\n--- Testing Performance Monitor ---" << std::endl;
        
        try {
            PerformanceMonitor monitor;
            
            // Test monitoring start/stop
            monitor.startMonitoring();
            if (!monitor.isMonitoring()) {
                std::cout << "FAIL: Monitor not started" << std::endl;
                return false;
            }
            
            // Test metric recording
            monitor.recordReadOperation(1024, 100);
            monitor.recordWriteOperation(512, 50);
            monitor.recordCacheHit();
            monitor.recordCacheMiss();
            monitor.recordMemoryUsage(1024 * 1024);
            
            // Test metrics retrieval
            const PerformanceMetrics& metrics = monitor.getCurrentMetrics();
            if (metrics.bytesRead != 1024 || metrics.bytesWritten != 512) {
                std::cout << "FAIL: Metrics not recorded correctly" << std::endl;
                return false;
            }
            
            if (metrics.cacheHits != 1 || metrics.cacheMisses != 1) {
                std::cout << "FAIL: Cache metrics not recorded correctly" << std::endl;
                return false;
            }
            
            // Test performance report
            QString report = monitor.generateReport();
            if (report.isEmpty()) {
                std::cout << "FAIL: Performance report is empty" << std::endl;
                return false;
            }
            
            // Test optimization suggestions
            QStringList suggestions = monitor.getOptimizationSuggestions();
            // Suggestions may be empty for good performance, so we just check it doesn't crash
            
            monitor.stopMonitoring();
            if (monitor.isMonitoring()) {
                std::cout << "FAIL: Monitor not stopped" << std::endl;
                return false;
            }
            
            std::cout << "PASS: Performance Monitor tests completed successfully" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "FAIL: Exception in performance monitor test: " << e.what() << std::endl;
            return false;
        }
    }
    
    static bool testPerformanceOptimizer() {
        std::cout << "\n--- Testing Performance Optimizer ---" << std::endl;
        
        try {
            PerformanceOptimizer optimizer;
            
            // Test optimization methods
            optimizer.optimizeForMemory();
            optimizer.optimizeForSpeed();
            optimizer.optimizeForBalance();
            optimizer.optimizeForFileSize(10 * 1024 * 1024); // 10MB
            
            // Test optimal calculations
            qint64 bufferSize = optimizer.getOptimalBufferSize(1024 * 1024);
            if (bufferSize <= 0) {
                std::cout << "FAIL: Invalid optimal buffer size" << std::endl;
                return false;
            }
            
            qint64 cacheSize = optimizer.getOptimalCacheSize(512 * 1024 * 1024);
            if (cacheSize <= 0) {
                std::cout << "FAIL: Invalid optimal cache size" << std::endl;
                return false;
            }
            
            int threadCount = optimizer.getOptimalThreadCount();
            if (threadCount <= 0 || threadCount > 32) {
                std::cout << "FAIL: Invalid optimal thread count: " << threadCount << std::endl;
                return false;
            }
            
            // Test thread pool configuration
            optimizer.configureThreadPool();
            
            std::cout << "PASS: Performance Optimizer tests completed successfully" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "FAIL: Exception in performance optimizer test: " << e.what() << std::endl;
            return false;
        }
    }
    
    static bool testIntegratedPerformance() {
        std::cout << "\n--- Testing Integrated Performance ---" << std::endl;
        
        try {
            // Initialize performance management
            PerformanceManager::initialize();
            
            // Test global manager access
            PerformanceMonitor* monitor = PerformanceManager::getMonitor();
            PerformanceOptimizer* optimizer = PerformanceManager::getOptimizer();
            
            if (!monitor || !optimizer) {
                std::cout << "FAIL: Failed to get performance manager instances" << std::endl;
                return false;
            }
            
            // Test performance profiles
            PerformanceManager::applyLowMemoryProfile();
            PerformanceManager::applyHighPerformanceProfile();
            PerformanceManager::applyBalancedProfile();
            
            // Create test data
            ServerItemList testItems;
            testItems.majorVersion = 1;
            testItems.minorVersion = 0;
            testItems.buildNumber = 1;
            testItems.clientVersion = 860;
            testItems.description = "Test OTB";
            
            // Add some test items
            for (int i = 1; i <= 10; ++i) {
                ServerItem item;
                item.id = i;
                item.name = QString("Test Item %1").arg(i);
                item.clientId = i + 100;
                item.type = ServerItemType::Ground;
                item.flags = 0;
                testItems.add(item);
            }
            
            // Test performance-optimized reading/writing
            QTemporaryFile tempFile;
            tempFile.setFileTemplate(QDir::tempPath() + "/otb_perf_test_XXXXXX.otb");
            if (!tempFile.open()) {
                std::cout << "FAIL: Failed to create temporary OTB file" << std::endl;
                return false;
            }
            tempFile.close();
            
            QString tempFilePath = tempFile.fileName();
            
            // Test writing with performance monitoring
            monitor->startMonitoring();
            
            OtbWriter writer;
            writer.setPerformanceMonitoring(true);
            writer.setBufferSize(optimizer->getOptimalBufferSize(1024));
            
            QString writeError;
            QElapsedTimer writeTimer;
            writeTimer.start();
            
            bool writeSuccess = writer.write(tempFilePath, testItems, writeError);
            qint64 writeTime = writeTimer.elapsed();
            
            if (!writeSuccess) {
                std::cout << "FAIL: Failed to write OTB file: " << writeError.toStdString() << std::endl;
                return false;
            }
            
            // Test reading with performance monitoring
            OtbReader reader;
            reader.setPerformanceMonitoring(true);
            reader.setCacheEnabled(true);
            reader.setBufferSize(optimizer->getOptimalBufferSize(QFileInfo(tempFilePath).size()));
            
            ServerItemList readItems;
            QString readError;
            QElapsedTimer readTimer;
            readTimer.start();
            
            bool readSuccess = reader.read(tempFilePath, readItems, readError);
            qint64 readTime = readTimer.elapsed();
            
            if (!readSuccess) {
                std::cout << "FAIL: Failed to read OTB file: " << readError.toStdString() << std::endl;
                return false;
            }
            
            // Verify data integrity
            if (readItems.items.size() != testItems.items.size()) {
                std::cout << "FAIL: Item count mismatch after read/write" << std::endl;
                return false;
            }
            
            for (int i = 0; i < testItems.items.size(); ++i) {
                const ServerItem& original = testItems.items[i];
                const ServerItem& read = readItems.items[i];
                
                if (original.id != read.id || original.name != read.name) {
                    std::cout << "FAIL: Item data mismatch after read/write" << std::endl;
                    return false;
                }
            }
            
            // Test performance metrics
            const PerformanceMetrics& readerMetrics = reader.getLastPerformanceMetrics();
            const PerformanceMetrics& writerMetrics = writer.getLastPerformanceMetrics();
            
            if (readerMetrics.bytesRead == 0 || writerMetrics.bytesWritten == 0) {
                std::cout << "FAIL: Performance metrics not recorded" << std::endl;
                return false;
            }
            
            // Generate performance report
            QString report = monitor->generateReport();
            std::cout << "Performance Report:" << std::endl;
            std::cout << report.toStdString() << std::endl;
            
            // Test cache effectiveness
            OtbCache* cache = CacheManager::getInstance();
            CacheStats cacheStats = cache->getCombinedStats();
            
            std::cout << "Cache Statistics:" << std::endl;
            std::cout << "  Hits: " << cacheStats.hits << std::endl;
            std::cout << "  Misses: " << cacheStats.misses << std::endl;
            std::cout << "  Hit Ratio: " << (cacheStats.hitRatio() * 100) << "%" << std::endl;
            
            monitor->stopMonitoring();
            
            // Cleanup
            QFile::remove(tempFilePath);
            PerformanceManager::shutdown();
            
            std::cout << "PASS: Integrated Performance tests completed successfully" << std::endl;
            std::cout << "Write Time: " << writeTime << "ms" << std::endl;
            std::cout << "Read Time: " << readTime << "ms" << std::endl;
            
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "FAIL: Exception in integrated performance test: " << e.what() << std::endl;
            return false;
        }
    }
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    bool success = PerformanceTest::runAllTests();
    
    return success ? 0 : 1;
}