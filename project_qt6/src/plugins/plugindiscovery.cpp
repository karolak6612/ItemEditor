#include "plugins/plugindiscovery.h"
#include "plugins/iplugin.h"
#include <QApplication>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QTimer>
#include <QElapsedTimer>
#include <QCryptographicHash>
#include <QMutexLocker>
#include <QThread>
#include <QDebug>
#include <QJsonParseError>

namespace PluginInterface {

PluginDiscovery::PluginDiscovery(QObject* parent)
    : QObject(parent)
    , m_cancelRequested(false)
    , m_staticPluginsCacheValid(false)
{
}

PluginDiscovery::~PluginDiscovery()
{
}

QList<DiscoveredPlugin> PluginDiscovery::scanDirectory(const QString& directory, const DiscoveryConfig& config)
{
    QMutexLocker locker(&m_mutex);
    
    QElapsedTimer timer;
    timer.start();
    
    clearStatistics();
    clearErrors();
    m_cancelRequested = false;
    
    QList<DiscoveredPlugin> results;
    
    emit discoveryStarted(directory);
    reportProgress(0, QStringLiteral("Starting plugin discovery in: %1").arg(directory));
    
    try {
        QDir dir(directory);
        if (!dir.exists()) {
            addError(QStringLiteral("Directory does not exist: %1").arg(directory));
            return results;
        }
        
        scanDirectoryRecursive(directory, config, results, 0);
        
        // Resolve dependencies if requested
        if (config.checkDependencies && !results.isEmpty()) {
            reportProgress(90, "Resolving plugin dependencies...");
            resolveDependencies(results);
        }
        
    } catch (const std::exception& e) {
        addError(QStringLiteral("Exception during discovery: %1").arg(e.what()));
    }
    
    m_statistics.discoveryTimeMs = timer.elapsed();
    
    emit discoveryFinished(directory, results.size());
    reportProgress(100, QStringLiteral("Discovery completed. Found %1 plugins.").arg(results.size()));
    
    return results;
}

QList<DiscoveredPlugin> PluginDiscovery::scanDirectories(const QStringList& directories, const DiscoveryConfig& config)
{
    QList<DiscoveredPlugin> allResults;
    
    for (const QString& directory : directories) {
        if (m_cancelRequested) break;
        
        QList<DiscoveredPlugin> results = scanDirectory(directory, config);
        allResults.append(results);
    }
    
    return allResults;
}

DiscoveredPlugin PluginDiscovery::analyzePlugin(const QString& filePath, const DiscoveryConfig& config)
{
    QMutexLocker locker(&m_mutex);
    
    DiscoveredPlugin plugin = createDiscoveredPlugin(filePath, config);
    updateStatistics(plugin);
    
    return plugin;
}

QList<DiscoveredPlugin> PluginDiscovery::discoverStaticPlugins()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_staticPluginsCacheValid) {
        return m_staticPluginsCache;
    }
    
    m_staticPluginsCache.clear();
    
    const QObjectList staticPlugins = QPluginLoader::staticInstances();
    for (QObject* plugin : staticPlugins) {
        DiscoveredPlugin discovered;
        discovered.filePath = QStringLiteral("static://") + plugin->metaObject()->className();
        discovered.fileName = plugin->metaObject()->className();
        discovered.isValid = true;
        discovered.isCompatible = true;
        discovered.hasMetadata = false;
        
        // Try to cast to IPlugin to get metadata
        if (IPlugin* iplugin = qobject_cast<IPlugin*>(plugin)) {
            discovered.metadata.name = iplugin->pluginName();
            discovered.metadata.description = iplugin->pluginDescription();
            discovered.metadata.version = iplugin->pluginVersion();
            discovered.hasMetadata = true;
        }
        
        discovered.typeName = QStringLiteral("static");
        m_staticPluginsCache.append(discovered);
    }
    
    m_staticPluginsCacheValid = true;
    return m_staticPluginsCache;
}

bool PluginDiscovery::validatePluginFile(const QString& filePath)
{
    // Check cache first
    if (m_validationCache.contains(filePath)) {
        return m_validationCache[filePath];
    }
    
    QString errorMessage;
    bool isValid = validatePluginWithLoader(filePath, errorMessage);
    
    // Cache the result
    m_validationCache[filePath] = isValid;
    
    if (!isValid) {
        addError(QStringLiteral("Plugin validation failed for %1: %2").arg(filePath, errorMessage));
    }
    
    emit pluginValidated(filePath, isValid);
    return isValid;
}

