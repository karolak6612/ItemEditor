#ifndef OTBWRITER_H
#define OTBWRITER_H

#include "otbtypes.h"
#include "binarytree.h"
#include <QString>

namespace OTB {

class OtbWriter {
public:
    OtbWriter();

    // Writes the ServerItemList to an OTB file at the given path.
    // Returns true on success, false on failure.
    // 'errorString' will contain a description if an error occurs.
    bool write(const QString& filePath, const ServerItemList& itemsList, QString& errorString);

private:
    bool writeRootNode(const ServerItemList& itemsList, QString& errorString);
    bool writeItemNode(const ServerItem& item, QString& errorString);

    BinaryTree m_tree; // The binary tree utility for file writing
};

} // namespace OTB

#endif // OTBWRITER_H
