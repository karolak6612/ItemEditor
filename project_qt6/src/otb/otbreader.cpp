#include "otbreader.h"
#include "otbheader.h"
#include "otbcache.h"
#include "otbperformance.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QFileInfo>
#include <QElapsedTimer>
#include <functional>

namespace OTB {

OtbReader::OtbReader() {
    m_stats.reset();
    m_performanceMetrics.reset();
    
    // Initialize performance optimizations
    m_ioBuffer = std::make_unique<IOBuffer>(m_bufferSize);
    m_cache = CacheManager::getInstance();
    
    // Start performance monitoring if enabled
    if (m_performanceMonitoring) {
        PerformanceManager::getMonitor()->startMonitoring();
    }
}

bool OtbReader::read(const QString& filePath, ServerItemList& itemsList, QString& errorString) {
    m_timer.start();
    m_stats.reset();
    m_performanceMetrics.reset();
    m_lastErrorCode = OtbReadError::None;
    m_lastErrorMessage.clear();
    
    itemsList.clear(); // Ensure we start with a clean list

    // Performance monitoring
    QElapsedTimer perfTimer;
    perfTimer.start();
    
    // Check cache first if enabled
    if (m_cacheEnabled && m_cache) {
        QByteArray cachedData;
        if (m_cache->getCachedFileData(filePath, cachedData)) {
            if (m_performanceMonitoring) {
                PerformanceManager::getMonitor()->recordCacheHit();
            }
            // TODO: Deserialize from cached data
            qDebug() << "Using cached file data for:" << filePath;
        } else {
            if (m_performanceMonitoring) {
                PerformanceManager::getMonitor()->recordCacheMiss();
            }
        }
    }

    // Enhanced file validation
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        setError(OtbReadError::FileNotFound, "File does not exist: " + filePath);
        errorString = m_lastErrorMessage;
        return false;
    }
    
    if (!fileInfo.isReadable()) {
        setError(OtbReadError::FileAccessDenied, "File is not readable: " + filePath);
        errorString = m_lastErrorMessage;
        return false;
    }
    
    // Check memory constraints for large files
    qint64 fileSize = fileInfo.size();
    if (!checkMemoryConstraints(fileSize * 2)) { // Estimate 2x file size for processing
        setError(OtbReadError::MemoryAllocationFailed, "Insufficient memory for file processing");
        errorString = m_lastErrorMessage;
        return false;
    }
    
    // Configure optimal buffer size based on file size
    if (m_ioBuffer) {
        qint64 optimalBufferSize = PerformanceManager::getOptimizer()->getOptimalBufferSize(fileSize);
        m_ioBuffer->setSize(optimalBufferSize);
    }

    if (!m_tree.open(filePath, QIODevice::ReadOnly)) {
        setError(OtbReadError::FileAccessDenied, "Failed to open file: " + filePath);
        errorString = m_lastErrorMessage;
        return false;
    }

    // Validate header with enhanced error reporting
    if (!validateHeader(errorString)) {
        m_tree.close();
        return false;
    }
    
    // Validate overall node structure
    if (m_strictValidation && !validateNodeStructure(errorString)) {
        m_tree.close();
        return false;
    }

    // Enter the root node with enhanced validation
    if (!m_tree.enterNode()) {
        setError(OtbReadError::InvalidNodeStructure, "OTB Error: Root node start (0xFE) not found or invalid structure.");
        errorString = m_lastErrorMessage;
        m_tree.close();
        return false;
    }

    if (m_tree.getCurrentNodeType() != 0) { // Root node type must be 0
        setError(OtbReadError::InvalidNodeStructure, 
                QString("OTB Error: Invalid root node type. Expected 0, got %1.").arg(m_tree.getCurrentNodeType()));
        errorString = m_lastErrorMessage;
        m_tree.leaveNode();
        m_tree.close();
        return false;
    }

    // Parse root node with enhanced error handling
    if (!parseRootNode(itemsList, errorString)) {
        m_tree.leaveNode();
        m_tree.close();
        return false;
    }

    // Leave root node to access its children
    if (!m_tree.leaveNode()) {
        setError(OtbReadError::InvalidNodeStructure, "OTB Error: Failed to leave root node.");
        errorString = m_lastErrorMessage;
        m_tree.close();
        return false;
    }

