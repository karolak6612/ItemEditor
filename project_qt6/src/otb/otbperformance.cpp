#include "otbperformance.h"
#include <QDebug>
#include <QCoreApplication>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <algorithm>

namespace OTB {

// Static members for PerformanceManager
std::unique_ptr<PerformanceMonitor> PerformanceManager::s_monitor = nullptr;
std::unique_ptr<PerformanceOptimizer> PerformanceManager::s_optimizer = nullptr;
QMutex PerformanceManager::s_mutex;
bool PerformanceManager::s_initialized = false;

// IOBuffer implementation
IOBuffer::IOBuffer(qint64 size) : m_size(size) {
    m_buffer.reserve(size);
}

IOBuffer::~IOBuffer() = default;

void IOBuffer::setSize(qint64 size) {
    m_size = size;
    m_buffer.reserve(size);
    if (m_position > size) {
        m_position = size;
    }
}

bool IOBuffer::read(QIODevice* device, QByteArray& data, qint64 maxSize) {
    if (!device || !device->isReadable()) return false;
    
    qint64 bytesToRead = qMin(maxSize, device->bytesAvailable());
    if (bytesToRead <= 0) return false;
    
    data.resize(bytesToRead);
    qint64 bytesRead = device->read(data.data(), bytesToRead);
    
    if (bytesRead > 0) {
        m_bytesRead += bytesRead;
        data.resize(bytesRead);
        return true;
    }
    
    return false;
}

bool IOBuffer::write(QIODevice* device, const QByteArray& data) {
    if (!device || !device->isWritable()) return false;
    
    // Check if we need to flush buffer first
    if (m_position + data.size() > m_size) {
        flush(device);
    }
    
    // If data is larger than buffer, write directly
    if (data.size() > m_size) {
        qint64 bytesWritten = device->write(data);
        if (bytesWritten > 0) {
            m_bytesWritten += bytesWritten;
            return bytesWritten == data.size();
        }
        return false;
    }
    
    // Add to buffer
    if (m_buffer.size() < m_position + data.size()) {
        m_buffer.resize(m_position + data.size());
    }
    
    memcpy(m_buffer.data() + m_position, data.constData(), data.size());
    m_position += data.size();
    
    return true;
}

void IOBuffer::flush(QIODevice* device) {
    if (!device || !device->isWritable() || m_position == 0) return;
    
    qint64 bytesWritten = device->write(m_buffer.constData(), m_position);
    if (bytesWritten > 0) {
        m_bytesWritten += bytesWritten;
        m_flushCount++;
    }
    
    m_position = 0;
}

void IOBuffer::clear() {
    m_buffer.clear();
    m_position = 0;
}

void IOBuffer::reset() {
    clear();
    m_bytesRead = 0;
    m_bytesWritten = 0;
    m_flushCount = 0;
}

bool IOBuffer::readBuffered(QIODevice* device, char* data, qint64 size) {
    if (!device || !data || size <= 0) return false;
    
    // Simple implementation - could be optimized with internal buffering
    return device->read(data, size) == size;
}

bool IOBuffer::readLine(QIODevice* device, QByteArray& line) {
    if (!device) return false;
    
    line = device->readLine();
    if (!line.isEmpty()) {
        m_bytesRead += line.size();
        return true;
    }
    
    return false;
}

void IOBuffer::ensureCapacity(qint64 requiredSize) {
    if (m_buffer.capacity() < requiredSize) {
        m_buffer.reserve(requiredSize);
    }
}

// PerformanceMonitor implementation
PerformanceMonitor::PerformanceMonitor(QObject* parent) : QObject(parent) {
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &PerformanceMonitor::updateMetrics);
    m_updateTimer->setInterval(1000); // Update every second
    
    m_currentMetrics.reset();
}

PerformanceMonitor::~PerformanceMonitor() {
    stopMonitoring();
}

void PerformanceMonitor::startMonitoring() {
    QMutexLocker locker(&m_mutex);
    
    if (!m_monitoring) {
        m_monitoring = true;
        m_paused = false;
        m_timer.start();
        m_updateTimer->start();
        
        qDebug() << "Performance monitoring started";
    }
}

void PerformanceMonitor::stopMonitoring() {
    QMutexLocker locker(&m_mutex);
    
    if (m_monitoring) {
        m_monitoring = false;
        m_updateTimer->stop();
        
        qDebug() << "Performance monitoring stopped";
    }
}

