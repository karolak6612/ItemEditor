#include "plugins/pluginfactory.h"
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace PluginInterface {

PluginFactory& PluginFactory::instance()
{
    static PluginFactory factory;
    return factory;
}

void PluginFactory::registerPluginType(const QString& typeName, PluginCreator creator)
{
    if (typeName.isEmpty() || !creator) {
        qWarning() << "Invalid plugin type registration:" << typeName;
        return;
    }

    if (m_creators.contains(typeName)) {
        qWarning() << "Plugin type already registered:" << typeName;
        return;
    }

    m_creators[typeName] = creator;
    qDebug() << "Registered plugin type:" << typeName;
}

void PluginFactory::unregisterPluginType(const QString& typeName)
{
    if (m_creators.remove(typeName) > 0) {
        m_typeInfos.remove(typeName);
        qDebug() << "Unregistered plugin type:" << typeName;
    } else {
        qWarning() << "Plugin type not found for unregistration:" << typeName;
    }
}

IPlugin* PluginFactory::createPlugin(const QString& typeName) const
{
    auto it = m_creators.find(typeName);
    if (it == m_creators.end()) {
        qWarning() << "Unknown plugin type:" << typeName;
        return nullptr;
    }

    try {
        IPlugin* plugin = it.value()();
        if (plugin) {
            qDebug() << "Created plugin instance of type:" << typeName;
        } else {
            qWarning() << "Plugin creator returned null for type:" << typeName;
        }
        return plugin;
    } catch (const std::exception& e) {
        qCritical() << "Exception creating plugin of type" << typeName << ":" << e.what();
        return nullptr;
    } catch (...) {
        qCritical() << "Unknown exception creating plugin of type:" << typeName;
        return nullptr;
    }
}

QStringList PluginFactory::getAvailablePluginTypes() const
{
    return m_creators.keys();
}

bool PluginFactory::isPluginTypeRegistered(const QString& typeName) const
{
    return m_creators.contains(typeName);
}void PluginFactory::registerBuiltInPlugins()
{
    // This method can be used to register built-in plugin types
    // For now, it's a placeholder for future built-in plugins
    qDebug() << "Registering built-in plugins...";
    
    // Example registration (commented out as we don't have concrete implementations yet):
    // registerPluginType("TibiaPlugin770", []() -> IPlugin* { return new TibiaPlugin770(); });
    // registerPluginType("TibiaPlugin860", []() -> IPlugin* { return new TibiaPlugin860(); });
}

void PluginFactory::registerPluginTypeInfo(const PluginTypeInfo& info)
{
    if (info.typeName.isEmpty()) {
        qWarning() << "Invalid plugin type info: empty type name";
        return;
    }

    m_typeInfos[info.typeName] = info;
    
    // Also register the creator if provided
    if (info.creator) {
        registerPluginType(info.typeName, info.creator);
    }

    qDebug() << "Registered plugin type info:" << info.typeName;
}

QList<PluginFactory::PluginTypeInfo> PluginFactory::getPluginTypeInfos() const
{
    return m_typeInfos.values();
}

PluginFactory::PluginTypeInfo PluginFactory::getPluginTypeInfo(const QString& typeName) const
{
    return m_typeInfos.value(typeName);
}

//=============================================================================
// PluginDiscovery Implementation
//=============================================================================

QList<PluginDiscovery::DiscoveredPlugin> PluginDiscovery::scanDirectory(const QString& directory)
{
    QList<DiscoveredPlugin> plugins;
    
    QDir dir(directory);
    if (!dir.exists()) {
        qWarning() << "Plugin directory does not exist:" << directory;
        return plugins;
    }

    QStringList nameFilters;
#if defined(Q_OS_WIN)
    nameFilters << "*.dll";
#elif defined(Q_OS_LINUX)
    nameFilters << "*.so";
#elif defined(Q_OS_MACOS)
    nameFilters << "*.dylib";
#endif

    const QStringList files = dir.entryList(nameFilters, QDir::Files);
    for (const QString& fileName : files) {
        QString filePath = dir.absoluteFilePath(fileName);
        DiscoveredPlugin plugin = analyzePlugin(filePath);
        plugins.append(plugin);
    }

    qDebug() << "Discovered" << plugins.size() << "plugins in" << directory;
    return plugins;
}PluginDiscovery::DiscoveredPlugin PluginDiscovery::analyzePlugin(const QString& filePath)
{
    DiscoveredPlugin plugin;
    plugin.filePath = filePath;
    plugin.isValid = false;

    if (!validatePluginFile(filePath)) {
        plugin.errorMessage = "Plugin file validation failed";
        return plugin;
    }

    try {
        plugin.metadata = loadMetadataFromFile(filePath);
        plugin.typeName = detectPluginType(filePath);
        
        if (plugin.typeName.isEmpty()) {
            plugin.errorMessage = "Could not detect plugin type";
        } else {
            plugin.isValid = true;
        }
    } catch (const std::exception& e) {
        plugin.errorMessage = QString("Error analyzing plugin: %1").arg(e.what());
    } catch (...) {
        plugin.errorMessage = "Unknown error analyzing plugin";
    }

    return plugin;
}

bool PluginDiscovery::validatePluginFile(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    
    if (!fileInfo.exists()) {
        qWarning() << "Plugin file does not exist:" << filePath;
        return false;
    }

    if (!fileInfo.isReadable()) {
        qWarning() << "Plugin file is not readable:" << filePath;
        return false;
    }

    if (fileInfo.size() == 0) {
        qWarning() << "Plugin file is empty:" << filePath;
        return false;
    }

    return true;
}

PluginMetadata PluginDiscovery::loadMetadataFromFile(const QString& filePath)
{
    PluginMetadata metadata;
    
    QFileInfo fileInfo(filePath);
    QString metadataPath = fileInfo.absolutePath() + "/" + fileInfo.baseName() + ".json";
    
    QFile metadataFile(metadataPath);
    if (!metadataFile.open(QIODevice::ReadOnly)) {
        // No metadata file found, create basic metadata from filename
        metadata.name = fileInfo.baseName();
        metadata.description = QString("Plugin: %1").arg(metadata.name);
        metadata.version = "1.0.0";
        return metadata;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(metadataFile.readAll(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse plugin metadata:" << metadataPath << error.errorString();
        return metadata;
    }

    if (!doc.isObject()) {
        qWarning() << "Plugin metadata is not a JSON object:" << metadataPath;
        return metadata;
    }    QJsonObject obj = doc.object();
    
    metadata.name = obj["name"].toString(fileInfo.baseName());
    metadata.description = obj["description"].toString();
    metadata.version = obj["version"].toString("1.0.0");
    metadata.author = obj["author"].toString();
    metadata.website = obj["website"].toString();
    metadata.license = obj["license"].toString();
    metadata.apiVersion = obj["apiVersion"].toInt(1);
    metadata.isCompatible = obj["isCompatible"].toBool(true);
    
    QJsonArray deps = obj["dependencies"].toArray();
    for (const QJsonValue& dep : deps) {
        metadata.dependencies.append(dep.toString());
    }

    return metadata;
}

QString PluginDiscovery::detectPluginType(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    QString baseName = fileInfo.baseName().toLower();
    
    // Try to detect plugin type from filename patterns
    if (baseName.contains("770")) {
        return "TibiaPlugin770";
    } else if (baseName.contains("860")) {
        return "TibiaPlugin860";
    } else if (baseName.contains("tibia")) {
        return "TibiaPlugin";
    } else if (baseName.contains("dummy")) {
        return "DummyPlugin";
    }
    
    // Default type if pattern not recognized
    return "GenericPlugin";
}

} // namespace PluginInterface