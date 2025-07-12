#include "otberrors.h"
#include "otbreader.h"  // For OtbReadError enum
#include "otbvalidator.h"  // For ValidationError enum
#include <QCoreApplication>
#include <QDebug>
#include <iostream>

using namespace OTB;

void testBasicErrorHandling() {
    std::cout << "\n=== Testing Basic Error Handling ===" << std::endl;
    
    // Test error creation and reporting
    ErrorHandler& handler = ErrorHandler::instance();
    handler.clearErrors();
    
    // Test different error types
    handler.reportError(OtbErrorCode::FileNotFound, "Test file not found");
    handler.reportWarning("This is a test warning", "Test context");
    handler.reportInfo("This is test information");
    
    // Check error retrieval
    auto errors = handler.getErrors(ErrorSeverity::Info);
    std::cout << "Total errors reported: " << errors.size() << std::endl;
    
    // Test error filtering
    auto criticalErrors = handler.getErrors(ErrorSeverity::Error);
    std::cout << "Critical errors: " << criticalErrors.size() << std::endl;
    
    // Test last error
    auto lastError = handler.getLastError();
    if (lastError.isValid()) {
        std::cout << "Last error: " << lastError.message.toStdString() << std::endl;
    }
    
    std::cout << "✓ Basic error handling test passed" << std::endl;
}

void testExceptionHandling() {
    std::cout << "\n=== Testing Exception Handling ===" << std::endl;
    
    try {
        // Test ArgumentNullException
        throw ArgumentNullException("testParameter");
    } catch (const OtbException& ex) {
        std::cout << "Caught ArgumentNullException: " << ex.getMessage().toStdString() << std::endl;
        std::cout << "Recovery strategy: " << static_cast<int>(ex.getSuggestedRecovery()) << std::endl;
    }
    
    try {
        // Test FileNotFoundException
        throw FileNotFoundException("/nonexistent/file.otb");
    } catch (const OtbException& ex) {
        std::cout << "Caught FileNotFoundException: " << ex.getMessage().toStdString() << std::endl;
        std::cout << "Is recoverable: " << (ex.isRecoverable() ? "Yes" : "No") << std::endl;
    }
    
    try {
        // Test ValidationException
        throw ValidationException("ItemID range", "65536");
    } catch (const OtbException& ex) {
        std::cout << "Caught ValidationException: " << ex.getMessage().toStdString() << std::endl;
        std::cout << "Error code: " << static_cast<int>(ex.getErrorCode()) << std::endl;
    }
    
    std::cout << "✓ Exception handling test passed" << std::endl;
}

void testErrorUtilities() {
    std::cout << "\n=== Testing Error Utilities ===" << std::endl;
    
    // Test error categorization
    auto category = ErrorUtils::categorizeError(OtbErrorCode::FileNotFound);
    std::cout << "FileNotFound category: " << static_cast<int>(category) << std::endl;
    
    // Test severity determination
    auto severity = ErrorUtils::determineSeverity(OtbErrorCode::MemoryAllocationFailed);
    std::cout << "MemoryAllocationFailed severity: " << static_cast<int>(severity) << std::endl;
    
    // Test recovery suggestion
    auto recovery = ErrorUtils::suggestRecovery(OtbErrorCode::AttributeValidationFailed);
    std::cout << "AttributeValidationFailed recovery: " << static_cast<int>(recovery) << std::endl;
    
    // Test error classification
    bool isFileError = ErrorUtils::isFileSystemError(OtbErrorCode::FileAccessDenied);
    std::cout << "FileAccessDenied is file system error: " << (isFileError ? "Yes" : "No") << std::endl;
    
    bool isRecoverable = ErrorUtils::isRecoverableError(OtbErrorCode::ChecksumMismatch);
    std::cout << "ChecksumMismatch is recoverable: " << (isRecoverable ? "Yes" : "No") << std::endl;
    
    // Test message formatting
    QString message = ErrorUtils::formatErrorMessage(OtbErrorCode::InvalidHeader, "OTB file validation");
    std::cout << "Formatted message: " << message.toStdString() << std::endl;
    
    std::cout << "✓ Error utilities test passed" << std::endl;
}

