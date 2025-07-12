#ifndef OTBBACKUP_H
#define OTBBACKUP_H

#include "otbtypes.h"
#include "otberrors.h"
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QObject>
#include <memory>

namespace OTB {

// Backup types for different scenarios
enum class BackupType {
    Manual = 0,           // User-initiated backup
    Automatic = 1,        // Automatic backup before modification
    Scheduled = 2,        // Time-based scheduled backup
    CrashRecovery = 3,    // Emergency backup during crash
    Checkpoint = 4        // Version checkpoint backup
};

// Backup compression levels
enum class CompressionLevel {
    None = 0,
    Fast = 1,
    Balanced = 2,
    Maximum = 3
};

// Recovery validation levels
enum class ValidationLevel {
    None = 0,           // No validation
    Basic = 1,          // Basic file integrity
    Structure = 2,      // OTB structure validation
    Complete = 3        // Full data validation
};

// Backup metadata structure
struct BackupMetadata {
    QString backupId;
    QString originalFilePath;
    QString backupFilePath;
    BackupType type;
    QDateTime createdAt;
    qint64 originalFileSize;
    qint64 backupFileSize;
    QString checksum;
    QString description;
    QString applicationVersion;
    QJsonObject additionalData;
    
    BackupMetadata() : type(BackupType::Manual), originalFileSize(0), backupFileSize(0) {}
    
    bool isValid() const;
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& json);
    QString toString() const;
};

// Backup configuration settings
struct BackupConfig {
    bool autoBackupEnabled = true;
    bool crashRecoveryEnabled = true;
    bool scheduledBackupEnabled = false;
    int maxBackupCount = 10;
    int maxBackupAgeDays = 30;
    qint64 maxTotalBackupSize = 1024 * 1024 * 1024; // 1GB
    CompressionLevel compressionLevel = CompressionLevel::Balanced;
    ValidationLevel validationLevel = ValidationLevel::Structure;
    QString backupDirectory;
    int scheduledBackupIntervalHours = 24;
    bool preserveBackupOnExit = true;
    bool verifyBackupIntegrity = true;
    
    BackupConfig();
    bool isValid() const;
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& json);
};

// Backup operation result
struct BackupResult {
    bool success = false;
    QString backupId;
    QString backupPath;
    QString errorMessage;
    qint64 processingTimeMs = 0;
    qint64 backupSize = 0;
    
    BackupResult() = default;
    BackupResult(bool success, const QString& message = QString()) 
        : success(success), errorMessage(message) {}
};

// Recovery operation result
struct RecoveryResult {
    bool success = false;
    QString restoredFilePath;
    QString errorMessage;
    qint64 processingTimeMs = 0;
    ValidationLevel validationPerformed = ValidationLevel::None;
    QStringList warnings;
    
    RecoveryResult() = default;
    RecoveryResult(bool success, const QString& message = QString()) 
        : success(success), errorMessage(message) {}
};

// Main backup and recovery system class
class OtbBackupSystem : public QObject {
    Q_OBJECT
    
public:
    explicit OtbBackupSystem(QObject* parent = nullptr);
    ~OtbBackupSystem();
    
    // Configuration management
    bool loadConfiguration(const QString& configPath = QString());
    bool saveConfiguration(const QString& configPath = QString()) const;
    void setConfiguration(const BackupConfig& config);
    const BackupConfig& getConfiguration() const { return m_config; }
    
    // Backup operations
    BackupResult createBackup(const QString& filePath, BackupType type = BackupType::Manual,
                             const QString& description = QString());
    BackupResult createAutomaticBackup(const QString& filePath);
    BackupResult createCrashRecoveryBackup(const QString& filePath);
    
    // Recovery operations
    RecoveryResult restoreFromBackup(const QString& backupId, const QString& targetPath = QString());
    RecoveryResult restoreLatestBackup(const QString& originalFilePath, const QString& targetPath = QString());
    RecoveryResult performCrashRecovery(const QString& originalFilePath);
    
    // Backup management
    QList<BackupMetadata> listBackups(const QString& originalFilePath = QString()) const;
    QList<BackupMetadata> listBackupsByType(BackupType type) const;
    bool deleteBackup(const QString& backupId);
    bool deleteOldBackups(int maxAge = -1);
    bool cleanupBackups();
    
