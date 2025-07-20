#include "OtbFileValidator.h"
#include "OtbReader.h"
#include <QFile>
#include <QFileInfo>
#include <QDataStream>
#include <QCryptographicHash>
#include <QDebug>

OtbFileValidator::OtbFileValidator()
{
    // Initialize with default settings
}

OtbFileValidator::~OtbFileValidator()
{
}

OtbFileValidator::ValidationResult OtbFileValidator::validateFile(const QString& filePath, ValidationLevel level)
{
    ValidationResult result;
    result.level = level;
    
    // Check if file exists
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        addError(result, "FILE_NOT_FOUND", filePath);
        return result;
    }
    
    if (!fileInfo.isReadable()) {
        addError(result, "FILE_ACCESS_DENIED", filePath);
        return result;
    }
    
    result.fileSize = fileInfo.size();
    
    // Check file size limits
    if (result.fileSize > m_settings.maxFileSize) {
        addError(result, "FILE_TOO_LARGE", QString("File size: %1 bytes").arg(result.fileSize));
        return result;
    }
    
    if (result.fileSize < 32) {
        addError(result, "FILE_TOO_SMALL", QString("File size: %1 bytes").arg(result.fileSize));
        return result;
    }
    
    // Read file data
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        addError(result, "FILE_READ_ERROR", file.errorString());
        return result;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    if (data.size() != result.fileSize) {
        addError(result, "FILE_READ_INCOMPLETE", 
                QString("Expected %1 bytes, read %2 bytes").arg(result.fileSize).arg(data.size()));
        return result;
    }
    
    return validateData(data, level);
}

OtbFileValidator::ValidationResult OtbFileValidator::validateData(const QByteArray& data, ValidationLevel level)
{
    ValidationResult result;
    result.level = level;
    result.fileSize = data.size();
    
    // Basic validation
    if (!validateFileHeader(data, result)) {
        return result;
    }
    
    if (!validateVersionInfo(data, result)) {
        return result;
    }
    
    if (level >= ValidationLevel::Standard) {
        if (!validateItemRange(data, result)) {
            return result;
        }
        
        if (!validateItemData(data, result)) {
            return result;
        }
    }
    
    if (level >= ValidationLevel::Thorough) {
        if (!validateDataIntegrity(data, result)) {
            return result;
        }
        
        // Corruption detection
        if (m_settings.checkForCorruption) {
            result.hasCorruption = !checkFileStructure(data) || !checkDataConsistency(data);
            if (result.hasCorruption) {
                result.corruptionDetails = m_corruptionDetails;
                addWarning(result, "CORRUPTION_DETECTED", "File may be corrupted");
            }
        }
    }
    
    if (level >= ValidationLevel::Paranoid) {
        // Additional paranoid checks
        if (!checkChecksums(data)) {
            addError(result, "CHECKSUM_MISMATCH", "Data integrity check failed");
            return result;
        }
    }
    
    result.isValid = result.errors.isEmpty();
    return result;
}

OtbFileValidator::ValidationResult OtbFileValidator::validateItems(const ServerItemList& items, ValidationLevel level)
{
    ValidationResult result;
    result.level = level;
    result.itemCount = items.size();
    result.versionInfo = items.versionInfo;
    result.itemRange = items.itemRange;
    
    if (items.isEmpty()) {
        addWarning(result, "EMPTY_ITEM_LIST", "No items to validate");
        result.isValid = true;
        return result;
    }
    
    // Check item count limits
    if (result.itemCount > m_settings.maxItemCount) {
        addError(result, "TOO_MANY_ITEMS", QString("Item count: %1").arg(result.itemCount));
        return result;
    }
    
    // Validate individual items
    for (const ServerItem& item : items) {
        if (!validateItem(item, result)) {
            if (level < ValidationLevel::Thorough) {
                // Stop on first error for basic/standard validation
                return result;
            }
        }
    }
    
    // Check for consistency issues
    if (level >= ValidationLevel::Standard) {
        validateItemConsistency(items, result);
    }
    
    result.isValid = result.errors.isEmpty();
    return result;
}

bool OtbFileValidator::isValidOtbFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray header = file.read(16);
    file.close();
    
    return isValidOtbData(header);
}

bool OtbFileValidator::isValidOtbData(const QByteArray& data)
{
    return hasValidSignature(data) && hasValidVersion(data);
}

bool OtbFileValidator::hasValidSignature(const QByteArray& data)
{
    if (data.size() < 4) {
        return false;
    }
    
    quint32 signature;
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream >> signature;
    
    return signature == 0x00000000; // OTB signature
}