void PerformanceMonitor::pauseMonitoring() {
    QMutexLocker locker(&m_mutex);
    m_paused = true;
}

void PerformanceMonitor::resumeMonitoring() {
    QMutexLocker locker(&m_mutex);
    m_paused = false;
}

void PerformanceMonitor::recordReadOperation(qint64 bytes, qint64 timeMs) {
    if (!m_monitoring || m_paused) return;
    
    QMutexLocker locker(&m_mutex);
    m_currentMetrics.bytesRead += bytes;
    m_currentMetrics.totalReadTime += timeMs;
    m_currentMetrics.readOperations++;
}

void PerformanceMonitor::recordWriteOperation(qint64 bytes, qint64 timeMs) {
    if (!m_monitoring || m_paused) return;
    
    QMutexLocker locker(&m_mutex);
    m_currentMetrics.bytesWritten += bytes;
    m_currentMetrics.totalWriteTime += timeMs;
    m_currentMetrics.writeOperations++;
}

void PerformanceMonitor::recordParseOperation(qint64 items, qint64 timeMs) {
    if (!m_monitoring || m_paused) return;
    
    QMutexLocker locker(&m_mutex);
    m_currentMetrics.itemsProcessed += items;
    m_currentMetrics.totalParseTime += timeMs;
}

void PerformanceMonitor::recordValidationOperation(qint64 timeMs) {
    if (!m_monitoring || m_paused) return;
    
    QMutexLocker locker(&m_mutex);
    m_currentMetrics.totalValidationTime += timeMs;
    m_currentMetrics.validationOperations++;
}

void PerformanceMonitor::recordCacheHit() {
    if (!m_monitoring || m_paused) return;
    
    QMutexLocker locker(&m_mutex);
    m_currentMetrics.cacheHits++;
}

void PerformanceMonitor::recordCacheMiss() {
    if (!m_monitoring || m_paused) return;
    
    QMutexLocker locker(&m_mutex);
    m_currentMetrics.cacheMisses++;
}

void PerformanceMonitor::recordError() {
    if (!m_monitoring || m_paused) return;
    
    QMutexLocker locker(&m_mutex);
    m_currentMetrics.errorsEncountered++;
}

void PerformanceMonitor::recordWarning() {
    if (!m_monitoring || m_paused) return;
    
    QMutexLocker locker(&m_mutex);
    m_currentMetrics.warningsGenerated++;
}

void PerformanceMonitor::recordMemoryUsage(qint64 bytes) {
    if (!m_monitoring || m_paused) return;
    
    QMutexLocker locker(&m_mutex);
    m_currentMetrics.currentMemoryUsage = bytes;
    m_currentMetrics.peakMemoryUsage = qMax(m_currentMetrics.peakMemoryUsage, bytes);
}

PerformanceMetrics PerformanceMonitor::getMetricsSince(const QDateTime& since) const {
    Q_UNUSED(since)
    // For now, return current metrics
    // In a full implementation, this would filter historical data
    QMutexLocker locker(&m_mutex);
    return m_currentMetrics;
}

QList<PerformanceMetrics> PerformanceMonitor::getHistoricalMetrics(int count) const {
    QMutexLocker locker(&m_mutex);
    
    int actualCount = qMin(count, m_historicalMetrics.size());
    return m_historicalMetrics.mid(m_historicalMetrics.size() - actualCount, actualCount);
}

