#ifndef PLUGINFACTORY_H
#define PLUGINFACTORY_H

#include "plugins/iplugin.h"
#include <QString>
#include <QMap>
#include <QSharedPointer>
#include <functional>

namespace PluginInterface {

/**
 * @brief Factory class for creating and managing plugin instances
 * 
 * This class provides a centralized way to create plugin instances
 * and manage plugin types for the ItemEditor application.
 */
class PluginFactory
{
public:
    // Type alias for plugin creator function
    using PluginCreator = std::function<IPlugin*()>;

    // Singleton access
    static PluginFactory& instance();

    // Plugin type registration
    void registerPluginType(const QString& typeName, PluginCreator creator);
    void unregisterPluginType(const QString& typeName);

    // Plugin creation
    IPlugin* createPlugin(const QString& typeName) const;
    QStringList getAvailablePluginTypes() const;

    // Plugin validation
    bool isPluginTypeRegistered(const QString& typeName) const;

    // Built-in plugin types registration
    void registerBuiltInPlugins();

    // Plugin discovery from metadata
    struct PluginTypeInfo {
        QString typeName;
        QString displayName;
        QString description;
        QString category;
        QStringList supportedVersions;
        PluginCreator creator;
    };

    void registerPluginTypeInfo(const PluginTypeInfo& info);
    QList<PluginTypeInfo> getPluginTypeInfos() const;
    PluginTypeInfo getPluginTypeInfo(const QString& typeName) const;

private:
    PluginFactory() = default;
    ~PluginFactory() = default;
    PluginFactory(const PluginFactory&) = delete;
    PluginFactory& operator=(const PluginFactory&) = delete;

    QMap<QString, PluginCreator> m_creators;
    QMap<QString, PluginTypeInfo> m_typeInfos;
};

/**
 * @brief Template helper for registering plugin types
 */
template<typename PluginType>
class PluginRegistrar
{
public:
    PluginRegistrar(const QString& typeName)
    {
        PluginFactory::instance().registerPluginType(typeName, []() -> IPlugin* {
            return new PluginType();
        });
    }
};

/**
 * @brief Macro for easy plugin registration
 */
#define REGISTER_PLUGIN_TYPE(PluginClass, TypeName) \
    static PluginInterface::PluginRegistrar<PluginClass> g_##PluginClass##_registrar(TypeName);

/**
 * @brief Plugin discovery helper for scanning directories
 */
class PluginDiscovery
{
public:
    struct DiscoveredPlugin {
        QString filePath;
        QString typeName;
        PluginMetadata metadata;
        bool isValid;
        QString errorMessage;
    };

    static QList<DiscoveredPlugin> scanDirectory(const QString& directory);
    static DiscoveredPlugin analyzePlugin(const QString& filePath);
    static bool validatePluginFile(const QString& filePath);

private:
    static PluginMetadata loadMetadataFromFile(const QString& filePath);
    static QString detectPluginType(const QString& filePath);
};

} // namespace PluginInterface

#endif // PLUGINFACTORY_H