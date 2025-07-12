#include "otbbackup_standalone.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QTextStream>
#include <QUuid>
#include <QElapsedTimer>
#include <QMutexLocker>
#include <QDebug>
#include <algorithm>

namespace OTB {

// Static constants
const QString OtbBackupSystemStandalone::BACKUP_METADATA_EXTENSION = ".backup.json";
const QString OtbBackupSystemStandalone::BACKUP_FILE_EXTENSION = ".backup.otb";
const QString OtbBackupSystemStandalone::CONFIG_FILE_NAME = "backup_config.json";
const QString OtbBackupSystemStandalone::CRASH_RECOVERY_PREFIX = "crash_recovery_";

// BackupMetadata implementation
bool BackupMetadata::isValid() const {
    return !backupId.isEmpty() && 
           !originalFilePath.isEmpty() && 
           !backupFilePath.isEmpty() &&
           originalFileSize >= 0 &&
           backupFileSize >= 0;
}

QJsonObject BackupMetadata::toJson() const {
    QJsonObject obj;
    obj["backupId"] = backupId;
    obj["originalFilePath"] = originalFilePath;
    obj["backupFilePath"] = backupFilePath;
    obj["type"] = static_cast<int>(type);
    obj["createdAt"] = createdAt.toString(Qt::ISODate);
    obj["originalFileSize"] = static_cast<qint64>(originalFileSize);
    obj["backupFileSize"] = static_cast<qint64>(backupFileSize);
    obj["checksum"] = checksum;
    obj["description"] = description;
    obj["applicationVersion"] = applicationVersion;
    obj["additionalData"] = additionalData;
    return obj;
}

bool BackupMetadata::fromJson(const QJsonObject& json) {
    if (!json.contains("backupId") || !json.contains("originalFilePath")) {
        return false;
    }
    
    backupId = json["backupId"].toString();
    originalFilePath = json["originalFilePath"].toString();
    backupFilePath = json["backupFilePath"].toString();
    type = static_cast<BackupType>(json["type"].toInt());
    createdAt = QDateTime::fromString(json["createdAt"].toString(), Qt::ISODate);
    originalFileSize = json["originalFileSize"].toVariant().toLongLong();
    backupFileSize = json["backupFileSize"].toVariant().toLongLong();
    checksum = json["checksum"].toString();
    description = json["description"].toString();
    applicationVersion = json["applicationVersion"].toString();
    additionalData = json["additionalData"].toObject();
    
    return isValid();
}

QString BackupMetadata::toString() const {
    return QString("Backup %1: %2 -> %3 (%4, %5)")
           .arg(backupId)
           .arg(QFileInfo(originalFilePath).fileName())
           .arg(QFileInfo(backupFilePath).fileName())
           .arg(createdAt.toString())
           .arg(description);
}

// BackupConfig implementation
BackupConfig::BackupConfig() {
    backupDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/backups";
}

bool BackupConfig::isValid() const {
    return maxBackupCount > 0 && 
           maxBackupAgeDays > 0 && 
           maxTotalBackupSize > 0 &&
           !backupDirectory.isEmpty();
}

QJsonObject BackupConfig::toJson() const {
    QJsonObject obj;
    obj["autoBackupEnabled"] = autoBackupEnabled;
    obj["crashRecoveryEnabled"] = crashRecoveryEnabled;
    obj["scheduledBackupEnabled"] = scheduledBackupEnabled;
    obj["maxBackupCount"] = maxBackupCount;
    obj["maxBackupAgeDays"] = maxBackupAgeDays;
    obj["maxTotalBackupSize"] = static_cast<qint64>(maxTotalBackupSize);
    obj["compressionLevel"] = static_cast<int>(compressionLevel);
    obj["validationLevel"] = static_cast<int>(validationLevel);
    obj["backupDirectory"] = backupDirectory;
    obj["scheduledBackupIntervalHours"] = scheduledBackupIntervalHours;
    obj["preserveBackupOnExit"] = preserveBackupOnExit;
    obj["verifyBackupIntegrity"] = verifyBackupIntegrity;
    return obj;
}

bool BackupConfig::fromJson(const QJsonObject& json) {
    autoBackupEnabled = json["autoBackupEnabled"].toBool(true);
    crashRecoveryEnabled = json["crashRecoveryEnabled"].toBool(true);
    scheduledBackupEnabled = json["scheduledBackupEnabled"].toBool(false);
    maxBackupCount = json["maxBackupCount"].toInt(10);
    maxBackupAgeDays = json["maxBackupAgeDays"].toInt(30);
    maxTotalBackupSize = json["maxTotalBackupSize"].toVariant().toLongLong();
    compressionLevel = static_cast<CompressionLevel>(json["compressionLevel"].toInt());
    validationLevel = static_cast<ValidationLevel>(json["validationLevel"].toInt());
    backupDirectory = json["backupDirectory"].toString();
    scheduledBackupIntervalHours = json["scheduledBackupIntervalHours"].toInt(24);
    preserveBackupOnExit = json["preserveBackupOnExit"].toBool(true);
    verifyBackupIntegrity = json["verifyBackupIntegrity"].toBool(true);
    
    if (backupDirectory.isEmpty()) {
        backupDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/backups";
    }
    
    return isValid();
}

// OtbBackupSystemStandalone implementation
OtbBackupSystemStandalone::OtbBackupSystemStandalone() {
    // Initialize default backup directory
    m_backupDirectory = m_config.backupDirectory;
    
    // Ensure backup directory exists
    ensureBackupDirectory();
}

OtbBackupSystemStandalone::~OtbBackupSystemStandalone() {
    // Cleanup if needed
}

bool OtbBackupSystemStandalone::loadConfiguration(const QString& configPath) {
    QMutexLocker locker(&m_mutex);
    
    QString path = configPath;
    if (path.isEmpty()) {
        path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/" + CONFIG_FILE_NAME;
    }
    
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        // Use default configuration if file doesn't exist
        m_config = BackupConfig();
        return true;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse backup configuration:" << error.errorString();
        return false;
    }
    
