#include "plugins/pluginservices.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QMutexLocker>
#include <QDebug>

namespace PluginInterface {

// FileIOService implementation

FileIOService::FileIOService(QObject* parent)
    : IPluginService(parent)
    , m_initialized(false)
{
}

bool FileIOService::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    // Add default allowed paths
    QString appDir = QCoreApplication::applicationDirPath();
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    
    m_allowedPaths << appDir << dataDir << tempDir;
    
    m_initialized = true;
    emit serviceInitialized();
    
    return true;
}

void FileIOService::shutdown()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        return;
    }
    
    m_allowedPaths.clear();
    m_initialized = false;
    
    emit serviceShutdown();
}

QStringList FileIOService::getCapabilities() const
{
    return QStringList() << "read" << "write" << "append" << "delete" << "directory" << "validation";
}

bool FileIOService::hasCapability(const QString& capability) const
{
    return getCapabilities().contains(capability);
}

QByteArray FileIOService::readFile(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isPathAllowed(filePath)) {
        emit serviceError(QString("Path not allowed: %1").arg(filePath));
        return QByteArray();
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit serviceError(QString("Cannot open file for reading: %1").arg(filePath));
        return QByteArray();
    }
    
    return file.readAll();
}

bool FileIOService::writeFile(const QString& filePath, const QByteArray& data)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isPathAllowed(filePath)) {
        emit serviceError(QString("Path not allowed: %1").arg(filePath));
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit serviceError(QString("Cannot open file for writing: %1").arg(filePath));
        return false;
    }
    
    qint64 written = file.write(data);
    return written == data.size();
}

bool FileIOService::appendToFile(const QString& filePath, const QByteArray& data)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isPathAllowed(filePath)) {
        emit serviceError(QString("Path not allowed: %1").arg(filePath));
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        emit serviceError(QString("Cannot open file for appending: %1").arg(filePath));
        return false;
    }
    
    qint64 written = file.write(data);
    return written == data.size();
}

bool FileIOService::deleteFile(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isPathAllowed(filePath)) {
        emit serviceError(QString("Path not allowed: %1").arg(filePath));
        return false;
    }
    
    return QFile::remove(filePath);
}

bool FileIOService::fileExists(const QString& filePath) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!isPathAllowed(filePath)) {
        return false;
    }
    
    return QFile::exists(filePath);
}

qint64 FileIOService::fileSize(const QString& filePath) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!isPathAllowed(filePath)) {
        return -1;
    }
    
    QFileInfo info(filePath);
    return info.size();
}

QDateTime FileIOService::fileModificationTime(const QString& filePath) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!isPathAllowed(filePath)) {
        return QDateTime();
    }
    
    QFileInfo info(filePath);
    return info.lastModified();
}

bool FileIOService::createDirectory(const QString& dirPath)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isPathAllowed(dirPath)) {
        emit serviceError(QString("Path not allowed: %1").arg(dirPath));
        return false;
    }
    
    QDir dir;
    return dir.mkpath(dirPath);
}

bool FileIOService::removeDirectory(const QString& dirPath)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isPathAllowed(dirPath)) {
        emit serviceError(QString("Path not allowed: %1").arg(dirPath));
        return false;
    }
    
    QDir dir(dirPath);
    return dir.removeRecursively();
}

QStringList FileIOService::listFiles(const QString& dirPath, const QStringList& filters)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isPathAllowed(dirPath)) {
        emit serviceError(QString("Path not allowed: %1").arg(dirPath));
        return QStringList();
    }
    
    QDir dir(dirPath);
    QStringList nameFilters = filters.isEmpty() ? QStringList("*") : filters;
    return dir.entryList(nameFilters, QDir::Files);
}

QStringList FileIOService::listDirectories(const QString& dirPath)
{
    QMutexLocker locker(&m_mutex);
    
    if (!isPathAllowed(dirPath)) {
        emit serviceError(QString("Path not allowed: %1").arg(dirPath));
        return QStringList();
    }
    
    QDir dir(dirPath);
    return dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
}

