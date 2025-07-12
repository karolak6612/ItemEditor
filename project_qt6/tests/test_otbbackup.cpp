#include <QtTest>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include "otbbackup.h"
#include "otbreader.h"
#include "otbwriter.h"

using namespace OTB;

class TestOtbBackup : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Configuration tests
    void testBackupConfiguration();
    void testConfigurationPersistence();

    // Basic backup operations
    void testCreateManualBackup();
    void testCreateAutomaticBackup();
    void testCreateCrashRecoveryBackup();

    // Recovery operations
    void testRestoreFromBackup();
    void testRestoreLatestBackup();
    void testCrashRecovery();

    // Backup management
    void testListBackups();
    void testDeleteBackup();
    void testCleanupOldBackups();

    // Validation and verification
    void testBackupValidation();
    void testBackupIntegrity();
    void testChecksumCalculation();

    // Integration tests
    void testWriterIntegration();
    void testMultipleBackups();
    void testBackupStatistics();

    // Error handling
    void testInvalidFileBackup();
    void testCorruptedBackupRecovery();
    void testInsufficientSpace();

private:
    void createTestOtbFile(const QString& filePath, int itemCount = 10);
    void verifyBackupMetadata(const BackupMetadata& metadata);
    void verifyOtbFileContent(const QString& filePath, int expectedItemCount = 10);

    QTemporaryDir* m_tempDir;
    QString m_testOtbFile;
    QString m_backupDir;
    OtbBackupSystem* m_backupSystem;
};

void TestOtbBackup::initTestCase()
{
    qDebug() << "Starting OTB Backup System Tests";
    
    // Create temporary directory for tests
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    
    m_backupDir = m_tempDir->path() + "/backups";
    m_testOtbFile = m_tempDir->path() + "/test.otb";
    
    qDebug() << "Test directory:" << m_tempDir->path();
    qDebug() << "Backup directory:" << m_backupDir;
}

void TestOtbBackup::cleanupTestCase()
{
    delete m_backupSystem;
    delete m_tempDir;
    qDebug() << "OTB Backup System Tests completed";
}

void TestOtbBackup::init()
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
    config.verifyBackupIntegrity = true;
    config.validationLevel = ValidationLevel::Structure;
    
    m_backupSystem->setConfiguration(config);
    
    // Create test OTB file
    createTestOtbFile(m_testOtbFile);
}

void TestOtbBackup::cleanup()
{
    delete m_backupSystem;
    m_backupSystem = nullptr;
    
    // Clean up test files
    QFile::remove(m_testOtbFile);
    
    // Clean up backup directory
    QDir backupDir(m_backupDir);
    if (backupDir.exists()) {
        backupDir.removeRecursively();
    }
}

void TestOtbBackup::testBackupConfiguration()
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

void TestOtbBackup::testConfigurationPersistence()
{
    QString configPath = m_tempDir->path() + "/backup_config.json";
    
    // Save configuration
    QVERIFY(m_backupSystem->saveConfiguration(configPath));
    QVERIFY(QFile::exists(configPath));
    
    // Create new backup system and load configuration
    OtbBackupSystem newSystem;
    QVERIFY(newSystem.loadConfiguration(configPath));
    
    // Verify configuration was loaded correctly
    const BackupConfig& loadedConfig = newSystem.getConfiguration();
    QCOMPARE(loadedConfig.backupDirectory, m_backupDir);
    QVERIFY(loadedConfig.autoBackupEnabled);
}

