/**
 * Item Editor Qt6 - Update Settings Dialog Header
 * Configuration dialog for update settings and preferences
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_UPDATESETTINGSFORM_H
#define ITEMEDITOR_UPDATESETTINGSFORM_H

#include <QDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QGroupBox>

QT_BEGIN_NAMESPACE
namespace Ui { class UpdateSettingsDialog; }
QT_END_NAMESPACE

namespace ItemEditor {

/**
 * Update Settings Dialog Class
 * Manages update preferences and configuration
 * 
 * Allows users to configure automatic update checking, update channels, etc.
 */
class UpdateSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateSettingsDialog(QWidget *parent = nullptr);
    ~UpdateSettingsDialog();

    // Settings access methods
    bool getAutoCheckEnabled() const;
    void setAutoCheckEnabled(bool enabled);
    
    int getCheckInterval() const;
    void setCheckInterval(int days);
    
    QString getUpdateChannel() const;
    void setUpdateChannel(const QString &channel);
    
    bool getNotifyOnlyEnabled() const;
    void setNotifyOnlyEnabled(bool enabled);
    
    bool getIncludeBetaEnabled() const;
    void setIncludeBetaEnabled(bool enabled);

public slots:
    void loadSettings();
    void saveSettings();
    void resetToDefaults();

signals:
    void settingsChanged();

private slots:
    void onOkClicked();
    void onCancelClicked();
    void onResetClicked();
    void onAutoCheckToggled(bool enabled);

private:
    void setupUi();
    void updateControlStates();
    void applySettings();

private:
    Ui::UpdateSettingsDialog *ui;
    
    // Settings storage
    bool autoCheckEnabled;
    int checkInterval;
    QString updateChannel;
    bool notifyOnlyEnabled;
    bool includeBetaEnabled;
};

} // namespace ItemEditor

#endif // ITEMEDITOR_UPDATESETTINGSFORM_H