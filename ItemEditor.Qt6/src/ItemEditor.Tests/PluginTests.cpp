#include <QtTest>
#include <QSignalSpy>
#include "../ItemEditor.Plugins/IPlugin.h"
#include "../ItemEditor.Plugins/BasePlugin.h"

// Test plugin implementation
class TestPlugin : public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.itemeditor.IPlugin/1.0")
    Q_INTERFACES(IPlugin)

public:
    explicit TestPlugin(QObject* parent = nullptr) 
        : IPlugin(parent), m_initialized(false), m_clientLoaded(false) {}

    bool initialize() override { 
        m_initialized = true; 
        return true; 
    }
    
    QString name() const override { return "TestPlugin"; }
    QString version() const override { return "2.1.0"; }
    QStringList supportedVersions() const override { return {"8.00", "8.10", "8.20"}; }
    
    bool loadClient(const QString& datPath, const QString& sprPath) override {
        if (datPath.isEmpty() || sprPath.isEmpty()) {
            return false;
        }
        m_clientLoaded = true;
        m_loadedDatPath = datPath;
        m_loadedSprPath = sprPath;
        return true;
    }
    
    QByteArray getClientData(quint16 clientId) override {
        if (!m_clientLoaded || clientId == 0) {
            return QByteArray();
        }
        return QByteArray("client_data_") + QByteArray::number(clientId);
    }
    
    QByteArray getSpriteHash(quint16 clientId) override {
        if (!m_clientLoaded || clientId == 0) {
            return QByteArray();
        }
        return QByteArray("hash_") + QByteArray::number(clientId);
    }
    
    QByteArray getSpriteSignature(quint16 clientId) override {
        if (!m_clientLoaded || clientId == 0) {
            return QByteArray();
        }
        return QByteArray("signature_") + QByteArray::number(clientId);
    }
    
    bool isClientLoaded() const override { return m_clientLoaded; }
    QString getClientVersion() const override { return m_clientLoaded ? "8.00" : QString(); }
    void cleanup() override { 
        m_initialized = false; 
        m_clientLoaded = false;
        m_loadedDatPath.clear();
        m_loadedSprPath.clear();
    }

    // Test helpers
    bool isInitialized() const { return m_initialized; }
    QString getLoadedDatPath() const { return m_loadedDatPath; }
    QString getLoadedSprPath() const { return m_loadedSprPath; }

private:
    bool m_initialized;
    bool m_clientLoaded;
    QString m_loadedDatPath;
    QString m_loadedSprPath;
};

class PluginTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // IPlugin interface tests
    void testPluginInitialization();
    void testPluginMetadata();
    void testClientLoading();
    void testClientDataRetrieval();
    void testSpriteOperations();
    void testPluginCleanup();
    void testPluginSignals();
    
    // BasePlugin tests
    void testBasePluginDefaults();
    void testBasePluginInterface();

private:
    TestPlugin* m_testPlugin;
    BasePlugin* m_basePlugin;
};

void PluginTests::initTestCase()
{
    // Setup test environment
}

void PluginTests::cleanupTestCase()
{
    // Cleanup test environment
}

void PluginTests::init()
{
    m_testPlugin = new TestPlugin(this);
    m_basePlugin = new BasePlugin(this);
}

void PluginTests::cleanup()
{
    delete m_testPlugin;
    delete m_basePlugin;
    m_testPlugin = nullptr;
    m_basePlugin = nullptr;
}

void PluginTests::testPluginInitialization()
{
    // Test initial state
    QVERIFY(!m_testPlugin->isInitialized());
    QVERIFY(!m_testPlugin->isClientLoaded());
    
    // Test initialization
    bool result = m_testPlugin->initialize();
    QVERIFY(result);
    QVERIFY(m_testPlugin->isInitialized());
    
    // Test cleanup
    m_testPlugin->cleanup();
    QVERIFY(!m_testPlugin->isInitialized());
    QVERIFY(!m_testPlugin->isClientLoaded());
}

void PluginTests::testPluginMetadata()
{
    // Test plugin metadata
    QCOMPARE(m_testPlugin->name(), QString("TestPlugin"));
    QCOMPARE(m_testPlugin->version(), QString("2.1.0"));
    
    QStringList supportedVersions = m_testPlugin->supportedVersions();
    QCOMPARE(supportedVersions.size(), 3);
    QVERIFY(supportedVersions.contains("8.00"));
    QVERIFY(supportedVersions.contains("8.10"));
    QVERIFY(supportedVersions.contains("8.20"));
}

void PluginTests::testClientLoading()
{
    // Initialize plugin first
    QVERIFY(m_testPlugin->initialize());
    
    // Test loading with valid paths
    bool result = m_testPlugin->loadClient("/path/to/test.dat", "/path/to/test.spr");
    QVERIFY(result);
    QVERIFY(m_testPlugin->isClientLoaded());
    QCOMPARE(m_testPlugin->getLoadedDatPath(), QString("/path/to/test.dat"));
    QCOMPARE(m_testPlugin->getLoadedSprPath(), QString("/path/to/test.spr"));
    QCOMPARE(m_testPlugin->getClientVersion(), QString("8.00"));
    
    // Test loading with invalid paths
    m_testPlugin->cleanup();
    m_testPlugin->initialize();
    result = m_testPlugin->loadClient("", "/path/to/test.spr");
    QVERIFY(!result);
    QVERIFY(!m_testPlugin->isClientLoaded());
    
    result = m_testPlugin->loadClient("/path/to/test.dat", "");
    QVERIFY(!result);
    QVERIFY(!m_testPlugin->isClientLoaded());
}