bool FileIOService::isPathAllowed(const QString& path) const
{
    QString cleanPath = QDir::cleanPath(path);
    
    for (const QString& allowedPath : m_allowedPaths) {
        if (cleanPath.startsWith(QDir::cleanPath(allowedPath))) {
            return true;
        }
    }
    
    return false;
}

void FileIOService::addAllowedPath(const QString& path)
{
    QMutexLocker locker(&m_mutex);
    
    QString cleanPath = QDir::cleanPath(path);
    if (!m_allowedPaths.contains(cleanPath)) {
        m_allowedPaths.append(cleanPath);
    }
}

void FileIOService::removeAllowedPath(const QString& path)
{
    QMutexLocker locker(&m_mutex);
    
    QString cleanPath = QDir::cleanPath(path);
    m_allowedPaths.removeAll(cleanPath);
}

// ConfigurationService implementation

ConfigurationService::ConfigurationService(QObject* parent)
    : IPluginService(parent)
    , m_initialized(false)
    , m_settings(nullptr)
{
}

bool ConfigurationService::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    m_settings = new QSettings(this);
    m_initialized = true;
    
    emit serviceInitialized();
    return true;
}

void ConfigurationService::shutdown()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        return;
    }
    
    if (m_settings) {
        m_settings->sync();
        delete m_settings;
        m_settings = nullptr;
    }
    
    m_initialized = false;
    emit serviceShutdown();
}

QStringList ConfigurationService::getCapabilities() const
{
    return QStringList() << "read" << "write" << "groups" << "plugin-specific" << "persistence";
}

bool ConfigurationService::hasCapability(const QString& capability) const
{
    return getCapabilities().contains(capability);
}

QVariant ConfigurationService::getValue(const QString& key, const QVariant& defaultValue) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_settings) {
        return defaultValue;
    }
    
    return m_settings->value(key, defaultValue);
}

void ConfigurationService::setValue(const QString& key, const QVariant& value)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_settings) {
        return;
    }
    
    m_settings->setValue(key, value);
    emit configurationChanged(key, value);
}

bool ConfigurationService::hasKey(const QString& key) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_settings) {
        return false;
    }
    
    return m_settings->contains(key);
}

void ConfigurationService::removeKey(const QString& key)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_settings) {
        return;
    }
    
    m_settings->remove(key);
    emit configurationChanged(key, QVariant());
}

QStringList ConfigurationService::getAllKeys() const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_settings) {
        return QStringList();
    }
    
    return m_settings->allKeys();
}

QVariant ConfigurationService::getPluginValue(const QString& pluginName, const QString& key, const QVariant& defaultValue) const
{
    return getValue(QString("plugins/%1/%2").arg(pluginName, key), defaultValue);
}

void ConfigurationService::setPluginValue(const QString& pluginName, const QString& key, const QVariant& value)
{
    setValue(QString("plugins/%1/%2").arg(pluginName, key), value);
}

QStringList ConfigurationService::getPluginKeys(const QString& pluginName) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_settings) {
        return QStringList();
    }
    
    m_settings->beginGroup(QString("plugins/%1").arg(pluginName));
    QStringList keys = m_settings->allKeys();
    m_settings->endGroup();
    
    return keys;
}

void ConfigurationService::beginGroup(const QString& group)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_settings) {
        m_settings->beginGroup(group);
    }
}

void ConfigurationService::endGroup()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_settings) {
        m_settings->endGroup();
    }
}

QString ConfigurationService::currentGroup() const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_settings) {
        return QString();
    }
    
    return m_settings->group();
}

// LoggingService implementation

LoggingService::LoggingService(QObject* parent)
    : IPluginService(parent)
    , m_initialized(false)
    , m_logLevel(Info)
    , m_consoleLogging(true)
{
}

bool LoggingService::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    // Set default log file if not specified
    if (m_logFile.isEmpty()) {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);
        m_logFile = QDir(dataDir).absoluteFilePath("plugin.log");
    }
    
    m_initialized = true;
    emit serviceInitialized();
    
    return true;
}

void LoggingService::shutdown()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        return;
    }
    
    m_initialized = false;
    emit serviceShutdown();
}

QStringList LoggingService::getCapabilities() const
{
    return QStringList() << "console" << "file" << "levels" << "categories" << "timestamps";
}