void TestOtbBackup::testCreateManualBackup()
{
    QString description = "Test manual backup";
    
    BackupResult result = m_backupSystem->createBackup(m_testOtbFile, BackupType::Manual, description);
    
    QVERIFY(result.success);
    QVERIFY(!result.backupId.isEmpty());
    QVERIFY(!result.backupPath.isEmpty());
    QVERIFY(QFile::exists(result.backupPath));
    QVERIFY(result.backupSize > 0);
    QVERIFY(result.processingTimeMs >= 0);
    
    // Verify backup metadata
    BackupMetadata metadata;
    QString metadataPath = m_backupDir + "/" + result.backupId + ".backup.json";
    QVERIFY(QFile::exists(metadataPath));
    
    QFile metadataFile(metadataPath);
    QVERIFY(metadataFile.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(metadataFile.readAll());
    QVERIFY(metadata.fromJson(doc.object()));
    
    verifyBackupMetadata(metadata);
    QCOMPARE(metadata.description, description);
    QCOMPARE(metadata.type, BackupType::Manual);
}

void TestOtbBackup::testCreateAutomaticBackup()
{
    BackupResult result = m_backupSystem->createAutomaticBackup(m_testOtbFile);
    
    QVERIFY(result.success);
    QVERIFY(!result.backupId.isEmpty());
    
    // Verify backup type
    QList<BackupMetadata> backups = m_backupSystem->listBackups(m_testOtbFile);
    QCOMPARE(backups.size(), 1);
    QCOMPARE(backups.first().type, BackupType::Automatic);
}

void TestOtbBackup::testCreateCrashRecoveryBackup()
{
    BackupResult result = m_backupSystem->createCrashRecoveryBackup(m_testOtbFile);
    
    QVERIFY(result.success);
    QVERIFY(!result.backupId.isEmpty());
    
    // Verify crash recovery data exists
    QVERIFY(m_backupSystem->hasCrashRecoveryData(m_testOtbFile));
    
    QStringList crashBackups = m_backupSystem->getCrashRecoveryBackups(m_testOtbFile);
    QCOMPARE(crashBackups.size(), 1);
    QCOMPARE(crashBackups.first(), result.backupId);
}

void TestOtbBackup::testRestoreFromBackup()
{
    // Create backup
    BackupResult backupResult = m_backupSystem->createBackup(m_testOtbFile, BackupType::Manual);
    QVERIFY(backupResult.success);
    
    // Modify original file
    createTestOtbFile(m_testOtbFile, 20); // Different content
    
    // Restore from backup
    QString restorePath = m_tempDir->path() + "/restored.otb";
    RecoveryResult recoveryResult = m_backupSystem->restoreFromBackup(backupResult.backupId, restorePath);
    
    QVERIFY(recoveryResult.success);
    QCOMPARE(recoveryResult.restoredFilePath, restorePath);
    QVERIFY(QFile::exists(restorePath));
    QVERIFY(recoveryResult.processingTimeMs >= 0);
    QCOMPARE(recoveryResult.validationPerformed, ValidationLevel::Structure);
    
    // Verify restored file content
    verifyOtbFileContent(restorePath, 10); // Original content
}

void TestOtbBackup::testRestoreLatestBackup()
{
    // Create multiple backups
    m_backupSystem->createBackup(m_testOtbFile, BackupType::Manual, "First backup");
    QThread::msleep(100); // Ensure different timestamps
    
    createTestOtbFile(m_testOtbFile, 15);
    BackupResult latestBackup = m_backupSystem->createBackup(m_testOtbFile, BackupType::Manual, "Latest backup");
    QVERIFY(latestBackup.success);
    
    // Modify file again
    createTestOtbFile(m_testOtbFile, 25);
    
    // Restore latest backup
    QString restorePath = m_tempDir->path() + "/restored_latest.otb";
    RecoveryResult result = m_backupSystem->restoreLatestBackup(m_testOtbFile, restorePath);
    
    QVERIFY(result.success);
    verifyOtbFileContent(restorePath, 15); // Content from latest backup
}

void TestOtbBackup::testCrashRecovery()
{
    // Create crash recovery backup
    BackupResult crashBackup = m_backupSystem->createCrashRecoveryBackup(m_testOtbFile);
    QVERIFY(crashBackup.success);
    
    // Simulate file corruption
    QFile corruptFile(m_testOtbFile);
    QVERIFY(corruptFile.open(QIODevice::WriteOnly));
    corruptFile.write("corrupted data");
    corruptFile.close();
    
    // Perform crash recovery
    RecoveryResult result = m_backupSystem->performCrashRecovery(m_testOtbFile);
    QVERIFY(result.success);
    
    // Verify file was restored
    verifyOtbFileContent(m_testOtbFile, 10);
    
    // Clear crash recovery data
    m_backupSystem->clearCrashRecoveryData(m_testOtbFile);
    QVERIFY(!m_backupSystem->hasCrashRecoveryData(m_testOtbFile));
}

void TestOtbBackup::testListBackups()
{
    // Create multiple backups
    m_backupSystem->createBackup(m_testOtbFile, BackupType::Manual, "Manual 1");
    m_backupSystem->createBackup(m_testOtbFile, BackupType::Automatic, "Auto 1");
    m_backupSystem->createBackup(m_testOtbFile, BackupType::CrashRecovery, "Crash 1");
    
    // Test listing all backups
    QList<BackupMetadata> allBackups = m_backupSystem->listBackups();
    QCOMPARE(allBackups.size(), 3);
    
    // Test listing backups for specific file
    QList<BackupMetadata> fileBackups = m_backupSystem->listBackups(m_testOtbFile);
    QCOMPARE(fileBackups.size(), 3);
    
    // Test listing by type
    QList<BackupMetadata> manualBackups = m_backupSystem->listBackupsByType(BackupType::Manual);
    QCOMPARE(manualBackups.size(), 1);
    QCOMPARE(manualBackups.first().type, BackupType::Manual);
    
    QList<BackupMetadata> autoBackups = m_backupSystem->listBackupsByType(BackupType::Automatic);
    QCOMPARE(autoBackups.size(), 1);
    
    QList<BackupMetadata> crashBackups = m_backupSystem->listBackupsByType(BackupType::CrashRecovery);
    QCOMPARE(crashBackups.size(), 1);
}

void TestOtbBackup::testDeleteBackup()
{
    // Create backup
    BackupResult result = m_backupSystem->createBackup(m_testOtbFile, BackupType::Manual);
    QVERIFY(result.success);
    
    // Verify backup exists
    QVERIFY(QFile::exists(result.backupPath));
    QList<BackupMetadata> backups = m_backupSystem->listBackups();
    QCOMPARE(backups.size(), 1);
    
    // Delete backup
    QVERIFY(m_backupSystem->deleteBackup(result.backupId));
    
    // Verify backup was deleted
    QVERIFY(!QFile::exists(result.backupPath));
    backups = m_backupSystem->listBackups();
    QCOMPARE(backups.size(), 0);
}

void TestOtbBackup::testCleanupOldBackups()
{
    // Create multiple backups with different ages
    BackupResult recent = m_backupSystem->createBackup(m_testOtbFile, BackupType::Manual, "Recent");
    QVERIFY(recent.success);
    
    // Simulate old backup by modifying metadata
    BackupMetadata oldMetadata;
    QString metadataPath = m_backupDir + "/" + recent.backupId + ".backup.json";
    QFile metadataFile(metadataPath);
    QVERIFY(metadataFile.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(metadataFile.readAll());
    metadataFile.close();
    
    QVERIFY(oldMetadata.fromJson(doc.object()));
    oldMetadata.createdAt = QDateTime::currentDateTime().addDays(-40); // Make it old
    
    QVERIFY(metadataFile.open(QIODevice::WriteOnly));
    doc.setObject(oldMetadata.toJson());
    metadataFile.write(doc.toJson());
    metadataFile.close();
    
    // Test cleanup
    QVERIFY(m_backupSystem->deleteOldBackups(30)); // Delete backups older than 30 days
    
    // Verify old backup was deleted
    QList<BackupMetadata> remainingBackups = m_backupSystem->listBackups();
    QCOMPARE(remainingBackups.size(), 0);
}

void TestOtbBackup::testBackupValidation()
{
    // Create backup
    BackupResult result = m_backupSystem->createBackup(m_testOtbFile, BackupType::Manual);
    QVERIFY(result.success);
    
    // Test different validation levels
    QVERIFY(m_backupSystem->validateBackup(result.backupId, ValidationLevel::None));
    QVERIFY(m_backupSystem->validateBackup(result.backupId, ValidationLevel::Basic));
    QVERIFY(m_backupSystem->validateBackup(result.backupId, ValidationLevel::Structure));
    QVERIFY(m_backupSystem->validateBackup(result.backupId, ValidationLevel::Complete));
}

void TestOtbBackup::testBackupIntegrity()
{
    // Create backup
    BackupResult result = m_backupSystem->createBackup(m_testOtbFile, BackupType::Manual);
    QVERIFY(result.success);
    
    // Test integrity verification
    QVERIFY(m_backupSystem->verifyBackupIntegrity(result.backupId));
    
    // Corrupt backup file
    QFile backupFile(result.backupPath);
    QVERIFY(backupFile.open(QIODevice::WriteOnly | QIODevice::Append));
    backupFile.write("corruption");
    backupFile.close();
    
    // Verify integrity check fails
    QVERIFY(!m_backupSystem->verifyBackupIntegrity(result.backupId));
}

void TestOtbBackup::testChecksumCalculation()
{
    QString checksum1 = m_backupSystem->calculateFileChecksum(m_testOtbFile);
    QVERIFY(!checksum1.isEmpty());
    
    // Create identical file
    QString identicalFile = m_tempDir->path() + "/identical.otb";
    QVERIFY(QFile::copy(m_testOtbFile, identicalFile));
    
    QString checksum2 = m_backupSystem->calculateFileChecksum(identicalFile);
    QCOMPARE(checksum1, checksum2);
    
    // Modify file and verify checksum changes
    QFile file(identicalFile);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Append));
    file.write("modification");
    file.close();
    
    QString checksum3 = m_backupSystem->calculateFileChecksum(identicalFile);
    QVERIFY(checksum1 != checksum3);
}

