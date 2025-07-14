#include "UpdateSettingsDialog.h"
#include "ui_UpdateSettingsDialog.h"

UpdateSettingsDialog::UpdateSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateSettingsDialog)
{
    ui->setupUi(this);
}

UpdateSettingsDialog::~UpdateSettingsDialog()
{
    delete ui;
}

bool UpdateSettingsDialog::reassignUnmatchedSprites() const
{
    return ui->reassignUnmatchedSpritesCheck->isChecked();
}

bool UpdateSettingsDialog::reloadItemAttributes() const
{
    return ui->reloadItemAttributesCheck->isChecked();
}

bool UpdateSettingsDialog::createNewItems() const
{
    return ui->createNewItemsCheck->isChecked();
}

bool UpdateSettingsDialog::generateSignature() const
{
    return ui->generateSignatureCheck->isChecked();
}

void UpdateSettingsDialog::on_closeBtn_clicked()
{
    setResult(QDialog::Accepted);
    close();
}