bool LoggingService::hasCapability(const QString& capability) const
{
    return getCapabilities().contains(capability);
}

void LoggingService::log(LogLevel level, const QString& category, const QString& message)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized || level < m_logLevel) {
        return;
    }
    
    QDateTime timestamp = QDateTime::currentDateTime();
    QString levelStr;
    
    switch (level) {
        case Debug: levelStr = "DEBUG"; break;
        case Info: levelStr = "INFO"; break;
        case Warning: levelStr = "WARNING"; break;
        case Error: levelStr = "ERROR"; break;
        case Critical: levelStr = "CRITICAL"; break;
    }
    
    QString logEntry = QString("[%1] [%2] [%3] %4")
                      .arg(timestamp.toString(Qt::ISODate))
                      .arg(levelStr)
                      .arg(category)
                      .arg(message);
    
    // Console logging
    if (m_consoleLogging) {
        qDebug() << logEntry;
    }
    
    // File logging
    if (!m_logFile.isEmpty()) {
        QFile file(m_logFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
            QTextStream stream(&file);
            stream << logEntry << Qt::endl;
        }
    }
    
    emit logMessage(level, category, message, timestamp);
}

void LoggingService::debug(const QString& category, const QString& message)
{
    log(Debug, category, message);
}

void LoggingService::info(const QString& category, const QString& message)
{
    log(Info, category, message);
}

void LoggingService::warning(const QString& category, const QString& message)
{
    log(Warning, category, message);
}

void LoggingService::error(const QString& category, const QString& message)
{
    log(Error, category, message);
}

void LoggingService::critical(const QString& category, const QString& message)
{
    log(Critical, category, message);
}

void LoggingService::setLogLevel(LogLevel level)
{
    QMutexLocker locker(&m_mutex);
    m_logLevel = level;
}

LoggingService::LogLevel LoggingService::getLogLevel() const
{
    QMutexLocker locker(&m_mutex);
    return m_logLevel;
}

void LoggingService::setLogFile(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);
    m_logFile = filePath;
}

QString LoggingService::getLogFile() const
{
    QMutexLocker locker(&m_mutex);
    return m_logFile;
}

void LoggingService::enableConsoleLogging(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_consoleLogging = enabled;
}

bool LoggingService::isConsoleLoggingEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_consoleLogging;
}

// NetworkService implementation

NetworkService::NetworkService(QObject* parent)
    : IPluginService(parent)
    , m_initialized(false)
    , m_networkManager(nullptr)
{
}

bool NetworkService::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    m_networkManager = new QNetworkAccessManager(this);
    m_initialized = true;
    
    emit serviceInitialized();
    return true;
}

void NetworkService::shutdown()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        return;
    }
    
    if (m_networkManager) {
        delete m_networkManager;
        m_networkManager = nullptr;
    }
    
    m_initialized = false;
    emit serviceShutdown();
}

QStringList NetworkService::getCapabilities() const
{
    return QStringList() << "http-get" << "http-post" << "download" << "domain-filtering" << "timeouts";
}

bool NetworkService::hasCapability(const QString& capability) const
{
    return getCapabilities().contains(capability);
}

QByteArray NetworkService::httpGet(const QString& url, int timeoutMs)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_networkManager || !isUrlAllowed(url)) {
        emit networkError(QString("URL not allowed or network manager not initialized: %1").arg(url));
        return QByteArray();
    }
    
    QNetworkRequest request(url);
    QNetworkReply* reply = m_networkManager->get(request);
    
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    
    timer.start(timeoutMs);
    loop.exec();
    
    QByteArray data;
    if (reply->error() == QNetworkReply::NoError) {
        data = reply->readAll();
    } else {
        emit networkError(reply->errorString());
    }
    
    reply->deleteLater();
    return data;
}

QByteArray NetworkService::httpPost(const QString& url, const QByteArray& data, int timeoutMs)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_networkManager || !isUrlAllowed(url)) {
        emit networkError(QString("URL not allowed or network manager not initialized: %1").arg(url));
        return QByteArray();
    }
    
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    
    QNetworkReply* reply = m_networkManager->post(request, data);
    
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    
    timer.start(timeoutMs);
    loop.exec();
    
    QByteArray responseData;
    if (reply->error() == QNetworkReply::NoError) {
        responseData = reply->readAll();
    } else {
        emit networkError(reply->errorString());
    }
    
    reply->deleteLater();
    return responseData;
}

