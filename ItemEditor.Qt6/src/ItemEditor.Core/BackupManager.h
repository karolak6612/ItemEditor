#pragma once

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QObject>
#include <QTimer>
#include <QFileSystemWatcher>
#include "ServerItemList.h"

/**
 * @brief Comprehensive backup and recovery system
 * 
 * Provides automatic backup creation, recovery procedures, and data integrity
 * verification with identical behavior to the legacy system.
 */
class BackupManager : public QObject
{
    Q_OBJECT

public:
    enum class BackupType {
        Manual,         // User-initiated backup
        Automatic,      // Scheduled automatic backup
        PreSave,        // Before save operation
        PreModification // Before major modification
    };

    enum class RecoveryResult {
        Success,
        Failed,
        PartialSuccess,
        NoBackupFound,
        BackupCorrupted
    };

    struct BackupInfo {
        QString filePath;
        QString originalPath;
        BackupType type;
        QDateTime timestamp;
        qint64 fileSize;
        QString description;
        bool isValid;
        QString checksum;
        
        BackupInfo() : type(BackupType::Manual), fileSize(0), isValid(false) {}
    };

    explicit BackupManager(QObject* parent = nullptr);
    ~BackupManager();

    // Backup creation
    bool createBackup(const QString& filePath, BackupType type = BackupType::Manual, 
                     const QString& description = QString());
    bool createBackupFromData(const QByteArray& data, const QString& originalPath, 
                             BackupType type = BackupType::Manual, const QString& description = QString());
    
    // Automatic backup management
    void enableAutomaticBackup(const QString& filePath, int intervalMinutes = 5);
    void disableAutomaticBackup();
    bool isAutomaticBackupEnabled() const;
    int getAutomaticBackupInterval() const;
    void setAutomaticBackupInterval(int minutes);
    
    // Backup discovery and management
    QList<BackupInfo> findBackups(const QString& originalPath);
    QList<BackupInfo> getAllBackups();
    BackupInfo getLatestBackup(const QString& originalPath);
    BackupInfo getBackupByTimestamp(const QString& originalPath, const QDateTime& timestamp);
    
    // Recovery operations
    RecoveryResult restoreFromBackup(const QString& backupPath, const QString& targetPath = QString());
    RecoveryResult restoreLatestBackup(const QString& originalPath);
    RecoveryResult restoreBackupByTimestamp(const QString& originalPath, const QDateTime& timestamp);
    
    // Data integrity verification
    bool verifyBackupIntegrity(const QString& backupPath);
    bool verifyBackupIntegrity(const BackupInfo& backup);
    QStringList verifyAllBackups();
    
    // Backup cleanup
    void cleanupOldBackups(const QString& originalPath, int maxBackups = 10);
    void cleanupBackupsByAge(int maxDays = 30);
    void cleanupCorruptedBackups();
    qint64 calculateBackupStorageUsage();
    
    // Settings
    struct BackupSettings {
        bool enableAutomaticBackup = true;
        int automaticBackupInterval = 5; // minutes
        int maxBackupsPerFile = 10;
        int maxBackupAge = 30; // days
        qint64 maxBackupStorageSize = 1024 * 1024 * 1024; // 1GB
        QString backupDirectory;
        QString backupSuffix = ".bak";
        bool compressBackups = false;
        bool verifyIntegrityOnCreate = true;
        bool verifyIntegrityOnRestore = true;
    };
    
    BackupSettings getSettings() const;
    void setSettings(const BackupSettings& settings);
    void loadSettings();
    void saveSettings();
    
    // Backup directory management
    QString getBackupDirectory() const;
    bool setBackupDirectory(const QString& directory);
    bool ensureBackupDirectoryExists();
    
    // Backup naming and paths
    static QString generateBackupFileName(const QString& originalPath, BackupType type, 
                                        const QDateTime& timestamp = QDateTime::currentDateTime());
    static QString getBackupPath(const QString& originalPath, BackupType type, 
                               const QString& backupDirectory, const QDateTime& timestamp = QDateTime::currentDateTime());
    static BackupType parseBackupType(const QString& fileName);
    static QDateTime parseBackupTimestamp(const QString& fileName);

signals:
    void backupCreated(const BackupInfo& backup);
    void backupRestored(const QString& filePath, RecoveryResult result);
    void automaticBackupTriggered(const QString& filePath);
    void backupError(const QString& error);
    void storageWarning(qint64 currentUsage, qint64 maxUsage);

private slots:
    void onAutomaticBackupTimer();
    void onFileChanged(const QString& path);

private:
    BackupSettings m_settings;
    QTimer* m_automaticBackupTimer;
    QFileSystemWatcher* m_fileWatcher;
    QString m_watchedFilePath;
    
    // Internal methods
    QString calculateFileChecksum(const QString& filePath);
    QString calculateDataChecksum(const QByteArray& data);
    bool writeBackupMetadata(const BackupInfo& backup);
    BackupInfo readBackupMetadata(const QString& backupPath);
    
    // File operations
    bool copyFileWithVerification(const QString& source, const QString& destination);
    bool writeDataToFile(const QByteArray& data, const QString& filePath);
    QByteArray readFileData(const QString& filePath);
    
    // Backup validation
    bool validateBackupFile(const QString& backupPath);
    bool isBackupFile(const QString& filePath);
    
    // Cleanup helpers
    QList<BackupInfo> getBackupsForCleanup(const QString& originalPath, int maxBackups);
    QList<BackupInfo> getOldBackups(int maxDays);
    QList<BackupInfo> getCorruptedBackups();
    
    // Error handling
    void reportError(const QString& error);
    void reportWarning(const QString& warning);
    
    // Settings management
    void initializeSettings();
    QString getDefaultBackupDirectory();
};