#include "ui/dialogs/finditemdialog.h"
namespace UI { namespace Dialogs {
FindItemDialog::FindItemDialog(const OTB::ServerItemList& items, QWidget *parent) : QDialog(parent) {
    Q_UNUSED(items);
    setWindowTitle("Find Item"); resize(300, 200);
}
} }