bool NetworkService::downloadFile(const QString& url, const QString& filePath, int timeoutMs)
{
    QByteArray data = httpGet(url, timeoutMs);
    if (data.isEmpty()) {
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit networkError(QString("Cannot open file for writing: %1").arg(filePath));
        return false;
    }
    
    qint64 written = file.write(data);
    return written == data.size();
}

bool NetworkService::isUrlAllowed(const QString& url) const
{
    if (m_allowedDomains.isEmpty()) {
        return true; // No restrictions if no domains specified
    }
    
    QUrl qurl(url);
    QString domain = qurl.host();
    
    for (const QString& allowedDomain : m_allowedDomains) {
        if (domain.endsWith(allowedDomain)) {
            return true;
        }
    }
    
    return false;
}

void NetworkService::addAllowedDomain(const QString& domain)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_allowedDomains.contains(domain)) {
        m_allowedDomains.append(domain);
    }
}

void NetworkService::removeAllowedDomain(const QString& domain)
{
    QMutexLocker locker(&m_mutex);
    m_allowedDomains.removeAll(domain);
}

bool NetworkService::isNetworkAvailable() const
{
    return m_networkManager && m_networkManager->networkAccessible() == QNetworkAccessManager::Accessible;
}

// ResourceCacheService implementation

ResourceCacheService::ResourceCacheService(QObject* parent)
    : IPluginService(parent)
    , m_initialized(false)
    , m_maxCacheSize(100 * 1024 * 1024) // 100 MB default
    , m_defaultTTL(3600) // 1 hour default
    , m_cleanupTimer(nullptr)
{
}

bool ResourceCacheService::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    m_cleanupTimer = new QTimer(this);
    connect(m_cleanupTimer, &QTimer::timeout, this, &ResourceCacheService::cleanupExpiredEntries);
    m_cleanupTimer->start(60000); // Cleanup every minute
    
    m_initialized = true;
    emit serviceInitialized();
    
    return true;
}

void ResourceCacheService::shutdown()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        return;
    }
    
    if (m_cleanupTimer) {
        m_cleanupTimer->stop();
        delete m_cleanupTimer;
        m_cleanupTimer = nullptr;
    }
    
    clearCache();
    m_initialized = false;
    
    emit serviceShutdown();
}

QStringList ResourceCacheService::getCapabilities() const
{
    return QStringList() << "cache" << "ttl" << "size-limit" << "cleanup" << "statistics";
}

bool ResourceCacheService::hasCapability(const QString& capability) const
{
    return getCapabilities().contains(capability);
}

void ResourceCacheService::cacheResource(const QString& key, const QByteArray& data, int ttlSeconds)
{
    QMutexLocker locker(&m_mutex);
    
    CacheEntry entry;
    entry.data = data;
    entry.size = data.size();
    entry.expirationTime = QDateTime::currentDateTime().addSecs(ttlSeconds);
    
    // Remove existing entry if present
    if (m_cache.contains(key)) {
        m_cache.remove(key);
    }
    
    // Check if we need to make space
    while (getCurrentCacheSize() + entry.size > m_maxCacheSize && !m_cache.isEmpty()) {
        // Remove oldest entry
        auto oldestIt = m_cache.begin();
        for (auto it = m_cache.begin(); it != m_cache.end(); ++it) {
            if (it.value().expirationTime < oldestIt.value().expirationTime) {
                oldestIt = it;
            }
        }
        m_cache.erase(oldestIt);
    }
    
    m_cache[key] = entry;
    emit cacheUpdated(key);
}

QByteArray ResourceCacheService::getCachedResource(const QString& key)
{
    QMutexLocker locker(&m_mutex);
    
    auto it = m_cache.find(key);
    if (it == m_cache.end()) {
        return QByteArray();
    }
    
    // Check if expired
    if (it.value().expirationTime < QDateTime::currentDateTime()) {
        m_cache.erase(it);
        return QByteArray();
    }
    
    return it.value().data;
}

