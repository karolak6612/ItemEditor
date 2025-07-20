#include "OtbWriter.h"
#include "ItemValidator.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QSaveFile>

// Binary format constants (matching OtbReader)
const quint32 OtbWriter::OTB_SIGNATURE = 0x00000000;
const quint32 OtbWriter::OTB_VERSION_1 = 0x00000001;
const quint32 OtbWriter::OTB_VERSION_2 = 0x00000002;
const quint32 OtbWriter::OTB_VERSION_3 = 0x00000003;

OtbWriter::OtbWriter()
    : m_itemsWritten(0)
    , m_itemsSkipped(0)
    , m_bytesWritten(0)
{
}

OtbWriter::~OtbWriter()
{
}

bool OtbWriter::writeFile(const QString& filePath, const ServerItemList& items)
{
    WriteOptions defaultOptions;
    return writeFile(filePath, items, defaultOptions);
}

bool OtbWriter::writeFile(const QString& filePath, const ServerItemList& items, const WriteOptions& options)
{
    clearErrors();
    
    // Reset statistics
    m_itemsWritten = 0;
    m_itemsSkipped = 0;
    m_bytesWritten = 0;
    
    if (!validateOutputPath(filePath)) {
        addError(QString("Invalid output path: %1").arg(filePath));
        return false;
    }
    
    if (!ensureDirectoryExists(filePath)) {
        addError(QString("Cannot create directory for: %1").arg(filePath));
        return false;
    }
    
    // Create backup if requested
    if (options.createBackup && QFile::exists(filePath)) {
        QString backupPath = getBackupPath(filePath, options.backupSuffix);
        if (!createBackup(filePath, backupPath)) {
            addWarning(QString("Failed to create backup: %1").arg(backupPath));
        }
    }
    
    reportProgress(0, 100, "Preparing to write...");
    
    // Validate items before writing
    if (options.validateItems && !validateItemsForWriting(items, options)) {
        return false;
    }
    
    reportProgress(10, 100, "Generating output data...");
    
    QByteArray data;
    if (!writeToData(data, items)) {
        return false;
    }
    
    reportProgress(90, 100, "Writing to file...");
    
    if (!writeToFileAtomic(filePath, data)) {
        addError(QString("Failed to write file: %1").arg(filePath));
        return false;
    }
    
    m_bytesWritten = data.size();
    reportProgress(100, 100, "Write complete");
    
    return true;
}

bool OtbWriter::writeToData(QByteArray& data, const ServerItemList& items)
{
    data.clear();
    
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    reportProgress(20, 100, "Writing header...");
    
    if (!writeHeader(stream, items)) {
        return false;
    }
    
    reportProgress(30, 100, "Writing version info...");
    
    if (!writeVersionInfo(stream, items.versionInfo)) {
        return false;
    }
    
    reportProgress(40, 100, "Writing item range...");
    
    if (!writeItemRange(stream, items.itemRange)) {
        return false;
    }
    
    reportProgress(50, 100, "Writing items...");
    
    WriteOptions defaultOptions;
    if (!writeItems(stream, items, defaultOptions)) {
        return false;
    }
    
    return stream.status() == QDataStream::Ok;
}

bool OtbWriter::hasError() const
{
    return !m_errors.isEmpty();
}

QString OtbWriter::getLastError() const
{
    return m_lastError;
}

QStringList OtbWriter::getAllErrors() const
{
    return m_errors;
}

void OtbWriter::clearErrors()
{
    m_errors.clear();
    m_lastError.clear();
}

bool OtbWriter::canWriteToPath(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();
    
    // Check if directory exists or can be created
    if (!dir.exists() && !dir.mkpath(".")) {
        return false;
    }
    
    // Check if file exists and is writable, or if directory is writable
    if (fileInfo.exists()) {
        return fileInfo.isWritable();
    } else {
        return QFileInfo(dir.absolutePath()).isWritable();
    }
}

