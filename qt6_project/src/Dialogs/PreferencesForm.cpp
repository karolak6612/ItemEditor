/**
 * Item Editor Qt6 - Preferences Dialog Implementation
 * Exact mirror of Legacy_App/csharp/Source/Dialogs/PreferencesForm.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "PreferencesForm.h"
#include "ui_PreferencesForm.h"
#include "../Host/Plugin.h"
#include "../Host/PluginServices.h"
#include "../Helpers/Utils.h"

#include <QApplication>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QDataStream>
#include <QStandardPaths>
#include <QMessageBox>

namespace ItemEditor {

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PreferencesDialog)
    , m_plugin(nullptr)
    , m_client(nullptr)
    , m_datSignature(0)
    , m_sprSignature(0)
    , m_settings(nullptr)
{
    ui->setupUi(this);
    
    // Setup dialog properties - exact mirror of C# PreferencesForm properties
    setModal(true);
    setFixedSize(500, 350);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    
    // Initialize settings
    m_settings = new QSettings(this);
    
    // Setup UI components
    setupUi();
    loadSettings();
    
    // Connect signals
    connect(ui->directoryPathLineEdit, &QLineEdit::textChanged, 
            this, &PreferencesDialog::onDirectoryPathChanged);
    connect(ui->browseButton, &QPushButton::clicked, 
            this, &PreferencesDialog::onBrowseClicked);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, 
            this, &PreferencesDialog::onConfirmClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, 
            this, &PreferencesDialog::onCancelClicked);
    
    // Initial directory validation
    onSelectFiles(ui->directoryPathLineEdit->text());
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::setupUi()
{
    // Set window icon
    setWindowIcon(QIcon(":/icons/preferences.png"));
    
    // Set initial focus
    ui->directoryPathLineEdit->setFocus();
    
    // Center the dialog on parent
    if (parentWidget()) {
        move(parentWidget()->geometry().center() - rect().center());
    }
}

void PreferencesDialog::loadSettings()
{
    // Load settings - exact mirror of C# PreferencesForm_Load
    ui->directoryPathLineEdit->setText(
        m_settings->value("ClientDirectory", QString()).toString());
    ui->extendedCheckBox->setChecked(
        m_settings->value("Extended", false).toBool());
    ui->frameDurationsCheckBox->setChecked(
        m_settings->value("FrameDurations", false).toBool());
    ui->transparencyCheckBox->setChecked(
        m_settings->value("Transparency", false).toBool());
    
    m_datSignature = m_settings->value("DatSignature", 0).toUInt();
    m_sprSignature = m_settings->value("SprSignature", 0).toUInt();
}

void PreferencesDialog::saveSettings()
{
    // Save settings - exact mirror of C# ConfirmButton_Click
    m_settings->setValue("ClientDirectory", ui->directoryPathLineEdit->text());
    m_settings->setValue("Extended", ui->extendedCheckBox->isChecked());
    m_settings->setValue("FrameDurations", ui->frameDurationsCheckBox->isChecked());
    m_settings->setValue("Transparency", ui->transparencyCheckBox->isChecked());
    m_settings->setValue("DatSignature", m_datSignature);
    m_settings->setValue("SprSignature", m_sprSignature);
    m_settings->sync();
}

void PreferencesDialog::onDirectoryPathChanged()
{
    // Exact mirror of C# DirectoryPathTextBox_TextChanged
    onSelectFiles(ui->directoryPathLineEdit->text());
}

void PreferencesDialog::onBrowseClicked()
{
    // Exact mirror of C# BrowseButton_Click
    QString directory = QFileDialog::getExistingDirectory(
        this,
        tr("Select Client Directory"),
        ui->directoryPathLineEdit->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if (!directory.isEmpty()) {
        ui->directoryPathLineEdit->setText(directory);
        onSelectFiles(directory);
    }
}

void PreferencesDialog::onSelectFiles(const QString& directory)
{
    // Clear alert message
    ui->alertLabel->clear();
    ui->alertLabel->setStyleSheet("");
    
    // Validate directory - exact mirror of C# OnSelectFiles
    if (directory.isEmpty() || !QDir(directory).exists()) {
        ui->alertLabel->setText(tr("Directory not found"));
        ui->alertLabel->setStyleSheet("color: red;");
        clearSettings();
        return;
    }
    
    // Find client files - exact mirror of C# Utils.FindClientFile calls
    QString datPath = Utils::findClientFile(directory, ".dat");
    QString sprPath = Utils::findClientFile(directory, ".spr");
    
    if (!QFile::exists(datPath) || !QFile::exists(sprPath)) {
        ui->alertLabel->setText(tr("Client files not found"));
        ui->alertLabel->setStyleSheet("color: red;");
        clearSettings();
        return;
    }
    
    // Get file signatures
    quint32 datSignature = getSignature(datPath);
    quint32 sprSignature = getSignature(sprPath);
    
    // Find compatible plugin - exact mirror of C# plugin finding logic
    PluginServices* pluginServices = PluginServices::getInstance();
    m_plugin = pluginServices->findPlugin(datSignature, sprSignature);
    
    if (!m_plugin) {
        ui->alertLabel->setText(tr("Unsupported version\nDat Signature: %1\nSpr Signature: %2")
                               .arg(datSignature, 0, 16)
                               .arg(sprSignature, 0, 16));
        ui->alertLabel->setStyleSheet("color: red;");
        clearSettings();
        return;
    }
    
    // Get client information
    m_client = m_plugin->getClientBySignatures(datSignature, sprSignature);
    if (!m_client) {
        ui->alertLabel->setText(tr("Client information not available"));
        ui->alertLabel->setStyleSheet("color: red;");
        clearSettings();
        return;
    }
    
    // Update control states based on client version - exact mirror of C# logic
    bool isVersion960OrHigher = m_client->version() >= 960;
    bool isVersion1050OrHigher = m_client->version() >= 1050;
    
    ui->extendedCheckBox->setChecked(ui->extendedCheckBox->isChecked() || isVersion960OrHigher);
    ui->extendedCheckBox->setEnabled(!isVersion960OrHigher);
    
    ui->frameDurationsCheckBox->setChecked(ui->frameDurationsCheckBox->isChecked() || isVersion1050OrHigher);
    ui->frameDurationsCheckBox->setEnabled(!isVersion1050OrHigher);
    
    // Store signatures
    m_datSignature = datSignature;
    m_sprSignature = sprSignature;
    
    // Show success message
    ui->alertLabel->setText(tr("Client files found and validated"));
    ui->alertLabel->setStyleSheet("color: green;");
}

quint32 PreferencesDialog::getSignature(const QString& fileName)
{
    // Exact mirror of C# GetSignature method
    quint32 signature = 0;
    QFile file(fileName);
    
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream >> signature;
        file.close();
    }
    
    return signature;
}

void PreferencesDialog::clearSettings()
{
    // Exact mirror of C# Clear method
    m_plugin = nullptr;
    m_client = nullptr;
    m_datSignature = 0;
    m_sprSignature = 0;
    
    // Reset checkboxes to default state
    ui->extendedCheckBox->setChecked(false);
    ui->extendedCheckBox->setEnabled(true);
    ui->frameDurationsCheckBox->setChecked(false);
    ui->frameDurationsCheckBox->setEnabled(true);
}

void PreferencesDialog::onConfirmClicked()
{
    // Validate that we have a valid plugin before accepting
    if (!m_plugin || !m_client) {
        QMessageBox::warning(this, tr("Invalid Configuration"), 
                           tr("Please select a valid client directory with supported files."));
        return;
    }
    
    // Save settings and accept - exact mirror of C# ConfirmButton_Click
    saveSettings();
    accept();
}

void PreferencesDialog::onCancelClicked()
{
    // Exact mirror of C# CancelButton_Click
    reject();
}

} // namespace ItemEditor