#ifndef DUMMYPLUGIN_H
#define DUMMYPLUGIN_H

#include "iplugin.h" // Base class: IPlugin
#include "otb/item.h" // For OTB::ClientItem, OTB::SupportedClient
#include <QObject> // Required for Q_INTERFACES

class DummyPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPlugin_iid FILE "dummyplugin.json") // Optional for QPluginLoader, but good practice
    Q_INTERFACES(IPlugin)

public:
    DummyPlugin(QObject *parent = nullptr);
    ~DummyPlugin() override;

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

    void Initialize() override {}; // C# IPlugin has this, empty for now

private:
    void createDummyData();

    QList<OTB::SupportedClient> m_supportedClients;
    QMap<quint16, OTB::ClientItem> m_clientItems;
    bool m_isClientLoaded;
    OTB::SupportedClient m_currentlyLoadedClient;
};

#endif // DUMMYPLUGIN_H