void TestOtbBackup::testWriterIntegration()
{
    // Create OTB writer
    OtbWriter writer;
    
    // Create test data
    ServerItemList items;
    items.majorVersion = 1;
    items.minorVersion = 0;
    items.buildNumber = 1;
    items.clientVersion = 770;
    
    // Add test items
    for (int i = 1; i <= 5; ++i) {
        ServerItem item;
        item.id = i;
        item.clientId = i;
        item.type = ServerItemType::Ground;
        items.items.append(item);
    }
    
    QString outputFile = m_tempDir->path() + "/writer_test.otb";
    
    // First write - no backup should be created
    QString errorString;
    QVERIFY(writer.write(outputFile, items, errorString));
    
    QList<BackupMetadata> backups = m_backupSystem->listBackups(outputFile);
    QCOMPARE(backups.size(), 0); // No backup for new file
    
    // Second write - automatic backup should be created
    items.items.append(ServerItem()); // Add another item
    QVERIFY(writer.write(outputFile, items, errorString));
    
    backups = m_backupSystem->listBackups(outputFile);
    QCOMPARE(backups.size(), 1); // One automatic backup
    QCOMPARE(backups.first().type, BackupType::Automatic);
}

void TestOtbBackup::testMultipleBackups()
{
    // Create multiple backups for the same file
    for (int i = 0; i < 7; ++i) {
        createTestOtbFile(m_testOtbFile, 10 + i);
        BackupResult result = m_backupSystem->createBackup(m_testOtbFile, BackupType::Manual, 
                                                          QString("Backup %1").arg(i));
        QVERIFY(result.success);
        QThread::msleep(10); // Ensure different timestamps
    }
    
    // Verify backup count limit is enforced
    QVERIFY(m_backupSystem->cleanupBackups());
    
    QList<BackupMetadata> backups = m_backupSystem->listBackups(m_testOtbFile);
    QVERIFY(backups.size() <= 5); // Should be limited by maxBackupCount
}