    if (!m_config.fromJson(doc.object())) {
        qWarning() << "Invalid backup configuration format";
        return false;
    }
    
    m_configFilePath = path;
    m_backupDirectory = m_config.backupDirectory;
    
    return ensureBackupDirectory();
}

bool OtbBackupSystemStandalone::saveConfiguration(const QString& configPath) const {
    QMutexLocker locker(&m_mutex);
    
    QString path = configPath;
    if (path.isEmpty()) {
        if (!m_configFilePath.isEmpty()) {
            path = m_configFilePath;
        } else {
            QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
            QDir().mkpath(configDir);
            path = configDir + "/" + CONFIG_FILE_NAME;
        }
    }
    
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open configuration file for writing:" << path;
        return false;
    }
    
    QJsonDocument doc(m_config.toJson());
    file.write(doc.toJson());
    
    return true;
}

void OtbBackupSystemStandalone::setConfiguration(const BackupConfig& config) {
    QMutexLocker locker(&m_mutex);
    
    m_config = config;
    m_backupDirectory = config.backupDirectory;
    
    ensureBackupDirectory();
}

BackupResult OtbBackupSystemStandalone::createBackup(const QString& filePath, BackupType type, const QString& description) {
    QMutexLocker locker(&m_mutex);
    QElapsedTimer timer;
    timer.start();
    
    updateStatus("Creating backup...");
    updateProgress(0);
    
    // Validate input parameters
    if (filePath.isEmpty()) {
        return BackupResult(false, "File path cannot be empty");
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return BackupResult(false, "Source file does not exist: " + filePath);
    }
    
    if (!fileInfo.isReadable()) {
        return BackupResult(false, "Source file is not readable: " + filePath);
    }
    
    try {
        // Ensure backup directory exists
        if (!ensureBackupDirectory()) {
            return BackupResult(false, "Failed to create backup directory");
        }
        
        updateProgress(10);
        
        // Generate backup metadata
        BackupMetadata metadata;
        metadata.backupId = generateBackupId();
        metadata.originalFilePath = QDir::toNativeSeparators(fileInfo.absoluteFilePath());
        metadata.type = type;
        metadata.createdAt = QDateTime::currentDateTime();
        metadata.originalFileSize = fileInfo.size();
        metadata.description = description;
        metadata.applicationVersion = QCoreApplication::applicationVersion();
        
        // Generate backup file path
        QString backupFileName = generateBackupFileName(filePath, type);
        metadata.backupFilePath = QDir(m_backupDirectory).absoluteFilePath(backupFileName);
        
        updateProgress(20);
        
        // Calculate source file checksum
        updateStatus("Calculating file checksum...");
        metadata.checksum = calculateFileChecksum(filePath);
        if (metadata.checksum.isEmpty()) {
            return BackupResult(false, "Failed to calculate source file checksum");
        }
        
        updateProgress(40);
        
        // Perform the actual backup
        updateStatus("Copying file...");
        if (!performBackup(filePath, metadata.backupFilePath, metadata)) {
            return BackupResult(false, "Failed to create backup file");
        }
        
        updateProgress(70);
        
        // Update backup file size
        QFileInfo backupFileInfo(metadata.backupFilePath);
        metadata.backupFileSize = backupFileInfo.size();
        
        // Save metadata
        updateStatus("Saving metadata...");
        if (!saveMetadata(metadata)) {
            // Clean up backup file if metadata save fails
            QFile::remove(metadata.backupFilePath);
            return BackupResult(false, "Failed to save backup metadata");
        }
        
        updateProgress(90);
        
        // Verify backup integrity if enabled
        if (m_config.verifyBackupIntegrity) {
            updateStatus("Verifying backup integrity...");
            if (!verifyBackupIntegrity(metadata.backupId)) {
                qWarning() << "Backup integrity verification failed for" << metadata.backupId;
                // Don't fail the operation, just log the warning
            }
        }
        
        updateProgress(100);
        updateStatus("Backup completed successfully");
        
        // Update last backup time
        m_lastBackupTime = QDateTime::currentDateTime();
        
        // Perform cleanup if needed
        if (type == BackupType::Automatic || type == BackupType::Scheduled) {
            cleanupBackups();
        }
        
        BackupResult result(true);
        result.backupId = metadata.backupId;
        result.backupPath = metadata.backupFilePath;
        result.processingTimeMs = timer.elapsed();
        result.backupSize = metadata.backupFileSize;
        
        BackupUtils::logBackupOperation("createBackup", true, 
            QString("Created backup %1 for %2").arg(metadata.backupId).arg(filePath));
        
        return result;
        
    } catch (const std::exception& e) {
        QString error = QString("Exception during backup creation: %1").arg(e.what());
        BackupUtils::logBackupOperation("createBackup", false, error);
        return BackupResult(false, error);
    }
}

