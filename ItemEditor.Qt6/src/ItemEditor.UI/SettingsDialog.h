#pragma once

#include <QDialog>
#include <QSettings>
#include <QListWidget>
#include <QStackedWidget>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>

/**
 * @brief Main settings and preferences dialog
 * 
 * Provides comprehensive application settings management with:
 * - Plugin configuration interface
 * - Import/export of settings functionality using QSettings
 * - Categorized settings pages
 * - Real-time preview of changes
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    // Settings management
    void loadSettings();
    void saveSettings();
    void resetToDefaults();
    
    // Import/Export functionality
    bool importSettings(const QString& filePath);
    bool exportSettings(const QString& filePath);
    
    // Show specific settings page
    void showGeneralSettings();
    void showPluginSettings();
    void showUISettings();
    void showFileSettings();
    void showAdvancedSettings();

public slots:
    void accept() override;
    void reject() override;

private slots:
    void onCategoryChanged();
    void onRestoreDefaultsClicked();
    void onImportSettingsClicked();
    void onExportSettingsClicked();
    void onSettingChanged();

signals:
    void settingsChanged();
    void pluginSettingsChanged();
    void uiSettingsChanged();

private:
    // UI components
    QSplitter* m_splitter;
    QListWidget* m_categoryList;
    QStackedWidget* m_settingsStack;
    QDialogButtonBox* m_buttonBox;
    QPushButton* m_restoreDefaultsButton;
    QPushButton* m_importButton;
    QPushButton* m_exportButton;
    
    // Settings pages
    QWidget* m_generalPage;
    QWidget* m_pluginPage;
    QWidget* m_uiPage;
    QWidget* m_filePage;
    QWidget* m_advancedPage;
    
    // Settings storage
    QSettings* m_settings;
    QSettings* m_tempSettings; // For preview/cancel functionality
    
    // State tracking
    bool m_settingsChanged;
    
    // UI setup methods
    void setupUI();
    void setupCategoryList();
    void setupSettingsPages();
    void setupConnections();
    void applyDarkTheme();
    
    // Settings page creation
    QWidget* createGeneralPage();
    QWidget* createPluginPage();
    QWidget* createUIPage();
    QWidget* createFilePage();
    QWidget* createAdvancedPage();
    
    // Settings management helpers
    void copySettings(QSettings* source, QSettings* destination);
    void applyTempSettings();
    void discardTempSettings();
    QVariant getDefaultValue(const QString& key) const;
    
    // Validation
    bool validateSettings() const;
    QStringList getValidationErrors() const;
};