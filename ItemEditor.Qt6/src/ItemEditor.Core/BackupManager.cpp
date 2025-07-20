#include "BackupManager.h"
#include "ErrorHandler.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QDebug>

BackupManager::BackupManager(QObject* parent)
    : QObject(parent)
    , m_automaticBackupTimer(nullptr)
    , m_fileWatcher(nullptr)
{
    initializeSettings();
    
    // Create automatic backup timer
    m_automaticBackupTimer = new QTimer(this);
    connect(m_automaticBackupTimer, &QTimer::timeout, this, &BackupManager::onAutomaticBackupTimer);
    
    // Create file system watcher
    m_fileWatcher = new QFileSystemWatcher(this);
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &BackupManager::onFileChanged);
}

BackupManager::~BackupManager()
{
    saveSettings();
    disableAutomaticBackup();
}

bool BackupManager::createBackup(const QString& filePath, BackupType type, const QString& description)
{
    if (!QFile::exists(filePath)) {
        reportError(QString("Cannot create backup: source file does not exist: %1").arg(filePath));
        return false;
    }
    
    QByteArray data = readFileData(filePath);
    if (data.isEmpty()) {
        reportError(QString("Cannot read source file for backup: %1").arg(filePath));
        return false;
    }
    
    return createBackupFromData(data, filePath, type, description);
}

bool BackupManager::createBackupFromData(const QByteArray& data, const QString& originalPath, 
                                        BackupType type, const QString& description)
{
    if (!ensureBackupDirectoryExists()) {
        reportError("Cannot create backup directory");
        return false;
    }
    
    QDateTime timestamp = QDateTime::currentDateTime();
    QString backupPath = getBackupPath(originalPath, type, m_settings.backupDirectory, timestamp);
    
    // Create backup info
    BackupInfo backup;
    backup.filePath = backupPath;
    backup.originalPath = originalPath;
    backup.type = type;
    backup.timestamp = timestamp;
    backup.fileSize = data.size();
    backup.description = description;
    backup.checksum = calculateDataChecksum(data);
    
    // Write backup file
    if (!writeDataToFile(data, backupPath)) {
        reportError(QString("Failed to write backup file: %1").arg(backupPath));
        return false;
    }
    
    // Verify integrity if enabled
    if (m_settings.verifyIntegrityOnCreate) {
        if (!verifyBackupIntegrity(backup)) {
            reportError(QString("Backup integrity verification failed: %1").arg(backupPath));
            QFile::remove(backupPath);
            return false;
        }
    }
    
    backup.isValid = true;
    
    // Write metadata
    if (!writeBackupMetadata(backup)) {
        reportWarning(QString("Failed to write backup metadata: %1").arg(backupPath));
    }
    
    // Cleanup old backups if needed
    cleanupOldBackups(originalPath, m_settings.maxBackupsPerFile);
    
    // Check storage usage
    qint64 currentUsage = calculateBackupStorageUsage();
    if (currentUsage > m_settings.maxBackupStorageSize) {
        emit storageWarning(currentUsage, m_settings.maxBackupStorageSize);
    }
    
    emit backupCreated(backup);
    
    qDebug() << "Backup created:" << backupPath;
    return true;
}

void BackupManager::enableAutomaticBackup(const QString& filePath, int intervalMinutes)
{
    disableAutomaticBackup();
    
    m_watchedFilePath = filePath;
    m_settings.automaticBackupInterval = intervalMinutes;
    
    if (intervalMinutes > 0) {
        m_automaticBackupTimer->start(intervalMinutes * 60 * 1000); // Convert to milliseconds
        
        // Watch for file changes
        if (QFile::exists(filePath)) {
            m_fileWatcher->addPath(filePath);
        }
        
        qDebug() << "Automatic backup enabled for:" << filePath << "interval:" << intervalMinutes << "minutes";
    }
}

void BackupManager::disableAutomaticBackup()
{
    if (m_automaticBackupTimer) {
        m_automaticBackupTimer->stop();
    }
    
    if (m_fileWatcher && !m_watchedFilePath.isEmpty()) {
        m_fileWatcher->removePath(m_watchedFilePath);
    }
    
    m_watchedFilePath.clear();
}

