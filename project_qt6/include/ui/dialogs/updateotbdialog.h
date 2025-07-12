#pragma once
#include <QDialog>
namespace UI { namespace Dialogs {
class UpdateOtbDialog : public QDialog {
    Q_OBJECT
public:
    explicit UpdateOtbDialog(int buildNumber, QWidget *parent = nullptr);
};
} }