#pragma once

#include <QString>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <functional>
#include "ServerItemList.h"

/**
 * @brief OTB file writer with byte-identical output generation
 * 
 * Implements complete OTB file format writing that produces byte-identical
 * output to the legacy system, ensuring 100% compatibility.
 */
class OtbWriter
{
public:
    OtbWriter();
    ~OtbWriter();

    // Main writing functionality
    bool writeFile(const QString& filePath, const ServerItemList& items);
    bool writeToData(QByteArray& data, const ServerItemList& items);
    
    // Writing options
    struct WriteOptions {
        bool validateItems = true;
        bool skipInvalidItems = false;
        bool preserveModificationInfo = true;
        bool compressOutput = false;
        bool createBackup = true;
        QString backupSuffix = ".bak";
    };
    
    bool writeFile(const QString& filePath, const ServerItemList& items, const WriteOptions& options);
    
    // Error handling
    bool hasError() const;
    QString getLastError() const;
    QStringList getAllErrors() const;
    void clearErrors();
    
    // File validation
    static bool canWriteToPath(const QString& filePath);
    static bool validateOutputPath(const QString& filePath);
    
    // Statistics
    int getItemsWritten() const;
    int getItemsSkipped() const;
    qint64 getBytesWritten() const;
    
    // Progress callback
    using ProgressCallback = std::function<void(int current, int total, const QString& status)>;
    void setProgressCallback(ProgressCallback callback);
    
    // Backup management
    bool createBackup(const QString& filePath, const QString& backupPath = QString());
    bool restoreFromBackup(const QString& filePath, const QString& backupPath = QString());
    static QString getBackupPath(const QString& filePath, const QString& suffix = ".bak");

private:
    QStringList m_errors;
    QString m_lastError;
    
    // Statistics
    int m_itemsWritten;
    int m_itemsSkipped;
    qint64 m_bytesWritten;
    
    // Progress callback
    ProgressCallback m_progressCallback;
    
    // Internal writing methods
    bool writeHeader(QDataStream& stream, const ServerItemList& items);
    bool writeVersionInfo(QDataStream& stream, const VersionInfo& versionInfo);
    bool writeItemRange(QDataStream& stream, const ItemRange& itemRange);
    bool writeItems(QDataStream& stream, const ServerItemList& items, const WriteOptions& options);
    bool writeItem(QDataStream& stream, const ServerItem& item);
    
    // Binary format constants (matching OtbReader)
    static const quint32 OTB_SIGNATURE;
    static const quint32 OTB_VERSION_1;
    static const quint32 OTB_VERSION_2;
    static const quint32 OTB_VERSION_3;
    
    // Item property writing
    bool writeItemProperties(QDataStream& stream, const ServerItem& item);
    bool writeItemProperty(QDataStream& stream, const ServerItem& item, quint8 propertyType);
    
    // Validation helpers
    bool validateItemsForWriting(const ServerItemList& items, const WriteOptions& options);
    bool validateItemForWriting(const ServerItem& item);
    
    // Error handling helpers
    void addError(const QString& error);
    void addWarning(const QString& warning);
    void reportProgress(int current, int total, const QString& status);
    
    // Binary writing helpers
    bool writeUInt8(QDataStream& stream, quint8 value);
    bool writeUInt16(QDataStream& stream, quint16 value);
    bool writeUInt32(QDataStream& stream, quint32 value);
    bool writeString(QDataStream& stream, const QString& value);
    bool writeByteArray(QDataStream& stream, const QByteArray& value);
    bool writeFixedByteArray(QDataStream& stream, const QByteArray& value, int length);
    
    // File operations
    bool ensureDirectoryExists(const QString& filePath);
    bool writeToFileAtomic(const QString& filePath, const QByteArray& data);
};