#ifndef IPLUGIN_H
#define IPLUGIN_H

#include "otb/item.h"
#include <QString>
#include <QList>
#include <QMap>
#include <QObject>
#include <QPluginLoader>
#include <QDir>
#include <QSharedPointer>
#include <QVariant>

// Forward declarations
namespace PluginInterface {
    class PluginDiscovery;
    class PluginLoader;
    class VersionManager;
    struct VersionDetectionResult;
}

namespace PluginInterface {

// Forward declarations
class IPluginHost;
class ClientItems;

/**
 * @brief Main plugin interface matching C# IPlugin interface
 * 
 * This interface defines the contract that all plugins must implement
 * to be compatible with the ItemEditor Qt6 application.
 */
class IPlugin
{
public:
    virtual ~IPlugin() = default;

    // Core plugin lifecycle methods
    virtual bool Initialize() = 0;
    virtual void Dispose() = 0;

    // Plugin identification
    virtual QString pluginName() const = 0;
    virtual QString pluginDescription() const = 0;
    virtual QString pluginVersion() const = 0;

    // Plugin host communication
    virtual IPluginHost* getHost() const = 0;
    virtual void setHost(IPluginHost* host) = 0;

    // Client management - matching C# interface
    virtual ClientItems* getItems() const = 0;
    virtual quint16 getMinItemId() const = 0;
    virtual quint16 getMaxItemId() const = 0;
    virtual QList<ItemEditor::SupportedClient> getSupportedClients() const = 0;
    virtual bool isLoaded() const = 0;

    // Client loading with full parameter set from C# interface
    virtual bool LoadClient(const ItemEditor::SupportedClient& client, 
                           bool extended, 
                           bool frameDurations, 
                           bool transparency, 
                           const QString& datFullPath, 
                           const QString& sprFullPath) = 0;

    // Client data access
    virtual ItemEditor::SupportedClient GetClientBySignatures(quint32 datSignature, quint32 sprSignature) const = 0;
    virtual ItemEditor::ClientItem GetClientItem(quint16 id) const = 0;
    virtual bool hasClientItem(quint16 id) const = 0;

    // Additional methods for Qt6 specific functionality
    virtual void unloadClient() = 0;
    virtual QString getLastError() const = 0;
    virtual bool validateClientFiles(const QString& datPath, const QString& sprPath) const = 0;

    // Plugin capabilities
    virtual bool supportsExtendedMode() const = 0;
    virtual bool supportsFrameDurations() const = 0;
    virtual bool supportsTransparency() const = 0;
    virtual bool supportsVersionDetection() const = 0;
};

/**
 * @brief Plugin host interface for communication between plugins and main application
 * 
 * This interface allows plugins to communicate with the main application
 * and access shared resources and services.
 */
class IPluginHost
{
public:
    virtual ~IPluginHost() = default;

    // Application services
    virtual QString getApplicationVersion() const = 0;
    virtual QString getApplicationDirectory() const = 0;
    virtual QString getPluginsDirectory() const = 0;
    virtual QString getTempDirectory() const = 0;

    // Logging and notification services
    virtual void logMessage(const QString& message, int level = 0) const = 0;
    virtual void logError(const QString& error) const = 0;
    virtual void logWarning(const QString& warning) const = 0;
    virtual void logDebug(const QString& debug) const = 0;

    // Progress reporting
    virtual void reportProgress(int percentage, const QString& status = QString()) const = 0;
    virtual void setProgressVisible(bool visible) const = 0;

    // Configuration access
    virtual QVariant getConfigValue(const QString& key, const QVariant& defaultValue = QVariant()) const = 0;
    virtual void setConfigValue(const QString& key, const QVariant& value) const = 0;

    // Resource access
    virtual QByteArray loadResource(const QString& resourcePath) const = 0;
    virtual bool saveResource(const QString& resourcePath, const QByteArray& data) const = 0;

    // Inter-plugin communication
    virtual QList<IPlugin*> getLoadedPlugins() const = 0;
    virtual IPlugin* findPlugin(const QString& pluginName) const = 0;
    virtual bool sendMessage(const QString& targetPlugin, const QString& message, const QVariant& data = QVariant()) const = 0;
};

/**
 * @brief Client items collection matching C# ClientItems class
 * 
 * This class manages a collection of client items with additional metadata
 * and provides methods for efficient access and manipulation.
 */
class ClientItems : public QMap<quint16, ItemEditor::ClientItem>
{
public:
    ClientItems() : signatureCalculated(false) {}
    virtual ~ClientItems() = default;

    // Properties matching C# ClientItems
    bool isSignatureCalculated() const { return signatureCalculated; }
    void setSignatureCalculated(bool calculated) { signatureCalculated = calculated; }

    // Additional methods for Qt6
    void clear();
    bool containsItem(quint16 id) const;
    ItemEditor::ClientItem getItem(quint16 id) const;
    bool setItem(quint16 id, const ItemEditor::ClientItem& item);
    bool removeItem(quint16 id);
    
    // Bulk operations
    QList<quint16> getItemIds() const;
    QList<ItemEditor::ClientItem> getItems() const;
    int getItemCount() const;

    // Search and filter operations
    QList<quint16> findItemsByName(const QString& name) const;
    QList<quint16> findItemsByType(quint8 type) const;
    QList<quint16> findItemsWithProperty(quint32 property) const;

