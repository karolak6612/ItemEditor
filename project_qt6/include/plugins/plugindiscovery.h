#ifndef PLUGINDISCOVERY_H
#define PLUGINDISCOVERY_H

#include "plugins/iplugin.h"
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QPluginLoader>
#include <QLibrary>
#include <QMutex>
#include <QObject>

namespace PluginInterface {

/**
 * @brief Plugin discovery result structure
 * 
 * Contains all information discovered about a plugin including
 * its location, metadata, validation status, and any errors.
 */
struct DiscoveredPlugin {
    QString filePath;                    // Full path to plugin file
    QString fileName;                    // Plugin file name only
    QString typeName;                    // Plugin type identifier
    PluginMetadata metadata;             // Plugin metadata from JSON
    bool isValid;                        // Whether plugin passed validation
    bool isCompatible;                   // Whether plugin is compatible with current version
    bool hasMetadata;                    // Whether metadata was successfully loaded
    QString errorMessage;                // Error description if validation failed
    QStringList missingDependencies;     // List of missing dependencies
    quint64 fileSize;                    // Plugin file size in bytes
    QDateTime lastModified;              // Last modification time
    QString checksum;                    // File checksum for integrity verification

    DiscoveredPlugin() 
        : isValid(false)
        , isCompatible(false)
        , hasMetadata(false)
        , fileSize(0)
    {}
};

/**
 * @brief Plugin discovery configuration
 * 
 * Configuration options for controlling plugin discovery behavior.
 */
struct DiscoveryConfig {
    bool recursive;                      // Scan subdirectories recursively
    bool validatePlugins;                // Perform plugin validation during discovery
    bool loadMetadata;                   // Extract metadata from plugins
    bool checkDependencies;              // Verify plugin dependencies
    bool calculateChecksums;             // Calculate file checksums
    QStringList fileExtensions;          // Allowed plugin file extensions
    QStringList excludePaths;            // Paths to exclude from scanning
    QStringList requiredInterfaces;      // Required interface names
    int maxDepth;                        // Maximum recursion depth (-1 for unlimited)
    int timeoutMs;                       // Timeout for plugin validation in milliseconds

    DiscoveryConfig()
        : recursive(true)
        , validatePlugins(true)
        , loadMetadata(true)
        , checkDependencies(true)
        , calculateChecksums(false)
        , maxDepth(-1)
        , timeoutMs(5000)
    {
#ifdef Q_OS_WINDOWS
        fileExtensions << "*.dll";
#elif defined(Q_OS_MACOS)
        fileExtensions << "*.dylib" << "*.so";
#else
        fileExtensions << "*.so";
#endif
    }
};

/**
 * @brief Plugin discovery statistics
 * 
 * Statistics collected during plugin discovery process.
 */
struct DiscoveryStatistics {
    int totalFilesScanned;
    int pluginsFound;
    int validPlugins;
    int invalidPlugins;
    int compatiblePlugins;
    int incompatiblePlugins;
    qint64 totalSizeBytes;
    int discoveryTimeMs;
    QStringList errorMessages;

    DiscoveryStatistics()
        : totalFilesScanned(0)
        , pluginsFound(0)
        , validPlugins(0)
        , invalidPlugins(0)
        , compatiblePlugins(0)
        , incompatiblePlugins(0)
        , totalSizeBytes(0)
        , discoveryTimeMs(0)
    {}
};

/**
 * @brief Plugin discovery system for finding and validating plugins
 * 
 * This class provides comprehensive plugin discovery functionality including:
 * - Directory scanning with configurable options
 * - Plugin file validation using QPluginLoader
 * - Metadata extraction from plugin JSON
 * - Dependency checking and compatibility verification
 * - Support for both dynamic and static plugins
 * - Thread-safe operations with progress reporting
 */
class PluginDiscovery : public QObject
{
    Q_OBJECT

public:
    explicit PluginDiscovery(QObject* parent = nullptr);
    virtual ~PluginDiscovery();

    // Main discovery methods
    QList<DiscoveredPlugin> scanDirectory(const QString& directory, 
                                         const DiscoveryConfig& config = DiscoveryConfig());
    QList<DiscoveredPlugin> scanDirectories(const QStringList& directories,
                                           const DiscoveryConfig& config = DiscoveryConfig());
    DiscoveredPlugin analyzePlugin(const QString& filePath,
                                  const DiscoveryConfig& config = DiscoveryConfig());

    // Static plugin discovery
    QList<DiscoveredPlugin> discoverStaticPlugins();

    // Plugin validation
    bool validatePluginFile(const QString& filePath);
    bool validatePluginInterface(const QString& filePath, const QStringList& requiredInterfaces);
    bool checkPluginCompatibility(const DiscoveredPlugin& plugin, quint32 apiVersion = 1);