bool PluginDiscovery::validatePluginInterface(const QString& filePath, const QStringList& requiredInterfaces)
{
    if (requiredInterfaces.isEmpty()) {
        return true; // No specific interface requirements
    }
    
    QPluginLoader loader(filePath);
    QJsonObject metadata = loader.metaData();
    
    if (metadata.isEmpty()) {
        return false;
    }
    
    QJsonObject metaDataObj = metadata[QStringLiteral("MetaData")].toObject();
    QJsonArray interfaces = metaDataObj[QStringLiteral("interfaces")].toArray();
    
    QStringList providedInterfaces;
    for (const QJsonValue& value : interfaces) {
        providedInterfaces << value.toString();
    }
    
    return PluginDiscoveryUtils::areInterfacesCompatible(requiredInterfaces, providedInterfaces);
}

bool PluginDiscovery::checkPluginCompatibility(const DiscoveredPlugin& plugin, quint32 apiVersion)
{
    if (!plugin.hasMetadata) {
        return false; // Cannot determine compatibility without metadata
    }
    
    return PluginDiscoveryUtils::isApiVersionCompatible(plugin.metadata.apiVersion, apiVersion);
}

PluginMetadata PluginDiscovery::loadPluginMetadata(const QString& filePath)
{
    // Check cache first
    if (m_metadataCache.contains(filePath)) {
        return m_metadataCache[filePath];
    }
    
    PluginMetadata metadata;
    QJsonObject json = extractPluginJson(filePath);
    
    if (!json.isEmpty()) {
        metadata = parsePluginMetadata(json);
        m_metadataCache[filePath] = metadata;
    }
    
    return metadata;
}

QJsonObject PluginDiscovery::extractPluginJson(const QString& filePath)
{
    QPluginLoader loader(filePath);
    QJsonObject metaData = loader.metaData();
    
    if (metaData.isEmpty()) {
        addError(QStringLiteral("No metadata found in plugin: %1").arg(filePath));
        return QJsonObject();
    }
    
    return metaData[QStringLiteral("MetaData")].toObject();
}

QString PluginDiscovery::detectPluginType(const QString& filePath)
{
    QJsonObject json = extractPluginJson(filePath);
    
    if (json.contains(QStringLiteral("type"))) {
        return json[QStringLiteral("type")].toString();
    }
    
    // Try to determine type from IID or other metadata
    QPluginLoader loader(filePath);
    QJsonObject metaData = loader.metaData();
    QString iid = metaData[QStringLiteral("IID")].toString();
    
    if (iid.contains(QStringLiteral("IPlugin"))) {
        return QStringLiteral("IPlugin");
    }
    
    return QStringLiteral("unknown");
}

QStringList PluginDiscovery::checkPluginDependencies(const DiscoveredPlugin& plugin, const QList<DiscoveredPlugin>& availablePlugins)
{
    QStringList missingDependencies;
    
    for (const QString& dependency : plugin.metadata.dependencies) {
        if (!checkSingleDependency(dependency, availablePlugins)) {
            missingDependencies << dependency;
        }
    }
    
    return missingDependencies;
}

bool PluginDiscovery::resolveDependencies(QList<DiscoveredPlugin>& plugins)
{
    bool allResolved = true;
    
    for (DiscoveredPlugin& plugin : plugins) {
        plugin.missingDependencies = checkPluginDependencies(plugin, plugins);
        if (!plugin.missingDependencies.isEmpty()) {
            allResolved = false;
            plugin.isCompatible = false;
            addError(QStringLiteral("Plugin %1 has missing dependencies: %2")
                    .arg(plugin.metadata.name, plugin.missingDependencies.join(", ")));
        }
    }
    
    return allResolved;
}

QStringList PluginDiscovery::getPluginFileExtensions() const
{
    return m_defaultConfig.fileExtensions;
}

