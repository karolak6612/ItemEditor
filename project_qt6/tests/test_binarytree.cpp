#include "otb/binarytree.h"
#include "test_otbheader_integration.cpp"
#include <QDebug>
#include <QTemporaryFile>
#include <QBuffer>

namespace OTB {

bool testBinaryTreeBasicOperations() {
    qDebug() << "Testing BinaryTree basic operations...";
    
    // Test 1: Create temporary file and test basic write/read
    QTemporaryFile tempFile;
    if (!tempFile.open()) {
        qDebug() << "Failed to create temporary file";
        return false;
    }
    
    QString testPath = tempFile.fileName();
    tempFile.close();
    
    // Test writing
    {
        BinaryTree writer;
        if (!writer.open(testPath, QIODevice::WriteOnly)) {
            qDebug() << "Failed to open file for writing";
            return false;
        }
        
        // Write root node with version info (matching C# OtbWriter pattern)
        writer.writeNodeStart(0x00); // Root node type
        
        // Write version property (matching C# RootAttribute.Version = 0x01)
        QByteArray versionData;
        QDataStream versionStream(&versionData, QIODevice::WriteOnly);
        versionStream.setByteOrder(QDataStream::LittleEndian);
        versionStream << quint32(1); // major version
        versionStream << quint32(2); // minor version  
        versionStream << quint32(3); // build number
        
        // Add 128 bytes padding (matching C# implementation)
        QByteArray padding(128, 0);
        versionData.append(padding);
        
        writer.writeProp(0x01, versionData); // RootAttribute.Version
        
        // Write child node
        writer.writeNodeStart(0x01); // Item node type
        writer.writeValue<quint16>(100); // Test item ID
        writer.writeString("TestItem", false); // Test name without length prefix
        writer.writeNodeEnd();
        
        writer.writeNodeEnd(); // Close root
        writer.close();
    }
    
    // Test reading
    {
        BinaryTree reader;
        if (!reader.open(testPath, QIODevice::ReadOnly)) {
            qDebug() << "Failed to open file for reading";
            return false;
        }
        
        // Read root node
        if (!reader.enterNode()) {
            qDebug() << "Failed to enter root node";
            return false;
        }
        
        if (reader.getCurrentNodeType() != 0x00) {
            qDebug() << "Root node type mismatch. Expected 0x00, got" << reader.getCurrentNodeType();
            return false;
        }
        
        // Test isolated stream reading
        QDataStream* nodeStream = reader.getCurrentNodeStream();
        if (!nodeStream) {
            qDebug() << "Failed to get isolated node stream";
            return false;
        }
        
        // Skip to version property reading (would need property parsing in real implementation)
        QByteArray nodeData = reader.extractNodeData();
        if (nodeData.isEmpty()) {
            qDebug() << "Node data extraction failed";
            return false;
        }
        
        // Check if there's a child node
        if (!reader.hasNextNode()) {
            qDebug() << "No child node found";
            return false;
        }
        
        // Enter child node
        if (!reader.enterNode()) {
            qDebug() << "Failed to enter child node";
            return false;
        }
        
        if (reader.getCurrentNodeType() != 0x01) {
            qDebug() << "Child node type mismatch. Expected 0x01, got" << reader.getCurrentNodeType();
            return false;
        }
        
        reader.leaveNode(); // Leave child
        reader.leaveNode(); // Leave root
        reader.close();
    }
    
    qDebug() << "BinaryTree basic operations test PASSED";
    return true;
}

bool testBinaryTreeEscaping() {
    qDebug() << "Testing BinaryTree escape character handling...";
    
    QTemporaryFile tempFile;
    if (!tempFile.open()) {
        qDebug() << "Failed to create temporary file";
        return false;
    }
    
    QString testPath = tempFile.fileName();
    tempFile.close();
    
    // Test data containing special characters that need escaping
    QByteArray testData;
    testData.append(static_cast<char>(NODE_START));  // 0xFE
    testData.append(static_cast<char>(NODE_END));    // 0xFF  
    testData.append(static_cast<char>(ESCAPE_CHAR)); // 0xFD
    testData.append("normal_data");
    
    // Write test data
    {
        BinaryTree writer;
        if (!writer.open(testPath, QIODevice::WriteOnly)) {
            qDebug() << "Failed to open file for writing";
            return false;
        }
        
        writer.writeNodeStart(0x02); // Test node type
        writer.writeBytes(testData); // This should properly escape special chars
        writer.writeNodeEnd();
        writer.close();
    }
    
    // Read and verify
    {
        BinaryTree reader;
        if (!reader.open(testPath, QIODevice::ReadOnly)) {
            qDebug() << "Failed to open file for reading";
            return false;
        }
        
        if (!reader.enterNode()) {
            qDebug() << "Failed to enter test node";
            return false;
        }
        
        QByteArray readData = reader.readBytes(testData.length());
        if (readData != testData) {
            qDebug() << "Escape character handling failed";
            qDebug() << "Expected:" << testData.toHex();
            qDebug() << "Got:" << readData.toHex();
            return false;
        }
        
        reader.close();
    }
    
    qDebug() << "BinaryTree escape character test PASSED";
    return true;
}

bool testBinaryTreeCompatibility() {
    qDebug() << "Testing BinaryTree C# compatibility...";
    
    // Test template value operations
    QTemporaryFile tempFile;
    if (!tempFile.open()) {
        qDebug() << "Failed to create temporary file";
        return false;
    }
    
    QString testPath = tempFile.fileName();
    tempFile.close();
    
    // Test various data types (matching C# BinaryWriter/Reader)
    {
        BinaryTree writer;
        if (!writer.open(testPath, QIODevice::WriteOnly)) {
            qDebug() << "Failed to open file for writing";
            return false;
        }
        
        writer.writeNodeStart(0x03);
        writer.writeValue<quint8>(255);
        writer.writeValue<quint16>(65535);
        writer.writeValue<quint32>(4294967295U);
        writer.writeNodeEnd();
        writer.close();
    }
    
    {
        BinaryTree reader;
        if (!reader.open(testPath, QIODevice::ReadOnly)) {
            qDebug() << "Failed to open file for reading";
            return false;
        }
        
        if (!reader.enterNode()) {
            qDebug() << "Failed to enter test node";
            return false;
        }
        
        quint8 val8 = reader.readValue<quint8>();
        quint16 val16 = reader.readValue<quint16>();
        quint32 val32 = reader.readValue<quint32>();
        
        if (val8 != 255 || val16 != 65535 || val32 != 4294967295U) {
            qDebug() << "Value read/write mismatch";
            qDebug() << "Expected: 255, 65535, 4294967295";
            qDebug() << "Got:" << val8 << val16 << val32;
            return false;
        }
        
        reader.close();
    }
    
    qDebug() << "BinaryTree C# compatibility test PASSED";
    return true;
}

} // namespace OTB

// Test runner function
bool runBinaryTreeTests() {
    qDebug() << "=== Running BinaryTree Tests ===";
    
    bool allPassed = true;
    allPassed &= OTB::testBinaryTreeBasicOperations();
    allPassed &= OTB::testBinaryTreeEscaping();
    allPassed &= OTB::testBinaryTreeCompatibility();
    
    if (allPassed) {
        qDebug() << "=== All BinaryTree Tests PASSED ===";
    } else {
        qDebug() << "=== Some BinaryTree Tests FAILED ===";
    }
    
    return allPassed;
}

// Test runner for OTB Header functionality
bool runOtbHeaderTests() {
    qDebug() << "=== Running OTB Header Tests ===";
    
    testOtbHeaderHandling();
    
    qDebug() << "=== OTB Header Tests Completed ===";
    return true;
}