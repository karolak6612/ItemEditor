/**
 * Item Editor Qt6 - About Dialog Implementation
 * Exact mirror of Legacy_App/csharp/Source/Dialogs/AboutForm.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "AboutForm.h"
#include "ui_AboutForm.h"
#include "../Properties/version.h"

#include <QApplication>
#include <QPixmap>
#include <QIcon>
#include <QDesktopServices>
#include <QUrl>

namespace ItemEditor {

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    
    // Setup dialog properties - exact mirror of C# AboutForm properties
    setModal(true);
    setFixedSize(400, 300);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    
    // Setup UI components
    setupUi();
    updateVersionInfo();
    updateApplicationInfo();
    
    // Connect signals
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AboutDialog::onOkClicked);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::setupUi()
{
    // Set application icon - exact mirror of C# icon setup
    QPixmap appIcon = QIcon(":/icons/application.ico").pixmap(64, 64);
    ui->iconLabel->setPixmap(appIcon);
    
    // Set window icon
    setWindowIcon(QIcon(":/icons/about.png"));
    
    // Configure labels
    ui->titleLabel->setStyleSheet("font-weight: bold; font-size: 16pt;");
    ui->versionLabel->setStyleSheet("font-size: 10pt;");
    ui->copyrightLabel->setStyleSheet("font-size: 8pt;");
    ui->websiteLabel->setStyleSheet("font-size: 8pt;");
    
    // Enable word wrap for description
    ui->descriptionLabel->setWordWrap(true);
    
    // Center the dialog on parent
    if (parentWidget()) {
        move(parentWidget()->geometry().center() - rect().center());
    }
}

void AboutDialog::updateVersionInfo()
{
    // Update version information - exact mirror of C# version display
    QString versionText = QString("Version %1").arg(Version::getVersionString());
    ui->versionLabel->setText(versionText);
}

void AboutDialog::updateApplicationInfo()
{
    // Set application title
    ui->titleLabel->setText(QApplication::applicationName());
    
    // Set description - exact mirror of C# description
    QString description = tr("A tool for editing OTB (Open Tibia Binary) item databases.\n"
                            "Supports client versions 8.00 - 10.77.");
    ui->descriptionLabel->setText(description);
    
    // Set copyright information - exact mirror of C# copyright
    QString copyright = tr("Copyright © 2014-2019 OTTools\n"
                          "Licensed under MIT License");
    ui->copyrightLabel->setText(copyright);
    
    // Set website link - exact mirror of C# website link
    QString website = QString("<a href=\"https://github.com/ottools/ItemEditor\">"
                             "https://github.com/ottools/ItemEditor</a>");
    ui->websiteLabel->setText(website);
    ui->websiteLabel->setOpenExternalLinks(true);
}

void AboutDialog::onOkClicked()
{
    // Exact mirror of C# buttonOK_Click event handler
    accept();
}

} // namespace ItemEditor