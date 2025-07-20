#include "OtbFileManager.h"
#include "OtbFileValidator.h"
#include "ErrorHandler.h"
#include "BackupManager.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QDebug>
#include <QStandardPaths>
#include <QSettings>

OtbFileManager::OtbFileManager(QObject* parent)
    : QObject(parent)
    , m_isModified(false)
    , m_qsettings(nullptr)
    , m_autoSaveTimer(nullptr)
    , m_backupManager(nullptr)
{
    m_qsettings = new QSettings(this);
    m_backupManager = new BackupManager(this);
    
    loadSettings();
    loadRecentFiles();
    setupAutoSave();
    
    // Connect reader/writer progress callbacks
    m_reader.setProgressCallback([this](int current, int total, const QString& status) {
        reportProgress(current, total, status);
    });
    
    m_writer.setProgressCallback([this](int current, int total, const QString& status) {
        reportProgress(current, total, status);
    });
}

OtbFileManager::~OtbFileManager()
{
    saveSettings();
    saveRecentFiles();
}

bool OtbFileManager::openFile(const QString& filePath)
{
    clearErrors();
    
    QString normalizedPath = normalizeFilePath(filePath);
    
    if (!isValidOtbFilePath(normalizedPath)) {
        addError(QString("Invalid file path: %1").arg(filePath));
        return false;
    }
    
    if (!QFile::exists(normalizedPath)) {
        addError(QString("File does not exist: %1").arg(normalizedPath));
        return false;
    }
    
    // Validate file before opening if enabled
    if (m_settings.validateOnOpen && !performFileValidation(normalizedPath)) {
        return false;
    }
    
    reportProgress(0, 100, "Opening file...");
    
    // Read the file
    if (!m_reader.readFile(normalizedPath)) {
        addError(QString("Failed to read file: %1").arg(m_reader.getLastError()));
        return false;
    }
    
    // Get the data
    m_items = m_reader.getItems();
    m_currentFilePath = normalizedPath;
    m_isModified = false;
    
    // Update recent files
    updateRecentFiles(normalizedPath);
    
    // Enable automatic backup if configured
    if (m_settings.autoSaveEnabled && m_backupManager) {
        m_backupManager->enableAutomaticBackup(normalizedPath, m_settings.autoSaveInterval / 60); // Convert seconds to minutes
    }
    
    reportProgress(100, 100, "File opened successfully");
    
    emit fileOpened(normalizedPath);
    emit fileModified(false);
    
    return true;
}

bool OtbFileManager::saveFile()
{
    if (m_currentFilePath.isEmpty()) {
        addError("No file is currently open");
        return false;
    }
    
    return saveFileAs(m_currentFilePath);
}

bool OtbFileManager::saveFileAs(const QString& filePath)
{
    clearErrors();
    
    QString normalizedPath = normalizeFilePath(filePath);
    
    if (!isValidOtbFilePath(normalizedPath)) {
        addError(QString("Invalid file path: %1").arg(filePath));
        return false;
    }
    
    // Validate data before saving if enabled
    if (m_settings.validateOnSave && !m_items.validateCollection()) {
        QStringList validationErrors = m_items.getValidationErrors();
        for (const QString& error : validationErrors) {
            addError(QString("Validation error: %1").arg(error));
        }
        return false;
    }
    
    reportProgress(0, 100, "Saving file...");
    
    // Create backup before save if enabled
    if (m_settings.createBackupOnSave && m_backupManager) {
        m_backupManager->createBackup(normalizedPath, BackupManager::BackupType::PreSave, "Before save operation");
    }
    
    // Configure writer options
    OtbWriter::WriteOptions options;
    options.createBackup = false; // We handle backup creation above
    options.validateItems = m_settings.validateOnSave;
    
    // Write the file
    if (!m_writer.writeFile(normalizedPath, m_items, options)) {
        addError(QString("Failed to write file: %1").arg(m_writer.getLastError()));
        return false;
    }
    
    // Verify data integrity after save
    reportProgress(95, 100, "Verifying data integrity...");
    
    OtbFileValidator validator;
    OtbFileValidator::ValidationResult verificationResult = validator.validateFile(normalizedPath, OtbFileValidator::ValidationLevel::Standard);
    
    if (!verificationResult.isValid) {
        addError("Data integrity verification failed after save");
        for (const QString& error : verificationResult.errors) {
            addError(QString("Verification error: %1").arg(error));
        }
        return false;
    }
    
    // Update state
    m_currentFilePath = normalizedPath;
    m_isModified = false;
    
    // Update recent files
    updateRecentFiles(normalizedPath);
    
    reportProgress(100, 100, "File saved successfully");
    
    emit fileSaved(normalizedPath);
    emit fileModified(false);
    
    return true;
}

