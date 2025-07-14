/**
 * Item Editor Qt6 - Preferences Dialog Header
 * Exact mirror of Legacy_App/csharp/Source/Dialogs/PreferencesForm.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_PREFERENCESFORM_H
#define ITEMEDITOR_PREFERENCESFORM_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class PreferencesDialog; }
QT_END_NAMESPACE

namespace ItemEditor {

// Forward declarations
class Plugin;
struct SupportedClient;

/**
 * Preferences Dialog Class
 * Exact mirror of C# PreferencesForm : DarkForm
 * 
 * Manages application settings including client directory, plugin selection, and feature flags
 */
class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();

    // Public properties - exact mirror of C# properties
    Plugin* getPlugin() const { return m_plugin; }
    const SupportedClient* getClient() const { return m_client; }

private slots:
    void onDirectoryPathChanged();
    void onBrowseClicked();
    void onConfirmClicked();
    void onCancelClicked();

private:
    void setupUi();
    void loadSettings();
    void saveSettings();
    void onSelectFiles(const QString& directory);
    quint32 getSignature(const QString& fileName);
    void clearSettings();
    void updateControlStates();

private:
    Ui::PreferencesDialog *ui;
    
    // Settings data - exact mirror of C# private properties
    Plugin* m_plugin;
    const SupportedClient* m_client;
    quint32 m_datSignature;
    quint32 m_sprSignature;
    
    // Settings object
    QSettings* m_settings;
};

// Alias for compatibility with legacy naming
using PreferencesForm = PreferencesDialog;

} // namespace ItemEditor

#endif // ITEMEDITOR_PREFERENCESFORM_H