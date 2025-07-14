/**
 * Item Editor Qt6 - OTB Writer Header
 * Full implementation with ServerItem integration
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_OTBWRITER_H
#define ITEMEDITOR_OTBWRITER_H

#include <QString>
#include <QObject>
#include <memory>
#include "OtbVersionInfo.h"

// Forward declarations
namespace OTLib {
namespace Collections { class ServerItemList; }
namespace Server { namespace Items { 
    class ServerItem; 
    enum class ServerItemGroup : quint8;
    enum class ServerItemFlag : quint32;
    enum class ServerItemAttribute : quint8;
    using ServerItemFlags = QFlags<ServerItemFlag>;
}}
namespace Utils { class BinaryTreeWriter; }
}

namespace OTLib {
namespace OTB {

/**
 * OTB Writer Class
 * Full implementation using existing ServerItem infrastructure
 */
class OtbWriter : public QObject
{
    Q_OBJECT

public:
    explicit OtbWriter(QObject *parent = nullptr);
    virtual ~OtbWriter() = default;

    // Writing methods
    bool write(const QString& filePath, OTLib::Collections::ServerItemList* items);
    bool write(const QString& filePath, OTLib::Collections::ServerItemList* items, const OtbVersionInfo& versionInfo);

    // Configuration
    void setVersionInfo(const OtbVersionInfo& versionInfo) { m_versionInfo = versionInfo; }
    OtbVersionInfo getVersionInfo() const { return m_versionInfo; }

    // Error handling
    QString getLastError() const { return m_lastError; }
    bool hasError() const { return !m_lastError.isEmpty(); }

signals:
    void progressChanged(int percentage);
    void statusChanged(const QString& status);

private:
    bool writeVersionHeader(OTLib::Utils::BinaryTreeWriter* writer, const OtbVersionInfo& versionInfo);
    bool writeItem(OTLib::Utils::BinaryTreeWriter* writer, OTLib::Server::Items::ServerItem* item);
    OTLib::Server::Items::ServerItemGroup getItemGroup(OTLib::Server::Items::ServerItem* item);
    OTLib::Server::Items::ServerItemFlags getItemFlags(OTLib::Server::Items::ServerItem* item);
    void writeItemAttributes(OTLib::Utils::BinaryTreeWriter* writer, OTLib::Server::Items::ServerItem* item);
    void setError(const QString& error);

private:
    OtbVersionInfo m_versionInfo;
    QString m_lastError;
};

} // namespace OTB
} // namespace OTLib

#endif // ITEMEDITOR_OTBWRITER_H