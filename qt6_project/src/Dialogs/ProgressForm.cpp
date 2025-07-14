/**
 * Item Editor Qt6 - Progress Dialog Implementation
 * Progress indication dialog for long-running operations
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "ProgressForm.h"
#include "ui_ProgressForm.h"

#include <QApplication>

namespace ItemEditor {

ProgressDialog::ProgressDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProgressDialog)
    , isCanceled(false)
{
    ui->setupUi(this);
    
    // Setup dialog properties
    setModal(true);
    setFixedSize(400, 120);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Progress"));
    
    // Setup UI components
    setupUi();
    
    // Connect signals
    connect(ui->cancelButton, &QPushButton::clicked, this, &ProgressDialog::onCancelClicked);
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

void ProgressDialog::setupUi()
{
    // Configure progress bar
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(0);
    
    // Configure labels
    ui->statusLabel->setText(tr("Please wait..."));
    ui->statusLabel->setWordWrap(true);
    
    // Configure cancel button
    ui->cancelButton->setText(tr("Cancel"));
    ui->cancelButton->setEnabled(true);
    
    // Reset canceled state
    isCanceled = false;
    
    // Center the dialog on parent
    if (parentWidget()) {
        move(parentWidget()->geometry().center() - rect().center());
    }
}

void ProgressDialog::setProgress(int value)
{
    ui->progressBar->setValue(value);
    QApplication::processEvents(); // Allow UI updates during long operations
}

void ProgressDialog::setMaximum(int maximum)
{
    ui->progressBar->setMaximum(maximum);
}

void ProgressDialog::setMinimum(int minimum)
{
    ui->progressBar->setMinimum(minimum);
}

void ProgressDialog::setRange(int minimum, int maximum)
{
    ui->progressBar->setRange(minimum, maximum);
}

void ProgressDialog::setText(const QString &text)
{
    ui->statusLabel->setText(text);
    QApplication::processEvents(); // Allow UI updates
}

void ProgressDialog::setTitle(const QString &title)
{
    setWindowTitle(title);
}

void ProgressDialog::setCancelEnabled(bool enabled)
{
    ui->cancelButton->setEnabled(enabled);
    ui->cancelButton->setVisible(enabled);
}

void ProgressDialog::reset()
{
    ui->progressBar->setValue(ui->progressBar->minimum());
    ui->statusLabel->setText(tr("Please wait..."));
    isCanceled = false;
}

void ProgressDialog::onCancelClicked()
{
    isCanceled = true;
    emit cancelRequested();
    reject();
}

} // namespace ItemEditor