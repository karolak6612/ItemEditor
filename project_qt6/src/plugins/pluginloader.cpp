#include "plugins/pluginloader.h"
#include "plugins/iplugin.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QMetaObject>
#include <QMetaMethod>
#include <QMutexLocker>
#include <QThread>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>

namespace PluginInterface {

PluginLoader::PluginLoader(QObject* parent)
    : QObject(parent)
    , m_pluginHost(nullptr)
    , m_cancelRequested(false)
    , m_loadTimer(new QTimer(this))
    , m_initTimer(new QTimer(this))
{
    // Setup timers
    m_loadTimer->setSingleShot(true);
    m_initTimer->setSingleShot(true);
    
    connect(m_loadTimer, &QTimer::timeout, this, &PluginLoader::onLoadTimeout);
    connect(m_initTimer, &QTimer::timeout, this, &PluginLoader::onInitTimeout);
}

PluginLoader::~PluginLoader()
{
    unloadAllPlugins();
}

LoadResult PluginLoader::loadPlugin(const QString& filePath, const LoadConfig& config)
{
    QMutexLocker locker(&m_mutex);
    
    recordLoadAttempt(filePath);
    emit pluginLoadStarted(filePath);
    reportProgress(0, QStringLiteral("Starting to load plugin: %1").arg(QFileInfo(filePath).fileName()));
    
    LoadResult result = loadPluginInternal(filePath, config);
    
    updateStatistics(result);
    
    if (result.success) {
        recordLoadSuccess(result);
        emit pluginLoadFinished(filePath, true);
        reportProgress(100, QStringLiteral("Successfully loaded plugin: %1").arg(result.metadata.name));
    } else {
        recordLoadFailure(filePath, result.errorMessage);
        emit pluginLoadFinished(filePath, false);
        reportProgress(100, QStringLiteral("Failed to load plugin: %1").arg(QFileInfo(filePath).fileName()));
    }
    
    return result;
}

LoadResult PluginLoader::loadPlugin(const DiscoveredPlugin& discoveredPlugin, const LoadConfig& config)
{
    LoadResult result = loadPlugin(discoveredPlugin.filePath, config);
    
    // Enhance result with discovery information
    if (result.success && discoveredPlugin.hasMetadata) {
        result.metadata = discoveredPlugin.metadata;
    }
    
    return result;
}

QList<LoadResult> PluginLoader::loadPlugins(const QStringList& filePaths, const LoadConfig& config)
{
    QList<LoadResult> results;
    
    for (int i = 0; i < filePaths.size(); ++i) {
        if (m_cancelRequested) break;
        
        const QString& filePath = filePaths[i];
        reportProgress((i * 100) / filePaths.size(), 
                      QStringLiteral("Loading plugin %1 of %2").arg(i + 1).arg(filePaths.size()));
        
        LoadResult result = loadPlugin(filePath, config);
        results.append(std::move(result));
    }
    
    return results;
}

QList<LoadResult> PluginLoader::loadPlugins(const QList<DiscoveredPlugin>& discoveredPlugins, const LoadConfig& config)
{
    QList<LoadResult> results;
    
    for (int i = 0; i < discoveredPlugins.size(); ++i) {
        if (m_cancelRequested) break;
        
        const DiscoveredPlugin& discovered = discoveredPlugins[i];
        reportProgress((i * 100) / discoveredPlugins.size(),
                      QStringLiteral("Loading plugin %1 of %2: %3")
                      .arg(i + 1).arg(discoveredPlugins.size()).arg(discovered.metadata.name));
        
        LoadResult result = loadPlugin(discovered, config);
        results.append(std::move(result));
    }
    
    return results;
}

LoadResult PluginLoader::loadStaticPlugin(const QString& pluginName, const LoadConfig& config)
{
    QMutexLocker locker(&m_mutex);
    
    if (!config.allowStaticPlugins) {
        LoadResult result;
        result.errorMessage = "Static plugins are disabled in configuration";
        handleLoadError(result, result.errorMessage);
        return result;
    }
    
    return loadStaticPluginInternal(pluginName, config);
}

QList<LoadResult> PluginLoader::loadAllStaticPlugins(const LoadConfig& config)
{
    QList<LoadResult> results;
    
    if (!config.allowStaticPlugins) {
        return results;
    }
    
    const QObjectList staticPlugins = QPluginLoader::staticInstances();
    
    for (int i = 0; i < staticPlugins.size(); ++i) {
        if (m_cancelRequested) break;
        
        QObject* pluginObject = staticPlugins[i];
        QString pluginName = pluginObject->metaObject()->className();
        
        reportProgress((i * 100) / staticPlugins.size(),
                      QStringLiteral("Loading static plugin %1 of %2: %3")
                      .arg(i + 1).arg(staticPlugins.size()).arg(pluginName));
        
        LoadResult result = loadStaticPlugin(pluginName, config);
        results.append(std::move(result));
    }
    
    return results;
}

bool PluginLoader::unloadPlugin(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);
    