bool OtbFileManager::closeFile()
{
    if (!hasOpenFile()) {
        return true;
    }
    
    resetFileState();
    
    emit fileClosed();
    
    return true;
}

bool OtbFileManager::hasOpenFile() const
{
    return !m_currentFilePath.isEmpty();
}

QString OtbFileManager::getCurrentFilePath() const
{
    return m_currentFilePath;
}

QString OtbFileManager::getCurrentFileName() const
{
    if (m_currentFilePath.isEmpty()) {
        return QString();
    }
    
    return QFileInfo(m_currentFilePath).fileName();
}

bool OtbFileManager::isModified() const
{
    return m_isModified;
}

void OtbFileManager::setModified(bool modified)
{
    if (m_isModified != modified) {
        m_isModified = modified;
        emit fileModified(modified);
    }
}

ServerItemList& OtbFileManager::getItems()
{
    return m_items;
}

const ServerItemList& OtbFileManager::getItems() const
{
    return m_items;
}

VersionInfo OtbFileManager::getVersionInfo() const
{
    return m_items.versionInfo;
}

ItemRange OtbFileManager::getItemRange() const
{
    return m_items.itemRange;
}

QStringList OtbFileManager::getRecentFiles() const
{
    return m_recentFiles;
}

void OtbFileManager::addRecentFile(const QString& filePath)
{
    updateRecentFiles(filePath);
}

void OtbFileManager::removeRecentFile(const QString& filePath)
{
    QString normalizedPath = normalizeFilePath(filePath);
    
    if (m_recentFiles.removeAll(normalizedPath) > 0) {
        saveRecentFiles();
        emit recentFilesChanged();
    }
}

void OtbFileManager::clearRecentFiles()
{
    if (!m_recentFiles.isEmpty()) {
        m_recentFiles.clear();
        saveRecentFiles();
        emit recentFilesChanged();
    }
}

bool OtbFileManager::validateFile(const QString& filePath)
{
    return performFileValidation(filePath);
}

bool OtbFileManager::validateCurrentFile()
{
    if (!hasOpenFile()) {
        addError("No file is currently open");
        return false;
    }
    
    return m_items.validateCollection();
}

QStringList OtbFileManager::getValidationErrors() const
{
    if (hasOpenFile()) {
        return m_items.getValidationErrors();
    }
    
    return QStringList();
}

bool OtbFileManager::createBackup(const QString& backupPath)
{
    if (!hasOpenFile()) {
        addError("No file is currently open");
        return false;
    }
    
    if (backupPath.isEmpty()) {
        return m_backupManager->createBackup(m_currentFilePath, BackupManager::BackupType::Manual);
    } else {
        // Create backup with specific path
        return m_backupManager->createBackup(m_currentFilePath, BackupManager::BackupType::Manual, 
                                           QString("Manual backup to %1").arg(backupPath));
    }
}

bool OtbFileManager::restoreFromBackup(const QString& backupPath)
{
    if (!hasOpenFile()) {
        addError("No file is currently open");
        return false;
    }
    
    BackupManager::RecoveryResult result;
    
    if (backupPath.isEmpty()) {
        result = m_backupManager->restoreLatestBackup(m_currentFilePath);
    } else {
        result = m_backupManager->restoreFromBackup(backupPath, m_currentFilePath);
    }
    
    if (result != BackupManager::RecoveryResult::Success) {
        QString errorMsg;
        switch (result) {
            case BackupManager::RecoveryResult::NoBackupFound:
                errorMsg = "No backup found for restoration";
                break;
            case BackupManager::RecoveryResult::BackupCorrupted:
                errorMsg = "Backup file is corrupted";
                break;
            case BackupManager::RecoveryResult::Failed:
                errorMsg = "Failed to restore from backup";
                break;
            default:
                errorMsg = "Unknown error during backup restoration";
                break;
        }
        addError(errorMsg);
        return false;
    }
    
    // Reload the file
    return openFile(m_currentFilePath);
}

