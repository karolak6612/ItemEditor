/**
 * Item Editor Qt6 - List Base Header
 * Exact mirror of Legacy_App/csharp/Source/Controls/ListBase.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_LISTBASE_H
#define ITEMEDITOR_LISTBASE_H

#include <QWidget>
#include <QList>
#include <QSet>
#include <QSize>
#include <QRect>
#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QTimer>
#include <QPropertyAnimation>

namespace ItemEditor {

/**
 * List Base Layout Enumeration
 * Exact mirror of C# ListBaseLayout enum
 */
enum class ListBaseLayout
{
    Vertical,
    Horizontal
};

/**
 * List Base Template Class
 * Exact mirror of C# ListBase<T> abstract class
 * 
 * Base class for custom list controls with virtual scrolling
 * Provides common functionality for item-based list widgets
 */
template<typename T>
class ListBase : public QWidget
{
public:
    explicit ListBase(ListBaseLayout layout = ListBaseLayout::Vertical, QWidget *parent = nullptr);
    virtual ~ListBase() = default;

    // Properties - exact mirror of C# properties
    ListBaseLayout layout() const { return m_layout; }
    void setLayout(ListBaseLayout layout);
    
    int itemSize() const { return m_itemSize; }
    void setItemSize(int size);
    
    bool multiSelect() const { return m_multiSelect; }
    void setMultiSelect(bool multiSelect);
    
    QSize contentSize() const { return m_contentSize; }
    QRect viewport() const { return m_viewport; }
    
    // Item management - exact mirror of C# item methods
    QList<T*>& items() { return m_items; }
    const QList<T*>& items() const { return m_items; }
    
    QSet<int>& selectedIndices() { return m_selectedIndices; }
    const QSet<int>& selectedIndices() const { return m_selectedIndices; }
    
    void clearSelection();
    void selectItem(int index, bool selected = true);
    void selectAll();
    
    // Update methods - exact mirror of C# update methods
    void beginUpdate();
    void endUpdate();
    bool isUpdating() const { return m_updating; }
    
    // Virtual scrolling - exact mirror of C# scrolling methods
    QList<int> getIndexesInView() const;
    void scrollToItem(int index);
    void ensureVisible(int index);

protected:
    // Event handlers - exact mirror of C# event handling
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    
    // Virtual methods - exact mirror of C# abstract/virtual methods
    virtual void updateItemPosition(T* item, int index) = 0;
    virtual void paintContent(QPainter& painter) = 0;
    virtual void onSelectionChanged() {}
    
    // Helper methods
    virtual void updateContentSize();
    virtual void updateViewport();
    virtual int getItemAt(const QPoint& point) const;
    virtual QRect getItemRect(int index) const;
    
    // Selection methods
    virtual void updateSelection(int index, bool ctrlPressed, bool shiftPressed);
    virtual void handleItemClick(int index, bool ctrlPressed, bool shiftPressed);

private slots:
    void onScrollValueChanged(int value);
    void onUpdateTimer();

private:
    // Private fields - exact mirror of C# private fields
    ListBaseLayout m_layout;
    int m_itemSize;
    bool m_multiSelect;
    bool m_updating;
    
    QList<T*> m_items;
    QSet<int> m_selectedIndices;
    
    QSize m_contentSize;
    QRect m_viewport;
    
    // Scrolling
    QScrollBar* m_verticalScrollBar;
    QScrollBar* m_horizontalScrollBar;
    int m_scrollOffset;
    
    // Mouse interaction
    bool m_mousePressed;
    QPoint m_lastMousePos;
    int m_lastClickedIndex;
    
    // Update timer
    QTimer* m_updateTimer;
    bool m_needsUpdate;
    
    // Helper methods
    void setupScrollBars();
    void updateScrollBars();
    void updateScrollBarVisibility();
    void scrollBy(int delta);
    void invalidate();
};

// Template implementation
template<typename T>
ListBase<T>::ListBase(ListBaseLayout layout, QWidget *parent)
    : QWidget(parent)
    , m_layout(layout)
    , m_itemSize(32)
    , m_multiSelect(true)
    , m_updating(false)
    , m_scrollOffset(0)
    , m_mousePressed(false)
    , m_lastClickedIndex(-1)
    , m_needsUpdate(false)
{
    // Setup widget properties
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    
    // Setup scroll bars
    setupScrollBars();
    
    // Setup update timer
    m_updateTimer = new QTimer(this);
    m_updateTimer->setSingleShot(true);
    m_updateTimer->setInterval(16); // ~60 FPS
    connect(m_updateTimer, &QTimer::timeout, this, &ListBase<T>::onUpdateTimer);
    
    // Initial setup
    updateContentSize();
    updateViewport();
}

