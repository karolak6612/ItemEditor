#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include "plugins/iplugin.h"
#include "plugins/plugindiscovery.h"
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QPluginLoader>
#include <QLibrary>
#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QElapsedTimer>
#include <QSharedPointer>

namespace PluginInterface {

/**
 * @brief Plugin loading result structure
 * 
 * Contains the result of a plugin loading operation including
 * the loaded plugin instance, loading status, and error information.
 */
struct LoadResult {
    IPlugin* plugin;                     // Loaded plugin instance (nullptr if failed)
    QObject* pluginObject;               // Raw plugin object from QPluginLoader
    QPluginLoader* loader;               // QPluginLoader instance (owned by PluginLoader)
    QString filePath;                    // Path to the loaded plugin file
    bool success;                        // Whether loading was successful
    QString errorMessage;                // Error description if loading failed
    qint64 loadTimeMs;                   // Time taken to load the plugin
    PluginMetadata metadata;             // Plugin metadata
    QStringList warnings;                // Non-fatal warnings during loading

    LoadResult() 
        : plugin(nullptr)
        , pluginObject(nullptr)
        , loader(nullptr)
        , success(false)
        , loadTimeMs(0)
    {}
    
    ~LoadResult() = default;
    
    // Move constructor and assignment
    LoadResult(LoadResult&& other) noexcept
        : plugin(other.plugin)
        , pluginObject(other.pluginObject)
        , loader(other.loader)
        , filePath(std::move(other.filePath))
        , success(other.success)
        , errorMessage(std::move(other.errorMessage))
        , loadTimeMs(other.loadTimeMs)
        , metadata(std::move(other.metadata))
        , warnings(std::move(other.warnings))
    {
        other.plugin = nullptr;
        other.pluginObject = nullptr;
        other.loader = nullptr;
    }
    
    LoadResult& operator=(LoadResult&& other) noexcept
    {
        if (this != &other) {
            plugin = other.plugin;
            pluginObject = other.pluginObject;
            loader = other.loader;
            filePath = std::move(other.filePath);
            success = other.success;
            errorMessage = std::move(other.errorMessage);
            loadTimeMs = other.loadTimeMs;
            metadata = std::move(other.metadata);
            warnings = std::move(other.warnings);
            
            other.plugin = nullptr;
            other.pluginObject = nullptr;
            other.loader = nullptr;
        }
        return *this;
    }
    
private:
    // Disable copy constructor and assignment
    LoadResult(const LoadResult&) = delete;
    LoadResult& operator=(const LoadResult&) = delete;
};

/**
 * @brief Plugin loading configuration
 * 
 * Configuration options for controlling plugin loading behavior.
 */
struct LoadConfig {
    bool initializeAfterLoad;            // Initialize plugin immediately after loading
    bool validateInterface;              // Validate plugin implements required interface
    bool checkDependencies;              // Verify plugin dependencies are available
    bool enableSandbox;                  // Enable plugin sandboxing (if supported)
    int loadTimeoutMs;                   // Timeout for plugin loading in milliseconds
    int initTimeoutMs;                   // Timeout for plugin initialization in milliseconds
    QStringList requiredInterfaces;      // Required interface names
    QLibrary::LoadHints loadHints;       // QLibrary load hints
    QString pluginDirectory;             // Override plugin directory
    bool allowStaticPlugins;             // Allow loading of static plugins

    LoadConfig()
        : initializeAfterLoad(true)
        , validateInterface(true)
        , checkDependencies(true)
        , enableSandbox(false)
        , loadTimeoutMs(10000)
        , initTimeoutMs(5000)
        , loadHints(QLibrary::PreventUnloadHint)
        , allowStaticPlugins(true)
    {}
};

/**
 * @brief Plugin loading statistics
 * 
 * Statistics collected during plugin loading operations.
 */
struct LoadStatistics {
    int totalLoadAttempts;
    int successfulLoads;
    int failedLoads;
    int pluginsInitialized;
    int initializationFailures;
    qint64 totalLoadTimeMs;
    qint64 averageLoadTimeMs;
    QStringList loadedPluginNames;
    QStringList failedPluginPaths;
    QMap<QString, QString> loadErrors;