    auto it = m_loadedPlugins.find(filePath);
    if (it == m_loadedPlugins.end()) {
        addError(QStringLiteral("Plugin not loaded: %1").arg(filePath));
        return false;
    }
    
    LoadResult& result = it.value();
    
    // Dispose plugin if it was initialized
    if (result.plugin) {
        try {
            result.plugin->Dispose();
        } catch (const std::exception& e) {
            addWarning(QStringLiteral("Exception during plugin disposal: %1").arg(e.what()));
        }
        
        // Remove from plugin-to-path mapping
        m_pluginToPath.remove(result.plugin);
    }
    
    // Unload the plugin library
    if (result.loader) {
        bool unloaded = result.loader->unload();
        if (!unloaded) {
            addWarning(QStringLiteral("Failed to unload plugin library: %1").arg(result.loader->errorString()));
        }
        
        // Remove from loaders list
        m_pluginLoaders.removeAll(result.loader);
        delete result.loader;
    }
    
    // Remove from loaded plugins
    m_loadedPlugins.erase(it);
    
    emit pluginUnloaded(filePath);
    return true;
}

bool PluginLoader::unloadPlugin(IPlugin* plugin)
{
    if (!plugin) return false;
    
    auto it = m_pluginToPath.find(plugin);
    if (it == m_pluginToPath.end()) {
        addError("Plugin instance not found in loaded plugins");
        return false;
    }
    
    return unloadPlugin(it.value());
}

void PluginLoader::unloadAllPlugins()
{
    QMutexLocker locker(&m_mutex);
    
    // Create a copy of the keys to avoid iterator invalidation
    QStringList pluginPaths = m_loadedPlugins.keys();
    
    for (const QString& filePath : pluginPaths) {
        unloadPlugin(filePath);
    }
    
    // Clean up any remaining loaders
    for (QPluginLoader* loader : m_pluginLoaders) {
        loader->unload();
        delete loader;
    }
    m_pluginLoaders.clear();
    
    // Clear all mappings
    m_loadedPlugins.clear();
    m_pluginToPath.clear();
    m_staticPlugins.clear();
}

bool PluginLoader::isPluginLoaded(const QString& filePath) const
{
    QMutexLocker locker(&m_mutex);
    return m_loadedPlugins.contains(filePath);
}

IPlugin* PluginLoader::getLoadedPlugin(const QString& filePath) const
{
    QMutexLocker locker(&m_mutex);
    
    auto it = m_loadedPlugins.find(filePath);
    if (it != m_loadedPlugins.end()) {
        return it.value().plugin;
    }
    
    return nullptr;
}

QList<IPlugin*> PluginLoader::getLoadedPlugins() const
{
    QMutexLocker locker(&m_mutex);
    
    QList<IPlugin*> plugins;
    for (const LoadResult& result : m_loadedPlugins) {
        if (result.plugin) {
            plugins.append(result.plugin);
        }
    }
    
    return plugins;
}

QStringList PluginLoader::getLoadedPluginPaths() const
{
    QMutexLocker locker(&m_mutex);
    return m_loadedPlugins.keys();
}

