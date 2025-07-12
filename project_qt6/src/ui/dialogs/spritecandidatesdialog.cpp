#include "ui/dialogs/spritecandidatesdialog.h"
namespace UI { namespace Dialogs {
SpriteCandidatesDialog::SpriteCandidatesDialog(const QList<const ItemEditor::ClientItem*>& candidates, QWidget *parent) : QDialog(parent) {
    Q_UNUSED(candidates);
    setWindowTitle("Sprite Candidates"); resize(400, 300);
}
} }