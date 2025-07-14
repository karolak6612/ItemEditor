#include "BitmapLocker.h"
#include <QPainter>
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

BitmapLocker::BitmapLocker(QPixmap& pixmap, QObject* parent)
    : QObject(parent)
    , m_pixmap(pixmap)
    , m_locked(false)
{
}

BitmapLocker::~BitmapLocker()
{
    if (m_locked) {
        unlockBits();
    }
}

void BitmapLocker::lockBits()
{
    if (m_locked) {
        qWarning() << "BitmapLocker: Already locked";
        return;
    }

    // Convert pixmap to image for direct pixel access
    m_image = m_pixmap.toImage();
    
    // Ensure we have ARGB32 format for consistent pixel access
    if (m_image.format() != QImage::Format_ARGB32) {
        m_image = m_image.convertToFormat(QImage::Format_ARGB32);
    }
    
    m_locked = true;
}

void BitmapLocker::copyPixels(const QPixmap& source, int x, int y)
{
    if (!m_locked) {
        qWarning() << "BitmapLocker: Must call lockBits() first";
        return;
    }

    QImage sourceImage = source.toImage();
    if (sourceImage.format() != QImage::Format_ARGB32) {
        sourceImage = sourceImage.convertToFormat(QImage::Format_ARGB32);
    }

    for (int py = 0; py < sourceImage.height(); py++) {
        for (int px = 0; px < sourceImage.width(); px++) {
            QColor color = sourceImage.pixelColor(px, py);
            setPixel(px + x, py + y, color);
        }
    }
}

void BitmapLocker::copyPixels(const QPixmap& source, int rx, int ry, int rw, int rh, int px, int py)
{
    if (!m_locked) {
        qWarning() << "BitmapLocker: Must call lockBits() first";
        return;
    }

    QImage sourceImage = source.toImage();
    if (sourceImage.format() != QImage::Format_ARGB32) {
        sourceImage = sourceImage.convertToFormat(QImage::Format_ARGB32);
    }

    for (int y = 0; y < rh; y++) {
        for (int x = 0; x < rw; x++) {
            if (rx + x < sourceImage.width() && ry + y < sourceImage.height()) {
                QColor color = sourceImage.pixelColor(rx + x, ry + y);
                setPixel(px + x, py + y, color);
            }
        }
    }
}

void BitmapLocker::unlockBits()
{
    if (!m_locked) {
        qWarning() << "BitmapLocker: Not locked";
        return;
    }

    // Convert the modified image back to pixmap
    m_pixmap = QPixmap::fromImage(m_image);
    m_locked = false;
}

void BitmapLocker::setPixel(int x, int y, const QColor& color)
{
    if (!m_locked) {
        return;
    }

    if (x >= 0 && x < m_image.width() && y >= 0 && y < m_image.height()) {
        m_image.setPixelColor(x, y, color);
    }
}

} // namespace Utils
} // namespace OTLib