    LoadStatistics()
        : totalLoadAttempts(0)
        , successfulLoads(0)
        , failedLoads(0)
        , pluginsInitialized(0)
        , initializationFailures(0)
        , totalLoadTimeMs(0)
        , averageLoadTimeMs(0)
    {}
};

/**
 * @brief Plugin loading system for dynamic plugin loading
 * 
 * This class provides comprehensive plugin loading functionality including:
 * - Dynamic plugin loading using Qt's QPluginLoader
 * - Symbol resolution and interface validation
 * - Plugin instantiation with proper error handling
 * - Plugin initialization sequence management
 * - Loading timeout and recovery mechanisms
 * - Support for both dynamic and static plugins
 * - Thread-safe operations with progress reporting
 */
class PluginLoader : public QObject
{
    Q_OBJECT

public:
    explicit PluginLoader(QObject* parent = nullptr);
    virtual ~PluginLoader();

    // Main loading methods
    LoadResult loadPlugin(const QString& filePath, 
                         const LoadConfig& config = LoadConfig());
    LoadResult loadPlugin(const DiscoveredPlugin& discoveredPlugin,
                         const LoadConfig& config = LoadConfig());
    QList<LoadResult> loadPlugins(const QStringList& filePaths,
                                 const LoadConfig& config = LoadConfig());
    QList<LoadResult> loadPlugins(const QList<DiscoveredPlugin>& discoveredPlugins,
                                 const LoadConfig& config = LoadConfig());

    // Static plugin loading
    LoadResult loadStaticPlugin(const QString& pluginName,
                               const LoadConfig& config = LoadConfig());
    QList<LoadResult> loadAllStaticPlugins(const LoadConfig& config = LoadConfig());

    // Plugin unloading
    bool unloadPlugin(const QString& filePath);
    bool unloadPlugin(IPlugin* plugin);
    void unloadAllPlugins();

    // Plugin management
    bool isPluginLoaded(const QString& filePath) const;
    IPlugin* getLoadedPlugin(const QString& filePath) const;
    QList<IPlugin*> getLoadedPlugins() const;
    QStringList getLoadedPluginPaths() const;

    // Plugin initialization
    bool initializePlugin(IPlugin* plugin, IPluginHost* host = nullptr);
    bool initializePlugins(const QList<IPlugin*>& plugins, IPluginHost* host = nullptr);
    void disposePlugin(IPlugin* plugin);
    void disposeAllPlugins();

    // Validation and verification
    bool validatePluginFile(const QString& filePath);
    bool validatePluginInterface(QObject* pluginObject, const QStringList& requiredInterfaces);
    bool verifyPluginDependencies(IPlugin* plugin, const QList<IPlugin*>& availablePlugins);

    // Symbol resolution
    QObject* resolvePluginSymbol(const QString& filePath);
    IPlugin* castToIPlugin(QObject* pluginObject);
    QStringList getPluginInterfaces(QObject* pluginObject);

    // Configuration
    void setDefaultConfig(const LoadConfig& config);
    LoadConfig getDefaultConfig() const;
    void setPluginHost(IPluginHost* host);
    IPluginHost* getPluginHost() const;

    // Statistics and reporting
    LoadStatistics getStatistics() const;
    void clearStatistics();

    // Error handling
    QString getLastError() const;
    QStringList getAllErrors() const;
    void clearErrors();

signals:
    void pluginLoadStarted(const QString& filePath);
    void pluginLoadFinished(const QString& filePath, bool success);
    void pluginInitialized(const QString& pluginName, bool success);
    void pluginUnloaded(const QString& filePath);
    void loadingProgress(int percentage, const QString& status);
    void errorOccurred(const QString& error);
    void warningOccurred(const QString& warning);

public slots:
    void cancelLoading();

private slots:
    void onLoadTimeout();
    void onInitTimeout();

private:
    // Internal loading methods
    LoadResult loadPluginInternal(const QString& filePath, const LoadConfig& config);
    LoadResult loadStaticPluginInternal(const QString& pluginName, const LoadConfig& config);
    
    bool performPluginLoad(QPluginLoader* loader, LoadResult& result, const LoadConfig& config);
    bool performPluginValidation(QObject* pluginObject, LoadResult& result, const LoadConfig& config);
    bool performPluginInitialization(IPlugin* plugin, LoadResult& result, const LoadConfig& config);
    