bool OtbWriter::validateOutputPath(const QString& filePath)
{
    if (filePath.isEmpty()) {
        return false;
    }
    
    QFileInfo fileInfo(filePath);
    
    // Check if path is absolute or relative
    if (!fileInfo.isAbsolute() && !fileInfo.isRelative()) {
        return false;
    }
    
    // Check file extension
    if (!filePath.toLower().endsWith(".otb")) {
        return false;
    }
    
    return canWriteToPath(filePath);
}

int OtbWriter::getItemsWritten() const
{
    return m_itemsWritten;
}

int OtbWriter::getItemsSkipped() const
{
    return m_itemsSkipped;
}

qint64 OtbWriter::getBytesWritten() const
{
    return m_bytesWritten;
}

void OtbWriter::setProgressCallback(ProgressCallback callback)
{
    m_progressCallback = callback;
}

bool OtbWriter::createBackup(const QString& filePath, const QString& backupPath)
{
    QString actualBackupPath = backupPath.isEmpty() ? getBackupPath(filePath) : backupPath;
    
    if (!QFile::exists(filePath)) {
        return true; // No file to backup
    }
    
    // Remove existing backup
    if (QFile::exists(actualBackupPath)) {
        QFile::remove(actualBackupPath);
    }
    
    return QFile::copy(filePath, actualBackupPath);
}

bool OtbWriter::restoreFromBackup(const QString& filePath, const QString& backupPath)
{
    QString actualBackupPath = backupPath.isEmpty() ? getBackupPath(filePath) : backupPath;
    
    if (!QFile::exists(actualBackupPath)) {
        return false;
    }
    
    // Remove current file
    if (QFile::exists(filePath)) {
        QFile::remove(filePath);
    }
    
    return QFile::copy(actualBackupPath, filePath);
}

QString OtbWriter::getBackupPath(const QString& filePath, const QString& suffix)
{
    return filePath + suffix;
}

bool OtbWriter::writeHeader(QDataStream& stream, const ServerItemList& items)
{
    Q_UNUSED(items)
    
    if (!writeUInt32(stream, OTB_SIGNATURE)) {
        addError("Failed to write file signature");
        return false;
    }
    
    return true;
}

bool OtbWriter::writeVersionInfo(QDataStream& stream, const VersionInfo& versionInfo)
{
    if (!writeUInt32(stream, versionInfo.majorVersion) ||
        !writeUInt32(stream, versionInfo.minorVersion) ||
        !writeUInt32(stream, versionInfo.buildNumber) ||
        !writeUInt32(stream, versionInfo.clientVersion)) {
        addError("Failed to write version information");
        return false;
    }
    
    return true;
}

bool OtbWriter::writeItemRange(QDataStream& stream, const ItemRange& itemRange)
{
    if (!writeUInt16(stream, itemRange.minId) ||
        !writeUInt16(stream, itemRange.maxId)) {
        addError("Failed to write item range");
        return false;
    }
    
    return true;
}

bool OtbWriter::writeItems(QDataStream& stream, const ServerItemList& items, const WriteOptions& options)
{
    quint32 itemCount = items.size();
    
    if (!writeUInt32(stream, itemCount)) {
        addError("Failed to write item count");
        return false;
    }
    
    for (int i = 0; i < items.size(); ++i) {
        const ServerItem& item = items[i];
        
        reportProgress(50 + (i * 40) / items.size(), 100, QString("Writing item %1/%2").arg(i + 1).arg(items.size()));
        
        if (options.validateItems && !validateItemForWriting(item)) {
            if (options.skipInvalidItems) {
                m_itemsSkipped++;
                continue;
            } else {
                addError(QString("Invalid item data for ID %1").arg(item.id));
                return false;
            }
        }
        
        if (!writeItem(stream, item)) {
            addError(QString("Failed to write item %1").arg(item.id));
            return false;
        }
        
        m_itemsWritten++;
    }
    
    return true;
}

