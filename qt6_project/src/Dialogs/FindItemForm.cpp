/**
 * Item Editor Qt6 - Find Item Dialog Implementation
 * Exact mirror of Legacy_App/csharp/Source/Dialogs/FindItemForm.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "FindItemForm.h"
#include "ui_FindItemForm.h"

namespace ItemEditor {

FindItemDialog::FindItemDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FindItemDialog)
{
    ui->setupUi(this);
    
    // Setup dialog properties - exact mirror of C# FindItemForm properties
    setModal(true);
    setFixedSize(350, 150);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    
    // Setup UI components
    setupUi();
    
    // Connect signals
    connect(ui->searchByIdRadio, &QRadioButton::toggled, this, &FindItemDialog::onSearchModeChanged);
    connect(ui->searchByNameRadio, &QRadioButton::toggled, this, &FindItemDialog::onSearchModeChanged);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &FindItemDialog::onOkClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &FindItemDialog::onCancelClicked);
    
    // Set initial state
    updateControlStates();
}

FindItemDialog::~FindItemDialog()
{
    delete ui;
}

void FindItemDialog::setupUi()
{
    // Set window icon
    setWindowIcon(QIcon(":/icons/find.png"));
    
    // Set default focus
    ui->itemIdSpinBox->setFocus();
    
    // Center the dialog on parent
    if (parentWidget()) {
        move(parentWidget()->geometry().center() - rect().center());
    }
}

bool FindItemDialog::searchById() const
{
    return ui->searchByIdRadio->isChecked();
}

quint16 FindItemDialog::itemId() const
{
    return static_cast<quint16>(ui->itemIdSpinBox->value());
}

QString FindItemDialog::itemName() const
{
    return ui->itemNameLineEdit->text().trimmed();
}

void FindItemDialog::onSearchModeChanged()
{
    updateControlStates();
}

void FindItemDialog::updateControlStates()
{
    // Enable/disable controls based on search mode - exact mirror of C# logic
    bool searchingById = ui->searchByIdRadio->isChecked();
    
    ui->itemIdSpinBox->setEnabled(searchingById);
    ui->itemNameLineEdit->setEnabled(!searchingById);
    
    // Set focus to the active control
    if (searchingById) {
        ui->itemIdSpinBox->setFocus();
        ui->itemIdSpinBox->selectAll();
    } else {
        ui->itemNameLineEdit->setFocus();
        ui->itemNameLineEdit->selectAll();
    }
}

void FindItemDialog::onOkClicked()
{
    // Validate input before accepting - exact mirror of C# validation
    if (searchById()) {
        // ID search - always valid since spinbox constrains the range
        accept();
    } else {
        // Name search - check if name is not empty
        if (itemName().isEmpty()) {
            ui->itemNameLineEdit->setFocus();
            return;
        }
        accept();
    }
}

void FindItemDialog::onCancelClicked()
{
    // Exact mirror of C# buttonCancel_Click event handler
    reject();
}

} // namespace ItemEditor