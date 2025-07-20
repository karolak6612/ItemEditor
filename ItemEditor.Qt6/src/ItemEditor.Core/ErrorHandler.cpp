#include "ErrorHandler.h"
#include <QDir>
#include <QStandardPaths>
#include <QMutexLocker>
#include <QDebug>
#include <QCoreApplication>

// Global error handler instance
ErrorHandler* g_errorHandler = nullptr;

ErrorHandler::ErrorHandler(QObject* parent)
    : QObject(parent)
    , m_loggingEnabled(false)
    , m_logFile(nullptr)
    , m_logStream(nullptr)
    , m_logLevel(ErrorLevel::Warning)
{
    // Set as global instance if not already set
    if (!g_errorHandler) {
        g_errorHandler = this;
    }
}

ErrorHandler::~ErrorHandler()
{
    cleanupLogging();
    
    // Clear global instance if this is it
    if (g_errorHandler == this) {
        g_errorHandler = nullptr;
    }
}

void ErrorHandler::reportError(ErrorLevel level, ErrorCategory category, const QString& message, 
                              const QString& source, const QString& details)
{
    ErrorInfo error;
    error.level = level;
    error.category = category;
    error.message = message;
    error.source = source;
    error.details = details;
    error.timestamp = QDateTime::currentDateTime();
    error.errorCode = getErrorCodeForCategory(category);
    error.suggestedAction = getRecoverySuggestion(category, message);
    
    reportError(error);
}

void ErrorHandler::reportError(const ErrorInfo& error)
{
    QMutexLocker locker(&m_mutex);
    
    m_errors.append(error);
    
    // Write to log if enabled
    if (m_loggingEnabled && error.level >= m_logLevel) {
        writeToLog(error);
    }
    
    // Emit signals
    emit errorReported(error);
    
    if (error.level >= ErrorLevel::Critical) {
        emit criticalErrorReported(error);
    }
    
    emit errorCountChanged(m_errors.size());
    
    // Debug output
    qDebug() << QString("[%1] %2: %3")
                .arg(levelToString(error.level))
                .arg(categoryToString(error.category))
                .arg(error.message);
    
    if (!error.details.isEmpty()) {
        qDebug() << "Details:" << error.details;
    }
}

void ErrorHandler::reportFileError(const QString& message, const QString& filePath, const QString& details)
{
    QString fullMessage = message;
    if (!filePath.isEmpty()) {
        fullMessage += QString(" (File: %1)").arg(filePath);
    }
    
    reportError(ErrorLevel::Error, ErrorCategory::FileIO, fullMessage, "FileSystem", details);
}

void ErrorHandler::reportValidationError(const QString& message, const QString& details)
{
    reportError(ErrorLevel::Warning, ErrorCategory::DataValidation, message, "Validator", details);
}

void ErrorHandler::reportPluginError(const QString& message, const QString& pluginName)
{
    QString fullMessage = message;
    if (!pluginName.isEmpty()) {
        fullMessage += QString(" (Plugin: %1)").arg(pluginName);
    }
    
    reportError(ErrorLevel::Error, ErrorCategory::PluginSystem, fullMessage, "PluginManager");
}

void ErrorHandler::reportSystemError(const QString& message, int errorCode)
{
    ErrorInfo error;
    error.level = ErrorLevel::Critical;
    error.category = ErrorCategory::System;
    error.message = message;
    error.source = "System";
    error.timestamp = QDateTime::currentDateTime();
    error.errorCode = errorCode;
    error.suggestedAction = getRecoverySuggestion(ErrorCategory::System, message);
    
    reportError(error);
}

QList<ErrorHandler::ErrorInfo> ErrorHandler::getAllErrors() const
{
    QMutexLocker locker(&m_mutex);
    return m_errors;
}

