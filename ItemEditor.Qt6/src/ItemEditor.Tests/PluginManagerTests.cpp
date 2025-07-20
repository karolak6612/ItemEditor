#include <QtTest>
#include <QTemporaryDir>
#include <QPluginLoader>
#include <QSignalSpy>
#include "../ItemEditor.Plugins/PluginManager.h"
#include "../ItemEditor.Plugins/IPlugin.h"

// Mock plugin for testing
class MockPlugin : public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.itemeditor.IPlugin/1.0")
    Q_INTERFACES(IPlugin)

public:
    explicit MockPlugin(QObject* parent = nullptr) 
        : IPlugin(parent), m_initialized(false), m_clientLoaded(false) {}

    bool initialize() override { 
        m_initialized = true; 
        return true; 
    }
    
    QString name() const override { return "MockPlugin"; }
    QString version() const override { return "1.0.0"; }
    QStringList supportedVersions() const override { return {"8.00", "8.10"}; }
    
    bool loadClient(const QString& datPath, const QString& sprPath) override {
        Q_UNUSED(datPath)
        Q_UNUSED(sprPath)
        m_clientLoaded = true;
        return true;
    }
    
    QByteArray getClientData(quint16 clientId) override {
        Q_UNUSED(clientId)
        return QByteArray("mock_data");
    }
    
    QByteArray getSpriteHash(quint16 clientId) override {
        Q_UNUSED(clientId)
        return QByteArray("mock_hash");
    }
    
    QByteArray getSpriteSignature(quint16 clientId) override {
        Q_UNUSED(clientId)
        return QByteArray("mock_signature");
    }
    
    bool isClientLoaded() const override { return m_clientLoaded; }
    QString getClientVersion() const override { return "8.00"; }
    void cleanup() override { m_initialized = false; m_clientLoaded = false; }

private:
    bool m_initialized;
    bool m_clientLoaded;
};

// Invalid mock plugin for testing validation
class InvalidMockPlugin : public IPlugin
{
    Q_OBJECT

public:
    explicit InvalidMockPlugin(QObject* parent = nullptr) : IPlugin(parent) {}

    bool initialize() override { return false; }
    QString name() const override { return ""; } // Invalid empty name
    QString version() const override { return ""; } // Invalid empty version
    QStringList supportedVersions() const override { return QStringList(); } // No supported versions
    bool loadClient(const QString&, const QString&) override { return false; }
    QByteArray getClientData(quint16) override { return QByteArray(); }
    QByteArray getSpriteHash(quint16) override { return QByteArray(); }
    QByteArray getSpriteSignature(quint16) override { return QByteArray(); }
    bool isClientLoaded() const override { return false; }
    QString getClientVersion() const override { return QString(); }
    void cleanup() override {}
};

class PluginManagerTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testInitialization();
    void testInitializationWithInvalidDirectory();
    void testPluginDiscovery();
    void testPluginLoading();
    void testPluginValidation();
    void testVersionCompatibility();
    void testPluginRetrieval();
    void testPluginReloading();
    void testCleanup();
    
    // Signal tests
    void testSignalEmission();
    void testErrorHandling();
    
    // Enhanced plugin management tests
    void testPluginStatistics();
    void testValidateAllPlugins();
    void testPluginHealthStatus();
    void testPluginsForVersionRange();

private:
    PluginManager* m_pluginManager;
    QTemporaryDir* m_tempDir;
    QString m_pluginDir;
};

void PluginManagerTests::initTestCase()
{
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    m_pluginDir = m_tempDir->path() + "/plugins";
    QDir().mkpath(m_pluginDir);
}

void PluginManagerTests::cleanupTestCase()
{
    delete m_tempDir;
}

void PluginManagerTests::init()
{
    m_pluginManager = new PluginManager(this);
}

void PluginManagerTests::cleanup()
{
    delete m_pluginManager;
    m_pluginManager = nullptr;
}

void PluginManagerTests::testInitialization()
{
    // Test successful initialization
    bool result = m_pluginManager->initialize(m_pluginDir);
    QVERIFY(result);
    QCOMPARE(m_pluginManager->getPluginCount(), 0); // No plugins in empty directory
}

