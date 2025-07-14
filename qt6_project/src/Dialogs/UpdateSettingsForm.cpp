/**
 * Item Editor Qt6 - Update Settings Dialog Implementation
 * Configuration dialog for update settings and preferences
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "UpdateSettingsForm.h"
#include "ui_UpdateSettingsForm.h"

#include <QSettings>
#include <QApplication>
#include <QMessageBox>

namespace ItemEditor {

UpdateSettingsDialog::UpdateSettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UpdateSettingsDialog)
    , autoCheckEnabled(true)
    , checkInterval(7)
    , updateChannel("stable")
    , notifyOnlyEnabled(false)
    , includeBetaEnabled(false)
{
    ui->setupUi(this);
    
    // Setup dialog properties
    setModal(true);
    setFixedSize(400, 300);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Update Settings"));
    
    // Setup UI components
    setupUi();
    loadSettings();
    
    // Connect signals
    connect(ui->okButton, &QPushButton::clicked, this, &UpdateSettingsDialog::onOkClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &UpdateSettingsDialog::onCancelClicked);
    connect(ui->resetButton, &QPushButton::clicked, this, &UpdateSettingsDialog::onResetClicked);
    connect(ui->autoCheckBox, &QCheckBox::toggled, this, &UpdateSettingsDialog::onAutoCheckToggled);
}

UpdateSettingsDialog::~UpdateSettingsDialog()
{
    delete ui;
}

void UpdateSettingsDialog::setupUi()
{
    // Configure auto-check settings
    ui->autoCheckBox->setText(tr("Automatically check for updates"));
    ui->autoCheckBox->setChecked(autoCheckEnabled);
    
    // Configure check interval
    ui->intervalLabel->setText(tr("Check every:"));
    ui->intervalSpinBox->setMinimum(1);
    ui->intervalSpinBox->setMaximum(30);
    ui->intervalSpinBox->setSuffix(tr(" days"));
    ui->intervalSpinBox->setValue(checkInterval);
    
    // Configure update channel
    ui->channelLabel->setText(tr("Update channel:"));
    ui->channelComboBox->addItem(tr("Stable"), "stable");
    ui->channelComboBox->addItem(tr("Beta"), "beta");
    ui->channelComboBox->addItem(tr("Development"), "development");
    
    // Set current channel
    int channelIndex = ui->channelComboBox->findData(updateChannel);
    if (channelIndex >= 0) {
        ui->channelComboBox->setCurrentIndex(channelIndex);
    }
    
    // Configure notification settings
    ui->notifyOnlyCheckBox->setText(tr("Only notify, don't download automatically"));
    ui->notifyOnlyCheckBox->setChecked(notifyOnlyEnabled);
    
    ui->includeBetaCheckBox->setText(tr("Include beta versions"));
    ui->includeBetaCheckBox->setChecked(includeBetaEnabled);
    
    // Configure buttons
    ui->okButton->setText(tr("OK"));
    ui->cancelButton->setText(tr("Cancel"));
    ui->resetButton->setText(tr("Reset to Defaults"));
    
    // Update control states
    updateControlStates();
    
    // Center the dialog on parent
    if (parentWidget()) {
        move(parentWidget()->geometry().center() - rect().center());
    }
}

void UpdateSettingsDialog::updateControlStates()
{
    bool autoEnabled = ui->autoCheckBox->isChecked();
    
    ui->intervalLabel->setEnabled(autoEnabled);
    ui->intervalSpinBox->setEnabled(autoEnabled);
    ui->channelLabel->setEnabled(autoEnabled);
    ui->channelComboBox->setEnabled(autoEnabled);
    ui->notifyOnlyCheckBox->setEnabled(autoEnabled);
    ui->includeBetaCheckBox->setEnabled(autoEnabled);
}

void UpdateSettingsDialog::loadSettings()
{
    QSettings settings;
    
    // Load update settings
    autoCheckEnabled = settings.value("Updates/AutoCheck", true).toBool();
    checkInterval = settings.value("Updates/CheckInterval", 7).toInt();
    updateChannel = settings.value("Updates/Channel", "stable").toString();
    notifyOnlyEnabled = settings.value("Updates/NotifyOnly", false).toBool();
    includeBetaEnabled = settings.value("Updates/IncludeBeta", false).toBool();
    
    // Apply to UI
    ui->autoCheckBox->setChecked(autoCheckEnabled);
    ui->intervalSpinBox->setValue(checkInterval);
    
    int channelIndex = ui->channelComboBox->findData(updateChannel);
    if (channelIndex >= 0) {
        ui->channelComboBox->setCurrentIndex(channelIndex);
    }
    
    ui->notifyOnlyCheckBox->setChecked(notifyOnlyEnabled);
    ui->includeBetaCheckBox->setChecked(includeBetaEnabled);
    
    updateControlStates();
}

void UpdateSettingsDialog::saveSettings()
{
    QSettings settings;
    
    // Save current UI values
    settings.setValue("Updates/AutoCheck", ui->autoCheckBox->isChecked());
    settings.setValue("Updates/CheckInterval", ui->intervalSpinBox->value());
    settings.setValue("Updates/Channel", ui->channelComboBox->currentData().toString());
    settings.setValue("Updates/NotifyOnly", ui->notifyOnlyCheckBox->isChecked());
    settings.setValue("Updates/IncludeBeta", ui->includeBetaCheckBox->isChecked());
    
    // Update internal state
    autoCheckEnabled = ui->autoCheckBox->isChecked();
    checkInterval = ui->intervalSpinBox->value();
    updateChannel = ui->channelComboBox->currentData().toString();
    notifyOnlyEnabled = ui->notifyOnlyCheckBox->isChecked();
    includeBetaEnabled = ui->includeBetaCheckBox->isChecked();
    
    emit settingsChanged();
}

void UpdateSettingsDialog::resetToDefaults()
{
    ui->autoCheckBox->setChecked(true);
    ui->intervalSpinBox->setValue(7);
    ui->channelComboBox->setCurrentIndex(0); // Stable
    ui->notifyOnlyCheckBox->setChecked(false);
    ui->includeBetaCheckBox->setChecked(false);
    
    updateControlStates();
}

// Getter methods
bool UpdateSettingsDialog::getAutoCheckEnabled() const
{
    return autoCheckEnabled;
}

int UpdateSettingsDialog::getCheckInterval() const
{
    return checkInterval;
}

QString UpdateSettingsDialog::getUpdateChannel() const
{
    return updateChannel;
}

bool UpdateSettingsDialog::getNotifyOnlyEnabled() const
{
    return notifyOnlyEnabled;
}

bool UpdateSettingsDialog::getIncludeBetaEnabled() const
{
    return includeBetaEnabled;
}

// Setter methods
void UpdateSettingsDialog::setAutoCheckEnabled(bool enabled)
{
    autoCheckEnabled = enabled;
    ui->autoCheckBox->setChecked(enabled);
    updateControlStates();
}

void UpdateSettingsDialog::setCheckInterval(int days)
{
    checkInterval = days;
    ui->intervalSpinBox->setValue(days);
}

void UpdateSettingsDialog::setUpdateChannel(const QString &channel)
{
    updateChannel = channel;
    int index = ui->channelComboBox->findData(channel);
    if (index >= 0) {
        ui->channelComboBox->setCurrentIndex(index);
    }
}

void UpdateSettingsDialog::setNotifyOnlyEnabled(bool enabled)
{
    notifyOnlyEnabled = enabled;
    ui->notifyOnlyCheckBox->setChecked(enabled);
}

void UpdateSettingsDialog::setIncludeBetaEnabled(bool enabled)
{
    includeBetaEnabled = enabled;
    ui->includeBetaCheckBox->setChecked(enabled);
}

// Slot implementations
void UpdateSettingsDialog::onOkClicked()
{
    saveSettings();
    accept();
}

void UpdateSettingsDialog::onCancelClicked()
{
    reject();
}

void UpdateSettingsDialog::onResetClicked()
{
    int result = QMessageBox::question(this, tr("Reset Settings"),
                                      tr("Are you sure you want to reset all update settings to their default values?"),
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        resetToDefaults();
    }
}

void UpdateSettingsDialog::onAutoCheckToggled(bool enabled)
{
    updateControlStates();
}

} // namespace ItemEditor