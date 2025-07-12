#pragma once
#include <QDialog>
#include "otb/otbtypes.h"
namespace UI { namespace Dialogs {
class FindItemDialog : public QDialog {
    Q_OBJECT
public:
    explicit FindItemDialog(const OTB::ServerItemList& items, QWidget *parent = nullptr);
    quint16 getSelectedServerId() const { return 0; } // Placeholder
};
} }