void TestOtbBackup::testBackupStatistics()
{
    // Create some backups
    m_backupSystem->createBackup(m_testOtbFile, BackupType::Manual);
    m_backupSystem->createBackup(m_testOtbFile, BackupType::Automatic);
    
    // Test statistics
    QCOMPARE(m_backupSystem->getBackupCount(), 2);
    QVERIFY(m_backupSystem->getTotalBackupSize() > 0);
    
    QStringList stats = m_backupSystem->getBackupStatistics();
    QVERIFY(!stats.isEmpty());
    QVERIFY(stats.first().contains("Total backups: 2"));
}

void TestOtbBackup::testInvalidFileBackup()
{
    QString nonExistentFile = m_tempDir->path() + "/nonexistent.otb";
    
    BackupResult result = m_backupSystem->createBackup(nonExistentFile, BackupType::Manual);
    QVERIFY(!result.success);
    QVERIFY(!result.errorMessage.isEmpty());
    QVERIFY(result.errorMessage.contains("does not exist"));
}

void TestOtbBackup::testCorruptedBackupRecovery()
{
    // Create backup
    BackupResult backupResult = m_backupSystem->createBackup(m_testOtbFile, BackupType::Manual);
    QVERIFY(backupResult.success);
    
    // Corrupt backup file
    QFile backupFile(backupResult.backupPath);
    QVERIFY(backupFile.open(QIODevice::WriteOnly));
    backupFile.write("corrupted");
    backupFile.close();
    
    // Try to restore - should fail
    RecoveryResult result = m_backupSystem->restoreFromBackup(backupResult.backupId);
    QVERIFY(!result.success);
}

