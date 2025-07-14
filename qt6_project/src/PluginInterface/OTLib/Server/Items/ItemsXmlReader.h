/**
 * Item Editor Qt6 - Items XML Reader Header
 * Exact mirror of Legacy_App/csharp/Source/PluginInterface/OTLib/Server/Items/ItemsXmlReader.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef OTLIB_SERVER_ITEMS_ITEMSXMLREADER_H
#define OTLIB_SERVER_ITEMS_ITEMSXMLREADER_H

#include <QObject>
#include <QString>
#include <QDir>

// Forward declarations
namespace OTLib {
namespace Collections {
class ServerItemList;
}
namespace Server {
namespace Items {
class ServerItem;
}
}
}

class QXmlStreamReader;

namespace OTLib {
namespace Server {
namespace Items {

/**
 * Items XML Reader class
 * Exact mirror of C# ItemsXmlReader class
 * 
 * Reads items.xml files and populates ServerItemList with item data
 */
class ItemsXmlReader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString directory READ directory NOTIFY directoryChanged)
    Q_PROPERTY(QString file READ file NOTIFY fileChanged)

public:
    explicit ItemsXmlReader(QObject *parent = nullptr);
    virtual ~ItemsXmlReader() = default;

    // Properties
    QString directory() const { return m_directory; }
    QString file() const { return m_file; }

    // Methods - exact mirror of C# ItemsXmlReader methods
    bool read(const QString& directory, OTLib::Collections::ServerItemList* items);

signals:
    void directoryChanged();
    void fileChanged();

protected:
    virtual bool parseItem(ServerItem* item, QXmlStreamReader& reader);

private:
    QString m_directory;
    QString m_file;
};

} // namespace Items
} // namespace Server
} // namespace OTLib

#endif // OTLIB_SERVER_ITEMS_ITEMSXMLREADER_H