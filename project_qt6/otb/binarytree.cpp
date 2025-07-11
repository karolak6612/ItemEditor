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

// Finds the end of the current node (position of its NODE_END marker)
// This is crucial for knowing the boundary of the current node's properties and children.
// It respects escaped characters and nested structures.
qint64 BinaryTree::findNodeEnd(qint64 startOffset) {
    if (!m_stream || m_openMode == QIODevice::WriteOnly) return -1;

    qint64 originalPos = m_file.pos();
    if (!m_file.seek(startOffset)) {
        qWarning() << "BinaryTree::findNodeEnd: Failed to seek to startOffset" << startOffset;
        m_file.seek(originalPos);
        return -1;
    }

    int depth = 0;
    quint8 byte;
    while (!m_stream->atEnd()) {
        qint64 currentBytePos = m_file.pos();
        *m_stream >> byte;

        if (byte == ESCAPE_CHAR) {
            if (m_stream->atEnd()) { // Escape at EOF
                qWarning() << "BinaryTree::findNodeEnd: Escape character at EOF";
                m_file.seek(originalPos);
                return -1;
            }
            *m_stream >> byte; // Skip the escaped byte
        } else if (byte == NODE_START) {
            depth++;
        } else if (byte == NODE_END) {
            depth--;
            if (depth < 0) { // Found the matching NODE_END for the initial startOffset
                m_file.seek(originalPos); // Restore original position
                return currentBytePos; // Position of this NODE_END marker
            }
        }
    }

    qWarning() << "BinaryTree::findNodeEnd: Matching NODE_END not found for node starting at offset" << startOffset;
    m_file.seek(originalPos); // Restore original position
    return -1; // NODE_END not found
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
    info.propsDataStartPos = m_file.pos();

    // Determine where this node's properties end and children begin, or where the node itself ends.
    // This requires scanning ahead to find the first child NODE_START or this node's own NODE_END.
    info.nodeEndPos = findNodeEnd(info.startPos + 1 + 1); // +1 for NODE_START, +1 for type byte
    if (info.nodeEndPos == -1) {
        qWarning() << "BinaryTree::enterNode: Could not find end for node type" << info.type << "starting at" << info.startPos;
        m_file.seek(nodeMarkerPos); // Rewind past NODE_START
        return false;
    }

    // Scan between propsDataStartPos and nodeEndPos for the first unescaped NODE_START (child)
    // or assume properties extend until nodeEndPos if no child is found.
    qint64 scanPos = info.propsDataStartPos;
    qint64 firstChildStartPos = -1;

    qint64 originalScanStreamPos = m_file.pos(); // Save current real stream position
    m_file.seek(scanPos);

    while(m_file.pos() < info.nodeEndPos && !m_stream->atEnd()){
        qint64 currentMarkerCandidatePos = m_file.pos();
        quint8 currentByte;
        *m_stream >> currentByte; // Direct read, not escaped, to find markers

        if(currentByte == ESCAPE_CHAR){
            if(m_stream->atEnd()) break;
            *m_stream >> currentByte; // Skip escaped byte
        } else if (currentByte == NODE_START) {
            firstChildStartPos = currentMarkerCandidatePos;
            break;
        } else if (currentByte == NODE_END) {
            // This should ideally not happen before info.nodeEndPos if logic is correct
            // but if it does, it means no children.
            break;
        }
    }
    m_file.seek(originalScanStreamPos); // Restore real stream position

    if (firstChildStartPos != -1) {
        info.propsDataEndPos = firstChildStartPos;
        info.childrenStartPos = firstChildStartPos;
    } else {
        info.propsDataEndPos = info.nodeEndPos; // Properties go up to the node's own end marker
        info.childrenStartPos = info.nodeEndPos; // No children, children start effectively at node end
    }

    m_nodeStack.push(info);
    return true;
}

