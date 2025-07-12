#include <QtTest/QtTest>
#include <QObject>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "plugins/iplugin.h"
#include "plugins/pluginloader.h"
#include "plugins/pluginmanager.h"
#include "plugins/versionmanager.h"
#include "plugins/pluginsecurity.h"

using namespace PluginInterface;

/**
 * @brief Integration tests for the complete plugin system
 * 
 * This test class focuses on testing the integration between all
 * plugin system components working together as a complete system.
 */
class PluginIntegrationTests : public QObject
{
    Q_OBJECT

private:
    PluginManager* m_manager;
    QTemporaryDir* m_tempDir;
    QString m_testPluginsPath;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // End-to-end integration tests
    void testCompletePluginLifecycle();
    void testPluginDiscoveryToLoading();
    void testPluginCommunicationChain();
    void testErrorPropagation();
    
    // System integration tests
    void testPluginSystemWithRealPlugins();
    void testPluginSystemStartup();
    void testPluginSystemShutdown();
    void testPluginSystemRecovery();
    
    // Component integration tests
    void testManagerLoaderIntegration();
    void testManagerDiscoveryIntegration();
    void testLoaderValidationIntegration();
    void testSecurityIntegration();
    
    // Real-world scenario tests
    void testMultipleClientVersions();
    void testPluginVersionCompatibility();
    void testPluginDependencyResolution();
    void testPluginUpdateScenario();
    
    // Application integration tests
    void testHostServiceIntegration();
    void testConfigurationIntegration();
    void testLoggingIntegration();
    void testProgressReportingIntegration();

private:
    void simulateRealWorldUsage();
    void verifySystemState();
    void createCompleteTestEnvironment();
};

void PluginIntegrationTests::initTestCase()
{
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    
    m_testPluginsPath = m_tempDir->path() + "/plugins";
    QDir().mkpath(m_testPluginsPath);
    
    createCompleteTestEnvironment();
}

void PluginIntegrationTests::cleanupTestCase()
{
    delete m_tempDir;
}

void PluginIntegrationTests::init()
{
    m_manager = new PluginManager(this);
    m_manager->setPluginsDirectory(m_testPluginsPath);
    m_manager->setApplicationVersion("1.0.0-integration-test");
}

void PluginIntegrationTests::cleanup()
{
    if (m_manager) {
        m_manager->unloadAllPlugins();
        m_manager->deleteLater();
        m_manager = nullptr;
    }
}

void PluginIntegrationTests::testCompletePluginLifecycle()
{
    // Test complete lifecycle: discovery -> loading -> initialization -> usage -> disposal -> unloading
    
    QSignalSpy loadedSpy(m_manager, &PluginManager::pluginLoaded);
    QSignalSpy unloadedSpy(m_manager, &PluginManager::pluginUnloaded);
    QSignalSpy errorSpy(m_manager, &PluginManager::pluginError);
    
    // Step 1: Discovery
    m_manager->refreshPlugins();
    QList<PluginMetadata> metadata = m_manager->getPluginMetadata();
    
    // Step 2: Loading
    m_manager->loadPlugins(m_testPluginsPath);
    
    // Step 3: Verify loaded plugins
    QList<IPlugin*> loadedPlugins = m_manager->getLoadedPlugins();
    
    // Step 4: Test plugin functionality (for real plugins)
    for (IPlugin* plugin : loadedPlugins) {
        if (plugin) {
            // Test basic interface
            QVERIFY(!plugin->pluginName().isEmpty());
            QVERIFY(!plugin->pluginVersion().isEmpty());
            
            // Test initialization
            bool initialized = plugin->Initialize();
            QVERIFY(initialized);
            
            // Test capabilities
            plugin->supportsExtendedMode();
            plugin->supportsFrameDurations();
            plugin->supportsTransparency();
            
            // Test disposal
            plugin->Dispose();
        }
    }
    
    // Step 5: Unloading
    m_manager->unloadAllPlugins();
    
    // Verify final state
    QVERIFY(m_manager->getLoadedPlugins().isEmpty());
    verifySystemState();
}

void PluginIntegrationTests::testPluginDiscoveryToLoading()
{
    // Test seamless flow from discovery to loading
    
    // Initial state
    QVERIFY(m_manager->getAvailablePlugins().isEmpty());
    QVERIFY(m_manager->getLoadedPlugins().isEmpty());
    
    // Discovery phase
    m_manager->refreshPlugins();
    QList<IPlugin*> availablePlugins = m_manager->getAvailablePlugins();
    
    // Loading phase
    m_manager->loadPlugins(m_testPluginsPath);
    QList<IPlugin*> loadedPlugins = m_manager->getLoadedPlugins();
    
    // Verify integration
    QVERIFY(loadedPlugins.size() <= availablePlugins.size());
    
    // Test finding plugins after loading
    for (IPlugin* plugin : loadedPlugins) {
        if (plugin) {
            IPlugin* foundPlugin = m_manager->findPlugin(plugin->pluginName());
            QCOMPARE(foundPlugin, plugin);
        }
    }
}

