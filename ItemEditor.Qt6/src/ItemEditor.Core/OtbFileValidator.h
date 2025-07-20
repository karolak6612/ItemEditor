#pragma once

#include <QString>
#include <QStringList>
#include <QByteArray>
#include "ServerItemList.h"
#include "ErrorHandler.h"

/**
 * @brief Comprehensive OTB file validation with identical error messages
 * 
 * Provides detailed file format validation and corruption detection
 * with exact error messages matching the legacy system.
 */
class OtbFileValidator
{
public:
    enum class ValidationLevel {
        Basic,      // Basic format validation
        Standard,   // Standard validation with data checks
        Thorough,   // Comprehensive validation with integrity checks
        Paranoid    // Exhaustive validation with all possible checks
    };

    struct ValidationResult {
        bool isValid;
        ValidationLevel level;
        QStringList errors;
        QStringList warnings;
        QStringList suggestions;
        
        // File statistics
        qint64 fileSize;
        quint32 itemCount;
        VersionInfo versionInfo;
        ItemRange itemRange;
        
        // Corruption indicators
        bool hasCorruption;
        QStringList corruptionDetails;
        
        ValidationResult() : isValid(false), level(ValidationLevel::Basic), 
                           fileSize(0), itemCount(0), hasCorruption(false) {}
    };

    OtbFileValidator();
    ~OtbFileValidator();

    // File validation
    ValidationResult validateFile(const QString& filePath, ValidationLevel level = ValidationLevel::Standard);
    ValidationResult validateData(const QByteArray& data, ValidationLevel level = ValidationLevel::Standard);
    ValidationResult validateItems(const ServerItemList& items, ValidationLevel level = ValidationLevel::Standard);
    
    // Quick validation checks
    static bool isValidOtbFile(const QString& filePath);
    static bool isValidOtbData(const QByteArray& data);
    static bool hasValidSignature(const QByteArray& data);
    static bool hasValidVersion(const QByteArray& data);
    
    // Corruption detection
    bool detectCorruption(const QString& filePath);
    bool detectCorruption(const QByteArray& data);
    QStringList getCorruptionDetails() const;
    
    // Recovery suggestions
    QStringList getRecoverySuggestions(const ValidationResult& result);
    static QStringList getCommonSolutions();
    
    // Error message compatibility (matching legacy system)
    static QString getCompatibleErrorMessage(const QString& errorType, const QString& details = QString());
    
    // Validation settings
    struct ValidationSettings {
        bool checkFileSignature = true;
        bool checkVersionCompatibility = true;
        bool checkItemRange = true;
        bool checkItemData = true;
        bool checkDataIntegrity = true;
        bool checkForCorruption = true;
        bool validateItemProperties = true;
        bool checkDuplicateIds = true;
        bool checkClientDataConsistency = false; // Requires plugins
        quint32 maxFileSize = 100 * 1024 * 1024; // 100MB
        quint32 maxItemCount = 65535;
    };
    
    ValidationSettings getSettings() const;
    void setSettings(const ValidationSettings& settings);

private:
    ValidationSettings m_settings;
    QStringList m_corruptionDetails;
    
    // Internal validation methods
    bool validateFileHeader(const QByteArray& data, ValidationResult& result);
    bool validateVersionInfo(const QByteArray& data, ValidationResult& result);
    bool validateItemRange(const QByteArray& data, ValidationResult& result);
    bool validateItemData(const QByteArray& data, ValidationResult& result);
    bool validateDataIntegrity(const QByteArray& data, ValidationResult& result);
    
    // Item validation
    bool validateItem(const ServerItem& item, ValidationResult& result);
    bool validateItemProperties(const ServerItem& item, ValidationResult& result);
    bool validateItemConsistency(const ServerItemList& items, ValidationResult& result);
    
    // Corruption detection methods
    bool checkFileStructure(const QByteArray& data);
    bool checkDataConsistency(const QByteArray& data);
    bool checkChecksums(const QByteArray& data);
    
    // Error message generation (matching legacy system)
    void addError(ValidationResult& result, const QString& errorType, const QString& details = QString());
    void addWarning(ValidationResult& result, const QString& warningType, const QString& details = QString());
    void addSuggestion(ValidationResult& result, const QString& suggestion);
    
    // Legacy error message mapping
    static QString mapToLegacyError(const QString& errorType, const QString& details);
    
    // Binary data helpers
    bool readUInt32(const QByteArray& data, int offset, quint32& value);
    bool readUInt16(const QByteArray& data, int offset, quint16& value);
    bool readUInt8(const QByteArray& data, int offset, quint8& value);
    
    // Checksum calculation
    quint32 calculateChecksum(const QByteArray& data, int start = 0, int length = -1);
    bool verifyChecksum(const QByteArray& data);
};