bool ResourceCacheService::hasCachedResource(const QString& key) const
{
    QMutexLocker locker(&m_mutex);
    
    auto it = m_cache.find(key);
    if (it == m_cache.end()) {
        return false;
    }
    
    // Check if expired
    return it.value().expirationTime >= QDateTime::currentDateTime();
}

void ResourceCacheService::removeCachedResource(const QString& key)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_cache.remove(key) > 0) {
        emit cacheUpdated(key);
    }
}

void ResourceCacheService::clearCache()
{
    QMutexLocker locker(&m_mutex);
    
    m_cache.clear();
    emit cacheCleared();
}

void ResourceCacheService::setMaxCacheSize(qint64 maxSize)
{
    QMutexLocker locker(&m_mutex);
    m_maxCacheSize = maxSize;
}

qint64 ResourceCacheService::getMaxCacheSize() const
{
    QMutexLocker locker(&m_mutex);
    return m_maxCacheSize;
}

qint64 ResourceCacheService::getCurrentCacheSize() const
{
    qint64 totalSize = 0;
    for (const CacheEntry& entry : m_cache) {
        totalSize += entry.size;
    }
    return totalSize;
}

void ResourceCacheService::setDefaultTTL(int ttlSeconds)
{
    QMutexLocker locker(&m_mutex);
    m_defaultTTL = ttlSeconds;
}

int ResourceCacheService::getDefaultTTL() const
{
    QMutexLocker locker(&m_mutex);
    return m_defaultTTL;
}

void ResourceCacheService::cleanupExpiredEntries()
{
    QMutexLocker locker(&m_mutex);
    
    QDateTime now = QDateTime::currentDateTime();
    auto it = m_cache.begin();
    
    while (it != m_cache.end()) {
        if (it.value().expirationTime < now) {
            it = m_cache.erase(it);
        } else {
            ++it;
        }
    }
}

// PluginServices implementation

PluginServices::PluginServices(QObject* parent)
    : QObject(parent)
    , m_fileIOService(nullptr)
    , m_configurationService(nullptr)
    , m_loggingService(nullptr)
    , m_networkService(nullptr)
    , m_resourceCacheService(nullptr)
    , m_pluginManager(nullptr)
    , m_autoInitializeServices(true)
{
    initializeBuiltInServices();
    registerBuiltInServices();
}

PluginServices::~PluginServices()
{
    shutdownAllServices();
}

bool PluginServices::registerService(IPluginService* service, QObject* provider, int priority)
{
    QMutexLocker locker(&m_mutex);
    
    if (!validateService(service)) {
        return false;
    }
    
    QString serviceName = service->serviceName();
    
    if (m_services.contains(serviceName)) {
        // Check if new service has higher priority
        if (priority <= m_services[serviceName].priority) {
            return false; // Existing service has higher or equal priority
        }
        
        // Unregister existing service
        unregisterService(serviceName);
    }
    
    ServiceInfo info;
    info.name = serviceName;
    info.version = service->serviceVersion();
    info.description = service->serviceDescription();
    info.capabilities = service->getCapabilities();
    info.service = service;
    info.provider = provider;
    info.priority = priority;
    info.isActive = false;
    info.registrationTime = QDateTime::currentDateTime();
    
    m_services[serviceName] = info;
    m_serviceInstances[serviceName] = service;
    
    // Connect service signals
    connect(service, &IPluginService::serviceInitialized, this, &PluginServices::onServiceInitialized);
    connect(service, &IPluginService::serviceShutdown, this, &PluginServices::onServiceShutdown);
    connect(service, &IPluginService::serviceError, this, &PluginServices::onServiceError);
    
    // Auto-initialize if enabled
    if (m_autoInitializeServices) {
        initializeService(serviceName);
    }
    
    emit serviceRegistered(serviceName);
    return true;
}

void PluginServices::unregisterService(const QString& serviceName)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_services.contains(serviceName)) {
        return;
    }
    
    // Shutdown service if active
    shutdownService(serviceName);
    
    // Clean up
    cleanupService(serviceName);
    
    m_services.remove(serviceName);
    m_serviceInstances.remove(serviceName);
    
    emit serviceUnregistered(serviceName);
}

