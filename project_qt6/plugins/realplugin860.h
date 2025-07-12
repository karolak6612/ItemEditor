#ifndef REALPLUGIN860_H
#define REALPLUGIN860_H

#include "iplugin.h"
#include "tibiadata/sprparser.h"
#include "tibiadata/datparser.h"
#include <QObject>
#include <QString>
#include <QList>
#include <QMap>

class RealPlugin860 : public QObject, public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPlugin_iid FILE "realplugin860.json")
    Q_INTERFACES(IPlugin)

public:
    explicit RealPlugin860(QObject *parent = nullptr);
    ~RealPlugin860() override;

    // IPlugin interface
    QString pluginName() const override;
    QString pluginDescription() const override;
    QList<OTB::SupportedClient> getSupportedClients() const override;

    bool loadClient(const OTB::SupportedClient& client,
                    const QString& clientDirectoryPath,
                    bool extended, bool frameDurations, bool transparency,
                    QString& errorString) override;

    bool isClientLoaded() const override;
    const OTB::SupportedClient& getCurrentLoadedClient() const override;

    const QMap<quint16, OTB::ClientItem>& getClientItems() const override;
    bool getClientItem(quint16 clientItemId, OTB::ClientItem& outItem) const override;
    void unloadClient() override;

    void Initialize() override;

private:
    TibiaData::SprParser m_sprParser;
    TibiaData::DatParser m_datParser;

    QList<OTB::SupportedClient> m_supportedClients;
    QMap<quint16, OTB::ClientItem> m_clientItems;
    bool m_isClientLoaded;
    OTB::SupportedClient m_currentlyLoadedClient;

    void populateSpriteDataForClientItems();
};

#endif // REALPLUGIN860_H