BackupResult OtbBackupSystemStandalone::createAutomaticBackup(const QString& filePath) {
    if (!m_config.autoBackupEnabled) {
        return BackupResult(false, "Automatic backup is disabled");
    }
    
    QString description = QString("Automatic backup before modification at %1")
                         .arg(QDateTime::currentDateTime().toString());
    
    return createBackup(filePath, BackupType::Automatic, description);
}

BackupResult OtbBackupSystemStandalone::createCrashRecoveryBackup(const QString& filePath) {
    if (!m_config.crashRecoveryEnabled) {
        return BackupResult(false, "Crash recovery backup is disabled");
    }
    
    QString description = QString("Crash recovery backup at %1")
                         .arg(QDateTime::currentDateTime().toString());
    
    return createBackup(filePath, BackupType::CrashRecovery, description);
}QList<BackupMetadata> OtbBackupSystemStandalone::listBackups(const QString& originalFilePath) const {
    QMutexLocker locker(&m_mutex);
    QList<BackupMetadata> backups;
    
    QDir backupDir(m_backupDirectory);
    if (!backupDir.exists()) {
        return backups;
    }
    
    QStringList metadataFiles = backupDir.entryList(
        QStringList() << "*" + BACKUP_METADATA_EXTENSION,
        QDir::Files);
    
    for (const QString& metadataFile : metadataFiles) {
        BackupMetadata metadata;
        QString backupId = metadataFile;
        backupId.remove(BACKUP_METADATA_EXTENSION);
        
        if (loadMetadata(backupId, metadata)) {
            if (originalFilePath.isEmpty() || 
                QDir::toNativeSeparators(metadata.originalFilePath) == QDir::toNativeSeparators(originalFilePath)) {
                backups.append(metadata);
            }
        }
    }
    
    // Sort by creation time (newest first)
    std::sort(backups.begin(), backups.end(), 
              [](const BackupMetadata& a, const BackupMetadata& b) {
                  return a.createdAt > b.createdAt;
              });
    
    return backups;
}

QList<BackupMetadata> OtbBackupSystemStandalone::listBackupsByType(BackupType type) const {
    QList<BackupMetadata> allBackups = listBackups();
    QList<BackupMetadata> filteredBackups;
    
    for (const BackupMetadata& backup : allBackups) {
        if (backup.type == type) {
            filteredBackups.append(backup);
        }
    }
    
    return filteredBackups;
}

bool OtbBackupSystemStandalone::deleteBackup(const QString& backupId) {
    QMutexLocker locker(&m_mutex);
    
    // Load metadata to get file paths
    BackupMetadata metadata;
    if (!loadMetadata(backupId, metadata)) {
        qWarning() << "Cannot delete backup: metadata not found for" << backupId;
        return false;
    }
    
    bool success = true;
    
    // Delete backup file
    if (QFile::exists(metadata.backupFilePath)) {
        if (!QFile::remove(metadata.backupFilePath)) {
            qWarning() << "Failed to delete backup file:" << metadata.backupFilePath;
            success = false;
        }
    }
    
    // Delete metadata file
    if (!deleteMetadata(backupId)) {
        qWarning() << "Failed to delete backup metadata for" << backupId;
        success = false;
    }
    
    if (success) {
        BackupUtils::logBackupOperation("deleteBackup", true, 
            QString("Deleted backup %1").arg(backupId));
    }
    
    return success;
}

