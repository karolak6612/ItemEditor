#pragma once

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QObject>
#include <QTextStream>
#include <QFile>
#include <QMutex>

/**
 * @brief Centralized error handling and logging system
 * 
 * Provides identical error handling and logging functionality to the legacy system,
 * including error categorization, logging, and recovery suggestions.
 */
class ErrorHandler : public QObject
{
    Q_OBJECT

public:
    enum class ErrorLevel {
        Info,
        Warning,
        Error,
        Critical,
        Fatal
    };

    enum class ErrorCategory {
        FileIO,
        DataValidation,
        PluginSystem,
        UserInterface,
        System,
        Unknown
    };

    struct ErrorInfo {
        ErrorLevel level;
        ErrorCategory category;
        QString message;
        QString details;
        QString source;
        QDateTime timestamp;
        QString suggestedAction;
        int errorCode;
        
        ErrorInfo() : level(ErrorLevel::Error), category(ErrorCategory::Unknown), errorCode(0) {}
    };

    explicit ErrorHandler(QObject* parent = nullptr);
    ~ErrorHandler();

    // Error reporting
    void reportError(ErrorLevel level, ErrorCategory category, const QString& message, 
                    const QString& source = QString(), const QString& details = QString());
    void reportError(const ErrorInfo& error);
    
    // Convenience methods matching legacy system
    void reportFileError(const QString& message, const QString& filePath = QString(), 
                        const QString& details = QString());
    void reportValidationError(const QString& message, const QString& details = QString());
    void reportPluginError(const QString& message, const QString& pluginName = QString());
    void reportSystemError(const QString& message, int errorCode = 0);
    
    // Error retrieval
    QList<ErrorInfo> getAllErrors() const;
    QList<ErrorInfo> getErrorsByLevel(ErrorLevel level) const;
    QList<ErrorInfo> getErrorsByCategory(ErrorCategory category) const;
    QList<ErrorInfo> getRecentErrors(int count = 10) const;
    
    QString getLastErrorMessage() const;
    ErrorInfo getLastError() const;
    
    // Error management
    void clearErrors();
    void clearErrorsByLevel(ErrorLevel level);
    void clearErrorsByCategory(ErrorCategory category);
    
    // Error statistics
    int getErrorCount() const;
    int getErrorCount(ErrorLevel level) const;
    int getErrorCount(ErrorCategory category) const;
    
    bool hasErrors() const;
    bool hasErrorsOfLevel(ErrorLevel level) const;
    bool hasCriticalErrors() const;
    
    // Logging
    bool enableLogging(const QString& logFilePath);
    void disableLogging();
    bool isLoggingEnabled() const;
    QString getLogFilePath() const;
    
    void setLogLevel(ErrorLevel minLevel);
    ErrorLevel getLogLevel() const;
    
    // Error formatting
    static QString formatError(const ErrorInfo& error);
    static QString formatErrorList(const QList<ErrorInfo>& errors);
    static QString levelToString(ErrorLevel level);
    static QString categoryToString(ErrorCategory category);
    
    // Recovery suggestions
    static QString getRecoverySuggestion(ErrorCategory category, const QString& message);
    static QStringList getCommonSolutions(ErrorCategory category);

signals:
    void errorReported(const ErrorInfo& error);
    void criticalErrorReported(const ErrorInfo& error);
    void errorCountChanged(int count);

private:
    QList<ErrorInfo> m_errors;
    mutable QMutex m_mutex;
    
    // Logging
    bool m_loggingEnabled;
    QString m_logFilePath;
    QFile* m_logFile;
    QTextStream* m_logStream;
    ErrorLevel m_logLevel;
    
    // Internal methods
    void writeToLog(const ErrorInfo& error);
    void initializeLogging();
    void cleanupLogging();
    QString generateErrorId(const ErrorInfo& error) const;
    
    // Error code mapping (matching legacy system)
    static int getErrorCodeForCategory(ErrorCategory category);
    static QString getDefaultSuggestion(ErrorCategory category);
};

// Global error handler instance
extern ErrorHandler* g_errorHandler;

// Convenience macros for error reporting (matching legacy system style)
#define REPORT_ERROR(message) \
    if (g_errorHandler) g_errorHandler->reportError(ErrorHandler::ErrorLevel::Error, \
        ErrorHandler::ErrorCategory::Unknown, message, __FUNCTION__)

#define REPORT_FILE_ERROR(message, filePath) \
    if (g_errorHandler) g_errorHandler->reportFileError(message, filePath)

#define REPORT_VALIDATION_ERROR(message, details) \
    if (g_errorHandler) g_errorHandler->reportValidationError(message, details)

#define REPORT_PLUGIN_ERROR(message, pluginName) \
    if (g_errorHandler) g_errorHandler->reportPluginError(message, pluginName)

#define REPORT_CRITICAL_ERROR(message) \
    if (g_errorHandler) g_errorHandler->reportError(ErrorHandler::ErrorLevel::Critical, \
        ErrorHandler::ErrorCategory::System, message, __FUNCTION__)