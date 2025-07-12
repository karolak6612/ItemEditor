#pragma once

#include <QWidget>
#include <QRect>
#include <QPaintEvent>
#include "otb/item.h"

namespace UI {
namespace Widgets {

/**
 * @brief Widget for displaying client item sprites
 * 
 * Port of C# ClientItemView.cs - displays item sprites with proper centering
 * and scaling, matching the original ItemEditor functionality.
 */
class ClientItemView : public QWidget
{
    Q_OBJECT

public:
    explicit ClientItemView(QWidget *parent = nullptr);
    virtual ~ClientItemView();

    // Main interface - matches C# ClientItem property
    void setClientItem(const ItemEditor::ClientItem* item);
    const ItemEditor::ClientItem* getClientItem() const;
    
    void clear();
    
    // Qt widget overrides
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    const ItemEditor::ClientItem* m_clientItem;
    QRect m_destRect;    // Destination rectangle for drawing
    QRect m_sourceRect;  // Source rectangle from bitmap
};

} // namespace Widgets
} // namespace UI