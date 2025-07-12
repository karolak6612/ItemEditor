#ifndef PLUGINSERVICES_H
#define PLUGINSERVICES_H

#include "plugins/iplugin.h"
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QObject>
#include <QMutex>
#include <QVariant>
#include <QSharedPointer>
#include <QNetworkAccessManager>
#include <QSettings>
#include <QTimer>
#include <functional>

namespace PluginInterface {

// Forward declarations
class PluginManager;

/**
 * @brief Base interface for all plugin services
 * 
 * All services that can be registered with the plugin services system
 * must implement this interface.
 */
class IPluginService : public QObject
{
    Q_OBJECT

public:
    explicit IPluginService(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IPluginService() = default;

    // Service identification
    virtual QString serviceName() const = 0;
    virtual QString serviceVersion() const = 0;
    virtual QString serviceDescription() const = 0;

    // Service lifecycle
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual bool isInitialized() const = 0;

    // Service capabilities
    virtual QStringList getCapabilities() const = 0;
    virtual bool hasCapability(const QString& capability) const = 0;

signals:
    void serviceInitialized();
    void serviceShutdown();
    void serviceError(const QString& error);
};

/**
 * @brief Service registration information
 * 
 * Contains metadata about a registered service.
 */
struct ServiceInfo {
    QString name;                        // Service name
    QString version;                     // Service version
    QString description;                 // Service description
    QStringList capabilities;            // Service capabilities
    IPluginService* service;             // Service instance
    QObject* provider;                   // Service provider (plugin or application)
    int priority;                        // Service priority (higher = preferred)
    bool isActive;                       // Whether service is active
    QDateTime registrationTime;          // When service was registered

    ServiceInfo() 
        : service(nullptr)
        , provider(nullptr)
        , priority(0)
        , isActive(false)
    {}
};

/**
 * @brief File I/O service for plugins
 * 
 * Provides safe file operations for plugins with proper error handling
 * and path validation.
 */
class FileIOService : public IPluginService
{
    Q_OBJECT

public:
    explicit FileIOService(QObject* parent = nullptr);

    // IPluginService implementation
    QString serviceName() const override { return "FileIO"; }
    QString serviceVersion() const override { return "1.0.0"; }
    QString serviceDescription() const override { return "File I/O operations for plugins"; }
    bool initialize() override;
    void shutdown() override;
    bool isInitialized() const override { return m_initialized; }
    QStringList getCapabilities() const override;
    bool hasCapability(const QString& capability) const override;

    // File operations
    QByteArray readFile(const QString& filePath);
    bool writeFile(const QString& filePath, const QByteArray& data);
    bool appendToFile(const QString& filePath, const QByteArray& data);
    bool deleteFile(const QString& filePath);
    bool fileExists(const QString& filePath) const;
    qint64 fileSize(const QString& filePath) const;
    QDateTime fileModificationTime(const QString& filePath) const;

    // Directory operations
    bool createDirectory(const QString& dirPath);
    bool removeDirectory(const QString& dirPath);
    QStringList listFiles(const QString& dirPath, const QStringList& filters = QStringList());
    QStringList listDirectories(const QString& dirPath);

    // Path validation
    bool isPathAllowed(const QString& path) const;
    void addAllowedPath(const QString& path);
    void removeAllowedPath(const QString& path);

private:
    bool m_initialized;
    QStringList m_allowedPaths;
    mutable QMutex m_mutex;
};

/**
 * @brief Configuration service for plugins
 * 
 * Provides centralized configuration management for plugins.
 */
class ConfigurationService : public IPluginService
{
    Q_OBJECT

public:
    explicit ConfigurationService(QObject* parent = nullptr);

    // IPluginService implementation
    QString serviceName() const override { return "Configuration"; }
    QString serviceVersion() const override { return "1.0.0"; }
    QString serviceDescription() const override { return "Configuration management for plugins"; }
    bool initialize() override;
    void shutdown() override;
    bool isInitialized() const override { return m_initialized; }
    QStringList getCapabilities() const override;
    bool hasCapability(const QString& capability) const override;

    // Configuration operations
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;
    void setValue(const QString& key, const QVariant& value);
    bool hasKey(const QString& key) const;
    void removeKey(const QString& key);
    QStringList getAllKeys() const;

    // Plugin-specific configuration
    QVariant getPluginValue(const QString& pluginName, const QString& key, const QVariant& defaultValue = QVariant()) const;
    void setPluginValue(const QString& pluginName, const QString& key, const QVariant& value);
    QStringList getPluginKeys(const QString& pluginName) const;

