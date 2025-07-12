#include "ui/dialogs/updateotbdialog.h"
namespace UI { namespace Dialogs {
UpdateOtbDialog::UpdateOtbDialog(int buildNumber, QWidget *parent) : QDialog(parent) {
    Q_UNUSED(buildNumber);
    setWindowTitle("Update OTB"); resize(400, 300);
}
} }