void PluginServices::unregisterAllServices()
{
    QStringList serviceNames = m_services.keys();
    for (const QString& serviceName : serviceNames) {
        unregisterService(serviceName);
    }
}

IPluginService* PluginServices::getService(const QString& serviceName) const
{
    QMutexLocker locker(&m_mutex);
    
    auto it = m_serviceInstances.find(serviceName);
    if (it != m_serviceInstances.end()) {
        return it.value();
    }
    
    return nullptr;
}

QList<IPluginService*> PluginServices::getAllServices() const
{
    QMutexLocker locker(&m_mutex);
    return m_serviceInstances.values();
}

QList<IPluginService*> PluginServices::getServicesByCapability(const QString& capability) const
{
    QMutexLocker locker(&m_mutex);
    
    QList<IPluginService*> services;
    for (const ServiceInfo& info : m_services.values()) {
        if (info.capabilities.contains(capability)) {
            services.append(info.service);
        }
    }
    
    return services;
}

QStringList PluginServices::getAvailableServices() const
{
    QMutexLocker locker(&m_mutex);
    return m_services.keys();
}

ServiceInfo PluginServices::getServiceInfo(const QString& serviceName) const
{
    QMutexLocker locker(&m_mutex);
    
    auto it = m_services.find(serviceName);
    if (it != m_services.end()) {
        return it.value();
    }
    
    return ServiceInfo();
}

QList<ServiceInfo> PluginServices::getAllServiceInfo() const
{
    QMutexLocker locker(&m_mutex);
    return m_services.values();
}

bool PluginServices::initializeService(const QString& serviceName)
{
    QMutexLocker locker(&m_mutex);
    
    auto it = m_services.find(serviceName);
    if (it == m_services.end()) {
        return false;
    }
    
    ServiceInfo& info = it.value();
    if (info.isActive) {
        return true; // Already initialized
    }
    
    bool success = info.service->initialize();
    if (success) {
        info.isActive = true;
        emit serviceInitialized(serviceName);
    }
    
    return success;
}

void PluginServices::shutdownService(const QString& serviceName)
{
    QMutexLocker locker(&m_mutex);
    
    auto it = m_services.find(serviceName);
    if (it == m_services.end()) {
        return;
    }
    
    ServiceInfo& info = it.value();
    if (!info.isActive) {
        return; // Already shutdown
    }
    
    info.service->shutdown();
    info.isActive = false;
    
    emit serviceShutdown(serviceName);
}

