#include "OtbReader.h"
#include "ItemValidator.h"
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QCryptographicHash>

// Binary format constants
const quint32 OtbReader::OTB_SIGNATURE = 0x00000000;
const quint32 OtbReader::OTB_VERSION_1 = 0x00000001;
const quint32 OtbReader::OTB_VERSION_2 = 0x00000002;
const quint32 OtbReader::OTB_VERSION_3 = 0x00000003;

OtbReader::OtbReader()
    : m_itemsRead(0)
    , m_itemsSkipped(0)
    , m_invalidItems(0)
    , m_bytesRead(0)
{
    // Initialize version info
    m_versionInfo.majorVersion = 0;
    m_versionInfo.minorVersion = 0;
    m_versionInfo.buildNumber = 0;
    m_versionInfo.clientVersion = 0;
    
    // Initialize item range
    m_itemRange.minId = 0;
    m_itemRange.maxId = 0;
}

OtbReader::~OtbReader()
{
}

bool OtbReader::readFile(const QString& filePath)
{
    ReadOptions defaultOptions;
    return readFile(filePath, defaultOptions);
}

bool OtbReader::readFile(const QString& filePath, const ReadOptions& options)
{
    clearErrors();
    
    // Reset statistics
    m_itemsRead = 0;
    m_itemsSkipped = 0;
    m_invalidItems = 0;
    m_bytesRead = 0;
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        addError(QString("Cannot open file: %1").arg(file.errorString()));
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    if (data.isEmpty()) {
        addError("File is empty");
        return false;
    }
    
    m_bytesRead = data.size();
    reportProgress(0, 100, "Reading file...");
    
    return readFromData(data);
}

bool OtbReader::readFromData(const QByteArray& data)
{
    if (!validateFileStructure(data)) {
        return false;
    }
    
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    reportProgress(10, 100, "Parsing header...");
    
    if (!parseHeader(stream)) {
        return false;
    }
    
    reportProgress(20, 100, "Reading version info...");
    
    if (!parseVersionInfo(stream)) {
        return false;
    }
    
    reportProgress(30, 100, "Reading item range...");
    
    if (!parseItemRange(stream)) {
        return false;
    }
    
    reportProgress(40, 100, "Reading items...");
    
    ReadOptions defaultOptions;
    if (!parseItems(stream, defaultOptions)) {
        return false;
    }
    
    reportProgress(100, 100, "Reading complete");
    
    return true;
}

ServerItemList OtbReader::getItems() const
{
    return m_items;
}

VersionInfo OtbReader::getVersionInfo() const
{
    return m_versionInfo;
}

ItemRange OtbReader::getItemRange() const
{
    return m_itemRange;
}

bool OtbReader::hasError() const
{
    return !m_errors.isEmpty();
}

QString OtbReader::getLastError() const
{
    return m_lastError;
}

QStringList OtbReader::getAllErrors() const
{
    return m_errors;
}

void OtbReader::clearErrors()
{
    m_errors.clear();
    m_lastError.clear();
}

bool OtbReader::isValidOtbFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray header = file.read(16);
    file.close();
    
    return isValidOtbData(header);
}

bool OtbReader::isValidOtbData(const QByteArray& data)
{
    if (data.size() < 16) {
        return false;
    }
    
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    quint32 signature;
    stream >> signature;
    
    return signature == OTB_SIGNATURE;
}

VersionInfo OtbReader::readVersionInfo(const QString& filePath)
{
    VersionInfo versionInfo = {};
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return versionInfo;
    }
    
    QByteArray data = file.read(32);
    file.close();
    
    if (data.size() < 32) {
        return versionInfo;
    }
    
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    quint32 signature;
    stream >> signature;
    
    if (signature != OTB_SIGNATURE) {
        return versionInfo;
    }
    
    stream >> versionInfo.majorVersion;
    stream >> versionInfo.minorVersion;
    stream >> versionInfo.buildNumber;
    stream >> versionInfo.clientVersion;
    
    return versionInfo;
}

int OtbReader::getItemsRead() const
{
    return m_itemsRead;
}

int OtbReader::getItemsSkipped() const
{
    return m_itemsSkipped;
}

int OtbReader::getInvalidItems() const
{
    return m_invalidItems;
}

qint64 OtbReader::getBytesRead() const
{
    return m_bytesRead;
}

void OtbReader::setProgressCallback(ProgressCallback callback)
{
    m_progressCallback = callback;
}

bool OtbReader::parseHeader(QDataStream& stream)
{
    quint32 signature;
    if (!readUInt32(stream, signature)) {
        addError("Failed to read file signature");
        return false;
    }
    
    if (signature != OTB_SIGNATURE) {
        addError(QString("Invalid file signature: 0x%1").arg(signature, 8, 16, QChar('0')));
        return false;
    }
    
    return true;
}

