#include "otbvalidator.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>

using namespace OTB;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== OTB Validation System Test ===";
    
    // Test 1: Validator initialization
    OtbValidator validator;
    qDebug() << "Validator initialized successfully";
    
    // Test 2: Validation options
    ValidationOptions options;
    options.enableChecksumValidation = true;
    options.enableStructureValidation = true;
    options.enableDataIntegrityChecks = true;
    options.strictMode = false;
    
    validator.setValidationOptions(options);
    qDebug() << "Validation options configured";
    
    // Test 3: Utility functions
    QStringList supportedVersions = validator.getSupportedVersions();
    qDebug() << "Supported versions:" << supportedVersions;
    
    // Test 4: ValidationUtils functions
    qDebug() << "Testing ValidationUtils:";
    qDebug() << "  isValidItemId(100):" << ValidationUtils::isValidItemId(100);
    qDebug() << "  isValidItemId(0):" << ValidationUtils::isValidItemId(0);
    qDebug() << "  isValidClientId(1000):" << ValidationUtils::isValidClientId(1000);
    qDebug() << "  isValidItemName('Test Item'):" << ValidationUtils::isValidItemName("Test Item");
    qDebug() << "  isVersionSupported(1,0,0):" << ValidationUtils::isVersionSupported(1, 0, 0);
    qDebug() << "  formatVersion(1,2,3):" << ValidationUtils::formatVersion(1, 2, 3);
    
    // Test 5: Test file validation (if test file exists)
    QString testFile = QDir::currentPath() + "/test.otb";
    if (QFile::exists(testFile)) {
        qDebug() << "Testing file validation with:" << testFile;
        
        ValidationResult result = validator.validateFile(testFile);
        qDebug() << "Validation result:";
        qDebug() << "  Valid:" << result.isValid;
        qDebug() << "  Error:" << result.errorMessage;
        qDebug() << "  Warnings:" << result.warnings.size();
        qDebug() << "  File size:" << result.fileSize;
        qDebug() << "  Validation time:" << result.validationTimeMs << "ms";
        
        if (!result.detailedReport.isEmpty()) {
            qDebug() << "Detailed report:";
            qDebug() << result.detailedReport;
        }
    } else {
        qDebug() << "No test file found at:" << testFile;
        qDebug() << "Skipping file validation test";
    }
    
    // Test 6: Error handling
    QString errorString;
    bool quickResult = validator.quickValidate("/nonexistent/file.otb", errorString);
    qDebug() << "Quick validation of nonexistent file:";
    qDebug() << "  Result:" << quickResult;
    qDebug() << "  Error:" << errorString;
    
    qDebug() << "=== All tests completed ===";
    
    return 0;
}