QString PerformanceMonitor::generateReport() const {
    QMutexLocker locker(&m_mutex);
    
    QStringList report;
    report << "=== Performance Report ===";
    report << QString("Monitoring Time: %1 ms").arg(m_timer.elapsed());
    report << "";
    
    // Read/Write Performance
    report << "--- I/O Performance ---";
    report << QString("Bytes Read: %1 KB").arg(m_currentMetrics.bytesRead / 1024);
    report << QString("Bytes Written: %1 KB").arg(m_currentMetrics.bytesWritten / 1024);
    report << QString("Average Read Speed: %1 KB/s").arg(m_currentMetrics.averageReadSpeed() / 1024, 0, 'f', 2);
    report << QString("Average Write Speed: %1 KB/s").arg(m_currentMetrics.averageWriteSpeed() / 1024, 0, 'f', 2);
    report << QString("Read Operations: %1").arg(m_currentMetrics.readOperations);
    report << QString("Write Operations: %1").arg(m_currentMetrics.writeOperations);
    report << "";
    
    // Processing Performance
    report << "--- Processing Performance ---";
    report << QString("Items Processed: %1").arg(m_currentMetrics.itemsProcessed);
    report << QString("Files Processed: %1").arg(m_currentMetrics.filesProcessed);
    report << QString("Total Parse Time: %1 ms").arg(m_currentMetrics.totalParseTime);
    report << QString("Average Item Processing Time: %1 ms").arg(m_currentMetrics.averageItemProcessingTime(), 0, 'f', 3);
    report << QString("Total Validation Time: %1 ms").arg(m_currentMetrics.totalValidationTime);
    report << QString("Validation Operations: %1").arg(m_currentMetrics.validationOperations);
    report << "";
    
    // Memory Performance
    report << "--- Memory Usage ---";
    report << QString("Current Memory Usage: %1 MB").arg(m_currentMetrics.currentMemoryUsage / (1024.0 * 1024.0), 0, 'f', 2);
    report << QString("Peak Memory Usage: %1 MB").arg(m_currentMetrics.peakMemoryUsage / (1024.0 * 1024.0), 0, 'f', 2);
    report << "";
    
    // Cache Performance
    report << "--- Cache Performance ---";
    report << QString("Cache Hits: %1").arg(m_currentMetrics.cacheHits);
    report << QString("Cache Misses: %1").arg(m_currentMetrics.cacheMisses);
    report << QString("Cache Hit Ratio: %1%").arg(m_currentMetrics.cacheHitRatio() * 100, 0, 'f', 2);
    report << "";
    
    // Error Statistics
    report << "--- Error Statistics ---";
    report << QString("Errors Encountered: %1").arg(m_currentMetrics.errorsEncountered);
    report << QString("Warnings Generated: %1").arg(m_currentMetrics.warningsGenerated);
    report << QString("Recovery Attempts: %1").arg(m_currentMetrics.recoveryAttempts);
    
    return report.join('\n');
}

QStringList PerformanceMonitor::getPerformanceWarnings() const {
    QMutexLocker locker(&m_mutex);
    QStringList warnings;
    
    // Check various performance thresholds
    if (m_currentMetrics.averageReadSpeed() < 1024 * 1024) { // < 1MB/s
        warnings << "Read speed is below optimal threshold";
    }
    
    if (m_currentMetrics.averageWriteSpeed() < 1024 * 1024) { // < 1MB/s
        warnings << "Write speed is below optimal threshold";
    }
    
    if (m_currentMetrics.cacheHitRatio() < 0.7) { // < 70%
        warnings << "Cache hit ratio is low";
    }
    
    if (m_currentMetrics.currentMemoryUsage > m_memoryThreshold) {
        warnings << "Memory usage exceeds threshold";
    }
    
    if (m_currentMetrics.errorsEncountered > 0) {
        warnings << QString("Errors encountered: %1").arg(m_currentMetrics.errorsEncountered);
    }
    
    return warnings;
}

QStringList PerformanceMonitor::getOptimizationSuggestions() const {
    QMutexLocker locker(&m_mutex);
    QStringList suggestions;
    
    // Analyze metrics and provide suggestions
    if (m_currentMetrics.cacheHitRatio() < 0.8) {
        suggestions << "Consider increasing cache size to improve hit ratio";
    }
    
    if (m_currentMetrics.averageReadSpeed() < 5 * 1024 * 1024) { // < 5MB/s
        suggestions << "Consider using larger I/O buffers for better read performance";
    }
    
    if (m_currentMetrics.averageItemProcessingTime() > 1.0) { // > 1ms per item
        suggestions << "Item processing time is high, consider optimizing parsing logic";
    }
    
    if (m_currentMetrics.currentMemoryUsage > m_memoryThreshold * 0.8) {
        suggestions << "Memory usage is high, consider enabling compression or reducing cache size";
    }
    
    if (m_currentMetrics.validationOperations > 0 && 
        m_currentMetrics.totalValidationTime / m_currentMetrics.validationOperations > 10) {
        suggestions << "Validation operations are slow, consider optimizing validation logic";
    }
    
    return suggestions;
}

void PerformanceMonitor::setMemoryThreshold(qint64 bytes) {
    QMutexLocker locker(&m_mutex);
    m_memoryThreshold = bytes;
}