template<typename T>
void ListBase<T>::setLayout(ListBaseLayout layout)
{
    if (m_layout != layout) {
        m_layout = layout;
        updateContentSize();
        updateScrollBars();
        update();
    }
}

template<typename T>
void ListBase<T>::setItemSize(int size)
{
    if (m_itemSize != size && size > 0) {
        m_itemSize = size;
        updateContentSize();
        updateScrollBars();
        update();
    }
}

template<typename T>
void ListBase<T>::setMultiSelect(bool multiSelect)
{
    if (m_multiSelect != multiSelect) {
        m_multiSelect = multiSelect;
        if (!multiSelect && m_selectedIndices.size() > 1) {
            // Keep only the first selected item
            auto it = m_selectedIndices.begin();
            int firstSelected = *it;
            clearSelection();
            selectItem(firstSelected);
        }
    }
}

template<typename T>
void ListBase<T>::clearSelection()
{
    if (!m_selectedIndices.isEmpty()) {
        m_selectedIndices.clear();
        update();
        onSelectionChanged();
    }
}

template<typename T>
void ListBase<T>::selectItem(int index, bool selected)
{
    if (index >= 0 && index < m_items.size()) {
        bool wasSelected = m_selectedIndices.contains(index);
        
        if (selected && !wasSelected) {
            if (!m_multiSelect) {
                clearSelection();
            }
            m_selectedIndices.insert(index);
            update();
            onSelectionChanged();
        } else if (!selected && wasSelected) {
            m_selectedIndices.remove(index);
            update();
            onSelectionChanged();
        }
    }
}

template<typename T>
void ListBase<T>::selectAll()
{
    if (m_multiSelect && !m_items.isEmpty()) {
        m_selectedIndices.clear();
        for (int i = 0; i < m_items.size(); ++i) {
            m_selectedIndices.insert(i);
        }
        update();
        onSelectionChanged();
    }
}

template<typename T>
void ListBase<T>::beginUpdate()
{
    m_updating = true;
}

template<typename T>
void ListBase<T>::endUpdate()
{
    if (m_updating) {
        m_updating = false;
        updateContentSize();
        updateScrollBars();
        update();
    }
}

template<typename T>
QList<int> ListBase<T>::getIndexesInView() const
{
    QList<int> result;
    
    if (m_items.isEmpty() || m_itemSize <= 0) {
        return result;
    }
    
    int viewportStart, viewportEnd;
    
    if (m_layout == ListBaseLayout::Vertical) {
        viewportStart = m_scrollOffset;
        viewportEnd = m_scrollOffset + m_viewport.height();
        
        int firstIndex = qMax(0, viewportStart / m_itemSize);
        int lastIndex = qMin(m_items.size() - 1, viewportEnd / m_itemSize);
        
        for (int i = firstIndex; i <= lastIndex; ++i) {
            result.append(i);
        }
    } else {
        viewportStart = m_scrollOffset;
        viewportEnd = m_scrollOffset + m_viewport.width();
        
        int firstIndex = qMax(0, viewportStart / m_itemSize);
        int lastIndex = qMin(m_items.size() - 1, viewportEnd / m_itemSize);
        
        for (int i = firstIndex; i <= lastIndex; ++i) {
            result.append(i);
        }
    }
    
    return result;
}

template<typename T>
void ListBase<T>::scrollToItem(int index)
{
    if (index >= 0 && index < m_items.size()) {
        int itemPos = index * m_itemSize;
        
        if (m_layout == ListBaseLayout::Vertical) {
            if (itemPos < m_scrollOffset) {
                m_verticalScrollBar->setValue(itemPos);
            } else if (itemPos + m_itemSize > m_scrollOffset + m_viewport.height()) {
                m_verticalScrollBar->setValue(itemPos + m_itemSize - m_viewport.height());
            }
        } else {
            if (itemPos < m_scrollOffset) {
                m_horizontalScrollBar->setValue(itemPos);
            } else if (itemPos + m_itemSize > m_scrollOffset + m_viewport.width()) {
                m_horizontalScrollBar->setValue(itemPos + m_itemSize - m_viewport.width());
            }
        }
    }
}

template<typename T>
void ListBase<T>::ensureVisible(int index)
{
    scrollToItem(index);
}

} // namespace ItemEditor

#include "ListBase.cpp"

#endif // ITEMEDITOR_LISTBASE_H