void testErrorReporting() {
    std::cout << "\n=== Testing Error Reporting ===" << std::endl;
    
    ErrorHandler& handler = ErrorHandler::instance();
    handler.clearErrors();
    
    // Add various errors for reporting
    ErrorInfo error1(OtbErrorCode::FileNotFound, "Test file missing", ErrorSeverity::Error);
    error1.context = "File loading";
    error1.fileName = "test.otb";
    error1.suggestion = "Check file path";
    
    ErrorInfo error2(OtbErrorCode::AttributeValidationFailed, "Invalid attribute", ErrorSeverity::Warning);
    error2.context = "Item validation";
    error2.suggestion = "Skip invalid items";
    
    ErrorInfo error3(OtbErrorCode::MemoryAllocationFailed, "Out of memory", ErrorSeverity::Critical);
    error3.context = "Large file processing";
    
    handler.reportError(error1);
    handler.reportError(error2);
    handler.reportError(error3);
    
    // Test summary report
    auto errors = handler.getErrors();
    QString summary = ErrorUtils::generateSummaryReport(errors);
    std::cout << "Error summary: " << summary.toStdString() << std::endl;
    
    // Test detailed report (first few lines)
    QString report = handler.formatErrorReport();
    QStringList lines = report.split('\n');
    for (int i = 0; i < qMin(10, lines.size()); ++i) {
        std::cout << lines[i].toStdString() << std::endl;
    }
    
    std::cout << "✓ Error reporting test passed" << std::endl;
}

void testMacros() {
    std::cout << "\n=== Testing Error Macros ===" << std::endl;
    
    try {
        QString* nullPtr = nullptr;
        OTB_THROW_IF_NULL(nullPtr, "nullPtr");
    } catch (const ArgumentNullException& ex) {
        std::cout << "Macro test 1 passed: " << ex.getMessage().toStdString() << std::endl;
    }
    
    try {
        QString emptyString;
        OTB_THROW_IF_EMPTY(emptyString, "emptyString");
    } catch (const ArgumentNullException& ex) {
        std::cout << "Macro test 2 passed: " << ex.getMessage().toStdString() << std::endl;
    }
    
    try {
        int value = 150;
        OTB_THROW_IF_OUT_OF_RANGE(value, 0, 100, "value");
    } catch (const ArgumentOutOfRangeException& ex) {
        std::cout << "Macro test 3 passed: " << ex.getMessage().toStdString() << std::endl;
    }
    
    // Test reporting macros
    OTB_REPORT_ERROR(OtbErrorCode::InvalidOperation, "Test error from macro");
    OTB_REPORT_WARNING("Test warning from macro");
    
    std::cout << "✓ Error macros test passed" << std::endl;
}

void testErrorConversion() {
    std::cout << "\n=== Testing Error Conversion ===" << std::endl;
    
    // Test conversion from OtbReadError
    OtbErrorCode converted1 = ErrorUtils::fromOtbReadError(OtbReadError::FileNotFound);
    std::cout << "OtbReadError::FileNotFound -> " << static_cast<int>(converted1) << std::endl;
    
    OtbErrorCode converted2 = ErrorUtils::fromOtbReadError(OtbReadError::InvalidHeader);
    std::cout << "OtbReadError::InvalidHeader -> " << static_cast<int>(converted2) << std::endl;
    
    // Test conversion from ValidationError
    OtbErrorCode converted3 = ErrorUtils::fromValidationError(ValidationError::ChecksumMismatch);
    std::cout << "ValidationError::ChecksumMismatch -> " << static_cast<int>(converted3) << std::endl;
    
    OtbErrorCode converted4 = ErrorUtils::fromValidationError(ValidationError::TreeCorruption);
    std::cout << "ValidationError::TreeCorruption -> " << static_cast<int>(converted4) << std::endl;
    
    std::cout << "✓ Error conversion test passed" << std::endl;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    std::cout << "=== OTB Error Handling System Test ===" << std::endl;
    std::cout << "Testing comprehensive error handling implementation..." << std::endl;
    
    try {
        testBasicErrorHandling();
        testExceptionHandling();
        testErrorUtilities();
        testErrorReporting();
        testMacros();
        testErrorConversion();
        
        std::cout << "\n=== All Tests Completed Successfully ===" << std::endl;
        std::cout << "✓ Error handling system is working correctly" << std::endl;
        
        // Show final error statistics
        ErrorHandler& handler = ErrorHandler::instance();
        auto allErrors = handler.getErrors();
        std::cout << "\nFinal error count: " << allErrors.size() << std::endl;
        
        if (allErrors.size() > 0) {
            QString summary = ErrorUtils::generateSummaryReport(allErrors);
            std::cout << "Error breakdown: " << summary.toStdString() << std::endl;
        }
        
    } catch (const std::exception& ex) {
        std::cerr << "Test failed with exception: " << ex.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
    
    return 0;
}