    // Validation and integrity
    bool validateItems() const;
    QStringList getValidationErrors() const;

private:
    bool signatureCalculated;
    mutable QStringList validationErrors;
};

/**
 * @brief Plugin metadata structure for plugin discovery and management
 */
struct PluginMetadata
{
    QString name;
    QString description;
    QString version;
    QString author;
    QString website;
    QString license;
    QStringList dependencies;
    quint32 apiVersion;
    bool isCompatible;

    PluginMetadata() : apiVersion(1), isCompatible(true) {}
};

/**
 * @brief Plugin manager for loading, managing, and coordinating multiple plugins
 * 
 * This class handles the lifecycle of all plugins and provides services
 * for plugin discovery, loading, and inter-plugin communication.
 */
class PluginManager : public QObject, public IPluginHost
{
    Q_OBJECT

public:
    explicit PluginManager(QObject* parent = nullptr);
    virtual ~PluginManager();

    // Plugin lifecycle management
    void loadPlugins(const QString& pluginsPath);
    void loadPlugin(const QString& pluginPath);
    void unloadPlugin(const QString& pluginName);
    void unloadAllPlugins();
    void registerStaticPlugin(IPlugin* plugin);

    // Plugin discovery and access
    QList<IPlugin*> getAvailablePlugins() const;
    QList<IPlugin*> getLoadedPlugins() const override;
    IPlugin* findPlugin(const QString& pluginName) const override;
    IPlugin* findPluginForOtbVersion(quint32 otbVersion) const;
    IPlugin* findPluginForClientVersion(quint32 clientVersion) const;
    IPlugin* findPluginBySignatures(quint32 datSignature, quint32 sprSignature) const;

    // Plugin metadata
    QList<PluginMetadata> getPluginMetadata() const;
    PluginMetadata getPluginMetadata(const QString& pluginName) const;

    // IPluginHost implementation
    QString getApplicationVersion() const override;
    QString getApplicationDirectory() const override;
    QString getPluginsDirectory() const override;
    QString getTempDirectory() const override;

    void logMessage(const QString& message, int level = 0) const override;
    void logError(const QString& error) const override;
    void logWarning(const QString& warning) const override;
    void logDebug(const QString& debug) const override;

    void reportProgress(int percentage, const QString& status = QString()) const override;
    void setProgressVisible(bool visible) const override;

    QVariant getConfigValue(const QString& key, const QVariant& defaultValue = QVariant()) const override;
    void setConfigValue(const QString& key, const QVariant& value) const override;

    QByteArray loadResource(const QString& resourcePath) const override;
    bool saveResource(const QString& resourcePath, const QByteArray& data) const override;

    bool sendMessage(const QString& targetPlugin, const QString& message, const QVariant& data = QVariant()) const override;

    // Configuration
    void setApplicationVersion(const QString& version);
    void setApplicationDirectory(const QString& directory);
    void setPluginsDirectory(const QString& directory);
    void setTempDirectory(const QString& directory);

signals:
    void pluginLoaded(const QString& pluginName);
    void pluginUnloaded(const QString& pluginName);
    void pluginError(const QString& pluginName, const QString& error);
    void progressChanged(int percentage, const QString& status);
    void logMessageEmitted(const QString& message, int level);

public slots:
    void refreshPlugins();

private slots:
    void onPluginDestroyed(QObject* plugin);

private:
    void rebuildLoadedPluginsCache();
    bool validatePlugin(IPlugin* plugin) const;
    PluginMetadata loadPluginMetadata(const QString& pluginPath) const;
    void cleanupPlugin(IPlugin* plugin);

    QList<IPlugin*> m_staticPlugins;
    QList<QPluginLoader*> m_pluginLoaders;
    QList<IPlugin*> m_loadedPluginsCache;
    QMap<QString, PluginMetadata> m_pluginMetadata;

    QString m_applicationVersion;
    QString m_applicationDirectory;
    QString m_pluginsDirectory;
    QString m_tempDirectory;

    mutable QStringList m_logMessages;

    // Subsystem components
    PluginDiscovery* m_discovery;
    PluginLoader* m_loader;
    VersionManager* m_versionManager;
    
    // Plugin tracking
    QMap<QString, IPlugin*> m_pluginPaths;  // filePath -> plugin

private slots:
    void onPluginDiscovered(const QString& filePath);
    void onDiscoveryError(const QString& error);
    void onPluginLoadFinished(const QString& filePath, bool success);
    void onPluginInitialized(const QString& pluginName, bool success);
    void onPluginUnloaded(const QString& filePath);
    void onLoaderError(const QString& error);
    void onVersionDetected(const VersionDetectionResult& result);

private:
    void initializeDefaultConfiguration();
    void registerLoadedPlugin(IPlugin* plugin, const QString& filePath);
    void unregisterLoadedPlugin(IPlugin* plugin);
};

} // namespace PluginInterface

// Qt plugin system integration
#define IPlugin_iid "com.ItemEditorQt.IPlugin/1.0"
Q_DECLARE_INTERFACE(PluginInterface::IPlugin, IPlugin_iid)

// Meta-type declarations for Qt's type system
Q_DECLARE_METATYPE(PluginInterface::PluginMetadata)

#endif // IPLUGIN_H