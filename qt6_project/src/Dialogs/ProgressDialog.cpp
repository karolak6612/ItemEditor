#include "ProgressDialog.h"
#include "ui_ProgressDialog.h"

ProgressDialog::ProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);
    setControlBox(false);
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

void ProgressDialog::setProgress(int value)
{
    ui->bar->setValue(value);
}

void ProgressDialog::setLabelText(const QString &text)
{
    ui->progressLbl->setText(text);
}
