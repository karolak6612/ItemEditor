#include "SettingsDialog.h"
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QApplication>
#include <QPushButton>
#include <QSlider>
#include <QColorDialog>
#include <QFontDialog>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , m_splitter(nullptr)
    , m_categoryList(nullptr)
    , m_settingsStack(nullptr)
    , m_buttonBox(nullptr)
    , m_restoreDefaultsButton(nullptr)
    , m_importButton(nullptr)
    , m_exportButton(nullptr)
    , m_generalPage(nullptr)
    , m_pluginPage(nullptr)
    , m_uiPage(nullptr)
    , m_filePage(nullptr)
    , m_advancedPage(nullptr)
    , m_settings(new QSettings(this))
    , m_tempSettings(new QSettings(QSettings::IniFormat, QSettings::UserScope, "ItemEditor", "TempSettings", this))
    , m_settingsChanged(false)
{
    setupUI();
    setupCategoryList();
    setupSettingsPages();
    setupConnections();
    applyDarkTheme();
    
    loadSettings();
    
    // Select first category by default
    if (m_categoryList->count() > 0) {
        m_categoryList->setCurrentRow(0);
    }
}

SettingsDialog::~SettingsDialog()
{
    // Cleanup temp settings file
    m_tempSettings->clear();
}

void SettingsDialog::loadSettings()
{
    // Copy current settings to temp settings for preview
    copySettings(m_settings, m_tempSettings);
    m_settingsChanged = false;
}

void SettingsDialog::saveSettings()
{
    if (!validateSettings()) {
        QStringList errors = getValidationErrors();
        QMessageBox::warning(this, "Invalid Settings", 
                           "Please correct the following errors:\n\n" + errors.join("\n"));
        return;
    }
    
    // Copy temp settings to actual settings
    copySettings(m_tempSettings, m_settings);
    m_settings->sync();
    
    m_settingsChanged = false;
    emit settingsChanged();
}

void SettingsDialog::resetToDefaults()
{
    int result = QMessageBox::question(this, "Reset Settings",
                                     "Are you sure you want to reset all settings to their default values?\n\n"
                                     "This action cannot be undone.",
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        m_tempSettings->clear();
        
        // Set default values
        m_tempSettings->setValue("General/AutoSave", true);
        m_tempSettings->setValue("General/AutoSaveInterval", 5);
        m_tempSettings->setValue("General/CheckForUpdates", true);
        m_tempSettings->setValue("General/ShowSplashScreen", true);
        
        m_tempSettings->setValue("UI/Theme", "Dark");
        m_tempSettings->setValue("UI/ShowToolTips", true);
        m_tempSettings->setValue("UI/ShowStatusBar", true);
        m_tempSettings->setValue("UI/ShowToolBar", true);
        
        m_tempSettings->setValue("Files/DefaultDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
        m_tempSettings->setValue("Files/CreateBackups", true);
        m_tempSettings->setValue("Files/MaxBackups", 5);
        m_tempSettings->setValue("Files/AutoValidate", true);
        
        m_tempSettings->setValue("Plugins/AutoLoadPlugins", true);
        m_tempSettings->setValue("Plugins/ShowPluginErrors", true);
        m_tempSettings->setValue("Plugins/PluginTimeout", 30);
        
        m_tempSettings->setValue("Advanced/LogLevel", "Info");
        m_tempSettings->setValue("Advanced/MaxLogFiles", 10);
        m_tempSettings->setValue("Advanced/EnableDebugMode", false);
        m_tempSettings->setValue("Advanced/MemoryOptimization", true);
        
        m_settingsChanged = true;
        onSettingChanged();
    }
}

bool SettingsDialog::importSettings(const QString& filePath)
{
    QSettings importSettings(filePath, QSettings::IniFormat);
    
    if (importSettings.status() != QSettings::NoError) {
        QMessageBox::warning(this, "Import Error", 
                           "Failed to read settings file:\n" + filePath);
        return false;
    }
    
    int result = QMessageBox::question(this, "Import Settings",
                                     "This will replace all current settings with those from the selected file.\n\n"
                                     "Do you want to continue?",
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        copySettings(&importSettings, m_tempSettings);
        m_settingsChanged = true;
        onSettingChanged();
        return true;
    }
    
    return false;
}

bool SettingsDialog::exportSettings(const QString& filePath)
{
    QSettings exportSettings(filePath, QSettings::IniFormat);
    copySettings(m_tempSettings, &exportSettings);
    exportSettings.sync();
    
    if (exportSettings.status() != QSettings::NoError) {
        QMessageBox::warning(this, "Export Error", 
                           "Failed to write settings file:\n" + filePath);
        return false;
    }
    
    QMessageBox::information(this, "Export Complete", 
                           "Settings have been successfully exported to:\n" + filePath);
    return true;
}

void SettingsDialog::showGeneralSettings()
{
    m_categoryList->setCurrentRow(0);
}

void SettingsDialog::showPluginSettings()
{
    m_categoryList->setCurrentRow(1);
}

void SettingsDialog::showUISettings()
{
    m_categoryList->setCurrentRow(2);
}

void SettingsDialog::showFileSettings()
{
    m_categoryList->setCurrentRow(3);
}

void SettingsDialog::showAdvancedSettings()
{
    m_categoryList->setCurrentRow(4);
}

void SettingsDialog::accept()
{
    saveSettings();
    QDialog::accept();
}

void SettingsDialog::reject()
{
    if (m_settingsChanged) {
        int result = QMessageBox::question(this, "Discard Changes",
                                         "You have unsaved changes. Do you want to discard them?",
                                         QMessageBox::Yes | QMessageBox::No,
                                         QMessageBox::No);
        
        if (result == QMessageBox::No) {
            return;
        }
    }
    
    discardTempSettings();
    QDialog::reject();
}

void SettingsDialog::onCategoryChanged()
{
    int currentRow = m_categoryList->currentRow();
    if (currentRow >= 0 && currentRow < m_settingsStack->count()) {
        m_settingsStack->setCurrentIndex(currentRow);
    }
}

void SettingsDialog::onRestoreDefaultsClicked()
{
    resetToDefaults();
}

void SettingsDialog::onImportSettingsClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                   "Import Settings",
                                                   QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                   "Settings Files (*.ini);;All Files (*.*)");
    
    if (!filePath.isEmpty()) {
        importSettings(filePath);
    }
}

