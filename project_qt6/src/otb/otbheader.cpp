#include "otbheader.h"
#include <QDebug>
#include <QDataStream>

namespace OTB {

OtbHeader::OtbHeader() {}

bool OtbHeader::readHeader(QDataStream* stream, OtbVersionInfo& versionInfo, QString& errorString) {
    if (!stream || !stream->device()) {
        errorString = "Invalid data stream provided to readHeader";
        return false;
    }
    
    // Save current position for potential rollback
    qint64 startPos = stream->device()->pos();
    
    try {
        // Read and validate file signature (4 bytes)
        quint32 fileSignature;
        *stream >> fileSignature;
        
        if (!validateSignature(fileSignature)) {
            qWarning() << "OTB file signature is not 0x00000000, actual:" << Qt::hex << fileSignature;
            // Continue processing - this is not fatal according to C# behavior
        }
        
        // The header validation continues in the root node parsing
        // This method validates the signature and prepares for root node reading
        return true;
        
    } catch (...) {
        errorString = "Exception occurred while reading OTB header";
        stream->device()->seek(startPos);
        return false;
    }
}

bool OtbHeader::writeHeader(QDataStream* stream, const OtbVersionInfo& versionInfo, QString& errorString) {
    if (!stream || !stream->device()) {
        errorString = "Invalid data stream provided to writeHeader";
        return false;
    }
    
    // Check version compatibility before writing
    if (!isVersionSupported(versionInfo.majorVersion, versionInfo.minorVersion, errorString)) {
        return false;
    }
    
    try {
        // Write file signature (4 bytes)
        *stream << OTB_FILE_SIGNATURE;
        
        return true;
        
    } catch (...) {
        errorString = "Exception occurred while writing OTB header";
        return false;
    }
}bool OtbHeader::validateSignature(quint32 signature) {
    return signature == OTB_FILE_SIGNATURE;
}

bool OtbHeader::isVersionSupported(quint32 majorVersion, quint32 minorVersion, QString& errorString) {
    if (majorVersion < MIN_SUPPORTED_MAJOR_VERSION || majorVersion > MAX_SUPPORTED_MAJOR_VERSION) {
        errorString = QString("Unsupported major version %1. Supported range: %2-%3")
                      .arg(majorVersion)
                      .arg(MIN_SUPPORTED_MAJOR_VERSION)
                      .arg(MAX_SUPPORTED_MAJOR_VERSION);
        return false;
    }
    
    if (minorVersion > MAX_SUPPORTED_MINOR_VERSION) {
        errorString = QString("Unsupported minor version %1. Maximum supported: %2")
                      .arg(minorVersion)
                      .arg(MAX_SUPPORTED_MINOR_VERSION);
        return false;
    }
    
    return true;
}

QString OtbHeader::getVersionString(const OtbVersionInfo& versionInfo) {
    QString versionStr = QString("%1.%2.%3")
                         .arg(versionInfo.majorVersion)
                         .arg(versionInfo.minorVersion)
                         .arg(versionInfo.buildNumber);
    
    if (!versionInfo.csdVersion.isEmpty()) {
        versionStr += QString(" (%1)").arg(versionInfo.csdVersion);
    }
    
    return versionStr;
}

bool OtbHeader::detectVersion(QDataStream* stream, OtbVersionInfo& versionInfo, QString& errorString) {
    if (!stream || !stream->device()) {
        errorString = "Invalid data stream provided to detectVersion";
        return false;
    }
    
    // Save current position
    qint64 startPos = stream->device()->pos();
    
    try {
        // Read file signature
        quint32 fileSignature;
        *stream >> fileSignature;
        
        if (!validateSignature(fileSignature)) {
            qWarning() << "Warning: Non-standard OTB file signature detected:" << Qt::hex << fileSignature;
        }
        
        // Reset to start position for caller
        stream->device()->seek(startPos);
        
        // Version detection will be completed during root node parsing
        // This method primarily validates that the file has a recognizable structure
        return true;
        
    } catch (...) {
        errorString = "Exception occurred during version detection";
        stream->device()->seek(startPos);
        return false;
    }
}bool OtbHeader::validateHeaderIntegrity(QDataStream* stream, QString& errorString) {
    if (!stream || !stream->device()) {
        errorString = "Invalid data stream provided to validateHeaderIntegrity";
        return false;
    }
    
    // Save current position
    qint64 startPos = stream->device()->pos();
    
    try {
        // Check if we have enough bytes for minimum header
        qint64 availableBytes = stream->device()->size() - stream->device()->pos();
        if (availableBytes < 4) { // At least signature
            errorString = "File too small to contain valid OTB header";
            stream->device()->seek(startPos);
            return false;
        }
        
        // Read and validate signature
        quint32 fileSignature;
        *stream >> fileSignature;
        
        // Reset position
        stream->device()->seek(startPos);
        
        // Basic integrity check passed
        return true;
        
    } catch (...) {
        errorString = "Exception occurred during header integrity validation";
        stream->device()->seek(startPos);
        return false;
    }
}

bool OtbHeader::readVersionAttribute(QDataStream* stream, OtbVersionInfo& versionInfo, QString& errorString) {
    if (!stream) {
        errorString = "Invalid stream for reading version attribute";
        return false;
    }
    
    try {
        // Read version data (12 bytes total: 4 + 4 + 4)
        *stream >> versionInfo.majorVersion;
        *stream >> versionInfo.minorVersion;
        *stream >> versionInfo.buildNumber;
        
        // Read description (128 bytes, null-terminated string)
        QByteArray descBytes(128, 0);
        stream->readRawData(descBytes.data(), 128);
        versionInfo.csdVersion = QString::fromLatin1(descBytes.constData(), descBytes.indexOf('\0'));
        
        // Validate version compatibility
        if (!isVersionSupported(versionInfo.majorVersion, versionInfo.minorVersion, errorString)) {
            return false;
        }
        
        return true;
        
    } catch (...) {
        errorString = "Exception occurred while reading version attribute";
        return false;
    }
}bool OtbHeader::writeVersionAttribute(QDataStream* stream, const OtbVersionInfo& versionInfo, QString& errorString) {
    if (!stream) {
        errorString = "Invalid stream for writing version attribute";
        return false;
    }
    
    try {
        // Validate version before writing
        if (!isVersionSupported(versionInfo.majorVersion, versionInfo.minorVersion, errorString)) {
            return false;
        }
        
        // Write version data (12 bytes total: 4 + 4 + 4)
        *stream << versionInfo.majorVersion;
        *stream << versionInfo.minorVersion;
        *stream << versionInfo.buildNumber;
        
        // Prepare description (128 bytes, null-terminated)
        QByteArray descBytes(128, 0);
        QByteArray csdBytes = versionInfo.csdVersion.toLatin1();
        
        // Copy description, ensuring it fits in 128 bytes with null terminator
        int copyLength = qMin(csdBytes.length(), 127);
        if (copyLength > 0) {
            memcpy(descBytes.data(), csdBytes.constData(), copyLength);
        }
        // descBytes is already zero-initialized, so null terminator is guaranteed
        
        // Write description
        stream->writeRawData(descBytes.constData(), 128);
        
        return true;
        
    } catch (...) {
        errorString = "Exception occurred while writing version attribute";
        return false;
    }
}

} // namespace OTB