    // Parse child item nodes with progress tracking
    while (m_tree.hasNextNode()) {
        if (!m_tree.enterNode()) {
            if (m_strictValidation) {
                setError(OtbReadError::InvalidNodeStructure, "OTB Error: Failed to enter item node.");
                errorString = m_lastErrorMessage;
                m_tree.close();
                return false;
            } else {
                logWarning("Failed to enter item node, skipping");
                continue;
            }
        }

        ServerItem currentItem;
        if (!parseItemNode(currentItem, errorString)) {
            if (m_strictValidation) {
                m_tree.leaveNode();
                m_tree.close();
                return false;
            } else {
                logWarning("Failed to parse item node, skipping: " + errorString);
                m_tree.leaveNode();
                continue;
            }
        }
        
        // Validate item consistency
        if (m_strictValidation && !validateItemConsistency(currentItem, errorString)) {
            if (m_strictValidation) {
                m_tree.leaveNode();
                m_tree.close();
                return false;
            } else {
                logWarning("Item consistency validation failed, skipping: " + errorString);
                m_tree.leaveNode();
                continue;
            }
        }
        
        itemsList.add(currentItem);
        m_stats.itemsProcessed++;
        
        // Cache the item if caching is enabled
        if (m_cacheEnabled && m_cache) {
            m_cache->cacheItem(currentItem.id, currentItem, OtbCache::L2_Medium);
        }

        if (!m_tree.leaveNode()) {
            setError(OtbReadError::InvalidNodeStructure, "OTB Error: Failed to leave item node.");
            errorString = m_lastErrorMessage;
            m_tree.close();
            return false;
        }
    }

    m_tree.close();
    
    // Finalize statistics and performance metrics
    qint64 totalTime = m_timer.elapsed();
    m_stats.readingTimeMs = totalTime;
    m_stats.bytesRead = fileSize;
    
    // Record performance metrics
    if (m_performanceMonitoring) {
        PerformanceManager::getMonitor()->recordReadOperation(fileSize, totalTime);
        PerformanceManager::getMonitor()->recordParseOperation(m_stats.itemsProcessed, totalTime);
        
        // Update our local performance metrics
        m_performanceMetrics.bytesRead = fileSize;
        m_performanceMetrics.totalReadTime = totalTime;
        m_performanceMetrics.itemsProcessed = m_stats.itemsProcessed;
        m_performanceMetrics.readOperations = 1;
    }
    
    // Cache the entire file data if caching is enabled
    if (m_cacheEnabled && m_cache && fileSize < 10 * 1024 * 1024) { // Only cache files < 10MB
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray fileData = file.readAll();
            m_cache->cacheFileData(filePath, fileData);
        }
    }
    
    if (m_detailedLogging) {
        qDebug() << "OTB Read completed successfully:";
        qDebug() << "  Items processed:" << m_stats.itemsProcessed;
        qDebug() << "  Attributes processed:" << m_stats.attributesProcessed;
        qDebug() << "  Reading time:" << m_stats.readingTimeMs << "ms";
        qDebug() << "  Warnings:" << m_stats.warnings.size();
    }
    
    return true;
}

