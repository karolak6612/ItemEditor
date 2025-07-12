#include "otbwriter.h"
#include "otbheader.h"
#include "otbperformance.h"
#include "otbbackup.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QElapsedTimer>
#include <QFileInfo>

namespace OTB {

OtbWriter::OtbWriter() {
    m_performanceMetrics.reset();
    
    // Initialize performance optimizations
    m_ioBuffer = std::make_unique<IOBuffer>(m_bufferSize);
    
    // Start performance monitoring if enabled
    if (m_performanceMonitoring) {
        PerformanceManager::getMonitor()->startMonitoring();
    }
}

bool OtbWriter::write(const QString& filePath, const ServerItemList& items, QString& errorString) {
    QElapsedTimer perfTimer;
    perfTimer.start();
    m_performanceMetrics.reset();
    
    // Create automatic backup before writing if file exists
    if (QFile::exists(filePath)) {
        try {
            OtbBackupSystem& backupSystem = GlobalBackupSystem::instance();
            BackupResult backupResult = backupSystem.createAutomaticBackup(filePath);
            
            if (!backupResult.success) {
                qWarning() << "Failed to create automatic backup:" << backupResult.errorMessage;
                // Continue with write operation even if backup fails
                // but log the warning for user awareness
            } else {
                qInfo() << "Created automatic backup:" << backupResult.backupId;
            }
        } catch (const std::exception& e) {
            qWarning() << "Exception during backup creation:" << e.what();
            // Continue with write operation
        }
    }
    
    // Configure optimal buffer size based on estimated file size
    qint64 estimatedSize = items.items.size() * 100; // Rough estimate
    if (m_ioBuffer) {
        qint64 optimalBufferSize = PerformanceManager::getOptimizer()->getOptimalBufferSize(estimatedSize);
        m_ioBuffer->setSize(optimalBufferSize);
    }
    
    if (!m_tree.open(filePath, QIODevice::WriteOnly)) {
        errorString = "Failed to create file: " + filePath;
        return false;
    }

    try {
        // Write OTB file header - version is always 0 (as per C# implementation)
        m_tree.writeValue<quint32>(0);

        // Create version info from items
        OtbVersionInfo versionInfo;
        versionInfo.majorVersion = items.majorVersion;
        versionInfo.minorVersion = items.minorVersion;
        versionInfo.buildNumber = items.buildNumber;
        // Format CSDVersion exactly like C# version
        versionInfo.csdVersion = QString("OTB %1.%2.%3-%4.%5")
            .arg(versionInfo.majorVersion)
            .arg(versionInfo.minorVersion)
            .arg(versionInfo.buildNumber)
            .arg(items.clientVersion / 100)
            .arg(items.clientVersion % 100);

        // Write root node
        if (!writeRootNode(items, errorString)) {
            m_tree.close();
            return false;
        }

        // Write item nodes
        for (const ServerItem& item : items.items) {
            if (!writeItemNode(item, errorString)) {
                m_tree.close();
                return false;
            }
        }

        // Close root node
        m_tree.writeNodeEnd();
        
        m_tree.close();
        
        // Record performance metrics
        qint64 totalTime = perfTimer.elapsed();
        QFileInfo fileInfo(filePath);
        qint64 fileSize = fileInfo.size();
        
        if (m_performanceMonitoring) {
            PerformanceManager::getMonitor()->recordWriteOperation(fileSize, totalTime);
            
            // Update our local performance metrics
            m_performanceMetrics.bytesWritten = fileSize;
            m_performanceMetrics.totalWriteTime = totalTime;
            m_performanceMetrics.itemsProcessed = items.items.size();
            m_performanceMetrics.writeOperations = 1;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        errorString = QString("Exception occurred during writing: %1").arg(e.what());
        m_tree.close();
        return false;
    } catch (...) {
        errorString = "Unknown exception occurred during writing";
        m_tree.close();
        return false;
    }
}bool OtbWriter::writeHeader(const OtbVersionInfo& versionInfo, QString& errorString) {
    QDataStream* stream = m_tree.stream();
    if (!stream) {
        errorString = "Failed to get data stream from BinaryTree.";
        return false;
    }

    // Use OtbHeader class for proper header writing
    return OtbHeader::writeHeader(stream, versionInfo, errorString);
}

bool OtbWriter::writeRootNode(const ServerItemList& items, QString& errorString) {
    try {
        // Start root node (type 0)
        m_tree.writeNodeStart(0);
        
        // Write root node flags (4 bytes, unused - always 0)
        m_tree.writeValue<quint32>(0);
        
        // Create version info exactly like C# implementation
        OtbVersionInfo versionInfo;
        versionInfo.majorVersion = items.majorVersion;
        versionInfo.minorVersion = items.minorVersion;
        versionInfo.buildNumber = items.buildNumber;
        versionInfo.csdVersion = QString("OTB %1.%2.%3-%4.%5")
            .arg(versionInfo.majorVersion)
            .arg(versionInfo.minorVersion)
            .arg(versionInfo.buildNumber)
            .arg(items.clientVersion / 100)
            .arg(items.clientVersion % 100);
        
        // Serialize version data exactly like C# BinaryWriter
        QByteArray versionData;
        QDataStream versionStream(&versionData, QIODevice::WriteOnly);
        versionStream.setByteOrder(QDataStream::LittleEndian);
        
        versionStream << versionInfo.majorVersion;
        versionStream << versionInfo.minorVersion;
        versionStream << versionInfo.buildNumber;
        
        // Write CSDVersion as 128-byte ASCII array (exactly like C# implementation)
        QByteArray csdBytes = versionInfo.csdVersion.toLatin1();
        QByteArray csdVersionBytes(128, 0); // Initialize with zeros
        int copyLength = qMin(csdBytes.length(), 127); // Leave room for null terminator
        if (copyLength > 0) {
            memcpy(csdVersionBytes.data(), csdBytes.constData(), copyLength);
        }
        versionStream.writeRawData(csdVersionBytes.constData(), 128);
        
        // Write version property
        m_tree.writeProp(static_cast<quint8>(RootAttribute::Version), versionData);
        
        return true;
        
    } catch (const std::exception& e) {
        errorString = QString("Exception occurred while writing root node: %1").arg(e.what());
        return false;
    } catch (...) {
        errorString = "Unknown exception occurred while writing root node";
        return false;
    }
}bool OtbWriter::writeItemNode(const ServerItem& item, QString& errorString) {
    try {
        // Get the item group for node type (exactly like C# implementation)
        ServerItemGroup itemGroup = getServerItemGroup(item.type);
        
        // Start item node with group type
        m_tree.writeNodeStart(static_cast<quint8>(itemGroup));
        
        // Calculate and write item flags (exactly like C# implementation)
        quint32 flags = calculateItemFlags(item);
        m_tree.writeValue<quint32>(flags);
        
        // Get list of attributes to save (exactly like C# implementation)
        QList<ServerItemAttribute> attributesToSave = getAttributesToSave(item);
        
        // Write each attribute
        for (ServerItemAttribute attribute : attributesToSave) {
            QByteArray attributeData;
            
            switch (attribute) {
                case ServerItemAttribute::ServerID:
                    attributeData = serializeServerID(item.id);
                    break;
                    
                case ServerItemAttribute::ClientID:
                    attributeData = serializeClientID(item.clientId);
                    break;
                    
                case ServerItemAttribute::GroundSpeed:
                    attributeData = serializeGroundSpeed(item.groundSpeed);
                    break;
                    
                case ServerItemAttribute::Name:
                    attributeData = serializeName(item.name);
                    break;
                    
                case ServerItemAttribute::SpriteHash:
                    attributeData = serializeSpriteHash(item.spriteHash);
                    break;
                    
                case ServerItemAttribute::MinimapColor:
                    attributeData = serializeMinimapColor(item.minimapColor);
                    break;
                    
                case ServerItemAttribute::MaxReadWriteChars:
                    attributeData = serializeMaxReadWriteChars(item.maxReadWriteChars);
                    break;
                    
                case ServerItemAttribute::MaxReadChars:
                    attributeData = serializeMaxReadChars(item.maxReadChars);
                    break;
                    
                case ServerItemAttribute::Light:
                    attributeData = serializeLight(item.lightLevel, item.lightColor);
                    break;
                    
                case ServerItemAttribute::StackOrder:
                    attributeData = serializeStackOrder(item.stackOrder);
                    break;
                    
                case ServerItemAttribute::TradeAs:
                    attributeData = serializeTradeAs(item.tradeAs);
                    break;
                    
                default:
                    qWarning() << "Unknown attribute type:" << static_cast<int>(attribute);
                    continue;
            }
            
            // Write the property
            m_tree.writeProp(static_cast<quint8>(attribute), attributeData);
        }
        
        // End item node
        m_tree.writeNodeEnd();
        
        return true;
        
    } catch (const std::exception& e) {
        errorString = QString("Exception occurred while writing item node (ID: %1): %2")
                     .arg(item.id).arg(e.what());
        return false;
    } catch (...) {
        errorString = QString("Unknown exception occurred while writing item node (ID: %1)")
                     .arg(item.id);
        return false;
    }
}

} // namespace OTB