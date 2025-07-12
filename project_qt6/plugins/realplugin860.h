#ifndef REALPLUGIN860_H
#define REALPLUGIN860_H

#include "plugins/iplugin.h"
#include "tibiadata/sprparser.h"
#include "tibiadata/datparser.h"
#include <QObject>
#include <QString>
#include <QList>
#include <QMap>

namespace PluginInterface {

/**
 * @brief Plugin for Tibia client versions 8.60 and higher
 * 
 * This plugin handles modern DAT and SPR file formats with extended features
 * including frame durations, enhanced transparency, and modern attribute support.
 * Supports multiple client versions from 8.60 to latest modern clients.
 */
class RealPlugin860 : public QObject, public IPlugin
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface::IPlugin)

public:
    explicit RealPlugin860(QObject *parent = nullptr);
    ~RealPlugin860() override;

    // Core plugin lifecycle methods
    bool Initialize() override;
    void Dispose() override;

    // Plugin identification
    QString pluginName() const override;
    QString pluginDescription() const override;
    QString pluginVersion() const override;

    // Plugin host communication
    IPluginHost* getHost() const override;
    void setHost(IPluginHost* host) override;

    // Client management - matching C# interface
    ClientItems* getItems() const override;
    quint16 getMinItemId() const override;
    quint16 getMaxItemId() const override;
    QList<ItemEditor::SupportedClient> getSupportedClients() const override;
    bool isLoaded() const override;

    // Client loading with full parameter set from C# interface
    bool LoadClient(const ItemEditor::SupportedClient& client,
                   bool extended,
                   bool frameDurations,
                   bool transparency,
                   const QString& datFullPath,
                   const QString& sprFullPath) override;

    // Client data access
    ItemEditor::SupportedClient GetClientBySignatures(quint32 datSignature, quint32 sprSignature) const override;
    ItemEditor::ClientItem GetClientItem(quint16 id) const override;
    bool hasClientItem(quint16 id) const override;

    // Additional methods for Qt6 specific functionality
    void unloadClient() override;
    QString getLastError() const override;
    bool validateClientFiles(const QString& datPath, const QString& sprPath) const override;

    // Plugin capabilities - enhanced for modern clients
    bool supportsExtendedMode() const override;
    bool supportsFrameDurations() const override;
    bool supportsTransparency() const override;
    bool supportsVersionDetection() const override;

private:
    // Core parsing components
    TibiaData::SprParser m_sprParser;
    TibiaData::DatParser m_datParser;

    // Plugin state
    IPluginHost* m_host;
    ClientItems* m_clientItems;
    bool m_isLoaded;
    
    // Client version management
    QList<ItemEditor::SupportedClient> m_supportedClients;
    ItemEditor::SupportedClient m_currentClient;
    
    // Item ID range tracking
    quint16 m_minItemId;
    quint16 m_maxItemId;
    
    // Error handling
    mutable QString m_lastError;
    
    // Modern client features
    bool m_frameDurationsEnabled;
    bool m_enhancedTransparencyEnabled;
    bool m_extendedAttributesEnabled;

    // Initialization methods
    void initializeSupportedClients();
    void initializeModernFeatures();

    // File loading methods
    bool loadDatFile(const QString& datPath, bool extended, bool enhancedAttributes = true);
    bool loadSprFile(const QString& sprPath, bool extended, bool transparency, bool frameDurations = true);

    // Data processing methods
    void populateClientItems();
    void populateModernSpriteData();
    void calculateItemIdRange();
    
    // Validation methods
    bool validateSignatures(const ItemEditor::SupportedClient& client) const;
    bool validateModernFormat(const ItemEditor::SupportedClient& client) const;
    
    // Helper methods
    void clearData();
    bool isModernClient(quint32 version) const;
    bool supportsEnhancedFeatures(quint32 version) const;
    
    // Logging helper methods
    void logMessage(const QString& message, int level = 0) const;
    void logError(const QString& error) const;
    void logWarning(const QString& warning) const;
    void logDebug(const QString& debug) const;
};

} // namespace PluginInterface

#endif // REALPLUGIN860_H