void PerformanceMonitor::setPerformanceThreshold(double minSpeed) {
    QMutexLocker locker(&m_mutex);
    m_performanceThreshold = minSpeed;
}

void PerformanceMonitor::setCacheHitRatioThreshold(double minRatio) {
    QMutexLocker locker(&m_mutex);
    m_cacheHitRatioThreshold = minRatio;
}

void PerformanceMonitor::resetMetrics() {
    QMutexLocker locker(&m_mutex);
    m_currentMetrics.reset();
    m_historicalMetrics.clear();
    m_timer.restart();
}

void PerformanceMonitor::saveMetricsToFile(const QString& filePath) {
    QMutexLocker locker(&m_mutex);
    
    QJsonObject json;
    json["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    json["totalReadTime"] = static_cast<double>(m_currentMetrics.totalReadTime);
    json["totalWriteTime"] = static_cast<double>(m_currentMetrics.totalWriteTime);
    json["totalParseTime"] = static_cast<double>(m_currentMetrics.totalParseTime);
    json["bytesRead"] = static_cast<double>(m_currentMetrics.bytesRead);
    json["bytesWritten"] = static_cast<double>(m_currentMetrics.bytesWritten);
    json["itemsProcessed"] = static_cast<double>(m_currentMetrics.itemsProcessed);
    json["cacheHits"] = static_cast<double>(m_currentMetrics.cacheHits);
    json["cacheMisses"] = static_cast<double>(m_currentMetrics.cacheMisses);
    json["peakMemoryUsage"] = static_cast<double>(m_currentMetrics.peakMemoryUsage);
    json["errorsEncountered"] = static_cast<double>(m_currentMetrics.errorsEncountered);
    
    QJsonDocument doc(json);
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        qDebug() << "Performance metrics saved to:" << filePath;
    } else {
        qWarning() << "Failed to save performance metrics to:" << filePath;
    }
}

void PerformanceMonitor::updateMetrics() {
    if (!m_monitoring || m_paused) return;
    
    QMutexLocker locker(&m_mutex);
    
    checkThresholds();
    
    // Archive current metrics to historical data
    if (m_timer.elapsed() % 60000 == 0) { // Every minute
        archiveOldMetrics();
    }
    
    emit metricsUpdated(m_currentMetrics);
}

void PerformanceMonitor::checkThresholds() {
    // Check memory threshold
    if (m_currentMetrics.currentMemoryUsage > m_memoryThreshold) {
        emit memoryThresholdExceeded(m_currentMetrics.currentMemoryUsage, m_memoryThreshold);
    }
    
    // Check performance threshold
    double avgSpeed = (m_currentMetrics.averageReadSpeed() + m_currentMetrics.averageWriteSpeed()) / 2.0;
    if (avgSpeed < m_performanceThreshold) {
        emit performanceThresholdExceeded(avgSpeed, m_performanceThreshold);
    }
    
    // Check cache hit ratio threshold
    double hitRatio = m_currentMetrics.cacheHitRatio();
    if (hitRatio < m_cacheHitRatioThreshold) {
        emit cacheHitRatioLow(hitRatio, m_cacheHitRatioThreshold);
    }
}

void PerformanceMonitor::archiveOldMetrics() {
    m_historicalMetrics.append(m_currentMetrics);
    
    // Keep only last 1000 entries
    if (m_historicalMetrics.size() > 1000) {
        m_historicalMetrics.removeFirst();
    }
}

// PerformanceOptimizer implementation
PerformanceOptimizer::PerformanceOptimizer(QObject* parent) : QObject(parent) {
    m_monitor = PerformanceManager::getMonitor();
    
    m_optimizationTimer = new QTimer(this);
    connect(m_optimizationTimer, &QTimer::timeout, this, &PerformanceOptimizer::performOptimization);
}

void PerformanceOptimizer::optimizeForMemory() {
    qDebug() << "Optimizing for memory usage";
    
    // Reduce cache sizes
    emit optimizationRecommendation("Reducing cache sizes to minimize memory usage");
    
    // Enable compression
    emit optimizationRecommendation("Enabling data compression to reduce memory footprint");
    
    // Use smaller buffer sizes
    emit optimizationRecommendation("Using smaller I/O buffers to reduce memory usage");
    
    emit optimizationCompleted();
}

