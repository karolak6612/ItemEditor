#ifndef OTBRREADER_H
#define OTBRREADER_H

#include "otbtypes.h"
#include "binarytree.h"
#include "otbheader.h"
#include "otberrors.h"
#include "otbcache.h"
#include "otbperformance.h"
#include <QString>
#include <QScopedPointer>
#include <QElapsedTimer>

namespace OTB {

// Enhanced error codes for better error reporting
enum class OtbReadError {
    None = 0,
    FileNotFound,
    FileAccessDenied,
    InvalidHeader,
    CorruptedData,
    UnsupportedVersion,
    InvalidNodeStructure,
    AttributeValidationFailed,
    UnexpectedEndOfFile,
    MemoryAllocationFailed,
    InvalidItemData,
    TreeStructureCorrupted
};

// Reading statistics for performance monitoring
struct ReadingStats {
    qint64 bytesRead = 0;
    qint64 itemsProcessed = 0;
    qint64 attributesProcessed = 0;
    qint64 readingTimeMs = 0;
    QList<QString> warnings;
    
    void reset() {
        bytesRead = 0;
        itemsProcessed = 0;
        attributesProcessed = 0;
        readingTimeMs = 0;
        warnings.clear();
    }
};

class OtbReader {
public:
    OtbReader();

    // Enhanced read method with comprehensive error handling and validation
    bool read(const QString& filePath, ServerItemList& items, QString& errorString);
    
    // Validates OTB file header and version compatibility
    bool validateFile(const QString& filePath, QString& errorString);
    
    // Detects OTB file version without full parsing
    bool detectFileVersion(const QString& filePath, OtbVersionInfo& versionInfo, QString& errorString);
    
    // Enhanced validation with detailed corruption detection
    bool validateFileIntegrity(const QString& filePath, QString& errorString);
    
    // Partial read for large files - reads only specified item range
    bool readPartial(const QString& filePath, ServerItemList& items, 
                    quint16 startId, quint16 endId, QString& errorString);
    
    // Read with progress callback for UI applications
    bool readWithProgress(const QString& filePath, ServerItemList& items, 
                         QString& errorString, 
                         std::function<void(int)> progressCallback = nullptr);
    
    // Get reading statistics from last operation
    const ReadingStats& getLastReadingStats() const { return m_stats; }
    
    // Get last error code for programmatic error handling
    OtbReadError getLastErrorCode() const { return m_lastErrorCode; }
    
    // Enable/disable strict validation (default: enabled)
    void setStrictValidation(bool enabled) { m_strictValidation = enabled; }
    
    // Enable/disable detailed logging (default: disabled)
    void setDetailedLogging(bool enabled) { m_detailedLogging = enabled; }
    
    // Performance optimization settings
    void setCacheEnabled(bool enabled) { m_cacheEnabled = enabled; }
    void setBufferSize(qint64 size) { m_bufferSize = size; }
    void setPerformanceMonitoring(bool enabled) { m_performanceMonitoring = enabled; }
    
    // Get performance metrics from last operation
    const PerformanceMetrics& getLastPerformanceMetrics() const { return m_performanceMetrics; }

private:
    // Enhanced validation and parsing methods
    bool validateHeader(QString& errorString);
    bool validateNodeStructure(QString& errorString);
    bool parseRootNode(ServerItemList& items, QString& errorString);
    bool parseItemNode(ServerItem& item, QString& errorString);
    
    // Data validation methods
    bool validateAttributeData(ServerItemAttribute attribute, quint16 dataLen, 
                              const QByteArray& data, QString& errorString);
    bool validateItemConsistency(const ServerItem& item, QString& errorString);
    bool validateRootAttributeData(RootAttribute attribute, quint16 dataLen, 
                                  const QByteArray& data, QString& errorString);
    
    // Error handling and recovery
    bool handleCorruptedNode(QString& errorString);
    bool attemptDataRecovery(QString& errorString);
    void logWarning(const QString& warning);
    void setError(OtbReadError errorCode, const QString& message);
    
    // Memory and performance optimization
    void optimizeMemoryUsage();
    bool checkMemoryConstraints(qint64 requiredBytes);
    
    BinaryTree m_tree;
    ReadingStats m_stats;
    OtbReadError m_lastErrorCode = OtbReadError::None;
    QString m_lastErrorMessage;
    bool m_strictValidation = true;
    bool m_detailedLogging = false;
    QElapsedTimer m_timer;
    
    // Performance optimization members
    bool m_cacheEnabled = true;
    bool m_performanceMonitoring = true;
    qint64 m_bufferSize = 64 * 1024; // 64KB default
    PerformanceMetrics m_performanceMetrics;
    std::unique_ptr<IOBuffer> m_ioBuffer;
    OtbCache* m_cache = nullptr;
};

} // namespace OTB

#endif // OTBRREADER_H
