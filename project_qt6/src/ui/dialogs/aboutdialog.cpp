#include "ui/dialogs/aboutdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace UI {
namespace Dialogs {

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("About ItemEditor Qt6");
    setModal(true);
    resize(300, 200);
    
    auto layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("ItemEditor Qt6 - Base Classes Implementation"));
    layout->addWidget(new QLabel("Version 2.0.0"));
    
    auto okButton = new QPushButton("OK", this);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(okButton);
}

} // namespace Dialogs
} // namespace UI