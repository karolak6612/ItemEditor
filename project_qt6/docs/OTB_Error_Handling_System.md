# OTB Error Handling System Documentation

## Overview

The OTB Error Handling System provides comprehensive error management for OTB file operations in the Qt6 Item Editor project. It implements C#-compatible error patterns while leveraging Qt6 capabilities for robust error reporting, recovery mechanisms, and user-friendly error messages.

## Key Features

### 1. **Comprehensive Error Classification**
- **Error Codes**: Organized into categories (1000-1799) for different error types
- **Error Severity**: Info, Warning, Error, Critical, Fatal levels
- **Error Categories**: FileSystem, Validation, Parsing, Memory, Network, Security, UserInput, Internal

### 2. **Exception Hierarchy**
- **Base Class**: `OtbException` inheriting from `QException`
- **Specific Exceptions**: `ArgumentNullException`, `FileNotFoundException`, `ValidationException`, etc.
- **C# Compatibility**: Matches .NET exception patterns and naming

### 3. **Recovery Strategies**
- **Automatic Recovery**: Retry, Skip, UseDefault, Repair
- **User Intervention**: Clear guidance on required actions
- **Abort Strategy**: For critical errors requiring immediate termination

### 4. **Centralized Error Management**
- **ErrorHandler Singleton**: Centralized error collection and reporting
- **Error Filtering**: By severity, category, and time
- **Statistics**: Error counts, trends, and summaries

## Core Components

### Error Codes (`OtbErrorCode`)

```cpp
enum class OtbErrorCode {
    // File System Errors (1000-1099)
    FileNotFound = 1001,
    FileAccessDenied = 1002,
    FileCorrupted = 1003,
    
    // Validation Errors (1100-1199)
    InvalidHeader = 1101,
    ChecksumMismatch = 1103,
    
    // Memory Errors (1300-1399)
    MemoryAllocationFailed = 1301,
    // ... more error codes
};
```

### Exception Classes

```cpp
// Base exception class
class OtbException : public QException {
public:
    explicit OtbException(OtbErrorCode errorCode, const QString& message);
    
    OtbErrorCode getErrorCode() const;
    ErrorSeverity getSeverity() const;
    RecoveryStrategy getSuggestedRecovery() const;
    bool isRecoverable() const;
};

// Specific exception types
class ArgumentNullException : public OtbException;
class FileNotFoundException : public OtbException;
class ValidationException : public OtbException;
```

### Error Information Structure

```cpp
struct ErrorInfo {
    OtbErrorCode errorCode;
    ErrorSeverity severity;
    ErrorCategory category;
    QString message;
    QString detailedDescription;
    QString context;
    QString suggestion;
    QDateTime timestamp;
    QString fileName;
    qint64 filePosition;
    RecoveryStrategy suggestedRecovery;
};
```

## Usage Examples

### 1. **Basic Error Handling**

```cpp
#include "otberrors.h"

using namespace OTB;

void loadOtbFile(const QString& filePath) {
    try {
        // Validate parameters
        OTB_THROW_IF_EMPTY(filePath, "filePath");
        
        // Attempt file operation
        if (!QFile::exists(filePath)) {
            throw FileNotFoundException(filePath);
        }
        
        // Process file...
        
    } catch (const ArgumentNullException& ex) {
        qCritical() << "Parameter error:" << ex.getMessage();
        // Handle parameter validation error
        
    } catch (const FileNotFoundException& ex) {
        qCritical() << "File error:" << ex.getMessage();
        
        if (ex.isRecoverable()) {
            // Suggest user action
            showUserMessage(ex.getMessage() + "\n" + 
                          "Please check the file path and try again.");
        }
        
    } catch (const OtbException& ex) {
        qCritical() << "OTB error:" << ex.getMessage();
        
        // Handle based on recovery strategy
        switch (ex.getSuggestedRecovery()) {
            case RecoveryStrategy::Retry:
                // Implement retry logic
                break;
            case RecoveryStrategy::UserIntervention:
                // Show user-friendly message
                break;
            case RecoveryStrategy::Abort:
                // Terminate operation
                return;
        }
    }
}
```

### 2. **Error Reporting and Statistics**

```cpp
void checkSystemErrors() {
    ErrorHandler& handler = ErrorHandler::instance();
    
    // Check for critical errors
    if (handler.hasErrors(ErrorSeverity::Critical)) {
        auto criticalErrors = handler.getErrors(ErrorSeverity::Critical);
        
        qCritical() << "System has" << criticalErrors.size() << "critical errors";
        
        // Generate user report
        QString summary = ErrorUtils::generateSummaryReport(criticalErrors);
        showUserDialog("Critical Errors Detected", summary);
    }
    
    // Export error log for debugging
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) 
                     + "/otb_errors.log";
    handler.exportErrorLog(logPath);
}
```

### 3. **Integration with Existing Components**

