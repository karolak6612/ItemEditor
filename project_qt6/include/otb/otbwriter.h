#ifndef OTBWRITER_H
#define OTBWRITER_H

#include "otbtypes.h"
#include "binarytree.h"
#include "otbheader.h"
#include "otbperformance.h"
#include <QString>
#include <QScopedPointer>
#include <QList>
#include <memory>

namespace OTB {

class OtbWriter {
public:
    OtbWriter();

    // Writes the ServerItemList to an OTB file at the given path.
    // Returns true on success, false on failure.
    // If an error occurs, 'errorString' will contain a description.
    bool write(const QString& filePath, const ServerItemList& items, QString& errorString);
    
    // Writes OTB file header with version information
    // Returns true if header is successfully written, false otherwise
    bool writeHeader(const OtbVersionInfo& versionInfo, QString& errorString);
    
    // Performance optimization settings
    void setBufferSize(qint64 size) { m_bufferSize = size; }
    void setPerformanceMonitoring(bool enabled) { m_performanceMonitoring = enabled; }
    void setCompressionEnabled(bool enabled) { m_compressionEnabled = enabled; }
    
    // Get performance metrics from last operation
    const PerformanceMetrics& getLastPerformanceMetrics() const { return m_performanceMetrics; }

private:
    bool writeRootNode(const ServerItemList& items, QString& errorString);
    bool writeItemNode(const ServerItem& item, QString& errorString);
    
    // Helper methods for attribute serialization
    QByteArray serializeServerID(quint16 serverID);
    QByteArray serializeClientID(quint16 clientID);
    QByteArray serializeGroundSpeed(quint16 groundSpeed);
    QByteArray serializeName(const QString& name);
    QByteArray serializeSpriteHash(const QByteArray& spriteHash);
    QByteArray serializeMinimapColor(quint16 minimapColor);
    QByteArray serializeMaxReadWriteChars(quint16 maxReadWriteChars);
    QByteArray serializeMaxReadChars(quint16 maxReadChars);
    QByteArray serializeLight(quint16 lightLevel, quint16 lightColor);
    QByteArray serializeStackOrder(TileStackOrder stackOrder);
    QByteArray serializeTradeAs(quint16 tradeAs);
    
    // Determines which attributes should be saved for an item
    QList<ServerItemAttribute> getAttributesToSave(const ServerItem& item);
    
    // Converts ServerItemType to ServerItemGroup for node creation
    ServerItemGroup getServerItemGroup(ServerItemType type);
    
    // Calculates flags from item properties (matching C# logic exactly)
    quint32 calculateItemFlags(const ServerItem& item);

    BinaryTree m_tree; // The binary tree utility for file writing
    
    // Performance optimization members
    bool m_performanceMonitoring = true;
    bool m_compressionEnabled = false;
    qint64 m_bufferSize = 64 * 1024; // 64KB default
    PerformanceMetrics m_performanceMetrics;
    std::unique_ptr<IOBuffer> m_ioBuffer;
};

} // namespace OTB

#endif // OTBWRITER_H