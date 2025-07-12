#pragma once
#include <QDialog>
#include <QList>
#include "otb/item.h"
namespace UI { namespace Dialogs {
class SpriteCandidatesDialog : public QDialog {
    Q_OBJECT
public:
    explicit SpriteCandidatesDialog(const QList<const ItemEditor::ClientItem*>& candidates, QWidget *parent = nullptr);
};
} }