    // Configuration groups
    void beginGroup(const QString& group);
    void endGroup();
    QString currentGroup() const;

signals:
    void configurationChanged(const QString& key, const QVariant& value);

private:
    bool m_initialized;
    QSettings* m_settings;
    mutable QMutex m_mutex;
};

/**
 * @brief Logging service for plugins
 * 
 * Provides centralized logging capabilities for plugins.
 */
class LoggingService : public IPluginService
{
    Q_OBJECT

public:
    enum LogLevel {
        Debug = 0,
        Info = 1,
        Warning = 2,
        Error = 3,
        Critical = 4
    };

    explicit LoggingService(QObject* parent = nullptr);

    // IPluginService implementation
    QString serviceName() const override { return "Logging"; }
    QString serviceVersion() const override { return "1.0.0"; }
    QString serviceDescription() const override { return "Logging service for plugins"; }
    bool initialize() override;
    void shutdown() override;
    bool isInitialized() const override { return m_initialized; }
    QStringList getCapabilities() const override;
    bool hasCapability(const QString& capability) const override;

    // Logging operations
    void log(LogLevel level, const QString& category, const QString& message);
    void debug(const QString& category, const QString& message);
    void info(const QString& category, const QString& message);
    void warning(const QString& category, const QString& message);
    void error(const QString& category, const QString& message);
    void critical(const QString& category, const QString& message);

    // Log management
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const;
    void setLogFile(const QString& filePath);
    QString getLogFile() const;
    void enableConsoleLogging(bool enabled);
    bool isConsoleLoggingEnabled() const;

signals:
    void logMessage(LogLevel level, const QString& category, const QString& message, const QDateTime& timestamp);

private:
    bool m_initialized;
    LogLevel m_logLevel;
    QString m_logFile;
    bool m_consoleLogging;
    mutable QMutex m_mutex;
};

/**
 * @brief Network service for plugins
 * 
 * Provides network operations for plugins with proper security and error handling.
 */
class NetworkService : public IPluginService
{
    Q_OBJECT

public:
    explicit NetworkService(QObject* parent = nullptr);

    // IPluginService implementation
    QString serviceName() const override { return "Network"; }
    QString serviceVersion() const override { return "1.0.0"; }
    QString serviceDescription() const override { return "Network operations for plugins"; }
    bool initialize() override;
    void shutdown() override;
    bool isInitialized() const override { return m_initialized; }
    QStringList getCapabilities() const override;
    bool hasCapability(const QString& capability) const override;

    // HTTP operations
    QByteArray httpGet(const QString& url, int timeoutMs = 30000);
    QByteArray httpPost(const QString& url, const QByteArray& data, int timeoutMs = 30000);
    bool downloadFile(const QString& url, const QString& filePath, int timeoutMs = 60000);

    // Network utilities
    bool isUrlAllowed(const QString& url) const;
    void addAllowedDomain(const QString& domain);
    void removeAllowedDomain(const QString& domain);
    bool isNetworkAvailable() const;

signals:
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void networkError(const QString& error);

private:
    bool m_initialized;
    QNetworkAccessManager* m_networkManager;
    QStringList m_allowedDomains;
    mutable QMutex m_mutex;
};

/**
 * @brief Resource cache service for plugins
 * 
 * Provides shared resource caching and management for plugins.
 */
class ResourceCacheService : public IPluginService
{
    Q_OBJECT

public:
    explicit ResourceCacheService(QObject* parent = nullptr);

    // IPluginService implementation
    QString serviceName() const override { return "ResourceCache"; }
    QString serviceVersion() const override { return "1.0.0"; }
    QString serviceDescription() const override { return "Resource caching for plugins"; }
    bool initialize() override;
    void shutdown() override;
    bool isInitialized() const override { return m_initialized; }
    QStringList getCapabilities() const override;
    bool hasCapability(const QString& capability) const override;

    // Cache operations
    void cacheResource(const QString& key, const QByteArray& data, int ttlSeconds = 3600);
    QByteArray getCachedResource(const QString& key);
    bool hasCachedResource(const QString& key) const;
    void removeCachedResource(const QString& key);
    void clearCache();

