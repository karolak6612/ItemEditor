/**
 * Item Editor Qt6 - Items XML Reader Implementation
 * Exact mirror of Legacy_App/csharp/Source/PluginInterface/OTLib/Server/Items/ItemsXmlReader.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "ItemsXmlReader.h"
#include "ServerItem.h"
#include "../../Collections/ServerItemList.h"
#include <QDir>
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>
#include <QStandardPaths>

namespace OTLib {
namespace Server {
namespace Items {

ItemsXmlReader::ItemsXmlReader(QObject *parent)
    : QObject(parent)
{
}

bool ItemsXmlReader::read(const QString& directory, OTLib::Collections::ServerItemList* items)
{
    if (directory.isNull()) {
        qWarning() << "ItemsXmlReader::read: directory is null";
        return false;
    }

    QDir dir(directory);
    if (!dir.exists()) {
        qWarning() << "ItemsXmlReader::read: directory does not exist:" << directory;
        return false;
    }

    if (!items) {
        qWarning() << "ItemsXmlReader::read: items is null";
        return false;
    }

    QString filePath = dir.absoluteFilePath("items.xml");
    QFile file(filePath);
    
    if (!file.exists()) {
        qWarning() << "ItemsXmlReader::read: items.xml not found in directory:" << directory;
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "ItemsXmlReader::read: failed to open file:" << filePath;
        return false;
    }

    try {
        QXmlStreamReader reader(&file);
        
        // Find the root element
        while (!reader.atEnd() && !reader.hasError()) {
            QXmlStreamReader::TokenType token = reader.readNext();
            
            if (token == QXmlStreamReader::StartElement) {
                if (reader.name() == "items") {
                    // Process items
                    while (!reader.atEnd() && !reader.hasError()) {
                        token = reader.readNext();
                        
                        if (token == QXmlStreamReader::StartElement && reader.name() == "item") {
                            QXmlStreamAttributes attributes = reader.attributes();
                            
                            if (attributes.hasAttribute("id")) {
                                // Single item
                                bool ok;
                                quint16 id = attributes.value("id").toUShort(&ok);
                                if (ok) {
                                    ServerItem* item = nullptr;
                                    if (items->tryGetValue(id, item) && item) {
                                        parseItem(item, reader);
                                    }
                                }
                            } else if (attributes.hasAttribute("fromid") && attributes.hasAttribute("toid")) {
                                // Range of items
                                bool fromOk, toOk;
                                quint16 fromId = attributes.value("fromid").toUShort(&fromOk);
                                quint16 toId = attributes.value("toid").toUShort(&toOk);
                                
                                if (fromOk && toOk) {
                                    for (quint16 id = fromId; id <= toId; ++id) {
                                        ServerItem* item = nullptr;
                                        if (items->tryGetValue(id, item) && item) {
                                            parseItem(item, reader);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    break;
                }
            }
        }
        
        if (reader.hasError()) {
            qWarning() << "ItemsXmlReader::read: XML parsing error:" << reader.errorString();
            return false;
        }
        
    } catch (const std::exception& ex) {
        qWarning() << "ItemsXmlReader::read: Exception occurred:" << ex.what();
        return false;
    } catch (...) {
        qWarning() << "ItemsXmlReader::read: Unknown exception occurred";
        return false;
    }

    m_directory = directory;
    m_file = filePath;
    emit directoryChanged();
    emit fileChanged();

    return true;
}

bool ItemsXmlReader::parseItem(ServerItem* item, QXmlStreamReader& reader)
{
    if (!item) return false;
    
    QXmlStreamAttributes attributes = reader.attributes();
    
    if (attributes.hasAttribute("name")) {
        item->setNameXml(attributes.value("name").toString());
    } else {
        qWarning() << QString("The item %1 is unnamed.").arg(item->id());
    }

    return true;
}

} // namespace Items
} // namespace Server
} // namespace OTLib