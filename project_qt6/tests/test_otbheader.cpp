#include "otbheader.h"
#include "otbreader.h"
#include "otbwriter.h"
#include <QDebug>
#include <QFile>
#include <QDataStream>
#include <QTemporaryFile>

using namespace OTB;

bool testHeaderValidation() {
    qDebug() << "Testing OTB header validation...";
    
    // Test signature validation
    if (!OtbHeader::validateSignature(0x00000000)) {
        qDebug() << "ERROR: Valid signature rejected";
        return false;
    }
    
    if (OtbHeader::validateSignature(0x12345678)) {
        qDebug() << "ERROR: Invalid signature accepted";
        return false;
    }
    
    qDebug() << "Signature validation: PASSED";
    
    // Test version compatibility
    QString errorString;
    if (!OtbHeader::isVersionSupported(3, 0, errorString)) {
        qDebug() << "ERROR: Valid version rejected:" << errorString;
        return false;
    }
    
    if (OtbHeader::isVersionSupported(99, 0, errorString)) {
        qDebug() << "ERROR: Invalid major version accepted";
        return false;
    }
    
    if (OtbHeader::isVersionSupported(3, 999, errorString)) {
        qDebug() << "ERROR: Invalid minor version accepted";
        return false;
    }
    
    qDebug() << "Version compatibility: PASSED";
    
    return true;
}

bool testHeaderReadWrite() {
    qDebug() << "Testing OTB header read/write...";
    
    // Create test version info
    OtbVersionInfo testVersion;
    testVersion.majorVersion = 3;
    testVersion.minorVersion = 0;
    testVersion.buildNumber = 1;
    testVersion.csdVersion = "Test OTB File";
    
    // Create temporary file
    QTemporaryFile tempFile;
    if (!tempFile.open()) {
        qDebug() << "ERROR: Failed to create temporary file";
        return false;
    }
    
    QString errorString;
    
    // Write header
    {
        QDataStream stream(&tempFile);
        stream.setByteOrder(QDataStream::LittleEndian);
        
        if (!OtbHeader::writeHeader(&stream, testVersion, errorString)) {
            qDebug() << "ERROR: Failed to write header:" << errorString;
            return false;
        }
    }
    
    // Read header back
    tempFile.seek(0);
    {
        QDataStream stream(&tempFile);
        stream.setByteOrder(QDataStream::LittleEndian);
        
        OtbVersionInfo readVersion;
        if (!OtbHeader::readHeader(&stream, readVersion, errorString)) {
            qDebug() << "ERROR: Failed to read header:" << errorString;
            return false;
        }
        
        // Validate header integrity
        tempFile.seek(0);
        if (!OtbHeader::validateHeaderIntegrity(&stream, errorString)) {
            qDebug() << "ERROR: Header integrity validation failed:" << errorString;
            return false;
        }
    }
    
    qDebug() << "Header read/write: PASSED";
    return true;
}

bool testVersionDetection() {
    qDebug() << "Testing OTB version detection...";
    
    // Create temporary file with header
    QTemporaryFile tempFile;
    if (!tempFile.open()) {
        qDebug() << "ERROR: Failed to create temporary file";
        return false;
    }
    
    // Write signature
    QDataStream stream(&tempFile);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << static_cast<quint32>(0x00000000);
    
    tempFile.seek(0);
    
    QString errorString;
    OtbVersionInfo detectedVersion;
    
    if (!OtbHeader::detectVersion(&stream, detectedVersion, errorString)) {
        qDebug() << "ERROR: Version detection failed:" << errorString;
        return false;
    }
    
    qDebug() << "Version detection: PASSED";
    return true;
}

bool testOtbReaderValidation() {
    qDebug() << "Testing OtbReader header validation...";
    
    OtbReader reader;
    QString errorString;
    
    // Test with non-existent file
    if (reader.validateFile("nonexistent.otb", errorString)) {
        qDebug() << "ERROR: Non-existent file validation should fail";
        return false;
    }
    
    qDebug() << "OtbReader validation: PASSED";
    return true;
}

int main() {
    qDebug() << "=== OTB Header Handling Tests ===";
    
    bool allPassed = true;
    
    allPassed &= testHeaderValidation();
    allPassed &= testHeaderReadWrite();
    allPassed &= testVersionDetection();
    allPassed &= testOtbReaderValidation();
    
    if (allPassed) {
        qDebug() << "\n=== ALL TESTS PASSED ===";
        return 0;
    } else {
        qDebug() << "\n=== SOME TESTS FAILED ===";
        return 1;
    }
}