void SettingsDialog::onExportSettingsClicked()
{
    QString filePath = QFileDialog::getSaveFileName(this,
                                                   "Export Settings",
                                                   QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/ItemEditor_Settings.ini",
                                                   "Settings Files (*.ini);;All Files (*.*)");
    
    if (!filePath.isEmpty()) {
        exportSettings(filePath);
    }
}

void SettingsDialog::onSettingChanged()
{
    m_settingsChanged = true;
    // Update UI to reflect changes (if needed)
}

void SettingsDialog::setupUI()
{
    setWindowTitle("Settings");
    setModal(true);
    setMinimumSize(800, 600);
    resize(900, 700);
    
    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Splitter for category list and settings pages
    m_splitter = new QSplitter(Qt::Horizontal, this);
    mainLayout->addWidget(m_splitter);
    
    // Category list
    m_categoryList = new QListWidget();
    m_categoryList->setMaximumWidth(200);
    m_categoryList->setMinimumWidth(150);
    m_splitter->addWidget(m_categoryList);
    
    // Settings stack
    m_settingsStack = new QStackedWidget();
    m_splitter->addWidget(m_settingsStack);
    
    // Set splitter proportions
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
    
    // Button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    // Import/Export buttons
    m_importButton = new QPushButton("Import...");
    m_exportButton = new QPushButton("Export...");
    m_restoreDefaultsButton = new QPushButton("Restore Defaults");
    
    buttonLayout->addWidget(m_importButton);
    buttonLayout->addWidget(m_exportButton);
    buttonLayout->addWidget(m_restoreDefaultsButton);
    buttonLayout->addStretch();
    
    // Standard dialog buttons
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
    buttonLayout->addWidget(m_buttonBox);
    
    mainLayout->addLayout(buttonLayout);
}

void SettingsDialog::setupCategoryList()
{
    m_categoryList->addItem("General");
    m_categoryList->addItem("Plugins");
    m_categoryList->addItem("User Interface");
    m_categoryList->addItem("Files");
    m_categoryList->addItem("Advanced");
}

void SettingsDialog::setupSettingsPages()
{
    m_generalPage = createGeneralPage();
    m_pluginPage = createPluginPage();
    m_uiPage = createUIPage();
    m_filePage = createFilePage();
    m_advancedPage = createAdvancedPage();
    
    m_settingsStack->addWidget(m_generalPage);
    m_settingsStack->addWidget(m_pluginPage);
    m_settingsStack->addWidget(m_uiPage);
    m_settingsStack->addWidget(m_filePage);
    m_settingsStack->addWidget(m_advancedPage);
}

