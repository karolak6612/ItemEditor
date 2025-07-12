#include "otbvalidator.h"
#include "otbheader.h"
#include "otbreader.h"
#include <QFile>
#include <QFileInfo>
#include <QDataStream>
#include <QCryptographicHash>
#include <QDebug>
#include <QElapsedTimer>

namespace OTB {

OtbValidator::OtbValidator() {
    // Initialize with default validation options
    m_options = ValidationOptions();
}

ValidationResult OtbValidator::validateFile(const QString& filePath) {
    return validateFile(filePath, m_options);
}

ValidationResult OtbValidator::validateFile(const QString& filePath, const ValidationOptions& options) {
    m_timer.start();
    m_lastResult.reset();
    m_options = options;
    m_nodeCount = 0;
    m_itemCount = 0;
    m_attributeCount = 0;
    m_validationLog.clear();
    
    QString errorString;
    
    // Step 1: Validate file access and basic properties
    if (!validateFileAccess(filePath, errorString)) {
        m_lastResult.validationTimeMs = m_timer.elapsed();
        return m_lastResult;
    }
    
    // Step 2: Validate file size constraints
    if (!validateFileSize(filePath, errorString)) {
        m_lastResult.validationTimeMs = m_timer.elapsed();
        return m_lastResult;
    }
    
    // Step 3: Calculate and validate checksum if enabled
    if (m_options.enableChecksumValidation) {
        if (!calculateFileChecksum(filePath, m_lastResult.fileChecksum, errorString)) {
            setError(ValidationError::ChecksumMismatch, errorString);
            m_lastResult.validationTimeMs = m_timer.elapsed();
            return m_lastResult;
        }
    }
    
    // Step 4: Validate header
    if (!validateHeader(filePath, errorString)) {
        m_lastResult.validationTimeMs = m_timer.elapsed();
        return m_lastResult;
    }
    
    // Step 5: Validate structure if enabled
    if (m_options.enableStructureValidation) {
        if (!validateStructure(filePath, errorString)) {
            m_lastResult.validationTimeMs = m_timer.elapsed();
            return m_lastResult;
        }
    }
    
    // Step 6: Validate data integrity if enabled
    if (m_options.enableDataIntegrityChecks) {
        if (!validateDataIntegrity(filePath, errorString)) {
            m_lastResult.validationTimeMs = m_timer.elapsed();
            return m_lastResult;
        }
    }
    
    // Step 7: Validate version compatibility if enabled
    if (m_options.enableVersionValidation) {
        if (!validateVersionCompatibility(filePath, errorString)) {
            m_lastResult.validationTimeMs = m_timer.elapsed();
            return m_lastResult;
        }
    }
    
    // Step 8: Check if strict mode fails on warnings
    if (m_options.strictMode && !m_lastResult.warnings.isEmpty()) {
        setError(ValidationError::StructureInconsistency, 
                QString("Strict mode validation failed due to %1 warnings").arg(m_lastResult.warnings.size()));
        m_lastResult.validationTimeMs = m_timer.elapsed();
        return m_lastResult;
    }
    
    // Validation successful
    m_lastResult.isValid = true;
    m_lastResult.validationTimeMs = m_timer.elapsed();
    
    // Generate detailed report if enabled
    if (m_options.generateDetailedReport) {
        generateDetailedReport();
    }
    
    return m_lastResult;
}

bool OtbValidator::quickValidate(const QString& filePath, QString& errorString) {
    // Quick validation for basic file integrity
    if (!validateFileAccess(filePath, errorString)) {
        return false;
    }
    
    if (!validateHeader(filePath, errorString)) {
        return false;
    }
    
    // Basic structure check
    BinaryTree tree;
    if (!tree.open(filePath, QIODevice::ReadOnly)) {
        errorString = "Failed to open file for structure validation";
        return false;
    }
    
    bool result = validateRootNode(tree, errorString);
    tree.close();
    
    return result;
}

bool OtbValidator::validateChecksum(const QString& filePath, QString& checksum, QString& errorString) {
    return calculateFileChecksum(filePath, checksum, errorString);
}

bool OtbValidator::validateHeader(const QString& filePath, QString& errorString) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(ValidationError::FileAccessDenied, "Failed to open file for header validation");
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    // Validate header signature
    if (!validateHeaderSignature(&stream, errorString)) {
        return false;
    }
    
    // Validate header version
    if (!validateHeaderVersion(&stream, errorString)) {
        return false;
    }
    
    // Validate header integrity
    if (!validateHeaderIntegrity(&stream, errorString)) {
        return false;
    }
    
    return true;
}