void PerformanceOptimizer::optimizeForSpeed() {
    qDebug() << "Optimizing for speed";
    
    // Increase cache sizes
    emit optimizationRecommendation("Increasing cache sizes to improve performance");
    
    // Disable compression
    emit optimizationRecommendation("Disabling compression to improve processing speed");
    
    // Use larger buffer sizes
    emit optimizationRecommendation("Using larger I/O buffers to improve throughput");
    
    // Enable parallel processing
    emit optimizationRecommendation("Enabling parallel processing where possible");
    
    emit optimizationCompleted();
}

void PerformanceOptimizer::optimizeForBalance() {
    qDebug() << "Optimizing for balanced performance";
    
    // Use moderate settings
    emit optimizationRecommendation("Using balanced cache sizes");
    emit optimizationRecommendation("Using moderate I/O buffer sizes");
    emit optimizationRecommendation("Selective compression for large data");
    
    emit optimizationCompleted();
}

void PerformanceOptimizer::optimizeForFileSize(qint64 estimatedFileSize) {
    qDebug() << "Optimizing for file size:" << estimatedFileSize;
    
    if (estimatedFileSize > 100 * 1024 * 1024) { // > 100MB
        emit optimizationRecommendation("Large file detected: enabling streaming mode");
        emit optimizationRecommendation("Using large I/O buffers for better throughput");
        emit optimizationRecommendation("Enabling compression to reduce memory usage");
    } else if (estimatedFileSize < 1024 * 1024) { // < 1MB
        emit optimizationRecommendation("Small file detected: using minimal buffering");
        emit optimizationRecommendation("Disabling compression for faster processing");
    } else {
        optimizeForBalance();
    }
    
    emit optimizationCompleted();
}

void PerformanceOptimizer::enableAutoOptimization(bool enabled) {
    m_autoOptimizationEnabled = enabled;
    
    if (enabled) {
        m_optimizationTimer->start(60000); // Every minute
        qDebug() << "Auto-optimization enabled";
    } else {
        m_optimizationTimer->stop();
        qDebug() << "Auto-optimization disabled";
    }
}

void PerformanceOptimizer::setOptimizationInterval(int seconds) {
    if (m_optimizationTimer) {
        m_optimizationTimer->setInterval(seconds * 1000);
    }
}

qint64 PerformanceOptimizer::getOptimalBufferSize(qint64 fileSize) const {
    // Calculate optimal buffer size based on file size
    if (fileSize < 1024 * 1024) { // < 1MB
        return 4 * 1024; // 4KB
    } else if (fileSize < 10 * 1024 * 1024) { // < 10MB
        return 32 * 1024; // 32KB
    } else if (fileSize < 100 * 1024 * 1024) { // < 100MB
        return 128 * 1024; // 128KB
    } else {
        return 1024 * 1024; // 1MB
    }
}

qint64 PerformanceOptimizer::getOptimalCacheSize(qint64 availableMemory) const {
    // Use 25% of available memory for caching
    return availableMemory / 4;
}

int PerformanceOptimizer::getOptimalThreadCount() const {
    // Use number of CPU cores, but limit to reasonable range
    int coreCount = QThread::idealThreadCount();
    return qBound(2, coreCount, 8);
}

void PerformanceOptimizer::configureThreadPool() {
    int optimalThreads = getOptimalThreadCount();
    QThreadPool::globalInstance()->setMaxThreadCount(optimalThreads);
    
    emit optimizationRecommendation(QString("Configured thread pool with %1 threads").arg(optimalThreads));
}

void PerformanceOptimizer::optimizeIOOperations() {
    emit optimizationRecommendation("Optimizing I/O operations");
    
    // Configure optimal buffer sizes based on current performance
    if (m_monitor) {
        const auto& metrics = m_monitor->getCurrentMetrics();
        
        if (metrics.averageReadSpeed() < 1024 * 1024) { // < 1MB/s
            emit optimizationRecommendation("Increasing read buffer size to improve throughput");
        }
        
        if (metrics.averageWriteSpeed() < 1024 * 1024) { // < 1MB/s
            emit optimizationRecommendation("Increasing write buffer size to improve throughput");
        }
    }
}

void PerformanceOptimizer::enableAsyncIO(bool enabled) {
    if (enabled) {
        emit optimizationRecommendation("Enabling asynchronous I/O operations");
    } else {
        emit optimizationRecommendation("Disabling asynchronous I/O operations");
    }
}

void PerformanceOptimizer::performOptimization() {
    if (!m_autoOptimizationEnabled) return;
    
    analyzeAndOptimize();
}

