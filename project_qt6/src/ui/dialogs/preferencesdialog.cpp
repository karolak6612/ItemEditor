#include "ui/dialogs/preferencesdialog.h"
namespace UI { namespace Dialogs {
PreferencesDialog::PreferencesDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Preferences"); resize(400, 300);
}
} }