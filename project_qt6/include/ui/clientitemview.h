#ifndef CLIENTITEMVIEW_H
#define CLIENTITEMVIEW_H

#include <QWidget>
#include <QPixmap> // For future sprite rendering

namespace ItemEditor {
    class ClientItem; // Forward declaration
    class Sprite;     // Forward declaration
}

class ClientItemView : public QWidget
{
    Q_OBJECT
public:
    explicit ClientItemView(QWidget *parent = nullptr);

    void setClientItem(const ItemEditor::ClientItem *item);
    const ItemEditor::ClientItem* getClientItem() const;

    // void setSprite(const OTB::Sprite* sprite); // Alternative if displaying single sprites

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    const ItemEditor::ClientItem *m_clientItem;
    // QPixmap m_cachedPixmap; // For caching the rendered sprite
    // bool m_pixmapDirty;

    void generatePlaceholderPixmap(); // For now
};

#endif // CLIENTITEMVIEW_H
