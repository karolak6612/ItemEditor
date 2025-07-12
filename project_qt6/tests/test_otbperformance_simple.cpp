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

class SimplePerformanceTest {
public:
    static bool runBasicTests() {
        std::cout << "=== OTB Performance Optimization Basic Tests ===" << std::endl;
        
        bool allPassed = true;
        allPassed &= testIOBuffer();
        allPassed &= testBasicCache();
        allPassed &= testPerformanceMetrics();
        
        std::cout << "\n=== Test Results ===" << std::endl;
        std::cout << "All tests " << (allPassed ? "PASSED" : "FAILED") << std::endl;
        
        return allPassed;
    }

private:
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
    
    static bool testBasicCache() {
        std::cout << "\n--- Testing Basic Cache ---" << std::endl;
        
        try {
            // Test cache creation
            auto cache = CacheFactory::createCache(CacheFactory::Balanced);
            if (!cache) {
                std::cout << "FAIL: Failed to create cache" << std::endl;
                return false;
            }
            
            // Test basic cache operations without signals
            cache->setMaxMemoryUsage(10 * 1024 * 1024); // 10MB
            cache->setCacheSize(OtbCache::L2_Medium, 100);
            
            // Test cache statistics
            const CacheStats& stats = cache->getStats(OtbCache::L2_Medium);
            if (stats.creationTime.isNull()) {
                std::cout << "FAIL: Cache statistics not initialized" << std::endl;
                return false;
            }
            
            // Test cache report
            QStringList report = cache->getCacheReport();
            if (report.isEmpty()) {
                std::cout << "FAIL: Cache report is empty" << std::endl;
                return false;
            }
            
            std::cout << "PASS: Basic Cache tests completed successfully" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "FAIL: Exception in basic cache test: " << e.what() << std::endl;
            return false;
        }
    }
    
    static bool testPerformanceMetrics() {
        std::cout << "\n--- Testing Performance Metrics ---" << std::endl;
        
        try {
            PerformanceMetrics metrics;
            
            // Test metrics initialization
            if (metrics.bytesRead != 0 || metrics.bytesWritten != 0) {
                std::cout << "FAIL: Metrics not initialized to zero" << std::endl;
                return false;
            }
            
            // Test metrics calculations
            metrics.bytesRead = 1024;
            metrics.totalReadTime = 100;
            metrics.bytesWritten = 512;
            metrics.totalWriteTime = 50;
            metrics.cacheHits = 10;
            metrics.cacheMisses = 5;
            
            double readSpeed = metrics.averageReadSpeed();
            double writeSpeed = metrics.averageWriteSpeed();
            double hitRatio = metrics.cacheHitRatio();
            
            if (readSpeed <= 0 || writeSpeed <= 0) {
                std::cout << "FAIL: Invalid speed calculations" << std::endl;
                return false;
            }
            
            if (hitRatio < 0.6 || hitRatio > 0.7) { // Should be 10/15 = 0.666...
                std::cout << "FAIL: Invalid cache hit ratio calculation" << std::endl;
                return false;
            }
            
            // Test metrics reset
            metrics.reset();
            if (metrics.bytesRead != 0 || metrics.cacheHits != 0) {
                std::cout << "FAIL: Metrics not reset properly" << std::endl;
                return false;
            }
            
            std::cout << "PASS: Performance Metrics tests completed successfully" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "FAIL: Exception in performance metrics test: " << e.what() << std::endl;
            return false;
        }
    }
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    bool success = SimplePerformanceTest::runBasicTests();
    
    return success ? 0 : 1;
}