bool OtbWriter::writeItem(QDataStream& stream, const ServerItem& item)
{
    // Write item ID
    if (!writeUInt16(stream, item.id)) {
        return false;
    }
    
    // Write item type
    if (!writeUInt8(stream, static_cast<quint8>(item.type))) {
        return false;
    }
    
    // Write basic properties
    if (!writeUInt16(stream, item.clientId) ||
        !writeUInt16(stream, item.previousClientId)) {
        return false;
    }
    
    // Write stack order
    if (!writeUInt8(stream, static_cast<quint8>(item.stackOrder))) {
        return false;
    }
    
    // Write strings
    if (!writeString(stream, item.name) ||
        !writeString(stream, item.description) ||
        !writeString(stream, item.article) ||
        !writeString(stream, item.plural)) {
        return false;
    }
    
    // Write sprite information
    if (!writeFixedByteArray(stream, item.spriteHash, 16) ||
        !writeUInt8(stream, item.width) ||
        !writeUInt8(stream, item.height) ||
        !writeUInt8(stream, item.layers) ||
        !writeUInt8(stream, item.patternX) ||
        !writeUInt8(stream, item.patternY) ||
        !writeUInt8(stream, item.patternZ) ||
        !writeUInt8(stream, item.frames)) {
        return false;
    }
    
    // Write item attributes
    if (!writeUInt32(stream, item.flags) ||
        !writeUInt16(stream, item.speed) ||
        !writeUInt16(stream, item.lightLevel) ||
        !writeUInt16(stream, item.lightColor) ||
        !writeUInt16(stream, item.minimapColor) ||
        !writeUInt8(stream, item.elevation)) {
        return false;
    }
    
    // Write trade properties
    if (!writeUInt16(stream, item.tradeAs)) {
        return false;
    }
    
    if (!writeUInt8(stream, item.showAs ? 1 : 0)) {
        return false;
    }
    
    // Write weapon properties
    if (!writeUInt8(stream, item.weaponType) ||
        !writeUInt8(stream, item.ammoType) ||
        !writeUInt8(stream, item.shootType) ||
        !writeUInt8(stream, item.effect) ||
        !writeUInt8(stream, item.distanceEffect)) {
        return false;
    }
    
    // Write armor and protection
    if (!writeUInt16(stream, item.armor) ||
        !writeUInt16(stream, item.defense) ||
        !writeUInt16(stream, item.extraDefense) ||
        !writeUInt16(stream, item.attack) ||
        !writeUInt16(stream, item.rotateTo)) {
        return false;
    }
    
    // Write container properties
    if (!writeUInt16(stream, item.containerSize)) {
        return false;
    }
    
    // Write fluid properties
    if (!writeUInt8(stream, item.fluidSource)) {
        return false;
    }
    
    // Write readable/writable properties
    if (!writeUInt16(stream, item.maxReadWriteChars) ||
        !writeUInt16(stream, item.maxReadChars) ||
        !writeUInt16(stream, item.maxWriteChars)) {
        return false;
    }
    
    // Write custom properties
    if (!writeUInt8(stream, item.isCustomCreated ? 1 : 0) ||
        !writeUInt8(stream, item.hasClientData ? 1 : 0)) {
        return false;
    }
    
    // Write modification info
    qint64 lastModifiedValue = item.lastModified.toMSecsSinceEpoch();
    if (stream.writeRawData(reinterpret_cast<const char*>(&lastModifiedValue), sizeof(lastModifiedValue)) != sizeof(lastModifiedValue)) {
        return false;
    }
    
    if (!writeString(stream, item.modifiedBy)) {
        return false;
    }
    
    return true;
}

bool OtbWriter::writeItemProperties(QDataStream& stream, const ServerItem& item)
{
    Q_UNUSED(stream)
    Q_UNUSED(item)
    
    // Property writing would be implemented here based on the specific
    // property types defined in the OTB format specification
    // For now, we'll write zero properties
    
    if (!writeUInt8(stream, 0)) { // Property count
        return false;
    }
    
    return true;
}