```cpp
// Convert from existing error enums
OtbErrorCode convertReadError(OtbReadError readError) {
    return ErrorUtils::fromOtbReadError(readError);
}

OtbErrorCode convertValidationError(ValidationError validationError) {
    return ErrorUtils::fromValidationError(validationError);
}

// Enhanced OTB reader with error handling
bool readOtbFileWithErrorHandling(const QString& filePath, ServerItemList& items) {
    try {
        OtbReader reader;
        QString errorString;
        
        if (!reader.read(filePath, items, errorString)) {
            // Convert reader error to our error system
            OtbErrorCode errorCode = ErrorUtils::fromOtbReadError(reader.getLastErrorCode());
            
            // Report to error handler
            ErrorHandler::instance().reportError(errorCode, errorString, 
                                               ErrorUtils::determineSeverity(errorCode));
            
            // Create appropriate exception
            throw OtbException(errorCode, errorString);
        }
        
        return true;
        
    } catch (const OtbException& ex) {
        // Log error details
        qCritical() << "OTB read failed:" << ex.getMessage();
        
        // Show user-friendly message
        QString userMessage = ErrorUtils::formatUserFriendlyMessage(ex.getErrorInfo());
        showErrorDialog("Failed to Load OTB File", userMessage);
        
        return false;
    }
}
```

### 4. **User-Friendly Error Messages**

```cpp
void showUserFriendlyError(const ErrorInfo& errorInfo) {
    QString userMessage = ErrorUtils::formatUserFriendlyMessage(errorInfo);
    
    // Example output:
    // "File not found (Loading OTB file)
    //  
    //  Suggestion: Check if the file exists and you have read permissions
    //  
    //  Recovery: Manual intervention is required."
    
    QMessageBox::critical(nullptr, "Error", userMessage);
}
```

## Utility Functions

### ErrorUtils Namespace

```cpp
namespace ErrorUtils {
    // Error conversion
    OtbErrorCode fromOtbReadError(OtbReadError readError);
    OtbErrorCode fromValidationError(ValidationError validationError);
    
    // Error classification
    ErrorCategory categorizeError(OtbErrorCode errorCode);
    ErrorSeverity determineSeverity(OtbErrorCode errorCode);
    RecoveryStrategy suggestRecovery(OtbErrorCode errorCode);
    
    // Error checking
    bool isFileSystemError(OtbErrorCode errorCode);
    bool isValidationError(OtbErrorCode errorCode);
    bool isRecoverableError(OtbErrorCode errorCode);
    
    // Message formatting
    QString formatErrorMessage(OtbErrorCode errorCode, const QString& context);
    QString formatUserFriendlyMessage(const ErrorInfo& errorInfo);
    
    // Reporting
    QString generateErrorReport(const QList<ErrorInfo>& errors);
    QString generateSummaryReport(const QList<ErrorInfo>& errors);
}
```

## Convenience Macros

```cpp
// Parameter validation macros
OTB_THROW_IF_NULL(ptr, paramName)
OTB_THROW_IF_EMPTY(str, paramName)
OTB_THROW_IF_OUT_OF_RANGE(value, min, max, paramName)

// Error reporting macros
OTB_REPORT_ERROR(code, message)
OTB_REPORT_WARNING(message)
```

## Configuration

### ErrorHandler Configuration

```cpp
ErrorHandler& handler = ErrorHandler::instance();

// Set maximum number of errors to keep in memory
handler.setMaxErrorCount(1000);

// Enable/disable logging
handler.setLoggingEnabled(true);

// Set minimum severity for reporting
handler.setSeverityFilter(ErrorSeverity::Warning);
```

### Validation Options

```cpp
ValidationOptions options;
options.enableChecksumValidation = true;
options.enableStructureValidation = true;
options.strictMode = false;  // Don't fail on warnings
options.maxFileSize = 1024 * 1024 * 1024;  // 1GB limit

OtbValidator validator;
validator.setValidationOptions(options);
```

## Integration with Qt Logging

The error handling system integrates with Qt's logging framework:

```cpp
// Enable OTB error logging
QLoggingCategory::setFilterRules("otb.errors.debug=true");

// Errors are automatically logged with appropriate severity:
// qCInfo(otbErrors) for Info level
// qCWarning(otbErrors) for Warning level  
// qCCritical(otbErrors) for Error/Critical/Fatal levels
```

## Testing

The system includes comprehensive tests:

- **TestOtbErrors.exe**: Basic error handling functionality
- **TestOtbErrorsIntegration.exe**: Integration with OTB components
- **Test Coverage**: Exception handling, error conversion, recovery strategies, user messages

## Best Practices

### 1. **Error Handling Strategy**
- Always use specific exception types when possible
- Provide context information in error messages
- Include suggestions for user action
- Log errors appropriately for debugging

### 2. **Recovery Implementation**
- Check `isRecoverable()` before attempting recovery
- Implement retry logic with limits
- Provide clear user feedback for required actions
- Use abort strategy for critical system errors

### 3. **Performance Considerations**
- Error handling has minimal performance impact during normal operation
- Error collection is bounded by `maxErrorCount` setting
- Use severity filtering to reduce noise
- Export error logs periodically for analysis

### 4. **User Experience**
- Always show user-friendly messages for errors requiring user action
- Provide specific suggestions when possible
- Use appropriate dialog types (warning, error, critical)
- Include recovery options in user interface

## File Structure

```
project_qt6/
├── include/otb/
│   └── otberrors.h              # Main error handling header
├── src/otb/
│   ├── otberrors.cpp            # Implementation
│   ├── test_otberrors.cpp       # Basic tests
│   └── test_otberrors_integration.cpp  # Integration tests
└── build_errors/
    └── Debug/
        ├── TestOtbErrors.exe
        └── TestOtbErrorsIntegration.exe
```

## Conclusion

The OTB Error Handling System provides a robust, user-friendly, and maintainable approach to error management in the Qt6 Item Editor project. It successfully bridges C# error handling patterns with Qt6 capabilities, ensuring consistent error reporting and recovery across all OTB operations.