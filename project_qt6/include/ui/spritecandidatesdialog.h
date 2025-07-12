#ifndef SPRITECANDIDATESDIALOG_H
#define SPRITECANDIDATESDIALOG_H

#include <QDialog>
#include <QList> // Forward declaration not enough for QList<ClientItemView*>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
QT_END_NAMESPACE

namespace OTB {
    struct ClientItem; // Forward declaration
}
class ClientItemView; // Forward declaration

class SpriteCandidatesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SpriteCandidatesDialog(const QList<const OTB::ClientItem*>& candidates, QWidget *parent = nullptr);
    ~SpriteCandidatesDialog();

    quint16 getSelectedClientId() const;

private slots:
    void onCandidateClicked(const OTB::ClientItem* candidateItem);

private:
    QList<ClientItemView*> m_candidateViews;
    QHBoxLayout* m_viewsLayout;
    QPushButton* m_cancelButton;

    quint16 m_selectedClientId;
};

#endif // SPRITECANDIDATESDIALOG_H
