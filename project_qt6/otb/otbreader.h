#ifndef OTBRREADER_H
#define OTBRREADER_H

#include "otbtypes.h"
#include "binarytree.h"
#include <QString>
#include <QScopedPointer>

namespace OTB {

class OtbReader {
public:
    OtbReader();

    // Reads the OTB file at the given path and populates the ServerItemList.
    // Returns true on success, false on failure.
    // The 'items' pointer will be owned and managed by the caller.
    // If an error occurs, 'errorString' will contain a description.
    bool read(const QString& filePath, ServerItemList& items, QString& errorString);

private:
    bool parseRootNode(ServerItemList& items, QString& errorString);
    bool parseItemNode(ServerItem& item, QString& errorString);

    BinaryTree m_tree; // The binary tree utility for file parsing
};

} // namespace OTB

#endif // OTBRREADER_H
