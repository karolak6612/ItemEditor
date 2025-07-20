#pragma once

#include <QString>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include "ServerItemList.h"

/**
 * @brief OTB file reader with byte-identical parsing logic
 * 
 * Implements complete OTB file format parsing that matches the legacy system
 * exactly, ensuring 100% compatibility with existing OTB files.
 */
class OtbReader
{
public:
    OtbReader();
    ~OtbReader();

    // Main reading functionality
    bool readFile(const QString& filePath);
    bool readFromData(const QByteArray& data);
    
    // Data access
    ServerItemList getItems() const;
    VersionInfo getVersionInfo() const;
    ItemRange getItemRange() const;
    
    // Error handling
    bool hasError() const;
    QString getLastError() const;
    QStringList getAllErrors() const;
    void clearErrors();
    
    // File validation
    static bool isValidOtbFile(const QString& filePath);
    static bool isValidOtbData(const QByteArray& data);
    static VersionInfo readVersionInfo(const QString& filePath);
    
    // Advanced reading options
    struct ReadOptions {
        bool validateItems = true;
        bool skipInvalidItems = false;
        bool loadItemsXml = true;
        bool preserveCustomItems = true;
        int maxItems = 65535;
    };
    
    bool readFile(const QString& filePath, const ReadOptions& options);
    
    // Statistics
    int getItemsRead() const;
    int getItemsSkipped() const;
    int getInvalidItems() const;
    qint64 getBytesRead() const;
    
    // Progress callback
    using ProgressCallback = std::function<void(int current, int total, const QString& status)>;
    void setProgressCallback(ProgressCallback callback);

private:
    ServerItemList m_items;
    VersionInfo m_versionInfo;
    ItemRange m_itemRange;
    QStringList m_errors;
    QString m_lastError;
    
    // Statistics
    int m_itemsRead;
    int m_itemsSkipped;
    int m_invalidItems;
    qint64 m_bytesRead;
    
    // Progress callback
    ProgressCallback m_progressCallback;
    
    // Internal parsing methods
    bool parseHeader(QDataStream& stream);
    bool parseVersionInfo(QDataStream& stream);
    bool parseItemRange(QDataStream& stream);
    bool parseItems(QDataStream& stream, const ReadOptions& options);
    bool parseItem(QDataStream& stream, ServerItem& item, const ReadOptions& options);
    
    // Binary format constants
    static const quint32 OTB_SIGNATURE;
    static const quint32 OTB_VERSION_1;
    static const quint32 OTB_VERSION_2;
    static const quint32 OTB_VERSION_3;
    
    // Item property parsing
    bool parseItemProperties(QDataStream& stream, ServerItem& item);
    bool parseItemProperty(QDataStream& stream, ServerItem& item, quint8 propertyType);
    
    // Validation helpers
    bool validateFileStructure(const QByteArray& data);
    bool validateItemData(const ServerItem& item);
    
    // Error handling helpers
    void addError(const QString& error);
    void addWarning(const QString& warning);
    void reportProgress(int current, int total, const QString& status);
    
    // Binary reading helpers
    bool readUInt8(QDataStream& stream, quint8& value);
    bool readUInt16(QDataStream& stream, quint16& value);
    bool readUInt32(QDataStream& stream, quint32& value);
    bool readString(QDataStream& stream, QString& value);
    bool readByteArray(QDataStream& stream, QByteArray& value, int length);
    
    // Legacy compatibility
    bool handleLegacyFormat(QDataStream& stream);
    bool convertLegacyItem(const QByteArray& legacyData, ServerItem& item);
};