bool BackupManager::isAutomaticBackupEnabled() const
{
    return m_automaticBackupTimer && m_automaticBackupTimer->isActive();
}

int BackupManager::getAutomaticBackupInterval() const
{
    return m_settings.automaticBackupInterval;
}

void BackupManager::setAutomaticBackupInterval(int minutes)
{
    m_settings.automaticBackupInterval = minutes;
    
    if (isAutomaticBackupEnabled()) {
        m_automaticBackupTimer->setInterval(minutes * 60 * 1000);
    }
}

QList<BackupManager::BackupInfo> BackupManager::findBackups(const QString& originalPath)
{
    QList<BackupInfo> backups;
    
    QDir backupDir(m_settings.backupDirectory);
    if (!backupDir.exists()) {
        return backups;
    }
    
    QFileInfo originalFile(originalPath);
    QString baseName = originalFile.baseName();
    
    // Find all backup files for this original file
    QStringList filters;
    filters << QString("%1_*.bak").arg(baseName);
    filters << QString("%1_*.backup").arg(baseName);
    
    QFileInfoList backupFiles = backupDir.entryInfoList(filters, QDir::Files, QDir::Time | QDir::Reversed);
    
    for (const QFileInfo& fileInfo : backupFiles) {
        BackupInfo backup = readBackupMetadata(fileInfo.absoluteFilePath());
        if (backup.originalPath.isEmpty()) {
            // Create backup info from filename if metadata is missing
            backup.filePath = fileInfo.absoluteFilePath();
            backup.originalPath = originalPath;
            backup.timestamp = fileInfo.lastModified();
            backup.fileSize = fileInfo.size();
            backup.type = parseBackupType(fileInfo.fileName());
            backup.isValid = validateBackupFile(backup.filePath);
        }
        
        backups.append(backup);
    }
    
    return backups;
}

QList<BackupManager::BackupInfo> BackupManager::getAllBackups()
{
    QList<BackupInfo> backups;
    
    QDir backupDir(m_settings.backupDirectory);
    if (!backupDir.exists()) {
        return backups;
    }
    
    QStringList filters;
    filters << "*.bak" << "*.backup";
    
    QFileInfoList backupFiles = backupDir.entryInfoList(filters, QDir::Files, QDir::Time | QDir::Reversed);
    
    for (const QFileInfo& fileInfo : backupFiles) {
        BackupInfo backup = readBackupMetadata(fileInfo.absoluteFilePath());
        if (backup.originalPath.isEmpty()) {
            // Create basic backup info from file
            backup.filePath = fileInfo.absoluteFilePath();
            backup.timestamp = fileInfo.lastModified();
            backup.fileSize = fileInfo.size();
            backup.type = parseBackupType(fileInfo.fileName());
            backup.isValid = validateBackupFile(backup.filePath);
        }
        
        backups.append(backup);
    }
    
    return backups;
}

BackupManager::BackupInfo BackupManager::getLatestBackup(const QString& originalPath)
{
    QList<BackupInfo> backups = findBackups(originalPath);
    
    if (backups.isEmpty()) {
        return BackupInfo();
    }
    
    // Sort by timestamp (newest first)
    std::sort(backups.begin(), backups.end(), 
              [](const BackupInfo& a, const BackupInfo& b) {
                  return a.timestamp > b.timestamp;
              });
    
    return backups.first();
}

BackupManager::BackupInfo BackupManager::getBackupByTimestamp(const QString& originalPath, const QDateTime& timestamp)
{
    QList<BackupInfo> backups = findBackups(originalPath);
    
    for (const BackupInfo& backup : backups) {
        if (backup.timestamp == timestamp) {
            return backup;
        }
    }
    
    return BackupInfo();
}

