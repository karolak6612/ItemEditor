#ifndef BINARYTREE_H
#define BINARYTREE_H

#include <QFile>
#include <QDataStream>
#include <QStack>
#include <QIODevice>

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
    bool enterNode(); // Reads 0xFE, then node_type. Pushes node onto stack. Returns false if not at NODE_START.
    bool leaveNode(); // Seeks past NODE_END. Pops node from stack. Returns false if not at NODE_END.
    bool hasNextNode(); // Checks if the next byte is NODE_START without consuming it.
    bool skipBytes(qint64 count); // Skips bytes, respecting escaped characters

    qint64 getCurrentNodeDataEndPos() const; // Returns end position of current node's data (before children or NODE_END)
    quint8 getCurrentNodeType() const; // Returns type of the current node from the stack

    // Writing methods
    void writeNodeStart(quint8 nodeType); // Writes 0xFE, then node_type. Pushes node onto stack for tracking.
    void writeNodeEnd();                  // Writes 0xFF. Pops node from stack.
    template<typename T>
    void writeValue(const T& value); // Writes a simple value, handling OTB escaping for its bytes


private:
    void writeByteEscaped(quint8 byte);
    quint8 readByteEscaped();
    qint64 findNodeEnd(qint64 startOffset); // Helper to find matching NODE_END for a NODE_START

    QFile m_file;
    QDataStream* m_stream = nullptr;
    QIODevice::OpenMode m_openMode;

    struct NodeInfo {
        qint64 startPos;        // Position of the 0xFE marker in the file
        qint64 propsDataStartPos; // Position after the node_type byte where properties start
        qint64 propsDataEndPos;   // Position after all properties of this node, before any children or its own NODE_END
                                // This is determined by finding the first child NODE_START or the parent's NODE_END
        qint64 childrenStartPos;  // Position of the first child's NODE_START, or node's own NODE_END if no children
        qint64 nodeEndPos;        // Position of this node's 0xFF marker in the file (calculated when entering)
        quint8 type;            // The type byte read after 0xFE
    };
    QStack<NodeInfo> m_nodeStack;
};

} // namespace OTB

#endif // BINARYTREE_H