bool OtbWriter::writeItemProperty(QDataStream& stream, const ServerItem& item, quint8 propertyType)
{
    Q_UNUSED(stream)
    Q_UNUSED(item)
    Q_UNUSED(propertyType)
    
    // Individual property writing would be implemented here
    return true;
}

bool OtbWriter::validateItemsForWriting(const ServerItemList& items, const WriteOptions& options)
{
    if (items.isEmpty()) {
        addWarning("Writing empty item list");
        return true;
    }
    
    int invalidCount = 0;
    
    for (const ServerItem& item : items) {
        if (!validateItemForWriting(item)) {
            invalidCount++;
            if (!options.skipInvalidItems) {
                addError(QString("Invalid item with ID %1").arg(item.id));
                return false;
            }
        }
    }
    
    if (invalidCount > 0) {
        addWarning(QString("Found %1 invalid items").arg(invalidCount));
    }
    
    return true;
}

bool OtbWriter::validateItemForWriting(const ServerItem& item)
{
    return ItemValidator::validateItem(item);
}

void OtbWriter::addError(const QString& error)
{
    m_lastError = error;
    m_errors.append(QString("[ERROR] %1").arg(error));
    qDebug() << "OtbWriter Error:" << error;
}

void OtbWriter::addWarning(const QString& warning)
{
    m_errors.append(QString("[WARNING] %1").arg(warning));
    qDebug() << "OtbWriter Warning:" << warning;
}

void OtbWriter::reportProgress(int current, int total, const QString& status)
{
    if (m_progressCallback) {
        m_progressCallback(current, total, status);
    }
}

bool OtbWriter::writeUInt8(QDataStream& stream, quint8 value)
{
    stream << value;
    return stream.status() == QDataStream::Ok;
}

bool OtbWriter::writeUInt16(QDataStream& stream, quint16 value)
{
    stream << value;
    return stream.status() == QDataStream::Ok;
}

bool OtbWriter::writeUInt32(QDataStream& stream, quint32 value)
{
    stream << value;
    return stream.status() == QDataStream::Ok;
}

bool OtbWriter::writeString(QDataStream& stream, const QString& value)
{
    QByteArray utf8Data = value.toUtf8();
    quint16 length = utf8Data.length();
    
    if (!writeUInt16(stream, length)) {
        return false;
    }
    
    if (length > 0) {
        return stream.writeRawData(utf8Data.constData(), length) == length;
    }
    
    return true;
}

bool OtbWriter::writeByteArray(QDataStream& stream, const QByteArray& value)
{
    quint32 length = value.length();
    
    if (!writeUInt32(stream, length)) {
        return false;
    }
    
    if (length > 0) {
        return stream.writeRawData(value.constData(), length) == static_cast<int>(length);
    }
    
    return true;
}

bool OtbWriter::writeFixedByteArray(QDataStream& stream, const QByteArray& value, int length)
{
    QByteArray data = value;
    
    // Pad or truncate to exact length
    if (data.length() < length) {
        data.resize(length); // Pads with zeros
    } else if (data.length() > length) {
        data.truncate(length);
    }
    
    return stream.writeRawData(data.constData(), length) == length;
}

bool OtbWriter::ensureDirectoryExists(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();
    
    if (!dir.exists()) {
        return dir.mkpath(".");
    }
    
    return true;
}

bool OtbWriter::writeToFileAtomic(const QString& filePath, const QByteArray& data)
{
    QSaveFile file(filePath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        addError(QString("Cannot open file for writing: %1").arg(file.errorString()));
        return false;
    }
    
    if (file.write(data) != data.size()) {
        addError(QString("Failed to write data: %1").arg(file.errorString()));
        return false;
    }
    
    if (!file.commit()) {
        addError(QString("Failed to commit file: %1").arg(file.errorString()));
        return false;
    }
    
    return true;
}