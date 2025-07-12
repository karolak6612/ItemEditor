#include "otberrors.h"
#include "otbreader.h"  // For OtbReadError
#include "otbvalidator.h"  // For ValidationError
#include <QTextStream>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QMetaEnum>
#include <QLoggingCategory>

namespace OTB {

// Define logging category
Q_LOGGING_CATEGORY(otbErrors, "otb.errors")

// ErrorInfo implementation
QString ErrorInfo::toString() const {
    QString result;
    QTextStream stream(&result);
    
    stream << "[" << static_cast<int>(severity) << "] ";
    stream << "Error " << static_cast<int>(errorCode) << ": " << message;
    
    if (!context.isEmpty()) {
        stream << " (Context: " << context << ")";
    }
    
    if (!fileName.isEmpty()) {
        stream << " [File: " << fileName;
        if (filePosition >= 0) {
            stream << ", Position: " << filePosition;
        }
        if (lineNumber >= 0) {
            stream << ", Line: " << lineNumber;
        }
        stream << "]";
    }
    
    stream << " [" << timestamp.toString(Qt::ISODate) << "]";
    
    return result;
}

ErrorCategory ErrorInfo::getErrorCategory(OtbErrorCode code) {
    int codeValue = static_cast<int>(code);
    
    if (codeValue >= 1001 && codeValue <= 1099) return ErrorCategory::FileSystem;
    if (codeValue >= 1100 && codeValue <= 1199) return ErrorCategory::Validation;
    if (codeValue >= 1200 && codeValue <= 1299) return ErrorCategory::Parsing;
    if (codeValue >= 1300 && codeValue <= 1399) return ErrorCategory::Memory;
    if (codeValue >= 1400 && codeValue <= 1499) return ErrorCategory::Network;
    if (codeValue >= 1500 && codeValue <= 1599) return ErrorCategory::Security;
    if (codeValue >= 1600 && codeValue <= 1699) return ErrorCategory::UserInput;
    
    return ErrorCategory::Internal;
}

// OtbException implementation
OtbException::OtbException(const ErrorInfo& errorInfo) : m_errorInfo(errorInfo) {
    qCWarning(otbErrors) << "OtbException created:" << errorInfo.toString();
}

OtbException::OtbException(OtbErrorCode errorCode, const QString& message, ErrorSeverity severity)
    : m_errorInfo(errorCode, message, severity) {
    qCWarning(otbErrors) << "OtbException created:" << m_errorInfo.toString();
}

OtbException::OtbException(const QString& message)
    : m_errorInfo(OtbErrorCode::InternalError, message, ErrorSeverity::Error) {
    qCWarning(otbErrors) << "OtbException created:" << m_errorInfo.toString();
}

const char* OtbException::what() const noexcept {
    m_whatBuffer = m_errorInfo.toString().toUtf8();
    return m_whatBuffer.constData();
}

bool OtbException::isRecoverable() const {
    return ErrorUtils::isRecoverableError(m_errorInfo.errorCode);
}

bool OtbException::requiresUserIntervention() const {
    return m_errorInfo.suggestedRecovery == RecoveryStrategy::UserIntervention;
}

// Specific exception implementations
ArgumentNullException::ArgumentNullException(const QString& parameterName)
    : OtbException(OtbErrorCode::ArgumentNull, 
                  QString("Parameter '%1' cannot be null or empty").arg(parameterName),
                  ErrorSeverity::Error) {
    m_errorInfo.context = "Parameter validation";
    m_errorInfo.suggestion = "Ensure the parameter is properly initialized before use";
    m_errorInfo.suggestedRecovery = RecoveryStrategy::UserIntervention;
}

ArgumentOutOfRangeException::ArgumentOutOfRangeException(const QString& parameterName, const QString& actualValue)
    : OtbException(OtbErrorCode::ArgumentOutOfRange,
                  QString("Parameter '%1' is out of valid range. Actual value: %2").arg(parameterName, actualValue),
                  ErrorSeverity::Error) {
    m_errorInfo.context = "Parameter validation";
    m_errorInfo.suggestion = "Check the valid range for this parameter";
    m_errorInfo.suggestedRecovery = RecoveryStrategy::UserIntervention;
}

UnauthorizedAccessException::UnauthorizedAccessException(const QString& filePath)
    : OtbException(OtbErrorCode::UnauthorizedAccess,
                  QString("Access to file '%1' is denied").arg(filePath),
                  ErrorSeverity::Error) {
    m_errorInfo.context = "File access";
    m_errorInfo.fileName = filePath;
    m_errorInfo.suggestion = "Check file permissions or run as administrator";
    m_errorInfo.suggestedRecovery = RecoveryStrategy::UserIntervention;
}

FileNotFoundException::FileNotFoundException(const QString& filePath)
    : OtbException(OtbErrorCode::FileNotFound,
                  QString("File '%1' was not found").arg(filePath),
                  ErrorSeverity::Error) {
    m_errorInfo.context = "File access";
    m_errorInfo.fileName = filePath;
    m_errorInfo.suggestion = "Verify the file path and ensure the file exists";
    m_errorInfo.suggestedRecovery = RecoveryStrategy::UserIntervention;
}

InvalidDataException::InvalidDataException(const QString& description, const QString& context)
    : OtbException(OtbErrorCode::DataIntegrityFailure,
                  QString("Invalid data encountered: %1").arg(description),
                  ErrorSeverity::Error) {
    m_errorInfo.context = context.isEmpty() ? "Data validation" : context;
    m_errorInfo.suggestion = "Check data integrity and file corruption";
    m_errorInfo.suggestedRecovery = RecoveryStrategy::Repair;
}

MemoryException::MemoryException(const QString& operation, qint64 requestedBytes)
    : OtbException(OtbErrorCode::MemoryAllocationFailed,
                  QString("Memory allocation failed during %1").arg(operation),
                  ErrorSeverity::Critical) {
    m_errorInfo.context = "Memory management";
    if (requestedBytes > 0) {
        m_errorInfo.detailedDescription = QString("Requested %1 bytes").arg(requestedBytes);
    }
    m_errorInfo.suggestion = "Close other applications to free memory or use a smaller file";
    m_errorInfo.suggestedRecovery = RecoveryStrategy::UserIntervention;
}

ValidationException::ValidationException(const QString& validationRule, const QString& actualValue)
    : OtbException(OtbErrorCode::AttributeValidationFailed,
                  QString("Validation failed for rule '%1'. Actual value: %2").arg(validationRule, actualValue),
                  ErrorSeverity::Warning) {
    m_errorInfo.context = "Data validation";
    m_errorInfo.suggestion = "Check data format and validation rules";
    m_errorInfo.suggestedRecovery = RecoveryStrategy::Skip;
}

// ErrorHandler implementation
ErrorHandler& ErrorHandler::instance() {
    static ErrorHandler instance;
    return instance;
}

void ErrorHandler::reportError(const ErrorInfo& errorInfo) {
    if (errorInfo.severity < m_minSeverity) {
        return;
    }
    
    m_errors.append(errorInfo);
    trimErrorList();
    
    if (m_loggingEnabled) {
        ErrorUtils::logError(errorInfo);
    }
}

void ErrorHandler::reportError(OtbErrorCode errorCode, const QString& message, ErrorSeverity severity) {
    ErrorInfo errorInfo(errorCode, message, severity);
    reportError(errorInfo);
}

void ErrorHandler::reportWarning(const QString& message, const QString& context) {
    ErrorInfo errorInfo(OtbErrorCode::None, message, ErrorSeverity::Warning);
    errorInfo.context = context;
    reportError(errorInfo);
}

void ErrorHandler::reportInfo(const QString& message, const QString& context) {
    ErrorInfo errorInfo(OtbErrorCode::None, message, ErrorSeverity::Info);
    errorInfo.context = context;
    reportError(errorInfo);
}

QList<ErrorInfo> ErrorHandler::getErrors(ErrorSeverity minSeverity) const {
    QList<ErrorInfo> result;
    for (const auto& error : m_errors) {
        if (error.severity >= minSeverity) {
            result.append(error);
        }
    }
    return result;
}

QList<ErrorInfo> ErrorHandler::getErrorsByCategory(ErrorCategory category) const {
    QList<ErrorInfo> result;
    for (const auto& error : m_errors) {
        if (error.category == category) {
            result.append(error);
        }
    }
    return result;
}

ErrorInfo ErrorHandler::getLastError() const {
    for (auto it = m_errors.rbegin(); it != m_errors.rend(); ++it) {
        if (it->severity >= ErrorSeverity::Error) {
            return *it;
        }
    }
    return ErrorInfo();
}

bool ErrorHandler::hasErrors(ErrorSeverity minSeverity) const {
    for (const auto& error : m_errors) {
        if (error.severity >= minSeverity) {
            return true;
        }
    }
    return false;
}

void ErrorHandler::clearErrors() {
    m_errors.clear();
}

void ErrorHandler::clearErrorsOlderThan(const QDateTime& cutoffTime) {
    m_errors.erase(std::remove_if(m_errors.begin(), m_errors.end(),
                                 [cutoffTime](const ErrorInfo& error) {
                                     return error.timestamp < cutoffTime;
                                 }), m_errors.end());
}

RecoveryStrategy ErrorHandler::suggestRecovery(const ErrorInfo& errorInfo) const {
    return ErrorUtils::suggestRecovery(errorInfo.errorCode);
}

bool ErrorHandler::canRecover(const ErrorInfo& errorInfo) const {
    return ErrorUtils::isRecoverableError(errorInfo.errorCode);
}

QString ErrorHandler::formatErrorReport() const {
    return ErrorUtils::generateErrorReport(m_errors);
}

void ErrorHandler::exportErrorLog(const QString& filePath) const {
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << formatErrorReport();
    }
}