void SettingsDialog::setupConnections()
{
    // Category selection
    connect(m_categoryList, &QListWidget::currentRowChanged, this, &SettingsDialog::onCategoryChanged);
    
    // Buttons
    connect(m_importButton, &QPushButton::clicked, this, &SettingsDialog::onImportSettingsClicked);
    connect(m_exportButton, &QPushButton::clicked, this, &SettingsDialog::onExportSettingsClicked);
    connect(m_restoreDefaultsButton, &QPushButton::clicked, this, &SettingsDialog::onRestoreDefaultsClicked);
    
    // Dialog buttons
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);
    connect(m_buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &SettingsDialog::saveSettings);
}

QWidget* SettingsDialog::createGeneralPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    
    // Startup group
    QGroupBox* startupGroup = new QGroupBox("Startup");
    QFormLayout* startupLayout = new QFormLayout(startupGroup);
    
    QCheckBox* showSplashCheck = new QCheckBox();
    showSplashCheck->setObjectName("showSplashCheck");
    startupLayout->addRow("Show splash screen:", showSplashCheck);
    
    QCheckBox* checkUpdatesCheck = new QCheckBox();
    checkUpdatesCheck->setObjectName("checkUpdatesCheck");
    startupLayout->addRow("Check for updates:", checkUpdatesCheck);
    
    layout->addWidget(startupGroup);
    
    // Auto-save group
    QGroupBox* autoSaveGroup = new QGroupBox("Auto-save");
    QFormLayout* autoSaveLayout = new QFormLayout(autoSaveGroup);
    
    QCheckBox* autoSaveCheck = new QCheckBox();
    autoSaveCheck->setObjectName("autoSaveCheck");
    autoSaveLayout->addRow("Enable auto-save:", autoSaveCheck);
    
    QSpinBox* autoSaveInterval = new QSpinBox();
    autoSaveInterval->setObjectName("autoSaveInterval");
    autoSaveInterval->setRange(1, 60);
    autoSaveInterval->setSuffix(" minutes");
    autoSaveLayout->addRow("Auto-save interval:", autoSaveInterval);
    
    layout->addWidget(autoSaveGroup);
    
    layout->addStretch();
    return page;
}

QWidget* SettingsDialog::createPluginPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    
    // Plugin loading group
    QGroupBox* loadingGroup = new QGroupBox("Plugin Loading");
    QFormLayout* loadingLayout = new QFormLayout(loadingGroup);
    
    QCheckBox* autoLoadCheck = new QCheckBox();
    autoLoadCheck->setObjectName("autoLoadCheck");
    loadingLayout->addRow("Auto-load plugins:", autoLoadCheck);
    
    QSpinBox* timeoutSpin = new QSpinBox();
    timeoutSpin->setObjectName("timeoutSpin");
    timeoutSpin->setRange(5, 300);
    timeoutSpin->setSuffix(" seconds");
    loadingLayout->addRow("Plugin timeout:", timeoutSpin);
    
    layout->addWidget(loadingGroup);
    
    // Error handling group
    QGroupBox* errorGroup = new QGroupBox("Error Handling");
    QFormLayout* errorLayout = new QFormLayout(errorGroup);
    
    QCheckBox* showErrorsCheck = new QCheckBox();
    showErrorsCheck->setObjectName("showErrorsCheck");
    errorLayout->addRow("Show plugin errors:", showErrorsCheck);
    
    layout->addWidget(errorGroup);
    
    layout->addStretch();
    return page;
}

QWidget* SettingsDialog::createUIPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    
    // Theme group
    QGroupBox* themeGroup = new QGroupBox("Theme");
    QFormLayout* themeLayout = new QFormLayout(themeGroup);
    
    QComboBox* themeCombo = new QComboBox();
    themeCombo->setObjectName("themeCombo");
    themeCombo->addItems({"Dark", "Light", "System"});
    themeLayout->addRow("Theme:", themeCombo);
    
    layout->addWidget(themeGroup);
    
    // Interface group
    QGroupBox* interfaceGroup = new QGroupBox("Interface");
    QFormLayout* interfaceLayout = new QFormLayout(interfaceGroup);
    
    QCheckBox* showToolTipsCheck = new QCheckBox();
    showToolTipsCheck->setObjectName("showToolTipsCheck");
    interfaceLayout->addRow("Show tooltips:", showToolTipsCheck);
    
    QCheckBox* showStatusBarCheck = new QCheckBox();
    showStatusBarCheck->setObjectName("showStatusBarCheck");
    interfaceLayout->addRow("Show status bar:", showStatusBarCheck);
    
    QCheckBox* showToolBarCheck = new QCheckBox();
    showToolBarCheck->setObjectName("showToolBarCheck");
    interfaceLayout->addRow("Show toolbar:", showToolBarCheck);
    
    layout->addWidget(interfaceGroup);
    
    layout->addStretch();
    return page;
}