bool OtbBackupSystemStandalone::cleanupBackups() {
    QMutexLocker locker(&m_mutex);
    
    QList<BackupMetadata> backups = listBackups();
    bool success = true;
    int deletedCount = 0;
    
    // Group backups by original file path
    QMap<QString, QList<BackupMetadata>> backupsByFile;
    for (const BackupMetadata& backup : backups) {
        backupsByFile[backup.originalFilePath].append(backup);
    }
    
    // Clean up each file's backups
    for (auto it = backupsByFile.begin(); it != backupsByFile.end(); ++it) {
        QList<BackupMetadata>& fileBackups = it.value();
        
        // Sort by creation time (newest first)
        std::sort(fileBackups.begin(), fileBackups.end(), 
                  [](const BackupMetadata& a, const BackupMetadata& b) {
                      return a.createdAt > b.createdAt;
                  });
        
        // Keep only the most recent backups up to maxBackupCount
        while (fileBackups.size() > m_config.maxBackupCount) {
            const BackupMetadata& oldestBackup = fileBackups.last();
            if (deleteBackup(oldestBackup.backupId)) {
                deletedCount++;
                fileBackups.removeLast();
            } else {
                success = false;
                break;
            }
        }
    }
    
    // Delete old backups based on age
    if (success) {
        deleteOldBackups();
    }
    
    BackupUtils::logBackupOperation("cleanupBackups", success, 
        QString("Cleanup completed, deleted %1 backups").arg(deletedCount));
    
    return success;
}

bool OtbBackupSystemStandalone::deleteOldBackups(int maxAge) {
    QMutexLocker locker(&m_mutex);
    
    int ageLimit = (maxAge > 0) ? maxAge : m_config.maxBackupAgeDays;
    QDateTime cutoffTime = QDateTime::currentDateTime().addDays(-ageLimit);
    
    QList<BackupMetadata> backups = listBackups();
    bool success = true;
    int deletedCount = 0;
    
    for (const BackupMetadata& backup : backups) {
        if (backup.createdAt < cutoffTime) {
            if (deleteBackup(backup.backupId)) {
                deletedCount++;
            } else {
                success = false;
            }
        }
    }
    
    BackupUtils::logBackupOperation("deleteOldBackups", success, 
        QString("Deleted %1 old backups").arg(deletedCount));
    
    return success;
}

RecoveryResult OtbBackupSystemStandalone::restoreFromBackup(const QString& backupId, const QString& targetPath) {
    QMutexLocker locker(&m_mutex);
    QElapsedTimer timer;
    timer.start();
    
    updateStatus("Restoring from backup...");
    updateProgress(0);
    
    try {
        // Load backup metadata
        BackupMetadata metadata;
        if (!loadMetadata(backupId, metadata)) {
            return RecoveryResult(false, "Backup metadata not found: " + backupId);
        }
        
        updateProgress(10);
        
        // Verify backup file exists
        if (!QFile::exists(metadata.backupFilePath)) {
            return RecoveryResult(false, "Backup file not found: " + metadata.backupFilePath);
        }
        
        // Determine target path
        QString actualTargetPath = targetPath;
        if (actualTargetPath.isEmpty()) {
            actualTargetPath = metadata.originalFilePath;
        }
        
        updateProgress(20);
        
        // Validate backup integrity
        updateStatus("Validating backup integrity...");
        if (!validateBackup(backupId, m_config.validationLevel)) {
            return RecoveryResult(false, "Backup validation failed");
        }
        
        updateProgress(50);
        
        // Create backup of current file if it exists
        if (QFile::exists(actualTargetPath)) {
            QString tempBackup = actualTargetPath + ".recovery_backup";
            if (!QFile::copy(actualTargetPath, tempBackup)) {
                qWarning() << "Failed to create temporary backup of current file";
            }
        }
        
        updateProgress(60);
        
        // Perform the recovery
        updateStatus("Restoring file...");
        if (!performRecovery(metadata.backupFilePath, actualTargetPath, metadata)) {
            return RecoveryResult(false, "Failed to restore file from backup");
        }
        
        updateProgress(90);
        
        // Verify restored file
        updateStatus("Verifying restored file...");
        ValidationLevel validationLevel = m_config.validationLevel;
        if (validationLevel > ValidationLevel::Basic && !validateFileBasic(actualTargetPath)) {
            RecoveryResult result(false, "Restored file validation failed");
            result.validationPerformed = validationLevel;
            return result;
        }
        
        updateProgress(100);
        updateStatus("Recovery completed successfully");
        
        RecoveryResult result(true);
        result.restoredFilePath = actualTargetPath;
        result.processingTimeMs = timer.elapsed();
        result.validationPerformed = validationLevel;
        
        BackupUtils::logBackupOperation("restoreFromBackup", true, 
            QString("Restored backup %1 to %2").arg(backupId).arg(actualTargetPath));
        
        return result;
        
    } catch (const std::exception& e) {
        QString error = QString("Exception during recovery: %1").arg(e.what());
        BackupUtils::logBackupOperation("restoreFromBackup", false, error);
        return RecoveryResult(false, error);
    }
}

