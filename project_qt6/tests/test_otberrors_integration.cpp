#include "otberrors.h"
#include "otbreader.h"
#include "otbvalidator.h"
#include <QCoreApplication>
#include <QDebug>
#include <iostream>

using namespace OTB;

void demonstrateErrorHandlingIntegration() {
    std::cout << "\n=== OTB Error Handling Integration Demo ===" << std::endl;
    
    ErrorHandler& handler = ErrorHandler::instance();
    handler.clearErrors();
    
    // Simulate OTB file reading with error handling
    std::cout << "\n--- Simulating OTB File Operations ---" << std::endl;
    
    try {
        // Simulate file not found error
        throw FileNotFoundException("test.otb");
    } catch (const OtbException& ex) {
        std::cout << "Caught file error: " << ex.getMessage().toStdString() << std::endl;
        std::cout << "Suggested recovery: ";
        
        switch (ex.getSuggestedRecovery()) {
            case RecoveryStrategy::UserIntervention:
                std::cout << "User should check file path" << std::endl;
                break;
            case RecoveryStrategy::Retry:
                std::cout << "Retry operation" << std::endl;
                break;
            default:
                std::cout << "No specific recovery" << std::endl;
                break;
        }
    }
    
    // Simulate validation errors
    try {
        // Simulate checksum validation failure
        ValidationException checksumError("File checksum", "expected: abc123, actual: def456");
        checksumError.getErrorInfo();  // Access error info
        throw checksumError;
    } catch (const OtbException& ex) {
        std::cout << "Caught validation error: " << ex.getMessage().toStdString() << std::endl;
        std::cout << "Is recoverable: " << (ex.isRecoverable() ? "Yes" : "No") << std::endl;
    }
    
    // Demonstrate error conversion from existing enums
    std::cout << "\n--- Error Code Conversion ---" << std::endl;
    
    OtbErrorCode convertedError1 = ErrorUtils::fromOtbReadError(OtbReadError::InvalidHeader);
    std::cout << "OtbReadError::InvalidHeader -> OtbErrorCode::" << static_cast<int>(convertedError1) << std::endl;
    
    OtbErrorCode convertedError2 = ErrorUtils::fromValidationError(ValidationError::DataIntegrityFailure);
    std::cout << "ValidationError::DataIntegrityFailure -> OtbErrorCode::" << static_cast<int>(convertedError2) << std::endl;
    
    // Demonstrate error categorization
    std::cout << "\n--- Error Categorization ---" << std::endl;
    
    ErrorCategory cat1 = ErrorUtils::categorizeError(OtbErrorCode::FileNotFound);
    ErrorCategory cat2 = ErrorUtils::categorizeError(OtbErrorCode::ChecksumMismatch);
    ErrorCategory cat3 = ErrorUtils::categorizeError(OtbErrorCode::MemoryAllocationFailed);
    
    std::cout << "FileNotFound category: " << static_cast<int>(cat1) << " (FileSystem)" << std::endl;
    std::cout << "ChecksumMismatch category: " << static_cast<int>(cat2) << " (Validation)" << std::endl;
    std::cout << "MemoryAllocationFailed category: " << static_cast<int>(cat3) << " (Memory)" << std::endl;
    
    // Demonstrate recovery strategies
    std::cout << "\n--- Recovery Strategies ---" << std::endl;
    
    RecoveryStrategy strategy1 = ErrorUtils::suggestRecovery(OtbErrorCode::FileAccessDenied);
    RecoveryStrategy strategy2 = ErrorUtils::suggestRecovery(OtbErrorCode::AttributeValidationFailed);
    RecoveryStrategy strategy3 = ErrorUtils::suggestRecovery(OtbErrorCode::MemoryAllocationFailed);
    
    std::cout << "FileAccessDenied recovery: " << static_cast<int>(strategy1) << " (Retry)" << std::endl;
    std::cout << "AttributeValidationFailed recovery: " << static_cast<int>(strategy2) << " (Skip)" << std::endl;
    std::cout << "MemoryAllocationFailed recovery: " << static_cast<int>(strategy3) << " (Abort)" << std::endl;
    
    // Show error handler statistics
    std::cout << "\n--- Error Handler Statistics ---" << std::endl;
    
    auto allErrors = handler.getErrors();
    auto criticalErrors = handler.getErrors(ErrorSeverity::Critical);
    
    std::cout << "Total errors in handler: " << allErrors.size() << std::endl;
    std::cout << "Critical errors: " << criticalErrors.size() << std::endl;
    
    if (handler.hasErrors(ErrorSeverity::Error)) {
        std::cout << "System has errors that need attention" << std::endl;
        
        // Generate summary report
        QString summary = ErrorUtils::generateSummaryReport(allErrors);
        std::cout << "Error summary: " << summary.toStdString() << std::endl;
    }
    
    std::cout << "\n=== Integration Demo Completed ===" << std::endl;
}

void demonstrateUserFriendlyErrorMessages() {
    std::cout << "\n=== User-Friendly Error Messages Demo ===" << std::endl;
    
    // Create various error scenarios
    ErrorInfo fileError(OtbErrorCode::FileNotFound, "Cannot open 'items.otb'", ErrorSeverity::Error);
    fileError.fileName = "items.otb";
    fileError.context = "Loading OTB file";
    fileError.suggestion = "Check if the file exists and you have read permissions";
    fileError.suggestedRecovery = RecoveryStrategy::UserIntervention;
    
    ErrorInfo validationError(OtbErrorCode::ChecksumMismatch, "File integrity check failed", ErrorSeverity::Warning);
    validationError.context = "File validation";
    validationError.suggestion = "The file may be corrupted. Try re-downloading it";
    validationError.suggestedRecovery = RecoveryStrategy::Repair;
    
    ErrorInfo memoryError(OtbErrorCode::MemoryAllocationFailed, "Cannot allocate 512MB for file processing", ErrorSeverity::Critical);
    memoryError.context = "Large file processing";
    memoryError.suggestion = "Close other applications to free memory";
    memoryError.suggestedRecovery = RecoveryStrategy::UserIntervention;
    
    // Format user-friendly messages
    std::cout << "\n--- User-Friendly Error Messages ---" << std::endl;
    
    QString userMessage1 = ErrorUtils::formatUserFriendlyMessage(fileError);
    std::cout << "File Error Message:\n" << userMessage1.toStdString() << std::endl << std::endl;
    
    QString userMessage2 = ErrorUtils::formatUserFriendlyMessage(validationError);
    std::cout << "Validation Error Message:\n" << userMessage2.toStdString() << std::endl << std::endl;
    
    QString userMessage3 = ErrorUtils::formatUserFriendlyMessage(memoryError);
    std::cout << "Memory Error Message:\n" << userMessage3.toStdString() << std::endl << std::endl;
    
    std::cout << "=== User-Friendly Messages Demo Completed ===" << std::endl;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    std::cout << "=== OTB Error Handling Integration Examples ===" << std::endl;
    std::cout << "Demonstrating how the error handling system integrates with OTB components..." << std::endl;
    
    try {
        demonstrateErrorHandlingIntegration();
        demonstrateUserFriendlyErrorMessages();
        
        std::cout << "\n=== All Integration Examples Completed Successfully ===" << std::endl;
        std::cout << "✓ Error handling system successfully integrates with OTB components" << std::endl;
        
    } catch (const std::exception& ex) {
        std::cerr << "Integration demo failed with exception: " << ex.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Integration demo failed with unknown exception" << std::endl;
        return 1;
    }
    
    return 0;
}