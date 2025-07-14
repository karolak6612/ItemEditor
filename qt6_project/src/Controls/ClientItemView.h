/**
 * Item Editor Qt6 - Client Item View Header
 * Exact mirror of Legacy_App/csharp/Source/Controls/ClientItemView.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_CLIENTITEMVIEW_H
#define ITEMEDITOR_CLIENTITEMVIEW_H

#include <QWidget>
#include <QRect>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>

// Forward declarations
namespace ItemEditor {
    class ClientItem;
}

// Qt MOC declaration for pointer types
Q_DECLARE_OPAQUE_POINTER(ItemEditor::ClientItem*)

namespace ItemEditor {

/**
 * Client Item View Widget
 * Exact mirror of C# ClientItemView : UserControl
 * 
 * Custom widget for displaying client item sprites with centered rendering
 * Handles custom painting similar to C# OnPaint override
 */
class ClientItemView : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(ItemEditor::ClientItem* clientItem READ clientItem WRITE setClientItem NOTIFY clientItemChanged)

public:
    explicit ClientItemView(QWidget *parent = nullptr);
    virtual ~ClientItemView() = default;

    // Properties - exact mirror of C# ClientItem property
    ItemEditor::ClientItem* clientItem() const { return m_item; }
    void setClientItem(ItemEditor::ClientItem* item);

    // Size hints for proper layout
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    
    // Test support methods
    bool hasValidBitmap() const;
    QPixmap getCurrentSprite() const;
    void clear();

signals:
    void clientItemChanged(ItemEditor::ClientItem* item);

protected:
    // Paint event - exact mirror of C# OnPaint(PaintEventArgs e)
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    // Private properties - exact mirror of C# private fields
    ItemEditor::ClientItem* m_item;
    QRect m_destRect;
    QRect m_sourceRect;
    
    // Performance optimization members
    QPixmap m_cachedPixmap;    // Cached rendered result
    bool m_cacheValid;         // Cache validity flag
    QSize m_lastSize;          // Last widget size for cache validation

    // Helper methods
    void updateRects();
    void invalidateItem();
};

} // namespace ItemEditor

#endif // ITEMEDITOR_CLIENTITEMVIEW_H