bool OtbFileValidator::hasValidVersion(const QByteArray& data)
{
    if (data.size() < 8) {
        return false;
    }
    
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    quint32 signature, version;
    stream >> signature >> version;
    
    return version >= 1 && version <= 3; // Valid version range
}

bool OtbFileValidator::detectCorruption(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return true; // Can't read = corrupted
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    return detectCorruption(data);
}

bool OtbFileValidator::detectCorruption(const QByteArray& data)
{
    m_corruptionDetails.clear();
    
    bool hasCorruption = false;
    
    // Check file structure
    if (!checkFileStructure(data)) {
        hasCorruption = true;
    }
    
    // Check data consistency
    if (!checkDataConsistency(data)) {
        hasCorruption = true;
    }
    
    return hasCorruption;
}

QStringList OtbFileValidator::getCorruptionDetails() const
{
    return m_corruptionDetails;
}

QStringList OtbFileValidator::getRecoverySuggestions(const ValidationResult& result)
{
    QStringList suggestions;
    
    if (result.hasCorruption) {
        suggestions.append("Try opening a backup copy of the file");
        suggestions.append("Re-download the file from the original source");
        suggestions.append("Use file recovery tools if available");
    }
    
    if (result.errors.contains("FILE_ACCESS_DENIED")) {
        suggestions.append("Check file permissions");
        suggestions.append("Close other applications that might be using the file");
        suggestions.append("Run the application as administrator");
    }
    
    if (result.errors.contains("INVALID_VERSION")) {
        suggestions.append("This file may be from an unsupported version");
        suggestions.append("Try using a different version of the application");
    }
    
    if (result.errors.contains("TOO_MANY_ITEMS")) {
        suggestions.append("The file contains too many items for this version");
        suggestions.append("Try splitting the file into smaller parts");
    }
    
    suggestions.append(result.suggestions);
    
    return suggestions;
}

QStringList OtbFileValidator::getCommonSolutions()
{
    return {
        "Verify the file is not corrupted",
        "Check file permissions and access rights",
        "Ensure sufficient disk space is available",
        "Close other applications using the file",
        "Try running as administrator",
        "Use a backup copy if available",
        "Re-download the file from the source",
        "Check for file system errors",
        "Restart the application",
        "Contact technical support if problems persist"
    };
}

QString OtbFileValidator::getCompatibleErrorMessage(const QString& errorType, const QString& details)
{
    return mapToLegacyError(errorType, details);
}

OtbFileValidator::ValidationSettings OtbFileValidator::getSettings() const
{
    return m_settings;
}

void OtbFileValidator::setSettings(const ValidationSettings& settings)
{
    m_settings = settings;
}

bool OtbFileValidator::validateFileHeader(const QByteArray& data, ValidationResult& result)
{
    if (data.size() < 4) {
        addError(result, "INVALID_FILE_HEADER", "File too small for valid header");
        return false;
    }
    
    if (m_settings.checkFileSignature && !hasValidSignature(data)) {
        addError(result, "INVALID_SIGNATURE", "File does not have a valid OTB signature");
        return false;
    }
    
    return true;
}

bool OtbFileValidator::validateVersionInfo(const QByteArray& data, ValidationResult& result)
{
    if (data.size() < 20) {
        addError(result, "INVALID_VERSION_INFO", "File too small for version information");
        return false;
    }
    
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    quint32 signature, majorVersion, minorVersion, buildNumber, clientVersion;
    stream >> signature >> majorVersion >> minorVersion >> buildNumber >> clientVersion;
    
    result.versionInfo.majorVersion = majorVersion;
    result.versionInfo.minorVersion = minorVersion;
    result.versionInfo.buildNumber = buildNumber;
    result.versionInfo.clientVersion = clientVersion;
    
    if (m_settings.checkVersionCompatibility) {
        if (majorVersion < 1 || majorVersion > 3) {
            addError(result, "UNSUPPORTED_VERSION", 
                    QString("Major version %1 is not supported").arg(majorVersion));
            return false;
        }
        
        if (clientVersion < 800 || clientVersion > 1077) {
            addWarning(result, "UNUSUAL_CLIENT_VERSION", 
                      QString("Client version %1 may not be supported").arg(clientVersion));
        }
    }
    
    return true;
}