RecoveryResult OtbBackupSystemStandalone::restoreLatestBackup(const QString& originalFilePath, const QString& targetPath) {
    QList<BackupMetadata> backups = listBackups(originalFilePath);
    
    if (backups.isEmpty()) {
        return RecoveryResult(false, "No backups found for file: " + originalFilePath);
    }
    
    // Sort by creation time (newest first)
    std::sort(backups.begin(), backups.end(), 
              [](const BackupMetadata& a, const BackupMetadata& b) {
                  return a.createdAt > b.createdAt;
              });
    
    return restoreFromBackup(backups.first().backupId, targetPath);
}

RecoveryResult OtbBackupSystemStandalone::performCrashRecovery(const QString& originalFilePath) {
    QStringList crashBackups = getCrashRecoveryBackups(originalFilePath);
    
    if (crashBackups.isEmpty()) {
        return RecoveryResult(false, "No crash recovery backups found");
    }
    
    // Use the most recent crash recovery backup
    return restoreFromBackup(crashBackups.first(), originalFilePath);
}

bool OtbBackupSystemStandalone::validateBackup(const QString& backupId, ValidationLevel level) {
    QMutexLocker locker(&m_mutex);
    
    // Load metadata
    BackupMetadata metadata;
    if (!loadMetadata(backupId, metadata)) {
        return false;
    }
    
    // Check if backup file exists
    if (!QFile::exists(metadata.backupFilePath)) {
        return false;
    }
    
    // Basic validation - file size and checksum
    if (level >= ValidationLevel::Basic) {
        QFileInfo fileInfo(metadata.backupFilePath);
        
        // For compressed backups, we can't directly compare file sizes
        if (m_config.compressionLevel == CompressionLevel::None) {
            if (fileInfo.size() != metadata.backupFileSize) {
                qWarning() << "Backup file size mismatch for" << backupId;
                return false;
            }
        }
        
        // Verify checksum if available
        if (!metadata.checksum.isEmpty()) {
            QString actualChecksum = calculateFileChecksum(metadata.backupFilePath);
            if (actualChecksum != metadata.checksum && m_config.compressionLevel == CompressionLevel::None) {
                qWarning() << "Backup checksum mismatch for" << backupId;
                return false;
            }
        }
    }
    
    // Structure validation - simplified for standalone version
    if (level >= ValidationLevel::Structure) {
        if (!validateFileBasic(metadata.backupFilePath)) {
            qWarning() << "Backup file structure validation failed for" << backupId;
            return false;
        }
    }
    
    return true;
}

bool OtbBackupSystemStandalone::verifyBackupIntegrity(const QString& backupId) {
    return validateBackup(backupId, ValidationLevel::Complete);
}

QString OtbBackupSystemStandalone::calculateFileChecksum(const QString& filePath) const {
    return BackupUtils::calculateSHA256(filePath);
}

bool OtbBackupSystemStandalone::hasCrashRecoveryData(const QString& filePath) const {
    QStringList crashBackups = getCrashRecoveryBackups(filePath);
    return !crashBackups.isEmpty();
}

QStringList OtbBackupSystemStandalone::getCrashRecoveryBackups(const QString& filePath) const {
    QList<BackupMetadata> crashBackups = listBackupsByType(BackupType::CrashRecovery);
    QStringList result;
    
    QString normalizedPath = QDir::toNativeSeparators(QFileInfo(filePath).absoluteFilePath());
    
    for (const BackupMetadata& backup : crashBackups) {
        if (QDir::toNativeSeparators(backup.originalFilePath) == normalizedPath) {
            result.append(backup.backupId);
        }
    }
    
    // Sort by creation time (newest first)
    std::sort(result.begin(), result.end(), [this](const QString& a, const QString& b) {
        BackupMetadata metaA, metaB;
        loadMetadata(a, metaA);
        loadMetadata(b, metaB);
        return metaA.createdAt > metaB.createdAt;
    });
    
    return result;
}

