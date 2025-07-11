#include "clientitemview.h"
#include "otb/item.h" // For OTB::ClientItem, OTB::Sprite

#include <QPainter>
#include <QDebug>

ClientItemView::ClientItemView(QWidget *parent)
    : QWidget(parent), m_clientItem(nullptr)/*, m_pixmapDirty(true)*/
{
    // Set a fixed size or rely on sizeHint for now
    // setFixedSize(OTB::Sprite::DefaultSize * 2, OTB::Sprite::DefaultSize * 2); // Example: 64x64
    // For a single 32x32 sprite, sizeHint will return that.
    // If items can be 2x2 sprites (64x64), this needs to be dynamic.
    // Let's make it fixed for typical items for now, can be adjusted.
    setFixedSize(OTB::Sprite::DefaultSize, OTB::Sprite::DefaultSize);
    // generatePlaceholderPixmap(); // Initial empty state
}

void ClientItemView::setClientItem(const OTB::ClientItem *item)
{
    if (m_clientItem == item)
        return;

    m_clientItem = item;
    // m_pixmapDirty = true;
    update(); // Trigger a repaint
}

const OTB::ClientItem* ClientItemView::getClientItem() const
{
    return m_clientItem;
}

QSize ClientItemView::sizeHint() const
{
    if (m_clientItem && m_clientItem->width > 0 && m_clientItem->height > 0) {
        // Based on C# ClientItem.GetBitmap(), it renders WxH sprites onto a canvas.
        // For now, let's assume we display one "frame" which itself could be multiple sprites.
        // A 1x1 item is 32x32. A 2x2 item would be 64x64.
        return QSize(m_clientItem->width * OTB::Sprite::DefaultSize,
                     m_clientItem->height * OTB::Sprite::DefaultSize);
    }
    return QSize(OTB::Sprite::DefaultSize, OTB::Sprite::DefaultSize); // Default to 32x32
}

QSize ClientItemView::minimumSizeHint() const
{
    return sizeHint();
}

void ClientItemView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false); // Pixel art often looks better without AA

    // Background (similar to C# DarkUI background)
    painter.fillRect(rect(), QColor(0x3C, 0x3C, 0x3C)); // Dark gray

    if (!m_clientItem) {
        // Draw a border or nothing if no item
        painter.setPen(Qt::darkGray);
        painter.drawRect(rect().adjusted(0,0,-1,-1)); // Inner border
        return;
    }

    // Actual sprite rendering will go here later using m_clientItem->getBitmap()
    // For now, draw a placeholder based on ClientItem
    QImage placeholderImage = m_clientItem->getBitmap(); // Uses stub ClientItem::getBitmap()

    // Scale the image to fit the widget, preserving aspect ratio if necessary
    // For now, assuming 1:1 if widget is sized correctly by sizeHint
    // If widget size is fixed and different from image, scaling is needed.
    // QImage scaledImage = placeholderImage.scaled(this->size(), Qt::KeepAspectRatio, Qt::FastTransformation);
    // painter.drawImage(rect().topLeft(), scaledImage);

    // We'll draw the placeholderImage directly for now.
    // It might be smaller than the widget if the item is 1x1 and widget is set for 2x2.
    // Or, the widget size could be derived from clientItem->width/height.

    // Adjust widget fixed size based on item's dimensions for a more dynamic placeholder
    // This should ideally be handled by layout if setFixedSize is removed.
    // For now, let's assume the fixed size is 32x32 and we draw within that.
    // If placeholderImage is larger (e.g. from a 2x1 item), it will be clipped or should be scaled.
    // The stub ClientItem::getBitmap() returns a 32x32 image.

    int itemPixelWidth = m_clientItem->width * OTB::Sprite::DefaultSize;
    int itemPixelHeight = m_clientItem->height * OTB::Sprite::DefaultSize;

    // Scale the placeholder image if its intended size is different from the widget's fixed size
    // For simplicity, we'll draw the 32x32 placeholder image centered.
    // A better approach would be for ClientItem::getBitmap() to return an image of actual WxH dimensions.

    int x = (width() - placeholderImage.width()) / 2;
    int y = (height() - placeholderImage.height()) / 2;
    painter.drawImage(x, y, placeholderImage);

    // Add text if it's more than a single sprite (e.g. WxH > 1x1, or layers/frames > 1)
    bool isComplex = (m_clientItem->width > 1 ||
                      m_clientItem->height > 1 ||
                      m_clientItem->layers > 1 ||
                      m_clientItem->frames > 1);

    if (isComplex) {
        painter.setPen(Qt::white);
        QString complexText;
        if (m_clientItem->width > 1 || m_clientItem->height > 1) {
            complexText += QString("%1x%2").arg(m_clientItem->width).arg(m_clientItem->height);
        }
        if (m_clientItem->layers > 1) {
            complexText += (complexText.isEmpty() ? "" : " ") + QString("L%1").arg(m_clientItem->layers);
        }
        if (m_clientItem->frames > 1) {
            complexText += (complexText.isEmpty() ? "" : " ") + QString("F%1").arg(m_clientItem->frames);
        }

        if (!complexText.isEmpty()) {
            painter.drawText(rect().adjusted(2,2,-2,-2), Qt::AlignBottom | Qt::AlignRight | Qt::TextWordWrap, complexText);
        }
    }


    // Border
    painter.setPen(Qt::gray);
    painter.drawRect(rect().adjusted(0,0,-1,-1));
}

void ClientItemView::generatePlaceholderPixmap()
{
    // This would be used if we cached QPixmap
    // QPixmap newPixmap(sizeHint());
    // ... drawing logic ...
    // m_cachedPixmap = newPixmap;
    // m_pixmapDirty = false;
}
