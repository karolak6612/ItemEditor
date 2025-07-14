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
#include <QPixmap>
#include <QImage>
#include <QColor>
#include <QRect>

namespace OTLib {
namespace Utils {

class BitmapLocker : public QObject
{
    Q_OBJECT

public:
    explicit BitmapLocker(QPixmap& pixmap, QObject* parent = nullptr);
    ~BitmapLocker();

    void lockBits();
    void copyPixels(const QPixmap& source, int x, int y);
    void copyPixels(const QPixmap& source, int rx, int ry, int rw, int rh, int px, int py);
    void unlockBits();

private:
    void setPixel(int x, int y, const QColor& color);

    QPixmap& m_pixmap;
    QImage m_image;
    bool m_locked;
};

} // namespace Utils
} // namespace OTLib