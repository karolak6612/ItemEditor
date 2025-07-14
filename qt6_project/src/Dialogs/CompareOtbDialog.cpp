#include "CompareOtbDialog.h"
#include "ui_CompareOtbDialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include "../PluginInterface/OTLib/OTB/OtbReader.h"
#include "../PluginInterface/OTLib/Server/Items/ServerItem.h"
#include "../Helpers/Utils.h"

CompareOtbDialog::CompareOtbDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CompareOtbDialog)
{
    ui->setupUi(this);
}

CompareOtbDialog::~CompareOtbDialog()
{
    delete ui;
}

void CompareOtbDialog::on_browseButton1_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open OTB File"), "", tr("OTB files (*.otb)"));
    if (!fileName.isEmpty()) {
        ui->file1Text->setText(fileName);
    }
}

void CompareOtbDialog::on_browseButton2_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open OTB File"), "", tr("OTB files (*.otb)"));
    if (!fileName.isEmpty()) {
        ui->file2Text->setText(fileName);
    }
}

void CompareOtbDialog::on_compareButton_clicked()
{
    ui->resultTextBox->clear();
    compareItems();
}

void CompareOtbDialog::on_file1Text_textChanged(const QString &arg1)
{
    ui->compareButton->setEnabled(!ui->file1Text->text().isEmpty() && !ui->file2Text->text().isEmpty());
}

void CompareOtbDialog::on_file2Text_textChanged(const QString &arg1)
{
    ui->compareButton->setEnabled(!ui->file1Text->text().isEmpty() && !ui->file2Text->text().isEmpty());
}

bool CompareOtbDialog::compareItems()
{
    OtbReader reader1;
    if (!reader1.read(ui->file1Text->text()))
    {
        QMessageBox::warning(this, "Error", QString("Could not open %1.").arg(ui->file1Text->text()));
        return false;
    }

    OtbReader reader2;
    if (!reader2.read(ui->file2Text->text()))
    {
        QMessageBox::warning(this, "Error", QString("Could not open %1.").arg(ui->file2Text->text()));
        return false;
    }

    if (reader1.getItems()->count() != reader2.getItems()->count())
    {
        ui->resultTextBox->appendPlainText(QString("Item count:  [ %1 / %2 ]").arg(reader1.getItems()->count()).arg(reader2.getItems()->count()));
    }

    for (int i = 0; i < reader1.getItems()->count(); ++i) {
        if (i >= reader2.getItems()->count()) {
            break;
        }

        ServerItem* item1 = reader1.getItems()->at(i);
        ServerItem* item2 = reader2.getItems()->at(i);

        if (item1->getClientId() != item2->getClientId())
        {
            ui->resultTextBox->appendPlainText(QString("ID: %1  -  Sprite changed  -  [ %2 / %3 ]").arg(item1->getID()).arg(item1->getClientId()).arg(item2->getClientId()));
            continue;
        }

        if (item1->getSpriteHash() != item2->getSpriteHash())
        {
            ui->resultTextBox->appendPlainText(QString("ID: %1  -  Sprite updated.").arg(item1->getID()));
        }

        // TODO: Compare other properties using reflection or a similar mechanism
    }


    if (ui->resultTextBox->toPlainText().isEmpty())
    {
        QMessageBox::information(this, "No Differences", "No differences found!");
    }

    return true;
}
