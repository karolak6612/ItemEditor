#include <QCoreApplication>
#include <QDebug>
#include <QTemporaryDir>
#include <QTextStream>
#include "otbbackup_standalone.h"

using namespace OTB;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== OTB Backup System Demo ===";
    
    // Create temporary directory for demo
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        qDebug() << "Failed to create temporary directory";
        return 1;
    }
    
    qDebug() << "Using temporary directory:" << tempDir.path();
    
    // Create test file
    QString testFile = tempDir.path() + "/test.txt";
    QFile file(testFile);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to create test file";
        return 1;
    }
    
    QTextStream stream(&file);
    stream << "This is a test file for backup demonstration.\n";
    stream << "Current time: " << QDateTime::currentDateTime().toString() << "\n";
    file.close();
    
    qDebug() << "Created test file:" << testFile;
    
    // Create backup system
    OtbBackupSystem backupSystem;
    
    // Configure backup system
    BackupConfig config;
    config.backupDirectory = tempDir.path() + "/backups";
    config.autoBackupEnabled = true;
    config.verifyBackupIntegrity = false; // Disable for demo
    config.validationLevel = ValidationLevel::None;
    
    backupSystem.setConfiguration(config);
    
    qDebug() << "Configured backup system with directory:" << config.backupDirectory;
    
    // Create manual backup
    qDebug() << "\n--- Creating Manual Backup ---";
    BackupResult result = backupSystem.createBackup(testFile, BackupType::Manual, "Demo backup");
    
    if (result.success) {
        qDebug() << "✓ Backup created successfully!";
        qDebug() << "  Backup ID:" << result.backupId;
        qDebug() << "  Backup path:" << result.backupPath;
        qDebug() << "  Backup size:" << result.backupSize << "bytes";
        qDebug() << "  Processing time:" << result.processingTimeMs << "ms";
    } else {
        qDebug() << "✗ Backup failed:" << result.errorMessage;
        return 1;
    }
    
    // List backups
    qDebug() << "\n--- Listing Backups ---";
    QList<BackupMetadata> backups = backupSystem.listBackups();
    qDebug() << "Total backups found:" << backups.size();
    
    for (const BackupMetadata& backup : backups) {
        qDebug() << "  Backup:" << backup.backupId;
        qDebug() << "    Type:" << static_cast<int>(backup.type);
        qDebug() << "    Created:" << backup.createdAt.toString();
        qDebug() << "    Description:" << backup.description;
        qDebug() << "    Original size:" << backup.originalFileSize;
        qDebug() << "    Backup size:" << backup.backupFileSize;
    }
    
    // Test backup validation
    qDebug() << "\n--- Validating Backup ---";
    bool isValid = backupSystem.validateBackup(result.backupId, ValidationLevel::Basic);
    qDebug() << "Backup validation result:" << (isValid ? "VALID" : "INVALID");
    
    // Test checksum calculation
    qDebug() << "\n--- Testing Checksum ---";
    QString originalChecksum = backupSystem.calculateFileChecksum(testFile);
    QString backupChecksum = backupSystem.calculateFileChecksum(result.backupPath);
    qDebug() << "Original file checksum:" << originalChecksum;
    qDebug() << "Backup file checksum:" << backupChecksum;
    qDebug() << "Checksums match:" << (originalChecksum == backupChecksum ? "YES" : "NO");
    
    // Test file recovery
    qDebug() << "\n--- Testing Recovery ---";
    QString recoveryFile = tempDir.path() + "/recovered.txt";
    RecoveryResult recoveryResult = backupSystem.restoreFromBackup(result.backupId, recoveryFile);
    
    if (recoveryResult.success) {
        qDebug() << "✓ Recovery successful!";
        qDebug() << "  Restored to:" << recoveryResult.restoredFilePath;
        qDebug() << "  Processing time:" << recoveryResult.processingTimeMs << "ms";
        
        // Verify recovered file content
        QFile recoveredFile(recoveryFile);
        if (recoveredFile.open(QIODevice::ReadOnly)) {
            QString content = recoveredFile.readAll();
            qDebug() << "  Recovered file content preview:" << content.left(50) + "...";
        }
    } else {
        qDebug() << "✗ Recovery failed:" << recoveryResult.errorMessage;
    }
    
    // Test backup statistics
    qDebug() << "\n--- Backup Statistics ---";
    QStringList stats = backupSystem.getBackupStatistics();
    for (const QString& stat : stats) {
        qDebug() << "  " << stat;
    }
    
    // Test automatic backup
    qDebug() << "\n--- Testing Automatic Backup ---";
    
    // Modify the original file
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream(&file);
        stream << "Modified content: " << QDateTime::currentDateTime().toString() << "\n";
        file.close();
    }
    
    BackupResult autoResult = backupSystem.createAutomaticBackup(testFile);
    if (autoResult.success) {
        qDebug() << "✓ Automatic backup created successfully!";
        qDebug() << "  Backup ID:" << autoResult.backupId;
    } else {
        qDebug() << "✗ Automatic backup failed:" << autoResult.errorMessage;
    }
    
    // Final backup count
    int finalBackupCount = backupSystem.getBackupCount();
    qDebug() << "\n--- Final Results ---";
    qDebug() << "Total backups created:" << finalBackupCount;
    qDebug() << "Total backup size:" << backupSystem.getTotalBackupSize() << "bytes";
    
    qDebug() << "\n=== Demo Completed Successfully ===";
    
    return 0;
}