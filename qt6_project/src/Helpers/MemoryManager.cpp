/**
 * Item Editor Qt6 - Memory Management Utilities Implementation
 * Provides comprehensive memory management, leak detection, and resource optimization
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "MemoryManager.h"
#include <QApplication>
#include <QDebug>
#include <QThread>
#include <QMutexLocker>
#include <QDateTime>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#elif defined(Q_OS_LINUX)
#include <unistd.h>
#include <fstream>
#elif defined(Q_OS_MAC)
#include <mach/mach.h>
#endif

namespace ItemEditor {

// Static instance
MemoryManager* MemoryManager::s_instance = nullptr;

MemoryManager::MemoryManager(QObject* parent)
    : QObject(parent)
    , m_monitoringEnabled(true)
    , m_leakDetectionEnabled(true)
    , m_memoryLimit(512 * 1024 * 1024) // 512MB default limit
{
    // Setup monitoring timer
    m_monitorTimer = new QTimer(this);
    m_monitorTimer->setInterval(5000); // Check every 5 seconds
    connect(m_monitorTimer, &QTimer::timeout, this, &MemoryManager::checkMemoryUsage);
    
    // Setup garbage collection timer
    m_gcTimer = new QTimer(this);
    m_gcTimer->setInterval(30000); // GC every 30 seconds
    connect(m_gcTimer, &QTimer::timeout, this, &MemoryManager::performGarbageCollection);
    
    if (m_monitoringEnabled) {
        m_monitorTimer->start();
        m_gcTimer->start();
    }
}

MemoryManager::~MemoryManager()
{
    if (m_leakDetectionEnabled && hasLeaks()) {
        QStringList leaks = getLeakReport();
        qWarning() << "Memory leaks detected on shutdown:";
        for (const QString& leak : leaks) {
            qWarning() << leak;
        }
    }
}

MemoryManager* MemoryManager::instance()
{
    if (!s_instance) {
        s_instance = new MemoryManager();
    }
    return s_instance;
}

void MemoryManager::trackAllocation(void* ptr, size_t size, const QString& category)
{
    if (!m_monitoringEnabled || !ptr) return;
    
    QMutexLocker locker(&m_mutex);
    
    AllocationInfo info;
    info.size = size;
    info.category = category;
    info.timestamp = QDateTime::currentMSecsSinceEpoch();
    
    if (m_leakDetectionEnabled) {
        info.stackTrace = captureStackTrace();
    }
    
    m_allocations.insert(ptr, info);
    updateStats(category, size, true);
    
    // Check memory limit
    if (m_globalStats.currentUsage > m_memoryLimit) {
        emit memoryLimitExceeded(m_globalStats.currentUsage, m_memoryLimit);
    }
}

void MemoryManager::trackDeallocation(void* ptr)
{
    if (!m_monitoringEnabled || !ptr) return;
    
    QMutexLocker locker(&m_mutex);
    
    auto it = m_allocations.find(ptr);
    if (it != m_allocations.end()) {
        updateStats(it->category, it->size, false);
        m_allocations.erase(it);
    }
}

MemoryStats MemoryManager::getStats() const
{
    QMutexLocker locker(&m_mutex);
    return m_globalStats;
}

MemoryStats MemoryManager::getStats(const QString& category) const
{
    QMutexLocker locker(&m_mutex);
    return m_categoryStats.value(category, MemoryStats());
}

QStringList MemoryManager::getCategories() const
{
    QMutexLocker locker(&m_mutex);
    return m_categoryStats.keys();
}

void MemoryManager::optimizeMemory()
{
    qint64 freedBytes = 0;
    
    // Clear caches
    clearCaches();
    
    // Force garbage collection
    garbageCollect();
    
    // Platform-specific memory optimization
#ifdef Q_OS_WIN
    SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
#endif
    
    emit memoryOptimized(freedBytes);
}

void MemoryManager::clearCaches()
{
    // This will be called by cache-owning objects
    // They should connect to this signal and clear their caches
    QMetaObject::invokeMethod(this, "memoryOptimized", Qt::QueuedConnection, Q_ARG(qint64, 0));
}

void MemoryManager::garbageCollect()
{
    // Force Qt's garbage collection
    QCoreApplication::processEvents();
    
    // Platform-specific garbage collection
#ifdef Q_OS_WIN
    // Windows memory compaction
    HANDLE heap = GetProcessHeap();
    if (heap) {
        HeapCompact(heap, 0);
    }
#endif
}

bool MemoryManager::hasLeaks() const
{
    QMutexLocker locker(&m_mutex);
    return !m_allocations.isEmpty();
}

QStringList MemoryManager::getLeakReport() const
{
    QMutexLocker locker(&m_mutex);
    QStringList report;
    
    QHash<QString, int> categoryLeaks;
    qint64 totalLeaked = 0;
    
    for (auto it = m_allocations.begin(); it != m_allocations.end(); ++it) {
        const AllocationInfo& info = it.value();
        categoryLeaks[info.category]++;
        totalLeaked += info.size;
        
        if (m_leakDetectionEnabled && !info.stackTrace.isEmpty()) {
            report << QString("Leak: %1 bytes in category '%2' at %3")
                      .arg(info.size)
                      .arg(info.category)
                      .arg(info.stackTrace);
        }
    }
    
    report.prepend(QString("Total leaked: %1 bytes in %2 allocations")
                   .arg(totalLeaked)
                   .arg(m_allocations.size()));
    
    for (auto it = categoryLeaks.begin(); it != categoryLeaks.end(); ++it) {
        report << QString("Category '%1': %2 leaks").arg(it.key()).arg(it.value());
    }
    
    return report;
}

void MemoryManager::setMonitoringEnabled(bool enabled)
{
    m_monitoringEnabled = enabled;
    
    if (enabled) {
        m_monitorTimer->start();
        m_gcTimer->start();
    } else {
        m_monitorTimer->stop();
        m_gcTimer->stop();
    }
}

void MemoryManager::setLeakDetectionEnabled(bool enabled)
{
    m_leakDetectionEnabled = enabled;
}

void MemoryManager::setMemoryLimit(qint64 limit)
{
    m_memoryLimit = limit;
}

void MemoryManager::checkMemoryUsage()
{
    // Get system memory usage
    qint64 systemMemory = 0;
    
#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        systemMemory = pmc.WorkingSetSize;
    }
#elif defined(Q_OS_LINUX)
    std::ifstream file("/proc/self/status");
    std::string line;
    while (std::getline(file, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            systemMemory = std::stoll(line.substr(7)) * 1024; // Convert KB to bytes
            break;
        }
    }
#elif defined(Q_OS_MAC)
    task_basic_info info;
    mach_msg_type_number_t size = sizeof(info);
    if (task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&info, &size) == KERN_SUCCESS) {
        systemMemory = info.resident_size;
    }
#endif
    
    // Update global stats with system memory
    QMutexLocker locker(&m_mutex);
    if (systemMemory > 0) {
        m_globalStats.currentUsage = systemMemory;
        if (systemMemory > m_globalStats.peakUsage) {
            m_globalStats.peakUsage = systemMemory;
        }
    }
    
    // Check for memory limit exceeded
    if (m_globalStats.currentUsage > m_memoryLimit) {
        emit memoryLimitExceeded(m_globalStats.currentUsage, m_memoryLimit);
    }
}

void MemoryManager::performGarbageCollection()
{
    garbageCollect();
}

QString MemoryManager::captureStackTrace() const
{
    // Simplified stack trace capture
    // In a production environment, you might want to use a more sophisticated
    // stack trace library like backward-cpp or platform-specific APIs
    return QString("Stack trace not implemented");
}

void MemoryManager::updateStats(const QString& category, qint64 sizeDelta, bool isAllocation)
{
    // Update global stats
    if (isAllocation) {
        m_globalStats.totalAllocated += sizeDelta;
        m_globalStats.currentUsage += sizeDelta;
        m_globalStats.activeAllocations++;
        m_globalStats.totalAllocations++;
        
        if (m_globalStats.currentUsage > m_globalStats.peakUsage) {
            m_globalStats.peakUsage = m_globalStats.currentUsage;
        }
    } else {
        m_globalStats.totalDeallocated += sizeDelta;
        m_globalStats.currentUsage -= sizeDelta;
        m_globalStats.activeAllocations--;
    }
    
    // Update category stats
    MemoryStats& categoryStats = m_categoryStats[category];
    if (isAllocation) {
        categoryStats.totalAllocated += sizeDelta;
        categoryStats.currentUsage += sizeDelta;
        categoryStats.activeAllocations++;
        categoryStats.totalAllocations++;
        
        if (categoryStats.currentUsage > categoryStats.peakUsage) {
            categoryStats.peakUsage = categoryStats.currentUsage;
        }
    } else {
        categoryStats.totalDeallocated += sizeDelta;
        categoryStats.currentUsage -= sizeDelta;
        categoryStats.activeAllocations--;
    }
}

} // namespace ItemEditor

#include "MemoryManager.moc"