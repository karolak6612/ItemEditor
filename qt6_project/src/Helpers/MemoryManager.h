/**
 * Item Editor Qt6 - Memory Management Utilities
 * Provides comprehensive memory management, leak detection, and resource optimization
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QHash>
#include <QElapsedTimer>
#include <QDateTime>
#include <memory>

namespace ItemEditor {

/**
 * Memory usage statistics structure
 */
struct MemoryStats {
    qint64 totalAllocated = 0;
    qint64 totalDeallocated = 0;
    qint64 currentUsage = 0;
    qint64 peakUsage = 0;
    int activeAllocations = 0;
    int totalAllocations = 0;
    
    qint64 netUsage() const { return totalAllocated - totalDeallocated; }
    double fragmentationRatio() const { 
        return activeAllocations > 0 ? (double)currentUsage / activeAllocations : 0.0; 
    }
};

/**
 * RAII wrapper for automatic resource cleanup
 */
template<typename T>
class ResourceGuard {
public:
    explicit ResourceGuard(T* resource, std::function<void(T*)> deleter = nullptr)
        : m_resource(resource), m_deleter(deleter) {}
    
    ~ResourceGuard() {
        if (m_resource) {
            if (m_deleter) {
                m_deleter(m_resource);
            } else {
                delete m_resource;
            }
        }
    }
    
    T* get() const { return m_resource; }
    T* release() { 
        T* temp = m_resource; 
        m_resource = nullptr; 
        return temp; 
    }
    
    ResourceGuard(const ResourceGuard&) = delete;
    ResourceGuard& operator=(const ResourceGuard&) = delete;
    
private:
    T* m_resource;
    std::function<void(T*)> m_deleter;
};

/**
 * Cache with automatic memory management and LRU eviction
 */
template<typename Key, typename Value>
class ManagedCache : public QObject {
public:
    explicit ManagedCache(int maxSize = 1000, QObject* parent = nullptr)
        : QObject(parent), m_maxSize(maxSize), m_currentSize(0) {}
    
    void insert(const Key& key, const Value& value, int cost = 1) {
        QMutexLocker locker(&m_mutex);
        
        // Remove existing entry if present
        if (m_cache.contains(key)) {
            remove(key);
        }
        
        // Evict items if necessary
        while (m_currentSize + cost > m_maxSize && !m_cache.isEmpty()) {
            evictLRU();
        }
        
        // Insert new item
        CacheItem item;
        item.value = value;
        item.cost = cost;
        item.accessTime = QDateTime::currentMSecsSinceEpoch();
        
        m_cache.insert(key, item);
        m_currentSize += cost;
    }
    
    bool contains(const Key& key) const {
        QMutexLocker locker(&m_mutex);
        return m_cache.contains(key);
    }
    
    Value value(const Key& key, const Value& defaultValue = Value()) {
        QMutexLocker locker(&m_mutex);
        auto it = m_cache.find(key);
        if (it != m_cache.end()) {
            it->accessTime = QDateTime::currentMSecsSinceEpoch();
            return it->value;
        }
        return defaultValue;
    }
    
    void remove(const Key& key) {
        QMutexLocker locker(&m_mutex);
        auto it = m_cache.find(key);
        if (it != m_cache.end()) {
            m_currentSize -= it->cost;
            m_cache.erase(it);
        }
    }
    
    void clear() {
        QMutexLocker locker(&m_mutex);
        m_cache.clear();
        m_currentSize = 0;
    }
    
    int size() const {
        QMutexLocker locker(&m_mutex);
        return m_cache.size();
    }
    
    int totalCost() const {
        QMutexLocker locker(&m_mutex);
        return m_currentSize;
    }
    
private:
    struct CacheItem {
        Value value;
        int cost;
        qint64 accessTime;
    };
    
    void evictLRU() {
        if (m_cache.isEmpty()) return;
        
        auto oldestIt = m_cache.begin();
        qint64 oldestTime = oldestIt->accessTime;
        
        for (auto it = m_cache.begin(); it != m_cache.end(); ++it) {
            if (it->accessTime < oldestTime) {
                oldestTime = it->accessTime;
                oldestIt = it;
            }
        }
        
        m_currentSize -= oldestIt->cost;
        m_cache.erase(oldestIt);
    }
    
    QHash<Key, CacheItem> m_cache;
    int m_maxSize;
    int m_currentSize;
    mutable QMutex m_mutex;
};

/**
 * Singleton memory manager for application-wide memory monitoring
 */
class MemoryManager : public QObject {
    Q_OBJECT
    
public:
    static MemoryManager* instance();
    
    // Memory tracking
    void trackAllocation(void* ptr, size_t size, const QString& category = "General");
    void trackDeallocation(void* ptr);
    
    // Statistics
    MemoryStats getStats() const;
    MemoryStats getStats(const QString& category) const;
    QStringList getCategories() const;
    
    // Memory optimization
    void optimizeMemory();
    void clearCaches();
    void garbageCollect();
    
    // Leak detection
    bool hasLeaks() const;
    QStringList getLeakReport() const;
    
    // Configuration
    void setMonitoringEnabled(bool enabled);
    void setLeakDetectionEnabled(bool enabled);
    void setMemoryLimit(qint64 limit);
    
signals:
    void memoryLimitExceeded(qint64 current, qint64 limit);
    void memoryLeakDetected(const QString& details);
    void memoryOptimized(qint64 freedBytes);
    
private slots:
    void checkMemoryUsage();
    void performGarbageCollection();
    
private:
    explicit MemoryManager(QObject* parent = nullptr);
    ~MemoryManager();
    
    struct AllocationInfo {
        size_t size;
        QString category;
        qint64 timestamp;
        QString stackTrace;
    };
    
    static MemoryManager* s_instance;
    
    mutable QMutex m_mutex;
    QHash<void*, AllocationInfo> m_allocations;
    QHash<QString, MemoryStats> m_categoryStats;
    MemoryStats m_globalStats;
    
    QTimer* m_monitorTimer;
    QTimer* m_gcTimer;
    
    bool m_monitoringEnabled;
    bool m_leakDetectionEnabled;
    qint64 m_memoryLimit;
    
    QString captureStackTrace() const;
    void updateStats(const QString& category, qint64 sizeDelta, bool isAllocation);
};

} // namespace ItemEditor

#endif // MEMORYMANAGER_H