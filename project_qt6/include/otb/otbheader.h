#ifndef OTBHEADER_H
#define OTBHEADER_H

#include "otbtypes.h"
#include <QString>
#include <QDataStream>
#include <QIODevice>

namespace OTB {

/**
 * @brief OTB file header constants and validation
 * 
 * This class handles OTB file header reading, writing, and validation
 * to ensure compatibility with the C# implementation.
 */
class OtbHeader {
public:
    // OTB file signature constants
    static const quint32 OTB_FILE_SIGNATURE = 0x00000000;
    static const quint8 ROOT_NODE_TYPE = 0x00;
    static const quint16 VERSION_ATTRIBUTE_LENGTH = 140; // 4 + 4 + 4 + 128 bytes
    
    // Version compatibility constants
    static const quint32 MIN_SUPPORTED_MAJOR_VERSION = 1;
    static const quint32 MAX_SUPPORTED_MAJOR_VERSION = 3;
    static const quint32 MIN_SUPPORTED_MINOR_VERSION = 0;
    static const quint32 MAX_SUPPORTED_MINOR_VERSION = 60;
    
    OtbHeader();
    
    /**
     * @brief Read and validate OTB file header
     * @param stream Data stream positioned at the beginning of the file
     * @param versionInfo Output parameter for version information
     * @param errorString Output parameter for error description
     * @return true if header is valid and successfully read, false otherwise
     */
    static bool readHeader(QDataStream* stream, OtbVersionInfo& versionInfo, QString& errorString);
    
    /**
     * @brief Write OTB file header
     * @param stream Data stream positioned at the beginning of the file
     * @param versionInfo Version information to write
     * @param errorString Output parameter for error description
     * @return true if header is successfully written, false otherwise
     */
    static bool writeHeader(QDataStream* stream, const OtbVersionInfo& versionInfo, QString& errorString);
    
    /**
     * @brief Validate file signature
     * @param signature The signature read from the file
     * @return true if signature is valid, false otherwise
     */
    static bool validateSignature(quint32 signature);
    
    /**
     * @brief Check version compatibility
     * @param majorVersion Major version number
     * @param minorVersion Minor version number
     * @param errorString Output parameter for error description
     * @return true if version is supported, false otherwise
     */
    static bool isVersionSupported(quint32 majorVersion, quint32 minorVersion, QString& errorString);
    
    /**
     * @brief Get version string representation
     * @param versionInfo Version information
     * @return Formatted version string
     */
    static QString getVersionString(const OtbVersionInfo& versionInfo);
    
    /**
     * @brief Detect OTB file format version from header
     * @param stream Data stream positioned at the beginning of the file
     * @param versionInfo Output parameter for detected version
     * @param errorString Output parameter for error description
     * @return true if version is successfully detected, false otherwise
     */
    static bool detectVersion(QDataStream* stream, OtbVersionInfo& versionInfo, QString& errorString);
    
    /**
     * @brief Validate header integrity
     * @param stream Data stream positioned at the beginning of the file
     * @param errorString Output parameter for error description
     * @return true if header structure is valid, false otherwise
     */
    static bool validateHeaderIntegrity(QDataStream* stream, QString& errorString);

private:
    /**
     * @brief Read version attribute from root node
     * @param stream Data stream positioned at version attribute
     * @param versionInfo Output parameter for version information
     * @param errorString Output parameter for error description
     * @return true if version is successfully read, false otherwise
     */
    static bool readVersionAttribute(QDataStream* stream, OtbVersionInfo& versionInfo, QString& errorString);
    
    /**
     * @brief Write version attribute to root node
     * @param stream Data stream positioned for writing version attribute
     * @param versionInfo Version information to write
     * @param errorString Output parameter for error description
     * @return true if version is successfully written, false otherwise
     */
    static bool writeVersionAttribute(QDataStream* stream, const OtbVersionInfo& versionInfo, QString& errorString);
};

} // namespace OTB

#endif // OTBHEADER_H