void OtbBackupSystemStandalone::clearCrashRecoveryData(const QString& filePath) {
    QStringList crashBackups = getCrashRecoveryBackups(filePath);
    
    for (const QString& backupId : crashBackups) {
        deleteBackup(backupId);
    }
}

qint64 OtbBackupSystemStandalone::getTotalBackupSize() const {
    QMutexLocker locker(&m_mutex);
    
    return BackupUtils::getDirectorySize(m_backupDirectory);
}

int OtbBackupSystemStandalone::getBackupCount() const {
    return listBackups().size();
}

QDateTime OtbBackupSystemStandalone::getLastBackupTime() const {
    QMutexLocker locker(&m_mutex);
    return m_lastBackupTime;
}

QStringList OtbBackupSystemStandalone::getBackupStatistics() const {
    QMutexLocker locker(&m_mutex);
    QStringList stats;
    
    QList<BackupMetadata> backups = listBackups();
    qint64 totalSize = getTotalBackupSize();
    
    stats << QString("Total backups: %1").arg(backups.size());
    stats << QString("Total size: %1 MB").arg(totalSize / (1024 * 1024));
    stats << QString("Last backup: %1").arg(m_lastBackupTime.toString());
    
    // Count by type
    int manualCount = 0, autoCount = 0, crashCount = 0, scheduledCount = 0;
    for (const BackupMetadata& backup : backups) {
        switch (backup.type) {
            case BackupType::Manual: manualCount++; break;
            case BackupType::Automatic: autoCount++; break;
            case BackupType::CrashRecovery: crashCount++; break;
            case BackupType::Scheduled: scheduledCount++; break;
            default: break;
        }
    }
    
    stats << QString("Manual: %1, Automatic: %2, Crash Recovery: %3, Scheduled: %4")
             .arg(manualCount).arg(autoCount).arg(crashCount).arg(scheduledCount);
    
    return stats;
}

QString OtbBackupSystemStandalone::generateBackupId() const {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString OtbBackupSystemStandalone::getDefaultBackupDirectory() const {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/backups";
}

bool OtbBackupSystemStandalone::isBackupDirectoryValid(const QString& directory) const {
    QDir dir(directory);
    return dir.exists() || dir.mkpath(".");
}// Private methods implementation

bool OtbBackupSystemStandalone::performBackup(const QString& sourceFile, const QString& backupFile, 
                                              const BackupMetadata& metadata) {
    
    // Create backup directory if it doesn't exist
    QFileInfo backupFileInfo(backupFile);
    QDir backupDir = backupFileInfo.dir();
    if (!backupDir.exists() && !backupDir.mkpath(".")) {
        qWarning() << "Failed to create backup directory:" << backupDir.absolutePath();
        return false;
    }
    
    // Direct copy (compression not implemented in standalone version)
    bool success = BackupUtils::copyFileWithProgress(sourceFile, backupFile, m_progressCallback);
    
    if (!success) {
        qWarning() << "Failed to copy file from" << sourceFile << "to" << backupFile;
        return false;
    }
    
    return true;
}

bool OtbBackupSystemStandalone::performRecovery(const QString& backupFile, const QString& targetFile,
                                                const BackupMetadata& metadata) {
    
    // Create target directory if it doesn't exist
    QFileInfo targetFileInfo(targetFile);
    QDir targetDir = targetFileInfo.dir();
    if (!targetDir.exists() && !targetDir.mkpath(".")) {
        qWarning() << "Failed to create target directory:" << targetDir.absolutePath();
        return false;
    }
    
    // Direct copy (decompression not implemented in standalone version)
    bool success = BackupUtils::copyFileWithProgress(backupFile, targetFile, m_progressCallback);
    
    if (!success) {
        qWarning() << "Failed to restore file from" << backupFile << "to" << targetFile;
        return false;
    }
    
    return true;
}

bool OtbBackupSystemStandalone::saveMetadata(const BackupMetadata& metadata) {
    QString metadataPath = getMetadataFilePath(metadata.backupId);
    
    QFile file(metadataPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open metadata file for writing:" << metadataPath;
        return false;
    }
    
    QJsonDocument doc(metadata.toJson());
    qint64 bytesWritten = file.write(doc.toJson());
    
    return bytesWritten > 0;
}

bool OtbBackupSystemStandalone::loadMetadata(const QString& backupId, BackupMetadata& metadata) const {
    QString metadataPath = getMetadataFilePath(backupId);
    
    QFile file(metadataPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse metadata file:" << metadataPath << error.errorString();
        return false;
    }
    
    return metadata.fromJson(doc.object());
}

bool OtbBackupSystemStandalone::deleteMetadata(const QString& backupId) {
    QString metadataPath = getMetadataFilePath(backupId);
    return QFile::remove(metadataPath);
}

QString OtbBackupSystemStandalone::getMetadataFilePath(const QString& backupId) const {
    return QDir(m_backupDirectory).absoluteFilePath(backupId + BACKUP_METADATA_EXTENSION);
}

bool OtbBackupSystemStandalone::ensureBackupDirectory() {
    QDir dir(m_backupDirectory);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Failed to create backup directory:" << m_backupDirectory;
            return false;
        }
    }
    
    // Check if directory is writable
    QFileInfo dirInfo(m_backupDirectory);
    if (!dirInfo.isWritable()) {
        qWarning() << "Backup directory is not writable:" << m_backupDirectory;
        return false;
    }
    
    return true;
}

