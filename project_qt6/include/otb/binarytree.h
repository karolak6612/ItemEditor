#ifndef BINARYTREE_H
#define BINARYTREE_H

#include <QFile>
#include <QDataStream>
#include <QStack>
#include <QIODevice>
#include <QBuffer>

namespace OTB {

// Node types for parsing control flow, not actually written to file as such.
// OTB uses 0xFE and 0xFF for node start/end markers.
const quint8 NODE_START = 0xFE;
const quint8 NODE_END = 0xFF;
const quint8 ESCAPE_CHAR = 0xFD;


class BinaryTree {
public:
    BinaryTree();
    ~BinaryTree();

    bool open(const QString& filePath, QIODevice::OpenMode openMode);
    void close();

    // Reading methods (placeholder, to be implemented based on C# OtbReader and BinaryTreeReader)
    bool seekNodeStart();
    bool seekNodeEnd();
    bool enterNode(); // Moves stream to child node's content
    bool leaveNode(); // Moves stream past current node's end marker
    bool hasNextNode(); // Checks if there's a sibling node

    template<typename T>
    T readValue() {
        T value;
        if (m_stream) *m_stream >> value;
        return value;
    }
    QByteArray readBytes(qint64 count);
    QString readString(quint16 length); // OTB strings are not null-terminated, length is given

    // Writing methods (placeholder, to be implemented based on C# OtbWriter and BinaryTreeWriter)
    void writeNodeStart(quint8 nodeType); // nodeType is the first byte after 0xFE
    void writeNodeEnd();
    void writeValue(const QVariant& value); // Generic value writer
    void writeBytes(const QByteArray& bytes);
    void writeString(const QString& str, bool writeLengthPrefix = true); // OTB strings usually have length prefix

    // Generic property writing (from OtbWriter.cs)
    // Properties are: Attribute (1 byte), DataLength (2 bytes), Data (DataLength bytes)
    void writeProp(quint8 attribute, const QByteArray& data);

    QDataStream* stream() { return m_stream; }

    // Node traversal and info
    bool skipBytes(qint64 count); // Skips bytes, respecting escaped characters
    quint8 getCurrentNodeType() const; // Returns type of the current node from the stack
    
    // C# parity method - extracts current node's content into isolated buffer
    QByteArray extractNodeData() const;
    
    // Get isolated stream for current node (C# parity)
    QDataStream* getCurrentNodeStream() const;

    template<typename T>
    void writeValue(const T& value); // Writes a simple value, handling OTB escaping for its bytes


private:
    void writeByteEscaped(quint8 byte);
    quint8 readByteEscaped();

    QFile m_file;
    QDataStream* m_stream = nullptr;
    QIODevice::OpenMode m_openMode;

    struct NodeInfo {
        qint64 startPos;        // Position of the 0xFE marker in the file
        quint8 type;            // The type byte read after 0xFE
        QByteArray nodeData;    // Isolated node content (C# parity)
        QDataStream* nodeStream; // Stream for parsing isolated node data
        
        NodeInfo() : startPos(-1), type(0), nodeStream(nullptr) {}
        ~NodeInfo() { delete nodeStream; }
    };
    QStack<NodeInfo> m_nodeStack;
};

} // namespace OTB

#endif // BINARYTREE_H