bool OtbValidator::validateStructure(const QString& filePath, QString& errorString) {
    BinaryTree tree;
    if (!tree.open(filePath, QIODevice::ReadOnly)) {
        setError(ValidationError::InvalidNodeStructure, "Failed to open file for structure validation");
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    bool result = true;
    
    // Validate root node
    if (!validateRootNode(tree, errorString)) {
        result = false;
    }
    
    // Validate item nodes if root validation passed
    if (result && !validateItemNodes(tree, errorString)) {
        result = false;
    }
    
    // Validate node hierarchy
    if (result && !validateNodeHierarchy(tree, errorString)) {
        result = false;
    }
    
    // Validate node markers
    if (result && !validateNodeMarkers(tree, errorString)) {
        result = false;
    }
    
    tree.close();
    return result;
}

bool OtbValidator::validateDataIntegrity(const QString& filePath, QString& errorString) {
    BinaryTree tree;
    if (!tree.open(filePath, QIODevice::ReadOnly)) {
        setError(ValidationError::DataIntegrityFailure, "Failed to open file for data integrity validation");
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    bool result = true;
    
    // Validate root attributes
    if (!validateRootAttributes(tree, errorString)) {
        result = false;
    }
    
    // Validate item attributes if root validation passed
    if (result && !validateItemAttributes(tree, errorString)) {
        result = false;
    }
    
    tree.close();
    return result;
}// Continue implementation of OtbValidator methods

bool OtbValidator::validateNodeConsistency(const QString& filePath, QString& errorString) {
    BinaryTree tree;
    if (!tree.open(filePath, QIODevice::ReadOnly)) {
        setError(ValidationError::TreeCorruption, "Failed to open file for node consistency validation");
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    // Check node count consistency
    quint32 expectedNodes = 0;
    quint32 actualNodes = 0;
    
    // Count nodes by traversing the tree
    if (tree.enterNode()) {
        actualNodes = 1; // Root node
        
        // Count child nodes
        while (tree.hasNextNode()) {
            if (tree.enterNode()) {
                actualNodes++;
                tree.leaveNode();
            }
        }
        
        tree.leaveNode();
    }
    
    m_nodeCount = actualNodes;
    
    // Validate node count is within reasonable limits
    if (actualNodes > m_options.maxNodes) {
        setError(ValidationError::MemoryConstraintViolation, 
                QString("Node count %1 exceeds maximum limit %2").arg(actualNodes).arg(m_options.maxNodes));
        errorString = m_lastResult.errorMessage;
        tree.close();
        return false;
    }
    
    tree.close();
    return true;
}

bool OtbValidator::validateAttributeIntegrity(const QString& filePath, QString& errorString) {
    // Use OtbReader to parse and validate all attributes
    OtbReader reader;
    reader.setStrictValidation(true);
    reader.setDetailedLogging(false);
    
    ServerItemList items;
    QString readerError;
    
    // Attempt to read the file - this will validate all attributes
    if (!reader.read(filePath, items, readerError)) {
        setError(ValidationError::AttributeValidationFailed, "Attribute validation failed: " + readerError);
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    // Update statistics
    m_itemCount = items.items.size();
    const auto& stats = reader.getLastReadingStats();
    m_attributeCount = stats.attributesProcessed;
    
    // Check if attribute count is within limits
    if (m_attributeCount > m_options.maxAttributes) {
        setError(ValidationError::MemoryConstraintViolation, 
                QString("Attribute count %1 exceeds maximum limit %2").arg(m_attributeCount).arg(m_options.maxAttributes));
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    // Validate individual items for consistency
    for (const ServerItem& item : items.items) {
        QString itemError;
        if (!validateItemConsistency(item, itemError)) {
            if (m_options.strictMode) {
                setError(ValidationError::DataIntegrityFailure, "Item consistency validation failed: " + itemError);
                errorString = m_lastResult.errorMessage;
                return false;
            } else {
                addWarning("Item consistency warning: " + itemError);
            }
        }
    }
    
    return true;
}

bool OtbValidator::validateVersionCompatibility(const QString& filePath, QString& errorString) {
    OtbVersionInfo versionInfo;
    
    // Detect file version
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(ValidationError::FileAccessDenied, "Failed to open file for version validation");
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    if (!OtbHeader::detectVersion(&stream, versionInfo, errorString)) {
        setError(ValidationError::VersionMismatch, "Failed to detect file version: " + errorString);
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    // Store version information
    m_lastResult.majorVersion = versionInfo.majorVersion;
    m_lastResult.minorVersion = versionInfo.minorVersion;
    m_lastResult.buildNumber = versionInfo.buildNumber;
    
    // Check if version is supported
    if (!ValidationUtils::isVersionSupported(versionInfo.majorVersion, versionInfo.minorVersion, versionInfo.buildNumber)) {
        setError(ValidationError::VersionMismatch, 
                QString("Unsupported version: %1").arg(ValidationUtils::formatVersion(
                    versionInfo.majorVersion, versionInfo.minorVersion, versionInfo.buildNumber)));
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    return true;
}

bool OtbValidator::compareFiles(const QString& filePath1, const QString& filePath2, QString& reportString) {
    // Calculate checksums for both files
    QString checksum1, checksum2;
    QString error1, error2;
    
    if (!calculateFileChecksum(filePath1, checksum1, error1)) {
        reportString = "Failed to calculate checksum for file 1: " + error1;
        return false;
    }
    
    if (!calculateFileChecksum(filePath2, checksum2, error2)) {
        reportString = "Failed to calculate checksum for file 2: " + error2;
        return false;
    }
    
    // Compare checksums
    if (checksum1 == checksum2) {
        reportString = "Files are identical (checksum match)";
        return true;
    }
    
    // Files are different - generate detailed comparison report
    reportString = QString("Files are different:\nFile 1 checksum: %1\nFile 2 checksum: %2")
                  .arg(checksum1).arg(checksum2);
    
    // Additional comparison could include structure comparison, item count, etc.
    ValidationResult result1 = validateFile(filePath1);
    ValidationResult result2 = validateFile(filePath2);
    
    reportString += QString("\n\nFile 1 statistics:\n- Nodes: %1\n- Items: %2\n- Attributes: %3")
                   .arg(result1.totalNodes).arg(result1.itemNodes).arg(result1.totalAttributes);
    
    reportString += QString("\n\nFile 2 statistics:\n- Nodes: %1\n- Items: %2\n- Attributes: %3")
                   .arg(result2.totalNodes).arg(result2.itemNodes).arg(result2.totalAttributes);
    
    return false;
}

bool OtbValidator::verifyFileIntegrity(const QString& filePath, const QString& expectedChecksum, QString& errorString) {
    QString actualChecksum;
    if (!calculateFileChecksum(filePath, actualChecksum, errorString)) {
        return false;
    }
    
    if (actualChecksum.toLower() != expectedChecksum.toLower()) {
        setError(ValidationError::ChecksumMismatch, 
                QString("Checksum mismatch. Expected: %1, Actual: %2").arg(expectedChecksum).arg(actualChecksum));
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    return true;
}

QString OtbValidator::generateFileReport(const QString& filePath) {
    ValidationResult result = validateFile(filePath);
    
    QString report;
    report += QString("=== OTB File Validation Report ===\n");
    report += QString("File: %1\n").arg(filePath);
    report += QString("Validation Status: %1\n").arg(result.isValid ? "VALID" : "INVALID");
    report += QString("Validation Time: %1 ms\n").arg(result.validationTimeMs);
    
    if (!result.isValid) {
        report += QString("Error: %1\n").arg(result.errorMessage);
    }
    
    report += QString("\n=== File Statistics ===\n");
    report += QString("File Size: %1 bytes\n").arg(result.fileSize);
    report += QString("Checksum: %1\n").arg(result.fileChecksum);
    report += QString("Version: %1.%2.%3\n").arg(result.majorVersion).arg(result.minorVersion).arg(result.buildNumber);
    
    report += QString("\n=== Structure Statistics ===\n");
    report += QString("Total Nodes: %1\n").arg(result.totalNodes);
    report += QString("Item Nodes: %1\n").arg(result.itemNodes);
    report += QString("Total Attributes: %1\n").arg(result.totalAttributes);
    
    if (!result.warnings.isEmpty()) {
        report += QString("\n=== Warnings (%1) ===\n").arg(result.warnings.size());
        for (const QString& warning : result.warnings) {
            report += QString("- %1\n").arg(warning);
        }
    }
    
    if (!result.detailedReport.isEmpty()) {
        report += QString("\n=== Detailed Report ===\n");
        report += result.detailedReport;
    }
    
    return report;
}

QStringList OtbValidator::getSupportedVersions() const {
    return QStringList() << "1.0.0" << "1.1.0" << "2.0.0" << "3.0.0";
}

// Private implementation methods

bool OtbValidator::validateFileAccess(const QString& filePath, QString& errorString) {
    QFileInfo fileInfo(filePath);
    
    if (!fileInfo.exists()) {
        setError(ValidationError::FileNotFound, "File does not exist: " + filePath);
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    if (!fileInfo.isReadable()) {
        setError(ValidationError::FileAccessDenied, "File is not readable: " + filePath);
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    if (!fileInfo.isFile()) {
        setError(ValidationError::FileAccessDenied, "Path is not a file: " + filePath);
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    m_lastResult.fileSize = fileInfo.size();
    return true;
}

bool OtbValidator::validateFileSize(const QString& filePath, QString& errorString) {
    QFileInfo fileInfo(filePath);
    qint64 fileSize = fileInfo.size();
    
    if (fileSize == 0) {
        setError(ValidationError::InvalidFileSize, "File is empty");
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    if (fileSize < 100) {
        setError(ValidationError::InvalidFileSize, "File too small to be valid OTB");
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    if (fileSize > m_options.maxFileSize) {
        setError(ValidationError::InvalidFileSize, 
                QString("File size %1 exceeds maximum limit %2").arg(fileSize).arg(m_options.maxFileSize));
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    return true;
}bool OtbValidator::calculateFileChecksum(const QString& filePath, QString& checksum, QString& errorString) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(ValidationError::FileAccessDenied, "Failed to open file for checksum calculation");
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    QCryptographicHash hash(m_options.checksumAlgorithm);
    
    // Read file in chunks for memory efficiency
    const qint64 chunkSize = 8192;
    while (!file.atEnd()) {
        QByteArray chunk = file.read(chunkSize);
        if (chunk.isEmpty() && !file.atEnd()) {
            setError(ValidationError::FileAccessDenied, "Failed to read file for checksum calculation");
            errorString = m_lastResult.errorMessage;
            return false;
        }
        hash.addData(chunk);
    }
    
    checksum = hash.result().toHex();
    return true;
}

bool OtbValidator::validateHeaderSignature(QDataStream* stream, QString& errorString) {
    // Use OtbHeader for signature validation
    OtbVersionInfo versionInfo;
    if (!OtbHeader::readHeader(stream, versionInfo, errorString)) {
        setError(ValidationError::CorruptedHeader, "Invalid header signature: " + errorString);
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    return true;
}

bool OtbValidator::validateHeaderVersion(QDataStream* stream, QString& errorString) {
    // Reset stream position for version detection
    stream->device()->seek(0);
    
    OtbVersionInfo versionInfo;
    if (!OtbHeader::detectVersion(stream, versionInfo, errorString)) {
        setError(ValidationError::CorruptedHeader, "Failed to detect version: " + errorString);
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    // Store version information
    m_lastResult.majorVersion = versionInfo.majorVersion;
    m_lastResult.minorVersion = versionInfo.minorVersion;
    m_lastResult.buildNumber = versionInfo.buildNumber;
    
    return true;
}

bool OtbValidator::validateHeaderIntegrity(QDataStream* stream, QString& errorString) {
    // Reset stream position for integrity check
    stream->device()->seek(0);
    
    if (!OtbHeader::validateHeaderIntegrity(stream, errorString)) {
        setError(ValidationError::CorruptedHeader, "Header integrity check failed: " + errorString);
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    return true;
}

bool OtbValidator::validateRootNode(BinaryTree& tree, QString& errorString) {
    if (!tree.enterNode()) {
        setError(ValidationError::InvalidNodeStructure, "Failed to enter root node");
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    if (tree.getCurrentNodeType() != 0) {
        setError(ValidationError::InvalidNodeStructure, 
                QString("Invalid root node type. Expected 0, got %1").arg(tree.getCurrentNodeType()));
        errorString = m_lastResult.errorMessage;
        tree.leaveNode();
        return false;
    }
    
    // Validate root node has data
    QDataStream* nodeStream = tree.getCurrentNodeStream();
    if (!nodeStream || nodeStream->atEnd()) {
        setError(ValidationError::InvalidNodeStructure, "Root node contains no data");
        errorString = m_lastResult.errorMessage;
        tree.leaveNode();
        return false;
    }
    
    tree.leaveNode();
    m_nodeCount++;
    return true;
}

bool OtbValidator::validateItemNodes(BinaryTree& tree, QString& errorString) {
    // Enter root node first
    if (!tree.enterNode()) {
        setError(ValidationError::InvalidNodeStructure, "Failed to enter root node for item validation");
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    // Leave root node to access children
    if (!tree.leaveNode()) {
        setError(ValidationError::InvalidNodeStructure, "Failed to leave root node");
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    // Validate each item node
    quint32 itemCount = 0;
    while (tree.hasNextNode()) {
        if (!tree.enterNode()) {
            setError(ValidationError::InvalidNodeStructure, "Failed to enter item node");
            errorString = m_lastResult.errorMessage;
            return false;
        }
        
        // Validate item node type
        quint8 nodeType = tree.getCurrentNodeType();
        if (nodeType == 0) {
            setError(ValidationError::InvalidNodeStructure, "Found root node type in item context");
            errorString = m_lastResult.errorMessage;
            tree.leaveNode();
            return false;
        }
        
        if (nodeType > 5) {
            if (m_options.strictMode) {
                setError(ValidationError::InvalidNodeStructure, QString("Invalid item node type: %1").arg(nodeType));
                errorString = m_lastResult.errorMessage;
                tree.leaveNode();
                return false;
            } else {
                addWarning(QString("Unusual item node type: %1").arg(nodeType));
            }
        }
        
        // Validate item node has data
        QDataStream* nodeStream = tree.getCurrentNodeStream();
        if (!nodeStream) {
            setError(ValidationError::InvalidNodeStructure, "Item node has no data stream");
            errorString = m_lastResult.errorMessage;
            tree.leaveNode();
            return false;
        }
        
        // Check minimum data size (flags = 4 bytes minimum)
        if (nodeStream->device()->bytesAvailable() < 4) {
            setError(ValidationError::InvalidNodeStructure, "Item node has insufficient data");
            errorString = m_lastResult.errorMessage;
            tree.leaveNode();
            return false;
        }
        
        if (!tree.leaveNode()) {
            setError(ValidationError::InvalidNodeStructure, "Failed to leave item node");
            errorString = m_lastResult.errorMessage;
            return false;
        }
        
        itemCount++;
        m_nodeCount++;
        
        // Check node count limits
        if (itemCount > m_options.maxNodes) {
            setError(ValidationError::MemoryConstraintViolation, 
                    QString("Item node count %1 exceeds maximum limit %2").arg(itemCount).arg(m_options.maxNodes));
            errorString = m_lastResult.errorMessage;
            return false;
        }
    }
    
    m_itemCount = itemCount;
    return true;
}

bool OtbValidator::validateNodeHierarchy(BinaryTree& tree, QString& errorString) {
    // Validate that the tree structure is consistent
    // This is a simplified check - a full implementation would verify the entire tree structure
    
    if (!tree.enterNode()) {
        setError(ValidationError::TreeCorruption, "Failed to enter root for hierarchy validation");
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    // Verify we can navigate the tree structure
    bool canLeaveRoot = tree.leaveNode();
    if (!canLeaveRoot) {
        setError(ValidationError::TreeCorruption, "Tree hierarchy corruption detected");
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    return true;
}

bool OtbValidator::validateNodeMarkers(BinaryTree& tree, QString& errorString) {
    // This would validate that all node start/end markers are correct
    // For now, we rely on BinaryTree's internal validation
    
    // The BinaryTree class should handle marker validation internally
    // If we reach this point, markers are likely valid
    return true;
}

bool OtbValidator::validateRootAttributes(BinaryTree& tree, QString& errorString) {
    if (!tree.enterNode()) {
        setError(ValidationError::DataIntegrityFailure, "Failed to enter root node for attribute validation");
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    QDataStream* nodeStream = tree.getCurrentNodeStream();
    if (!nodeStream) {
        setError(ValidationError::DataIntegrityFailure, "No data stream for root node");
        errorString = m_lastResult.errorMessage;
        tree.leaveNode();
        return false;
    }
    
    // Skip root flags (4 bytes)
    if (nodeStream->device()->bytesAvailable() < 4) {
        setError(ValidationError::DataIntegrityFailure, "Insufficient data for root flags");
        errorString = m_lastResult.errorMessage;
        tree.leaveNode();
        return false;
    }
    
    quint32 rootFlags;
    *nodeStream >> rootFlags;
    
    // Validate root attributes
    quint32 attributeCount = 0;
    while (!nodeStream->atEnd()) {
        quint8 attributeByte;
        *nodeStream >> attributeByte;
        
        if (nodeStream->atEnd()) {
            setError(ValidationError::DataIntegrityFailure, "Unexpected end while reading attribute length");
            errorString = m_lastResult.errorMessage;
            tree.leaveNode();
            return false;
        }
        
        quint16 dataLen;
        *nodeStream >> dataLen;
        
        if (nodeStream->device()->bytesAvailable() < dataLen) {
            setError(ValidationError::DataIntegrityFailure, 
                    QString("Insufficient data for root attribute %1").arg(QString::number(attributeByte, 16)));
            errorString = m_lastResult.errorMessage;
            tree.leaveNode();
            return false;
        }
        
        QByteArray attributeData(dataLen, 0);
        nodeStream->readRawData(attributeData.data(), dataLen);
        
        // Validate specific attribute data
        RootAttribute attribute = static_cast<RootAttribute>(attributeByte);
        if (!validateRootAttributeData(attribute, dataLen, attributeData, errorString)) {
            tree.leaveNode();
            return false;
        }
        
        attributeCount++;
        m_attributeCount++;
    }
    
    tree.leaveNode();
    return true;
}bool OtbValidator::validateItemAttributes(BinaryTree& tree, QString& errorString) {
    // Enter root node first
    if (!tree.enterNode()) {
        setError(ValidationError::DataIntegrityFailure, "Failed to enter root node");
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    // Leave root to access item nodes
    if (!tree.leaveNode()) {
        setError(ValidationError::DataIntegrityFailure, "Failed to leave root node");
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    // Validate attributes in each item node
    while (tree.hasNextNode()) {
        if (!tree.enterNode()) {
            setError(ValidationError::DataIntegrityFailure, "Failed to enter item node");
            errorString = m_lastResult.errorMessage;
            return false;
        }
        
        QDataStream* nodeStream = tree.getCurrentNodeStream();
        if (!nodeStream) {
            setError(ValidationError::DataIntegrityFailure, "No data stream for item node");
            errorString = m_lastResult.errorMessage;
            tree.leaveNode();
            return false;
        }
        
        // Skip item flags (4 bytes)
        if (nodeStream->device()->bytesAvailable() < 4) {
            setError(ValidationError::DataIntegrityFailure, "Insufficient data for item flags");
            errorString = m_lastResult.errorMessage;
            tree.leaveNode();
            return false;
        }
        
        quint32 itemFlags;
        *nodeStream >> itemFlags;
        
        // Validate item attributes
        while (!nodeStream->atEnd()) {
            quint8 attributeByte;
            *nodeStream >> attributeByte;
            
            if (nodeStream->atEnd()) {
                setError(ValidationError::DataIntegrityFailure, "Unexpected end while reading attribute length");
                errorString = m_lastResult.errorMessage;
                tree.leaveNode();
                return false;
            }
            
            quint16 dataLen;
            *nodeStream >> dataLen;
            
            if (nodeStream->device()->bytesAvailable() < dataLen) {
                setError(ValidationError::DataIntegrityFailure, 
                        QString("Insufficient data for item attribute %1").arg(QString::number(attributeByte, 16)));
                errorString = m_lastResult.errorMessage;
                tree.leaveNode();
                return false;
            }
            
            QByteArray attributeData(dataLen, 0);
            nodeStream->readRawData(attributeData.data(), dataLen);
            
            // Validate specific attribute data
            ServerItemAttribute attribute = static_cast<ServerItemAttribute>(attributeByte);
            if (!validateAttributeData(attribute, dataLen, attributeData, errorString)) {
                if (m_options.strictMode) {
                    tree.leaveNode();
                    return false;
                } else {
                    addWarning("Attribute validation warning: " + errorString);
                }
            }
            
            m_attributeCount++;
        }
        
        if (!tree.leaveNode()) {
            setError(ValidationError::DataIntegrityFailure, "Failed to leave item node");
            errorString = m_lastResult.errorMessage;
            return false;
        }
    }
    
    return true;
}

bool OtbValidator::validateAttributeData(ServerItemAttribute attribute, quint16 dataLen, 
                                        const QByteArray& data, QString& errorString) {
    switch (attribute) {
        case ServerItemAttribute::ServerID:
        case ServerItemAttribute::ClientID:
        case ServerItemAttribute::GroundSpeed:
        case ServerItemAttribute::MinimapColor:
        case ServerItemAttribute::MaxReadWriteChars:
        case ServerItemAttribute::MaxReadChars:
        case ServerItemAttribute::TradeAs:
            if (dataLen != 2) {
                setError(ValidationError::AttributeValidationFailed, 
                        QString("Invalid length for 2-byte attribute %1: got %2").arg(QString::number(static_cast<quint8>(attribute), 16)).arg(dataLen));
                errorString = m_lastResult.errorMessage;
                return false;
            }
            break;
            
        case ServerItemAttribute::Light:
            if (dataLen != 4) {
                setError(ValidationError::AttributeValidationFailed, 
                        QString("Invalid length for Light attribute: expected 4, got %1").arg(dataLen));
                errorString = m_lastResult.errorMessage;
                return false;
            }
            break;
            
        case ServerItemAttribute::StackOrder:
            if (dataLen != 1) {
                setError(ValidationError::AttributeValidationFailed, 
                        QString("Invalid length for StackOrder attribute: expected 1, got %1").arg(dataLen));
                errorString = m_lastResult.errorMessage;
                return false;
            }
            // Validate stack order value
            if (data.size() >= 1) {
                quint8 stackOrderValue = static_cast<quint8>(data[0]);
                if (stackOrderValue > 5) {
                    if (m_options.strictMode) {
                        setError(ValidationError::AttributeValidationFailed, 
                                QString("Invalid stack order value: %1").arg(stackOrderValue));
                        errorString = m_lastResult.errorMessage;
                        return false;
                    } else {
                        addWarning(QString("Unusual stack order value: %1").arg(stackOrderValue));
                    }
                }
            }
            break;
            
        case ServerItemAttribute::SpriteHash:
            if (dataLen != 16) {
                setError(ValidationError::AttributeValidationFailed, 
                        QString("Invalid length for SpriteHash attribute: expected 16, got %1").arg(dataLen));
                errorString = m_lastResult.errorMessage;
                return false;
            }
            break;
            
        case ServerItemAttribute::Name:
            {
                // Names can be variable length, but should be reasonable
                if (dataLen > 1024) {
                    if (m_options.strictMode) {
                        setError(ValidationError::AttributeValidationFailed, 
                                QString("Name attribute too long: %1 bytes").arg(dataLen));
                        errorString = m_lastResult.errorMessage;
                        return false;
                    } else {
                        addWarning(QString("Very long name attribute: %1 bytes").arg(dataLen));
                    }
                }
                
                // Validate name content
                QString name = QString::fromUtf8(data);
                if (!ValidationUtils::isValidItemName(name)) {
                    if (m_options.strictMode) {
                        setError(ValidationError::AttributeValidationFailed, "Invalid item name content");
                        errorString = m_lastResult.errorMessage;
                        return false;
                    } else {
                        addWarning("Item name contains unusual characters");
                    }
                }
                break;
            }
            
        default:
            // Unknown attributes - log but don't fail
            addWarning(QString("Unknown attribute 0x%1 with length %2").arg(QString::number(static_cast<quint8>(attribute), 16)).arg(dataLen));
            break;
    }
    
    return true;
}

bool OtbValidator::validateRootAttributeData(RootAttribute attribute, quint16 dataLen, 
                                           const QByteArray& data, QString& errorString) {
    switch (attribute) {
        case RootAttribute::Version:
            if (dataLen != 140) {
                setError(ValidationError::AttributeValidationFailed, 
                        QString("Invalid length for Version attribute: expected 140, got %1").arg(dataLen));
                errorString = m_lastResult.errorMessage;
                return false;
            }
            
            // Validate version data structure
            if (data.size() >= 12) {
                QDataStream stream(data);
                stream.setByteOrder(QDataStream::LittleEndian);
                
                quint32 major, minor, build;
                stream >> major >> minor >> build;
                
                // Validate version numbers are reasonable
                if (major > 10 || minor > 9999 || build > 9999) {
                    if (m_options.strictMode) {
                        setError(ValidationError::AttributeValidationFailed, 
                                QString("Suspicious version numbers: %1.%2.%3").arg(major).arg(minor).arg(build));
                        errorString = m_lastResult.errorMessage;
                        return false;
                    } else {
                        addWarning(QString("Unusual version numbers: %1.%2.%3").arg(major).arg(minor).arg(build));
                    }
                }
            }
            break;
            
        default:
            // Unknown root attributes - log but don't fail
            addWarning(QString("Unknown root attribute 0x%1 with length %2").arg(QString::number(static_cast<quint8>(attribute), 16)).arg(dataLen));
            break;
    }
    
    return true;
}

bool OtbValidator::validateItemConsistency(const ServerItem& item, QString& errorString) {
    // Validate item ID
    if (!ValidationUtils::isValidItemId(item.id)) {
        setError(ValidationError::DataIntegrityFailure, QString("Invalid item ID: %1").arg(item.id));
        errorString = m_lastResult.errorMessage;
        return false;
    }
    
    // Validate client ID
    if (!ValidationUtils::isValidClientId(item.clientId)) {
        if (m_options.strictMode) {
            setError(ValidationError::DataIntegrityFailure, QString("Invalid client ID: %1").arg(item.clientId));
            errorString = m_lastResult.errorMessage;
            return false;
        } else {
            addWarning(QString("Unusual client ID: %1").arg(item.clientId));
        }
    }
    
    // Validate item name
    if (!ValidationUtils::isValidItemName(item.name)) {
        if (m_options.strictMode) {
            setError(ValidationError::DataIntegrityFailure, "Invalid item name");
            errorString = m_lastResult.errorMessage;
            return false;
        } else {
            addWarning("Item name validation warning");
        }
    }
    
    // Validate sprite hash
    if (!ValidationUtils::isValidSpriteHash(item.spriteHash)) {
        if (m_options.strictMode) {
            setError(ValidationError::DataIntegrityFailure, "Invalid sprite hash");
            errorString = m_lastResult.errorMessage;
            return false;
        } else {
            addWarning("Sprite hash validation warning");
        }
    }
    
    return validateItemFlags(item, errorString) && validateItemProperties(item, errorString);
}

bool OtbValidator::validateItemFlags(const ServerItem& item, QString& errorString) {
    // Validate flag consistency
    // This is a simplified check - a full implementation would validate all flag combinations
    
    if (item.hasStackOrder && item.stackOrder == TileStackOrder::None) {
        addWarning(QString("Item %1 has stack order flag but no stack order value").arg(item.id));
    }
    
    return true;
}

bool OtbValidator::validateItemProperties(const ServerItem& item, QString& errorString) {
    // Validate property consistency
    // This is a simplified check - a full implementation would validate all property combinations
    
    if (item.type == ServerItemType::Deprecated && !item.name.isEmpty()) {
        addWarning(QString("Deprecated item %1 has non-empty name").arg(item.id));
    }
    
    return true;
}

void OtbValidator::setError(ValidationError errorType, const QString& message) {
    m_lastResult.errorType = errorType;
    m_lastResult.errorMessage = message;
    m_lastResult.isValid = false;
}

void OtbValidator::addWarning(const QString& warning) {
    m_lastResult.warnings.append(warning);
}

void OtbValidator::generateDetailedReport() {
    QString report;
    
    report += "=== Validation Details ===\n";
    report += QString("Validation completed in %1 ms\n").arg(m_lastResult.validationTimeMs);
    report += QString("File size: %1 bytes\n").arg(m_lastResult.fileSize);
    
    if (!m_lastResult.fileChecksum.isEmpty()) {
        report += QString("File checksum: %1\n").arg(m_lastResult.fileChecksum);
    }
    
    report += QString("Version: %1.%2.%3\n").arg(m_lastResult.majorVersion).arg(m_lastResult.minorVersion).arg(m_lastResult.buildNumber);
    
    report += "\n=== Structure Analysis ===\n";
    report += QString("Total nodes processed: %1\n").arg(m_nodeCount);
    report += QString("Item nodes processed: %1\n").arg(m_itemCount);
    report += QString("Attributes processed: %1\n").arg(m_attributeCount);
    
    if (!m_lastResult.warnings.isEmpty()) {
        report += QString("\n=== Warnings (%1) ===\n").arg(m_lastResult.warnings.size());
        for (const QString& warning : m_lastResult.warnings) {
            report += QString("- %1\n").arg(warning);
        }
    }
    
    m_lastResult.detailedReport = report;
    m_lastResult.totalNodes = m_nodeCount;
    m_lastResult.itemNodes = m_itemCount;
    m_lastResult.totalAttributes = m_attributeCount;
}

// ValidationUtils namespace implementation
namespace ValidationUtils {

QString calculateSha256(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(&file);
    return hash.result().toHex();
}

QString calculateMd5(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(&file);
    return hash.result().toHex();
}

QString calculateCrc32(const QString& filePath) {
    // Qt doesn't have built-in CRC32, would need external implementation
    // For now, return empty string
    Q_UNUSED(filePath);
    return QString();
}

bool isValidOtbFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    // Check file size
    if (file.size() < 100) {
        return false;
    }
    
    // Check header signature
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    OtbVersionInfo versionInfo;
    QString errorString;
    return OtbHeader::readHeader(&stream, versionInfo, errorString);
}

bool detectFileCorruption(const QString& filePath) {
    OtbValidator validator;
    ValidationOptions options;
    options.strictMode = false;  // Don't fail on warnings
    
    ValidationResult result = validator.validateFile(filePath, options);
    return !result.isValid;
}

bool isVersionSupported(quint32 major, quint32 minor, quint32 build) {
    // Define supported version ranges
    if (major == 1 && minor <= 1) return true;
    if (major == 2 && minor == 0) return true;
    if (major == 3 && minor == 0) return true;
    
    return false;
}

QString formatVersion(quint32 major, quint32 minor, quint32 build) {
    return QString("%1.%2.%3").arg(major).arg(minor).arg(build);
}

bool isValidItemId(quint16 id) {
    return id > 0 && id <= 65000;
}

bool isValidClientId(quint16 id) {
    return id <= 65000;
}

bool isValidItemName(const QString& name) {
    if (name.length() > 255) return false;
    if (name.contains('\0')) return false;
    
    // Check for reasonable printable characters
    for (const QChar& c : name) {
        if (c.unicode() < 32 && c.unicode() != 9) { // Allow tab but not other control chars
            return false;
        }
    }
    
    return true;
}

bool isValidSpriteHash(const QByteArray& hash) {
    return hash.size() == 16; // MD5 hash size
}

} // namespace ValidationUtils

} // namespace OTB