    // Symbol resolution helpers
    QObject* extractPluginInstance(QPluginLoader* loader);
    bool validateRequiredInterfaces(QObject* pluginObject, const QStringList& required);
    QStringList extractPluginInterfaces(QObject* pluginObject);
    
    // Dependency checking
    bool checkPluginDependencies(IPlugin* plugin, const LoadConfig& config);
    QStringList findMissingDependencies(IPlugin* plugin);
    
    // Error handling and recovery
    void handleLoadError(LoadResult& result, const QString& error);
    void handleLoadWarning(LoadResult& result, const QString& warning);
    void cleanupFailedLoad(QPluginLoader* loader);
    
    // Timeout management
    void setupLoadTimeout(int timeoutMs);
    void setupInitTimeout(int timeoutMs);
    void cancelTimeouts();
    
    // Statistics tracking
    void updateStatistics(const LoadResult& result);
    void recordLoadAttempt(const QString& filePath);
    void recordLoadSuccess(const LoadResult& result);
    void recordLoadFailure(const QString& filePath, const QString& error);
    
    // Utility methods
    void reportProgress(int percentage, const QString& status);
    void addError(const QString& error);
    void addWarning(const QString& warning);
    QString generateLoadId(const QString& filePath);
    
    // Thread safety
    mutable QMutex m_mutex;
    
    // Configuration and state
    LoadConfig m_defaultConfig;
    IPluginHost* m_pluginHost;
    LoadStatistics m_statistics;
    
    // Error tracking
    QStringList m_errors;
    QStringList m_warnings;
    QString m_lastError;
    
    // Loading state
    bool m_cancelRequested;
    QTimer* m_loadTimer;
    QTimer* m_initTimer;
    QString m_currentLoadingPlugin;
    
    // Plugin management
    QMap<QString, LoadResult> m_loadedPlugins;      // filePath -> LoadResult
    QMap<IPlugin*, QString> m_pluginToPath;         // plugin -> filePath
    QList<QPluginLoader*> m_pluginLoaders;          // All active loaders
    
    // Static plugin cache
    QMap<QString, QObject*> m_staticPlugins;        // pluginName -> plugin object
    
    // Loading cache and optimization
    QMap<QString, bool> m_validationCache;          // filePath -> validation result
    QMap<QString, QStringList> m_interfaceCache;    // filePath -> interfaces
};

/**
 * @brief Plugin loading helper functions
 * 
 * Standalone utility functions for plugin loading operations.
 */
namespace PluginLoaderUtils {
    // File and path utilities
    bool isValidPluginFile(const QString& filePath);
    QString normalizePluginPath(const QString& filePath);
    QStringList getPluginSearchPaths();
    QString findPluginFile(const QString& pluginName);
    
    // Loading utilities
    QLibrary::LoadHints getOptimalLoadHints();
    QStringList getSystemRequiredInterfaces();
    bool isPluginCompatible(const PluginMetadata& metadata);
    
    // Error formatting
    QString formatLoadError(const QString& operation, const QString& plugin, const QString& error);
    QString formatInitError(const QString& plugin, const QString& error);
    QString formatTimeoutError(const QString& operation, const QString& plugin, int timeoutMs);
    
    // Validation utilities
    bool validatePluginMetadata(const PluginMetadata& metadata);
    bool validatePluginVersion(const QString& version);
    QStringList extractInterfaceNames(const QMetaObject* metaObject);
    
    // Dependency utilities
    QStringList resolveDependencyOrder(const QList<IPlugin*>& plugins);
    bool hasCyclicDependencies(const QList<IPlugin*>& plugins);
    QStringList findDependencyChain(IPlugin* plugin, const QList<IPlugin*>& availablePlugins);
}

} // namespace PluginInterface

// Meta-type declarations for Qt's type system
Q_DECLARE_METATYPE(PluginInterface::LoadResult)
Q_DECLARE_METATYPE(PluginInterface::LoadConfig)
Q_DECLARE_METATYPE(PluginInterface::LoadStatistics)

#endif // PLUGINLOADER_H