QWidget* SettingsDialog::createFilePage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    
    // Default directory group
    QGroupBox* directoryGroup = new QGroupBox("Default Directory");
    QFormLayout* directoryLayout = new QFormLayout(directoryGroup);
    
    QHBoxLayout* dirLayout = new QHBoxLayout();
    QLineEdit* defaultDirEdit = new QLineEdit();
    defaultDirEdit->setObjectName("defaultDirEdit");
    QPushButton* browseDirButton = new QPushButton("Browse...");
    dirLayout->addWidget(defaultDirEdit);
    dirLayout->addWidget(browseDirButton);
    directoryLayout->addRow("Default directory:", dirLayout);
    
    layout->addWidget(directoryGroup);
    
    // Backup group
    QGroupBox* backupGroup = new QGroupBox("Backups");
    QFormLayout* backupLayout = new QFormLayout(backupGroup);
    
    QCheckBox* createBackupsCheck = new QCheckBox();
    createBackupsCheck->setObjectName("createBackupsCheck");
    backupLayout->addRow("Create backups:", createBackupsCheck);
    
    QSpinBox* maxBackupsSpin = new QSpinBox();
    maxBackupsSpin->setObjectName("maxBackupsSpin");
    maxBackupsSpin->setRange(1, 50);
    backupLayout->addRow("Maximum backups:", maxBackupsSpin);
    
    layout->addWidget(backupGroup);
    
    // Validation group
    QGroupBox* validationGroup = new QGroupBox("Validation");
    QFormLayout* validationLayout = new QFormLayout(validationGroup);
    
    QCheckBox* autoValidateCheck = new QCheckBox();
    autoValidateCheck->setObjectName("autoValidateCheck");
    validationLayout->addRow("Auto-validate files:", autoValidateCheck);
    
    layout->addWidget(validationGroup);
    
    layout->addStretch();
    return page;
}

QWidget* SettingsDialog::createAdvancedPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    
    // Logging group
    QGroupBox* loggingGroup = new QGroupBox("Logging");
    QFormLayout* loggingLayout = new QFormLayout(loggingGroup);
    
    QComboBox* logLevelCombo = new QComboBox();
    logLevelCombo->setObjectName("logLevelCombo");
    logLevelCombo->addItems({"Error", "Warning", "Info", "Debug"});
    loggingLayout->addRow("Log level:", logLevelCombo);
    
    QSpinBox* maxLogFilesSpin = new QSpinBox();
    maxLogFilesSpin->setObjectName("maxLogFilesSpin");
    maxLogFilesSpin->setRange(1, 100);
    loggingLayout->addRow("Maximum log files:", maxLogFilesSpin);
    
    layout->addWidget(loggingGroup);
    
    // Performance group
    QGroupBox* performanceGroup = new QGroupBox("Performance");
    QFormLayout* performanceLayout = new QFormLayout(performanceGroup);
    
    QCheckBox* memoryOptCheck = new QCheckBox();
    memoryOptCheck->setObjectName("memoryOptCheck");
    performanceLayout->addRow("Memory optimization:", memoryOptCheck);
    
    QCheckBox* debugModeCheck = new QCheckBox();
    debugModeCheck->setObjectName("debugModeCheck");
    performanceLayout->addRow("Enable debug mode:", debugModeCheck);
    
    layout->addWidget(performanceGroup);
    
    layout->addStretch();
    return page;
}

