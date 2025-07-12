#include "otbwriter.h"
#include "otbtypes.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

using namespace OTB;

bool testOtbSerialization() {
    qDebug() << "Testing OTB serialization...";
    
    // Create a test ServerItemList
    ServerItemList testItems;
    testItems.majorVersion = 3;
    testItems.minorVersion = 0;
    testItems.buildNumber = 1;
    testItems.clientVersion = 860;
    testItems.description = "Test OTB File";
    
    // Create a test item
    ServerItem testItem;
    testItem.id = 100;
    testItem.clientId = 1000;
    testItem.type = ServerItemType::Ground;
    testItem.name = "Test Item";
    testItem.groundSpeed = 150;
    testItem.movable = true;
    testItem.stackable = false;
    testItem.pickupable = true;
    testItem.minimapColor = 255;
    testItem.lightLevel = 5;
    testItem.lightColor = 0xFF00;
    testItem.stackOrder = TileStackOrder::Ground;
    testItem.tradeAs = 50;
    
    // Initialize sprite hash
    testItem.spriteHash = QByteArray(16, 0);
    for (int i = 0; i < 16; ++i) {
        testItem.spriteHash[i] = static_cast<char>(i);
    }
    
    // Update flags from properties
    testItem.updateFlagsFromProperties();
    
    testItems.add(testItem);
    
    // Test writing
    QString testFilePath = "test_output.otb";
    OtbWriter writer;
    QString errorString;
    
    bool success = writer.write(testFilePath, testItems, errorString);
    
    if (!success) {
        qDebug() << "Failed to write OTB file:" << errorString;
        return false;
    }
    
    // Check if file was created and has content
    QFileInfo fileInfo(testFilePath);
    if (!fileInfo.exists()) {
        qDebug() << "Output file was not created";
        return false;
    }
    
    if (fileInfo.size() == 0) {
        qDebug() << "Output file is empty";
        return false;
    }
    
    qDebug() << "OTB file written successfully!";
    qDebug() << "File size:" << fileInfo.size() << "bytes";
    qDebug() << "File path:" << fileInfo.absoluteFilePath();
    
    // Clean up test file
    QFile::remove(testFilePath);
    
    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "OTB Serialization Test Starting...";
    
    bool success = testOtbSerialization();
    
    if (success) {
        qDebug() << "All tests passed!";
        return 0;
    } else {
        qDebug() << "Tests failed!";
        return 1;
    }
}