bool OtbReader::parseVersionInfo(QDataStream& stream)
{
    if (!readUInt32(stream, m_versionInfo.majorVersion) ||
        !readUInt32(stream, m_versionInfo.minorVersion) ||
        !readUInt32(stream, m_versionInfo.buildNumber) ||
        !readUInt32(stream, m_versionInfo.clientVersion)) {
        addError("Failed to read version information");
        return false;
    }
    
    // Validate version compatibility
    if (m_versionInfo.majorVersion > 3) {
        addWarning(QString("Unsupported major version: %1").arg(m_versionInfo.majorVersion));
    }
    
    return true;
}

bool OtbReader::parseItemRange(QDataStream& stream)
{
    if (!readUInt16(stream, m_itemRange.minId) ||
        !readUInt16(stream, m_itemRange.maxId)) {
        addError("Failed to read item range");
        return false;
    }
    
    if (m_itemRange.minId > m_itemRange.maxId) {
        addError(QString("Invalid item range: %1-%2").arg(m_itemRange.minId).arg(m_itemRange.maxId));
        return false;
    }
    
    if (m_itemRange.maxId > 65535) {
        addError(QString("Item range exceeds maximum: %1").arg(m_itemRange.maxId));
        return false;
    }
    
    return true;
}

bool OtbReader::parseItems(QDataStream& stream, const ReadOptions& options)
{
    m_items.clear();
    m_items.versionInfo = m_versionInfo;
    m_items.itemRange = m_itemRange;
    
    quint32 itemCount;
    if (!readUInt32(stream, itemCount)) {
        addError("Failed to read item count");
        return false;
    }
    
    if (itemCount > static_cast<quint32>(options.maxItems)) {
        addWarning(QString("Item count (%1) exceeds maximum (%2), limiting").arg(itemCount).arg(options.maxItems));
        itemCount = options.maxItems;
    }
    
    m_items.reserve(itemCount);
    
    for (quint32 i = 0; i < itemCount; ++i) {
        ServerItem item;
        
        reportProgress(40 + (i * 50) / itemCount, 100, QString("Reading item %1/%2").arg(i + 1).arg(itemCount));
        
        if (parseItem(stream, item, options)) {
            if (options.validateItems && !validateItemData(item)) {
                m_invalidItems++;
                if (options.skipInvalidItems) {
                    m_itemsSkipped++;
                    continue;
                } else {
                    addError(QString("Invalid item data for ID %1").arg(item.id));
                    return false;
                }
            }
            
            m_items.addItem(item);
            m_itemsRead++;
        } else {
            m_itemsSkipped++;
            if (!options.skipInvalidItems) {
                addError(QString("Failed to parse item %1").arg(i + 1));
                return false;
            }
        }
    }
    
    m_items.clearModified();
    return true;
}

bool OtbReader::parseItem(QDataStream& stream, ServerItem& item, const ReadOptions& options)
{
    Q_UNUSED(options)
    
    // Read item ID
    if (!readUInt16(stream, item.id)) {
        return false;
    }
    
    // Read item type
    quint8 typeValue;
    if (!readUInt8(stream, typeValue)) {
        return false;
    }
    item.type = static_cast<ServerItemType>(typeValue);
    
    // Read basic properties
    if (!readUInt16(stream, item.clientId) ||
        !readUInt16(stream, item.previousClientId)) {
        return false;
    }
    
    // Read stack order
    quint8 stackOrderValue;
    if (!readUInt8(stream, stackOrderValue)) {
        return false;
    }
    item.stackOrder = static_cast<TileStackOrder>(stackOrderValue);
    
    // Read strings
    if (!readString(stream, item.name) ||
        !readString(stream, item.description) ||
        !readString(stream, item.article) ||
        !readString(stream, item.plural)) {
        return false;
    }
    
    // Read sprite information
    if (!readByteArray(stream, item.spriteHash, 16) ||
        !readUInt8(stream, item.width) ||
        !readUInt8(stream, item.height) ||
        !readUInt8(stream, item.layers) ||
        !readUInt8(stream, item.patternX) ||
        !readUInt8(stream, item.patternY) ||
        !readUInt8(stream, item.patternZ) ||
        !readUInt8(stream, item.frames)) {
        return false;
    }
    
    // Read item attributes
    if (!readUInt32(stream, item.flags) ||
        !readUInt16(stream, item.speed) ||
        !readUInt16(stream, item.lightLevel) ||
        !readUInt16(stream, item.lightColor) ||
        !readUInt16(stream, item.minimapColor) ||
        !readUInt8(stream, item.elevation)) {
        return false;
    }
    
    // Read trade properties
    if (!readUInt16(stream, item.tradeAs)) {
        return false;
    }
    
    quint8 showAsValue;
    if (!readUInt8(stream, showAsValue)) {
        return false;
    }
    item.showAs = (showAsValue != 0);
    
    // Read weapon properties
    if (!readUInt8(stream, item.weaponType) ||
        !readUInt8(stream, item.ammoType) ||
        !readUInt8(stream, item.shootType) ||
        !readUInt8(stream, item.effect) ||
        !readUInt8(stream, item.distanceEffect)) {
        return false;
    }
    
    // Read armor and protection
    if (!readUInt16(stream, item.armor) ||
        !readUInt16(stream, item.defense) ||
        !readUInt16(stream, item.extraDefense) ||
        !readUInt16(stream, item.attack) ||
        !readUInt16(stream, item.rotateTo)) {
        return false;
    }
    
    // Read container properties
    if (!readUInt16(stream, item.containerSize)) {
        return false;
    }
    
    // Read fluid properties
    if (!readUInt8(stream, item.fluidSource)) {
        return false;
    }
    
    // Read readable/writable properties
    if (!readUInt16(stream, item.maxReadWriteChars) ||
        !readUInt16(stream, item.maxReadChars) ||
        !readUInt16(stream, item.maxWriteChars)) {
        return false;
    }
    
    // Read custom properties
    quint8 isCustomValue, hasClientDataValue;
    if (!readUInt8(stream, isCustomValue) ||
        !readUInt8(stream, hasClientDataValue)) {
        return false;
    }
    
    item.isCustomCreated = (isCustomValue != 0);
    item.hasClientData = (hasClientDataValue != 0);
    
    // Read modification info
    qint64 lastModifiedValue;
    if (!stream.device()->read(reinterpret_cast<char*>(&lastModifiedValue), sizeof(lastModifiedValue))) {
        return false;
    }
    item.lastModified = QDateTime::fromMSecsSinceEpoch(lastModifiedValue);
    
    if (!readString(stream, item.modifiedBy)) {
        return false;
    }
    
    return true;
}