bool OtbFileValidator::validateItemRange(const QByteArray& data, ValidationResult& result)
{
    if (data.size() < 24) {
        addError(result, "INVALID_ITEM_RANGE", "File too small for item range information");
        return false;
    }
    
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.skipRawData(20); // Skip version info
    
    quint16 minId, maxId;
    stream >> minId >> maxId;
    
    result.itemRange.minId = minId;
    result.itemRange.maxId = maxId;
    
    if (m_settings.checkItemRange) {
        if (minId > maxId) {
            addError(result, "INVALID_ITEM_RANGE", 
                    QString("Invalid range: %1-%2").arg(minId).arg(maxId));
            return false;
        }
        
        if (maxId > 65535) {
            addError(result, "ITEM_RANGE_TOO_LARGE", 
                    QString("Maximum ID %1 exceeds limit").arg(maxId));
            return false;
        }
        
        if (minId == 0 && maxId == 0) {
            addWarning(result, "EMPTY_ITEM_RANGE", "Item range is empty");
        }
    }
    
    return true;
}

bool OtbFileValidator::validateItemData(const QByteArray& data, ValidationResult& result)
{
    if (data.size() < 28) {
        addError(result, "INVALID_ITEM_DATA", "File too small for item data");
        return false;
    }
    
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.skipRawData(24); // Skip header and range
    
    quint32 itemCount;
    stream >> itemCount;
    
    result.itemCount = itemCount;
    
    if (m_settings.checkItemData) {
        if (itemCount > m_settings.maxItemCount) {
            addError(result, "TOO_MANY_ITEMS", 
                    QString("Item count %1 exceeds maximum %2").arg(itemCount).arg(m_settings.maxItemCount));
            return false;
        }
        
        // Estimate minimum required file size
        qint64 minRequiredSize = 28 + (itemCount * 50); // Rough estimate
        if (data.size() < minRequiredSize) {
            addError(result, "FILE_TOO_SMALL_FOR_ITEMS", 
                    QString("File size %1 too small for %2 items").arg(data.size()).arg(itemCount));
            return false;
        }
    }
    
    return true;
}

bool OtbFileValidator::validateDataIntegrity(const QByteArray& data, ValidationResult& result)
{
    Q_UNUSED(result)
    
    // Try to parse the entire file to check for structural integrity
    try {
        OtbReader reader;
        if (!reader.readFromData(data)) {
            addError(result, "DATA_INTEGRITY_FAILURE", reader.getLastError());
            return false;
        }
    } catch (...) {
        addError(result, "DATA_INTEGRITY_EXCEPTION", "Exception occurred during data parsing");
        return false;
    }
    
    return true;
}

bool OtbFileValidator::validateItem(const ServerItem& item, ValidationResult& result)
{
    if (m_settings.validateItemProperties) {
        return validateItemProperties(item, result);
    }
    
    return true;
}

bool OtbFileValidator::validateItemProperties(const ServerItem& item, ValidationResult& result)
{
    bool isValid = true;
    
    // Validate item ID
    if (item.id == 0) {
        addWarning(result, "INVALID_ITEM_ID", QString("Item has ID 0"));
        isValid = false;
    }
    
    // Validate item type
    if (static_cast<int>(item.type) < 0 || static_cast<int>(item.type) > 20) {
        addError(result, "INVALID_ITEM_TYPE", QString("Item %1 has invalid type %2").arg(item.id).arg(static_cast<int>(item.type)));
        isValid = false;
    }
    
    // Validate sprite dimensions
    if (item.width == 0 || item.height == 0) {
        addWarning(result, "INVALID_SPRITE_DIMENSIONS", QString("Item %1 has invalid dimensions %2x%3").arg(item.id).arg(item.width).arg(item.height));
    }
    
    // Validate name
    if (item.name.isEmpty()) {
        addWarning(result, "EMPTY_ITEM_NAME", QString("Item %1 has no name").arg(item.id));
    }
    
    return isValid;
}

bool OtbFileValidator::validateItemConsistency(const ServerItemList& items, ValidationResult& result)
{
    if (m_settings.checkDuplicateIds) {
        QSet<quint16> seenIds;
        
        for (const ServerItem& item : items) {
            if (seenIds.contains(item.id)) {
                addError(result, "DUPLICATE_ITEM_ID", QString("Duplicate item ID: %1").arg(item.id));
                return false;
            }
            seenIds.insert(item.id);
        }
    }
    
    return true;
}

bool OtbFileValidator::checkFileStructure(const QByteArray& data)
{
    // Basic structure checks
    if (data.size() < 32) {
        m_corruptionDetails.append("File too small for valid OTB structure");
        return false;
    }
    
    // Check for null bytes in unexpected places
    int nullCount = 0;
    for (int i = 32; i < qMin(data.size(), 1024); ++i) {
        if (data[i] == 0) {
            nullCount++;
        }
    }
    
    if (nullCount > data.size() / 4) {
        m_corruptionDetails.append("Excessive null bytes detected");
        return false;
    }
    
    return true;
}