bool OtbReader::parseRootNode(ServerItemList& itemsList, QString& errorString) {
    // Get isolated stream for current node
    QDataStream* nodeStream = m_tree.getCurrentNodeStream();
    if (!nodeStream) {
        setError(OtbReadError::InvalidNodeStructure, "Internal Error: No isolated stream available for root node.");
        errorString = m_lastErrorMessage;
        return false;
    }

    // Verify we're in a root node context (type 0)
    if (m_tree.getCurrentNodeType() != 0) {
        setError(OtbReadError::InvalidNodeStructure, 
                "Internal Error: Expected root node (type 0) in parseRootNode, got type " + QString::number(m_tree.getCurrentNodeType()));
        errorString = m_lastErrorMessage;
        return false;
    }

    // Enhanced data availability check
    if (nodeStream->atEnd()) {
        setError(OtbReadError::UnexpectedEndOfFile, "Root node contains no data");
        errorString = m_lastErrorMessage;
        return false;
    }

    // Read root node flags (4 bytes, unused) - these come first in the node data
    quint32 rootNodeFlags;
    if (nodeStream->device()->bytesAvailable() < 4) {
        setError(OtbReadError::UnexpectedEndOfFile, "Insufficient data for root node flags");
        errorString = m_lastErrorMessage;
        return false;
    }
    *nodeStream >> rootNodeFlags; // Read and discard root node flags

    // Parse attributes with enhanced validation
    while (!nodeStream->atEnd()) {
        quint8 attributeByte;
        *nodeStream >> attributeByte;

        RootAttribute attribute = static_cast<RootAttribute>(attributeByte);
        
        // Check if we have enough data for length field
        if (nodeStream->atEnd()) {
            setError(OtbReadError::UnexpectedEndOfFile,
                    QString("Unexpected end of data while reading length for root attribute %1")
                    .arg(QString::number(static_cast<quint8>(attribute), 16)));
            errorString = m_lastErrorMessage;
            return false;
        }
        
        quint16 dataLen;
        *nodeStream >> dataLen;

        // Enhanced data availability check
        if (nodeStream->device()->bytesAvailable() < dataLen) {
            setError(OtbReadError::UnexpectedEndOfFile,
                    QString("Not enough data for root attribute %1 (need %2 bytes, have %3)")
                    .arg(QString::number(static_cast<quint8>(attribute), 16))
                    .arg(dataLen)
                    .arg(nodeStream->device()->bytesAvailable()));
            errorString = m_lastErrorMessage;
            return false;
        }

        // Read attribute data for validation
        QByteArray attributeData(dataLen, 0);
        nodeStream->readRawData(attributeData.data(), dataLen);
        
        // Validate attribute data
        if (m_strictValidation && !validateRootAttributeData(attribute, dataLen, attributeData, errorString)) {
            return false;
        }
        
        // Create a stream for the attribute data
        QDataStream attrStream(attributeData);
        attrStream.setByteOrder(QDataStream::LittleEndian);

        if (attribute == RootAttribute::Version) {
            if (dataLen != 140) {
                setError(OtbReadError::AttributeValidationFailed,
                        QString("Invalid data length for root Version attribute. Expected 140, got %1.").arg(dataLen));
                errorString = m_lastErrorMessage;
                return false;
            }

            // Read version data with validation
            attrStream >> itemsList.majorVersion;
            attrStream >> itemsList.minorVersion;
            attrStream >> itemsList.buildNumber;

            // Validate version numbers
            if (itemsList.majorVersion > 10 || itemsList.minorVersion > 9999 || itemsList.buildNumber > 9999) {
                if (m_strictValidation) {
                    setError(OtbReadError::AttributeValidationFailed,
                            QString("Suspicious version numbers: %1.%2.%3")
                            .arg(itemsList.majorVersion).arg(itemsList.minorVersion).arg(itemsList.buildNumber));
                    errorString = m_lastErrorMessage;
                    return false;
                } else {
                    logWarning(QString("Suspicious version numbers: %1.%2.%3")
                              .arg(itemsList.majorVersion).arg(itemsList.minorVersion).arg(itemsList.buildNumber));
                }
            }

            QByteArray descBytes(128, 0);
            attrStream.readRawData(descBytes.data(), 128);
            
            // Enhanced description parsing with validation
            int nullPos = descBytes.indexOf('\0');
            if (nullPos == -1) {
                // No null terminator found, use entire buffer but validate for printable characters
                itemsList.description = QString::fromLatin1(descBytes);
                if (m_strictValidation) {
                    for (char c : descBytes) {
                        if (c != 0 && (c < 32 || c > 126)) { // Non-printable ASCII
                            logWarning("Description contains non-printable characters");
                            break;
                        }
                    }
                }
            } else {
                itemsList.description = QString::fromLatin1(descBytes.constData(), nullPos);
            }
            
            if (m_detailedLogging) {
                qDebug() << "Root version parsed:" << itemsList.majorVersion << "." 
                        << itemsList.minorVersion << "." << itemsList.buildNumber;
                qDebug() << "Description:" << itemsList.description;
            }
        } else {
            // Unknown attribute handling
            QString warningMsg = QString("Unknown root attribute 0x%1 with length %2. Skipping.")
                               .arg(static_cast<quint8>(attribute), 2, 16, QChar('0')).arg(dataLen);
            logWarning(warningMsg);
            
            if (m_detailedLogging) {
                qDebug() << warningMsg;
            }
        }
        
        m_stats.attributesProcessed++;
    }
    
    return true;
}

