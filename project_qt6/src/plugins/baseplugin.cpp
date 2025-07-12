#include "plugins/baseplugin.h"
#include <QFileInfo>
#include <QDebug>

namespace PluginInterface {

BasePlugin::BasePlugin(QObject* parent)
    : QObject(parent)
    , m_host(nullptr)
    , m_items(new ClientItems())
    , m_minItemId(0)
    , m_maxItemId(0)
    , m_loaded(false)
    , m_supportsExtended(true)
    , m_supportsFrameDurations(true)
    , m_supportsTransparency(true)
    , m_supportsVersionDetection(true)
{
}

BasePlugin::~BasePlugin()
{
    Dispose();
    delete m_items;
}

bool BasePlugin::Initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_pluginName.isEmpty()) {
        m_lastError = "Plugin name not set";
        return false;
    }

    if (m_pluginVersion.isEmpty()) {
        m_lastError = "Plugin version not set";
        return false;
    }

    bool result = doInitialize();
    if (!result && m_lastError.isEmpty()) {
        m_lastError = "Plugin initialization failed";
    }

    return result;
}

void BasePlugin::Dispose()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_loaded) {
        unloadClient();
    }

    doDispose();
    
    if (m_items) {
        m_items->clear();
    }
}

QString BasePlugin::pluginName() const
{
    QMutexLocker locker(&m_mutex);
    return m_pluginName;
}

QString BasePlugin::pluginDescription() const
{
    QMutexLocker locker(&m_mutex);
    return m_pluginDescription;
}

QString BasePlugin::pluginVersion() const
{
    QMutexLocker locker(&m_mutex);
    return m_pluginVersion;
}

IPluginHost* BasePlugin::getHost() const
{
    QMutexLocker locker(&m_mutex);
    return m_host;
}

void BasePlugin::setHost(IPluginHost* host)
{
    QMutexLocker locker(&m_mutex);
    m_host = host;
}ClientItems* BasePlugin::getItems() const
{
    QMutexLocker locker(&m_mutex);
    return m_items;
}

quint16 BasePlugin::getMinItemId() const
{
    QMutexLocker locker(&m_mutex);
    return m_minItemId;
}

quint16 BasePlugin::getMaxItemId() const
{
    QMutexLocker locker(&m_mutex);
    return m_maxItemId;
}

bool BasePlugin::isLoaded() const
{
    QMutexLocker locker(&m_mutex);
    return m_loaded;
}

ItemEditor::SupportedClient BasePlugin::GetClientBySignatures(quint32 datSignature, quint32 sprSignature) const
{
    QMutexLocker locker(&m_mutex);
    
    const QList<ItemEditor::SupportedClient> clients = getSupportedClients();
    for (const ItemEditor::SupportedClient& client : clients) {
        if (client.datSignature == datSignature && client.sprSignature == sprSignature) {
            return client;
        }
    }

    // Return empty client if not found
    return ItemEditor::SupportedClient();
}

ItemEditor::ClientItem BasePlugin::GetClientItem(quint16 id) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_loaded || !m_items) {
        return ItemEditor::ClientItem();
    }

    return m_items->getItem(id);
}

bool BasePlugin::hasClientItem(quint16 id) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_loaded || !m_items) {
        return false;
    }

    return m_items->containsItem(id);
}

void BasePlugin::unloadClient()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_loaded) {
        return;
    }

    doUnloadClient();
    
    if (m_items) {
        m_items->clear();
    }

    m_loaded = false;
    m_currentClient = ItemEditor::SupportedClient();
    m_lastError.clear();
}QString BasePlugin::getLastError() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastError;
}

bool BasePlugin::validateClientFiles(const QString& datPath, const QString& sprPath) const
{
    QFileInfo datInfo(datPath);
    QFileInfo sprInfo(sprPath);

    if (!datInfo.exists()) {
        const_cast<BasePlugin*>(this)->m_lastError = QString("DAT file does not exist: %1").arg(datPath);
        return false;
    }

    if (!sprInfo.exists()) {
        const_cast<BasePlugin*>(this)->m_lastError = QString("SPR file does not exist: %1").arg(sprPath);
        return false;
    }

    if (!datInfo.isReadable()) {
        const_cast<BasePlugin*>(this)->m_lastError = QString("DAT file is not readable: %1").arg(datPath);
        return false;
    }

    if (!sprInfo.isReadable()) {
        const_cast<BasePlugin*>(this)->m_lastError = QString("SPR file is not readable: %1").arg(sprPath);
        return false;
    }

    return true;
}