    // Metadata operations
    PluginMetadata loadPluginMetadata(const QString& filePath);
    QJsonObject extractPluginJson(const QString& filePath);
    QString detectPluginType(const QString& filePath);

    // Dependency checking
    QStringList checkPluginDependencies(const DiscoveredPlugin& plugin,
                                       const QList<DiscoveredPlugin>& availablePlugins);
    bool resolveDependencies(QList<DiscoveredPlugin>& plugins);

    // Utility methods
    QStringList getPluginFileExtensions() const;
    QStringList findPluginFiles(const QString& directory, bool recursive = true);
    QString calculateFileChecksum(const QString& filePath);
    bool isPluginFile(const QString& filePath);

    // Configuration
    void setDefaultConfig(const DiscoveryConfig& config);
    DiscoveryConfig getDefaultConfig() const;

    // Statistics and reporting
    DiscoveryStatistics getLastStatistics() const;
    void clearStatistics();

    // Error handling
    QString getLastError() const;
    QStringList getAllErrors() const;
    void clearErrors();

signals:
    void discoveryStarted(const QString& directory);
    void discoveryFinished(const QString& directory, int pluginsFound);
    void pluginFound(const QString& filePath);
    void pluginValidated(const QString& filePath, bool isValid);
    void progressChanged(int percentage, const QString& status);
    void errorOccurred(const QString& error);

public slots:
    void cancelDiscovery();

private slots:
    void onPluginValidationTimeout();

private:
    // Internal discovery methods
    void scanDirectoryRecursive(const QString& directory, 
                               const DiscoveryConfig& config,
                               QList<DiscoveredPlugin>& results,
                               int currentDepth = 0);
    
    DiscoveredPlugin createDiscoveredPlugin(const QString& filePath,
                                          const DiscoveryConfig& config);
    
    bool validatePluginWithLoader(const QString& filePath, QString& errorMessage);
    PluginMetadata parsePluginMetadata(const QJsonObject& json);
    QString generatePluginSignature(const QString& filePath);
    
    // Dependency resolution
    bool checkSingleDependency(const QString& dependency,
                              const QList<DiscoveredPlugin>& availablePlugins);
    QList<DiscoveredPlugin> sortPluginsByDependencies(const QList<DiscoveredPlugin>& plugins);
    
    // Utility methods
    void updateStatistics(const DiscoveredPlugin& plugin);
    void reportProgress(int percentage, const QString& status);
    void addError(const QString& error);
    bool shouldExcludePath(const QString& path, const DiscoveryConfig& config);
    
    // Thread safety
    mutable QMutex m_mutex;
    
    // Configuration and state
    DiscoveryConfig m_defaultConfig;
    DiscoveryStatistics m_statistics;
    QStringList m_errors;
    QString m_lastError;
    bool m_cancelRequested;
    
    // Plugin validation cache
    QMap<QString, bool> m_validationCache;
    QMap<QString, PluginMetadata> m_metadataCache;
    
    // Static plugin cache
    mutable QList<DiscoveredPlugin> m_staticPluginsCache;
    mutable bool m_staticPluginsCacheValid;
};

/**
 * @brief Plugin discovery helper functions
 * 
 * Standalone utility functions for plugin discovery operations.
 */
namespace PluginDiscoveryUtils {
    // File system utilities
    QStringList getSystemPluginPaths();
    QString getApplicationPluginPath();
    bool createPluginDirectory(const QString& path);
    
    // Plugin file utilities
    bool isValidPluginExtension(const QString& fileName);
    QString normalizePluginPath(const QString& path);
    QStringList expandPluginPath(const QString& path);
    
    // Metadata utilities
    QString extractPluginName(const QJsonObject& metadata);
    QString extractPluginVersion(const QJsonObject& metadata);
    QStringList extractPluginDependencies(const QJsonObject& metadata);
    
    // Compatibility checking
    bool isApiVersionCompatible(quint32 pluginVersion, quint32 appVersion);
    bool areInterfacesCompatible(const QStringList& required, const QStringList& provided);
    
    // Error formatting
    QString formatDiscoveryError(const QString& operation, const QString& path, const QString& error);
    QString formatValidationError(const QString& plugin, const QString& error);
}

} // namespace PluginInterface

// Meta-type declarations for Qt's type system
Q_DECLARE_METATYPE(PluginInterface::DiscoveredPlugin)
Q_DECLARE_METATYPE(PluginInterface::DiscoveryConfig)
Q_DECLARE_METATYPE(PluginInterface::DiscoveryStatistics)

#endif // PLUGINDISCOVERY_H