bool OtbReader::parseItemNode(ServerItem& item, QString& errorString) {
    // Get isolated stream for current node
    QDataStream* nodeStream = m_tree.getCurrentNodeStream();
    if (!nodeStream || m_tree.getCurrentNodeType() == 0) {
        setError(OtbReadError::InvalidNodeStructure, "Internal Error: No isolated stream available or root node found in item context.");
        errorString = m_lastErrorMessage;
        return false;
    }

    ServerItemGroup group = static_cast<ServerItemGroup>(m_tree.getCurrentNodeType());
    item.type = static_cast<ServerItemType>(group);
    
    // Validate item type
    if (m_strictValidation && static_cast<quint8>(item.type) > 5) {
        setError(OtbReadError::InvalidItemData, 
                QString("Invalid item type: %1").arg(static_cast<quint8>(item.type)));
        errorString = m_lastErrorMessage;
        return false;
    }

    // Enhanced data availability check
    if (nodeStream->atEnd()) {
        setError(OtbReadError::UnexpectedEndOfFile, "Item node contains no data");
        errorString = m_lastErrorMessage;
        return false;
    }

    // Item flags are the first part of the node data (4 bytes)
    if (nodeStream->device()->bytesAvailable() < 4) {
        setError(OtbReadError::UnexpectedEndOfFile, "Insufficient data for item flags");
        errorString = m_lastErrorMessage;
        return false;
    }
    *nodeStream >> item.flags;

    // Parse attributes with enhanced validation
    while (!nodeStream->atEnd()) {
        quint8 attributeByte;
        *nodeStream >> attributeByte;

        ServerItemAttribute attribute = static_cast<ServerItemAttribute>(attributeByte);

        // Check if we have enough data for length field
        if (nodeStream->atEnd()) {
            setError(OtbReadError::UnexpectedEndOfFile,
                    QString("Unexpected end of data while reading length for item attribute %1")
                    .arg(QString::number(static_cast<quint8>(attribute), 16)));
            errorString = m_lastErrorMessage;
            return false;
        }
        
        quint16 dataLen;
        *nodeStream >> dataLen;

        // Enhanced data availability check
        if (nodeStream->device()->bytesAvailable() < dataLen) {
            setError(OtbReadError::UnexpectedEndOfFile,
                    QString("Not enough data for item attribute %1 (need %2 bytes, have %3)")
                    .arg(QString::number(static_cast<quint8>(attribute), 16))
                    .arg(dataLen)
                    .arg(nodeStream->device()->bytesAvailable()));
            errorString = m_lastErrorMessage;
            return false;
        }

        // Read attribute data for validation
        QByteArray attributeData(dataLen, 0);
        nodeStream->readRawData(attributeData.data(), dataLen);
        
        // Validate attribute data
        if (m_strictValidation && !validateAttributeData(attribute, dataLen, attributeData, errorString)) {
            return false;
        }
        
        // Create a stream for the attribute data
        QDataStream attrStream(attributeData);
        attrStream.setByteOrder(QDataStream::LittleEndian);

        switch (attribute) {
            case ServerItemAttribute::ServerID:
                if (dataLen != 2) { 
                    setError(OtbReadError::AttributeValidationFailed, "Invalid length for ServerID");
                    errorString = m_lastErrorMessage;
                    return false; 
                }
                attrStream >> item.id;
                if (m_strictValidation && (item.id == 0 || item.id > 65000)) {
                    logWarning(QString("Suspicious ServerID: %1").arg(item.id));
                }
                break;
                
            case ServerItemAttribute::ClientID:
                if (dataLen != 2) { 
                    setError(OtbReadError::AttributeValidationFailed, "Invalid length for ClientID");
                    errorString = m_lastErrorMessage;
                    return false; 
                }
                attrStream >> item.clientId;
                if (m_strictValidation && item.clientId > 65000) {
                    logWarning(QString("Suspicious ClientID: %1").arg(item.clientId));
                }
                break;
                
            case ServerItemAttribute::Name:
                {
                    // Enhanced name validation
                    item.name = QString::fromUtf8(attributeData);
                    if (m_strictValidation) {
                        if (item.name.length() > 255) {
                            logWarning("Item name exceeds 255 characters");
                        }
                        if (item.name.contains('\0')) {
                            logWarning("Item name contains null characters");
                        }
                    }
                }
                break;
                
            case ServerItemAttribute::GroundSpeed:
                if (dataLen != 2) { 
                    setError(OtbReadError::AttributeValidationFailed, "Invalid length for GroundSpeed");
                    errorString = m_lastErrorMessage;
                    return false; 
                }
                attrStream >> item.groundSpeed;
                if (m_strictValidation && item.groundSpeed > 1000) {
                    logWarning(QString("Unusual ground speed: %1").arg(item.groundSpeed));
                }
                break;

            case ServerItemAttribute::SpriteHash:
                if (dataLen != 16) { 
                    setError(OtbReadError::AttributeValidationFailed, "Invalid length for SpriteHash");
                    errorString = m_lastErrorMessage;
                    return false; 
                }
                item.spriteHash = attributeData;
                break;
                
            case ServerItemAttribute::MinimapColor:
                if (dataLen != 2) { 
                    setError(OtbReadError::AttributeValidationFailed, "Invalid length for MinimapColor");
                    errorString = m_lastErrorMessage;
                    return false; 
                }
                attrStream >> item.minimapColor;
                break;
                
            case ServerItemAttribute::MaxReadWriteChars:
                if (dataLen != 2) { 
                    setError(OtbReadError::AttributeValidationFailed, "Invalid length for MaxReadWriteChars");
                    errorString = m_lastErrorMessage;
                    return false; 
                }
                attrStream >> item.maxReadWriteChars;
                if (m_strictValidation && item.maxReadWriteChars > 10000) {
                    logWarning(QString("Unusual MaxReadWriteChars: %1").arg(item.maxReadWriteChars));
                }
                break;
                
            case ServerItemAttribute::MaxReadChars:
                if (dataLen != 2) { 
                    setError(OtbReadError::AttributeValidationFailed, "Invalid length for MaxReadChars");
                    errorString = m_lastErrorMessage;
                    return false; 
                }
                attrStream >> item.maxReadChars;
                if (m_strictValidation && item.maxReadChars > 10000) {
                    logWarning(QString("Unusual MaxReadChars: %1").arg(item.maxReadChars));
                }
                break;
                
            case ServerItemAttribute::Light:
                if (dataLen != 4) { 
                    setError(OtbReadError::AttributeValidationFailed, "Invalid length for Light");
                    errorString = m_lastErrorMessage;
                    return false; 
                }
                attrStream >> item.lightLevel;
                attrStream >> item.lightColor;
                if (m_strictValidation && item.lightLevel > 255) {
                    logWarning(QString("Unusual light level: %1").arg(item.lightLevel));
                }
                break;
                
            case ServerItemAttribute::StackOrder:
                if (dataLen != 1) { 
                    setError(OtbReadError::AttributeValidationFailed, "Invalid length for StackOrder");
                    errorString = m_lastErrorMessage;
                    return false; 
                }
                {
                    quint8 stackOrderValue;
                    attrStream >> stackOrderValue;
                    item.stackOrder = static_cast<TileStackOrder>(stackOrderValue);
                    item.hasStackOrder = true;
                    
                    if (m_strictValidation && stackOrderValue > 5) {
                        logWarning(QString("Invalid stack order value: %1").arg(stackOrderValue));
                    }
                }
                break;
                
            case ServerItemAttribute::TradeAs:
                if (dataLen != 2) { 
                    setError(OtbReadError::AttributeValidationFailed, "Invalid length for TradeAs");
                    errorString = m_lastErrorMessage;
                    return false; 
                }
                attrStream >> item.tradeAs;
                break;
                
            default:
                // Unknown attribute handling with detailed logging
                QString warningMsg = QString("Unknown item attribute 0x%1 with length %2 for item type %3. Skipping.")
                                   .arg(static_cast<quint8>(attribute), 2, 16, QChar('0'))
                                   .arg(dataLen)
                                   .arg(m_tree.getCurrentNodeType());
                logWarning(warningMsg);
                
                if (m_detailedLogging) {
                    qDebug() << warningMsg;
                }
                break;
        }
        
        m_stats.attributesProcessed++;
    }

    // Initialize sprite hash if empty and not deprecated
    if (item.spriteHash.isEmpty() && item.type != ServerItemType::Deprecated) {
        item.spriteHash.fill(0, 16);
    }

    // Update boolean properties from flags
    item.updatePropertiesFromFlags();
    
    if (m_detailedLogging) {
        qDebug() << "Parsed item:" << item.id << item.name << "type:" << static_cast<int>(item.type);
    }
    
    return true;
}