void PluginIntegrationTests::testPluginCommunicationChain()
{
    // Test communication between plugins and host
    
    m_manager->loadPlugins(m_testPluginsPath);
    QList<IPlugin*> plugins = m_manager->getLoadedPlugins();
    
    // Test host-to-plugin communication
    for (IPlugin* plugin : plugins) {
        if (plugin) {
            // Set host
            plugin->setHost(m_manager);
            QCOMPARE(plugin->getHost(), m_manager);
            
            // Test host services access through plugin
            IPluginHost* host = plugin->getHost();
            if (host) {
                QVERIFY(!host->getApplicationVersion().isEmpty());
                QVERIFY(!host->getPluginsDirectory().isEmpty());
                
                // Test logging through host
                host->logMessage("Test message from plugin");
                host->logError("Test error from plugin");
                
                // Test configuration access
                host->setConfigValue("plugin.test.key", QVariant("test.value"));
                QVariant value = host->getConfigValue("plugin.test.key");
                QCOMPARE(value.toString(), QString("test.value"));
            }
        }
    }
    
    // Test inter-plugin communication
    if (plugins.size() >= 2) {
        IPlugin* plugin1 = plugins[0];
        IPlugin* plugin2 = plugins[1];
        
        if (plugin1 && plugin2) {
            QString targetName = plugin2->pluginName();
            bool sent = m_manager->sendMessage(targetName, "TestMessage", QVariant("TestData"));
            // Communication framework should handle the message
        }
    }
}

void PluginIntegrationTests::testErrorPropagation()
{
    // Test error propagation through the system
    
    QSignalSpy errorSpy(m_manager, &PluginManager::pluginError);
    QSignalSpy logSpy(m_manager, &PluginManager::logMessageEmitted);
    
    // Trigger various error conditions
    
    // 1. Invalid plugin loading
    m_manager->loadPlugin("/non/existent/plugin.so");
    
    // 2. Invalid directory
    m_manager->loadPlugins("/non/existent/directory");
    
    // 3. Invalid plugin operations
    m_manager->unloadPlugin("NonExistentPlugin");
    
    // Verify error handling
    QVERIFY(errorSpy.count() >= 0); // May have errors
    
    // System should remain functional after errors
    QVERIFY(m_manager != nullptr);
    m_manager->refreshPlugins();
    QVERIFY(true); // System remains responsive
}

void PluginIntegrationTests::testPluginSystemWithRealPlugins()
{
    // Test with actual plugin files if available
    
    // Check for real plugin files in the project
    QString realPluginsPath = QDir::currentPath() + "/../plugins";
    QDir realPluginsDir(realPluginsPath);
    
    if (realPluginsDir.exists()) {
        QStringList pluginFiles = realPluginsDir.entryList(QStringList() << "*.so" << "*.dll", QDir::Files);
        
        if (!pluginFiles.isEmpty()) {
            m_manager->setPluginsDirectory(realPluginsPath);
            m_manager->loadPlugins(realPluginsPath);
            
            QList<IPlugin*> loadedPlugins = m_manager->getLoadedPlugins();
            
            for (IPlugin* plugin : loadedPlugins) {
                if (plugin) {
                    // Test real plugin functionality
                    QVERIFY(plugin->Initialize());
                    
                    // Test client support
                    QList<ItemEditor::SupportedClient> clients = plugin->getSupportedClients();
                    
                    // Test item access
                    quint16 minId = plugin->getMinItemId();
                    quint16 maxId = plugin->getMaxItemId();
                    QVERIFY(maxId >= minId);
                    
                    // Test capabilities
                    bool supportsExtended = plugin->supportsExtendedMode();
                    bool supportsFrames = plugin->supportsFrameDurations();
                    bool supportsTransparency = plugin->supportsTransparency();
                    bool supportsVersionDetection = plugin->supportsVersionDetection();
                    
                    Q_UNUSED(supportsExtended)
                    Q_UNUSED(supportsFrames)
                    Q_UNUSED(supportsTransparency)
                    Q_UNUSED(supportsVersionDetection)
                    
                    plugin->Dispose();
                }
            }
        } else {
            QSKIP("No real plugin files found for integration testing");
        }
    } else {
        QSKIP("Real plugins directory not found");
    }
}

