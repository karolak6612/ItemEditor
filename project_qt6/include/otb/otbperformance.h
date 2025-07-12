#ifndef OTBPERFORMANCE_H
#define OTBPERFORMANCE_H

#include "otbtypes.h"
#include <QObject>
#include <QElapsedTimer>
#include <QMutex>
#include <QHash>
#include <QThread>
#include <QThreadPool>
#include <QRunnable>
#include <QFuture>
#include <QFutureWatcher>
#include <QTimer>
#include <memory>
#include <vector>

namespace OTB {

// Performance metrics collection
struct PerformanceMetrics {
    // Timing metrics
    qint64 totalReadTime = 0;
    qint64 totalWriteTime = 0;
    qint64 totalParseTime = 0;
    qint64 totalValidationTime = 0;
    
    // Throughput metrics
    qint64 bytesRead = 0;
    qint64 bytesWritten = 0;
    qint64 itemsProcessed = 0;
    qint64 filesProcessed = 0;
    
    // Resource usage metrics
    qint64 peakMemoryUsage = 0;
    qint64 currentMemoryUsage = 0;
    qint64 cacheHits = 0;
    qint64 cacheMisses = 0;
    
    // Error metrics
    qint64 errorsEncountered = 0;
    qint64 warningsGenerated = 0;
    qint64 recoveryAttempts = 0;
    
    // Operation counts
    qint64 readOperations = 0;
    qint64 writeOperations = 0;
    qint64 validationOperations = 0;
    
    // Calculated metrics
    double averageReadSpeed() const {
        return totalReadTime > 0 ? (double)bytesRead / totalReadTime * 1000.0 : 0.0; // bytes/sec
    }
    
    double averageWriteSpeed() const {
        return totalWriteTime > 0 ? (double)bytesWritten / totalWriteTime * 1000.0 : 0.0; // bytes/sec
    }
    
    double cacheHitRatio() const {
        qint64 total = cacheHits + cacheMisses;
        return total > 0 ? (double)cacheHits / total : 0.0;
    }
    
    double averageItemProcessingTime() const {
        return itemsProcessed > 0 ? (double)totalParseTime / itemsProcessed : 0.0; // ms per item
    }
    
    void reset() {
        *this = PerformanceMetrics{};
    }
};

// I/O Buffer management for optimized file operations
class IOBuffer {
public:
    explicit IOBuffer(qint64 size = 64 * 1024); // 64KB default
    ~IOBuffer();

    // Buffer operations
    void setSize(qint64 size);
    qint64 size() const { return m_size; }
    qint64 available() const { return m_size - m_position; }
    qint64 position() const { return m_position; }
    
    // Data operations
    bool read(QIODevice* device, QByteArray& data, qint64 maxSize);
    bool write(QIODevice* device, const QByteArray& data);
    void flush(QIODevice* device);
    void clear();
    void reset();
    
    // Buffered reading
    bool readBuffered(QIODevice* device, char* data, qint64 size);
    bool readLine(QIODevice* device, QByteArray& line);
    
    // Statistics
    qint64 getBytesRead() const { return m_bytesRead; }
    qint64 getBytesWritten() const { return m_bytesWritten; }
    qint64 getFlushCount() const { return m_flushCount; }

private:
    QByteArray m_buffer;
    qint64 m_size;
    qint64 m_position = 0;
    qint64 m_bytesRead = 0;
    qint64 m_bytesWritten = 0;
    qint64 m_flushCount = 0;
    
    void ensureCapacity(qint64 requiredSize);
};

// Memory pool for efficient object allocation
template<typename T>
class MemoryPool {
public:
    explicit MemoryPool(size_t initialSize = 100, size_t maxSize = 10000)
        : m_maxSize(maxSize) {
        m_pool.reserve(initialSize);
        for (size_t i = 0; i < initialSize; ++i) {
            m_pool.push_back(std::make_unique<T>());
        }
    }

    std::unique_ptr<T> acquire() {
        QMutexLocker locker(&m_mutex);
        if (!m_pool.empty()) {
            auto obj = std::move(m_pool.back());
            m_pool.pop_back();
            return obj;
        }
        return std::make_unique<T>();
    }

    void release(std::unique_ptr<T> obj) {
        if (!obj) return;
        
        QMutexLocker locker(&m_mutex);
        if (m_pool.size() < m_maxSize) {
            // Reset object to clean state if needed
            obj->reset();
            m_pool.push_back(std::move(obj));
        }
        // If pool is full, let the unique_ptr destroy the object
    }

    size_t poolSize() const {
        QMutexLocker locker(&m_mutex);
        return m_pool.size();
    }

    void clear() {
        QMutexLocker locker(&m_mutex);
        m_pool.clear();
    }

private:
    std::vector<std::unique_ptr<T>> m_pool;
    size_t m_maxSize;
    mutable QMutex m_mutex;
};

// Asynchronous operation support
class AsyncOperation : public QRunnable {
public:
    AsyncOperation() { setAutoDelete(false); }
    virtual ~AsyncOperation() = default;
    
