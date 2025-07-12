#ifndef OTBCACHE_H
#define OTBCACHE_H

#include "otbtypes.h"
#include "item.h"
#include <QCache>
#include <QMutex>
#include <QMutexLocker>
#include <QHash>
#include <QDateTime>
#include <QTimer>
#include <QElapsedTimer>
#include <memory>

namespace OTB {

// Cache statistics for monitoring performance
struct CacheStats {
    qint64 hits = 0;
    qint64 misses = 0;
    qint64 evictions = 0;
    qint64 totalMemoryUsed = 0;
    qint64 maxMemoryUsed = 0;
    QDateTime lastAccessTime;
    QDateTime creationTime;
    
    double hitRatio() const {
        qint64 total = hits + misses;
        return total > 0 ? (double)hits / total : 0.0;
    }
    
    void reset() {
        hits = 0;
        misses = 0;
        evictions = 0;
        totalMemoryUsed = 0;
        maxMemoryUsed = 0;
        lastAccessTime = QDateTime::currentDateTime();
        creationTime = QDateTime::currentDateTime();
    }
};

// Cache entry metadata
struct CacheEntry {
    QDateTime accessTime;
    QDateTime creationTime;
    qint64 accessCount = 0;
    qint64 memorySize = 0;
    bool isPinned = false; // Prevents eviction
    
    void updateAccess() {
        accessTime = QDateTime::currentDateTime();
        accessCount++;
    }
};

// Multi-level cache system for OTB operations
class OtbCache : public QObject {
    Q_OBJECT

public:
    enum CacheLevel {
        L1_Fast = 0,    // Small, very fast cache for frequently accessed items
        L2_Medium = 1,  // Medium cache for recently accessed items
        L3_Large = 2    // Large cache for all accessed items
    };

    explicit OtbCache(QObject* parent = nullptr);
    ~OtbCache();

    // Cache configuration
    void setMaxMemoryUsage(qint64 maxBytes);
    void setCacheSize(CacheLevel level, int maxItems);
    void setEvictionPolicy(CacheLevel level, const QString& policy); // "lru", "lfu", "fifo"
    void setCompressionEnabled(bool enabled);
    void setAutoCleanupInterval(int seconds);

    // Item caching
    bool cacheItem(quint16 itemId, const ServerItem& item, CacheLevel level = L2_Medium);
    bool getCachedItem(quint16 itemId, ServerItem& item, CacheLevel level = L2_Medium);
    void removeItem(quint16 itemId, CacheLevel level = L2_Medium);
    void pinItem(quint16 itemId, CacheLevel level = L2_Medium);
    void unpinItem(quint16 itemId, CacheLevel level = L2_Medium);

    // File-level caching
    bool cacheFileData(const QString& filePath, const QByteArray& data);
    bool getCachedFileData(const QString& filePath, QByteArray& data);
    void removeFileData(const QString& filePath);

    // Sprite hash caching
    bool cacheSpriteHash(const QByteArray& hash, const QByteArray& spriteData);
    bool getCachedSpriteData(const QByteArray& hash, QByteArray& spriteData);

    // Preloading and prefetching
    void preloadItems(const QList<quint16>& itemIds);
    void prefetchSimilarItems(quint16 baseItemId, int count = 10);
    void preloadFrequentlyUsed();

    // Cache management
    void clear(CacheLevel level = L2_Medium);
    void clearAll();
    void compact();
    void optimize();
    qint64 getCurrentMemoryUsage() const;
    qint64 getMaxMemoryUsage() const { return m_maxMemoryUsage; }

    // Statistics and monitoring
    const CacheStats& getStats(CacheLevel level) const;
    CacheStats getCombinedStats() const;
    QStringList getCacheReport() const;
    void resetStats();

    // Cache warming strategies
    void warmupFromFile(const QString& filePath);
    void warmupFromItemList(const ServerItemList& items);
    void warmupMostUsed(int count = 100);

    // Thread safety
    void setThreadSafe(bool enabled) { m_threadSafe = enabled; }
    bool isThreadSafe() const { return m_threadSafe; }

public slots:
    void performCleanup();
    void performOptimization();

signals:
    void cacheHit(CacheLevel level, quint16 itemId);
    void cacheMiss(CacheLevel level, quint16 itemId);
    void memoryLimitReached(qint64 currentUsage, qint64 limit);
    void evictionOccurred(CacheLevel level, quint16 itemId);

private slots:
    void onCleanupTimer();

private:
    // Internal cache structures
    struct LevelCache {
        QCache<quint16, ServerItem> itemCache;
        QHash<quint16, CacheEntry> metadata;
        CacheStats stats;
        QString evictionPolicy = "lru";
        QMutex mutex;
        
        LevelCache() {
            stats.creationTime = QDateTime::currentDateTime();
        }
    };

    // Cache levels
    std::array<std::unique_ptr<LevelCache>, 3> m_caches;
    
    // File and sprite caches
    QCache<QString, QByteArray> m_fileCache;
    QCache<QByteArray, QByteArray> m_spriteCache;
    QHash<QString, CacheEntry> m_fileMetadata;
    QHash<QByteArray, CacheEntry> m_spriteMetadata;

    // Configuration
    qint64 m_maxMemoryUsage = 256 * 1024 * 1024; // 256MB default
    bool m_compressionEnabled = false;
    bool m_threadSafe = true;
    QTimer* m_cleanupTimer = nullptr;
    int m_cleanupInterval = 300; // 5 minutes

    // Synchronization
    mutable QMutex m_globalMutex;

    // Helper methods
    void evictOldestItems(CacheLevel level, int count = 1);
    void evictLeastUsedItems(CacheLevel level, int count = 1);
    void evictByPolicy(CacheLevel level, int count = 1);
    qint64 calculateItemMemorySize(const ServerItem& item) const;
    qint64 calculateCurrentMemoryUsage() const;
    void updateMemoryUsage();
    bool canFitInMemory(qint64 additionalBytes) const;
    
    // Compression helpers
    QByteArray compressData(const QByteArray& data) const;
    QByteArray decompressData(const QByteArray& data) const;
    
    // Thread safety helpers
    void recordHit(CacheLevel level, quint16 itemId);
    void recordMiss(CacheLevel level, quint16 itemId);
    void recordEviction(CacheLevel level, quint16 itemId);
};

// Cache factory for creating optimized cache instances
class CacheFactory {
public:
    enum CacheProfile {
        MemoryOptimized,    // Minimize memory usage
        PerformanceOptimized, // Maximize performance
        Balanced,           // Balance between memory and performance
        LargeFile,         // Optimized for large OTB files
        SmallFile          // Optimized for small OTB files
    };

    static std::unique_ptr<OtbCache> createCache(CacheProfile profile);
    static void configureCache(OtbCache* cache, CacheProfile profile);
};

// Global cache instance management
class CacheManager {
public:
    static OtbCache* getInstance();
    static void setInstance(std::unique_ptr<OtbCache> cache);
    static void destroyInstance();
    
private:
    static std::unique_ptr<OtbCache> s_instance;
    static QMutex s_instanceMutex;
};

} // namespace OTB

#endif // OTBCACHE_H