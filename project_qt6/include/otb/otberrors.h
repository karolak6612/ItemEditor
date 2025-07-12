#ifndef OTBERRORS_H
#define OTBERRORS_H

#include <QString>
#include <QStringList>
#include <QException>
#include <QDebug>
#include <QDateTime>
#include <QLoggingCategory>

namespace OTB {

// Forward declarations
enum class OtbReadError;
enum class ValidationError;

// Logging category for OTB errors
Q_DECLARE_LOGGING_CATEGORY(otbErrors)

// Error severity levels matching C# patterns
enum class ErrorSeverity {
    Info = 0,
    Warning = 1,
    Error = 2,
    Critical = 3,
    Fatal = 4
};

// Error categories for better organization
enum class ErrorCategory {
    FileSystem = 0,
    Validation = 1,
    Parsing = 2,
    Memory = 3,
    Network = 4,
    Security = 5,
    UserInput = 6,
    Internal = 7
};

// Enhanced error codes that encompass all OTB operations
enum class OtbErrorCode {
    // Success
    None = 0,
    
    // File System Errors (1000-1099)
    FileNotFound = 1001,
    FileAccessDenied = 1002,
    FileCorrupted = 1003,
    FileTooBig = 1004,
    FileWriteError = 1005,
    DirectoryNotFound = 1006,
    DiskSpaceInsufficient = 1007,
    
    // Validation Errors (1100-1199)
    InvalidHeader = 1101,
    InvalidVersion = 1102,
    ChecksumMismatch = 1103,
    StructureCorrupted = 1104,
    DataIntegrityFailure = 1105,
    AttributeValidationFailed = 1106,
    NodeStructureInvalid = 1107,
    TreeCorruption = 1108,
    
    // Parsing Errors (1200-1299)
    UnexpectedEndOfFile = 1201,
    InvalidNodeMarker = 1202,
    InvalidAttributeData = 1203,
    UnsupportedDataFormat = 1204,
    ParsingTimeout = 1205,
    InvalidItemData = 1206,
    
    // Memory Errors (1300-1399)
    MemoryAllocationFailed = 1301,
    MemoryConstraintViolation = 1302,
    OutOfMemory = 1303,
    
    // Network Errors (1400-1499) - for future use
    NetworkTimeout = 1401,
    NetworkConnectionFailed = 1402,
    
    // Security Errors (1500-1599)
    UnauthorizedAccess = 1501,
    SecurityViolation = 1502,
    
    // User Input Errors (1600-1699)
    InvalidArgument = 1601,
    ArgumentNull = 1602,
    ArgumentOutOfRange = 1603,
    
    // Internal Errors (1700-1799)
    InternalError = 1701,
    NotImplemented = 1702,
    InvalidOperation = 1703,
    StateCorruption = 1704
};

// Error recovery strategies
enum class RecoveryStrategy {
    None = 0,
    Retry = 1,
    Skip = 2,
    UseDefault = 3,
    Repair = 4,
    Abort = 5,
    UserIntervention = 6
};

// Detailed error information structure
struct ErrorInfo {
    OtbErrorCode errorCode = OtbErrorCode::None;
    ErrorSeverity severity = ErrorSeverity::Error;
    ErrorCategory category = ErrorCategory::Internal;
    QString message;
    QString detailedDescription;
    QString context;  // Where the error occurred
    QString suggestion;  // What user can do
    QDateTime timestamp;
    QString fileName;
    qint64 filePosition = -1;
    int lineNumber = -1;
    RecoveryStrategy suggestedRecovery = RecoveryStrategy::None;
    QStringList additionalInfo;
    
    ErrorInfo() : timestamp(QDateTime::currentDateTime()) {}
    
    ErrorInfo(OtbErrorCode code, const QString& msg, ErrorSeverity sev = ErrorSeverity::Error)
        : errorCode(code), severity(sev), message(msg), timestamp(QDateTime::currentDateTime()) {
        category = getErrorCategory(code);
    }
    
    bool isValid() const { return errorCode != OtbErrorCode::None; }
    QString toString() const;
    
    static ErrorCategory getErrorCategory(OtbErrorCode code);
    
private:
};

// Base exception class for OTB operations (matching C# Exception patterns)
class OtbException : public QException {
public:
    explicit OtbException(const ErrorInfo& errorInfo);
    explicit OtbException(OtbErrorCode errorCode, const QString& message, 
                         ErrorSeverity severity = ErrorSeverity::Error);
    explicit OtbException(const QString& message);
    
    virtual ~OtbException() noexcept = default;
    
    // QException interface
    void raise() const override { throw *this; }
    OtbException* clone() const override { return new OtbException(*this); }
    const char* what() const noexcept override;
    
    // OTB-specific interface
    const ErrorInfo& getErrorInfo() const { return m_errorInfo; }
    OtbErrorCode getErrorCode() const { return m_errorInfo.errorCode; }
    ErrorSeverity getSeverity() const { return m_errorInfo.severity; }
    QString getMessage() const { return m_errorInfo.message; }
    QString getDetailedDescription() const { return m_errorInfo.detailedDescription; }
    RecoveryStrategy getSuggestedRecovery() const { return m_errorInfo.suggestedRecovery; }
    