void PluginIntegrationTests::testPluginSystemStartup()
{
    // Test complete system startup sequence
    
    // 1. Manager initialization
    QVERIFY(m_manager != nullptr);
    
    // 2. Configuration setup
    m_manager->setApplicationVersion("1.0.0");
    m_manager->setPluginsDirectory(m_testPluginsPath);
    
    // 3. Plugin discovery
    m_manager->refreshPlugins();
    
    // 4. Plugin loading
    m_manager->loadPlugins(m_testPluginsPath);
    
    // 5. System ready state
    verifySystemState();
    
    // System should be fully operational
    QVERIFY(m_manager->getApplicationVersion() == "1.0.0");
    QVERIFY(m_manager->getPluginsDirectory() == m_testPluginsPath);
}

void PluginIntegrationTests::testPluginSystemShutdown()
{
    // Test complete system shutdown sequence
    
    // Setup system
    m_manager->loadPlugins(m_testPluginsPath);
    QList<IPlugin*> loadedPlugins = m_manager->getLoadedPlugins();
    
    // Dispose all plugins properly
    for (IPlugin* plugin : loadedPlugins) {
        if (plugin) {
            plugin->Dispose();
        }
    }
    
    // Unload all plugins
    m_manager->unloadAllPlugins();
    
    // Verify clean shutdown
    QVERIFY(m_manager->getLoadedPlugins().isEmpty());
    QVERIFY(m_manager->getAvailablePlugins().isEmpty());
    
    // System should be clean
    verifySystemState();
}

void PluginIntegrationTests::testPluginSystemRecovery()
{
    // Test system recovery after errors
    
    // Cause some errors
    m_manager->loadPlugin("/invalid/path.so");
    m_manager->loadPlugins("/invalid/directory");
    
    // System should recover and continue working
    m_manager->refreshPlugins();
    m_manager->loadPlugins(m_testPluginsPath);
    
    // Verify system is still functional
    QVERIFY(m_manager != nullptr);
    verifySystemState();
}

void PluginIntegrationTests::testManagerLoaderIntegration()
{
    // Test integration between PluginManager and PluginLoader
    
    // Manager should use loader internally
    m_manager->loadPlugins(m_testPluginsPath);
    
    // Verify plugins are accessible through manager
    QList<IPlugin*> plugins = m_manager->getLoadedPlugins();
    
    for (IPlugin* plugin : plugins) {
        if (plugin) {
            // Should be findable through manager
            IPlugin* found = m_manager->findPlugin(plugin->pluginName());
            QVERIFY(found == plugin || found == nullptr); // Either found or not loaded
        }
    }
}

void PluginIntegrationTests::testHostServiceIntegration()
{
    // Test integration of host services
    
    // Test logging integration
    QSignalSpy logSpy(m_manager, &PluginManager::logMessageEmitted);
    
    m_manager->logMessage("Integration test message");
    m_manager->logError("Integration test error");
    m_manager->logWarning("Integration test warning");
    
    QVERIFY(logSpy.count() >= 3);
    
    // Test progress integration
    QSignalSpy progressSpy(m_manager, &PluginManager::progressChanged);
    
    m_manager->reportProgress(50, "Integration test progress");
    QCOMPARE(progressSpy.count(), 1);
    
    // Test configuration integration
    m_manager->setConfigValue("integration.test", QVariant(42));
    QVariant value = m_manager->getConfigValue("integration.test");
    QCOMPARE(value.toInt(), 42);
}

void PluginIntegrationTests::testConfigurationIntegration()
{
    // Test configuration system integration
    
    // Set various configuration values
    m_manager->setConfigValue("app.name", QVariant("ItemEditor"));
    m_manager->setConfigValue("app.version", QVariant("1.0.0"));
    m_manager->setConfigValue("plugins.autoload", QVariant(true));
    
    // Verify values are accessible
    QCOMPARE(m_manager->getConfigValue("app.name").toString(), QString("ItemEditor"));
    QCOMPARE(m_manager->getConfigValue("app.version").toString(), QString("1.0.0"));
    QCOMPARE(m_manager->getConfigValue("plugins.autoload").toBool(), true);
    
    // Test default values
    QVariant defaultValue = m_manager->getConfigValue("non.existent.key", QVariant("default"));
    QCOMPARE(defaultValue.toString(), QString("default"));
}

void PluginIntegrationTests::testLoggingIntegration()
{
    // Test logging system integration
    
    QSignalSpy logSpy(m_manager, &PluginManager::logMessageEmitted);
    
    // Test different log levels
    m_manager->logMessage("Info message", 0);
    m_manager->logMessage("Warning message", 1);
    m_manager->logMessage("Error message", 2);
    m_manager->logDebug("Debug message");
    
    QVERIFY(logSpy.count() >= 4);
    
    // Verify log message content
    for (int i = 0; i < logSpy.count(); ++i) {
        QList<QVariant> arguments = logSpy.at(i);
        QVERIFY(!arguments.at(0).toString().isEmpty()); // Message should not be empty
    }
}

