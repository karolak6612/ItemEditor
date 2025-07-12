#ifndef REALPLUGIN770_H
#define REALPLUGIN770_H

#include "plugins/iplugin.h"
#include "tibiadata/sprparser.h"
#include "tibiadata/datparser.h"
#include <QObject>
#include <QString>
#include <QList>
#include <QMap>

namespace PluginInterface {

/**
 * @brief Plugin implementation for Tibia client version 7.70
 * 
 * This plugin handles DAT and SPR file formats specific to Tibia client version 7.70.
 * It implements the IPlugin interface to provide seamless integration with the
 * ItemEditor Qt6 application.
 */
class RealPlugin770 : public QObject, public IPlugin
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface::IPlugin)

public:
    explicit RealPlugin770(QObject *parent = nullptr);
    ~RealPlugin770() override;

    // IPlugin interface implementation
    bool Initialize() override;
    void Dispose() override;

    // Plugin identification
    QString pluginName() const override;
    QString pluginDescription() const override;
    QString pluginVersion() const override;

    // Plugin host communication
    IPluginHost* getHost() const override;
    void setHost(IPluginHost* host) override;

    // Client management
    ClientItems* getItems() const override;
    quint16 getMinItemId() const override;
    quint16 getMaxItemId() const override;
    QList<ItemEditor::SupportedClient> getSupportedClients() const override;
    bool isLoaded() const override;

    // Client loading
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

    // Additional methods
    void unloadClient() override;
    QString getLastError() const override;
    bool validateClientFiles(const QString& datPath, const QString& sprPath) const override;

    // Plugin capabilities
    bool supportsExtendedMode() const override;
    bool supportsFrameDurations() const override;
    bool supportsTransparency() const override;
    bool supportsVersionDetection() const override;

private:
    // Plugin host reference
    IPluginHost* m_host;

    // Data parsers
    TibiaData::SprParser m_sprParser;
    TibiaData::DatParser m_datParser;

    // Plugin state
    QList<ItemEditor::SupportedClient> m_supportedClients;
    ClientItems* m_clientItems;
    bool m_isLoaded;
    ItemEditor::SupportedClient m_currentClient;
    QString m_lastError;

    // Item ID range for 7.70
    quint16 m_minItemId;
    quint16 m_maxItemId;

    // Private helper methods
    void initializeSupportedClients();
    bool loadDatFile(const QString& datPath, bool extended);
    bool loadSprFile(const QString& sprPath, bool extended, bool transparency);
    void populateClientItems();
    void calculateItemIdRange();
    bool validateSignatures(const ItemEditor::SupportedClient& client) const;
    void clearData();
    void logMessage(const QString& message, int level = 0) const;
    void logError(const QString& error) const;
};

} // namespace PluginInterface

#endif // REALPLUGIN770_H