void ErrorHandler::trimErrorList() {
    if (m_errors.size() > m_maxErrorCount) {
        int removeCount = m_errors.size() - m_maxErrorCount;
        m_errors.erase(m_errors.begin(), m_errors.begin() + removeCount);
    }
}

} // namespace OTB

// ErrorUtils namespace implementation
namespace OTB {
namespace ErrorUtils {

OtbErrorCode fromOtbReadError(OtbReadError readError) {
    switch (readError) {
        case OtbReadError::None:
            return OtbErrorCode::None;
        case OtbReadError::FileNotFound:
            return OtbErrorCode::FileNotFound;
        case OtbReadError::FileAccessDenied:
            return OtbErrorCode::FileAccessDenied;
        case OtbReadError::InvalidHeader:
            return OtbErrorCode::InvalidHeader;
        case OtbReadError::CorruptedData:
            return OtbErrorCode::DataIntegrityFailure;
        case OtbReadError::UnsupportedVersion:
            return OtbErrorCode::InvalidVersion;
        case OtbReadError::InvalidNodeStructure:
            return OtbErrorCode::NodeStructureInvalid;
        case OtbReadError::AttributeValidationFailed:
            return OtbErrorCode::AttributeValidationFailed;
        case OtbReadError::UnexpectedEndOfFile:
            return OtbErrorCode::UnexpectedEndOfFile;
        case OtbReadError::MemoryAllocationFailed:
            return OtbErrorCode::MemoryAllocationFailed;
        case OtbReadError::InvalidItemData:
            return OtbErrorCode::InvalidItemData;
        case OtbReadError::TreeStructureCorrupted:
            return OtbErrorCode::TreeCorruption;
        default:
            return OtbErrorCode::InternalError;
    }
}

OtbErrorCode fromValidationError(ValidationError validationError) {
    switch (validationError) {
        case ValidationError::None:
            return OtbErrorCode::None;
        case ValidationError::FileNotFound:
            return OtbErrorCode::FileNotFound;
        case ValidationError::FileAccessDenied:
            return OtbErrorCode::FileAccessDenied;
        case ValidationError::InvalidFileSize:
            return OtbErrorCode::FileTooBig;
        case ValidationError::ChecksumMismatch:
            return OtbErrorCode::ChecksumMismatch;
        case ValidationError::CorruptedHeader:
            return OtbErrorCode::InvalidHeader;
        case ValidationError::InvalidNodeStructure:
            return OtbErrorCode::NodeStructureInvalid;
        case ValidationError::DataIntegrityFailure:
            return OtbErrorCode::DataIntegrityFailure;
        case ValidationError::StructureInconsistency:
            return OtbErrorCode::StructureCorrupted;
        case ValidationError::AttributeValidationFailed:
            return OtbErrorCode::AttributeValidationFailed;
        case ValidationError::VersionMismatch:
            return OtbErrorCode::InvalidVersion;
        case ValidationError::TreeCorruption:
            return OtbErrorCode::TreeCorruption;
        case ValidationError::MemoryConstraintViolation:
            return OtbErrorCode::MemoryConstraintViolation;
        default:
            return OtbErrorCode::InternalError;
    }
}

QString formatErrorMessage(OtbErrorCode errorCode, const QString& context) {
    QString baseMessage;
    
    switch (errorCode) {
        case OtbErrorCode::None:
            return "No error";
        case OtbErrorCode::FileNotFound:
            baseMessage = "File not found";
            break;
        case OtbErrorCode::FileAccessDenied:
            baseMessage = "Access to file denied";
            break;
        case OtbErrorCode::FileCorrupted:
            baseMessage = "File is corrupted or invalid";
            break;
        case OtbErrorCode::FileTooBig:
            baseMessage = "File size exceeds maximum allowed limit";
            break;
        case OtbErrorCode::InvalidHeader:
            baseMessage = "Invalid or corrupted file header";
            break;
        case OtbErrorCode::InvalidVersion:
            baseMessage = "Unsupported file version";
            break;
        case OtbErrorCode::ChecksumMismatch:
            baseMessage = "File checksum verification failed";
            break;
        case OtbErrorCode::StructureCorrupted:
            baseMessage = "File structure is corrupted";
            break;
        case OtbErrorCode::DataIntegrityFailure:
            baseMessage = "Data integrity check failed";
            break;
        case OtbErrorCode::AttributeValidationFailed:
            baseMessage = "Attribute validation failed";
            break;
        case OtbErrorCode::UnexpectedEndOfFile:
            baseMessage = "Unexpected end of file encountered";
            break;
        case OtbErrorCode::MemoryAllocationFailed:
            baseMessage = "Memory allocation failed";
            break;
        case OtbErrorCode::ArgumentNull:
            baseMessage = "Required argument is null or empty";
            break;
        case OtbErrorCode::ArgumentOutOfRange:
            baseMessage = "Argument value is out of valid range";
            break;
        case OtbErrorCode::UnauthorizedAccess:
            baseMessage = "Unauthorized access attempt";
            break;
        case OtbErrorCode::InvalidOperation:
            baseMessage = "Invalid operation for current state";
            break;
        default:
            baseMessage = QString("Unknown error (code: %1)").arg(static_cast<int>(errorCode));
    }
    
    if (!context.isEmpty()) {
        return QString("%1 (%2)").arg(baseMessage, context);
    }
    
    return baseMessage;
}

QString formatUserFriendlyMessage(const ErrorInfo& errorInfo) {
    QString message = formatErrorMessage(errorInfo.errorCode, errorInfo.context);
    
    if (!errorInfo.suggestion.isEmpty()) {
        message += QString("\n\nSuggestion: %1").arg(errorInfo.suggestion);
    }
    
    if (errorInfo.suggestedRecovery != RecoveryStrategy::None) {
        QString recoveryText;
        switch (errorInfo.suggestedRecovery) {
            case RecoveryStrategy::Retry:
                recoveryText = "You can try the operation again.";
                break;
            case RecoveryStrategy::Skip:
                recoveryText = "You can skip this item and continue.";
                break;
            case RecoveryStrategy::UseDefault:
                recoveryText = "Default values will be used.";
                break;
            case RecoveryStrategy::Repair:
                recoveryText = "Attempt to repair the data automatically.";
                break;
            case RecoveryStrategy::UserIntervention:
                recoveryText = "Manual intervention is required.";
                break;
            case RecoveryStrategy::Abort:
                recoveryText = "Operation must be aborted.";
                break;
            default:
                break;
        }
        
        if (!recoveryText.isEmpty()) {
            message += QString("\n\nRecovery: %1").arg(recoveryText);
        }
    }
    
    return message;
}

ErrorCategory categorizeError(OtbErrorCode errorCode) {
    return ErrorInfo::getErrorCategory(errorCode);
}

ErrorSeverity determineSeverity(OtbErrorCode errorCode) {
    switch (errorCode) {
        case OtbErrorCode::None:
            return ErrorSeverity::Info;
            
        // Warnings
        case OtbErrorCode::AttributeValidationFailed:
        case OtbErrorCode::InvalidItemData:
            return ErrorSeverity::Warning;
            
        // Critical errors
        case OtbErrorCode::MemoryAllocationFailed:
        case OtbErrorCode::OutOfMemory:
        case OtbErrorCode::SecurityViolation:
            return ErrorSeverity::Critical;
            
        // Fatal errors
        case OtbErrorCode::StateCorruption:
            return ErrorSeverity::Fatal;
            
        // Default to Error
        default:
            return ErrorSeverity::Error;
    }
}

RecoveryStrategy suggestRecovery(OtbErrorCode errorCode) {
    switch (errorCode) {
        case OtbErrorCode::None:
            return RecoveryStrategy::None;
            
        // Retry strategies
        case OtbErrorCode::NetworkTimeout:
        case OtbErrorCode::NetworkConnectionFailed:
        case OtbErrorCode::FileAccessDenied:
            return RecoveryStrategy::Retry;
            
        // Skip strategies
        case OtbErrorCode::AttributeValidationFailed:
        case OtbErrorCode::InvalidItemData:
            return RecoveryStrategy::Skip;
            
        // Repair strategies
        case OtbErrorCode::ChecksumMismatch:
        case OtbErrorCode::DataIntegrityFailure:
        case OtbErrorCode::StructureCorrupted:
            return RecoveryStrategy::Repair;
            
        // User intervention required
        case OtbErrorCode::FileNotFound:
        case OtbErrorCode::ArgumentNull:
        case OtbErrorCode::ArgumentOutOfRange:
        case OtbErrorCode::UnauthorizedAccess:
        case OtbErrorCode::InvalidVersion:
            return RecoveryStrategy::UserIntervention;
            
        // Abort strategies
        case OtbErrorCode::MemoryAllocationFailed:
        case OtbErrorCode::OutOfMemory:
        case OtbErrorCode::StateCorruption:
            return RecoveryStrategy::Abort;
            
        default:
            return RecoveryStrategy::UserIntervention;
    }
}

bool isFileSystemError(OtbErrorCode errorCode) {
    return categorizeError(errorCode) == ErrorCategory::FileSystem;
}

bool isValidationError(OtbErrorCode errorCode) {
    return categorizeError(errorCode) == ErrorCategory::Validation;
}

bool isRecoverableError(OtbErrorCode errorCode) {
    RecoveryStrategy strategy = suggestRecovery(errorCode);
    return strategy != RecoveryStrategy::Abort && strategy != RecoveryStrategy::None;
}

bool isUserError(OtbErrorCode errorCode) {
    return categorizeError(errorCode) == ErrorCategory::UserInput;
}

void logError(const ErrorInfo& errorInfo) {
    switch (errorInfo.severity) {
        case ErrorSeverity::Info:
            qCInfo(otbErrors) << errorInfo.toString();
            break;
        case ErrorSeverity::Warning:
            qCWarning(otbErrors) << errorInfo.toString();
            break;
        case ErrorSeverity::Error:
        case ErrorSeverity::Critical:
        case ErrorSeverity::Fatal:
            qCCritical(otbErrors) << errorInfo.toString();
            break;
    }
}

void logException(const OtbException& exception) {
    logError(exception.getErrorInfo());
}

QString generateErrorReport(const QList<ErrorInfo>& errors) {
    if (errors.isEmpty()) {
        return "No errors to report.";
    }
    
    QString report;
    QTextStream stream(&report);
    
    stream << "=== OTB Error Report ===\n";
    stream << "Generated: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
    stream << "Total errors: " << errors.size() << "\n\n";
    
    // Group by severity
    QMap<ErrorSeverity, QList<ErrorInfo>> groupedErrors;
    for (const auto& error : errors) {
        groupedErrors[error.severity].append(error);
    }
    
    // Report by severity (highest first)
    QList<ErrorSeverity> severities = {ErrorSeverity::Fatal, ErrorSeverity::Critical, 
                                      ErrorSeverity::Error, ErrorSeverity::Warning, ErrorSeverity::Info};
    
    for (ErrorSeverity severity : severities) {
        if (!groupedErrors.contains(severity)) continue;
        
        const auto& severityErrors = groupedErrors[severity];
        QString severityName;
        switch (severity) {
            case ErrorSeverity::Info: severityName = "INFO"; break;
            case ErrorSeverity::Warning: severityName = "WARNING"; break;
            case ErrorSeverity::Error: severityName = "ERROR"; break;
            case ErrorSeverity::Critical: severityName = "CRITICAL"; break;
            case ErrorSeverity::Fatal: severityName = "FATAL"; break;
        }
        stream << "=== " << severityName << " (" << severityErrors.size() << ") ===\n";
        
        for (const auto& error : severityErrors) {
            stream << error.toString() << "\n";
            if (!error.detailedDescription.isEmpty()) {
                stream << "  Details: " << error.detailedDescription << "\n";
            }
            if (!error.suggestion.isEmpty()) {
                stream << "  Suggestion: " << error.suggestion << "\n";
            }
            stream << "\n";
        }
    }
    
    return report;
}

QString generateSummaryReport(const QList<ErrorInfo>& errors) {
    if (errors.isEmpty()) {
        return "No errors";
    }
    
    QMap<ErrorSeverity, int> counts;
    for (const auto& error : errors) {
        counts[error.severity]++;
    }
    
    QStringList parts;
    if (counts[ErrorSeverity::Fatal] > 0) {
        parts << QString("%1 fatal").arg(counts[ErrorSeverity::Fatal]);
    }
    if (counts[ErrorSeverity::Critical] > 0) {
        parts << QString("%1 critical").arg(counts[ErrorSeverity::Critical]);
    }
    if (counts[ErrorSeverity::Error] > 0) {
        parts << QString("%1 errors").arg(counts[ErrorSeverity::Error]);
    }
    if (counts[ErrorSeverity::Warning] > 0) {
        parts << QString("%1 warnings").arg(counts[ErrorSeverity::Warning]);
    }
    
    return parts.join(", ");
}

} // namespace ErrorUtils
} // namespace OTB