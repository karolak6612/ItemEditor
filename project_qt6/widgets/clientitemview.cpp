#include "clientitemview.h"
#include "otb/item.h" // For OTB::ClientItem, OTB::Sprite

#include <QPainter>
#include <QDebug>

ClientItemView::ClientItemView(QWidget *parent)
    : QWidget(parent), m_clientItem(nullptr)
{
    setFixedSize(OTB::Sprite::DefaultSize, OTB::Sprite::DefaultSize);
}

void ClientItemView::setClientItem(const OTB::ClientItem *item)
{
    if (m_clientItem == item) // Also handles both being nullptr
        return;

    m_clientItem = item;

    // Adjust fixed size if item has different dimensions (e.g. 2x2 sprites)
    // This is a simple way; a layout-based approach might be better for dynamic sizing.
    if (m_clientItem && (m_clientItem->width > 0 && m_clientItem->height > 0)) {
        setFixedSize(m_clientItem->width * OTB::Sprite::DefaultSize,
                     m_clientItem->height * OTB::Sprite::DefaultSize);
    } else {
        setFixedSize(OTB::Sprite::DefaultSize, OTB::Sprite::DefaultSize); // Default to 32x32 if no item or invalid dims
    }
    update(); // Trigger a repaint
}

const OTB::ClientItem* ClientItemView::getClientItem() const
{
    return m_clientItem;
}

QSize ClientItemView::sizeHint() const
{
    if (m_clientItem && m_clientItem->width > 0 && m_clientItem->height > 0) {
        return QSize(m_clientItem->width * OTB::Sprite::DefaultSize,
                     m_clientItem->height * OTB::Sprite::DefaultSize);
    }
    return QSize(OTB::Sprite::DefaultSize, OTB::Sprite::DefaultSize);
}

QSize ClientItemView::minimumSizeHint() const
{
    return sizeHint();
}

void ClientItemView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    painter.fillRect(rect(), QColor(0x3C, 0x3C, 0x3C)); // Dark gray background

    if (!m_clientItem) {
        painter.setPen(Qt::darkGray);
        painter.drawRect(rect().adjusted(0,0,-1,-1));
        return;
    }

    // ClientItem::getBitmap() should now return a potentially decompressed QImage
    // based on its spriteList and OTB::Sprite::getBitmap() logic.
    // The current stub for ClientItem::getBitmap() returns the first sprite's bitmap.
    QImage itemImage = m_clientItem->getBitmap();

    if (!itemImage.isNull()) {
        // If the itemImage is larger than the widget (e.g. a 2x2 item but widget is fixed at 32x32),
        // it will be clipped. If smaller, it will be centered.
        // For proper display of WxH items, the widget's size should match itemImage size.
        // The setFixedSize in setClientItem attempts to handle this.
        int x = (width() - itemImage.width()) / 2;
        int y = (height() - itemImage.height()) / 2;
        painter.drawImage(x, y, itemImage);
    } else {
        // Draw a fallback placeholder if itemImage is null (e.g. sprite data missing/error)
        painter.setPen(Qt::red);
        painter.drawText(rect(), Qt::AlignCenter, tr("No Sprite"));
    }

    // Border
    painter.setPen(Qt::gray);
    painter.drawRect(rect().adjusted(0,0,-1,-1));
}

void ClientItemView::generatePlaceholderPixmap()
{
    // This would be used if we cached QPixmap
}
