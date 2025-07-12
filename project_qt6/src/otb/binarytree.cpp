#include "binarytree.h"
#include <QIODevice>
#include <QDebug>

namespace OTB {

BinaryTree::BinaryTree() : m_stream(nullptr), m_openMode(QIODevice::NotOpen) {}

BinaryTree::~BinaryTree() {
    close();
}

bool BinaryTree::open(const QString& filePath, QIODevice::OpenMode openMode) {
    if (m_file.isOpen()) {
        close();
    }

    m_file.setFileName(filePath);
    m_openMode = openMode;

    if (!m_file.open(openMode)) {
        qWarning() << "BinaryTree: Could not open file" << filePath << ":" << m_file.errorString();
        return false;
    }

    if (m_stream) {
        delete m_stream;
        m_stream = nullptr; // prevent double deletion if new QDataStream fails
    }
    m_stream = new QDataStream(&m_file);
    m_stream->setByteOrder(QDataStream::LittleEndian);

    m_nodeStack.clear();

    return true;
}

void BinaryTree::close() {
    if (m_stream) {
        delete m_stream;
        m_stream = nullptr;
    }
    if (m_file.isOpen()) {
        m_file.close();
    }
    m_openMode = QIODevice::NotOpen;
    m_nodeStack.clear();
}

// Writes a byte, escaping NODE_START, NODE_END, ESCAPE_CHAR if necessary
void BinaryTree::writeByteEscaped(quint8 byte) {
    if (!m_stream || m_openMode == QIODevice::ReadOnly) return;

    if (byte == NODE_START || byte == NODE_END || byte == ESCAPE_CHAR) {
        *m_stream << ESCAPE_CHAR;
        *m_stream << byte;
    } else {
        *m_stream << byte;
    }
}

// Reads a byte, unescaping if ESCAPE_CHAR is encountered
quint8 BinaryTree::readByteEscaped() {
    if (!m_stream || m_openMode == QIODevice::WriteOnly || m_stream->atEnd()) {
        // Consider throwing an exception or returning an error code
        return 0;
    }

    quint8 byte;
    *m_stream >> byte;

    if (byte == ESCAPE_CHAR) {
        if (m_stream->atEnd()) {
            // Error: escape char at end of stream
            qWarning() << "BinaryTree: Escape character at end of stream.";
            return 0; // Or throw
        }
        *m_stream >> byte; // Read the actual escaped byte
    }
    return byte;
}


QByteArray BinaryTree::readBytes(qint64 count) {
    QByteArray bytes;
    if (!m_stream || m_openMode == QIODevice::WriteOnly) return bytes;

    bytes.resize(static_cast<int>(count));
    for (qint64 i = 0; i < count; ++i) {
        if (m_stream->atEnd()) {
            qWarning() << "BinaryTree::readBytes: Unexpected end of stream. Requested" << count << "got" << i;
            bytes.resize(static_cast<int>(i));
            break;
        }
        // OTB data bytes are escaped
        bytes[static_cast<int>(i)] = static_cast<char>(readByteEscaped());
    }
    return bytes;
}

QString BinaryTree::readString(quint16 length) {
    if (!m_stream || m_openMode == QIODevice::WriteOnly || length == 0) return QString();

    QByteArray stringBytes = readBytes(length);
    return QString::fromUtf8(stringBytes); // OTB strings are UTF-8 in C#
}

void BinaryTree::writeBytes(const QByteArray& bytes) {
    if (!m_stream || m_openMode == QIODevice::ReadOnly) return;

    for (char byte : bytes) {
        writeByteEscaped(static_cast<quint8>(byte));
    }
}

// Strings in OTB are typically: 2 bytes length, then N bytes (UTF8) data.
// The C# OtbWriter for Name attribute writes the string bytes directly, length is handled by writeProp.
// This method is more generic if needed elsewhere.
void BinaryTree::writeString(const QString& str, bool writeLengthPrefix) {
    if (!m_stream || m_openMode == QIODevice::ReadOnly) return;

    QByteArray utf8Bytes = str.toUtf8();
    if (writeLengthPrefix) {
        if (utf8Bytes.length() > 0xFFFF) {
             qWarning() << "BinaryTree::writeString: String too long for quint16 length prefix.";
             // Potentially truncate or throw
        }
        quint16 length = static_cast<quint16>(utf8Bytes.length());
        // Length itself is not escaped
        *m_stream << length;
    }
    writeBytes(utf8Bytes);
}


// Writes a property: Attribute ID (1 byte), Data Length (2 bytes), Data (N bytes)
// Data bytes are escaped. Attribute ID and Data Length are not.
void BinaryTree::writeProp(quint8 attribute, const QByteArray& data) {
    if (!m_stream || m_openMode == QIODevice::ReadOnly) return;

    if (data.length() > 0xFFFF) {
        qWarning() << "BinaryTree::writeProp: Data for attribute" << attribute << "is too long (" << data.length() << "bytes). Max is 65535.";
        // Handle error: throw, truncate, or skip
        return;
    }

    *m_stream << attribute;
    *m_stream << static_cast<quint16>(data.length());
    writeBytes(data);
}


bool BinaryTree::enterNode() {
    if (!m_stream || m_openMode == QIODevice::WriteOnly || m_stream->atEnd()) return false;

    quint8 marker;
    qint64 nodeMarkerPos = m_file.pos();
    *m_stream >> marker; // Read the marker directly, not escaped

    if (marker != NODE_START) {
        m_file.seek(nodeMarkerPos); // Rewind
        return false;
    }

    NodeInfo info;
    info.startPos = nodeMarkerPos;
    *m_stream >> info.type; // Read the node type (not escaped)
    
    // Find the end of this node by scanning for matching NODE_END
    qint64 dataStartPos = m_file.pos(); // Current position after reading type
    qint64 originalPos = m_file.pos();
    
    // Scan for matching NODE_END with proper depth tracking
    int depth = 0;
    qint64 nodeEndPos = -1;
    
    while (!m_stream->atEnd()) {
        qint64 currentBytePos = m_file.pos();
        quint8 byte;
        *m_stream >> byte;

        if (byte == ESCAPE_CHAR) {
            if (m_stream->atEnd()) break;
            *m_stream >> byte; // Skip the escaped byte - it's data, not a marker
        } else if (byte == NODE_START) {
            depth++;
        } else if (byte == NODE_END) {
            if (depth == 0) {
                nodeEndPos = currentBytePos;
                break;
            }
            depth--;
        }
    }
    
    if (nodeEndPos == -1) {
        qWarning() << "BinaryTree::enterNode: Could not find end for node type" << info.type << "starting at" << info.startPos;
        m_file.seek(nodeMarkerPos); // Rewind past NODE_START
        return false;
    }
    
    // Extract node data into isolated buffer with proper unescaping
    m_file.seek(dataStartPos);
    qint64 dataLength = nodeEndPos - dataStartPos;
    
    if (dataLength > 0) {
        info.nodeData.clear();
        info.nodeData.reserve(static_cast<int>(dataLength));
        
        qint64 bytesProcessed = 0;
        while (bytesProcessed < dataLength && !m_stream->atEnd()) {
            qint64 currentPos = m_file.pos();
            if (currentPos >= nodeEndPos) break; // Don't read past node end
            
            quint8 byte;
            *m_stream >> byte;
            bytesProcessed++;
            
            if (byte == ESCAPE_CHAR && m_file.pos() < nodeEndPos) {
                // This is an escape character, read the next byte
                *m_stream >> byte;
                bytesProcessed++;
            }
            
            info.nodeData.append(static_cast<char>(byte));
        }
        
        // Create isolated stream for parsing
        QBuffer* buffer = new QBuffer(&info.nodeData);
        buffer->open(QIODevice::ReadOnly);
        info.nodeStream = new QDataStream(buffer);
        info.nodeStream->setByteOrder(QDataStream::LittleEndian);
    }

    // Position stream at end of node for next navigation
    m_file.seek(nodeEndPos + 1); // +1 to skip the NODE_END marker
    
    m_nodeStack.push(info);
    return true;
}

bool BinaryTree::leaveNode() {
    if (!m_stream || m_nodeStack.isEmpty()) return false;

    // Simply pop the node from stack - navigation is handled by hasNextNode()
    m_nodeStack.pop();
    return true;
}

// Checks if the next readable byte (non-escaped) is NODE_START
bool BinaryTree::hasNextNode() {
    if (!m_stream || m_stream->atEnd() || m_openMode == QIODevice::WriteOnly) return false;

    qint64 originalPos = m_file.pos();
    
    // Skip any remaining data until we find NODE_START or NODE_END or EOF
    while (!m_stream->atEnd()) {
        quint8 nextByte;
        *m_stream >> nextByte;

        if (nextByte == ESCAPE_CHAR) {
            // Skip the escaped byte
            if (!m_stream->atEnd()) {
                *m_stream >> nextByte; // Read and discard escaped byte
            }
        } else if (nextByte == NODE_START) {
            // Found next node
            m_file.seek(originalPos); // Rewind
            return true;
        } else if (nextByte == NODE_END) {
            // Found end marker, no more nodes at this level
            m_file.seek(originalPos); // Rewind
            return false;
        }
        // Continue scanning for markers
    }

    m_file.seek(originalPos); // Rewind
    return false;
}


quint8 BinaryTree::getCurrentNodeType() const {
    if (m_nodeStack.isEmpty()) return 0; // Or some error indicator
    return m_nodeStack.top().type;
}

// Get isolated stream for current node (C# parity)
QDataStream* BinaryTree::getCurrentNodeStream() const {
    if (m_nodeStack.isEmpty()) return nullptr;
    return m_nodeStack.top().nodeStream;
}

// C# parity method - extracts current node's content into isolated buffer
// With the new approach, data is already extracted in enterNode()
QByteArray BinaryTree::extractNodeData() const {
    if (m_nodeStack.isEmpty()) {
        return QByteArray();
    }
    
    return m_nodeStack.top().nodeData;
}

bool BinaryTree::skipBytes(qint64 count) {
    if (!m_stream || m_openMode == QIODevice::WriteOnly) return false;
    for (qint64 i = 0; i < count; ++i) {
        if (m_stream->atEnd()) {
            qWarning() << "BinaryTree::skipBytes: Unexpected EOF while skipping.";
            return false;
        }
        (void)readByteEscaped(); // Read and discard one (potentially escaped) byte
    }
    return true;
}


// Template implementation moved to header file to avoid duplicate definition

// Explicit instantiations for types used by OtbReader/Writer
template quint8 BinaryTree::readValue<quint8>();
template quint16 BinaryTree::readValue<quint16>();
template quint32 BinaryTree::readValue<quint32>();


template<typename T>
void BinaryTree::writeValue(const T& value) {
    if (!m_stream || m_openMode == QIODevice::ReadOnly) {
        qWarning() << "BinaryTree::writeValue called on non-writable stream.";
        return;
    }

    QByteArray byteArray;
    QDataStream ds(&byteArray, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian); // OTB is little endian
    ds << value;

    // Write byte-by-byte to handle escaping
    for (char byte : byteArray) {
        writeByteEscaped(static_cast<quint8>(byte));
    }
}

// Explicit instantiations for types used by OtbReader/Writer
template void BinaryTree::writeValue<quint8>(const quint8&);
template void BinaryTree::writeValue<quint16>(const quint16&);
template void BinaryTree::writeValue<quint32>(const quint32&);


void BinaryTree::writeNodeStart(quint8 nodeType) {
    if (!m_stream || m_openMode == QIODevice::ReadOnly) return;

    *m_stream << NODE_START; // Not escaped
    *m_stream << nodeType;   // Not escaped

    NodeInfo info;
    info.startPos = m_file.pos() - 2; // Position of NODE_START
    info.type = nodeType;
    // For writing, we don't need isolated streams
    m_nodeStack.push(info);
}

void BinaryTree::writeNodeEnd() {
    if (!m_stream || m_openMode == QIODevice::ReadOnly || m_nodeStack.isEmpty()) return;

    *m_stream << NODE_END; // Not escaped
    m_nodeStack.pop();
}


} // namespace OTB