bool PluginLoader::initializePlugin(IPlugin* plugin, IPluginHost* host)
{
    if (!plugin) {
        addError("Cannot initialize null plugin");
        return false;
    }
    
    IPluginHost* pluginHost = host ? host : m_pluginHost;
    if (pluginHost) {
        plugin->setHost(pluginHost);
    }
    
    setupInitTimeout(m_defaultConfig.initTimeoutMs);
    
    bool success = false;
    try {
        success = plugin->Initialize();
        
        if (success) {
            emit pluginInitialized(plugin->pluginName(), true);
            m_statistics.pluginsInitialized++;
        } else {
            emit pluginInitialized(plugin->pluginName(), false);
            m_statistics.initializationFailures++;
            addError(QStringLiteral("Plugin initialization failed: %1").arg(plugin->pluginName()));
        }
    } catch (const std::exception& e) {
        success = false;
        emit pluginInitialized(plugin->pluginName(), false);
        m_statistics.initializationFailures++;
        addError(QStringLiteral("Exception during plugin initialization: %1").arg(e.what()));
    }
    
    cancelTimeouts();
    return success;
}

bool PluginLoader::initializePlugins(const QList<IPlugin*>& plugins, IPluginHost* host)
{
    bool allSuccess = true;
    
    for (int i = 0; i < plugins.size(); ++i) {
        if (m_cancelRequested) break;
        
        IPlugin* plugin = plugins[i];
        reportProgress((i * 100) / plugins.size(),
                      QStringLiteral("Initializing plugin %1 of %2: %3")
                      .arg(i + 1).arg(plugins.size()).arg(plugin->pluginName()));
        
        bool success = initializePlugin(plugin, host);
        if (!success) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

void PluginLoader::disposePlugin(IPlugin* plugin)
{
    if (!plugin) return;
    
    try {
        plugin->Dispose();
    } catch (const std::exception& e) {
        addWarning(QStringLiteral("Exception during plugin disposal: %1").arg(e.what()));
    }
}

void PluginLoader::disposeAllPlugins()
{
    QList<IPlugin*> plugins = getLoadedPlugins();
    
    for (IPlugin* plugin : plugins) {
        disposePlugin(plugin);
    }
}

bool PluginLoader::validatePluginFile(const QString& filePath)
{
    // Check cache first
    if (m_validationCache.contains(filePath)) {
        return m_validationCache[filePath];
    }
    
    if (!PluginLoaderUtils::isValidPluginFile(filePath)) {
        m_validationCache[filePath] = false;
        return false;
    }
    
    QPluginLoader loader(filePath);
    bool isValid = loader.load();
    
    if (isValid) {
        loader.unload();
    }
    
    m_validationCache[filePath] = isValid;
    return isValid;
}

bool PluginLoader::validatePluginInterface(QObject* pluginObject, const QStringList& requiredInterfaces)
{
    if (!pluginObject || requiredInterfaces.isEmpty()) {
        return true; // No validation needed
    }
    
    return validateRequiredInterfaces(pluginObject, requiredInterfaces);
}

bool PluginLoader::verifyPluginDependencies(IPlugin* plugin, const QList<IPlugin*>& availablePlugins)
{
    if (!plugin) return false;
    
    // This would need to be implemented based on how dependencies are defined
    // For now, assume no dependencies or all dependencies are satisfied
    Q_UNUSED(availablePlugins)
    return true;
}

QObject* PluginLoader::resolvePluginSymbol(const QString& filePath)
{
    QPluginLoader loader(filePath);
    return extractPluginInstance(&loader);
}

IPlugin* PluginLoader::castToIPlugin(QObject* pluginObject)
{
    if (!pluginObject) return nullptr;
    
    return qobject_cast<IPlugin*>(pluginObject);
}

QStringList PluginLoader::getPluginInterfaces(QObject* pluginObject)
{
    return extractPluginInterfaces(pluginObject);
}

void PluginLoader::setDefaultConfig(const LoadConfig& config)
{
    QMutexLocker locker(&m_mutex);
    m_defaultConfig = config;
}

LoadConfig PluginLoader::getDefaultConfig() const
{
    QMutexLocker locker(&m_mutex);
    return m_defaultConfig;
}

void PluginLoader::setPluginHost(IPluginHost* host)
{
    QMutexLocker locker(&m_mutex);
    m_pluginHost = host;
}

IPluginHost* PluginLoader::getPluginHost() const
{
    QMutexLocker locker(&m_mutex);
    return m_pluginHost;
}

LoadStatistics PluginLoader::getStatistics() const
{
    QMutexLocker locker(&m_mutex);
    return m_statistics;
}

void PluginLoader::clearStatistics()
{
    QMutexLocker locker(&m_mutex);
    m_statistics = LoadStatistics();
}

QString PluginLoader::getLastError() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastError;
}

QStringList PluginLoader::getAllErrors() const
{
    QMutexLocker locker(&m_mutex);
    return m_errors;
}

void PluginLoader::clearErrors()
{
    QMutexLocker locker(&m_mutex);
    m_errors.clear();
    m_warnings.clear();
    m_lastError.clear();
}

void PluginLoader::cancelLoading()
{
    QMutexLocker locker(&m_mutex);
    m_cancelRequested = true;
    cancelTimeouts();
}

void PluginLoader::onLoadTimeout()
{
    addError(QStringLiteral("Plugin loading timeout for: %1").arg(m_currentLoadingPlugin));
    m_cancelRequested = true;
}

void PluginLoader::onInitTimeout()
{
    addError(QStringLiteral("Plugin initialization timeout for: %1").arg(m_currentLoadingPlugin));
}

// Private implementation methods

LoadResult PluginLoader::loadPluginInternal(const QString& filePath, const LoadConfig& config)
{
    LoadResult result;
    result.filePath = filePath;
    m_currentLoadingPlugin = QFileInfo(filePath).fileName();
    
    QElapsedTimer timer;
    timer.start();
    
    try {
        // Check if already loaded
        if (isPluginLoaded(filePath)) {
            result.errorMessage = "Plugin already loaded";
            handleLoadError(result, result.errorMessage);
            return result;
        }
        
        // Validate file
        reportProgress(10, "Validating plugin file...");
        if (!validatePluginFile(filePath)) {
            result.errorMessage = "Plugin file validation failed";
            handleLoadError(result, result.errorMessage);
            return result;
        }
        
        // Create plugin loader
        reportProgress(20, "Creating plugin loader...");
        QPluginLoader* loader = new QPluginLoader(filePath, this);
        loader->setLoadHints(config.loadHints);
        result.loader = loader;
        
        // Perform plugin loading
        reportProgress(40, "Loading plugin library...");
        if (!performPluginLoad(loader, result, config)) {
            cleanupFailedLoad(loader);
            return result;
        }
        
        // Perform validation
        reportProgress(60, "Validating plugin interface...");
        if (!performPluginValidation(result.pluginObject, result, config)) {
            cleanupFailedLoad(loader);
            return result;
        }
        
        // Cast to IPlugin
        reportProgress(70, "Resolving plugin interface...");
        result.plugin = castToIPlugin(result.pluginObject);
        if (!result.plugin) {
            result.errorMessage = "Plugin does not implement IPlugin interface";
            handleLoadError(result, result.errorMessage);
            cleanupFailedLoad(loader);
            return result;
        }
        
        // Initialize if requested
        if (config.initializeAfterLoad) {
            reportProgress(80, "Initializing plugin...");
            if (!performPluginInitialization(result.plugin, result, config)) {
                cleanupFailedLoad(loader);
                return result;
            }
        }
        
        // Store successful result
        reportProgress(90, "Finalizing plugin loading...");
        m_loadedPlugins[filePath] = std::move(result);
        m_pluginToPath[result.plugin] = filePath;
        m_pluginLoaders.append(loader);
        
        result.success = true;
        result.loadTimeMs = timer.elapsed();
        
    } catch (const std::exception& e) {
        result.errorMessage = QStringLiteral("Exception during plugin loading: %1").arg(e.what());
        handleLoadError(result, result.errorMessage);
    }
    
    m_currentLoadingPlugin.clear();
    return result;
}

LoadResult PluginLoader::loadStaticPluginInternal(const QString& pluginName, const LoadConfig& config)
{
    LoadResult result;
    result.filePath = QStringLiteral("static://") + pluginName;
    
    // Check if already loaded
    if (m_staticPlugins.contains(pluginName)) {
        QObject* pluginObject = m_staticPlugins[pluginName];
        result.pluginObject = pluginObject;
        result.plugin = castToIPlugin(pluginObject);
        result.success = (result.plugin != nullptr);
        
        if (!result.success) {
            result.errorMessage = "Static plugin does not implement IPlugin interface";
        }
        
        return result;
    }
    
    // Find static plugin
    const QObjectList staticPlugins = QPluginLoader::staticInstances();
    QObject* foundPlugin = nullptr;
    
    for (QObject* plugin : staticPlugins) {
        if (plugin->metaObject()->className() == pluginName) {
            foundPlugin = plugin;
            break;
        }
    }
    
    if (!foundPlugin) {
        result.errorMessage = QStringLiteral("Static plugin not found: %1").arg(pluginName);
        handleLoadError(result, result.errorMessage);
        return result;
    }
    
    // Validate and cast
    result.pluginObject = foundPlugin;
    result.plugin = castToIPlugin(foundPlugin);
    
    if (!result.plugin) {
        result.errorMessage = "Static plugin does not implement IPlugin interface";
        handleLoadError(result, result.errorMessage);
        return result;
    }
    
    // Initialize if requested
    if (config.initializeAfterLoad) {
        if (!performPluginInitialization(result.plugin, result, config)) {
            return result;
        }
    }
    
    // Cache static plugin
    m_staticPlugins[pluginName] = foundPlugin;
    result.success = true;
    
    return result;
}

bool PluginLoader::performPluginLoad(QPluginLoader* loader, LoadResult& result, const LoadConfig& config)
{
    setupLoadTimeout(config.loadTimeoutMs);
    
    bool loaded = loader->load();
    
    cancelTimeouts();
    
    if (!loaded) {
        result.errorMessage = loader->errorString();
        handleLoadError(result, QStringLiteral("Failed to load plugin: %1").arg(result.errorMessage));
        return false;
    }
    
    result.pluginObject = extractPluginInstance(loader);
    if (!result.pluginObject) {
        result.errorMessage = "Failed to get plugin instance";
        handleLoadError(result, result.errorMessage);
        return false;
    }
    
    return true;
}

bool PluginLoader::performPluginValidation(QObject* pluginObject, LoadResult& result, const LoadConfig& config)
{
    if (!pluginObject) {
        result.errorMessage = "Plugin object is null";
        handleLoadError(result, result.errorMessage);
        return false;
    }
    
    if (config.validateInterface && !config.requiredInterfaces.isEmpty()) {
        if (!validateRequiredInterfaces(pluginObject, config.requiredInterfaces)) {
            result.errorMessage = "Plugin does not implement required interfaces";
            handleLoadError(result, result.errorMessage);
            return false;
        }
    }
    
    return true;
}

bool PluginLoader::performPluginInitialization(IPlugin* plugin, LoadResult& result, const LoadConfig& config)
{
    Q_UNUSED(config)
    
    if (!initializePlugin(plugin, m_pluginHost)) {
        result.errorMessage = QStringLiteral("Plugin initialization failed: %1").arg(plugin->getLastError());
        handleLoadError(result, result.errorMessage);
        return false;
    }
    
    return true;
}

QObject* PluginLoader::extractPluginInstance(QPluginLoader* loader)
{
    if (!loader) return nullptr;
    
    return loader->instance();
}

bool PluginLoader::validateRequiredInterfaces(QObject* pluginObject, const QStringList& required)
{
    if (!pluginObject || required.isEmpty()) {
        return true;
    }
    
    QStringList provided = extractPluginInterfaces(pluginObject);
    
    for (const QString& requiredInterface : required) {
        if (!provided.contains(requiredInterface)) {
            return false;
        }
    }
    
    return true;
}

QStringList PluginLoader::extractPluginInterfaces(QObject* pluginObject)
{
    QStringList interfaces;
    
    if (!pluginObject) return interfaces;
    
    // Check cache first
    QString cacheKey = QString::number(reinterpret_cast<quintptr>(pluginObject));
    if (m_interfaceCache.contains(cacheKey)) {
        return m_interfaceCache[cacheKey];
    }
    
    // Extract interfaces from meta object
    const QMetaObject* metaObject = pluginObject->metaObject();
    interfaces = PluginLoaderUtils::extractInterfaceNames(metaObject);
    
    // Cache the result
    m_interfaceCache[cacheKey] = interfaces;
    
    return interfaces;
}

bool PluginLoader::checkPluginDependencies(IPlugin* plugin, const LoadConfig& config)
{
    Q_UNUSED(plugin)
    Q_UNUSED(config)
    
    // Dependency checking would be implemented here
    // For now, assume all dependencies are satisfied
    return true;
}

QStringList PluginLoader::findMissingDependencies(IPlugin* plugin)
{
    Q_UNUSED(plugin)
    
    // This would return a list of missing dependencies
    // For now, return empty list
    return QStringList();
}

void PluginLoader::handleLoadError(LoadResult& result, const QString& error)
{
    result.success = false;
    result.errorMessage = error;
    addError(error);
}

void PluginLoader::handleLoadWarning(LoadResult& result, const QString& warning)
{
    result.warnings.append(warning);
    addWarning(warning);
}

void PluginLoader::cleanupFailedLoad(QPluginLoader* loader)
{
    if (loader) {
        loader->unload();
        m_pluginLoaders.removeAll(loader);
        delete loader;
    }
}

void PluginLoader::setupLoadTimeout(int timeoutMs)
{
    if (timeoutMs > 0) {
        m_loadTimer->start(timeoutMs);
    }
}

void PluginLoader::setupInitTimeout(int timeoutMs)
{
    if (timeoutMs > 0) {
        m_initTimer->start(timeoutMs);
    }
}

void PluginLoader::cancelTimeouts()
{
    m_loadTimer->stop();
    m_initTimer->stop();
}

void PluginLoader::updateStatistics(const LoadResult& result)
{
    m_statistics.totalLoadAttempts++;
    m_statistics.totalLoadTimeMs += result.loadTimeMs;
    
    if (result.success) {
        m_statistics.successfulLoads++;
        m_statistics.loadedPluginNames.append(result.metadata.name);
        
        if (m_statistics.successfulLoads > 0) {
            m_statistics.averageLoadTimeMs = m_statistics.totalLoadTimeMs / m_statistics.successfulLoads;
        }
    } else {
        m_statistics.failedLoads++;
        m_statistics.failedPluginPaths.append(result.filePath);
        m_statistics.loadErrors[result.filePath] = result.errorMessage;
    }
}

void PluginLoader::recordLoadAttempt(const QString& filePath)
{
    Q_UNUSED(filePath)
    // This could be used for more detailed tracking
}

void PluginLoader::recordLoadSuccess(const LoadResult& result)
{
    Q_UNUSED(result)
    // This could be used for more detailed tracking
}

void PluginLoader::recordLoadFailure(const QString& filePath, const QString& error)
{
    Q_UNUSED(filePath)
    Q_UNUSED(error)
    // This could be used for more detailed tracking
}

void PluginLoader::reportProgress(int percentage, const QString& status)
{
    emit loadingProgress(percentage, status);
}

void PluginLoader::addError(const QString& error)
{
    m_errors.append(error);
    m_lastError = error;
    emit errorOccurred(error);
}

void PluginLoader::addWarning(const QString& warning)
{
    m_warnings.append(warning);
    emit warningOccurred(warning);
}

QString PluginLoader::generateLoadId(const QString& filePath)
{
    return QFileInfo(filePath).baseName();
}

// PluginLoaderUtils namespace implementation

namespace PluginLoaderUtils {

bool isValidPluginFile(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    
    if (!fileInfo.exists() || !fileInfo.isFile() || !fileInfo.isReadable()) {
        return false;
    }
    
    QString suffix = fileInfo.suffix().toLower();
    
#ifdef Q_OS_WINDOWS
    return suffix == "dll";
#elif defined(Q_OS_MACOS)
    return suffix == "dylib" || suffix == "so";
#else
    return suffix == "so";
#endif
}

QString normalizePluginPath(const QString& filePath)
{
    return QDir::cleanPath(QFileInfo(filePath).absoluteFilePath());
}

QStringList getPluginSearchPaths()
{
    QStringList paths;
    
    // Application directory
    QString appDir = QCoreApplication::applicationDirPath();
    paths.append(QDir(appDir).absoluteFilePath("plugins"));
    
    // Qt library paths
    paths.append(QCoreApplication::libraryPaths());
    
    return paths;
}

QString findPluginFile(const QString& pluginName)
{
    QStringList searchPaths = getPluginSearchPaths();
    QStringList extensions;
    
#ifdef Q_OS_WINDOWS
    extensions << ".dll";
#elif defined(Q_OS_MACOS)
    extensions << ".dylib" << ".so";
#else
    extensions << ".so";
#endif
    
    for (const QString& path : searchPaths) {
        for (const QString& ext : extensions) {
            QString fullPath = QDir(path).absoluteFilePath(pluginName + ext);
            if (QFile::exists(fullPath)) {
                return fullPath;
            }
        }
    }
    
    return QString();
}

QLibrary::LoadHints getOptimalLoadHints()
{
    return QLibrary::PreventUnloadHint | QLibrary::ExportExternalSymbolsHint;
}

QStringList getSystemRequiredInterfaces()
{
    return QStringList() << "PluginInterface::IPlugin";
}

bool isPluginCompatible(const PluginMetadata& metadata)
{
    // Check API version compatibility
    return metadata.apiVersion <= 1; // Current API version
}

QString formatLoadError(const QString& operation, const QString& plugin, const QString& error)
{
    return QStringLiteral("%1 failed for plugin '%2': %3").arg(operation, plugin, error);
}

QString formatInitError(const QString& plugin, const QString& error)
{
    return QStringLiteral("Initialization failed for plugin '%1': %2").arg(plugin, error);
}

QString formatTimeoutError(const QString& operation, const QString& plugin, int timeoutMs)
{
    return QStringLiteral("%1 timeout (%2ms) for plugin '%3'").arg(operation).arg(timeoutMs).arg(plugin);
}

bool validatePluginMetadata(const PluginMetadata& metadata)
{
    return !metadata.name.isEmpty() && !metadata.version.isEmpty();
}

bool validatePluginVersion(const QString& version)
{
    // Simple version validation - should contain at least one dot
    return version.contains('.');
}

QStringList extractInterfaceNames(const QMetaObject* metaObject)
{
    QStringList interfaces;
    
    if (!metaObject) return interfaces;
    
    // Check class info for interface declarations
    for (int i = 0; i < metaObject->classInfoCount(); ++i) {
        QMetaClassInfo classInfo = metaObject->classInfo(i);
        if (QString(classInfo.name()) == "interfaces") {
            QString interfaceList = classInfo.value();
            interfaces = interfaceList.split(',', Qt::SkipEmptyParts);
            break;
        }
    }
    
    // If no explicit interface info, use class name
    if (interfaces.isEmpty()) {
        interfaces.append(metaObject->className());
    }
    
    return interfaces;
}

QStringList resolveDependencyOrder(const QList<IPlugin*>& plugins)
{
    Q_UNUSED(plugins)
    
    // This would implement topological sorting of plugins based on dependencies
    // For now, return plugins in the order they were provided
    QStringList order;
    for (IPlugin* plugin : plugins) {
        order.append(plugin->pluginName());
    }
    
    return order;
}

bool hasCyclicDependencies(const QList<IPlugin*>& plugins)
{
    Q_UNUSED(plugins)
    
    // This would check for cyclic dependencies
    // For now, assume no cycles
    return false;
}

QStringList findDependencyChain(IPlugin* plugin, const QList<IPlugin*>& availablePlugins)
{
    Q_UNUSED(plugin)
    Q_UNUSED(availablePlugins)
    
    // This would find the dependency chain for a plugin
    // For now, return empty chain
    return QStringList();
}

} // namespace PluginLoaderUtils

} // namespace PluginInterface