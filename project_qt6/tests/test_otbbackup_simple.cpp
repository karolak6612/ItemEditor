#include <QtTest>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include "otbbackup_standalone.h"

using namespace OTB;

class TestOtbBackupSimple : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testBackupConfiguration();
    void testBackupMetadata();
    void testCreateManualBackup();
    void testBackupUtilities();

private:
    void createTestFile(const QString& filePath, const QString& content = "test content");

    QTemporaryDir* m_tempDir;
    QString m_testFile;
    QString m_backupDir;
    OtbBackupSystem* m_backupSystem;
};

void TestOtbBackupSimple::initTestCase()
{
    qDebug() << "Starting Simple OTB Backup System Tests";
    
    // Create temporary directory for tests
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    
    m_backupDir = m_tempDir->path() + "/backups";
    m_testFile = m_tempDir->path() + "/test.txt";
    
    qDebug() << "Test directory:" << m_tempDir->path();
    qDebug() << "Backup directory:" << m_backupDir;
}

void TestOtbBackupSimple::cleanupTestCase()
{
    delete m_backupSystem;
    delete m_tempDir;
    qDebug() << "Simple OTB Backup System Tests completed";
}

void TestOtbBackupSimple::init()
{
    // Create fresh backup system for each test
    m_backupSystem = new OtbBackupSystem();
    
    // Configure backup system
    BackupConfig config;
    config.backupDirectory = m_backupDir;
    config.autoBackupEnabled = true;
    config.crashRecoveryEnabled = true;
    config.maxBackupCount = 5;
    config.maxBackupAgeDays = 30;
    config.verifyBackupIntegrity = false; // Disable OTB validation for simple tests
    config.validationLevel = ValidationLevel::None;
    
    m_backupSystem->setConfiguration(config);
    
    // Create test file
    createTestFile(m_testFile);
}

void TestOtbBackupSimple::cleanup()
{
    delete m_backupSystem;
    m_backupSystem = nullptr;
    
    // Clean up test files
    QFile::remove(m_testFile);
    
    // Clean up backup directory
    QDir backupDir(m_backupDir);
    if (backupDir.exists()) {
        backupDir.removeRecursively();
    }
}

void TestOtbBackupSimple::testBackupConfiguration()
{
    // Test default configuration
    BackupConfig defaultConfig;
    QVERIFY(defaultConfig.isValid());
    QVERIFY(defaultConfig.autoBackupEnabled);
    QVERIFY(defaultConfig.crashRecoveryEnabled);
    QCOMPARE(defaultConfig.maxBackupCount, 10);
    QCOMPARE(defaultConfig.maxBackupAgeDays, 30);
    
    // Test configuration validation
    BackupConfig invalidConfig;
    invalidConfig.maxBackupCount = 0;
    QVERIFY(!invalidConfig.isValid());
    
    // Test configuration serialization
    QJsonObject json = defaultConfig.toJson();
    QVERIFY(!json.isEmpty());
    
    BackupConfig deserializedConfig;
    QVERIFY(deserializedConfig.fromJson(json));
    QCOMPARE(deserializedConfig.maxBackupCount, defaultConfig.maxBackupCount);
}

void TestOtbBackupSimple::testBackupMetadata()
{
    BackupMetadata metadata;
    metadata.backupId = "test-backup-id";
    metadata.originalFilePath = "/path/to/original.txt";
    metadata.backupFilePath = "/path/to/backup.txt";
    metadata.type = BackupType::Manual;
    metadata.createdAt = QDateTime::currentDateTime();
    metadata.originalFileSize = 1024;
    metadata.backupFileSize = 1024;
    metadata.checksum = "abc123";
    metadata.description = "Test backup";
    metadata.applicationVersion = "1.0.0";
    
    QVERIFY(metadata.isValid());
    
    // Test serialization
    QJsonObject json = metadata.toJson();
    QVERIFY(!json.isEmpty());
    
    BackupMetadata deserializedMetadata;
    QVERIFY(deserializedMetadata.fromJson(json));
    QCOMPARE(deserializedMetadata.backupId, metadata.backupId);
    QCOMPARE(deserializedMetadata.type, metadata.type);
    QCOMPARE(deserializedMetadata.originalFileSize, metadata.originalFileSize);
}

void TestOtbBackupSimple::testCreateManualBackup()
{
    QString description = "Test manual backup";
    
    BackupResult result = m_backupSystem->createBackup(m_testFile, BackupType::Manual, description);
    
    QVERIFY(result.success);
    QVERIFY(!result.backupId.isEmpty());
    QVERIFY(!result.backupPath.isEmpty());
    QVERIFY(QFile::exists(result.backupPath));
    QVERIFY(result.backupSize > 0);
    QVERIFY(result.processingTimeMs >= 0);
    
    // Verify backup metadata
    QList<BackupMetadata> backups = m_backupSystem->listBackups(m_testFile);
    QCOMPARE(backups.size(), 1);
    QCOMPARE(backups.first().description, description);
    QCOMPARE(backups.first().type, BackupType::Manual);
}

void TestOtbBackupSimple::testBackupUtilities()
{
    // Test checksum calculation
    QString checksum1 = BackupUtils::calculateSHA256(m_testFile);
    QVERIFY(!checksum1.isEmpty());
    
    // Create identical file
    QString identicalFile = m_tempDir->path() + "/identical.txt";
    QVERIFY(QFile::copy(m_testFile, identicalFile));
    
    QString checksum2 = BackupUtils::calculateSHA256(identicalFile);
    QCOMPARE(checksum1, checksum2);
    
    // Test file comparison
    QVERIFY(BackupUtils::compareFiles(m_testFile, identicalFile));
    
    // Modify file and verify checksum changes
    createTestFile(identicalFile, "different content");
    QString checksum3 = BackupUtils::calculateSHA256(identicalFile);
    QVERIFY(checksum1 != checksum3);
    QVERIFY(!BackupUtils::compareFiles(m_testFile, identicalFile));
    
    // Test directory operations
    QString testDir = m_tempDir->path() + "/test_subdir";
    QVERIFY(BackupUtils::createDirectoryRecursive(testDir));
    QVERIFY(QDir(testDir).exists());
    
    qint64 dirSize = BackupUtils::getDirectorySize(m_tempDir->path());
    QVERIFY(dirSize > 0);
}

void TestOtbBackupSimple::createTestFile(const QString& filePath, const QString& content)
{
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QTextStream stream(&file);
    stream << content;
    file.close();
    QVERIFY(file.exists());
}

QTEST_MAIN(TestOtbBackupSimple)
#include "test_otbbackup_simple.moc"