    // Cache management
    void setMaxCacheSize(qint64 maxSize);
    qint64 getMaxCacheSize() const;
    qint64 getCurrentCacheSize() const;
    void setDefaultTTL(int ttlSeconds);
    int getDefaultTTL() const;

signals:
    void cacheUpdated(const QString& key);
    void cacheCleared();

private:
    struct CacheEntry {
        QByteArray data;
        QDateTime expirationTime;
        qint64 size;
    };

    bool m_initialized;
    QMap<QString, CacheEntry> m_cache;
    qint64 m_maxCacheSize;
    int m_defaultTTL;
    QTimer* m_cleanupTimer;
    mutable QMutex m_mutex;

private slots:
    void cleanupExpiredEntries();
};

/**
 * @brief Main plugin services system
 * 
 * Central service registry and provider for all plugin services.
 * Manages service lifecycle, registration, and discovery.
 */
class PluginServices : public QObject
{
    Q_OBJECT

public:
    explicit PluginServices(QObject* parent = nullptr);
    virtual ~PluginServices();

    // Service registration
    bool registerService(IPluginService* service, QObject* provider = nullptr, int priority = 0);
    void unregisterService(const QString& serviceName);
    void unregisterAllServices();

    // Service discovery
    IPluginService* getService(const QString& serviceName) const;
    QList<IPluginService*> getAllServices() const;
    QList<IPluginService*> getServicesByCapability(const QString& capability) const;
    QStringList getAvailableServices() const;
    ServiceInfo getServiceInfo(const QString& serviceName) const;
    QList<ServiceInfo> getAllServiceInfo() const;

    // Service management
    bool initializeService(const QString& serviceName);
    void shutdownService(const QString& serviceName);
    bool initializeAllServices();
    void shutdownAllServices();
    bool isServiceInitialized(const QString& serviceName) const;

    // Built-in services
    FileIOService* getFileIOService() const;
    ConfigurationService* getConfigurationService() const;
    LoggingService* getLoggingService() const;
    NetworkService* getNetworkService() const;
    ResourceCacheService* getResourceCacheService() const;

    // Plugin manager integration
    void setPluginManager(PluginManager* manager);
    PluginManager* getPluginManager() const;

    // Service lifecycle events
    void onPluginLoaded(IPlugin* plugin);
    void onPluginUnloaded(IPlugin* plugin);

    // Configuration
    void setAutoInitializeServices(bool autoInit);
    bool isAutoInitializeServices() const;

signals:
    void serviceRegistered(const QString& serviceName);
    void serviceUnregistered(const QString& serviceName);
    void serviceInitialized(const QString& serviceName);
    void serviceShutdown(const QString& serviceName);
    void serviceError(const QString& serviceName, const QString& error);

public slots:
    void refreshServices();

private slots:
    void onServiceInitialized();
    void onServiceShutdown();
    void onServiceError(const QString& error);

private:
    void initializeBuiltInServices();
    void registerBuiltInServices();
    bool validateService(IPluginService* service) const;
    void cleanupService(const QString& serviceName);

    // Thread safety
    mutable QMutex m_mutex;

    // Service registry
    QMap<QString, ServiceInfo> m_services;
    QMap<QString, IPluginService*> m_serviceInstances;

    // Built-in services
    FileIOService* m_fileIOService;
    ConfigurationService* m_configurationService;
    LoggingService* m_loggingService;
    NetworkService* m_networkService;
    ResourceCacheService* m_resourceCacheService;

    // Plugin manager integration
    PluginManager* m_pluginManager;

    // Configuration
    bool m_autoInitializeServices;
};

/**
 * @brief Service utilities and helpers
 * 
 * Utility functions for working with plugin services.
 */
namespace ServiceUtils {
    // Service validation
    bool isValidServiceName(const QString& name);
    bool isValidServiceVersion(const QString& version);
    
    // Service discovery helpers
    QStringList findServicesWithCapability(const QList<ServiceInfo>& services, const QString& capability);
    ServiceInfo findBestService(const QList<ServiceInfo>& services, const QString& capability);
    
    // Service lifecycle helpers
    bool waitForServiceInitialization(IPluginService* service, int timeoutMs = 5000);
    bool waitForServiceShutdown(IPluginService* service, int timeoutMs = 5000);
    
    // Error handling
    QString formatServiceError(const QString& serviceName, const QString& operation, const QString& error);
}

} // namespace PluginInterface

// Meta-type declarations for Qt's type system
Q_DECLARE_METATYPE(PluginInterface::ServiceInfo)
Q_DECLARE_METATYPE(PluginInterface::LoggingService::LogLevel)

#endif // PLUGINSERVICES_H