BackupManager::RecoveryResult BackupManager::restoreFromBackup(const QString& backupPath, const QString& targetPath)
{
    if (!QFile::exists(backupPath)) {
        reportError(QString("Backup file does not exist: %1").arg(backupPath));
        return RecoveryResult::NoBackupFound;
    }
    
    // Verify backup integrity if enabled
    if (m_settings.verifyIntegrityOnRestore) {
        if (!verifyBackupIntegrity(backupPath)) {
            reportError(QString("Backup file is corrupted: %1").arg(backupPath));
            return RecoveryResult::BackupCorrupted;
        }
    }
    
    // Determine target path
    QString actualTargetPath = targetPath;
    if (actualTargetPath.isEmpty()) {
        BackupInfo backup = readBackupMetadata(backupPath);
        if (!backup.originalPath.isEmpty()) {
            actualTargetPath = backup.originalPath;
        } else {
            reportError("Cannot determine target path for restore operation");
            return RecoveryResult::Failed;
        }
    }
    
    // Create backup of current file before restore
    if (QFile::exists(actualTargetPath)) {
        if (!createBackup(actualTargetPath, BackupType::PreModification, "Before restore operation")) {
            reportWarning("Failed to create backup before restore");
        }
    }
    
    // Copy backup to target location
    if (!copyFileWithVerification(backupPath, actualTargetPath)) {
        reportError(QString("Failed to restore backup to: %1").arg(actualTargetPath));
        return RecoveryResult::Failed;
    }
    
    emit backupRestored(actualTargetPath, RecoveryResult::Success);
    
    qDebug() << "Backup restored from:" << backupPath << "to:" << actualTargetPath;
    return RecoveryResult::Success;
}

BackupManager::RecoveryResult BackupManager::restoreLatestBackup(const QString& originalPath)
{
    BackupInfo latestBackup = getLatestBackup(originalPath);
    
    if (latestBackup.filePath.isEmpty()) {
        return RecoveryResult::NoBackupFound;
    }
    
    return restoreFromBackup(latestBackup.filePath, originalPath);
}

BackupManager::RecoveryResult BackupManager::restoreBackupByTimestamp(const QString& originalPath, const QDateTime& timestamp)
{
    BackupInfo backup = getBackupByTimestamp(originalPath, timestamp);
    
    if (backup.filePath.isEmpty()) {
        return RecoveryResult::NoBackupFound;
    }
    
    return restoreFromBackup(backup.filePath, originalPath);
}

bool BackupManager::verifyBackupIntegrity(const QString& backupPath)
{
    BackupInfo backup = readBackupMetadata(backupPath);
    return verifyBackupIntegrity(backup);
}

bool BackupManager::verifyBackupIntegrity(const BackupInfo& backup)
{
    if (!QFile::exists(backup.filePath)) {
        return false;
    }
    
    // Check file size
    QFileInfo fileInfo(backup.filePath);
    if (fileInfo.size() != backup.fileSize) {
        return false;
    }
    
    // Check checksum if available
    if (!backup.checksum.isEmpty()) {
        QString currentChecksum = calculateFileChecksum(backup.filePath);
        if (currentChecksum != backup.checksum) {
            return false;
        }
    }
    
    // Additional validation
    return validateBackupFile(backup.filePath);
}

QStringList BackupManager::verifyAllBackups()
{
    QStringList corruptedBackups;
    QList<BackupInfo> allBackups = getAllBackups();
    
    for (const BackupInfo& backup : allBackups) {
        if (!verifyBackupIntegrity(backup)) {
            corruptedBackups.append(backup.filePath);
        }
    }
    
    return corruptedBackups;
}

void BackupManager::cleanupOldBackups(const QString& originalPath, int maxBackups)
{
    QList<BackupInfo> backups = findBackups(originalPath);
    
    if (backups.size() <= maxBackups) {
        return;
    }
    
    // Sort by timestamp (oldest first)
    std::sort(backups.begin(), backups.end(), 
              [](const BackupInfo& a, const BackupInfo& b) {
                  return a.timestamp < b.timestamp;
              });
    
    // Remove oldest backups
    int toRemove = backups.size() - maxBackups;
    for (int i = 0; i < toRemove; ++i) {
        QFile::remove(backups[i].filePath);
        
        // Remove metadata file if exists
        QString metadataPath = backups[i].filePath + ".meta";
        QFile::remove(metadataPath);
        
        qDebug() << "Removed old backup:" << backups[i].filePath;
    }
}