QStringList PluginDiscovery::findPluginFiles(const QString& directory, bool recursive)
{
    QStringList files;
    QDir dir(directory);
    
    if (!dir.exists()) {
        return files;
    }
    
    QStringList nameFilters = getPluginFileExtensions();
    QDir::Filters filters = QDir::Files | QDir::Readable;
    
    QFileInfoList fileInfos = dir.entryInfoList(nameFilters, filters);
    for (const QFileInfo& fileInfo : fileInfos) {
        files << fileInfo.absoluteFilePath();
    }
    
    if (recursive) {
        QFileInfoList dirInfos = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo& dirInfo : dirInfos) {
            files.append(findPluginFiles(dirInfo.absoluteFilePath(), true));
        }
    }
    
    return files;
}

QString PluginDiscovery::calculateFileChecksum(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (hash.addData(&file)) {
        return hash.result().toHex();
    }
    
    return QString();
}

bool PluginDiscovery::isPluginFile(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    QString suffix = QStringLiteral("*.") + fileInfo.suffix();
    
    return getPluginFileExtensions().contains(suffix, Qt::CaseInsensitive);
}

void PluginDiscovery::setDefaultConfig(const DiscoveryConfig& config)
{
    QMutexLocker locker(&m_mutex);
    m_defaultConfig = config;
}

DiscoveryConfig PluginDiscovery::getDefaultConfig() const
{
    QMutexLocker locker(&m_mutex);
    return m_defaultConfig;
}

DiscoveryStatistics PluginDiscovery::getLastStatistics() const
{
    QMutexLocker locker(&m_mutex);
    return m_statistics;
}

void PluginDiscovery::clearStatistics()
{
    m_statistics = DiscoveryStatistics();
}

QString PluginDiscovery::getLastError() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastError;
}

QStringList PluginDiscovery::getAllErrors() const
{
    QMutexLocker locker(&m_mutex);
    return m_errors;
}

void PluginDiscovery::clearErrors()
{
    m_errors.clear();
    m_lastError.clear();
}

void PluginDiscovery::cancelDiscovery()
{
    QMutexLocker locker(&m_mutex);
    m_cancelRequested = true;
}

void PluginDiscovery::onPluginValidationTimeout()
{
    addError("Plugin validation timeout occurred");
}

// Private implementation methods

void PluginDiscovery::scanDirectoryRecursive(const QString& directory, const DiscoveryConfig& config, QList<DiscoveredPlugin>& results, int currentDepth)
{
    if (m_cancelRequested) return;
    
    if (config.maxDepth >= 0 && currentDepth > config.maxDepth) {
        return;
    }
    
    if (shouldExcludePath(directory, config)) {
        return;
    }
    
    QDir dir(directory);
    if (!dir.exists()) {
        addError(QStringLiteral("Directory does not exist: %1").arg(directory));
        return;
    }
    
    // Scan for plugin files
    QStringList nameFilters = config.fileExtensions;
    QFileInfoList fileInfos = dir.entryInfoList(nameFilters, QDir::Files | QDir::Readable);
    
    int totalFiles = fileInfos.size();
    int processedFiles = 0;
    
    for (const QFileInfo& fileInfo : fileInfos) {
        if (m_cancelRequested) break;
        
        QString filePath = fileInfo.absoluteFilePath();
        emit pluginFound(filePath);
        
        DiscoveredPlugin plugin = createDiscoveredPlugin(filePath, config);
        results.append(plugin);
        updateStatistics(plugin);
        
        processedFiles++;
        if (totalFiles > 0) {
            int progress = (processedFiles * 80) / totalFiles; // Reserve 20% for dependency resolution
            reportProgress(progress, QStringLiteral("Processing: %1").arg(fileInfo.fileName()));
        }
    }
    
    // Recurse into subdirectories if enabled
    if (config.recursive) {
        QFileInfoList dirInfos = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo& dirInfo : dirInfos) {
            if (m_cancelRequested) break;
            scanDirectoryRecursive(dirInfo.absoluteFilePath(), config, results, currentDepth + 1);
        }
    }
}

