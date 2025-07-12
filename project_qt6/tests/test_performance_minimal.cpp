#include "otbperformance.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QTemporaryFile>
#include <iostream>

using namespace OTB;

class MinimalPerformanceTest {
public:
    static bool runTests() {
        std::cout << "=== OTB Performance Optimization Tests ===" << std::endl;
        
        bool allPassed = true;
        allPassed &= testIOBuffer();
        allPassed &= testPerformanceMetrics();
        allPassed &= testMemoryPool();
        
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
            
            if (buffer.available() != 1024) {
                std::cout << "FAIL: Buffer available space not correct" << std::endl;
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
                std::cout << "Expected: " << testData.toStdString() << std::endl;
                std::cout << "Got: " << readData.toStdString() << std::endl;
                return false;
            }
            
            // Test buffer statistics
            if (buffer.getBytesWritten() == 0) {
                std::cout << "FAIL: Buffer statistics not updated" << std::endl;
                return false;
            }
            
            std::cout << "PASS: I/O Buffer tests completed successfully" << std::endl;
            std::cout << "  Bytes written: " << buffer.getBytesWritten() << std::endl;
            std::cout << "  Flush count: " << buffer.getFlushCount() << std::endl;
            
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "FAIL: Exception in I/O buffer test: " << e.what() << std::endl;
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
            metrics.itemsProcessed = 20;
            metrics.totalParseTime = 200;
            
            double readSpeed = metrics.averageReadSpeed();
            double writeSpeed = metrics.averageWriteSpeed();
            double hitRatio = metrics.cacheHitRatio();
            double avgItemTime = metrics.averageItemProcessingTime();
            
            if (readSpeed <= 0 || writeSpeed <= 0) {
                std::cout << "FAIL: Invalid speed calculations" << std::endl;
                std::cout << "Read speed: " << readSpeed << ", Write speed: " << writeSpeed << std::endl;
                return false;
            }
            
            if (hitRatio < 0.6 || hitRatio > 0.7) { // Should be 10/15 = 0.666...
                std::cout << "FAIL: Invalid cache hit ratio calculation: " << hitRatio << std::endl;
                return false;
            }
            
            if (avgItemTime != 10.0) { // 200ms / 20 items = 10ms per item
                std::cout << "FAIL: Invalid average item processing time: " << avgItemTime << std::endl;
                return false;
            }
            
            // Test metrics reset
            metrics.reset();
            if (metrics.bytesRead != 0 || metrics.cacheHits != 0) {
                std::cout << "FAIL: Metrics not reset properly" << std::endl;
                return false;
            }
            
            std::cout << "PASS: Performance Metrics tests completed successfully" << std::endl;
            std::cout << "  Read speed: " << readSpeed << " bytes/sec" << std::endl;
            std::cout << "  Write speed: " << writeSpeed << " bytes/sec" << std::endl;
            std::cout << "  Cache hit ratio: " << (hitRatio * 100) << "%" << std::endl;
            std::cout << "  Avg item processing time: " << avgItemTime << " ms" << std::endl;
            
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "FAIL: Exception in performance metrics test: " << e.what() << std::endl;
            return false;
        }
    }
    
    static bool testMemoryPool() {
        std::cout << "\n--- Testing Memory Pool ---" << std::endl;
        
        try {
            // Test with a simple struct
            struct TestObject {
                int value = 0;
                void reset() { value = 0; }
            };
            
            MemoryPool<TestObject> pool(5, 10);
            
            // Test pool initial state
            if (pool.poolSize() != 5) {
                std::cout << "FAIL: Pool not initialized with correct size" << std::endl;
                return false;
            }
            
            // Test object acquisition
            auto obj1 = pool.acquire();
            auto obj2 = pool.acquire();
            
            if (!obj1 || !obj2) {
                std::cout << "FAIL: Failed to acquire objects from pool" << std::endl;
                return false;
            }
            
            // Test object usage
            obj1->value = 42;
            obj2->value = 84;
            
            if (obj1->value != 42 || obj2->value != 84) {
                std::cout << "FAIL: Object values not set correctly" << std::endl;
                return false;
            }
            
            // Test object release
            pool.release(std::move(obj1));
            pool.release(std::move(obj2));
            
            // Pool should have objects back (may be 5 or more depending on implementation)
            if (pool.poolSize() < 5) {
                std::cout << "FAIL: Objects not returned to pool properly" << std::endl;
                return false;
            }
            
            // Test pool clearing
            pool.clear();
            if (pool.poolSize() != 0) {
                std::cout << "FAIL: Pool not cleared properly" << std::endl;
                return false;
            }
            
            std::cout << "PASS: Memory Pool tests completed successfully" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "FAIL: Exception in memory pool test: " << e.what() << std::endl;
            return false;
        }
    }
};

int main(int argc, char *argv[]) {
    Q_UNUSED(argc)
    Q_UNUSED(argv)
    
    std::cout << "Starting OTB Performance Optimization Tests..." << std::endl;
    
    bool success = MinimalPerformanceTest::runTests();
    
    if (success) {
        std::cout << "\n🎉 All performance optimization tests PASSED!" << std::endl;
        std::cout << "Performance optimization system is working correctly." << std::endl;
    } else {
        std::cout << "\n❌ Some performance optimization tests FAILED!" << std::endl;
        std::cout << "Please check the implementation." << std::endl;
    }
    
    return success ? 0 : 1;
}