bool OtbFileValidator::checkDataConsistency(const QByteArray& data)
{
    // Check for consistent data patterns
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    // Skip to item count
    stream.skipRawData(24);
    
    quint32 itemCount;
    stream >> itemCount;
    
    if (itemCount > 100000) { // Sanity check
        m_corruptionDetails.append("Item count appears corrupted");
        return false;
    }
    
    return true;
}

bool OtbFileValidator::checkChecksums(const QByteArray& data)
{
    // Calculate and verify checksums
    quint32 calculatedChecksum = calculateChecksum(data, 0, data.size() - 4);
    
    quint32 storedChecksum;
    if (!readUInt32(data, data.size() - 4, storedChecksum)) {
        return false;
    }
    
    return calculatedChecksum == storedChecksum;
}

void OtbFileValidator::addError(ValidationResult& result, const QString& errorType, const QString& details)
{
    QString message = mapToLegacyError(errorType, details);
    result.errors.append(message);
    
    // Report to error handler
    REPORT_VALIDATION_ERROR(message, details);
}

void OtbFileValidator::addWarning(ValidationResult& result, const QString& warningType, const QString& details)
{
    QString message = mapToLegacyError(warningType, details);
    result.warnings.append(message);
}

void OtbFileValidator::addSuggestion(ValidationResult& result, const QString& suggestion)
{
    result.suggestions.append(suggestion);
}

QString OtbFileValidator::mapToLegacyError(const QString& errorType, const QString& details)
{
    // Map error types to legacy system error messages
    if (errorType == "FILE_NOT_FOUND") {
        return QString("The file '%1' could not be found.").arg(details);
    } else if (errorType == "FILE_ACCESS_DENIED") {
        return QString("Access to file '%1' was denied.").arg(details);
    } else if (errorType == "INVALID_SIGNATURE") {
        return "The file does not appear to be a valid OTB file.";
    } else if (errorType == "UNSUPPORTED_VERSION") {
        return QString("The file version is not supported. %1").arg(details);
    } else if (errorType == "INVALID_ITEM_RANGE") {
        return QString("The item range in the file is invalid. %1").arg(details);
    } else if (errorType == "TOO_MANY_ITEMS") {
        return QString("The file contains too many items. %1").arg(details);
    } else if (errorType == "DATA_INTEGRITY_FAILURE") {
        return QString("The file data appears to be corrupted. %1").arg(details);
    } else if (errorType == "DUPLICATE_ITEM_ID") {
        return QString("The file contains duplicate item IDs. %1").arg(details);
    } else if (errorType == "CORRUPTION_DETECTED") {
        return "The file may be corrupted and should be verified.";
    }
    
    // Default error message
    return QString("%1: %2").arg(errorType).arg(details);
}

bool OtbFileValidator::readUInt32(const QByteArray& data, int offset, quint32& value)
{
    if (offset + 4 > data.size()) {
        return false;
    }
    
    QDataStream stream(data.mid(offset, 4));
    stream.setByteOrder(QDataStream::LittleEndian);
    stream >> value;
    
    return stream.status() == QDataStream::Ok;
}

bool OtbFileValidator::readUInt16(const QByteArray& data, int offset, quint16& value)
{
    if (offset + 2 > data.size()) {
        return false;
    }
    
    QDataStream stream(data.mid(offset, 2));
    stream.setByteOrder(QDataStream::LittleEndian);
    stream >> value;
    
    return stream.status() == QDataStream::Ok;
}

bool OtbFileValidator::readUInt8(const QByteArray& data, int offset, quint8& value)
{
    if (offset + 1 > data.size()) {
        return false;
    }
    
    value = static_cast<quint8>(data[offset]);
    return true;
}

quint32 OtbFileValidator::calculateChecksum(const QByteArray& data, int start, int length)
{
    if (length == -1) {
        length = data.size() - start;
    }
    
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(data.constData() + start, length);
    
    QByteArray result = hash.result();
    
    // Convert first 4 bytes to uint32
    quint32 checksum = 0;
    if (result.size() >= 4) {
        checksum = (static_cast<quint32>(result[0]) << 24) |
                  (static_cast<quint32>(result[1]) << 16) |
                  (static_cast<quint32>(result[2]) << 8) |
                  static_cast<quint32>(result[3]);
    }
    
    return checksum;
}

bool OtbFileValidator::verifyChecksum(const QByteArray& data)
{
    if (data.size() < 8) {
        return false;
    }
    
    return checkChecksums(data);
}