QString OtbBackupSystemStandalone::generateBackupFileName(const QString& originalPath, BackupType type) const {
    QFileInfo fileInfo(originalPath);
    QString baseName = fileInfo.baseName();
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString typePrefix;
    
    switch (type) {
        case BackupType::Manual: typePrefix = "manual"; break;
        case BackupType::Automatic: typePrefix = "auto"; break;
        case BackupType::Scheduled: typePrefix = "scheduled"; break;
        case BackupType::CrashRecovery: typePrefix = CRASH_RECOVERY_PREFIX.chopped(1); break;
        case BackupType::Checkpoint: typePrefix = "checkpoint"; break;
    }
    
    return QString("%1_%2_%3%4").arg(baseName).arg(typePrefix).arg(timestamp).arg(BACKUP_FILE_EXTENSION);
}

QString OtbBackupSystemStandalone::getBackupFilePath(const QString& backupId) const {
    // Load metadata to get the actual backup file path
    BackupMetadata metadata;
    if (loadMetadata(backupId, metadata)) {
        return metadata.backupFilePath;
    }
    
    return QString();
}

bool OtbBackupSystemStandalone::validateFileBasic(const QString& filePath) const {
    // Basic file existence and readability check
    QFileInfo fileInfo(filePath);
    return fileInfo.exists() && fileInfo.isReadable() && fileInfo.size() > 0;
}

bool OtbBackupSystemStandalone::validateFileIntegrity(const QString& filePath, const QString& expectedChecksum) const {
    if (expectedChecksum.isEmpty()) {
        return true; // No checksum to validate against
    }
    
    QString actualChecksum = calculateFileChecksum(filePath);
    return actualChecksum == expectedChecksum;
}

void OtbBackupSystemStandalone::performMaintenanceCleanup() {
    // This method can be called for scheduled maintenance
    cleanupBackups();
}

bool OtbBackupSystemStandalone::isBackupExpired(const BackupMetadata& metadata) const {
    QDateTime cutoffTime = QDateTime::currentDateTime().addDays(-m_config.maxBackupAgeDays);
    return metadata.createdAt < cutoffTime;
}

void OtbBackupSystemStandalone::updateProgress(int percentage) {
    if (m_progressCallback) {
        m_progressCallback(percentage);
    }
}

void OtbBackupSystemStandalone::updateStatus(const QString& status) {
    if (m_statusCallback) {
        m_statusCallback(status);
    }
}

// BackupUtils namespace implementation
namespace BackupUtils {

bool copyFileWithProgress(const QString& source, const QString& destination,
                         std::function<void(int)> progressCallback) {
    QFile sourceFile(source);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open source file:" << source;
        return false;
    }
    
    QFile destFile(destination);
    if (!destFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open destination file:" << destination;
        return false;
    }
    
    qint64 totalSize = sourceFile.size();
    qint64 copiedSize = 0;
    const qint64 bufferSize = 64 * 1024; // 64KB buffer
    
    QByteArray buffer;
    buffer.resize(bufferSize);
    
    while (!sourceFile.atEnd()) {
        qint64 bytesRead = sourceFile.read(buffer.data(), bufferSize);
        if (bytesRead <= 0) {
            qWarning() << "Failed to read from source file";
            return false;
        }
        
        qint64 bytesWritten = destFile.write(buffer.data(), bytesRead);
        if (bytesWritten != bytesRead) {
            qWarning() << "Failed to write to destination file";
            return false;
        }
        
        copiedSize += bytesWritten;
        
        if (progressCallback && totalSize > 0) {
            int percentage = static_cast<int>((copiedSize * 100) / totalSize);
            progressCallback(percentage);
        }
    }
    