void BackupManager::cleanupBackupsByAge(int maxDays)
{
    QDateTime cutoffDate = QDateTime::currentDateTime().addDays(-maxDays);
    QList<BackupInfo> oldBackups = getOldBackups(maxDays);
    
    for (const BackupInfo& backup : oldBackups) {
        QFile::remove(backup.filePath);
        
        // Remove metadata file if exists
        QString metadataPath = backup.filePath + ".meta";
        QFile::remove(metadataPath);
        
        qDebug() << "Removed old backup:" << backup.filePath;
    }
}

void BackupManager::cleanupCorruptedBackups()
{
    QStringList corruptedBackups = verifyAllBackups();
    
    for (const QString& backupPath : corruptedBackups) {
        QFile::remove(backupPath);
        
        // Remove metadata file if exists
        QString metadataPath = backupPath + ".meta";
        QFile::remove(metadataPath);
        
        qDebug() << "Removed corrupted backup:" << backupPath;
    }
}

qint64 BackupManager::calculateBackupStorageUsage()
{
    qint64 totalSize = 0;
    QList<BackupInfo> allBackups = getAllBackups();
    
    for (const BackupInfo& backup : allBackups) {
        totalSize += backup.fileSize;
    }
    
    return totalSize;
}

BackupManager::BackupSettings BackupManager::getSettings() const
{
    return m_settings;
}

void BackupManager::setSettings(const BackupSettings& settings)
{
    m_settings = settings;
    saveSettings();
}

void BackupManager::loadSettings()
{
    QSettings settings;
    
    m_settings.enableAutomaticBackup = settings.value("BackupManager/enableAutomaticBackup", true).toBool();
    m_settings.automaticBackupInterval = settings.value("BackupManager/automaticBackupInterval", 5).toInt();
    m_settings.maxBackupsPerFile = settings.value("BackupManager/maxBackupsPerFile", 10).toInt();
    m_settings.maxBackupAge = settings.value("BackupManager/maxBackupAge", 30).toInt();
    m_settings.maxBackupStorageSize = settings.value("BackupManager/maxBackupStorageSize", 1024LL * 1024 * 1024).toLongLong();
    m_settings.backupDirectory = settings.value("BackupManager/backupDirectory", getDefaultBackupDirectory()).toString();
    m_settings.backupSuffix = settings.value("BackupManager/backupSuffix", ".bak").toString();
    m_settings.compressBackups = settings.value("BackupManager/compressBackups", false).toBool();
    m_settings.verifyIntegrityOnCreate = settings.value("BackupManager/verifyIntegrityOnCreate", true).toBool();
    m_settings.verifyIntegrityOnRestore = settings.value("BackupManager/verifyIntegrityOnRestore", true).toBool();
}

void BackupManager::saveSettings()
{
    QSettings settings;
    
    settings.setValue("BackupManager/enableAutomaticBackup", m_settings.enableAutomaticBackup);
    settings.setValue("BackupManager/automaticBackupInterval", m_settings.automaticBackupInterval);
    settings.setValue("BackupManager/maxBackupsPerFile", m_settings.maxBackupsPerFile);
    settings.setValue("BackupManager/maxBackupAge", m_settings.maxBackupAge);
    settings.setValue("BackupManager/maxBackupStorageSize", m_settings.maxBackupStorageSize);
    settings.setValue("BackupManager/backupDirectory", m_settings.backupDirectory);
    settings.setValue("BackupManager/backupSuffix", m_settings.backupSuffix);
    settings.setValue("BackupManager/compressBackups", m_settings.compressBackups);
    settings.setValue("BackupManager/verifyIntegrityOnCreate", m_settings.verifyIntegrityOnCreate);
    settings.setValue("BackupManager/verifyIntegrityOnRestore", m_settings.verifyIntegrityOnRestore);
}

QString BackupManager::getBackupDirectory() const
{
    return m_settings.backupDirectory;
}

