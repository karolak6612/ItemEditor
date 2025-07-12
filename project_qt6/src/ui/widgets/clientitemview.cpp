#include "ui/widgets/clientitemview.h"
#include <QPainter>
#include <QStyleOption>
#include <QDebug>

namespace UI {
namespace Widgets {

ClientItemView::ClientItemView(QWidget *parent)
    : QWidget(parent)
    , m_clientItem(nullptr)
    , m_destRect()
    , m_sourceRect()
{
    setMinimumSize(32, 32);
    setMaximumSize(64, 64);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    // Enable custom painting
    setAttribute(Qt::WA_OpaquePaintEvent, false);
}

ClientItemView::~ClientItemView()
{
    // Qt handles cleanup automatically
}

void ClientItemView::setClientItem(const ItemEditor::ClientItem* item)
{
    if (m_clientItem != item) {
        m_clientItem = item;
        update(); // Schedule repaint
    }
}

const ItemEditor::ClientItem* ClientItemView::getClientItem() const
{
    return m_clientItem;
}

void ClientItemView::clear()
{
    setClientItem(nullptr);
}

void ClientItemView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // Fill background for stylesheet support
    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    
    if (m_clientItem != nullptr) {
        QImage bitmap = m_clientItem->GetBitmap();
        if (!bitmap.isNull()) {
            // Calculate centered position like C# version
            m_destRect.setX(qMax(0, (width() - bitmap.width()) / 2));
            m_destRect.setY(qMax(0, (height() - bitmap.height()) / 2));
            m_destRect.setWidth(qMin(width(), bitmap.width()));
            m_destRect.setHeight(qMin(height(), bitmap.height()));
            
            m_sourceRect.setWidth(bitmap.width());
            m_sourceRect.setHeight(bitmap.height());
            m_sourceRect.setX(0);
            m_sourceRect.setY(0);
            
            // Draw the sprite image
            painter.drawImage(m_destRect, bitmap, m_sourceRect);
        }
    }
}

QSize ClientItemView::sizeHint() const
{
    return QSize(32, 32);
}

} // namespace Widgets
} // namespace UI