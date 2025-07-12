#ifndef EXAMPLEPLUGIN_H
#define EXAMPLEPLUGIN_H

#include "plugins/baseplugin.h"

namespace PluginInterface {

/**
 * @brief Example plugin implementation demonstrating the plugin interface
 * 
 * This plugin serves as a template and example for creating new plugins
 * that are compatible with the ItemEditor Qt6 application.
 */
class ExamplePlugin : public BasePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPlugin_iid FILE "exampleplugin.json")
    Q_INTERFACES(PluginInterface::IPlugin)

public:
    explicit ExamplePlugin(QObject* parent = nullptr);
    virtual ~ExamplePlugin();

    // IPlugin interface implementation
    QList<ItemEditor::SupportedClient> getSupportedClients() const override;
    
    bool LoadClient(const ItemEditor::SupportedClient& client,
                   bool extended,
                   bool frameDurations,
                   bool transparency,
                   const QString& datFullPath,
                   const QString& sprFullPath) override;

protected:
    // BasePlugin virtual methods override
    bool doInitialize() override;
    void doDispose() override;
    bool doLoadClient(const ItemEditor::SupportedClient& client,
                     bool extended, bool frameDurations, bool transparency,
                     const QString& datPath, const QString& sprPath) override;
    void doUnloadClient() override;

private:
    void initializeSupportedClients();
    bool parseClientFiles(const QString& datPath, const QString& sprPath);
    void populateClientItems();

    QList<ItemEditor::SupportedClient> m_supportedClients;
    bool m_initialized;
};

} // namespace PluginInterface

#endif // EXAMPLEPLUGIN_H