DiscoveredPlugin PluginDiscovery::createDiscoveredPlugin(const QString& filePath, const DiscoveryConfig& config)
{
    DiscoveredPlugin plugin;
    plugin.filePath = filePath;
    plugin.fileName = QFileInfo(filePath).fileName();
    
    // Get file information
    QFileInfo fileInfo(filePath);
    plugin.fileSize = fileInfo.size();
    plugin.lastModified = fileInfo.lastModified();
    
    // Calculate checksum if requested
    if (config.calculateChecksums) {
        plugin.checksum = calculateFileChecksum(filePath);
    }
    
    // Validate plugin if requested
    if (config.validatePlugins) {
        plugin.isValid = validatePluginFile(filePath);
    } else {
        plugin.isValid = true; // Assume valid if not validating
    }
    
    // Load metadata if requested and plugin is valid
    if (config.loadMetadata && plugin.isValid) {
        plugin.metadata = loadPluginMetadata(filePath);
        plugin.hasMetadata = !plugin.metadata.name.isEmpty();
        plugin.isCompatible = plugin.metadata.isCompatible;
    }
    
    // Detect plugin type
    plugin.typeName = detectPluginType(filePath);
    
    return plugin;
}

bool PluginDiscovery::validatePluginWithLoader(const QString& filePath, QString& errorMessage)
{
    QPluginLoader loader(filePath);
    
    // Try to load the plugin
    if (!loader.load()) {
        errorMessage = loader.errorString();
        return false;
    }
    
    // Get the plugin instance
    QObject* instance = loader.instance();
    if (!instance) {
        errorMessage = "Failed to get plugin instance";
        loader.unload();
        return false;
    }
    
    // Check if it implements IPlugin interface
    IPlugin* plugin = qobject_cast<IPlugin*>(instance);
    if (!plugin) {
        errorMessage = "Plugin does not implement IPlugin interface";
        loader.unload();
        return false;
    }
    
    // Unload the plugin (we were just testing)
    loader.unload();
    
    return true;
}

PluginMetadata PluginDiscovery::parsePluginMetadata(const QJsonObject& json)
{
    PluginMetadata metadata;
    
    metadata.name = PluginDiscoveryUtils::extractPluginName(json);
    metadata.description = json[QStringLiteral("description")].toString();
    metadata.version = PluginDiscoveryUtils::extractPluginVersion(json);
    metadata.author = json[QStringLiteral("author")].toString();
    metadata.website = json[QStringLiteral("website")].toString();
    metadata.license = json[QStringLiteral("license")].toString();
    metadata.dependencies = PluginDiscoveryUtils::extractPluginDependencies(json);
    metadata.apiVersion = json[QStringLiteral("apiVersion")].toInt(1);
    metadata.isCompatible = json[QStringLiteral("compatible")].toBool(true);
    
    return metadata;
}

QString PluginDiscovery::generatePluginSignature(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    QString signature = QStringLiteral("%1_%2_%3")
                       .arg(fileInfo.fileName())
                       .arg(fileInfo.size())
                       .arg(fileInfo.lastModified().toString(Qt::ISODate));
    
    return QCryptographicHash::hash(signature.toUtf8(), QCryptographicHash::Md5).toHex();
}

bool PluginDiscovery::checkSingleDependency(const QString& dependency, const QList<DiscoveredPlugin>& availablePlugins)
{
    for (const DiscoveredPlugin& plugin : availablePlugins) {
        if (plugin.metadata.name == dependency && plugin.isValid) {
            return true;
        }
    }
    return false;
}

QList<DiscoveredPlugin> PluginDiscovery::sortPluginsByDependencies(const QList<DiscoveredPlugin>& plugins)
{
    // Simple topological sort based on dependencies
    QList<DiscoveredPlugin> sorted;
    QList<DiscoveredPlugin> remaining = plugins;
    
    while (!remaining.isEmpty()) {
        bool foundIndependent = false;
        
        for (int i = 0; i < remaining.size(); ++i) {
            const DiscoveredPlugin& plugin = remaining[i];
            
            // Check if all dependencies are already in sorted list
            bool dependenciesSatisfied = true;
            for (const QString& dependency : plugin.metadata.dependencies) {
                bool found = false;
                for (const DiscoveredPlugin& sortedPlugin : sorted) {
                    if (sortedPlugin.metadata.name == dependency) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    dependenciesSatisfied = false;
                    break;
                }
            }
            
            if (dependenciesSatisfied) {
                sorted.append(plugin);
                remaining.removeAt(i);
                foundIndependent = true;
                break;
            }
        }
        
        if (!foundIndependent) {
            // Circular dependency or missing dependency - add remaining plugins anyway
            sorted.append(remaining);
            break;
        }
    }
    
    return sorted;
}

