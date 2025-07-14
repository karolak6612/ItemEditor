/**
 * Item Editor Qt6 - Client Item View Implementation
 * Exact mirror of Legacy_App/csharp/Source/Controls/ClientItemView.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "ClientItemView.h"
#include "../PluginInterface/Item.h"

#include <QPainter>
#include <QPixmap>
#include <QPaintEvent>
#include <QSize>
#include <QRect>
#include <QPoint>
#include <cmath>

namespace ItemEditor {

ClientItemView::ClientItemView(QWidget *parent)
    : QWidget(parent)
    , m_item(nullptr)
    , m_destRect()
    , m_sourceRect()
    , m_cachedPixmap()
    , m_cacheValid(false)
    , m_lastSize()
{
    // Initialize rectangles - exact mirror of C# constructor
    m_destRect = QRect();
    m_sourceRect = QRect();
    
    // Set widget properties
    setMinimumSize(32, 32);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // Enable custom painting optimizations
    setAttribute(Qt::WA_OpaquePaintEvent, true);  // Optimize for opaque painting
    setAttribute(Qt::WA_NoSystemBackground, true); // We handle our own background
    setAttribute(Qt::WA_StaticContents, true);     // Content doesn't change often
    
    // Performance optimization: reduce update frequency
    setUpdatesEnabled(true);
}

void ClientItemView::setClientItem(ItemEditor::ClientItem* item)
{
    // Exact mirror of C# ClientItem property setter
    if (m_item != item) {
        m_item = item;
        m_cacheValid = false; // Invalidate cache when item changes
        
        // Enhanced sprite loading validation
        if (m_item) {
            // Validate sprite data before rendering
            if (!m_item->spriteList().isEmpty()) {
                // Force bitmap generation if needed
                QPixmap bitmap = m_item->getBitmap();
                if (bitmap.isNull()) {
                    // Try to generate bitmap manually
                    m_item->generateBitmap();
                    bitmap = m_item->getBitmap();
                }
                
                if (!bitmap.isNull()) {
                    qDebug() << "ClientItemView: Successfully loaded sprite for item" << m_item->id() 
                             << "with bitmap size" << bitmap.width() << "x" << bitmap.height();
                } else {
                    qDebug() << "ClientItemView: Warning - Failed to generate bitmap for item" << m_item->id();
                }
            } else {
                qDebug() << "ClientItemView: Warning - Item" << m_item->id() << "has no sprites";
            }
        }
        
        invalidateItem();
        emit clientItemChanged(item);
    }
}

QSize ClientItemView::sizeHint() const
{
    return QSize(64, 64);
}

QSize ClientItemView::minimumSizeHint() const
{
    return QSize(32, 32);
}

void ClientItemView::paintEvent(QPaintEvent *event)
{
    // Performance optimization: Use cached rendering when possible
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false); // Disable for performance
    
    // Check if we can use cached pixmap
    if (m_cacheValid && !m_cachedPixmap.isNull() && m_lastSize == size()) {
        painter.drawPixmap(0, 0, m_cachedPixmap);
        return;
    }
    
    // Create cache pixmap if needed
    if (m_cachedPixmap.size() != size()) {
        m_cachedPixmap = QPixmap(size());
        m_lastSize = size();
    }
    
    // Paint to cache
    m_cachedPixmap.fill(palette().color(QPalette::Base));
    QPainter cacheP(&m_cachedPixmap);
    cacheP.setRenderHint(QPainter::SmoothPixmapTransform, false);
    
    if (m_item != nullptr) {
        QPixmap bitmap = m_item->getBitmap();
        if (!bitmap.isNull() && bitmap.width() > 0 && bitmap.height() > 0) {
            // Calculate destination rectangle - exact mirror of C# calculation
            m_destRect.setX(qMax(0, static_cast<int>((width() - bitmap.width()) * 0.5)));
            m_destRect.setY(qMax(0, static_cast<int>((height() - bitmap.height()) * 0.5)));
            m_destRect.setWidth(qMin(width(), bitmap.width()));
            m_destRect.setHeight(qMin(height(), bitmap.height()));
            
            // Set source rectangle - exact mirror of C# source rect
            m_sourceRect.setWidth(bitmap.width());
            m_sourceRect.setHeight(bitmap.height());
            m_sourceRect.setX(0);
            m_sourceRect.setY(0);
            
            // Draw image with optimized settings
            cacheP.drawPixmap(m_destRect, bitmap, m_sourceRect);
        } else {
            // Fallback rendering for missing or corrupted sprites
            QRect fallbackRect = QRect(5, 5, width() - 10, height() - 10);
            
            // Draw subtle border for missing sprites
            cacheP.setPen(QPen(QColor(200, 200, 200), 1, Qt::DashLine));
            cacheP.drawRect(fallbackRect);
            
            // Draw item ID with subtle styling
            cacheP.setPen(QColor(128, 128, 128));
            cacheP.setFont(QFont("Arial", 8));
            QString idText = QString("ID: %1").arg(m_item->id());
            
            // Center the text
            QFontMetrics fm(cacheP.font());
            QRect textRect = fm.boundingRect(idText);
            QPoint textPos = fallbackRect.center() - textRect.center();
            
            cacheP.drawText(textPos, idText);
            
            // Add sprite status info
            QString statusText;
            if (m_item->spriteList().isEmpty()) {
                statusText = "No sprites";
            } else {
                statusText = QString("%1 sprites").arg(m_item->spriteList().size());
            }
            
            QRect statusRect = fm.boundingRect(statusText);
            QPoint statusPos = QPoint(fallbackRect.center().x() - statusRect.width() / 2, 
                                    textPos.y() + textRect.height() + 5);
            
            cacheP.setPen(QColor(100, 100, 100));
            cacheP.setFont(QFont("Arial", 7));
            cacheP.drawText(statusPos, statusText);
        }
    } else {
        // No item selected - draw empty state
        QRect emptyRect = QRect(10, 10, width() - 20, height() - 20);
        cacheP.setPen(QPen(QColor(180, 180, 180), 1, Qt::DotLine));
        cacheP.drawRect(emptyRect);
        
        cacheP.setPen(QColor(150, 150, 150));
        cacheP.setFont(QFont("Arial", 9));
        cacheP.drawText(emptyRect, Qt::AlignCenter, "No item selected");
    }
    
    cacheP.end();
    m_cacheValid = true;
    
    // Draw cached result
    painter.drawPixmap(0, 0, m_cachedPixmap);
}

void ClientItemView::updateRects()
{
    // Helper method to update rectangles when widget is resized
    if (m_item != nullptr) {
        QPixmap bitmap = m_item->getBitmap();
        if (!bitmap.isNull()) {
            // Recalculate destination rectangle
            m_destRect.setX(qMax(0, static_cast<int>((width() - bitmap.width()) * 0.5)));
            m_destRect.setY(qMax(0, static_cast<int>((height() - bitmap.height()) * 0.5)));
            m_destRect.setWidth(qMin(width(), bitmap.width()));
            m_destRect.setHeight(qMin(height(), bitmap.height()));
            
            // Update source rectangle
            m_sourceRect.setWidth(bitmap.width());
            m_sourceRect.setHeight(bitmap.height());
            m_sourceRect.setX(0);
            m_sourceRect.setY(0);
        }
    }
}

void ClientItemView::invalidateItem()
{
    // Exact mirror of C# Invalidate() call with performance optimization
    m_cacheValid = false; // Invalidate cache
    updateRects();
    update(); // Triggers a repaint
}

void ClientItemView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    
    // Invalidate cache on resize for proper scaling
    m_cacheValid = false;
    updateRects();
}

// Test support method implementations
bool ClientItemView::hasValidBitmap() const
{
    return m_item && m_item->isValid();
}

QPixmap ClientItemView::getCurrentSprite() const
{
    if (m_item && m_item->isValid()) {
        return m_item->getBitmap();
    }
    return QPixmap();
}

void ClientItemView::clear()
{
    setClientItem(nullptr);
    update();
}

} // namespace ItemEditor