bool OtbReader::validateHeader(QString& errorString) {
    QDataStream* stream = m_tree.stream();
    if (!stream) {
        setError(OtbReadError::InvalidHeader, "Failed to get data stream from BinaryTree.");
        errorString = m_lastErrorMessage;
        return false;
    }

    // Use OtbHeader class for comprehensive validation
    OtbVersionInfo versionInfo;
    if (!OtbHeader::readHeader(stream, versionInfo, errorString)) {
        setError(OtbReadError::InvalidHeader, "Header validation failed: " + errorString);
        errorString = m_lastErrorMessage;
        return false;
    }

    // Additional integrity checks
    if (!OtbHeader::validateHeaderIntegrity(stream, errorString)) {
        setError(OtbReadError::InvalidHeader, "Header integrity check failed: " + errorString);
        errorString = m_lastErrorMessage;
        return false;
    }

    if (m_detailedLogging) {
        qDebug() << "Header validated successfully";
        qDebug() << "Version:" << versionInfo.majorVersion << "." << versionInfo.minorVersion << "." << versionInfo.buildNumber;
    }

    return true;
}

bool OtbReader::validateFile(const QString& filePath, QString& errorString) {
    if (!QFile::exists(filePath)) {
        setError(OtbReadError::FileNotFound, "File does not exist: " + filePath);
        errorString = m_lastErrorMessage;
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(OtbReadError::FileAccessDenied, "Failed to open file for validation: " + filePath);
        errorString = m_lastErrorMessage;
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    // Validate header
    OtbVersionInfo versionInfo;
    if (!OtbHeader::readHeader(&stream, versionInfo, errorString)) {
        setError(OtbReadError::InvalidHeader, "Header validation failed: " + errorString);
        errorString = m_lastErrorMessage;
        return false;
    }

    if (!OtbHeader::validateHeaderIntegrity(&stream, errorString)) {
        setError(OtbReadError::InvalidHeader, "Header integrity check failed: " + errorString);
        errorString = m_lastErrorMessage;
        return false;
    }

    return true;
}

bool OtbReader::detectFileVersion(const QString& filePath, OtbVersionInfo& versionInfo, QString& errorString) {
    if (!QFile::exists(filePath)) {
        setError(OtbReadError::FileNotFound, "File does not exist: " + filePath);
        errorString = m_lastErrorMessage;
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(OtbReadError::FileAccessDenied, "Failed to open file for version detection: " + filePath);
        errorString = m_lastErrorMessage;
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    return OtbHeader::detectVersion(&stream, versionInfo, errorString);
}

// Enhanced validation methods
bool OtbReader::validateNodeStructure(QString& errorString) {
    // This is a placeholder for comprehensive node structure validation
    // In a full implementation, this would traverse the entire tree structure
    // to ensure proper nesting and node markers
    return true;
}

bool OtbReader::validateAttributeData(ServerItemAttribute attribute, quint16 dataLen, 
                                     const QByteArray& data, QString& errorString) {
    // Validate attribute data based on expected formats
    switch (attribute) {
        case ServerItemAttribute::ServerID:
        case ServerItemAttribute::ClientID:
        case ServerItemAttribute::GroundSpeed:
        case ServerItemAttribute::MinimapColor:
        case ServerItemAttribute::MaxReadWriteChars:
        case ServerItemAttribute::MaxReadChars:
        case ServerItemAttribute::TradeAs:
            return dataLen == 2;
            
        case ServerItemAttribute::Light:
            return dataLen == 4;
            
        case ServerItemAttribute::StackOrder:
            return dataLen == 1;
            
        case ServerItemAttribute::SpriteHash:
            return dataLen == 16;
            
        case ServerItemAttribute::Name:
            // Names can be variable length, but should be reasonable
            return dataLen <= 1024;
            
        default:
            // Unknown attributes - allow any length
            return true;
    }
}

bool OtbReader::validateRootAttributeData(RootAttribute attribute, quint16 dataLen, 
                                         const QByteArray& data, QString& errorString) {
    switch (attribute) {
        case RootAttribute::Version:
            return dataLen == 140;
        default:
            return true;
    }
}

bool OtbReader::validateItemConsistency(const ServerItem& item, QString& errorString) {
    // Validate item data consistency
    if (item.id == 0 && item.type != ServerItemType::Deprecated) {
        setError(OtbReadError::InvalidItemData, "Item has invalid ID 0");
        errorString = m_lastErrorMessage;
        return false;
    }
    
    if (item.name.isEmpty() && item.type != ServerItemType::Deprecated) {
        logWarning(QString("Item %1 has empty name").arg(item.id));
    }
    
    // Validate flag consistency
    if (item.hasStackOrder && item.stackOrder == TileStackOrder::None) {
        logWarning(QString("Item %1 has stack order flag but no stack order value").arg(item.id));
    }
    
    return true;
}

bool OtbReader::validateFileIntegrity(const QString& filePath, QString& errorString) {
    m_timer.start();
    m_stats.reset();
    
    // Enhanced file integrity validation
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        setError(OtbReadError::FileNotFound, "File does not exist: " + filePath);
        errorString = m_lastErrorMessage;
        return false;
    }
    
    if (fileInfo.size() == 0) {
        setError(OtbReadError::CorruptedData, "File is empty");
        errorString = m_lastErrorMessage;
        return false;
    }
    
    if (fileInfo.size() < 100) { // Minimum reasonable OTB file size
        setError(OtbReadError::CorruptedData, "File too small to be valid OTB");
        errorString = m_lastErrorMessage;
        return false;
    }
    
    // Validate basic file structure
    if (!m_tree.open(filePath, QIODevice::ReadOnly)) {
        setError(OtbReadError::FileAccessDenied, "Failed to open file for integrity check");
        errorString = m_lastErrorMessage;
        return false;
    }
    
    bool result = validateHeader(errorString) && validateNodeStructure(errorString);
    m_tree.close();
    
    return result;
}

bool OtbReader::readPartial(const QString& filePath, ServerItemList& items, 
                           quint16 startId, quint16 endId, QString& errorString) {
    // Implementation for partial reading would filter items by ID range
    // For now, delegate to full read and filter results
    if (!read(filePath, items, errorString)) {
        return false;
    }
    
    // Filter items by ID range
    QList<ServerItem> filteredItems;
    for (const ServerItem& item : items.items) {
        if (item.id >= startId && item.id <= endId) {
            filteredItems.append(item);
        }
    }
    
    items.items = filteredItems;
    return true;
}

bool OtbReader::readWithProgress(const QString& filePath, ServerItemList& items, 
                                QString& errorString, 
                                std::function<void(int)> progressCallback) {
    // For now, delegate to regular read
    // In a full implementation, this would provide progress updates
    if (progressCallback) {
        progressCallback(0);
    }
    
    bool result = read(filePath, items, errorString);
    
    if (progressCallback) {
        progressCallback(100);
    }
    
    return result;
}

// Error handling and utility methods
void OtbReader::setError(OtbReadError errorCode, const QString& message) {
    m_lastErrorCode = errorCode;
    m_lastErrorMessage = message;
    
    if (m_detailedLogging) {
        qDebug() << "OtbReader Error:" << message;
    }
}

void OtbReader::logWarning(const QString& warning) {
    m_stats.warnings.append(warning);
    
    if (m_detailedLogging) {
        qDebug() << "OtbReader Warning:" << warning;
    }
}

bool OtbReader::checkMemoryConstraints(qint64 requiredBytes) {
    // Basic memory constraint check
    // In a production implementation, this would check available system memory
    const qint64 maxAllowedBytes = 1024 * 1024 * 1024; // 1GB limit
    return requiredBytes <= maxAllowedBytes;
}

void OtbReader::optimizeMemoryUsage() {
    // Placeholder for memory optimization
    // Could implement memory pool management, buffer reuse, etc.
}

} // namespace OTB