// Plugin capabilities
bool BasePlugin::supportsExtendedMode() const
{
    QMutexLocker locker(&m_mutex);
    return m_supportsExtended;
}

bool BasePlugin::supportsFrameDurations() const
{
    QMutexLocker locker(&m_mutex);
    return m_supportsFrameDurations;
}

bool BasePlugin::supportsTransparency() const
{
    QMutexLocker locker(&m_mutex);
    return m_supportsTransparency;
}

bool BasePlugin::supportsVersionDetection() const
{
    QMutexLocker locker(&m_mutex);
    return m_supportsVersionDetection;
}

// Protected setters
void BasePlugin::setPluginName(const QString& name)
{
    QMutexLocker locker(&m_mutex);
    m_pluginName = name;
}

void BasePlugin::setPluginDescription(const QString& description)
{
    QMutexLocker locker(&m_mutex);
    m_pluginDescription = description;
}

void BasePlugin::setPluginVersion(const QString& version)
{
    QMutexLocker locker(&m_mutex);
    m_pluginVersion = version;
}void BasePlugin::setMinItemId(quint16 minId)
{
    QMutexLocker locker(&m_mutex);
    m_minItemId = minId;
}

void BasePlugin::setMaxItemId(quint16 maxId)
{
    QMutexLocker locker(&m_mutex);
    m_maxItemId = maxId;
}

void BasePlugin::setLoaded(bool loaded)
{
    QMutexLocker locker(&m_mutex);
    m_loaded = loaded;
}

void BasePlugin::setLastError(const QString& error)
{
    QMutexLocker locker(&m_mutex);
    m_lastError = error;
}

// Helper methods
void BasePlugin::logMessage(const QString& message, int level) const
{
    if (m_host) {
        m_host->logMessage(QString("[%1] %2").arg(m_pluginName).arg(message), level);
    } else {
        qDebug() << QString("[%1] %2").arg(m_pluginName).arg(message);
    }
}

void BasePlugin::logError(const QString& error) const
{
    if (m_host) {
        m_host->logError(QString("[%1] %2").arg(m_pluginName).arg(error));
    } else {
        qCritical() << QString("[%1] ERROR: %2").arg(m_pluginName).arg(error);
    }
}

void BasePlugin::logWarning(const QString& warning) const
{
    if (m_host) {
        m_host->logWarning(QString("[%1] %2").arg(m_pluginName).arg(warning));
    } else {
        qWarning() << QString("[%1] WARNING: %2").arg(m_pluginName).arg(warning);
    }
}

void BasePlugin::logDebug(const QString& debug) const
{
    if (m_host) {
        m_host->logDebug(QString("[%1] %2").arg(m_pluginName).arg(debug));
    } else {
        qDebug() << QString("[%1] DEBUG: %2").arg(m_pluginName).arg(debug);
    }
}

bool BasePlugin::loadClientData(const QString& datPath, const QString& sprPath)
{
    // Basic validation
    if (!validateClientFiles(datPath, sprPath)) {
        return false;
    }

    // This is a placeholder - subclasses should override doLoadClient
    // to implement actual client data loading
    return true;
}bool BasePlugin::validateSignatures(const QString& datPath, const QString& sprPath,
                                   quint32 expectedDatSig, quint32 expectedSprSig) const
{
    // This is a placeholder implementation
    // Real implementation would read file headers and validate signatures
    Q_UNUSED(datPath)
    Q_UNUSED(sprPath)
    Q_UNUSED(expectedDatSig)
    Q_UNUSED(expectedSprSig)
    
    // For now, just return true - subclasses should implement proper validation
    return true;
}

// Virtual methods with default implementations
bool BasePlugin::doInitialize()
{
    // Default implementation - subclasses can override
    logDebug("Default initialization completed");
    return true;
}

void BasePlugin::doDispose()
{
    // Default implementation - subclasses can override
    logDebug("Default disposal completed");
}

bool BasePlugin::doLoadClient(const ItemEditor::SupportedClient& client,
                            bool extended, bool frameDurations, bool transparency,
                            const QString& datPath, const QString& sprPath)
{
    // Default implementation - subclasses must override for actual functionality
    Q_UNUSED(client)
    Q_UNUSED(extended)
    Q_UNUSED(frameDurations)
    Q_UNUSED(transparency)
    Q_UNUSED(datPath)
    Q_UNUSED(sprPath)
    
    m_lastError = "doLoadClient not implemented in subclass";
    return false;
}

void BasePlugin::doUnloadClient()
{
    // Default implementation - subclasses can override
    logDebug("Default client unload completed");
}

} // namespace PluginInterface

#include "baseplugin.moc"