void PluginDiscovery::updateStatistics(const DiscoveredPlugin& plugin)
{
    m_statistics.totalFilesScanned++;
    m_statistics.pluginsFound++;
    m_statistics.totalSizeBytes += plugin.fileSize;
    
    if (plugin.isValid) {
        m_statistics.validPlugins++;
    } else {
        m_statistics.invalidPlugins++;
    }
    
    if (plugin.isCompatible) {
        m_statistics.compatiblePlugins++;
    } else {
        m_statistics.incompatiblePlugins++;
    }
}

void PluginDiscovery::reportProgress(int percentage, const QString& status)
{
    emit progressChanged(percentage, status);
}

void PluginDiscovery::addError(const QString& error)
{
    m_errors.append(error);
    m_lastError = error;
    m_statistics.errorMessages.append(error);
    emit errorOccurred(error);
}

bool PluginDiscovery::shouldExcludePath(const QString& path, const DiscoveryConfig& config)
{
    for (const QString& excludePath : config.excludePaths) {
        if (path.contains(excludePath, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

// PluginDiscoveryUtils namespace implementation

namespace PluginDiscoveryUtils {

QStringList getSystemPluginPaths()
{
    QStringList paths;
    
    // Qt's standard plugin paths
    paths.append(QCoreApplication::libraryPaths());
    
    // Application-specific plugin path
    QString appPath = getApplicationPluginPath();
    if (!appPath.isEmpty() && !paths.contains(appPath)) {
        paths.append(appPath);
    }
    
    return paths;
}

QString getApplicationPluginPath()
{
    QString appDir = QCoreApplication::applicationDirPath();
    return QDir(appDir).absoluteFilePath("plugins");
}

bool createPluginDirectory(const QString& path)
{
    QDir dir;
    return dir.mkpath(path);
}

bool isValidPluginExtension(const QString& fileName)
{
    QFileInfo fileInfo(fileName);
    QString suffix = fileInfo.suffix().toLower();
    
#ifdef Q_OS_WINDOWS
    return suffix == "dll";
#elif defined(Q_OS_MACOS)
    return suffix == "dylib" || suffix == "so";
#else
    return suffix == "so";
#endif
}

QString normalizePluginPath(const QString& path)
{
    return QDir::cleanPath(QDir(path).absolutePath());
}

QStringList expandPluginPath(const QString& path)
{
    QStringList expanded;
    QDir dir(path);
    
    if (dir.exists()) {
        expanded.append(dir.absolutePath());
        
        // Add common subdirectories
        QStringList subdirs = {"plugins", "lib", "bin"};
        for (const QString& subdir : subdirs) {
            QString subdirPath = dir.absoluteFilePath(subdir);
            if (QDir(subdirPath).exists()) {
                expanded.append(subdirPath);
            }
        }
    }
    
    return expanded;
}

QString extractPluginName(const QJsonObject& metadata)
{
    if (metadata.contains("name")) {
        return metadata["name"].toString();
    }
    if (metadata.contains("Name")) {
        return metadata["Name"].toString();
    }
    return QString();
}

QString extractPluginVersion(const QJsonObject& metadata)
{
    if (metadata.contains("version")) {
        return metadata["version"].toString();
    }
    if (metadata.contains("Version")) {
        return metadata["Version"].toString();
    }
    return "1.0.0";
}

QStringList extractPluginDependencies(const QJsonObject& metadata)
{
    QStringList dependencies;
    
    if (metadata.contains("dependencies")) {
        QJsonArray deps = metadata["dependencies"].toArray();
        for (const QJsonValue& dep : deps) {
            dependencies.append(dep.toString());
        }
    }
    
    return dependencies;
}

bool isApiVersionCompatible(quint32 pluginVersion, quint32 appVersion)
{
    // Simple compatibility check - plugin version should be <= app version
    return pluginVersion <= appVersion;
}

bool areInterfacesCompatible(const QStringList& required, const QStringList& provided)
{
    for (const QString& requiredInterface : required) {
        if (!provided.contains(requiredInterface)) {
            return false;
        }
    }
    return true;
}

QString formatDiscoveryError(const QString& operation, const QString& path, const QString& error)
{
    return QStringLiteral("%1 failed for '%2': %3").arg(operation, path, error);
}

QString formatValidationError(const QString& plugin, const QString& error)
{
    return QStringLiteral("Plugin validation failed for '%1': %2").arg(plugin, error);
}

} // namespace PluginDiscoveryUtils

} // namespace PluginInterface