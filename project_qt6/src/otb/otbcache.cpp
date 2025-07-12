#include "otbcache.h"
#include <QDebug>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QDataStream>
#include <algorithm>

namespace OTB {

// Static members for CacheManager
std::unique_ptr<OtbCache> CacheManager::s_instance = nullptr;
QMutex CacheManager::s_instanceMutex;

OtbCache::OtbCache(QObject* parent) : QObject(parent) {
    // Initialize cache levels
    for (int i = 0; i < 3; ++i) {
        m_caches[i] = std::make_unique<LevelCache>();
    }
    
    // Set default cache sizes
    setCacheSize(L1_Fast, 100);     // 100 items in L1
    setCacheSize(L2_Medium, 1000);  // 1000 items in L2  
    setCacheSize(L3_Large, 10000);  // 10000 items in L3
    
    // Set default file and sprite cache sizes
    m_fileCache.setMaxCost(50);     // 50 files
    m_spriteCache.setMaxCost(1000); // 1000 sprites
    
    // Setup cleanup timer
    m_cleanupTimer = new QTimer(this);
    connect(m_cleanupTimer, &QTimer::timeout, this, &OtbCache::onCleanupTimer);
    m_cleanupTimer->start(m_cleanupInterval * 1000);
}

OtbCache::~OtbCache() {
    clearAll();
}

void OtbCache::setMaxMemoryUsage(qint64 maxBytes) {
    QMutexLocker locker(m_threadSafe ? &m_globalMutex : nullptr);
    m_maxMemoryUsage = maxBytes;
    
    // Adjust cache sizes proportionally
    qint64 l1Size = maxBytes * 0.1;  // 10% for L1
    qint64 l2Size = maxBytes * 0.3;  // 30% for L2
    qint64 l3Size = maxBytes * 0.5;  // 50% for L3
    // Remaining 10% for file and sprite caches
    
    m_caches[L1_Fast]->itemCache.setMaxCost(l1Size / 1024);   // Approximate items
    m_caches[L2_Medium]->itemCache.setMaxCost(l2Size / 1024);
    m_caches[L3_Large]->itemCache.setMaxCost(l3Size / 1024);
}

void OtbCache::setCacheSize(CacheLevel level, int maxItems) {
    if (level < 0 || level >= 3) return;
    
    QMutexLocker locker(m_threadSafe ? &m_caches[level]->mutex : nullptr);
    m_caches[level]->itemCache.setMaxCost(maxItems);
}

void OtbCache::setEvictionPolicy(CacheLevel level, const QString& policy) {
    if (level < 0 || level >= 3) return;
    
    QMutexLocker locker(m_threadSafe ? &m_caches[level]->mutex : nullptr);
    m_caches[level]->evictionPolicy = policy.toLower();
}

void OtbCache::setCompressionEnabled(bool enabled) {
    QMutexLocker locker(m_threadSafe ? &m_globalMutex : nullptr);
    m_compressionEnabled = enabled;
}

void OtbCache::setAutoCleanupInterval(int seconds) {
    m_cleanupInterval = seconds;
    if (m_cleanupTimer) {
        m_cleanupTimer->setInterval(seconds * 1000);
    }
}

bool OtbCache::cacheItem(quint16 itemId, const ServerItem& item, CacheLevel level) {
    if (level < 0 || level >= 3) return false;
    
    auto& cache = m_caches[level];
    QMutexLocker locker(m_threadSafe ? &cache->mutex : nullptr);
    
    // Check memory constraints
    qint64 itemSize = calculateItemMemorySize(item);
    if (!canFitInMemory(itemSize)) {
        // Try to make room by evicting items
        evictByPolicy(level, 1);
        if (!canFitInMemory(itemSize)) {
            return false;
        }
    }
    
    // Create cache entry
    ServerItem* cachedItem = new ServerItem(item);
    cache->itemCache.insert(itemId, cachedItem, itemSize);
    
    // Update metadata
    CacheEntry entry;
    entry.creationTime = QDateTime::currentDateTime();
    entry.accessTime = entry.creationTime;
    entry.memorySize = itemSize;
    entry.accessCount = 1;
    cache->metadata[itemId] = entry;
    
    // Update statistics
    cache->stats.totalMemoryUsed += itemSize;
    cache->stats.maxMemoryUsed = qMax(cache->stats.maxMemoryUsed, cache->stats.totalMemoryUsed);
    
    updateMemoryUsage();
    return true;
}

bool OtbCache::getCachedItem(quint16 itemId, ServerItem& item, CacheLevel level) {
    if (level < 0 || level >= 3) return false;
    
    auto& cache = m_caches[level];
    QMutexLocker locker(m_threadSafe ? &cache->mutex : nullptr);
    
    ServerItem* cachedItem = cache->itemCache.object(itemId);
    if (cachedItem) {
        item = *cachedItem;
        
        // Update metadata
        if (cache->metadata.contains(itemId)) {
            cache->metadata[itemId].updateAccess();
        }
        
        recordHit(level, itemId);
        return true;
    }
    
    recordMiss(level, itemId);
    return false;
}

void OtbCache::removeItem(quint16 itemId, CacheLevel level) {
    if (level < 0 || level >= 3) return;
    
    auto& cache = m_caches[level];
    QMutexLocker locker(m_threadSafe ? &cache->mutex : nullptr);
    
    if (cache->metadata.contains(itemId)) {
        qint64 memorySize = cache->metadata[itemId].memorySize;
        cache->stats.totalMemoryUsed -= memorySize;
        cache->metadata.remove(itemId);
    }
    
    cache->itemCache.remove(itemId);
    updateMemoryUsage();
}

void OtbCache::pinItem(quint16 itemId, CacheLevel level) {
    if (level < 0 || level >= 3) return;
    
    auto& cache = m_caches[level];
    QMutexLocker locker(m_threadSafe ? &cache->mutex : nullptr);
    
    if (cache->metadata.contains(itemId)) {
        cache->metadata[itemId].isPinned = true;
    }
}

void OtbCache::unpinItem(quint16 itemId, CacheLevel level) {
    if (level < 0 || level >= 3) return;
    
    auto& cache = m_caches[level];
    QMutexLocker locker(m_threadSafe ? &cache->mutex : nullptr);
    
    if (cache->metadata.contains(itemId)) {
        cache->metadata[itemId].isPinned = false;
    }
}

bool OtbCache::cacheFileData(const QString& filePath, const QByteArray& data) {
    QMutexLocker locker(m_threadSafe ? &m_globalMutex : nullptr);
    
    QByteArray* cachedData = new QByteArray(m_compressionEnabled ? compressData(data) : data);
    qint64 size = cachedData->size();
    
    if (!canFitInMemory(size)) {
        delete cachedData;
        return false;
    }
    
    m_fileCache.insert(filePath, cachedData, size);
    
    CacheEntry entry;
    entry.creationTime = QDateTime::currentDateTime();
    entry.accessTime = entry.creationTime;
    entry.memorySize = size;
    entry.accessCount = 1;
    m_fileMetadata[filePath] = entry;
    
    return true;
}

bool OtbCache::getCachedFileData(const QString& filePath, QByteArray& data) {
    QMutexLocker locker(m_threadSafe ? &m_globalMutex : nullptr);
    
    QByteArray* cachedData = m_fileCache.object(filePath);
    if (cachedData) {
        data = m_compressionEnabled ? decompressData(*cachedData) : *cachedData;
        
        if (m_fileMetadata.contains(filePath)) {
            m_fileMetadata[filePath].updateAccess();
        }
        
        return true;
    }
    
    return false;
}

void OtbCache::removeFileData(const QString& filePath) {
    QMutexLocker locker(m_threadSafe ? &m_globalMutex : nullptr);
    
    m_fileCache.remove(filePath);
    m_fileMetadata.remove(filePath);
}

bool OtbCache::cacheSpriteHash(const QByteArray& hash, const QByteArray& spriteData) {
    QMutexLocker locker(m_threadSafe ? &m_globalMutex : nullptr);
    
    QByteArray* cachedData = new QByteArray(m_compressionEnabled ? compressData(spriteData) : spriteData);
    qint64 size = cachedData->size();
    
    if (!canFitInMemory(size)) {
        delete cachedData;
        return false;
    }
    
    m_spriteCache.insert(hash, cachedData, size);
    
    CacheEntry entry;
    entry.creationTime = QDateTime::currentDateTime();
    entry.accessTime = entry.creationTime;
    entry.memorySize = size;
    entry.accessCount = 1;
    m_spriteMetadata[hash] = entry;
    
    return true;
}

bool OtbCache::getCachedSpriteData(const QByteArray& hash, QByteArray& spriteData) {
    QMutexLocker locker(m_threadSafe ? &m_globalMutex : nullptr);
    
    QByteArray* cachedData = m_spriteCache.object(hash);
    if (cachedData) {
        spriteData = m_compressionEnabled ? decompressData(*cachedData) : *cachedData;
        
        if (m_spriteMetadata.contains(hash)) {
            m_spriteMetadata[hash].updateAccess();
        }
        
        return true;
    }
    
    return false;
}

void OtbCache::preloadItems(const QList<quint16>& itemIds) {
    // This would typically load items from storage
    // For now, it's a placeholder that could be connected to the main item storage
    Q_UNUSED(itemIds)
    qDebug() << "Preloading" << itemIds.size() << "items";
}

void OtbCache::prefetchSimilarItems(quint16 baseItemId, int count) {
    // Prefetch items with IDs near the base ID
    QList<quint16> itemIds;
    for (int i = 1; i <= count/2; ++i) {
        if (baseItemId >= i) itemIds.append(baseItemId - i);
        itemIds.append(baseItemId + i);
    }
    preloadItems(itemIds);
}

void OtbCache::preloadFrequentlyUsed() {
    // Load the most frequently accessed items across all cache levels
    QHash<quint16, qint64> accessCounts;
    
    for (int level = 0; level < 3; ++level) {
        auto& cache = m_caches[level];
        QMutexLocker locker(m_threadSafe ? &cache->mutex : nullptr);
        
        for (auto it = cache->metadata.begin(); it != cache->metadata.end(); ++it) {
            accessCounts[it.key()] += it.value().accessCount;
        }
    }
    
    // Sort by access count and preload top items
    QList<QPair<qint64, quint16>> sortedItems;
    for (auto it = accessCounts.begin(); it != accessCounts.end(); ++it) {
        sortedItems.append({it.value(), it.key()});
    }
    
    std::sort(sortedItems.begin(), sortedItems.end(), std::greater<>());
    
    QList<quint16> topItems;
    for (int i = 0; i < qMin(100, sortedItems.size()); ++i) {
        topItems.append(sortedItems[i].second);
    }
    
    preloadItems(topItems);
}

void OtbCache::clear(CacheLevel level) {
    if (level < 0 || level >= 3) return;
    
    auto& cache = m_caches[level];
    QMutexLocker locker(m_threadSafe ? &cache->mutex : nullptr);
    
    cache->itemCache.clear();
    cache->metadata.clear();
    cache->stats.reset();
}

void OtbCache::clearAll() {
    for (int i = 0; i < 3; ++i) {
        clear(static_cast<CacheLevel>(i));
    }
    
    QMutexLocker locker(m_threadSafe ? &m_globalMutex : nullptr);
    m_fileCache.clear();
    m_spriteCache.clear();
    m_fileMetadata.clear();
    m_spriteMetadata.clear();
}

void OtbCache::compact() {
    // Remove expired or least used items to free up memory
    QDateTime cutoffTime = QDateTime::currentDateTime().addSecs(-3600); // 1 hour ago
    
    for (int level = 0; level < 3; ++level) {
        auto& cache = m_caches[level];
        QMutexLocker locker(m_threadSafe ? &cache->mutex : nullptr);
        
        QList<quint16> itemsToRemove;
        for (auto it = cache->metadata.begin(); it != cache->metadata.end(); ++it) {
            if (!it.value().isPinned && it.value().accessTime < cutoffTime) {
                itemsToRemove.append(it.key());
            }
        }
        
        for (quint16 itemId : itemsToRemove) {
            cache->itemCache.remove(itemId);
            cache->stats.totalMemoryUsed -= cache->metadata[itemId].memorySize;
            cache->metadata.remove(itemId);
            cache->stats.evictions++;
        }
    }
    
    updateMemoryUsage();
}

void OtbCache::optimize() {
    compact();
    
    // Move frequently accessed items to faster cache levels
    for (int level = L3_Large; level >= L2_Medium; --level) {
        auto& currentCache = m_caches[level];
        auto& fasterCache = m_caches[level - 1];
        
        QMutexLocker currentLocker(m_threadSafe ? &currentCache->mutex : nullptr);
        QMutexLocker fasterLocker(m_threadSafe ? &fasterCache->mutex : nullptr);
        
        // Find highly accessed items
        QList<QPair<qint64, quint16>> accessList;
        for (auto it = currentCache->metadata.begin(); it != currentCache->metadata.end(); ++it) {
            accessList.append({it.value().accessCount, it.key()});
        }
        
        std::sort(accessList.begin(), accessList.end(), std::greater<>());
        
        // Move top 10% to faster cache if there's room
        int moveCount = qMin(accessList.size() / 10, fasterCache->itemCache.maxCost() / 4);
        for (int i = 0; i < moveCount; ++i) {
            quint16 itemId = accessList[i].second;
            ServerItem* item = currentCache->itemCache.object(itemId);
            if (item && fasterCache->itemCache.totalCost() < fasterCache->itemCache.maxCost()) {
                ServerItem* newItem = new ServerItem(*item);
                qint64 size = currentCache->metadata[itemId].memorySize;
                
                fasterCache->itemCache.insert(itemId, newItem, size);
                fasterCache->metadata[itemId] = currentCache->metadata[itemId];
                
                currentCache->itemCache.remove(itemId);
                currentCache->metadata.remove(itemId);
            }
        }
    }
}

qint64 OtbCache::getCurrentMemoryUsage() const {
    return calculateCurrentMemoryUsage();
}

const CacheStats& OtbCache::getStats(CacheLevel level) const {
    if (level < 0 || level >= 3) {
        static CacheStats empty;
        return empty;
    }
    
    return m_caches[level]->stats;
}

CacheStats OtbCache::getCombinedStats() const {
    CacheStats combined;
    
    for (int i = 0; i < 3; ++i) {
        const auto& stats = m_caches[i]->stats;
        combined.hits += stats.hits;
        combined.misses += stats.misses;
        combined.evictions += stats.evictions;
        combined.totalMemoryUsed += stats.totalMemoryUsed;
        combined.maxMemoryUsed = qMax(combined.maxMemoryUsed, stats.maxMemoryUsed);
        
        if (combined.creationTime.isNull() || stats.creationTime < combined.creationTime) {
            combined.creationTime = stats.creationTime;
        }
        if (stats.lastAccessTime > combined.lastAccessTime) {
            combined.lastAccessTime = stats.lastAccessTime;
        }
    }
    
    return combined;
}

QStringList OtbCache::getCacheReport() const {
    QStringList report;
    
    CacheStats combined = getCombinedStats();
    report << QString("=== OTB Cache Report ===");
    report << QString("Total Hits: %1").arg(combined.hits);
    report << QString("Total Misses: %1").arg(combined.misses);
    report << QString("Hit Ratio: %1%").arg(combined.hitRatio() * 100, 0, 'f', 2);
    report << QString("Total Evictions: %1").arg(combined.evictions);
    report << QString("Current Memory Usage: %1 KB").arg(combined.totalMemoryUsed / 1024);
    report << QString("Peak Memory Usage: %1 KB").arg(combined.maxMemoryUsed / 1024);
    report << QString("Memory Limit: %1 KB").arg(m_maxMemoryUsage / 1024);
    
    for (int i = 0; i < 3; ++i) {
        const auto& stats = m_caches[i]->stats;
        QString levelName = (i == 0) ? "L1 (Fast)" : (i == 1) ? "L2 (Medium)" : "L3 (Large)";
        report << QString("");
        report << QString("--- %1 ---").arg(levelName);
        report << QString("Hits: %1").arg(stats.hits);
        report << QString("Misses: %1").arg(stats.misses);
        report << QString("Hit Ratio: %1%").arg(stats.hitRatio() * 100, 0, 'f', 2);
        report << QString("Items: %1/%2").arg(m_caches[i]->itemCache.size()).arg(m_caches[i]->itemCache.maxCost());
        report << QString("Memory: %1 KB").arg(stats.totalMemoryUsed / 1024);
    }
    
    return report;
}

void OtbCache::resetStats() {
    for (int i = 0; i < 3; ++i) {
        auto& cache = m_caches[i];
        QMutexLocker locker(m_threadSafe ? &cache->mutex : nullptr);
        cache->stats.reset();
    }
}

void OtbCache::warmupFromFile(const QString& filePath) {
    Q_UNUSED(filePath)
    // Implementation would read file and preload commonly accessed items
    qDebug() << "Warming up cache from file:" << filePath;
}

void OtbCache::warmupFromItemList(const ServerItemList& items) {
    // Cache the first 100 items in L2 cache
    int count = 0;
    for (const ServerItem& item : items.items) {
        if (count++ >= 100) break;
        cacheItem(item.id, item, L2_Medium);
    }
    qDebug() << "Warmed up cache with" << count << "items";
}

void OtbCache::warmupMostUsed(int count) {
    Q_UNUSED(count)
    preloadFrequentlyUsed();
}

void OtbCache::performCleanup() {
    compact();
}

void OtbCache::performOptimization() {
    optimize();
}

void OtbCache::onCleanupTimer() {
    if (getCurrentMemoryUsage() > m_maxMemoryUsage * 0.8) { // 80% threshold
        performCleanup();
    }
}

// Helper methods implementation
void OtbCache::evictOldestItems(CacheLevel level, int count) {
    if (level < 0 || level >= 3) return;
    
    auto& cache = m_caches[level];
    
    QList<QPair<QDateTime, quint16>> ageList;
    for (auto it = cache->metadata.begin(); it != cache->metadata.end(); ++it) {
        if (!it.value().isPinned) {
            ageList.append({it.value().creationTime, it.key()});
        }
    }
    
    std::sort(ageList.begin(), ageList.end());
    
    for (int i = 0; i < qMin(count, ageList.size()); ++i) {
        quint16 itemId = ageList[i].second;
        cache->itemCache.remove(itemId);
        cache->stats.totalMemoryUsed -= cache->metadata[itemId].memorySize;
        cache->metadata.remove(itemId);
        cache->stats.evictions++;
        recordEviction(level, itemId);
    }
}

void OtbCache::evictLeastUsedItems(CacheLevel level, int count) {
    if (level < 0 || level >= 3) return;
    
    auto& cache = m_caches[level];
    
    QList<QPair<qint64, quint16>> usageList;
    for (auto it = cache->metadata.begin(); it != cache->metadata.end(); ++it) {
        if (!it.value().isPinned) {
            usageList.append({it.value().accessCount, it.key()});
        }
    }
    
    std::sort(usageList.begin(), usageList.end());
    
    for (int i = 0; i < qMin(count, usageList.size()); ++i) {
        quint16 itemId = usageList[i].second;
        cache->itemCache.remove(itemId);
        cache->stats.totalMemoryUsed -= cache->metadata[itemId].memorySize;
        cache->metadata.remove(itemId);
        cache->stats.evictions++;
        recordEviction(level, itemId);
    }
}

void OtbCache::evictByPolicy(CacheLevel level, int count) {
    if (level < 0 || level >= 3) return;
    
    auto& cache = m_caches[level];
    
    if (cache->evictionPolicy == "lru") {
        // Evict least recently used
        QList<QPair<QDateTime, quint16>> accessList;
        for (auto it = cache->metadata.begin(); it != cache->metadata.end(); ++it) {
            if (!it.value().isPinned) {
                accessList.append({it.value().accessTime, it.key()});
            }
        }
        
        std::sort(accessList.begin(), accessList.end());
        
        for (int i = 0; i < qMin(count, accessList.size()); ++i) {
            quint16 itemId = accessList[i].second;
            cache->itemCache.remove(itemId);
            cache->stats.totalMemoryUsed -= cache->metadata[itemId].memorySize;
            cache->metadata.remove(itemId);
            cache->stats.evictions++;
            recordEviction(level, itemId);
        }
    } else if (cache->evictionPolicy == "lfu") {
        evictLeastUsedItems(level, count);
    } else { // "fifo" or default
        evictOldestItems(level, count);
    }
}

qint64 OtbCache::calculateItemMemorySize(const ServerItem& item) const {
    qint64 size = sizeof(ServerItem);
    size += item.name.length() * sizeof(QChar);
    size += item.spriteHash.size();
    return size;
}

qint64 OtbCache::calculateCurrentMemoryUsage() const {
    qint64 total = 0;
    
    for (int i = 0; i < 3; ++i) {
        total += m_caches[i]->stats.totalMemoryUsed;
    }
    
    // Add file and sprite cache usage
    total += m_fileCache.totalCost();
    total += m_spriteCache.totalCost();
    
    return total;
}

void OtbCache::updateMemoryUsage() {
    qint64 currentUsage = calculateCurrentMemoryUsage();
    
    if (currentUsage > m_maxMemoryUsage) {
        emit memoryLimitReached(currentUsage, m_maxMemoryUsage);
    }
}

bool OtbCache::canFitInMemory(qint64 additionalBytes) const {
    return (calculateCurrentMemoryUsage() + additionalBytes) <= m_maxMemoryUsage;
}

QByteArray OtbCache::compressData(const QByteArray& data) const {
    // Simple compression using qCompress
    return qCompress(data);
}

QByteArray OtbCache::decompressData(const QByteArray& data) const {
    // Simple decompression using qUncompress
    return qUncompress(data);
}

void OtbCache::recordHit(CacheLevel level, quint16 itemId) {
    if (level < 0 || level >= 3) return;
    
    m_caches[level]->stats.hits++;
    m_caches[level]->stats.lastAccessTime = QDateTime::currentDateTime();
    emit cacheHit(level, itemId);
}

void OtbCache::recordMiss(CacheLevel level, quint16 itemId) {
    if (level < 0 || level >= 3) return;
    
    m_caches[level]->stats.misses++;
    emit cacheMiss(level, itemId);
}

void OtbCache::recordEviction(CacheLevel level, quint16 itemId) {
    if (level < 0 || level >= 3) return;
    
    emit evictionOccurred(level, itemId);
}

// CacheFactory implementation
std::unique_ptr<OtbCache> CacheFactory::createCache(CacheProfile profile) {
    auto cache = std::make_unique<OtbCache>();
    configureCache(cache.get(), profile);
    return cache;
}

void CacheFactory::configureCache(OtbCache* cache, CacheProfile profile) {
    if (!cache) return;
    
    switch (profile) {
        case MemoryOptimized:
            cache->setMaxMemoryUsage(64 * 1024 * 1024); // 64MB
            cache->setCacheSize(OtbCache::L1_Fast, 50);
            cache->setCacheSize(OtbCache::L2_Medium, 200);
            cache->setCacheSize(OtbCache::L3_Large, 500);
            cache->setCompressionEnabled(true);
            break;
            
        case PerformanceOptimized:
            cache->setMaxMemoryUsage(512 * 1024 * 1024); // 512MB
            cache->setCacheSize(OtbCache::L1_Fast, 200);
            cache->setCacheSize(OtbCache::L2_Medium, 2000);
            cache->setCacheSize(OtbCache::L3_Large, 20000);
            cache->setCompressionEnabled(false);
            break;
            
        case Balanced:
            cache->setMaxMemoryUsage(256 * 1024 * 1024); // 256MB
            cache->setCacheSize(OtbCache::L1_Fast, 100);
            cache->setCacheSize(OtbCache::L2_Medium, 1000);
            cache->setCacheSize(OtbCache::L3_Large, 10000);
            cache->setCompressionEnabled(false);
            break;
            
        case LargeFile:
            cache->setMaxMemoryUsage(1024 * 1024 * 1024); // 1GB
            cache->setCacheSize(OtbCache::L1_Fast, 500);
            cache->setCacheSize(OtbCache::L2_Medium, 5000);
            cache->setCacheSize(OtbCache::L3_Large, 50000);
            cache->setCompressionEnabled(true);
            break;
            
        case SmallFile:
            cache->setMaxMemoryUsage(32 * 1024 * 1024); // 32MB
            cache->setCacheSize(OtbCache::L1_Fast, 25);
            cache->setCacheSize(OtbCache::L2_Medium, 100);
            cache->setCacheSize(OtbCache::L3_Large, 250);
            cache->setCompressionEnabled(false);
            break;
    }
}

// CacheManager implementation
OtbCache* CacheManager::getInstance() {
    QMutexLocker locker(&s_instanceMutex);
    if (!s_instance) {
        s_instance = CacheFactory::createCache(CacheFactory::Balanced);
    }
    return s_instance.get();
}

void CacheManager::setInstance(std::unique_ptr<OtbCache> cache) {
    QMutexLocker locker(&s_instanceMutex);
    s_instance = std::move(cache);
}

void CacheManager::destroyInstance() {
    QMutexLocker locker(&s_instanceMutex);
    s_instance.reset();
}

} // namespace OTB