void PluginManagerTests::testInitializationWithInvalidDirectory()
{
    QSignalSpy errorSpy(m_pluginManager, &PluginManager::errorOccurred);
    
    // Test initialization with non-existent directory
    bool result = m_pluginManager->initialize("/non/existent/directory");
    QVERIFY(!result);
    QCOMPARE(errorSpy.count(), 1);
    
    QString errorMessage = errorSpy.at(0).at(0).toString();
    QVERIFY(errorMessage.contains("Plugin directory does not exist"));
}

void PluginManagerTests::testPluginDiscovery()
{
    // Initialize with empty directory
    m_pluginManager->initialize(m_pluginDir);
    QCOMPARE(m_pluginManager->getPluginCount(), 0);
    QVERIFY(!m_pluginManager->hasPlugins());
    
    // Test that available plugins list is empty
    QList<IPlugin*> plugins = m_pluginManager->getAvailablePlugins();
    QVERIFY(plugins.isEmpty());
}

void PluginManagerTests::testPluginLoading()
{
    // This test would require creating actual plugin files
    // For now, we test the loading logic with mock scenarios
    m_pluginManager->initialize(m_pluginDir);
    
    // Test getting non-existent plugin
    IPlugin* plugin = m_pluginManager->getPlugin("NonExistentPlugin");
    QVERIFY(plugin == nullptr);
    
    // Test getting plugin for unsupported version
    plugin = m_pluginManager->getPluginForVersion("99.99");
    QVERIFY(plugin == nullptr);
}

void PluginManagerTests::testPluginValidation()
{
    // Test validation with mock plugin
    MockPlugin mockPlugin;
    bool isValid = m_pluginManager->validatePlugin(&mockPlugin);
    QVERIFY(isValid);
    
    // Test validation with invalid plugin
    InvalidMockPlugin invalidPlugin;
    bool isInvalid = m_pluginManager->validatePlugin(&invalidPlugin);
    QVERIFY(!isInvalid);
    
    // Test validation with null plugin
    bool isNull = m_pluginManager->validatePlugin(nullptr);
    QVERIFY(!isNull);
}

void PluginManagerTests::testVersionCompatibility()
{
    // Test valid version formats
    QVERIFY(m_pluginManager->isVersionCompatible("1.0"));
    QVERIFY(m_pluginManager->isVersionCompatible("1.0.0"));
    QVERIFY(m_pluginManager->isVersionCompatible("10.25.3"));
    
    // Test invalid version formats
    QVERIFY(!m_pluginManager->isVersionCompatible(""));
    QVERIFY(!m_pluginManager->isVersionCompatible("1"));
    QVERIFY(!m_pluginManager->isVersionCompatible("1.0.0.0"));
    QVERIFY(!m_pluginManager->isVersionCompatible("1.a"));
    QVERIFY(!m_pluginManager->isVersionCompatible("a.b.c"));
}

void PluginManagerTests::testPluginRetrieval()
{
    m_pluginManager->initialize(m_pluginDir);
    
    // Test retrieval methods with empty plugin list
    QCOMPARE(m_pluginManager->getPluginCount(), 0);
    QVERIFY(!m_pluginManager->hasPlugins());
    QVERIFY(m_pluginManager->getAvailablePlugins().isEmpty());
    QVERIFY(m_pluginManager->getPlugin("TestPlugin") == nullptr);
    QVERIFY(m_pluginManager->getPluginForVersion("8.00") == nullptr);
}

void PluginManagerTests::testPluginReloading()
{
    // Initialize plugin manager
    m_pluginManager->initialize(m_pluginDir);
    int initialCount = m_pluginManager->getPluginCount();
    
    // Test reloading
    bool reloadResult = m_pluginManager->reloadPlugins();
    QVERIFY(reloadResult);
    QCOMPARE(m_pluginManager->getPluginCount(), initialCount);
}

void PluginManagerTests::testCleanup()
{
    // Initialize and then cleanup
    m_pluginManager->initialize(m_pluginDir);
    m_pluginManager->cleanup();
    
    // Verify cleanup
    QCOMPARE(m_pluginManager->getPluginCount(), 0);
    QVERIFY(!m_pluginManager->hasPlugins());
    QVERIFY(m_pluginManager->getAvailablePlugins().isEmpty());
}