void PerformanceOptimizer::analyzeAndOptimize() {
    qDebug() << "Analyzing performance and applying optimizations";
    
    analyzeCurrentPerformance();
    applyOptimizations();
    
    emit optimizationCompleted();
}

void PerformanceOptimizer::analyzeCurrentPerformance() {
    if (!m_monitor) return;
    
    const auto& metrics = m_monitor->getCurrentMetrics();
    
    qDebug() << "Current performance analysis:";
    qDebug() << "  Read speed:" << metrics.averageReadSpeed() / 1024 << "KB/s";
    qDebug() << "  Write speed:" << metrics.averageWriteSpeed() / 1024 << "KB/s";
    qDebug() << "  Cache hit ratio:" << metrics.cacheHitRatio() * 100 << "%";
    qDebug() << "  Memory usage:" << metrics.currentMemoryUsage / (1024 * 1024) << "MB";
}

void PerformanceOptimizer::applyOptimizations() {
    if (!m_monitor) return;
    
    const auto& metrics = m_monitor->getCurrentMetrics();
    
    // Apply optimizations based on current metrics
    if (metrics.cacheHitRatio() < 0.7) {
        emit optimizationRecommendation("Low cache hit ratio detected - consider increasing cache size");
    }
    
    if (metrics.averageReadSpeed() < 1024 * 1024) { // < 1MB/s
        emit optimizationRecommendation("Low read speed detected - optimizing I/O buffers");
    }
    
    if (metrics.currentMemoryUsage > 512 * 1024 * 1024) { // > 512MB
        emit optimizationRecommendation("High memory usage detected - enabling compression");
    }
}

// PerformanceManager implementation
void PerformanceManager::initialize() {
    QMutexLocker locker(&s_mutex);
    
    if (!s_initialized) {
        s_monitor = std::make_unique<PerformanceMonitor>();
        s_optimizer = std::make_unique<PerformanceOptimizer>();
        s_initialized = true;
        
        qDebug() << "Performance management system initialized";
    }
}

void PerformanceManager::shutdown() {
    QMutexLocker locker(&s_mutex);
    
    if (s_initialized) {
        s_monitor.reset();
        s_optimizer.reset();
        s_initialized = false;
        
        qDebug() << "Performance management system shutdown";
    }
}

PerformanceMonitor* PerformanceManager::getMonitor() {
    QMutexLocker locker(&s_mutex);
    
    if (!s_initialized) {
        initialize();
    }
    
    return s_monitor.get();
}

PerformanceOptimizer* PerformanceManager::getOptimizer() {
    QMutexLocker locker(&s_mutex);
    
    if (!s_initialized) {
        initialize();
    }
    
    return s_optimizer.get();
}

void PerformanceManager::setGlobalBufferSize(qint64 size) {
    Q_UNUSED(size)
    qDebug() << "Setting global buffer size to:" << size;
    // Implementation would configure global buffer settings
}

void PerformanceManager::setGlobalCacheSize(qint64 size) {
    Q_UNUSED(size)
    qDebug() << "Setting global cache size to:" << size;
    // Implementation would configure global cache settings
}

void PerformanceManager::setGlobalThreadCount(int count) {
    QThreadPool::globalInstance()->setMaxThreadCount(count);
    qDebug() << "Setting global thread count to:" << count;
}

void PerformanceManager::applyLowMemoryProfile() {
    qDebug() << "Applying low memory profile";
    setGlobalBufferSize(16 * 1024); // 16KB buffers
    setGlobalCacheSize(32 * 1024 * 1024); // 32MB cache
    setGlobalThreadCount(2); // Minimal threads
}

void PerformanceManager::applyHighPerformanceProfile() {
    qDebug() << "Applying high performance profile";
    setGlobalBufferSize(1024 * 1024); // 1MB buffers
    setGlobalCacheSize(512 * 1024 * 1024); // 512MB cache
    setGlobalThreadCount(QThread::idealThreadCount()); // All available cores
}

void PerformanceManager::applyBalancedProfile() {
    qDebug() << "Applying balanced profile";
    setGlobalBufferSize(128 * 1024); // 128KB buffers
    setGlobalCacheSize(256 * 1024 * 1024); // 256MB cache
    setGlobalThreadCount(qBound(2, QThread::idealThreadCount() / 2, 4)); // Half cores, max 4
}

} // namespace OTB