    virtual void run() override = 0;
    virtual void cancel() { m_cancelled = true; }
    virtual bool isCancelled() const { return m_cancelled; }
    
protected:
    volatile bool m_cancelled = false;
};

// Performance monitor for real-time monitoring
class PerformanceMonitor : public QObject {
    Q_OBJECT

public:
    explicit PerformanceMonitor(QObject* parent = nullptr);
    ~PerformanceMonitor();

    // Monitoring control
    void startMonitoring();
    void stopMonitoring();
    void pauseMonitoring();
    void resumeMonitoring();
    bool isMonitoring() const { return m_monitoring; }

    // Metric recording
    void recordReadOperation(qint64 bytes, qint64 timeMs);
    void recordWriteOperation(qint64 bytes, qint64 timeMs);
    void recordParseOperation(qint64 items, qint64 timeMs);
    void recordValidationOperation(qint64 timeMs);
    void recordCacheHit();
    void recordCacheMiss();
    void recordError();
    void recordWarning();
    void recordMemoryUsage(qint64 bytes);

    // Metrics access
    const PerformanceMetrics& getCurrentMetrics() const { return m_currentMetrics; }
    PerformanceMetrics getMetricsSince(const QDateTime& since) const;
    QList<PerformanceMetrics> getHistoricalMetrics(int count = 100) const;

    // Performance analysis
    QString generateReport() const;
    QStringList getPerformanceWarnings() const;
    QStringList getOptimizationSuggestions() const;

    // Thresholds and alerts
    void setMemoryThreshold(qint64 bytes);
    void setPerformanceThreshold(double minSpeed); // bytes/sec
    void setCacheHitRatioThreshold(double minRatio);

public slots:
    void resetMetrics();
    void saveMetricsToFile(const QString& filePath);

signals:
    void memoryThresholdExceeded(qint64 current, qint64 threshold);
    void performanceThresholdExceeded(double current, double threshold);
    void cacheHitRatioLow(double current, double threshold);
    void metricsUpdated(const PerformanceMetrics& metrics);

private slots:
    void updateMetrics();

private:
    PerformanceMetrics m_currentMetrics;
    QList<PerformanceMetrics> m_historicalMetrics;
    QElapsedTimer m_timer;
    QTimer* m_updateTimer = nullptr;
    
    bool m_monitoring = false;
    bool m_paused = false;
    
    // Thresholds
    qint64 m_memoryThreshold = 512 * 1024 * 1024; // 512MB
    double m_performanceThreshold = 10 * 1024 * 1024; // 10MB/s
    double m_cacheHitRatioThreshold = 0.8; // 80%
    
    mutable QMutex m_mutex;
    
    void checkThresholds();
    void archiveOldMetrics();
};

// Performance optimizer with automatic tuning
class PerformanceOptimizer : public QObject {
    Q_OBJECT

public:
    explicit PerformanceOptimizer(QObject* parent = nullptr);

    // Optimization strategies
    void optimizeForMemory();
    void optimizeForSpeed();
    void optimizeForBalance();
    void optimizeForFileSize(qint64 estimatedFileSize);

    // Automatic optimization
    void enableAutoOptimization(bool enabled);
    void setOptimizationInterval(int seconds);

    // Buffer size optimization
    qint64 getOptimalBufferSize(qint64 fileSize) const;
    qint64 getOptimalCacheSize(qint64 availableMemory) const;
    
    // Thread optimization
    int getOptimalThreadCount() const;
    void configureThreadPool();

    // I/O optimization
    void optimizeIOOperations();
    void enableAsyncIO(bool enabled);

public slots:
    void performOptimization();
    void analyzeAndOptimize();

signals:
    void optimizationCompleted();
    void optimizationRecommendation(const QString& recommendation);

private:
    bool m_autoOptimizationEnabled = false;
    QTimer* m_optimizationTimer = nullptr;
    PerformanceMonitor* m_monitor = nullptr;
    
    void analyzeCurrentPerformance();
    void applyOptimizations();
};

// Global performance manager
class PerformanceManager {
public:
    static PerformanceMonitor* getMonitor();
    static PerformanceOptimizer* getOptimizer();
    static void initialize();
    static void shutdown();
    
    // Global settings
    static void setGlobalBufferSize(qint64 size);
    static void setGlobalCacheSize(qint64 size);
    static void setGlobalThreadCount(int count);
    
    // Quick optimization presets
    static void applyLowMemoryProfile();
    static void applyHighPerformanceProfile();
    static void applyBalancedProfile();

private:
    static std::unique_ptr<PerformanceMonitor> s_monitor;
    static std::unique_ptr<PerformanceOptimizer> s_optimizer;
    static QMutex s_mutex;
    static bool s_initialized;
};

} // namespace OTB

#endif // OTBPERFORMANCE_H