bool BackupManager::setBackupDirectory(const QString& directory)
{
    QDir dir(directory);
    if (!dir.exists() && !dir.mkpath(".")) {
        return false;
    }
    
    m_settings.backupDirectory = directory;
    saveSettings();
    return true;
}

bool BackupManager::ensureBackupDirectoryExists()
{
    QDir dir(m_settings.backupDirectory);
    if (!dir.exists()) {
        return dir.mkpath(".");
    }
    return true;
}

QString BackupManager::generateBackupFileName(const QString& originalPath, BackupType type, const QDateTime& timestamp)
{
    QFileInfo fileInfo(originalPath);
    QString baseName = fileInfo.baseName();
    QString extension = fileInfo.suffix();
    
    QString typeStr;
    switch (type) {
        case BackupType::Manual: typeStr = "manual"; break;
        case BackupType::Automatic: typeStr = "auto"; break;
        case BackupType::PreSave: typeStr = "presave"; break;
        case BackupType::PreModification: typeStr = "premod"; break;
    }
    
    QString timestampStr = timestamp.toString("yyyyMMdd_hhmmss");
    
    return QString("%1_%2_%3.%4.bak").arg(baseName).arg(typeStr).arg(timestampStr).arg(extension);
}

QString BackupManager::getBackupPath(const QString& originalPath, BackupType type, 
                                   const QString& backupDirectory, const QDateTime& timestamp)
{
    QString fileName = generateBackupFileName(originalPath, type, timestamp);
    return QDir(backupDirectory).filePath(fileName);
}

BackupManager::BackupType BackupManager::parseBackupType(const QString& fileName)
{
    if (fileName.contains("_manual_")) return BackupType::Manual;
    if (fileName.contains("_auto_")) return BackupType::Automatic;
    if (fileName.contains("_presave_")) return BackupType::PreSave;
    if (fileName.contains("_premod_")) return BackupType::PreModification;
    return BackupType::Manual;
}

QDateTime BackupManager::parseBackupTimestamp(const QString& fileName)
{
    QRegularExpression regex(R"(_(\d{8}_\d{6})\.)");
    QRegularExpressionMatch match = regex.match(fileName);
    
    if (match.hasMatch()) {
        QString timestampStr = match.captured(1);
        return QDateTime::fromString(timestampStr, "yyyyMMdd_hhmmss");
    }
    
    return QDateTime();
}

void BackupManager::onAutomaticBackupTimer()
{
    if (!m_watchedFilePath.isEmpty() && QFile::exists(m_watchedFilePath)) {
        if (createBackup(m_watchedFilePath, BackupType::Automatic, "Automatic backup")) {
            emit automaticBackupTriggered(m_watchedFilePath);
        }
    }
}

void BackupManager::onFileChanged(const QString& path)
{
    Q_UNUSED(path)
    // File changed, could trigger immediate backup if needed
    // For now, we rely on the timer-based automatic backup
}

QString BackupManager::calculateFileChecksum(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    
    QCryptographicHash hash(QCryptographicHash::Md5);
    if (hash.addData(&file)) {
        return hash.result().toHex();
    }
    
    return QString();
}

QString BackupManager::calculateDataChecksum(const QByteArray& data)
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(data);
    return hash.result().toHex();
}