void PluginTests::testClientDataRetrieval()
{
    // Initialize and load client
    QVERIFY(m_testPlugin->initialize());
    QVERIFY(m_testPlugin->loadClient("/path/to/test.dat", "/path/to/test.spr"));
    
    // Test valid client data retrieval
    QByteArray clientData = m_testPlugin->getClientData(100);
    QCOMPARE(clientData, QByteArray("client_data_100"));
    
    clientData = m_testPlugin->getClientData(500);
    QCOMPARE(clientData, QByteArray("client_data_500"));
    
    // Test invalid client ID
    clientData = m_testPlugin->getClientData(0);
    QVERIFY(clientData.isEmpty());
    
    // Test without loaded client
    m_testPlugin->cleanup();
    m_testPlugin->initialize();
    clientData = m_testPlugin->getClientData(100);
    QVERIFY(clientData.isEmpty());
}

void PluginTests::testSpriteOperations()
{
    // Initialize and load client
    QVERIFY(m_testPlugin->initialize());
    QVERIFY(m_testPlugin->loadClient("/path/to/test.dat", "/path/to/test.spr"));
    
    // Test sprite hash retrieval
    QByteArray spriteHash = m_testPlugin->getSpriteHash(100);
    QCOMPARE(spriteHash, QByteArray("hash_100"));
    
    spriteHash = m_testPlugin->getSpriteHash(200);
    QCOMPARE(spriteHash, QByteArray("hash_200"));
    
    // Test sprite signature retrieval
    QByteArray spriteSignature = m_testPlugin->getSpriteSignature(100);
    QCOMPARE(spriteSignature, QByteArray("signature_100"));
    
    spriteSignature = m_testPlugin->getSpriteSignature(300);
    QCOMPARE(spriteSignature, QByteArray("signature_300"));
    
    // Test with invalid client ID
    spriteHash = m_testPlugin->getSpriteHash(0);
    QVERIFY(spriteHash.isEmpty());
    
    spriteSignature = m_testPlugin->getSpriteSignature(0);
    QVERIFY(spriteSignature.isEmpty());
    
    // Test without loaded client
    m_testPlugin->cleanup();
    m_testPlugin->initialize();
    spriteHash = m_testPlugin->getSpriteHash(100);
    QVERIFY(spriteHash.isEmpty());
    
    spriteSignature = m_testPlugin->getSpriteSignature(100);
    QVERIFY(spriteSignature.isEmpty());
}

void PluginTests::testPluginCleanup()
{
    // Initialize and load client
    QVERIFY(m_testPlugin->initialize());
    QVERIFY(m_testPlugin->loadClient("/path/to/test.dat", "/path/to/test.spr"));
    QVERIFY(m_testPlugin->isInitialized());
    QVERIFY(m_testPlugin->isClientLoaded());
    
    // Test cleanup
    m_testPlugin->cleanup();
    QVERIFY(!m_testPlugin->isInitialized());
    QVERIFY(!m_testPlugin->isClientLoaded());
    QVERIFY(m_testPlugin->getLoadedDatPath().isEmpty());
    QVERIFY(m_testPlugin->getLoadedSprPath().isEmpty());
    QVERIFY(m_testPlugin->getClientVersion().isEmpty());
}

void PluginTests::testPluginSignals()
{
    QSignalSpy loadingProgressSpy(m_testPlugin, &IPlugin::loadingProgress);
    QSignalSpy errorSpy(m_testPlugin, &IPlugin::errorOccurred);
    
    // Test signal emission
    emit m_testPlugin->loadingProgress(50, "Loading sprites...");
    QCOMPARE(loadingProgressSpy.count(), 1);
    
    QList<QVariant> progressArgs = loadingProgressSpy.at(0);
    QCOMPARE(progressArgs.at(0).toInt(), 50);
    QCOMPARE(progressArgs.at(1).toString(), QString("Loading sprites..."));
    
    emit m_testPlugin->errorOccurred("Test error message");
    QCOMPARE(errorSpy.count(), 1);
    
    QList<QVariant> errorArgs = errorSpy.at(0);
    QCOMPARE(errorArgs.at(0).toString(), QString("Test error message"));
}

void PluginTests::testBasePluginDefaults()
{
    // Test BasePlugin default values
    QVERIFY(m_basePlugin->name().isEmpty());
    QVERIFY(m_basePlugin->version().isEmpty());
    QVERIFY(m_basePlugin->supportedVersions().isEmpty());
    QVERIFY(!m_basePlugin->isClientLoaded());
    QVERIFY(m_basePlugin->getClientVersion().isEmpty());
    
    // Test BasePlugin initialization
    bool result = m_basePlugin->initialize();
    QVERIFY(result); // BasePlugin::initialize() returns true by default
}

void PluginTests::testBasePluginInterface()
{
    // Test BasePlugin interface methods return expected defaults
    QVERIFY(!m_basePlugin->loadClient("/test.dat", "/test.spr"));
    QVERIFY(m_basePlugin->getClientData(100).isEmpty());
    QVERIFY(m_basePlugin->getSpriteHash(100).isEmpty());
    QVERIFY(m_basePlugin->getSpriteSignature(100).isEmpty());
    
    // Test cleanup doesn't crash
    m_basePlugin->cleanup();
    QVERIFY(!m_basePlugin->isClientLoaded());
}

QTEST_MAIN(PluginTests)
#include "PluginTests.moc"