bool BinaryTree::leaveNode() {
    if (!m_stream || m_nodeStack.isEmpty()) return false;

    const NodeInfo& currentNode = m_nodeStack.top();
    if (!m_file.seek(currentNode.nodeEndPos)) { // Seek to where NODE_END should be
         qWarning() << "BinaryTree::leaveNode: Failed to seek to calculated node end position" << currentNode.nodeEndPos;
        return false;
    }

    quint8 marker;
    *m_stream >> marker; // Read marker directly (not escaped)

    if (marker != NODE_END) {
        // This indicates a structural error or miscalculation in findNodeEnd / propsDataEndPos
        qWarning() << "BinaryTree::leaveNode: Expected NODE_END (0xFF) at position"
                   << currentNode.nodeEndPos << "but found" << Qt::hex << marker;
        // Try to recover or error out. For now, let's assume it's an error.
        // We might be at an escaped 0xFF, or the structure is broken.
        // Rewind to before the marker read attempt.
        m_file.seek(currentNode.nodeEndPos);
        return false;
    }

    // Successfully read NODE_END, stream is now positioned after it.
    m_nodeStack.pop();
    return true;
}

// Checks if the next readable byte (non-escaped) is NODE_START
bool BinaryTree::hasNextNode() {
    if (!m_stream || m_stream->atEnd() || m_openMode == QIODevice::WriteOnly) return false;

    // Cannot be called if inside properties of a node, only when expecting next sibling or first child.
    if (!m_nodeStack.isEmpty()) {
        if (m_file.pos() < m_nodeStack.top().propsDataEndPos) {
             // Still within current node's properties, cannot have a "next node" here in that sense
            //  qWarning() << "BinaryTree::hasNextNode called while inside properties section.";
            return false;
        }
        // If at childrenStartPos, that's where the next node (first child) would be
        // If after a leaveNode, current pos is where next sibling would be.
    }


    quint8 nextByte;
    qint64 originalPos = m_file.pos();

    *m_stream >> nextByte; // Read directly, not escaped for marker check

    bool isNodeStart = (nextByte == NODE_START);

    if (!m_file.seek(originalPos)) { // Rewind
        qWarning() << "BinaryTree::hasNextNode: Failed to seek back to original position.";
        // This is a critical error for stream state
        return false;
    }
    return isNodeStart;
}


qint64 BinaryTree::getCurrentNodeDataEndPos() const {
    if (m_nodeStack.isEmpty()) return -1;
    return m_nodeStack.top().propsDataEndPos;
}

quint8 BinaryTree::getCurrentNodeType() const {
    if (m_nodeStack.isEmpty()) return 0; // Or some error indicator
    return m_nodeStack.top().type;
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


template<typename T>
T BinaryTree::readValue() {
    if (!m_stream || m_openMode == QIODevice::WriteOnly) {
        qWarning() << "BinaryTree::readValue called on non-readable stream.";
        return T();
    }
    if (sizeof(T) == 1) { // Single byte, can be read directly (after un-escaping)
        return static_cast<T>(readByteEscaped());
    } else {
        // Multi-byte type, read byte-by-byte due to escaping
        QByteArray rawBytes = readBytes(sizeof(T));
        if (rawBytes.size() != sizeof(T)) {
            qWarning() << "BinaryTree::readValue: Failed to read enough bytes for type."
                       << "Expected" << sizeof(T) << "got" << rawBytes.size();
            return T();
        }
        T value;
        QDataStream ds(rawBytes);
        ds.setByteOrder(QDataStream::LittleEndian); // Ensure correct byte order for reconstruction
        ds >> value;
        return value;
    }
}

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
    info.propsDataStartPos = m_file.pos();
    // endPos will be determined when writeNodeEnd is called, or by children.
    // For writing, we don't pre-calculate endPos like in reading.
    info.nodeEndPos = -1;
    info.propsDataEndPos = -1;
    info.childrenStartPos = -1;
    m_nodeStack.push(info);
}

void BinaryTree::writeNodeEnd() {
    if (!m_stream || m_openMode == QIODevice::ReadOnly || m_nodeStack.isEmpty()) return;

    *m_stream << NODE_END; // Not escaped
    m_nodeStack.pop();
}


} // namespace OTB