void TestOtbBackup::testInsufficientSpace()
{
    // This test is difficult to implement reliably across different systems
    // We'll just verify the backup system handles errors gracefully
    
    // Try to create backup with invalid backup directory
    BackupConfig config = m_backupSystem->getConfiguration();
    config.backupDirectory = "/invalid/path/that/should/not/exist";
    m_backupSystem->setConfiguration(config);
    
    BackupResult result = m_backupSystem->createBackup(m_testOtbFile, BackupType::Manual);
    QVERIFY(!result.success);
    QVERIFY(!result.errorMessage.isEmpty());
}

// Helper methods

void TestOtbBackup::createTestOtbFile(const QString& filePath, int itemCount)
{
    OtbWriter writer;
    ServerItemList items;
    
    items.majorVersion = 1;
    items.minorVersion = 0;
    items.buildNumber = 1;
    items.clientVersion = 770;
    
    for (int i = 1; i <= itemCount; ++i) {
        ServerItem item;
        item.id = i;
        item.clientId = i;
        item.type = ServerItemType::Ground;
        item.name = QString("Test Item %1").arg(i);
        items.items.append(item);
    }
    
    QString errorString;
    bool success = writer.write(filePath, items, errorString);
    if (!success) {
        qWarning() << "Failed to create test OTB file:" << errorString;
    }
    QVERIFY(success);
}

void TestOtbBackup::verifyBackupMetadata(const BackupMetadata& metadata)
{
    QVERIFY(metadata.isValid());
    QVERIFY(!metadata.backupId.isEmpty());
    QVERIFY(!metadata.originalFilePath.isEmpty());
    QVERIFY(!metadata.backupFilePath.isEmpty());
    QVERIFY(metadata.originalFileSize > 0);
    QVERIFY(metadata.backupFileSize > 0);
    QVERIFY(!metadata.checksum.isEmpty());
    QVERIFY(metadata.createdAt.isValid());
}

void TestOtbBackup::verifyOtbFileContent(const QString& filePath, int expectedItemCount)
{
    OtbReader reader;
    ServerItemList items;
    QString errorString;
    
    QVERIFY(reader.read(filePath, items, errorString));
    QCOMPARE(items.items.size(), expectedItemCount);
}

QTEST_MAIN(TestOtbBackup)
#include "test_otbbackup.moc"