void SettingsDialog::applyDarkTheme()
{
    setStyleSheet(R"(
        QDialog {
            background-color: #3C3F41;
            color: #DCDCDC;
        }
        
        QListWidget {
            background-color: #2B2B2B;
            border: 1px solid #555555;
            color: #DCDCDC;
            selection-background-color: #6897BB;
        }
        
        QListWidget::item {
            padding: 8px;
            border-bottom: 1px solid #555555;
        }
        
        QListWidget::item:selected {
            background-color: #6897BB;
        }
        
        QListWidget::item:hover {
            background-color: #4C5052;
        }
        
        QGroupBox {
            font-weight: bold;
            border: 2px solid #555555;
            border-radius: 5px;
            margin-top: 1ex;
            color: #DCDCDC;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        
        QLabel {
            color: #DCDCDC;
        }
        
        QLineEdit {
            background-color: #45494A;
            border: 1px solid #555555;
            border-radius: 3px;
            padding: 5px;
            color: #DCDCDC;
        }
        
        QLineEdit:focus {
            border: 1px solid #6897BB;
        }
        
        QComboBox {
            background-color: #45494A;
            border: 1px solid #555555;
            border-radius: 3px;
            padding: 5px;
            color: #DCDCDC;
        }
        
        QComboBox:focus {
            border: 1px solid #6897BB;
        }
        
        QSpinBox {
            background-color: #45494A;
            border: 1px solid #555555;
            border-radius: 3px;
            padding: 5px;
            color: #DCDCDC;
        }
        
        QSpinBox:focus {
            border: 1px solid #6897BB;
        }
        
        QCheckBox {
            color: #DCDCDC;
        }
        
        QCheckBox::indicator {
            width: 13px;
            height: 13px;
        }
        
        QCheckBox::indicator:unchecked {
            background-color: #45494A;
            border: 1px solid #555555;
        }
        
        QCheckBox::indicator:checked {
            background-color: #6897BB;
            border: 1px solid #6897BB;
        }
        
        QPushButton {
            background-color: #45494A;
            border: 1px solid #555555;
            border-radius: 3px;
            padding: 5px 15px;
            color: #DCDCDC;
        }
        
        QPushButton:hover {
            background-color: #4C5052;
        }
        
        QPushButton:pressed {
            background-color: #3C3F41;
        }
        
        QSplitter::handle {
            background-color: #555555;
        }
    )");
}

void SettingsDialog::copySettings(QSettings* source, QSettings* destination)
{
    destination->clear();
    
    for (const QString& key : source->allKeys()) {
        destination->setValue(key, source->value(key));
    }
    
    destination->sync();
}

void SettingsDialog::applyTempSettings()
{
    copySettings(m_tempSettings, m_settings);
}

void SettingsDialog::discardTempSettings()
{
    copySettings(m_settings, m_tempSettings);
}

QVariant SettingsDialog::getDefaultValue(const QString& key) const
{
    // Default values for all settings
    static QHash<QString, QVariant> defaults = {
        {"General/AutoSave", true},
        {"General/AutoSaveInterval", 5},
        {"General/CheckForUpdates", true},
        {"General/ShowSplashScreen", true},
        
        {"UI/Theme", "Dark"},
        {"UI/ShowToolTips", true},
        {"UI/ShowStatusBar", true},
        {"UI/ShowToolBar", true},
        
        {"Files/DefaultDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)},
        {"Files/CreateBackups", true},
        {"Files/MaxBackups", 5},
        {"Files/AutoValidate", true},
        
        {"Plugins/AutoLoadPlugins", true},
        {"Plugins/ShowPluginErrors", true},
        {"Plugins/PluginTimeout", 30},
        
        {"Advanced/LogLevel", "Info"},
        {"Advanced/MaxLogFiles", 10},
        {"Advanced/EnableDebugMode", false},
        {"Advanced/MemoryOptimization", true}
    };
    
    return defaults.value(key);
}

bool SettingsDialog::validateSettings() const
{
    // Validate all settings
    return getValidationErrors().isEmpty();
}

QStringList SettingsDialog::getValidationErrors() const
{
    QStringList errors;
    
    // Validate auto-save interval
    int interval = m_tempSettings->value("General/AutoSaveInterval", 5).toInt();
    if (interval < 1 || interval > 60) {
        errors << "Auto-save interval must be between 1 and 60 minutes";
    }
    
    // Validate plugin timeout
    int timeout = m_tempSettings->value("Plugins/PluginTimeout", 30).toInt();
    if (timeout < 5 || timeout > 300) {
        errors << "Plugin timeout must be between 5 and 300 seconds";
    }
    
    // Validate max backups
    int maxBackups = m_tempSettings->value("Files/MaxBackups", 5).toInt();
    if (maxBackups < 1 || maxBackups > 50) {
        errors << "Maximum backups must be between 1 and 50";
    }
    
    // Validate max log files
    int maxLogFiles = m_tempSettings->value("Advanced/MaxLogFiles", 10).toInt();
    if (maxLogFiles < 1 || maxLogFiles > 100) {
        errors << "Maximum log files must be between 1 and 100";
    }
    
    return errors;
}