bool OtbReader::parseItemProperties(QDataStream& stream, ServerItem& item)
{
    quint8 propertyCount;
    if (!readUInt8(stream, propertyCount)) {
        return false;
    }
    
    for (quint8 i = 0; i < propertyCount; ++i) {
        quint8 propertyType;
        if (!readUInt8(stream, propertyType)) {
            return false;
        }
        
        if (!parseItemProperty(stream, item, propertyType)) {
            return false;
        }
    }
    
    return true;
}

bool OtbReader::parseItemProperty(QDataStream& stream, ServerItem& item, quint8 propertyType)
{
    Q_UNUSED(item)
    Q_UNUSED(propertyType)
    
    // Property parsing would be implemented here based on the specific
    // property types defined in the OTB format specification
    // For now, we'll skip unknown properties
    
    quint16 propertyLength;
    if (!readUInt16(stream, propertyLength)) {
        return false;
    }
    
    // Skip property data
    if (stream.skipRawData(propertyLength) != propertyLength) {
        return false;
    }
    
    return true;
}

bool OtbReader::validateFileStructure(const QByteArray& data)
{
    if (data.size() < 32) {
        addError("File too small to be a valid OTB file");
        return false;
    }
    
    if (!isValidOtbData(data)) {
        addError("Invalid OTB file signature");
        return false;
    }
    
    return true;
}

bool OtbReader::validateItemData(const ServerItem& item)
{
    return ItemValidator::validateItem(item);
}

void OtbReader::addError(const QString& error)
{
    m_lastError = error;
    m_errors.append(QString("[ERROR] %1").arg(error));
    qDebug() << "OtbReader Error:" << error;
}

void OtbReader::addWarning(const QString& warning)
{
    m_errors.append(QString("[WARNING] %1").arg(warning));
    qDebug() << "OtbReader Warning:" << warning;
}

void OtbReader::reportProgress(int current, int total, const QString& status)
{
    if (m_progressCallback) {
        m_progressCallback(current, total, status);
    }
}

bool OtbReader::readUInt8(QDataStream& stream, quint8& value)
{
    stream >> value;
    return stream.status() == QDataStream::Ok;
}

bool OtbReader::readUInt16(QDataStream& stream, quint16& value)
{
    stream >> value;
    return stream.status() == QDataStream::Ok;
}

bool OtbReader::readUInt32(QDataStream& stream, quint32& value)
{
    stream >> value;
    return stream.status() == QDataStream::Ok;
}

bool OtbReader::readString(QDataStream& stream, QString& value)
{
    quint16 length;
    if (!readUInt16(stream, length)) {
        return false;
    }
    
    if (length == 0) {
        value.clear();
        return true;
    }
    
    QByteArray data(length, 0);
    if (stream.readRawData(data.data(), length) != length) {
        return false;
    }
    
    value = QString::fromUtf8(data);
    return true;
}

bool OtbReader::readByteArray(QDataStream& stream, QByteArray& value, int length)
{
    value.resize(length);
    return stream.readRawData(value.data(), length) == length;
}

bool OtbReader::handleLegacyFormat(QDataStream& stream)
{
    Q_UNUSED(stream)
    // Legacy format handling would be implemented here
    // for compatibility with older OTB file versions
    addWarning("Legacy format detected, some features may not be available");
    return true;
}

bool OtbReader::convertLegacyItem(const QByteArray& legacyData, ServerItem& item)
{
    Q_UNUSED(legacyData)
    Q_UNUSED(item)
    // Legacy item conversion would be implemented here
    return false;
}