#include "otbheader.h"
#include "otbreader.h"
#include <QDebug>
#include <QTemporaryFile>

using namespace OTB;

void testOtbHeaderHandling() {
    qDebug() << "Testing OTB Header Handling Implementation...";
    
    // Test 1: Signature validation
    if (OtbHeader::validateSignature(0x00000000)) {
        qDebug() << "✓ Valid signature validation: PASSED";
    } else {
        qDebug() << "✗ Valid signature validation: FAILED";
    }
    
    if (!OtbHeader::validateSignature(0x12345678)) {
        qDebug() << "✓ Invalid signature rejection: PASSED";
    } else {
        qDebug() << "✗ Invalid signature rejection: FAILED";
    }
    
    // Test 2: Version compatibility
    QString errorString;
    if (OtbHeader::isVersionSupported(3, 0, errorString)) {
        qDebug() << "✓ Valid version support: PASSED";
    } else {
        qDebug() << "✗ Valid version support: FAILED -" << errorString;
    }
    
    if (!OtbHeader::isVersionSupported(99, 0, errorString)) {
        qDebug() << "✓ Invalid version rejection: PASSED";
    } else {
        qDebug() << "✗ Invalid version rejection: FAILED";
    }
    
    // Test 3: Version string formatting
    OtbVersionInfo testVersion;
    testVersion.majorVersion = 3;
    testVersion.minorVersion = 0;
    testVersion.buildNumber = 1;
    testVersion.csdVersion = "Test OTB";
    
    QString versionStr = OtbHeader::getVersionString(testVersion);
    if (versionStr.contains("3.0.1")) {
        qDebug() << "✓ Version string formatting: PASSED -" << versionStr;
    } else {
        qDebug() << "✗ Version string formatting: FAILED -" << versionStr;
    }
    
    // Test 4: OtbReader integration
    OtbReader reader;
    if (!reader.validateFile("nonexistent.otb", errorString)) {
        qDebug() << "✓ Non-existent file validation: PASSED";
    } else {
        qDebug() << "✗ Non-existent file validation: FAILED";
    }
    
    qDebug() << "OTB Header Handling tests completed.";
}