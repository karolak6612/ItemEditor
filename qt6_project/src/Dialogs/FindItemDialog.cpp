#include "FindItemDialog.h"
#include "ui_FindItemDialog.h"
#include "../MainForm.h"
#include <QMessageBox>

FindItemDialog::FindItemDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindItemDialog),
    m_mainForm(nullptr),
    m_properties(ServerItemFlag::None)
{
    ui->setupUi(this);
}

FindItemDialog::~FindItemDialog()
{
    delete ui;
}

void FindItemDialog::setMainForm(MainForm *mainForm)
{
    m_mainForm = mainForm;
    connect(m_mainForm, &MainForm::cleaned, this, &FindItemDialog::mainForm_CleanedHandler);
    //ui->serverItemList->setPlugin(m_mainForm->getCurrentPlugin()->getInstance());
    updateProperties();
}

void FindItemDialog::on_findBySidButton_toggled(bool checked)
{
    if (checked) {
        updateProperties();
    }
}

void FindItemDialog::on_findByCidButton_toggled(bool checked)
{
    if (checked) {
        updateProperties();
    }
}

void FindItemDialog::on_findByPropertiesButton_toggled(bool checked)
{
    if (checked) {
        updateProperties();
    }
}

void FindItemDialog::on_findItemButton_clicked()
{
    startFind();
}

void FindItemDialog::on_findIdNumericUpDown_editingFinished()
{
    startFind();
}

void FindItemDialog::on_serverItemList_itemClicked(QListWidgetItem *item)
{
    // TODO: Implement item selection
}

void FindItemDialog::on_propertyCheckBox_stateChanged(int arg1)
{
    // TODO: Implement property selection
}

void FindItemDialog::mainForm_CleanedHandler()
{
    ui->serverItemList->clear();
    updateProperties();
}

void FindItemDialog::updateProperties()
{
    if (ui->findBySidButton->isChecked())
    {
        ui->itemIdGroupBox->setEnabled(true);
        ui->itemIdGroupBox->setTitle("Server ID");
        ui->propertiesGroupBox->setEnabled(false);
        //ui->findIdNumericUpDown->setMaximum(m_mainForm->getMaxServerItemId());
    }
    else if (ui->findByCidButton->isChecked())
    {
        ui->itemIdGroupBox->setEnabled(true);
        ui->itemIdGroupBox->setTitle("Client ID");
        ui->propertiesGroupBox->setEnabled(false);
        //ui->findIdNumericUpDown->setMaximum(m_mainForm->getMaxClientItemId());
    }
    else if (ui->findByPropertiesButton->isChecked())
    {
        ui->itemIdGroupBox->setEnabled(false);
        ui->propertiesGroupBox->setEnabled(true);
    }
}

void FindItemDialog::startFind()
{
    ui->serverItemList->clear();
    ui->findItemButton->setEnabled(false);

    if (ui->findBySidButton->isChecked())
    {
        // TODO: Implement find by server id
    }
    else if (ui->findByCidButton->isChecked())
    {
        // TODO: Implement find by client id
    }
    else if (ui->findByPropertiesButton->isChecked())
    {
        // TODO: Implement find by properties
    }

    ui->findItemButton->setEnabled(true);
}