    destFile.flush();
    return true;
}

bool moveFileAtomic(const QString& source, const QString& destination) {
    // First try to rename (atomic on same filesystem)
    if (QFile::rename(source, destination)) {
        return true;
    }
    
    // If rename fails, copy and delete
    if (copyFileWithProgress(source, destination)) {
        return QFile::remove(source);
    }
    
    return false;
}

QString calculateMD5(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    
    QCryptographicHash hash(QCryptographicHash::Md5);
    const qint64 bufferSize = 64 * 1024;
    
    while (!file.atEnd()) {
        QByteArray buffer = file.read(bufferSize);
        hash.addData(buffer);
    }
    
    return hash.result().toHex();
}

QString calculateSHA256(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    
    QCryptographicHash hash(QCryptographicHash::Sha256);
    const qint64 bufferSize = 64 * 1024;
    
    while (!file.atEnd()) {
        QByteArray buffer = file.read(bufferSize);
        hash.addData(buffer);
    }
    
    return hash.result().toHex();
}

bool createDirectoryRecursive(const QString& path) {
    QDir dir;
    return dir.mkpath(path);
}

qint64 getDirectorySize(const QString& path) {
    QDir dir(path);
    if (!dir.exists()) {
        return 0;
    }
    
    qint64 totalSize = 0;
    QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    
    for (const QFileInfo& entry : entries) {
        if (entry.isFile()) {
            totalSize += entry.size();
        } else if (entry.isDir()) {
            totalSize += getDirectorySize(entry.absoluteFilePath());
        }
    }
    
    return totalSize;
}

bool cleanDirectory(const QString& path, int maxAgeDays) {
    QDir dir(path);
    if (!dir.exists()) {
        return true;
    }
    
    QDateTime cutoffTime;
    if (maxAgeDays > 0) {
        cutoffTime = QDateTime::currentDateTime().addDays(-maxAgeDays);
    }
    
    QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    bool success = true;
    
    for (const QFileInfo& entry : entries) {
        if (maxAgeDays > 0 && entry.lastModified() >= cutoffTime) {
            continue; // Skip files that are not old enough
        }
        
        if (entry.isFile()) {
            if (!QFile::remove(entry.absoluteFilePath())) {
                qWarning() << "Failed to remove file:" << entry.absoluteFilePath();
                success = false;
            }
        } else if (entry.isDir()) {
            QDir subDir(entry.absoluteFilePath());
            if (!subDir.removeRecursively()) {
                qWarning() << "Failed to remove directory:" << entry.absoluteFilePath();
                success = false;
            }
        }
    }
    
    return success;
}

bool compareFiles(const QString& file1, const QString& file2) {
    QFile f1(file1), f2(file2);
    
    if (!f1.open(QIODevice::ReadOnly) || !f2.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    if (f1.size() != f2.size()) {
        return false;
    }
    
    const qint64 bufferSize = 64 * 1024;
    while (!f1.atEnd() && !f2.atEnd()) {
        QByteArray buffer1 = f1.read(bufferSize);
        QByteArray buffer2 = f2.read(bufferSize);
        
        if (buffer1 != buffer2) {
            return false;
        }
    }
    
    return f1.atEnd() && f2.atEnd();
}

bool isCompressionBeneficial(const QString& filePath, qint64 threshold) {
    QFileInfo fileInfo(filePath);
    return fileInfo.size() >= threshold;
}

CompressionLevel getOptimalCompressionLevel(qint64 fileSize) {
    if (fileSize < 1024 * 1024) { // < 1MB
        return CompressionLevel::Fast;
    } else if (fileSize < 10 * 1024 * 1024) { // < 10MB
        return CompressionLevel::Balanced;
    } else {
        return CompressionLevel::Maximum;
    }
}

QString formatBackupError(const QString& operation, const QString& details) {
    return QString("Backup operation '%1' failed: %2").arg(operation).arg(details);
}

void logBackupOperation(const QString& operation, bool success, const QString& details) {
    QString logMessage = QString("[BACKUP] %1: %2").arg(operation).arg(success ? "SUCCESS" : "FAILED");
    if (!details.isEmpty()) {
        logMessage += QString(" - %1").arg(details);
    }
    
    if (success) {
        qInfo() << logMessage;
    } else {
        qWarning() << logMessage;
    }
}

} // namespace BackupUtils

} // namespace OTB