    // Helper methods
    bool isRecoverable() const;
    bool requiresUserIntervention() const;
    
protected:
    ErrorInfo m_errorInfo;
    mutable QByteArray m_whatBuffer;  // For what() method
};

// Specific exception types matching C# patterns
class ArgumentNullException : public OtbException {
public:
    explicit ArgumentNullException(const QString& parameterName);
};

class ArgumentOutOfRangeException : public OtbException {
public:
    explicit ArgumentOutOfRangeException(const QString& parameterName, const QString& actualValue);
};

class UnauthorizedAccessException : public OtbException {
public:
    explicit UnauthorizedAccessException(const QString& filePath);
};

class FileNotFoundException : public OtbException {
public:
    explicit FileNotFoundException(const QString& filePath);
};

class InvalidDataException : public OtbException {
public:
    explicit InvalidDataException(const QString& description, const QString& context = QString());
};

class MemoryException : public OtbException {
public:
    explicit MemoryException(const QString& operation, qint64 requestedBytes = -1);
};

class ValidationException : public OtbException {
public:
    explicit ValidationException(const QString& validationRule, const QString& actualValue);
};

// Error handler class for centralized error management
class ErrorHandler {
public:
    static ErrorHandler& instance();
    
    // Error reporting
    void reportError(const ErrorInfo& errorInfo);
    void reportError(OtbErrorCode errorCode, const QString& message, 
                    ErrorSeverity severity = ErrorSeverity::Error);
    void reportWarning(const QString& message, const QString& context = QString());
    void reportInfo(const QString& message, const QString& context = QString());
    
    // Error retrieval
    QList<ErrorInfo> getErrors(ErrorSeverity minSeverity = ErrorSeverity::Warning) const;
    QList<ErrorInfo> getErrorsByCategory(ErrorCategory category) const;
    ErrorInfo getLastError() const;
    bool hasErrors(ErrorSeverity minSeverity = ErrorSeverity::Error) const;
    
    // Error management
    void clearErrors();
    void clearErrorsOlderThan(const QDateTime& cutoffTime);
    
    // Configuration
    void setMaxErrorCount(int maxCount) { m_maxErrorCount = maxCount; }
    void setLoggingEnabled(bool enabled) { m_loggingEnabled = enabled; }
    void setSeverityFilter(ErrorSeverity minSeverity) { m_minSeverity = minSeverity; }
    
    // Error recovery
    RecoveryStrategy suggestRecovery(const ErrorInfo& errorInfo) const;
    bool canRecover(const ErrorInfo& errorInfo) const;
    
    // Utility methods
    QString formatErrorReport() const;
    void exportErrorLog(const QString& filePath) const;
    
private:
    ErrorHandler() = default;
    void trimErrorList();
    
    QList<ErrorInfo> m_errors;
    int m_maxErrorCount = 1000;
    bool m_loggingEnabled = true;
    ErrorSeverity m_minSeverity = ErrorSeverity::Info;
};

// Utility functions for error handling
namespace ErrorUtils {
    // Error code conversion from existing enums
    OtbErrorCode fromOtbReadError(OtbReadError readError);
    OtbErrorCode fromValidationError(ValidationError validationError);
    
    // Error message formatting
    QString formatErrorMessage(OtbErrorCode errorCode, const QString& context = QString());
    QString formatUserFriendlyMessage(const ErrorInfo& errorInfo);
    
    // Error categorization
    ErrorCategory categorizeError(OtbErrorCode errorCode);
    ErrorSeverity determineSeverity(OtbErrorCode errorCode);
    RecoveryStrategy suggestRecovery(OtbErrorCode errorCode);
    
    // Validation helpers
    bool isFileSystemError(OtbErrorCode errorCode);
    bool isValidationError(OtbErrorCode errorCode);
    bool isRecoverableError(OtbErrorCode errorCode);
    bool isUserError(OtbErrorCode errorCode);
    
    // Logging helpers
    void logError(const ErrorInfo& errorInfo);
    void logException(const OtbException& exception);
    
    // Error reporting helpers
    QString generateErrorReport(const QList<ErrorInfo>& errors);
    QString generateSummaryReport(const QList<ErrorInfo>& errors);
}

// Macros for convenient error handling
#define OTB_THROW_IF_NULL(ptr, paramName) \
    do { if (!(ptr)) throw ArgumentNullException(paramName); } while(0)

#define OTB_THROW_IF_EMPTY(str, paramName) \
    do { if ((str).isEmpty()) throw ArgumentNullException(paramName); } while(0)

#define OTB_THROW_IF_OUT_OF_RANGE(value, min, max, paramName) \
    do { if ((value) < (min) || (value) > (max)) \
        throw ArgumentOutOfRangeException(paramName, QString::number(value)); } while(0)

#define OTB_REPORT_ERROR(code, message) \
    ErrorHandler::instance().reportError(code, message)

#define OTB_REPORT_WARNING(message) \
    ErrorHandler::instance().reportWarning(message)

} // namespace OTB

#endif // OTBERRORS_H