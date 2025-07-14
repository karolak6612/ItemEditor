#include "BinaryTreeWriter.h"
#include "../Server/Items/ServerItemAttribute.h"
#include <QIODevice>
#include <QDebug>
#include <QBuffer>

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

namespace OTLib {
namespace Utils {

BinaryTreeWriter::BinaryTreeWriter(const QString& path, QObject* parent)
    : QObject(parent)
    , m_disposed(false)
{
    if (path.isEmpty()) {
        qWarning() << "BinaryTreeWriter: path cannot be empty";
        m_disposed = true;
        return;
    }

    m_file = std::make_unique<QFile>(path);
    if (!m_file->open(QIODevice::WriteOnly)) {
        qWarning() << "BinaryTreeWriter: Failed to open file for writing:" << path;
        m_disposed = true;
        return;
    }

    m_stream = std::make_unique<QDataStream>(m_file.get());
    m_stream->setByteOrder(QDataStream::LittleEndian);
}

BinaryTreeWriter::~BinaryTreeWriter()
{
    if (m_file && m_file->isOpen()) {
        m_file->close();
    }
    m_disposed = true;
}

void BinaryTreeWriter::createNode(quint8 type)
{
    writeByte(static_cast<quint8>(SpecialChar::NodeStart), false);
    writeByte(type);
}

void BinaryTreeWriter::writeByte(quint8 value)
{
    writeBytes(QByteArray(1, static_cast<char>(value)), true);
}

void BinaryTreeWriter::writeByte(quint8 value, bool unescape)
{
    writeBytes(QByteArray(1, static_cast<char>(value)), unescape);
}

void BinaryTreeWriter::writeUInt16(quint16 value)
{
    writeUInt16(value, true);
}

void BinaryTreeWriter::writeUInt16(quint16 value, bool unescape)
{
    QByteArray bytes;
    QDataStream tempStream(&bytes, QIODevice::WriteOnly);
    tempStream.setByteOrder(QDataStream::LittleEndian);
    tempStream << value;
    writeBytes(bytes, unescape);
}

void BinaryTreeWriter::writeUInt32(quint32 value)
{
    writeUInt32(value, true);
}

void BinaryTreeWriter::writeUInt32(quint32 value, bool unescape)
{
    QByteArray bytes;
    QDataStream tempStream(&bytes, QIODevice::WriteOnly);
    tempStream.setByteOrder(QDataStream::LittleEndian);
    tempStream << value;
    writeBytes(bytes, unescape);
}

void BinaryTreeWriter::writeProp(OTLib::Server::Items::ServerItemAttribute attribute, QDataStream& writer)
{
    // Get the data from the writer's device
    QIODevice* device = writer.device();
    if (!device) {
        return;
    }

    qint64 currentPos = device->pos();
    device->seek(0);
    QByteArray bytes = device->readAll();
    device->seek(0);
    
    // Clear the device if it's a buffer
    if (auto buffer = qobject_cast<QBuffer*>(device)) {
        buffer->buffer().clear();
    }

    writeProp(static_cast<quint8>(attribute), bytes);
}

void BinaryTreeWriter::writeProp(RootAttribute attribute, QDataStream& writer)
{
    // Get the data from the writer's device
    QIODevice* device = writer.device();
    if (!device) {
        return;
    }

    qint64 currentPos = device->pos();
    device->seek(0);
    QByteArray bytes = device->readAll();
    device->seek(0);
    
    // Clear the device if it's a buffer
    if (auto buffer = qobject_cast<QBuffer*>(device)) {
        buffer->buffer().clear();
    }

    writeProp(static_cast<quint8>(attribute), bytes);
}

void BinaryTreeWriter::writeBytes(const QByteArray& bytes, bool unescape)
{
    if (m_disposed || !m_stream) {
        return;
    }

    for (char byte : bytes) {
        quint8 b = static_cast<quint8>(byte);
        if (unescape && (b == static_cast<quint8>(SpecialChar::NodeStart) || 
                        b == static_cast<quint8>(SpecialChar::NodeEnd) || 
                        b == static_cast<quint8>(SpecialChar::EscapeChar))) {
            *m_stream << static_cast<quint8>(SpecialChar::EscapeChar);
        }
        *m_stream << b;
    }
}

void BinaryTreeWriter::closeNode()
{
    writeByte(static_cast<quint8>(SpecialChar::NodeEnd), false);
}

void BinaryTreeWriter::writeProp(quint8 attr, const QByteArray& bytes)
{
    writeByte(attr);
    writeUInt16(static_cast<quint16>(bytes.length()));
    writeBytes(bytes, true);
}

} // namespace Utils
} // namespace OTLib