    // Validation and verification
    bool validateBackup(const QString& backupId, ValidationLevel level = ValidationLevel::Structure);
    bool verifyBackupIntegrity(const QString& backupId);
    QString calculateFileChecksum(const QString& filePath) const;
    
    // Crash recovery detection
    bool hasCrashRecoveryData(const QString& filePath) const;
    QStringList getCrashRecoveryBackups(const QString& filePath) const;
    void clearCrashRecoveryData(const QString& filePath);
    
    // Statistics and monitoring
    qint64 getTotalBackupSize() const;
    int getBackupCount() const;
    QDateTime getLastBackupTime() const;
    QStringList getBackupStatistics() const;
    
    // Utility methods
    QString generateBackupId() const;
    QString getDefaultBackupDirectory() const;
    bool isBackupDirectoryValid(const QString& directory) const;
    
    // Event callbacks (for UI integration)
    void setProgressCallback(std::function<void(int)> callback) { m_progressCallback = callback; }
    void setStatusCallback(std::function<void(const QString&)> callback) { m_statusCallback = callback; }

private:
    // Internal backup operations
    bool performBackup(const QString& sourceFile, const QString& backupFile, 
                      const BackupMetadata& metadata);
    bool performRecovery(const QString& backupFile, const QString& targetFile,
                        const BackupMetadata& metadata);
    
    // Compression and decompression
    bool compressFile(const QString& sourceFile, const QString& targetFile) const;
    bool decompressFile(const QString& sourceFile, const QString& targetFile) const;
    
    // Metadata management
    bool saveMetadata(const BackupMetadata& metadata);
    bool loadMetadata(const QString& backupId, BackupMetadata& metadata) const;
    bool deleteMetadata(const QString& backupId);
    QString getMetadataFilePath(const QString& backupId) const;
    
    // Directory and file management
    bool ensureBackupDirectory();
    QString generateBackupFileName(const QString& originalPath, BackupType type) const;
    QString getBackupFilePath(const QString& backupId) const;
    
    // Validation helpers
    bool validateOtbFile(const QString& filePath, ValidationLevel level) const;
    bool validateFileIntegrity(const QString& filePath, const QString& expectedChecksum) const;
    
    // Cleanup and maintenance
    void performMaintenanceCleanup();
    bool isBackupExpired(const BackupMetadata& metadata) const;
    void updateProgress(int percentage);
    void updateStatus(const QString& status);
    
    BackupConfig m_config;
    QString m_configFilePath;
    QString m_backupDirectory;
    mutable QMutex m_mutex;
    
    // Callbacks for UI integration
    std::function<void(int)> m_progressCallback;
    std::function<void(const QString&)> m_statusCallback;
    
    // Internal state
    QTimer* m_scheduledBackupTimer;
    QDateTime m_lastBackupTime;
    
    // Constants
    static const QString BACKUP_METADATA_EXTENSION;
    static const QString BACKUP_FILE_EXTENSION;
    static const QString CONFIG_FILE_NAME;
    static const QString CRASH_RECOVERY_PREFIX;
};

// Utility functions for backup operations
namespace BackupUtils {
    // File operations
    bool copyFileWithProgress(const QString& source, const QString& destination,
                             std::function<void(int)> progressCallback = nullptr);
    bool moveFileAtomic(const QString& source, const QString& destination);
    
    // Checksum calculation
    QString calculateMD5(const QString& filePath);
    QString calculateSHA256(const QString& filePath);
    
    // Directory operations
    bool createDirectoryRecursive(const QString& path);
    qint64 getDirectorySize(const QString& path);
    bool cleanDirectory(const QString& path, int maxAgeDays = -1);
    
    // Validation helpers
    bool isValidOtbFile(const QString& filePath);
    bool compareFiles(const QString& file1, const QString& file2);
    
    // Compression helpers
    bool isCompressionBeneficial(const QString& filePath, qint64 threshold = 1024);
    CompressionLevel getOptimalCompressionLevel(qint64 fileSize);
    
    // Error handling
    QString formatBackupError(const QString& operation, const QString& details);
    void logBackupOperation(const QString& operation, bool success, const QString& details = QString());
}

// Global backup system instance (singleton pattern)
class GlobalBackupSystem {
public:
    static OtbBackupSystem& instance();
    static void initialize(const QString& configPath = QString());
    static void shutdown();
    
private:
    static std::unique_ptr<OtbBackupSystem> s_instance;
    static QMutex s_mutex;
};

} // namespace OTB

#endif // OTBBACKUP_H