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
#include <QBuffer>
#include <QString>
#include <memory>
#include "SpecialChar.h"

namespace OTLib {
namespace Utils {

class BinaryTreeReader : public QObject
{
    Q_OBJECT

public:
    explicit BinaryTreeReader(const QString& path, QObject* parent = nullptr);
    ~BinaryTreeReader();

    bool isDisposed() const { return m_disposed; }
    
    std::unique_ptr<QDataStream> getRootNode();
    std::unique_ptr<QDataStream> getChildNode();
    std::unique_ptr<QDataStream> getNextNode();

private:
    std::unique_ptr<QDataStream> getNodeData();
    bool advance();

    std::unique_ptr<QFile> m_file;
    std::unique_ptr<QDataStream> m_stream;
    qint64 m_currentNodePosition;
    quint32 m_currentNodeSize;
    bool m_disposed;
};

} // namespace Utils
} // namespace OTLib