QList<ErrorHandler::ErrorInfo> ErrorHandler::getErrorsByLevel(ErrorLevel level) const
{
    QMutexLocker locker(&m_mutex);
    QList<ErrorInfo> filtered;
    
    for (const ErrorInfo& error : m_errors) {
        if (error.level == level) {
            filtered.append(error);
        }
    }
    
    return filtered;
}

QList<ErrorHandler::ErrorInfo> ErrorHandler::getErrorsByCategory(ErrorCategory category) const
{
    QMutexLocker locker(&m_mutex);
    QList<ErrorInfo> filtered;
    
    for (const ErrorInfo& error : m_errors) {
        if (error.category == category) {
            filtered.append(error);
        }
    }
    
    return filtered;
}

QList<ErrorHandler::ErrorInfo> ErrorHandler::getRecentErrors(int count) const
{
    QMutexLocker locker(&m_mutex);
    
    if (count >= m_errors.size()) {
        return m_errors;
    }
    
    return m_errors.mid(m_errors.size() - count);
}

QString ErrorHandler::getLastErrorMessage() const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_errors.isEmpty()) {
        return QString();
    }
    
    return m_errors.last().message;
}

ErrorHandler::ErrorInfo ErrorHandler::getLastError() const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_errors.isEmpty()) {
        return ErrorInfo();
    }
    
    return m_errors.last();
}

void ErrorHandler::clearErrors()
{
    QMutexLocker locker(&m_mutex);
    m_errors.clear();
    emit errorCountChanged(0);
}

void ErrorHandler::clearErrorsByLevel(ErrorLevel level)
{
    QMutexLocker locker(&m_mutex);
    
    auto it = std::remove_if(m_errors.begin(), m_errors.end(),
        [level](const ErrorInfo& error) { return error.level == level; });
    
    m_errors.erase(it, m_errors.end());
    emit errorCountChanged(m_errors.size());
}

void ErrorHandler::clearErrorsByCategory(ErrorCategory category)
{
    QMutexLocker locker(&m_mutex);
    
    auto it = std::remove_if(m_errors.begin(), m_errors.end(),
        [category](const ErrorInfo& error) { return error.category == category; });
    
    m_errors.erase(it, m_errors.end());
    emit errorCountChanged(m_errors.size());
}

int ErrorHandler::getErrorCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_errors.size();
}

int ErrorHandler::getErrorCount(ErrorLevel level) const
{
    QMutexLocker locker(&m_mutex);
    
    int count = 0;
    for (const ErrorInfo& error : m_errors) {
        if (error.level == level) {
            count++;
        }
    }
    
    return count;
}

int ErrorHandler::getErrorCount(ErrorCategory category) const
{
    QMutexLocker locker(&m_mutex);
    
    int count = 0;
    for (const ErrorInfo& error : m_errors) {
        if (error.category == category) {
            count++;
        }
    }
    
    return count;
}

bool ErrorHandler::hasErrors() const
{
    QMutexLocker locker(&m_mutex);
    return !m_errors.isEmpty();
}

bool ErrorHandler::hasErrorsOfLevel(ErrorLevel level) const
{
    QMutexLocker locker(&m_mutex);
    
    for (const ErrorInfo& error : m_errors) {
        if (error.level == level) {
            return true;
        }
    }
    
    return false;
}

bool ErrorHandler::hasCriticalErrors() const
{
    return hasErrorsOfLevel(ErrorLevel::Critical) || hasErrorsOfLevel(ErrorLevel::Fatal);
}

bool ErrorHandler::enableLogging(const QString& logFilePath)
{
    cleanupLogging();
    
    m_logFilePath = logFilePath;
    
    // Ensure directory exists
    QFileInfo fileInfo(m_logFilePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists() && !dir.mkpath(".")) {
        qWarning() << "Failed to create log directory:" << dir.absolutePath();
        return false;
    }
    
    m_logFile = new QFile(m_logFilePath, this);
    if (!m_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
        qWarning() << "Failed to open log file:" << m_logFilePath;
        delete m_logFile;
        m_logFile = nullptr;
        return false;
    }
    
    m_logStream = new QTextStream(m_logFile);
    m_logStream->setEncoding(QStringConverter::Utf8);
    
    m_loggingEnabled = true;
    
    // Write session start marker
    *m_logStream << QString("\n=== ItemEditor Session Started: %1 ===\n")
                    .arg(QDateTime::currentDateTime().toString(Qt::ISODate));
    m_logStream->flush();
    
    return true;
}