bool BackupManager::writeBackupMetadata(const BackupInfo& backup)
{
    QString metadataPath = backup.filePath + ".meta";
    
    QJsonObject json;
    json["originalPath"] = backup.originalPath;
    json["type"] = static_cast<int>(backup.type);
    json["timestamp"] = backup.timestamp.toString(Qt::ISODate);
    json["fileSize"] = backup.fileSize;
    json["description"] = backup.description;
    json["checksum"] = backup.checksum;
    json["isValid"] = backup.isValid;
    
    QJsonDocument doc(json);
    
    QFile file(metadataPath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    file.write(doc.toJson());
    return true;
}

BackupManager::BackupInfo BackupManager::readBackupMetadata(const QString& backupPath)
{
    BackupInfo backup;
    backup.filePath = backupPath;
    
    QString metadataPath = backupPath + ".meta";
    
    QFile file(metadataPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return backup; // Return empty backup info
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (!doc.isObject()) {
        return backup;
    }
    
    QJsonObject json = doc.object();
    
    backup.originalPath = json["originalPath"].toString();
    backup.type = static_cast<BackupType>(json["type"].toInt());
    backup.timestamp = QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate);
    backup.fileSize = json["fileSize"].toVariant().toLongLong();
    backup.description = json["description"].toString();
    backup.checksum = json["checksum"].toString();
    backup.isValid = json["isValid"].toBool();
    
    return backup;
}

bool BackupManager::copyFileWithVerification(const QString& source, const QString& destination)
{
    // Remove destination if it exists
    if (QFile::exists(destination)) {
        QFile::remove(destination);
    }
    
    // Copy file
    if (!QFile::copy(source, destination)) {
        return false;
    }
    
    // Verify copy
    QString sourceChecksum = calculateFileChecksum(source);
    QString destChecksum = calculateFileChecksum(destination);
    
    return sourceChecksum == destChecksum;
}

bool BackupManager::writeDataToFile(const QByteArray& data, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    qint64 written = file.write(data);
    file.close();
    
    return written == data.size();
}

QByteArray BackupManager::readFileData(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    
    return file.readAll();
}

bool BackupManager::validateBackupFile(const QString& backupPath)
{
    // Basic validation - check if file exists and is readable
    QFile file(backupPath);
    return file.exists() && file.open(QIODevice::ReadOnly);
}

bool BackupManager::isBackupFile(const QString& filePath)
{
    return filePath.endsWith(".bak") || filePath.endsWith(".backup");
}

QList<BackupManager::BackupInfo> BackupManager::getBackupsForCleanup(const QString& originalPath, int maxBackups)
{
    QList<BackupInfo> backups = findBackups(originalPath);
    
    if (backups.size() <= maxBackups) {
        return QList<BackupInfo>();
    }
    
    // Sort by timestamp (oldest first)
    std::sort(backups.begin(), backups.end(), 
              [](const BackupInfo& a, const BackupInfo& b) {
                  return a.timestamp < b.timestamp;
              });
    
    // Return backups to remove
    int toRemove = backups.size() - maxBackups;
    return backups.mid(0, toRemove);
}

QList<BackupManager::BackupInfo> BackupManager::getOldBackups(int maxDays)
{
    QDateTime cutoffDate = QDateTime::currentDateTime().addDays(-maxDays);
    QList<BackupInfo> allBackups = getAllBackups();
    QList<BackupInfo> oldBackups;
    
    for (const BackupInfo& backup : allBackups) {
        if (backup.timestamp < cutoffDate) {
            oldBackups.append(backup);
        }
    }
    
    return oldBackups;
}

QList<BackupManager::BackupInfo> BackupManager::getCorruptedBackups()
{
    QList<BackupInfo> allBackups = getAllBackups();
    QList<BackupInfo> corruptedBackups;
    
    for (const BackupInfo& backup : allBackups) {
        if (!verifyBackupIntegrity(backup)) {
            corruptedBackups.append(backup);
        }
    }
    
    return corruptedBackups;
}

void BackupManager::reportError(const QString& error)
{
    qDebug() << "BackupManager Error:" << error;
    emit backupError(error);
    
    if (g_errorHandler) {
        g_errorHandler->reportError(ErrorHandler::ErrorLevel::Error, 
                                   ErrorHandler::ErrorCategory::FileIO, 
                                   error, "BackupManager");
    }
}

void BackupManager::reportWarning(const QString& warning)
{
    qDebug() << "BackupManager Warning:" << warning;
    
    if (g_errorHandler) {
        g_errorHandler->reportError(ErrorHandler::ErrorLevel::Warning, 
                                   ErrorHandler::ErrorCategory::FileIO, 
                                   warning, "BackupManager");
    }
}

void BackupManager::initializeSettings()
{
    loadSettings();
    ensureBackupDirectoryExists();
}

QString BackupManager::getDefaultBackupDirectory()
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(appDataPath).filePath("Backups");
}