QString OtbFileManager::getBackupPath() const
{
    if (!hasOpenFile()) {
        return QString();
    }
    
    BackupManager::BackupInfo latestBackup = m_backupManager->getLatestBackup(m_currentFilePath);
    return latestBackup.filePath;
}

bool OtbFileManager::hasBackup() const
{
    if (!hasOpenFile()) {
        return false;
    }
    
    QList<BackupManager::BackupInfo> backups = m_backupManager->findBackups(m_currentFilePath);
    return !backups.isEmpty();
}

OtbFileManager::FileInfo OtbFileManager::getFileInfo() const
{
    if (!hasOpenFile()) {
        return FileInfo();
    }
    
    return getFileInfo(m_currentFilePath);
}

OtbFileManager::FileInfo OtbFileManager::getFileInfo(const QString& filePath)
{
    FileInfo info;
    info.filePath = filePath;
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        info.isValid = false;
        info.errors.append("File does not exist");
        return info;
    }
    
    info.fileName = fileInfo.fileName();
    info.fileSize = fileInfo.size();
    info.lastModified = fileInfo.lastModified();
    
    // Try to read version info
    info.versionInfo = OtbReader::readVersionInfo(filePath);
    
    // Validate file
    if (OtbReader::isValidOtbFile(filePath)) {
        info.isValid = true;
        
        // Try to get more detailed info
        OtbReader reader;
        if (reader.readFile(filePath)) {
            ServerItemList items = reader.getItems();
            info.itemRange = items.itemRange;
            info.itemCount = items.size();
        } else {
            info.errors = reader.getAllErrors();
        }
    } else {
        info.isValid = false;
        info.errors.append("Invalid OTB file format");
    }
    
    return info;
}

void OtbFileManager::setProgressCallback(ProgressCallback callback)
{
    m_progressCallback = callback;
}

bool OtbFileManager::hasError() const
{
    return !m_errors.isEmpty();
}

QString OtbFileManager::getLastError() const
{
    return m_lastError;
}

QStringList OtbFileManager::getAllErrors() const
{
    return m_errors;
}

void OtbFileManager::clearErrors()
{
    m_errors.clear();
    m_lastError.clear();
}

OtbFileManager::Settings OtbFileManager::getSettings() const
{
    return m_settings;
}

void OtbFileManager::setSettings(const Settings& settings)
{
    m_settings = settings;
    setupAutoSave();
    saveSettings();
}

