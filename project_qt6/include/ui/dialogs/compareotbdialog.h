#pragma once
#include <QDialog>
namespace UI { namespace Dialogs {
class CompareOtbDialog : public QDialog {
    Q_OBJECT
public:
    explicit CompareOtbDialog(QWidget *parent = nullptr);
};
} }