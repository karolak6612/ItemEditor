#include "otbwriter.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QBuffer> // For writing properties to a QByteArray first

namespace OTB {

OtbWriter::OtbWriter() {}

bool OtbWriter::write(const QString& filePath, const ServerItemList& itemsList, QString& errorString) {
    if (!m_tree.open(filePath, QIODevice::WriteOnly | QIODevice::Truncate)) {
        errorString = "Failed to open file for writing: " + filePath;
        return false;
    }

    QDataStream* stream = m_tree.stream();
    if (!stream) {
        errorString = "Failed to get data stream from BinaryTree for writing.";
        m_tree.close();
        return false;
    }

    // Write OTB Version (4 bytes, always 0)
    *stream << static_cast<quint32>(0);

    // Root Node
    m_tree.writeNodeStart(0); // Root node type is 0

    // Write Root Node Flags (4 bytes, unused)
    *stream << static_cast<quint32>(0);

    if (!writeRootNode(itemsList, errorString)) {
        m_tree.close(); // Also closes file
        return false;
    }

    // Write Item Nodes as children of the root node
    for (const ServerItem& item : itemsList.items) {
        ServerItemGroup group = static_cast<ServerItemGroup>(item.type);
        m_tree.writeNodeStart(static_cast<quint8>(group));

        // Write item flags
        // Ensure ServerItem::flags is up-to-date if boolean members were modified
        // For now, assuming item.flags is correct.
        // A call like `((ServerItem&)item).updateFlagsFromProperties();` might be needed if bools are primary.
        *stream << item.flags;


        if (!writeItemNode(item, errorString)) {
            m_tree.close();
            return false;
        }
        m_tree.writeNodeEnd(); // End of current item node
    }

    m_tree.writeNodeEnd(); // End of root node

    m_tree.close(); // Flushes and closes the file
    return true;
}

// Helper to write data to a QByteArray for m_tree.writeProp
template<typename T>
QByteArray packData(T val) {
    QByteArray data;
    QDataStream ds(&data, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian);
    ds << val;
    return data;
}

QByteArray packLightData(quint16 level, quint16 color) {
    QByteArray data;
    QDataStream ds(&data, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian);
    ds << level << color;
    return data;
}


bool OtbWriter::writeRootNode(const ServerItemList& itemsList, QString& errorString) {
    // Write Version Information Property
    OtbVersionInfo vi;
    vi.majorVersion = itemsList.majorVersion;
    vi.minorVersion = itemsList.minorVersion; // This is client version in C#
    vi.buildNumber = itemsList.buildNumber;

    // Construct CSDVersion string similar to C#
    // Example: "OTB 3.770.1-10.98" (Major.Client.Build-ClientMajor.ClientMinor)
    // itemsList.clientVersion should be like 1098 for 10.98
    // itemsList.minorVersion is the OTB representation of client (e.g. 770)
    QString csdFormat = QString("OTB %1.%2.%3-%4.%5")
        .arg(vi.majorVersion)
        .arg(vi.minorVersion) // OTB Client Version (e.g., 770)
        .arg(vi.buildNumber)
        .arg(itemsList.clientVersion / 100) // Actual Client Major (e.g., 10 from 1098)
        .arg(itemsList.clientVersion % 100); // Actual Client Minor (e.g., 98 from 1098)
    vi.csdVersion = csdFormat;


    QByteArray versionData;
    QDataStream ds(&versionData, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian);
    ds << vi.majorVersion << vi.minorVersion << vi.buildNumber;

    QByteArray csdBytes = vi.csdVersion.toLatin1(); // or toUtf8? C# uses Encoding.ASCII.GetBytes
    csdBytes.resize(128); // Pad with nulls to 128 bytes
    ds.writeRawData(csdBytes.constData(), 128);

    m_tree.writeProp(static_cast<quint8>(RootAttribute::Version), versionData);
    return true;
}

bool OtbWriter::writeItemNode(const ServerItem& item, QString& errorString) {
    // ServerID is always written
    m_tree.writeProp(static_cast<quint8>(ServerItemAttribute::ServerID), packData(item.id));

    if (item.type != ServerItemType::Deprecated) {
        m_tree.writeProp(static_cast<quint8>(ServerItemAttribute::ClientID), packData(item.clientId));

        if (item.spriteHash.size() == 16) { // Ensure hash is valid
            m_tree.writeProp(static_cast<quint8>(ServerItemAttribute::SpriteHash), item.spriteHash);
        } else {
            qWarning() << "Item ID" << item.id << "has invalid spriteHash size:" << item.spriteHash.size() << ". Writing 16 zero bytes.";
            m_tree.writeProp(static_cast<quint8>(ServerItemAttribute::SpriteHash), QByteArray(16,0));
        }

        if (!item.name.isEmpty()) {
            m_tree.writeProp(static_cast<quint8>(ServerItemAttribute::Name), item.name.toUtf8());
        }

        if (item.groundSpeed != 0 && item.type == ServerItemType::Ground) { // GroundSpeed only for ground items
             m_tree.writeProp(static_cast<quint8>(ServerItemAttribute::GroundSpeed), packData(item.groundSpeed));
        }

        if (item.minimapColor != 0) {
            m_tree.writeProp(static_cast<quint8>(ServerItemAttribute::MinimapColor), packData(item.minimapColor));
        }
        if (item.maxReadWriteChars != 0) {
            m_tree.writeProp(static_cast<quint8>(ServerItemAttribute::MaxReadWriteChars), packData(item.maxReadWriteChars));
        }
        if (item.maxReadChars != 0) {
            m_tree.writeProp(static_cast<quint8>(ServerItemAttribute::MaxReadChars), packData(item.maxReadChars));
        }
        if (item.lightLevel != 0 || item.lightColor != 0) {
            m_tree.writeProp(static_cast<quint8>(ServerItemAttribute::Light), packLightData(item.lightLevel, item.lightColor));
        }
        if (item.stackOrder != TileStackOrder::None) { // C# checks item.HasStackOrder which is true if stackOrder != None
            m_tree.writeProp(static_cast<quint8>(ServerItemAttribute::StackOrder), packData(static_cast<quint8>(item.stackOrder)));
        }
        if (item.tradeAs != 0) {
            m_tree.writeProp(static_cast<quint8>(ServerItemAttribute::TradeAs), packData(item.tradeAs));
        }
        // Other attributes from C# OtbWriter logic:
        // - Description (deprecated)
        // - Flags are written outside this function, directly after node start.
    }
    return true;
}

} // namespace OTB
