#include "otbreader.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>

namespace OTB {

OtbReader::OtbReader() {}

bool OtbReader::read(const QString& filePath, ServerItemList& itemsList, QString& errorString) {
    itemsList.clear(); // Ensure we start with a clean list

    if (!m_tree.open(filePath, QIODevice::ReadOnly)) {
        errorString = "Failed to open file: " + filePath;
        return false;
    }

    QDataStream* stream = m_tree.stream();
    if (!stream) {
        errorString = "Failed to get data stream from BinaryTree.";
        m_tree.close();
        return false;
    }

    // OTB Version (4 bytes, unused according to C# OtbReader - likely file signature 0x00000000)
    // The C# code implies the first 4 bytes are a version/signature, then node handling begins.
    // Let's assume the BinaryTree handles the initial markers if they are part of its own structure,
    // or we read them here if they are global file headers.
    // C# OtbReader:
    // reader.GetRootNode(); node.ReadByte(); node.ReadUInt32();
    // This means the BinaryTree is expected to handle the file's initial 0x00000000 signature
    // then NODE_START, then the root node type (0), then root node flags (4 bytes).
    // Our BinaryTree::enterNode() reads NODE_START and then node_type.
    // The 4-byte file signature and root node flags need to be handled.

    quint32 fileSignature;
    *stream >> fileSignature; // Read the initial 4-byte "version" or signature. Should be 0.
    if (fileSignature != 0) {
        qWarning() << "OTB file signature is not 0x00000000, actual:" << Qt::hex << fileSignature;
        // This is not a fatal error according to C# behavior, but good to note.
    }

    // Enter the root node
    if (!m_tree.enterNode()) {
        errorString = "OTB Error: Root node start (0xFE) not found or invalid structure.";
        m_tree.close();
        return false;
    }

    if (m_tree.getCurrentNodeType() != 0) { // Root node type must be 0
        errorString = QString("OTB Error: Invalid root node type. Expected 0, got %1.").arg(m_tree.getCurrentNodeType());
        m_tree.leaveNode(); // Attempt to clean up stack
        m_tree.close();
        return false;
    }

    // Root node flags (4 bytes), unused. These are part of the node attributes in C# BinaryTreeReader's view.
    // In our model, they are read after node type, before properties.
    // We need to ensure our stream position is correct for parseRootNode.
    // The C# `BinaryTreeReader.GetRootNode()` returns a `BinaryReader` whose position is *after* the node type and flags.
    // Our `enterNode` makes `propsDataStartPos` point after node type. So flags are the first thing to read from `propsDataStartPos`.
    /* quint32 rootNodeFlags = */ m_tree.readValue<quint32>(); // Read and discard root node flags.

    if (!parseRootNode(itemsList, errorString)) {
        m_tree.leaveNode();
        m_tree.close();
        return false;
    }

    // After parsing root's properties, check for child item nodes.
    // The stream should be at m_nodeStack.top().childrenStartPos
    stream->device()->seek(m_tree.getCurrentNodeChildrenStartPos());


    while (m_tree.hasNextNode()) { // Check if a child node (item) exists
        if (!m_tree.enterNode()) { // Enter the item node
            errorString = "OTB Error: Failed to enter item node.";
            // Attempt to leave root node before closing
            if(!m_nodeStack.isEmpty() && m_nodeStack.top().type == 0) m_tree.leaveNode();
            m_tree.close();
            return false;
        }

        ServerItem currentItem;
        if (!parseItemNode(currentItem, errorString)) {
            m_tree.leaveNode(); // Leave current failed item node
            if(!m_nodeStack.isEmpty() && m_nodeStack.top().type == 0) m_tree.leaveNode(); // Leave root
            m_tree.close();
            return false;
        }
        itemsList.add(currentItem);

        if (!m_tree.leaveNode()) { // Leave the successfully parsed item node
            errorString = "OTB Error: Failed to leave item node.";
             if(!m_nodeStack.isEmpty() && m_nodeStack.top().type == 0) m_tree.leaveNode(); // Leave root
            m_tree.close();
            return false;
        }
    }

    // After all item nodes (or if there were none), leave the root node.
    if (!m_tree.leaveNode()) {
        errorString = "OTB Error: Missing end marker for root node or structure error.";
        m_tree.close();
        return false;
    }

    if (!m_nodeStack.isEmpty()) {
        errorString = "OTB Error: Node stack not empty after parsing. Structural error.";
        m_tree.close();
        return false;
    }

    m_tree.close();
    return true;
}

bool OtbReader::parseRootNode(ServerItemList& itemsList, QString& errorString) {
    QDataStream* stream = m_tree.stream();
    if (!stream || m_nodeStack.isEmpty()) { // Check m_nodeStack for current node context
        errorString = "Internal Error: Stream not available or no current node in parseRootNode.";
        return false;
    }

    // Properties of the root node are read from propsDataStartPos up to propsDataEndPos
    while (stream->device()->pos() < m_tree.getCurrentNodeDataEndPos() && !stream->atEnd()) {
        quint8 attributeByte;
        *stream >> attributeByte; // Read attribute type (not escaped)

        // NODE_END should not appear here as per getCurrentNodeDataEndPos logic in BinaryTree
        // but as a safeguard:
        if (attributeByte == NODE_END || attributeByte == NODE_START || attributeByte == ESCAPE_CHAR) {
             errorString = QString("OTB Error: Unexpected control character (0x%1) where attribute was expected in root node properties.").arg(QString::number(attributeByte, 16));
             return false;
        }

        RootAttribute attribute = static_cast<RootAttribute>(attributeByte);
        quint16 dataLen;
        *stream >> dataLen; // Read data length (not escaped)

        qint64 expectedPosAfterData = stream->device()->pos() + dataLen;
        if (expectedPosAfterData > m_tree.getCurrentNodeDataEndPos()) {
            errorString = QString("OTB Error: Attribute data length (%1 bytes) for root attribute %2 exceeds node property boundary.")
                          .arg(dataLen).arg(QString::number(static_cast<quint8>(attribute),16));
            return false;
        }

        if (attribute == RootAttribute::Version) {
            if (dataLen != 140) {
                errorString = QString("OTB Error: Invalid data length for root Version attribute. Expected 140, got %1.").arg(dataLen);
                return false;
            }
            // These values are part of the node's data, so they are subject to escaping if BinaryTree::readValue uses readByteEscaped.
            // For simple types like quint32, QDataStream handles endianness but not OTB's byte-level escaping.
            // We need BinaryTree::readValue<T> to use readBytes(sizeof(T)) then cast.
            // Or, OtbReader reads bytes then converts. Let's assume readValue handles it.
            itemsList.majorVersion = m_tree.readValue<quint32>();
            itemsList.minorVersion = m_tree.readValue<quint32>();
            itemsList.buildNumber = m_tree.readValue<quint32>();

            QByteArray descBytes = m_tree.readBytes(128); // readBytes must handle escaping
            itemsList.description = QString::fromLatin1(descBytes.constData(), descBytes.indexOf('\0'));
        } else {
            qWarning() << "OTB Warning: Unknown root attribute" << Qt::hex << static_cast<quint8>(attribute) << "with length" << dataLen << ". Skipping.";
            if (!m_tree.skipBytes(dataLen)) {
                 errorString = QString("OTB Error: Failed to skip %1 bytes for unknown root attribute %2.").arg(dataLen).arg(QString::number(static_cast<quint8>(attribute), 16));
                 return false;
            }
        }
    }
     // Ensure stream is positioned exactly at propsDataEndPos after reading all props
    if (stream->device()->pos() != m_tree.getCurrentNodeDataEndPos()) {
        qWarning() << "OTB Warning: Stream position" << stream->device()->pos()
                   << "does not match expected end of root properties" << m_tree.getCurrentNodeDataEndPos()
                   << ". Potential parsing error or extra data.";
        // Optionally, make this an error:
        // errorString = "Stream position mismatch after parsing root node properties.";
        // return false;
        stream->device()->seek(m_tree.getCurrentNodeDataEndPos()); // Force alignment
    }
    return true;
}

bool OtbReader::parseItemNode(ServerItem& item, QString& errorString) {
    QDataStream* stream = m_tree.stream();
    if (!stream || m_nodeStack.isEmpty()) {
        errorString = "Internal Error: Stream not available or no current node in parseItemNode.";
        return false;
    }

    ServerItemGroup group = static_cast<ServerItemGroup>(m_tree.getCurrentNodeType());
    item.type = static_cast<ServerItemType>(group);

    // Item flags are the first part of the properties payload.
    item.flags = m_tree.readValue<quint32>();

    while (stream->device()->pos() < m_tree.getCurrentNodeDataEndPos() && !stream->atEnd()) {
        quint8 attributeByte;
        *stream >> attributeByte;

        if (attributeByte == NODE_END || attributeByte == NODE_START || attributeByte == ESCAPE_CHAR) {
             errorString = QString("OTB Error: Unexpected control character (0x%1) where attribute was expected for item ID %2 (current type %3). Stream pos: %4, PropsEnd: %5")
                           .arg(QString::number(attributeByte, 16))
                           .arg(item.id) // item.id might not be set yet
                           .arg(m_tree.getCurrentNodeType())
                           .arg(stream->device()->pos()-1)
                           .arg(m_tree.getCurrentNodeDataEndPos());
             return false;
        }

        ServerItemAttribute attribute = static_cast<ServerItemAttribute>(attributeByte);
        quint16 dataLen;
        *stream >> dataLen;

        qint64 expectedPosAfterData = stream->device()->pos() + dataLen;
        if (expectedPosAfterData > m_tree.getCurrentNodeDataEndPos()) {
            errorString = QString("OTB Error: Attribute data length (%1 bytes) for item attribute %2 exceeds node property boundary. Item type: %3. Pos: %4, End: %5")
                          .arg(dataLen).arg(QString::number(static_cast<quint8>(attribute),16)).arg(m_tree.getCurrentNodeType())
                          .arg(stream->device()->pos()-3).arg(m_tree.getCurrentNodeDataEndPos());
            return false;
        }


        switch (attribute) {
            case ServerItemAttribute::ServerID:
                if (dataLen != 2) { errorString = "Invalid length for ServerID"; return false; }
                item.id = m_tree.readValue<quint16>();
                break;
            case ServerItemAttribute::ClientID:
                if (dataLen != 2) { errorString = "Invalid length for ClientID"; return false; }
                item.clientId = m_tree.readValue<quint16>();
                break;
            case ServerItemAttribute::Name:
                item.name = m_tree.readString(dataLen); // readString handles escaping via readBytes
                break;
            case ServerItemAttribute::GroundSpeed:
                if (dataLen != 2) { errorString = "Invalid length for GroundSpeed"; return false; }
                item.groundSpeed = m_tree.readValue<quint16>();
                break;
            case ServerItemAttribute::SpriteHash:
                if (dataLen != 16) { errorString = "Invalid length for SpriteHash"; return false; }
                item.spriteHash = m_tree.readBytes(dataLen); // readBytes handles escaping
                break;
            case ServerItemAttribute::MinimapColor:
                if (dataLen != 2) { errorString = "Invalid length for MinimapColor"; return false; }
                item.minimapColor = m_tree.readValue<quint16>();
                break;
            case ServerItemAttribute::MaxReadWriteChars:
                if (dataLen != 2) { errorString = "Invalid length for MaxReadWriteChars"; return false; }
                item.maxReadWriteChars = m_tree.readValue<quint16>();
                break;
            case ServerItemAttribute::MaxReadChars:
                if (dataLen != 2) { errorString = "Invalid length for MaxReadChars"; return false; }
                item.maxReadChars = m_tree.readValue<quint16>();
                break;
            case ServerItemAttribute::Light:
                if (dataLen != 4) { errorString = "Invalid length for Light"; return false; }
                item.lightLevel = m_tree.readValue<quint16>();
                item.lightColor = m_tree.readValue<quint16>();
                break;
            case ServerItemAttribute::StackOrder:
                if (dataLen != 1) { errorString = "Invalid length for StackOrder"; return false; }
                item.stackOrder = static_cast<TileStackOrder>(m_tree.readValue<quint8>());
                item.hasStackOrder = true;
                break;
            case ServerItemAttribute::TradeAs:
                if (dataLen != 2) { errorString = "Invalid length for TradeAs"; return false; }
                item.tradeAs = m_tree.readValue<quint16>();
                break;
            default:
                qWarning() << "OTB Warning: Unknown item attribute" << Qt::hex << static_cast<quint8>(attribute)
                           << "with length" << dataLen << "for item (current type" << m_tree.getCurrentNodeType() << "). Skipping.";
                if (!m_tree.skipBytes(dataLen)) {
                    errorString = QString("OTB Error: Failed to skip %1 bytes for unknown item attribute %2.").arg(dataLen).arg(QString::number(static_cast<quint8>(attribute), 16));
                    return false;
                }
                break;
        }
    }

    // Ensure stream is positioned exactly at propsDataEndPos for the item node
    if (stream->device()->pos() != m_tree.getCurrentNodeDataEndPos()) {
         qWarning() << "OTB Warning: Stream position" << stream->device()->pos()
                   << "does not match expected end of item properties" << m_tree.getCurrentNodeDataEndPos()
                   << "for item type" << m_tree.getCurrentNodeType() << ". Potential parsing error or extra data.";
        // Optionally, make this an error
        stream->device()->seek(m_tree.getCurrentNodeDataEndPos()); // Force alignment
    }


    if (item.spriteHash.isEmpty() && item.type != ServerItemType::Deprecated) {
        item.spriteHash.fill(0, 16);
    }

    item.updatePropertiesFromFlags();
    // No specific cross-check for StackOrder flag vs attribute here, matching C# for now.
    return true;
}

} // namespace OTB
