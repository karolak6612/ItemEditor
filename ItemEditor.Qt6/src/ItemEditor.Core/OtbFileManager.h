#pragma once

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QObject>
#include <QSettings>
#include <functional>
#include "ServerItemList.h"
#include "OtbReader.h"
#include "OtbWriter.h"

// Forward declarations
class BackupManager;

/**
 * @brief File operations manager for OTB files
 * 
 * Provides comprehensive file management functionality including
 * open, save, recent files, backup, and validation with identical
 * behavior to the legacy system.
 */
class OtbFileManager : public QObject
{
    Q_OBJECT

public:
    explicit OtbFileManager(QObject* parent = nullptr);
    ~OtbFileManager();

    // File operations
    bool openFile(const QString& filePath);
    bool saveFile();
    bool saveFileAs(const QString& filePath);
    bool closeFile();
    
    // File state
    bool hasOpenFile() const;
    QString getCurrentFilePath() const;
    QString getCurrentFileName() const;
    bool isModified() const;
    void setModified(bool modified = true);
    
    // Data access
    ServerItemList& getItems();
    const ServerItemList& getItems() const;
    VersionInfo getVersionInfo() const;
    ItemRange getItemRange() const;
    
    // Recent files management
    QStringList getRecentFiles() const;
    void addRecentFile(const QString& filePath);
    void removeRecentFile(const QString& filePath);
    void clearRecentFiles();
    static const int MaxRecentFiles = 10;
    
    // File validation
    bool validateFile(const QString& filePath);
    bool validateCurrentFile();
    QStringList getValidationErrors() const;
    
    // Backup and recovery
    bool createBackup(const QString& backupPath = QString());
    bool restoreFromBackup(const QString& backupPath = QString());
    QString getBackupPath() const;
    bool hasBackup() const;
    
    // File information
    struct FileInfo {
        QString filePath;
        QString fileName;
        qint64 fileSize;
        QDateTime lastModified;
        VersionInfo versionInfo;
        ItemRange itemRange;
        int itemCount;
        bool isValid;
        QStringList errors;
    };
    
    FileInfo getFileInfo() const;
    static FileInfo getFileInfo(const QString& filePath);
    
    // Progress and error handling
    using ProgressCallback = std::function<void(int current, int total, const QString& status)>;
    void setProgressCallback(ProgressCallback callback);
    
    bool hasError() const;
    QString getLastError() const;
    QStringList getAllErrors() const;
    void clearErrors();
    
    // Settings
    struct Settings {
        bool createBackupOnSave = true;
        bool validateOnOpen = true;
        bool validateOnSave = true;
        bool autoSaveEnabled = false;
        int autoSaveInterval = 300; // seconds
        QString defaultDirectory;
        QString backupSuffix = ".bak";
    };
    
    Settings getSettings() const;
    void setSettings(const Settings& settings);
    void loadSettings();
    void saveSettings();

signals:
    void fileOpened(const QString& filePath);
    void fileSaved(const QString& filePath);
    void fileClosed();
    void fileModified(bool modified);
    void errorOccurred(const QString& error);
    void progressChanged(int current, int total, const QString& status);
    void recentFilesChanged();

private slots:
    void onAutoSaveTimer();

private:
    // Core data
    ServerItemList m_items;
    QString m_currentFilePath;
    bool m_isModified;
    
    // File operations
    OtbReader m_reader;
    OtbWriter m_writer;
    BackupManager* m_backupManager;
    
    // Error handling
    QStringList m_errors;
    QString m_lastError;
    
    // Progress callback
    ProgressCallback m_progressCallback;
    
    // Recent files
    QStringList m_recentFiles;
    
    // Settings
    Settings m_settings;
    QSettings* m_qsettings;
    
    // Auto-save
    QTimer* m_autoSaveTimer;
    
    // Internal methods
    void addError(const QString& error);
    void reportProgress(int current, int total, const QString& status);
    void updateRecentFiles(const QString& filePath);
    void loadRecentFiles();
    void saveRecentFiles();
    bool performFileValidation(const QString& filePath);
    void setupAutoSave();
    void resetFileState();
    
    // File path utilities
    static QString normalizeFilePath(const QString& filePath);
    static bool isValidOtbFilePath(const QString& filePath);
};