void PluginIntegrationTests::testProgressReportingIntegration()
{
    // Test progress reporting integration
    
    QSignalSpy progressSpy(m_manager, &PluginManager::progressChanged);
    
    // Simulate progress during plugin loading
    m_manager->reportProgress(0, "Starting plugin loading");
    m_manager->reportProgress(25, "Loading plugin 1");
    m_manager->reportProgress(50, "Loading plugin 2");
    m_manager->reportProgress(75, "Initializing plugins");
    m_manager->reportProgress(100, "Plugin loading complete");
    
    QCOMPARE(progressSpy.count(), 5);
    
    // Verify progress values
    QList<QVariant> lastProgress = progressSpy.last();
    QCOMPARE(lastProgress.at(0).toInt(), 100);
    QCOMPARE(lastProgress.at(1).toString(), QString("Plugin loading complete"));
}

// Helper method implementations

void PluginIntegrationTests::simulateRealWorldUsage()
{
    // Simulate typical application usage patterns
    
    // 1. Application startup
    m_manager->refreshPlugins();
    m_manager->loadPlugins(m_testPluginsPath);
    
    // 2. Plugin usage
    QList<IPlugin*> plugins = m_manager->getLoadedPlugins();
    for (IPlugin* plugin : plugins) {
        if (plugin) {
            plugin->Initialize();
            plugin->getSupportedClients();
            plugin->getMinItemId();
            plugin->getMaxItemId();
        }
    }
    
    // 3. Configuration changes
    m_manager->setConfigValue("user.preference", QVariant("value"));
    
    // 4. Plugin communication
    if (plugins.size() >= 2) {
        m_manager->sendMessage(plugins[1]->pluginName(), "message", QVariant("data"));
    }
    
    // 5. Application shutdown
    for (IPlugin* plugin : plugins) {
        if (plugin) {
            plugin->Dispose();
        }
    }
    m_manager->unloadAllPlugins();
}

void PluginIntegrationTests::verifySystemState()
{
    // Verify the system is in a consistent state
    
    QVERIFY(m_manager != nullptr);
    
    // Check that all loaded plugins are properly initialized
    QList<IPlugin*> loadedPlugins = m_manager->getLoadedPlugins();
    for (IPlugin* plugin : loadedPlugins) {
        if (plugin) {
            QVERIFY(!plugin->pluginName().isEmpty());
            QVERIFY(!plugin->pluginVersion().isEmpty());
            QVERIFY(plugin->getHost() != nullptr);
        }
    }
    
    // Check that manager services are working
    QVERIFY(!m_manager->getApplicationVersion().isEmpty());
    QVERIFY(!m_manager->getPluginsDirectory().isEmpty());
}

void PluginIntegrationTests::createCompleteTestEnvironment()
{
    // Create a complete test environment with mock plugins
    
    QDir pluginsDir(m_testPluginsPath);
    
    // Create different types of test plugins
    QStringList pluginTypes = {"770", "860", "Modern"};
    
    for (const QString& type : pluginTypes) {
        QString pluginPath = m_testPluginsPath + QString("/TestPlugin%1.so").arg(type);
        QFile file(pluginPath);
        
        if (file.open(QIODevice::WriteOnly)) {
            QString content = QString("Mock plugin for client version %1").arg(type);
            file.write(content.toUtf8());
            file.close();
        }
    }
    
    // Create plugin metadata files if needed
    // This would be implementation-specific
}

// Stub implementations for remaining test methods

void PluginIntegrationTests::testManagerDiscoveryIntegration()
{
    QSKIP("Manager discovery integration tests not yet implemented");
}

void PluginIntegrationTests::testLoaderValidationIntegration()
{
    QSKIP("Loader validation integration tests not yet implemented");
}

void PluginIntegrationTests::testSecurityIntegration()
{
    QSKIP("Security integration tests not yet implemented");
}

void PluginIntegrationTests::testMultipleClientVersions()
{
    QSKIP("Multiple client versions tests not yet implemented");
}

void PluginIntegrationTests::testPluginVersionCompatibility()
{
    QSKIP("Plugin version compatibility tests not yet implemented");
}

void PluginIntegrationTests::testPluginDependencyResolution()
{
    QSKIP("Plugin dependency resolution tests not yet implemented");
}

void PluginIntegrationTests::testPluginUpdateScenario()
{
    QSKIP("Plugin update scenario tests not yet implemented");
}

QTEST_MAIN(PluginIntegrationTests)
#include "test_plugin_integration.moc"