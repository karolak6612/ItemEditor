#pragma once

#include <QDialog>
#include <QListWidget>
#include <QTextEdit>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include "../ItemEditor.Plugins/PluginManager.h"
#include "../ItemEditor.Plugins/IPlugin.h"

/**
 * @brief Plugin configuration dialog
 * 
 * Provides detailed plugin management and configuration:
 * - View loaded plugins and their status
 * - Configure plugin-specific settings
 * - Enable/disable plugins
 * - View plugin information and diagnostics
 */
class PluginConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PluginConfigDialog(PluginManager* pluginManager, QWidget *parent = nullptr);
    ~PluginConfigDialog();

    // Plugin management
    void refreshPluginList();
    void selectPlugin(const QString& pluginName);

public slots:
    void accept() override;
    void reject() override;

private slots:
    void onPluginSelectionChanged();
    void onEnablePluginToggled(bool enabled);
    void onRefreshPluginsClicked();
    void onPluginSettingsChanged();

signals:
    void pluginConfigurationChanged();

private:
    // UI components
    QSplitter* m_splitter;
    QListWidget* m_pluginList;
    QWidget* m_detailsWidget;
    
    // Plugin details
    QLabel* m_pluginNameLabel;
    QLabel* m_pluginVersionLabel;
    QLabel* m_pluginDescriptionLabel;
    QLabel* m_pluginStatusLabel;
    QLabel* m_supportedVersionsLabel;
    QTextEdit* m_pluginInfoText;
    
    // Plugin configuration
    QGroupBox* m_configGroup;
    QCheckBox* m_enabledCheckBox;
    QLineEdit* m_pluginPathEdit;
    QSpinBox* m_timeoutSpinBox;
    QCheckBox* m_autoLoadCheckBox;
    
    // Buttons
    QPushButton* m_refreshButton;
    QPushButton* m_loadPluginButton;
    QPushButton* m_unloadPluginButton;
    
    // Data
    PluginManager* m_pluginManager;
    QString m_selectedPluginName;
    bool m_settingsChanged;
    
    // UI setup
    void setupUI();
    void setupPluginList();
    void setupDetailsPanel();
    void setupConnections();
    void applyDarkTheme();
    
    // Plugin information display
    void updatePluginDetails();
    void clearPluginDetails();
    QString getPluginStatusText(IPlugin* plugin) const;
    QString formatPluginInfo(IPlugin* plugin) const;
    
    // Configuration management
    void loadPluginConfiguration();
    void savePluginConfiguration();
    bool validateConfiguration() const;
};