void PluginManagerTests::testSignalEmission()
{
    QSignalSpy loadingProgressSpy(m_pluginManager, &PluginManager::loadingProgress);
    QSignalSpy pluginsLoadedSpy(m_pluginManager, &PluginManager::pluginsLoaded);
    
    // Initialize plugin manager
    m_pluginManager->initialize(m_pluginDir);
    
    // Verify signals were emitted
    QVERIFY(loadingProgressSpy.count() >= 1);
    QCOMPARE(pluginsLoadedSpy.count(), 1);
    
    // Check pluginsLoaded signal argument
    QList<QVariant> arguments = pluginsLoadedSpy.at(0);
    QCOMPARE(arguments.at(0).toInt(), 0); // No plugins loaded from empty directory
}

void PluginManagerTests::testErrorHandling()
{
    QSignalSpy errorSpy(m_pluginManager, &PluginManager::errorOccurred);
    
    // Test error handling with invalid directory
    m_pluginManager->initialize("/invalid/path");
    QCOMPARE(errorSpy.count(), 1);
    
    // Test error handling - we can't directly test private slot,
    // but we can test that error signals are properly connected during plugin loading
    // This is tested indirectly through the plugin loading process
}

void PluginManagerTests::testPluginStatistics()
{
    // Initialize with empty directory
    m_pluginManager->initialize(m_pluginDir);
    
    // Test statistics with no plugins
    QString stats = m_pluginManager->getPluginStatistics();
    QVERIFY(stats.contains("Total Plugins Loaded: 0"));
    QVERIFY(stats.contains("Plugin Manager Statistics"));
    QVERIFY(stats.contains("Loading Errors: 0"));
    
    // Test that statistics include plugin directory
    QVERIFY(stats.contains(m_pluginDir));
}

void PluginManagerTests::testValidateAllPlugins()
{
    // Initialize with empty directory
    m_pluginManager->initialize(m_pluginDir);
    
    // Test validation with no plugins (should return false)
    bool isValid = m_pluginManager->validateAllPlugins();
    QVERIFY(!isValid); // No plugins means validation fails
    
    // Note: Testing with actual plugins would require loading real plugin files
    // This test validates the basic logic with empty plugin list
}

void PluginManagerTests::testPluginHealthStatus()
{
    // Test with null plugin
    QString healthStatus = m_pluginManager->getPluginHealthStatus(nullptr);
    QVERIFY(healthStatus.contains("ERROR: Null plugin"));
    
    // Test with mock plugin
    MockPlugin mockPlugin;
    healthStatus = m_pluginManager->getPluginHealthStatus(&mockPlugin);
    QVERIFY(healthStatus.contains("Plugin Health Status: MockPlugin"));
    QVERIFY(healthStatus.contains("Plugin Name: MockPlugin"));
    QVERIFY(healthStatus.contains("Plugin Version: 1.0.0"));
    QVERIFY(healthStatus.contains("Supported Versions: 8.00, 8.10"));
    
    // Test with invalid plugin
    InvalidMockPlugin invalidPlugin;
    healthStatus = m_pluginManager->getPluginHealthStatus(&invalidPlugin);
    QVERIFY(healthStatus.contains("UNHEALTHY"));
    QVERIFY(healthStatus.contains("Empty plugin name"));
    QVERIFY(healthStatus.contains("Empty plugin version"));
}

void PluginManagerTests::testPluginsForVersionRange()
{
    // Initialize with empty directory
    m_pluginManager->initialize(m_pluginDir);
    
    // Test with empty plugin list
    QList<IPlugin*> plugins = m_pluginManager->getPluginsForVersionRange("8.00", "9.00");
    QVERIFY(plugins.isEmpty());
    
    // Test with invalid version format
    plugins = m_pluginManager->getPluginsForVersionRange("invalid", "9.00");
    QVERIFY(plugins.isEmpty());
    
    plugins = m_pluginManager->getPluginsForVersionRange("8.00", "invalid");
    QVERIFY(plugins.isEmpty());
    
    // Note: Testing with actual plugins would require loading real plugin files
    // This test validates the basic logic with empty plugin list and invalid inputs
}

QTEST_MAIN(PluginManagerTests)
#include "PluginManagerTests.moc"