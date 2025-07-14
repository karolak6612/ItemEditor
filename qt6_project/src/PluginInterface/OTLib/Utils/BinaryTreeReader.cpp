#include "BinaryTreeReader.h"
#include <QIODevice>
#include <QDebug>

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

BinaryTreeReader::BinaryTreeReader(const QString& path, QObject* parent)
    : QObject(parent)
    , m_currentNodePosition(0)
    , m_currentNodeSize(0)
    , m_disposed(false)
{
    if (path.isEmpty()) {
        qWarning() << "BinaryTreeReader: path cannot be empty";
        m_disposed = true;
        return;
    }

    m_file = std::make_unique<QFile>(path);
    if (!m_file->open(QIODevice::ReadOnly)) {
        qWarning() << "BinaryTreeReader: Failed to open file:" << path;
        m_disposed = true;
        return;
    }

    m_stream = std::make_unique<QDataStream>(m_file.get());
    m_stream->setByteOrder(QDataStream::LittleEndian);
}

BinaryTreeReader::~BinaryTreeReader()
{
    if (m_file && m_file->isOpen()) {
        m_file->close();
    }
    m_disposed = true;
}

std::unique_ptr<QDataStream> BinaryTreeReader::getRootNode()
{
    return getChildNode();
}

std::unique_ptr<QDataStream> BinaryTreeReader::getChildNode()
{
    if (!advance()) {
        return nullptr;
    }
    return getNodeData();
}

std::unique_ptr<QDataStream> BinaryTreeReader::getNextNode()
{
    if (m_disposed || !m_stream) {
        return nullptr;
    }

    m_stream->device()->seek(m_currentNodePosition);

    quint8 value;
    *m_stream >> value;
    if (static_cast<SpecialChar>(value) != SpecialChar::NodeStart) {
        return nullptr;
    }

    *m_stream >> value;

    int level = 1;
    while (!m_stream->atEnd()) {
        *m_stream >> value;
        if (static_cast<SpecialChar>(value) == SpecialChar::NodeEnd) {
            --level;
            if (level == 0) {
                if (m_stream->atEnd()) {
                    return nullptr;
                }
                *m_stream >> value;
                if (static_cast<SpecialChar>(value) == SpecialChar::NodeEnd) {
                    return nullptr;
                } else if (static_cast<SpecialChar>(value) != SpecialChar::NodeStart) {
                    return nullptr;
                } else {
                    m_currentNodePosition = m_stream->device()->pos() - 1;
                    return getNodeData();
                }
            }
        } else if (static_cast<SpecialChar>(value) == SpecialChar::NodeStart) {
            ++level;
        } else if (static_cast<SpecialChar>(value) == SpecialChar::EscapeChar) {
            if (!m_stream->atEnd()) {
                *m_stream >> value; // Skip escaped character
            }
        }
    }

    return nullptr;
}

std::unique_ptr<QDataStream> BinaryTreeReader::getNodeData()
{
    if (m_disposed || !m_stream) {
        return nullptr;
    }

    m_stream->device()->seek(m_currentNodePosition);

    quint8 value;
    *m_stream >> value;

    if (static_cast<SpecialChar>(value) != SpecialChar::NodeStart) {
        return nullptr;
    }

    QByteArray nodeData;
    nodeData.reserve(200);

    m_currentNodeSize = 0;
    while (!m_stream->atEnd()) {
        *m_stream >> value;
        if (static_cast<SpecialChar>(value) == SpecialChar::NodeEnd || 
            static_cast<SpecialChar>(value) == SpecialChar::NodeStart) {
            break;
        } else if (static_cast<SpecialChar>(value) == SpecialChar::EscapeChar) {
            if (!m_stream->atEnd()) {
                *m_stream >> value;
            }
        }

        m_currentNodeSize++;
        nodeData.append(static_cast<char>(value));
    }

    m_stream->device()->seek(m_currentNodePosition);
    
    auto buffer = std::make_unique<QBuffer>();
    buffer->setData(nodeData);
    buffer->open(QIODevice::ReadOnly);
    
    auto dataStream = std::make_unique<QDataStream>(buffer.get());
    dataStream->setByteOrder(QDataStream::LittleEndian);
    
    // Transfer ownership of buffer to the stream
    buffer.release();
    
    return dataStream;
}

bool BinaryTreeReader::advance()
{
    if (m_disposed || !m_stream) {
        return false;
    }

    try {
        qint64 seekPos = 0;
        if (m_currentNodePosition == 0) {
            seekPos = 4;
        } else {
            seekPos = m_currentNodePosition;
        }

        m_stream->device()->seek(seekPos);

        quint8 value;
        *m_stream >> value;
        if (static_cast<SpecialChar>(value) != SpecialChar::NodeStart) {
            return false;
        }

        if (m_currentNodePosition == 0) {
            m_currentNodePosition = m_stream->device()->pos() - 1;
            return true;
        } else {
            *m_stream >> value;

            while (!m_stream->atEnd()) {
                *m_stream >> value;
                if (static_cast<SpecialChar>(value) == SpecialChar::NodeEnd) {
                    return false;
                } else if (static_cast<SpecialChar>(value) == SpecialChar::NodeStart) {
                    m_currentNodePosition = m_stream->device()->pos() - 1;
                    return true;
                } else if (static_cast<SpecialChar>(value) == SpecialChar::EscapeChar) {
                    if (!m_stream->atEnd()) {
                        *m_stream >> value; // Skip escaped character
                    }
                }
            }
        }
    } catch (...) {
        return false;
    }

    return false;
}

} // namespace Utils
} // namespace OTLib