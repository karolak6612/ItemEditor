#ifndef BASEPLUGIN_H
#define BASEPLUGIN_H

#include "plugins/iplugin.h"
#include <QObject>
#include <QMutex>
#include <QMutexLocker>

namespace PluginInterface {

/**
 * @brief Base implementation of IPlugin interface
 * 
 * This class provides a default implementation of common plugin functionality
 * that concrete plugins can inherit from and override as needed.
 */
class BasePlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface::IPlugin)

public:
    explicit BasePlugin(QObject* parent = nullptr);
    virtual ~BasePlugin();

    // IPlugin interface implementation
    bool Initialize() override;
    void Dispose() override;

    QString pluginName() const override;
    QString pluginDescription() const override;
    QString pluginVersion() const override;

    IPluginHost* getHost() const override;
    void setHost(IPluginHost* host) override;

    ClientItems* getItems() const override;
    quint16 getMinItemId() const override;
    quint16 getMaxItemId() const override;
    QList<ItemEditor::SupportedClient> getSupportedClients() const override = 0; // Pure virtual
    bool isLoaded() const override;

    bool LoadClient(const ItemEditor::SupportedClient& client,
                   bool extended,
                   bool frameDurations,
                   bool transparency,
                   const QString& datFullPath,
                   const QString& sprFullPath) override = 0; // Pure virtual

    ItemEditor::SupportedClient GetClientBySignatures(quint32 datSignature, quint32 sprSignature) const override;
    ItemEditor::ClientItem GetClientItem(quint16 id) const override;
    bool hasClientItem(quint16 id) const override;

    void unloadClient() override;
    QString getLastError() const override;
    bool validateClientFiles(const QString& datPath, const QString& sprPath) const override;

    // Plugin capabilities - default implementations
    bool supportsExtendedMode() const override;
    bool supportsFrameDurations() const override;
    bool supportsTransparency() const override;
    bool supportsVersionDetection() const override;

protected:
    // Protected methods for subclass use
    void setPluginName(const QString& name);
    void setPluginDescription(const QString& description);
    void setPluginVersion(const QString& version);
    void setMinItemId(quint16 minId);
    void setMaxItemId(quint16 maxId);
    void setLoaded(bool loaded);
    void setLastError(const QString& error);

    // Helper methods for subclasses
    void logMessage(const QString& message, int level = 0) const;
    void logError(const QString& error) const;
    void logWarning(const QString& warning) const;
    void logDebug(const QString& debug) const;

    bool loadClientData(const QString& datPath, const QString& sprPath);
    bool validateSignatures(const QString& datPath, const QString& sprPath, 
                          quint32 expectedDatSig, quint32 expectedSprSig) const;

    // Virtual methods for subclass customization
    virtual bool doInitialize();
    virtual void doDispose();
    virtual bool doLoadClient(const ItemEditor::SupportedClient& client,
                            bool extended, bool frameDurations, bool transparency,
                            const QString& datPath, const QString& sprPath);
    virtual void doUnloadClient();

    // Thread safety
    mutable QMutex m_mutex;

private:
    IPluginHost* m_host;
    ClientItems* m_items;
    QString m_pluginName;
    QString m_pluginDescription;
    QString m_pluginVersion;
    quint16 m_minItemId;
    quint16 m_maxItemId;
    bool m_loaded;
    QString m_lastError;
    ItemEditor::SupportedClient m_currentClient;

    // Plugin capabilities
    bool m_supportsExtended;
    bool m_supportsFrameDurations;
    bool m_supportsTransparency;
    bool m_supportsVersionDetection;
};

} // namespace PluginInterface

#endif // BASEPLUGIN_H