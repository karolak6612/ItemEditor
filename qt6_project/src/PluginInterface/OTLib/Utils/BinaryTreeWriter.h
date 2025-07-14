#pragma once

/**
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <QObject>
#include <QDataStream>
#include <QFile>
#include <QString>
#include <memory>
#include "SpecialChar.h"

// Forward declarations
namespace OTLib {
namespace Server {
namespace Items {
    enum class ServerItemAttribute : quint8;
}
}
}

namespace OTLib {
namespace Utils {

class BinaryTreeWriter : public QObject
{
    Q_OBJECT

public:
    explicit BinaryTreeWriter(const QString& path, QObject* parent = nullptr);
    ~BinaryTreeWriter();

    bool isDisposed() const { return m_disposed; }

    void createNode(quint8 type);
    void writeByte(quint8 value);
    void writeByte(quint8 value, bool unescape);
    void writeUInt16(quint16 value);
    void writeUInt16(quint16 value, bool unescape);
    void writeUInt32(quint32 value);
    void writeUInt32(quint32 value, bool unescape);
    void writeProp(OTLib::Server::Items::ServerItemAttribute attribute, QDataStream& writer);
    void writeProp(RootAttribute attribute, QDataStream& writer);
    void writeBytes(const QByteArray& bytes, bool unescape);
    void closeNode();

private:
    void writeProp(quint8 attr, const QByteArray& bytes);

    std::unique_ptr<QFile> m_file;
    std::unique_ptr<QDataStream> m_stream;
    bool m_disposed;
};

} // namespace Utils
} // namespace OTLib