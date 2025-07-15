/**
 * Item Editor Qt6 - List Base Template Implementation
 * Explicit template instantiation and virtual method implementations
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "ListBase.h"
#include "ServerItem.h"
#include <QPainter>
#include <QScrollBar>
#include <QApplication>
#include <QStyle>

// Forward declaration for ItemEditor::ServerItem
namespace ItemEditor { class ServerItem; }

namespace ItemEditor {

// Template method implementations for ListBase<ItemEditor::ServerItem>

template<>
void ListBase<ItemEditor::ServerItem>::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    
    // Fill background
    painter.fillRect(rect(), palette().base());
    
    // Paint visible items
    QList<int> visibleItems = getIndexesInView();
    for (int index : visibleItems) {
        QRect itemRect = getItemRect(index);
        if (itemRect.intersects(event->rect())) {
            // Paint item background
            bool selected = m_selectedIndices.contains(index);
            if (selected) {
                painter.fillRect(itemRect, palette().highlight());
            }
            
            // Paint item border
            painter.setPen(palette().mid().color());
            painter.drawRect(itemRect);
        }
    }
    
    // Paint content using virtual method
    paintContent(painter);
}

template<>
void ListBase<ItemEditor::ServerItem>::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_mousePressed = true;
        m_lastMousePos = event->pos();
        
        int index = getItemAt(event->pos());
        if (index >= 0) {
            bool ctrlPressed = event->modifiers() & Qt::ControlModifier;
            bool shiftPressed = event->modifiers() & Qt::ShiftModifier;
            
            handleItemClick(index, ctrlPressed, shiftPressed);
            m_lastClickedIndex = index;
        } else {
            // Clicked on empty area
            if (!(event->modifiers() & Qt::ControlModifier)) {
                clearSelection();
            }
        }
    }
    
    QWidget::mousePressEvent(event);
}

template<>
void ListBase<ItemEditor::ServerItem>::updateContentSize()
{
    int itemCount = m_items.size();
    
    if (m_layout == ListBaseLayout::Vertical) {
        m_contentSize = QSize(width(), itemCount * m_itemSize);
    } else {
        m_contentSize = QSize(itemCount * m_itemSize, height());
    }
    
    updateScrollBars();
}

template<>
void ListBase<ItemEditor::ServerItem>::updateViewport()
{
    QRect clientRect = rect();
    
    // Account for scroll bars
    if (m_verticalScrollBar && m_verticalScrollBar->isVisible()) {
        clientRect.setWidth(clientRect.width() - m_verticalScrollBar->width());
    }
    
    if (m_horizontalScrollBar && m_horizontalScrollBar->isVisible()) {
        clientRect.setHeight(clientRect.height() - m_horizontalScrollBar->height());
    }
    
    m_viewport = clientRect;
}

template<>
int ListBase<ItemEditor::ServerItem>::getItemAt(const QPoint& point) const
{
    if (m_items.isEmpty() || m_itemSize <= 0) {
        return -1;
    }
    
    QPoint adjustedPoint = point;
    
    if (m_layout == ListBaseLayout::Vertical) {
        adjustedPoint.setY(adjustedPoint.y() + m_scrollOffset);
        int index = adjustedPoint.y() / m_itemSize;
        return (index >= 0 && index < m_items.size()) ? index : -1;
    } else {
        adjustedPoint.setX(adjustedPoint.x() + m_scrollOffset);
        int index = adjustedPoint.x() / m_itemSize;
        return (index >= 0 && index < m_items.size()) ? index : -1;
    }
}

template<>
QRect ListBase<ItemEditor::ServerItem>::getItemRect(int index) const
{
    if (index < 0 || index >= m_items.size() || m_itemSize <= 0) {
        return QRect();
    }
    
    if (m_layout == ListBaseLayout::Vertical) {
        int y = index * m_itemSize - m_scrollOffset;
        return QRect(0, y, m_viewport.width(), m_itemSize);
    } else {
        int x = index * m_itemSize - m_scrollOffset;
        return QRect(x, 0, m_itemSize, m_viewport.height());
    }
}

template<>
void ListBase<ItemEditor::ServerItem>::updateSelection(int index, bool ctrlPressed, bool shiftPressed)
{
    if (index < 0 || index >= m_items.size()) {
        return;
    }
    
    if (shiftPressed && m_multiSelect && m_lastClickedIndex >= 0) {
        // Range selection
        int start = qMin(index, m_lastClickedIndex);
        int end = qMax(index, m_lastClickedIndex);
        
        if (!ctrlPressed) {
            clearSelection();
        }
        
        for (int i = start; i <= end; ++i) {
            selectItem(i, true);
        }
    } else if (ctrlPressed && m_multiSelect) {
        // Toggle selection
        bool wasSelected = m_selectedIndices.contains(index);
        selectItem(index, !wasSelected);
    } else {
        // Single selection
        clearSelection();
        selectItem(index, true);
    }
}

template<>
void ListBase<ItemEditor::ServerItem>::handleItemClick(int index, bool ctrlPressed, bool shiftPressed)
{
    updateSelection(index, ctrlPressed, shiftPressed);
    ensureVisible(index);
}

// Private method implementations

template<>
void ListBase<ItemEditor::ServerItem>::setupScrollBars()
{
    // Vertical scroll bar
    m_verticalScrollBar = new QScrollBar(Qt::Vertical, this);
    m_verticalScrollBar->setVisible(false);
    connect(m_verticalScrollBar, &QScrollBar::valueChanged, 
            this, &ListBase<ItemEditor::ServerItem>::onScrollValueChanged);
    
    // Horizontal scroll bar
    m_horizontalScrollBar = new QScrollBar(Qt::Horizontal, this);
    m_horizontalScrollBar->setVisible(false);
    connect(m_horizontalScrollBar, &QScrollBar::valueChanged, 
            this, &ListBase<ItemEditor::ServerItem>::onScrollValueChanged);
}

template<>
void ListBase<ItemEditor::ServerItem>::updateScrollBars()
{
    updateScrollBarVisibility();
    
    if (m_layout == ListBaseLayout::Vertical) {
        if (m_verticalScrollBar->isVisible()) {
            int maxScroll = qMax(0, m_contentSize.height() - m_viewport.height());
            m_verticalScrollBar->setRange(0, maxScroll);
            m_verticalScrollBar->setPageStep(m_viewport.height());
            m_verticalScrollBar->setSingleStep(m_itemSize);
        }
    } else {
        if (m_horizontalScrollBar->isVisible()) {
            int maxScroll = qMax(0, m_contentSize.width() - m_viewport.width());
            m_horizontalScrollBar->setRange(0, maxScroll);
            m_horizontalScrollBar->setPageStep(m_viewport.width());
            m_horizontalScrollBar->setSingleStep(m_itemSize);
        }
    }
    
    // Position scroll bars
    if (m_verticalScrollBar->isVisible()) {
        m_verticalScrollBar->setGeometry(
            width() - m_verticalScrollBar->sizeHint().width(), 0,
            m_verticalScrollBar->sizeHint().width(), 
            height() - (m_horizontalScrollBar->isVisible() ? m_horizontalScrollBar->height() : 0)
        );
    }
    
    if (m_horizontalScrollBar->isVisible()) {
        m_horizontalScrollBar->setGeometry(
            0, height() - m_horizontalScrollBar->sizeHint().height(),
            width() - (m_verticalScrollBar->isVisible() ? m_verticalScrollBar->width() : 0),
            m_horizontalScrollBar->sizeHint().height()
        );
    }
}

template<>
void ListBase<ItemEditor::ServerItem>::updateScrollBarVisibility()
{
    bool needVertical = false;
    bool needHorizontal = false;
    
    if (m_layout == ListBaseLayout::Vertical) {
        needVertical = m_contentSize.height() > height();
    } else {
        needHorizontal = m_contentSize.width() > width();
    }
    
    m_verticalScrollBar->setVisible(needVertical);
    m_horizontalScrollBar->setVisible(needHorizontal);
    
    updateViewport();
}

template<>
void ListBase<ItemEditor::ServerItem>::scrollBy(int delta)
{
    if (m_layout == ListBaseLayout::Vertical) {
        int newValue = m_verticalScrollBar->value() + delta;
        m_verticalScrollBar->setValue(newValue);
    } else {
        int newValue = m_horizontalScrollBar->value() + delta;
        m_horizontalScrollBar->setValue(newValue);
    }
}

template<>
void ListBase<ItemEditor::ServerItem>::invalidate()
{
    if (!m_updating && !m_needsUpdate) {
        m_needsUpdate = true;
        m_updateTimer->start();
    }
}

// Slot implementations

template<>
void ListBase<ItemEditor::ServerItem>::onScrollValueChanged(int value)
{
    m_scrollOffset = value;
    update();
}

template<>
void ListBase<ItemEditor::ServerItem>::onUpdateTimer()
{
    if (m_needsUpdate) {
        m_needsUpdate = false;
        updateContentSize();
        updateScrollBars();
        update();
    }
}

// Additional event handler implementations

template<>
void ListBase<ItemEditor::ServerItem>::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mousePressed) {
        // Handle drag selection if needed
        // For now, just update the last mouse position
        m_lastMousePos = event->pos();
    }
    
    QWidget::mouseMoveEvent(event);
}

template<>
void ListBase<ItemEditor::ServerItem>::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_mousePressed = false;
    }
    
    QWidget::mouseReleaseEvent(event);
}

template<>
void ListBase<ItemEditor::ServerItem>::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Up:
            if (!m_selectedIndices.isEmpty()) {
                int currentIndex = *m_selectedIndices.begin();
                if (currentIndex > 0) {
                    bool ctrlPressed = event->modifiers() & Qt::ControlModifier;
                    bool shiftPressed = event->modifiers() & Qt::ShiftModifier;
                    handleItemClick(currentIndex - 1, ctrlPressed, shiftPressed);
                }
            }
            break;
            
        case Qt::Key_Down:
            if (!m_selectedIndices.isEmpty()) {
                // Find the maximum selected index
                int currentIndex = 0;
                for (int index : m_selectedIndices) {
                    if (index > currentIndex) {
                        currentIndex = index;
                    }
                }
                if (currentIndex < m_items.size() - 1) {
                    bool ctrlPressed = event->modifiers() & Qt::ControlModifier;
                    bool shiftPressed = event->modifiers() & Qt::ShiftModifier;
                    handleItemClick(currentIndex + 1, ctrlPressed, shiftPressed);
                }
            }
            break;
            
        case Qt::Key_A:
            if (event->modifiers() & Qt::ControlModifier) {
                selectAll();
            }
            break;
            
        default:
            QWidget::keyPressEvent(event);
            break;
    }
}

template<>
void ListBase<ItemEditor::ServerItem>::wheelEvent(QWheelEvent *event)
{
    int delta = event->angleDelta().y();
    int scrollAmount = delta / 8; // Convert from 1/8 degree to pixels
    
    scrollBy(-scrollAmount);
    
    event->accept();
}

template<>
void ListBase<ItemEditor::ServerItem>::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    
    updateViewport();
    updateScrollBars();
    
    // Trigger a repaint
    update();
}


} // namespace ItemEditor