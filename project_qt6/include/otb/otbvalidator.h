#ifndef OTBVALIDATOR_H
#define OTBVALIDATOR_H

#include "otbtypes.h"
#include "binarytree.h"
#include "otberrors.h"
#include <QString>
#include <QByteArray>
#include <QCryptographicHash>
#include <QElapsedTimer>

namespace OTB {

// Validation error types for detailed error reporting
enum class ValidationError {
    None = 0,
    FileNotFound,
    FileAccessDenied,
    InvalidFileSize,
    ChecksumMismatch,
    CorruptedHeader,
    InvalidNodeStructure,
    DataIntegrityFailure,
    StructureInconsistency,
    AttributeValidationFailed,
    VersionMismatch,
    TreeCorruption,
    MemoryConstraintViolation
};

// Validation result with detailed information
struct ValidationResult {
    bool isValid = false;
    ValidationError errorType = ValidationError::None;
    QString errorMessage;
    QString detailedReport;
    QStringList warnings;
    qint64 validationTimeMs = 0;
    
    // File statistics
    qint64 fileSize = 0;
    QString fileChecksum;
    
    // Structure statistics
    quint32 totalNodes = 0;
    quint32 itemNodes = 0;
    quint32 totalAttributes = 0;
    
    // Version information
    quint32 majorVersion = 0;
    quint32 minorVersion = 0;
    quint32 buildNumber = 0;
    
    void reset() {
        isValid = false;
        errorType = ValidationError::None;
        errorMessage.clear();
        detailedReport.clear();
        warnings.clear();
        validationTimeMs = 0;
        fileSize = 0;
        fileChecksum.clear();
        totalNodes = 0;
        itemNodes = 0;
        totalAttributes = 0;
        majorVersion = 0;
        minorVersion = 0;
        buildNumber = 0;
    }
};

// Validation options for customizing validation behavior
struct ValidationOptions {
    bool enableChecksumValidation = true;
    bool enableStructureValidation = true;
    bool enableDataIntegrityChecks = true;
    bool enableAttributeValidation = true;
    bool enableVersionValidation = true;
    bool strictMode = false;  // Fail on warnings in strict mode
    bool generateDetailedReport = true;
    QCryptographicHash::Algorithm checksumAlgorithm = QCryptographicHash::Sha256;
    
    // Performance options
    qint64 maxFileSize = 1024 * 1024 * 1024; // 1GB limit
    quint32 maxNodes = 100000;  // Maximum nodes to validate
    quint32 maxAttributes = 1000000;  // Maximum attributes to validate
};

class OtbValidator {
public:
    OtbValidator();
    
    // Main validation methods
    ValidationResult validateFile(const QString& filePath);
    ValidationResult validateFile(const QString& filePath, const ValidationOptions& options);
    
    // Quick validation for basic file integrity
    bool quickValidate(const QString& filePath, QString& errorString);
    
    // Specific validation methods
    bool validateChecksum(const QString& filePath, QString& checksum, QString& errorString);
    bool validateHeader(const QString& filePath, QString& errorString);
    bool validateStructure(const QString& filePath, QString& errorString);
    bool validateDataIntegrity(const QString& filePath, QString& errorString);
    
    // Advanced validation methods
    bool validateNodeConsistency(const QString& filePath, QString& errorString);
    bool validateAttributeIntegrity(const QString& filePath, QString& errorString);
    bool validateVersionCompatibility(const QString& filePath, QString& errorString);
    
    // Comparison and verification methods
    bool compareFiles(const QString& filePath1, const QString& filePath2, QString& reportString);
    bool verifyFileIntegrity(const QString& filePath, const QString& expectedChecksum, QString& errorString);
    
    // Utility methods
    QString generateFileReport(const QString& filePath);
    QStringList getSupportedVersions() const;
    
    // Configuration methods
    void setValidationOptions(const ValidationOptions& options) { m_options = options; }
    const ValidationOptions& getValidationOptions() const { return m_options; }
    
    // Get last validation result
    const ValidationResult& getLastResult() const { return m_lastResult; }

private:
    // Internal validation methods
    bool validateFileAccess(const QString& filePath, QString& errorString);
    bool validateFileSize(const QString& filePath, QString& errorString);
    bool calculateFileChecksum(const QString& filePath, QString& checksum, QString& errorString);
    
    // Header validation methods
    bool validateHeaderSignature(QDataStream* stream, QString& errorString);
    bool validateHeaderVersion(QDataStream* stream, QString& errorString);
    bool validateHeaderIntegrity(QDataStream* stream, QString& errorString);
    
    // Structure validation methods
    bool validateRootNode(BinaryTree& tree, QString& errorString);
    bool validateItemNodes(BinaryTree& tree, QString& errorString);
    bool validateNodeHierarchy(BinaryTree& tree, QString& errorString);
    bool validateNodeMarkers(BinaryTree& tree, QString& errorString);
    
    // Data integrity validation methods
    bool validateRootAttributes(BinaryTree& tree, QString& errorString);
    bool validateItemAttributes(BinaryTree& tree, QString& errorString);
    bool validateAttributeData(ServerItemAttribute attribute, quint16 dataLen, 
                              const QByteArray& data, QString& errorString);
    bool validateRootAttributeData(RootAttribute attribute, quint16 dataLen, 
                                  const QByteArray& data, QString& errorString);
    
    // Consistency validation methods
    bool validateItemConsistency(const ServerItem& item, QString& errorString);
    bool validateItemFlags(const ServerItem& item, QString& errorString);
    bool validateItemProperties(const ServerItem& item, QString& errorString);
    
    // Error handling and reporting
    void setError(ValidationError errorType, const QString& message);
    void addWarning(const QString& warning);
    void generateDetailedReport();
    
    // Statistics tracking
    void updateStatistics(qint64 fileSize, quint32 nodes, quint32 items, quint32 attributes);
    
    ValidationOptions m_options;
    ValidationResult m_lastResult;
    QElapsedTimer m_timer;
    
    // Validation state
    quint32 m_nodeCount = 0;
    quint32 m_itemCount = 0;
    quint32 m_attributeCount = 0;
    QStringList m_validationLog;
};

// Utility functions for validation
namespace ValidationUtils {
    // Checksum utilities
    QString calculateSha256(const QString& filePath);
    QString calculateMd5(const QString& filePath);
    QString calculateCrc32(const QString& filePath);
    
    // File format detection
    bool isValidOtbFile(const QString& filePath);
    bool detectFileCorruption(const QString& filePath);
    
    // Version utilities
    bool isVersionSupported(quint32 major, quint32 minor, quint32 build);
    QString formatVersion(quint32 major, quint32 minor, quint32 build);
    
    // Data validation utilities
    bool isValidItemId(quint16 id);
    bool isValidClientId(quint16 id);
    bool isValidItemName(const QString& name);
    bool isValidSpriteHash(const QByteArray& hash);
}

} // namespace OTB

#endif // OTBVALIDATOR_H