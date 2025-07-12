#pragma once
#include <QDialog>
namespace UI { namespace Dialogs {
class PreferencesDialog : public QDialog {
    Q_OBJECT
public:
    explicit PreferencesDialog(QWidget *parent = nullptr);
};
} }