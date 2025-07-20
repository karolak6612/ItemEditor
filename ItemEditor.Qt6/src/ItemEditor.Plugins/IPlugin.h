#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QtPlugin>

/**
 * @brief Core plugin interface for ItemEditor
 * 
 * This interface defines the contract that all plugins must implement
 * to provide client data loading and processing capabilities.
 * Maintains exact compatibility with the legacy plugin system.
 */
class IPlugin : public QObject
{
    Q_OBJECT
    
public:
    explicit IPlugin(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IPlugin() = default;
    
    /**
     * @brief Initialize the plugin
     * @return true if initialization successful, false otherwise
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Get plugin name
     * @return Plugin display name
     */
    virtual QString name() const = 0;
    
    /**
     * @brief Get plugin version
     * @return Plugin version string
     */
    virtual QString version() const = 0;
    
    /**
     * @brief Get supported client versions
     * @return List of supported client version strings
     */
    virtual QStringList supportedVersions() const = 0;
    
    /**
     * @brief Load client data from DAT and SPR files
     * @param datPath Path to DAT file
     * @param sprPath Path to SPR file
     * @return true if loading successful, false otherwise
     */
    virtual bool loadClient(const QString& datPath, const QString& sprPath) = 0;
    
    /**
     * @brief Get client data for specific item ID
     * @param clientId Client item ID
     * @return Client data as byte array
     */
    virtual QByteArray getClientData(quint16 clientId) = 0;
    
    /**
     * @brief Get sprite hash for item
     * @param clientId Client item ID
     * @return MD5 hash of sprite data
     */
    virtual QByteArray getSpriteHash(quint16 clientId) = 0;
    
    /**
     * @brief Get sprite signature for similarity comparison
     * @param clientId Client item ID
     * @return Fourier transform signature data
     */
    virtual QByteArray getSpriteSignature(quint16 clientId) = 0;
    
    /**
     * @brief Check if client data is loaded
     * @return true if client data is available
     */
    virtual bool isClientLoaded() const = 0;
    
    /**
     * @brief Get client version currently loaded
     * @return Client version string
     */
    virtual QString getClientVersion() const = 0;
    
    /**
     * @brief Cleanup plugin resources
     */
    virtual void cleanup() = 0;

signals:
    /**
     * @brief Emitted when client loading progress changes
     * @param progress Progress percentage (0-100)
     * @param message Status message
     */
    void loadingProgress(int progress, const QString& message);
    
    /**
     * @brief Emitted when an error occurs
     * @param error Error message
     */
    void errorOccurred(const QString& error);
};

Q_DECLARE_INTERFACE(IPlugin, "com.itemeditor.IPlugin/1.0")