void OtbFileManager::loadSettings()
{
    m_settings.createBackupOnSave = m_qsettings->value("FileManager/createBackupOnSave", true).toBool();
    m_settings.validateOnOpen = m_qsettings->value("FileManager/validateOnOpen", true).toBool();
    m_settings.validateOnSave = m_qsettings->value("FileManager/validateOnSave", true).toBool();
    m_settings.autoSaveEnabled = m_qsettings->value("FileManager/autoSaveEnabled", false).toBool();
    m_settings.autoSaveInterval = m_qsettings->value("FileManager/autoSaveInterval", 300).toInt();
    m_settings.defaultDirectory = m_qsettings->value("FileManager/defaultDirectory", 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    m_settings.backupSuffix = m_qsettings->value("FileManager/backupSuffix", ".bak").toString();
}

void OtbFileManager::saveSettings()
{
    m_qsettings->setValue("FileManager/createBackupOnSave", m_settings.createBackupOnSave);
    m_qsettings->setValue("FileManager/validateOnOpen", m_settings.validateOnOpen);
    m_qsettings->setValue("FileManager/validateOnSave", m_settings.validateOnSave);
    m_qsettings->setValue("FileManager/autoSaveEnabled", m_settings.autoSaveEnabled);
    m_qsettings->setValue("FileManager/autoSaveInterval", m_settings.autoSaveInterval);
    m_qsettings->setValue("FileManager/defaultDirectory", m_settings.defaultDirectory);
    m_qsettings->setValue("FileManager/backupSuffix", m_settings.backupSuffix);
}

void OtbFileManager::onAutoSaveTimer()
{
    if (hasOpenFile() && isModified()) {
        QString autoSavePath = m_currentFilePath + ".autosave";
        
        // Save to auto-save file
        OtbWriter::WriteOptions options;
        options.createBackup = false; // Don't create backup for auto-save
        
        if (m_writer.writeFile(autoSavePath, m_items, options)) {
            qDebug() << "Auto-saved to:" << autoSavePath;
        } else {
            qWarning() << "Auto-save failed:" << m_writer.getLastError();
        }
    }
}

void OtbFileManager::addError(const QString& error)
{
    m_lastError = error;
    m_errors.append(error);
    qDebug() << "OtbFileManager Error:" << error;
    emit errorOccurred(error);
}

void OtbFileManager::reportProgress(int current, int total, const QString& status)
{
    if (m_progressCallback) {
        m_progressCallback(current, total, status);
    }
    
    emit progressChanged(current, total, status);
}

void OtbFileManager::updateRecentFiles(const QString& filePath)
{
    QString normalizedPath = normalizeFilePath(filePath);
    
    // Remove if already exists
    m_recentFiles.removeAll(normalizedPath);
    
    // Add to front
    m_recentFiles.prepend(normalizedPath);
    
    // Limit to maximum count
    while (m_recentFiles.size() > MaxRecentFiles) {
        m_recentFiles.removeLast();
    }
    
    saveRecentFiles();
    emit recentFilesChanged();
}

void OtbFileManager::loadRecentFiles()
{
    int size = m_qsettings->beginReadArray("RecentFiles");
    m_recentFiles.clear();
    
    for (int i = 0; i < size && i < MaxRecentFiles; ++i) {
        m_qsettings->setArrayIndex(i);
        QString filePath = m_qsettings->value("filePath").toString();
        
        // Only add if file still exists
        if (QFile::exists(filePath)) {
            m_recentFiles.append(filePath);
        }
    }
    
    m_qsettings->endArray();
}

void OtbFileManager::saveRecentFiles()
{
    m_qsettings->beginWriteArray("RecentFiles");
    
    for (int i = 0; i < m_recentFiles.size(); ++i) {
        m_qsettings->setArrayIndex(i);
        m_qsettings->setValue("filePath", m_recentFiles[i]);
    }
    
    m_qsettings->endArray();
}

bool OtbFileManager::performFileValidation(const QString& filePath)
{
    clearErrors();
    
    OtbFileValidator validator;
    OtbFileValidator::ValidationResult result = validator.validateFile(filePath, OtbFileValidator::ValidationLevel::Standard);
    
    if (!result.isValid) {
        for (const QString& error : result.errors) {
            addError(error);
        }
        return false;
    }
    
    // Add warnings to error list for user information
    for (const QString& warning : result.warnings) {
        addError(QString("Warning: %1").arg(warning));
    }
    
    return true;
}

void OtbFileManager::setupAutoSave()
{
    if (m_autoSaveTimer) {
        m_autoSaveTimer->stop();
        m_autoSaveTimer->deleteLater();
        m_autoSaveTimer = nullptr;
    }
    
    if (m_settings.autoSaveEnabled && m_settings.autoSaveInterval > 0) {
        m_autoSaveTimer = new QTimer(this);
        connect(m_autoSaveTimer, &QTimer::timeout, this, &OtbFileManager::onAutoSaveTimer);
        m_autoSaveTimer->start(m_settings.autoSaveInterval * 1000); // Convert to milliseconds
    }
}

void OtbFileManager::resetFileState()
{
    m_items.clear();
    m_currentFilePath.clear();
    m_isModified = false;
    clearErrors();
}

QString OtbFileManager::normalizeFilePath(const QString& filePath)
{
    return QDir::cleanPath(QFileInfo(filePath).absoluteFilePath());
}

bool OtbFileManager::isValidOtbFilePath(const QString& filePath)
{
    if (filePath.isEmpty()) {
        return false;
    }
    
    QFileInfo fileInfo(filePath);
    
    // Check extension
    if (!filePath.toLower().endsWith(".otb")) {
        return false;
    }
    
    // Check if path is valid
    return fileInfo.isAbsolute() || fileInfo.isRelative();
}