void ErrorHandler::disableLogging()
{
    if (m_loggingEnabled && m_logStream) {
        // Write session end marker
        *m_logStream << QString("=== ItemEditor Session Ended: %1 ===\n\n")
                        .arg(QDateTime::currentDateTime().toString(Qt::ISODate));
        m_logStream->flush();
    }
    
    cleanupLogging();
}

bool ErrorHandler::isLoggingEnabled() const
{
    return m_loggingEnabled;
}

QString ErrorHandler::getLogFilePath() const
{
    return m_logFilePath;
}

void ErrorHandler::setLogLevel(ErrorLevel minLevel)
{
    m_logLevel = minLevel;
}

ErrorHandler::ErrorLevel ErrorHandler::getLogLevel() const
{
    return m_logLevel;
}

QString ErrorHandler::formatError(const ErrorInfo& error)
{
    QString formatted = QString("[%1] %2 - %3: %4")
        .arg(error.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
        .arg(levelToString(error.level))
        .arg(categoryToString(error.category))
        .arg(error.message);
    
    if (!error.source.isEmpty()) {
        formatted += QString(" (Source: %1)").arg(error.source);
    }
    
    if (!error.details.isEmpty()) {
        formatted += QString("\n  Details: %1").arg(error.details);
    }
    
    if (!error.suggestedAction.isEmpty()) {
        formatted += QString("\n  Suggestion: %1").arg(error.suggestedAction);
    }
    
    return formatted;
}

QString ErrorHandler::formatErrorList(const QList<ErrorInfo>& errors)
{
    QStringList formatted;
    
    for (const ErrorInfo& error : errors) {
        formatted.append(formatError(error));
    }
    
    return formatted.join("\n\n");
}

QString ErrorHandler::levelToString(ErrorLevel level)
{
    switch (level) {
        case ErrorLevel::Info: return "INFO";
        case ErrorLevel::Warning: return "WARNING";
        case ErrorLevel::Error: return "ERROR";
        case ErrorLevel::Critical: return "CRITICAL";
        case ErrorLevel::Fatal: return "FATAL";
        default: return "UNKNOWN";
    }
}

QString ErrorHandler::categoryToString(ErrorCategory category)
{
    switch (category) {
        case ErrorCategory::FileIO: return "File I/O";
        case ErrorCategory::DataValidation: return "Data Validation";
        case ErrorCategory::PluginSystem: return "Plugin System";
        case ErrorCategory::UserInterface: return "User Interface";
        case ErrorCategory::System: return "System";
        case ErrorCategory::Unknown: return "Unknown";
        default: return "Unknown";
    }
}

QString ErrorHandler::getRecoverySuggestion(ErrorCategory category, const QString& message)
{
    // Provide specific suggestions based on common error patterns
    QString lowerMessage = message.toLower();
    
    if (category == ErrorCategory::FileIO) {
        if (lowerMessage.contains("permission") || lowerMessage.contains("access denied")) {
            return "Check file permissions and ensure the file is not in use by another application.";
        } else if (lowerMessage.contains("not found") || lowerMessage.contains("does not exist")) {
            return "Verify the file path is correct and the file exists.";
        } else if (lowerMessage.contains("corrupt") || lowerMessage.contains("invalid format")) {
            return "The file may be corrupted. Try opening a backup copy or re-downloading the file.";
        } else if (lowerMessage.contains("disk") || lowerMessage.contains("space")) {
            return "Check available disk space and try freeing up storage.";
        }
        return "Check file path, permissions, and ensure the file is not corrupted.";
    }
    
    if (category == ErrorCategory::DataValidation) {
        if (lowerMessage.contains("range") || lowerMessage.contains("bounds")) {
            return "Ensure all values are within the valid range for this item type.";
        } else if (lowerMessage.contains("duplicate")) {
            return "Check for duplicate IDs and resolve conflicts.";
        }
        return "Review the data for consistency and correct any validation errors.";
    }
    
    if (category == ErrorCategory::PluginSystem) {
        if (lowerMessage.contains("not found") || lowerMessage.contains("missing")) {
            return "Ensure all required plugin files are present in the plugins directory.";
        } else if (lowerMessage.contains("version") || lowerMessage.contains("compatibility")) {
            return "Check plugin version compatibility and update if necessary.";
        }
        return "Try reloading plugins or reinstalling the plugin system.";
    }
    
    return getDefaultSuggestion(category);
}

QStringList ErrorHandler::getCommonSolutions(ErrorCategory category)
{
    switch (category) {
        case ErrorCategory::FileIO:
            return {
                "Check file permissions",
                "Ensure file is not in use by another application",
                "Verify file path is correct",
                "Check available disk space",
                "Try running as administrator"
            };
            
        case ErrorCategory::DataValidation:
            return {
                "Review item properties for valid ranges",
                "Check for duplicate item IDs",
                "Validate client data consistency",
                "Ensure all required fields are filled",
                "Check item type compatibility"
            };
            
        case ErrorCategory::PluginSystem:
            return {
                "Reload plugins (F5)",
                "Check plugin directory permissions",
                "Verify plugin file integrity",
                "Update plugin versions",
                "Restart the application"
            };
            
        case ErrorCategory::System:
            return {
                "Restart the application",
                "Check system resources",
                "Update application to latest version",
                "Check Windows compatibility",
                "Contact technical support"
            };
            
        default:
            return {
                "Restart the application",
                "Check system resources",
                "Review recent changes",
                "Contact technical support"
            };
    }
}

void ErrorHandler::writeToLog(const ErrorInfo& error)
{
    if (!m_logStream) {
        return;
    }
    
    *m_logStream << formatError(error) << "\n";
    m_logStream->flush();
}

void ErrorHandler::initializeLogging()
{
    // Default log file location
    QString defaultLogPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(defaultLogPath);
    defaultLogPath = QDir(defaultLogPath).filePath("ItemEditor.log");
    
    enableLogging(defaultLogPath);
}

void ErrorHandler::cleanupLogging()
{
    m_loggingEnabled = false;
    
    if (m_logStream) {
        delete m_logStream;
        m_logStream = nullptr;
    }
    
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }
}

QString ErrorHandler::generateErrorId(const ErrorInfo& error) const
{
    return QString("%1_%2_%3")
        .arg(static_cast<int>(error.category))
        .arg(static_cast<int>(error.level))
        .arg(error.timestamp.toMSecsSinceEpoch());
}

int ErrorHandler::getErrorCodeForCategory(ErrorCategory category)
{
    switch (category) {
        case ErrorCategory::FileIO: return 1000;
        case ErrorCategory::DataValidation: return 2000;
        case ErrorCategory::PluginSystem: return 3000;
        case ErrorCategory::UserInterface: return 4000;
        case ErrorCategory::System: return 5000;
        default: return 9000;
    }
}

QString ErrorHandler::getDefaultSuggestion(ErrorCategory category)
{
    switch (category) {
        case ErrorCategory::FileIO:
            return "Check file permissions and path.";
        case ErrorCategory::DataValidation:
            return "Review and correct the data.";
        case ErrorCategory::PluginSystem:
            return "Try reloading plugins.";
        case ErrorCategory::UserInterface:
            return "Restart the application.";
        case ErrorCategory::System:
            return "Check system resources and restart.";
        default:
            return "Contact technical support if the problem persists.";
    }
}