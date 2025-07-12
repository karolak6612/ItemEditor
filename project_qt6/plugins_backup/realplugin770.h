#ifndef REALPLUGIN770_H
#define REALPLUGIN770_H

#include "iplugin.h"
#include "tibiadata/sprparser.h" // For TibiaData::SprParser
#include "tibiadata/datparser.h" // For TibiaData::DatParser
#include <QObject>
#include <QString>
#include <QList>
#include <QMap>

class RealPlugin770 : public QObject, public IPlugin
{
    Q_OBJECT
    // Q_PLUGIN_METADATA(IID IPlugin_iid FILE "realplugin770.json") // Commented out for static linking
    Q_INTERFACES(IPlugin)

public:
    explicit RealPlugin770(QObject *parent = nullptr);
    ~RealPlugin770() override;

    // IPlugin interface
    QString pluginName() const override;
    QString pluginDescription() const override;
    QList<OTB::SupportedClient> getSupportedClients() const override;

    bool loadClient(const OTB::SupportedClient& client,
                    const QString& clientDirectoryPath,
                    bool extended, bool frameDurations, bool transparency, // These options might influence parsing
                    QString& errorString) override;

    bool isClientLoaded() const override;
    const OTB::SupportedClient& getCurrentLoadedClient() const override;

    const QMap<quint16, OTB::ClientItem>& getClientItems() const override;
    bool getClientItem(quint16 clientItemId, OTB::ClientItem& outItem) const override;
    void unloadClient() override;

    bool Initialize() override;

private:
    TibiaData::SprParser m_sprParser;
    TibiaData::DatParser m_datParser;

    QList<OTB::SupportedClient> m_supportedClients;
    QMap<quint16, OTB::ClientItem> m_clientItems; // Populated from DAT and SPR
    bool m_isClientLoaded;
    OTB::SupportedClient m_currentlyLoadedClient;

    // Helper to populate ClientItem's spriteList after DAT/SPR parsing
    void populateSpriteDataForClientItems();
};

#endif // REALPLUGIN770_H
