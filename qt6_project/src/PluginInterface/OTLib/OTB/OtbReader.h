/**
 * Item Editor Qt6 - OTB Reader Header
 * Full implementation with ServerItem integration
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef OTLIB_OTB_OTBREADER_H
#define OTLIB_OTB_OTBREADER_H

#include <QObject>
#include <QString>
#include <memory>

// Forward declarations
namespace OTLib {
namespace Collections { class ServerItemList; }
namespace Utils { class BinaryTreeReader; }
namespace Server { namespace Items { 
    class ServerItem; 
    enum class ServerItemGroup : quint8;
    enum class ServerItemType : quint8;
    enum class TileStackOrder : quint8;
    enum class ServerItemFlag : quint32;
    enum class ServerItemAttribute : quint8;
    using ServerItemFlags = QFlags<ServerItemFlag>;
}}
}

namespace OTLib {
namespace OTB {

/**
 * Root Attribute enumeration for OTB header
 */
enum class RootAttribute : quint8
{
    Version = 0x01
};

/**
 * OTB Reader class
 * Full implementation using existing ServerItem infrastructure
 */
class OtbReader : public QObject
{
    Q_OBJECT

public:
    explicit OtbReader(QObject *parent = nullptr);
    virtual ~OtbReader();
    
    // Main reading method
    bool read(const QString& filePath);
    
    // Access methods
    OTLib::Collections::ServerItemList* items() const { return m_items.get(); }
    int count() const;
    
    // Clear loaded data
    void clear();

private:
    // Private methods for reading OTB structure using BinaryTreeReader
    bool readVersionHeader(QDataStream* node);
    OTLib::Server::Items::ServerItem* readItem(QDataStream* node);
    void parseItemFlags(OTLib::Server::Items::ServerItemFlags flags, OTLib::Server::Items::ServerItem* item);
    void parseItemAttributes(QDataStream* node, OTLib::Server::Items::ServerItem* item);
    
    // Data members
    std::unique_ptr<OTLib::Collections::ServerItemList> m_items;
    QString m_filePath;
};

} // namespace OTB
} // namespace OTLib

#endif // OTLIB_OTB_OTBREADER_H