bool PluginServices::initializeAllServices()
{
    QStringList serviceNames = getAvailableServices();
    bool allSuccess = true;
    
    for (const QString& serviceName : serviceNames) {
        if (!initializeService(serviceName)) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

void PluginServices::shutdownAllServices()
{
    QStringList serviceNames = getAvailableServices();
    for (const QString& serviceName : serviceNames) {
        shutdownService(serviceName);
    }
}

bool PluginServices::isServiceInitialized(const QString& serviceName) const
{
    ServiceInfo info = getServiceInfo(serviceName);
    return info.isActive;
}

// Built-in service getters

FileIOService* PluginServices::getFileIOService() const
{
    return m_fileIOService;
}

ConfigurationService* PluginServices::getConfigurationService() const
{
    return m_configurationService;
}

LoggingService* PluginServices::getLoggingService() const
{
    return m_loggingService;
}

NetworkService* PluginServices::getNetworkService() const
{
    return m_networkService;
}

ResourceCacheService* PluginServices::getResourceCacheService() const
{
    return m_resourceCacheService;
}

void PluginServices::setPluginManager(PluginManager* manager)
{
    m_pluginManager = manager;
}

PluginManager* PluginServices::getPluginManager() const
{
    return m_pluginManager;
}

void PluginServices::onPluginLoaded(IPlugin* plugin)
{
    Q_UNUSED(plugin)
    // This could be used to notify services about plugin loading
}

void PluginServices::onPluginUnloaded(IPlugin* plugin)
{
    Q_UNUSED(plugin)
    // This could be used to clean up plugin-specific resources
}

void PluginServices::setAutoInitializeServices(bool autoInit)
{
    QMutexLocker locker(&m_mutex);
    m_autoInitializeServices = autoInit;
}

bool PluginServices::isAutoInitializeServices() const
{
    QMutexLocker locker(&m_mutex);
    return m_autoInitializeServices;
}

void PluginServices::refreshServices()
{
    // This could reload service configurations or refresh service states
    QStringList serviceNames = getAvailableServices();
    for (const QString& serviceName : serviceNames) {
        if (isServiceInitialized(serviceName)) {
            shutdownService(serviceName);
            initializeService(serviceName);
        }
    }
}

// Private slots

void PluginServices::onServiceInitialized()
{
    IPluginService* service = qobject_cast<IPluginService*>(sender());
    if (service) {
        emit serviceInitialized(service->serviceName());
    }
}

void PluginServices::onServiceShutdown()
{
    IPluginService* service = qobject_cast<IPluginService*>(sender());
    if (service) {
        emit serviceShutdown(service->serviceName());
    }
}

void PluginServices::onServiceError(const QString& error)
{
    IPluginService* service = qobject_cast<IPluginService*>(sender());
    if (service) {
        emit serviceError(service->serviceName(), error);
    }
}

// Private implementation methods

void PluginServices::initializeBuiltInServices()
{
    m_fileIOService = new FileIOService(this);
    m_configurationService = new ConfigurationService(this);
    m_loggingService = new LoggingService(this);
    m_networkService = new NetworkService(this);
    m_resourceCacheService = new ResourceCacheService(this);
}

void PluginServices::registerBuiltInServices()
{
    registerService(m_fileIOService, this, 1000);
    registerService(m_configurationService, this, 1000);
    registerService(m_loggingService, this, 1000);
    registerService(m_networkService, this, 1000);
    registerService(m_resourceCacheService, this, 1000);
}

bool PluginServices::validateService(IPluginService* service) const
{
    if (!service) {
        return false;
    }
    
    if (service->serviceName().isEmpty()) {
        return false;
    }
    
    if (service->serviceVersion().isEmpty()) {
        return false;
    }
    
    return true;
}

void PluginServices::cleanupService(const QString& serviceName)
{
    Q_UNUSED(serviceName)
    // This could perform additional cleanup if needed
}

// ServiceUtils namespace implementation

namespace ServiceUtils {

bool isValidServiceName(const QString& name)
{
    return !name.isEmpty() && !name.contains('/') && !name.contains('\\');
}

bool isValidServiceVersion(const QString& version)
{
    return !version.isEmpty() && version.contains('.');
}

QStringList findServicesWithCapability(const QList<ServiceInfo>& services, const QString& capability)
{
    QStringList serviceNames;
    for (const ServiceInfo& info : services) {
        if (info.capabilities.contains(capability)) {
            serviceNames.append(info.name);
        }
    }
    return serviceNames;
}

ServiceInfo findBestService(const QList<ServiceInfo>& services, const QString& capability)
{
    ServiceInfo bestService;
    int highestPriority = -1;
    
    for (const ServiceInfo& info : services) {
        if (info.capabilities.contains(capability) && info.priority > highestPriority) {
            bestService = info;
            highestPriority = info.priority;
        }
    }
    
    return bestService;
}

bool waitForServiceInitialization(IPluginService* service, int timeoutMs)
{
    if (!service || service->isInitialized()) {
        return service != nullptr;
    }
    
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    
    QObject::connect(service, &IPluginService::serviceInitialized, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    
    timer.start(timeoutMs);
    loop.exec();
    
    return service->isInitialized();
}

bool waitForServiceShutdown(IPluginService* service, int timeoutMs)
{
    if (!service || !service->isInitialized()) {
        return true;
    }
    
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    
    QObject::connect(service, &IPluginService::serviceShutdown, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    
    timer.start(timeoutMs);
    loop.exec();
    
    return !service->isInitialized();
}

QString formatServiceError(const QString& serviceName, const QString& operation, const QString& error)
{
    return QString("Service '%1' %2 failed: %3").arg(serviceName, operation, error);
}

} // namespace ServiceUtils

} // namespace PluginInterface