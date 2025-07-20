#include "PluginConfigDialog.h"
#include <QFormLayout>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QSettings>
#include <QFileInfo>
#include <QDir>

PluginConfigDialog::PluginConfigDialog(PluginManager* pluginManager, QWidget *parent)
    : QDialog(parent)
    , m_splitter(nullptr)
    , m_pluginList(nullptr)
    , m_detailsWidget(nullptr)
    , m_pluginNameLabel(nullptr)
    , m_pluginVersionLabel(nullptr)
    , m_pluginDescriptionLabel(nullptr)
    , m_pluginStatusLabel(nullptr)
    , m_supportedVersionsLabel(nullptr)
    , m_pluginInfoText(nullptr)
    , m_configGroup(nullptr)
    , m_enabledCheckBox(nullptr)
    , m_pluginPathEdit(nullptr)
    , m_timeoutSpinBox(nullptr)
    , m_autoLoadCheckBox(nullptr)
    , m_refreshButton(nullptr)
    , m_loadPluginButton(nullptr)
    , m_unloadPluginButton(nullptr)
    , m_pluginManager(pluginManager)
    , m_settingsChanged(false)
{
    setupUI();
    setupPluginList();
    setupDetailsPanel();
    setupConnections();
    applyDarkTheme();
    
    refreshPluginList();
}

PluginConfigDialog::~PluginConfigDialog()
{
}

void PluginConfigDialog::refreshPluginList()
{
    m_pluginList->clear();
    
    if (!m_pluginManager) {
        return;
    }
    
    // Add loaded plugins
    QList<IPlugin*> availablePlugins = m_pluginManager->getAvailablePlugins();
    for (IPlugin* plugin : availablePlugins) {
        if (plugin) {
            QString name = plugin->name();
            QListWidgetItem* item = new QListWidgetItem(name);
            item->setData(Qt::UserRole, name);
            
            // Color code as enabled (all loaded plugins are considered enabled)
            item->setForeground(QColor("#90EE90")); // Light green for enabled
            
            m_pluginList->addItem(item);
        }
    }
    
    // Select first item if available
    if (m_pluginList->count() > 0) {
        m_pluginList->setCurrentRow(0);
    }
}

void PluginConfigDialog::selectPlugin(const QString& pluginName)
{
    for (int i = 0; i < m_pluginList->count(); ++i) {
        QListWidgetItem* item = m_pluginList->item(i);
        if (item && item->data(Qt::UserRole).toString() == pluginName) {
            m_pluginList->setCurrentItem(item);
            break;
        }
    }
}

void PluginConfigDialog::accept()
{
    if (m_settingsChanged) {
        savePluginConfiguration();
        emit pluginConfigurationChanged();
    }
    
    QDialog::accept();
}

void PluginConfigDialog::reject()
{
    if (m_settingsChanged) {
        int result = QMessageBox::question(this, "Discard Changes",
                                         "You have unsaved plugin configuration changes. Do you want to discard them?",
                                         QMessageBox::Yes | QMessageBox::No,
                                         QMessageBox::No);
        
        if (result == QMessageBox::No) {
            return;
        }
    }
    
    QDialog::reject();
}

void PluginConfigDialog::onPluginSelectionChanged()
{
    QListWidgetItem* currentItem = m_pluginList->currentItem();
    if (!currentItem) {
        clearPluginDetails();
        return;
    }
    
    m_selectedPluginName = currentItem->data(Qt::UserRole).toString();
    updatePluginDetails();
    loadPluginConfiguration();
}

void PluginConfigDialog::onEnablePluginToggled(bool enabled)
{
    if (m_selectedPluginName.isEmpty() || !m_pluginManager) {
        return;
    }
    
    if (enabled) {
        if (!m_pluginManager->isPluginLoaded(m_selectedPluginName)) {
            // Try to load the plugin first
            if (!m_pluginManager->loadPlugin(m_selectedPluginName)) {
                QMessageBox::warning(this, "Plugin Error", 
                                   "Failed to load plugin: " + m_selectedPluginName);
                m_enabledCheckBox->setChecked(false);
                return;
            }
        }
        m_pluginManager->enablePlugin(m_selectedPluginName);
    } else {
        m_pluginManager->disablePlugin(m_selectedPluginName);
    }
    
    m_settingsChanged = true;
    refreshPluginList();
    updatePluginDetails();
}

void PluginConfigDialog::onRefreshPluginsClicked()
{
    if (m_pluginManager) {
        m_pluginManager->refreshAvailablePlugins();
        refreshPluginList();
    }
}

void PluginConfigDialog::onPluginSettingsChanged()
{
    m_settingsChanged = true;
}

void PluginConfigDialog::setupUI()
{
    setWindowTitle("Plugin Configuration");
    setModal(true);
    setMinimumSize(800, 600);
    resize(900, 700);
    
    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Splitter for plugin list and details
    m_splitter = new QSplitter(Qt::Horizontal, this);
    mainLayout->addWidget(m_splitter);
    
    // Plugin list
    m_pluginList = new QListWidget();
    m_pluginList->setMaximumWidth(250);
    m_pluginList->setMinimumWidth(200);
    m_splitter->addWidget(m_pluginList);
    
    // Details widget
    m_detailsWidget = new QWidget();
    m_splitter->addWidget(m_detailsWidget);
    
    // Set splitter proportions
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
    
    // Button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_refreshButton = new QPushButton("Refresh");
    m_loadPluginButton = new QPushButton("Load Plugin");
    m_unloadPluginButton = new QPushButton("Unload Plugin");
    
    buttonLayout->addWidget(m_refreshButton);
    buttonLayout->addWidget(m_loadPluginButton);
    buttonLayout->addWidget(m_unloadPluginButton);
    buttonLayout->addStretch();
    
    // Standard dialog buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonLayout->addWidget(buttonBox);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect dialog buttons
    connect(buttonBox, &QDialogButtonBox::accepted, this, &PluginConfigDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &PluginConfigDialog::reject);
}

void PluginConfigDialog::setupPluginList()
{
    m_pluginList->setAlternatingRowColors(true);
    m_pluginList->setSelectionMode(QAbstractItemView::SingleSelection);
}

void PluginConfigDialog::setupDetailsPanel()
{
    QVBoxLayout* detailsLayout = new QVBoxLayout(m_detailsWidget);
    
    // Plugin information group
    QGroupBox* infoGroup = new QGroupBox("Plugin Information");
    QFormLayout* infoLayout = new QFormLayout(infoGroup);
    
    m_pluginNameLabel = new QLabel();
    m_pluginVersionLabel = new QLabel();
    m_pluginDescriptionLabel = new QLabel();
    m_pluginStatusLabel = new QLabel();
    m_supportedVersionsLabel = new QLabel();
    
    infoLayout->addRow("Name:", m_pluginNameLabel);
    infoLayout->addRow("Version:", m_pluginVersionLabel);
    infoLayout->addRow("Description:", m_pluginDescriptionLabel);
    infoLayout->addRow("Status:", m_pluginStatusLabel);
    infoLayout->addRow("Supported Versions:", m_supportedVersionsLabel);
    
    detailsLayout->addWidget(infoGroup);
    
    // Plugin configuration group
    m_configGroup = new QGroupBox("Configuration");
    QFormLayout* configLayout = new QFormLayout(m_configGroup);
    
    m_enabledCheckBox = new QCheckBox();
    configLayout->addRow("Enabled:", m_enabledCheckBox);
    
    m_pluginPathEdit = new QLineEdit();
    m_pluginPathEdit->setReadOnly(true);
    configLayout->addRow("Path:", m_pluginPathEdit);
    
    m_timeoutSpinBox = new QSpinBox();
    m_timeoutSpinBox->setRange(5, 300);
    m_timeoutSpinBox->setSuffix(" seconds");
    configLayout->addRow("Timeout:", m_timeoutSpinBox);
    
    m_autoLoadCheckBox = new QCheckBox();
    configLayout->addRow("Auto-load:", m_autoLoadCheckBox);
    
    detailsLayout->addWidget(m_configGroup);
    
    // Plugin details text
    QGroupBox* detailsGroup = new QGroupBox("Details");
    QVBoxLayout* detailsTextLayout = new QVBoxLayout(detailsGroup);
    
    m_pluginInfoText = new QTextEdit();
    m_pluginInfoText->setReadOnly(true);
    m_pluginInfoText->setMaximumHeight(200);
    detailsTextLayout->addWidget(m_pluginInfoText);
    
    detailsLayout->addWidget(detailsGroup);
    
    detailsLayout->addStretch();
}

void PluginConfigDialog::setupConnections()
{
    // Plugin list selection
    connect(m_pluginList, &QListWidget::currentItemChanged, this, &PluginConfigDialog::onPluginSelectionChanged);
    
    // Configuration controls
    connect(m_enabledCheckBox, &QCheckBox::toggled, this, &PluginConfigDialog::onEnablePluginToggled);
    connect(m_timeoutSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &PluginConfigDialog::onPluginSettingsChanged);
    connect(m_autoLoadCheckBox, &QCheckBox::toggled, this, &PluginConfigDialog::onPluginSettingsChanged);
    
    // Buttons
    connect(m_refreshButton, &QPushButton::clicked, this, &PluginConfigDialog::onRefreshPluginsClicked);
    
    // Load/Unload buttons
    connect(m_loadPluginButton, &QPushButton::clicked, [this]() {
        if (!m_selectedPluginName.isEmpty() && m_pluginManager) {
            if (m_pluginManager->loadPlugin(m_selectedPluginName)) {
                refreshPluginList();
                updatePluginDetails();
            } else {
                QMessageBox::warning(this, "Plugin Error", 
                                   "Failed to load plugin: " + m_selectedPluginName);
            }
        }
    });
    
    connect(m_unloadPluginButton, &QPushButton::clicked, [this]() {
        if (!m_selectedPluginName.isEmpty() && m_pluginManager) {
            if (m_pluginManager->unloadPlugin(m_selectedPluginName)) {
                refreshPluginList();
                updatePluginDetails();
            } else {
                QMessageBox::warning(this, "Plugin Error", 
                                   "Failed to unload plugin: " + m_selectedPluginName);
            }
        }
    });
}

void PluginConfigDialog::applyDarkTheme()
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
        
        QTextEdit {
            background-color: #2B2B2B;
            border: 1px solid #555555;
            color: #DCDCDC;
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
        
        QSpinBox {
            background-color: #45494A;
            border: 1px solid #555555;
            border-radius: 3px;
            padding: 5px;
            color: #DCDCDC;
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

void PluginConfigDialog::updatePluginDetails()
{
    if (m_selectedPluginName.isEmpty() || !m_pluginManager) {
        clearPluginDetails();
        return;
    }
    
    IPlugin* plugin = m_pluginManager->getPlugin(m_selectedPluginName);
    
    if (plugin) {
        m_pluginNameLabel->setText(plugin->name());
        m_pluginVersionLabel->setText(plugin->version());
        m_pluginDescriptionLabel->setText("Plugin for client data processing");
        m_pluginStatusLabel->setText(getPluginStatusText(plugin));
        m_supportedVersionsLabel->setText(plugin->supportedVersions().join(", "));
        m_pluginInfoText->setPlainText(formatPluginInfo(plugin));
        
        // Update configuration controls
        m_enabledCheckBox->setChecked(m_pluginManager->isPluginEnabled(m_selectedPluginName));
        m_pluginPathEdit->setText(m_pluginManager->getPluginPath(m_selectedPluginName));
        
        // Enable controls
        m_configGroup->setEnabled(true);
        m_loadPluginButton->setEnabled(false);
        m_unloadPluginButton->setEnabled(true);
    } else {
        // Plugin not loaded
        m_pluginNameLabel->setText(m_selectedPluginName);
        m_pluginVersionLabel->setText("Unknown");
        m_pluginDescriptionLabel->setText("Plugin not loaded");
        m_pluginStatusLabel->setText("Not Loaded");
        m_supportedVersionsLabel->setText("Unknown");
        m_pluginInfoText->setPlainText("Plugin is not currently loaded.");
        
        // Update configuration controls
        m_enabledCheckBox->setChecked(false);
        m_pluginPathEdit->setText(m_pluginManager->getPluginPath(m_selectedPluginName));
        
        // Enable/disable controls appropriately
        m_configGroup->setEnabled(false);
        m_loadPluginButton->setEnabled(true);
        m_unloadPluginButton->setEnabled(false);
    }
}

void PluginConfigDialog::clearPluginDetails()
{
    m_pluginNameLabel->clear();
    m_pluginVersionLabel->clear();
    m_pluginDescriptionLabel->clear();
    m_pluginStatusLabel->clear();
    m_supportedVersionsLabel->clear();
    m_pluginInfoText->clear();
    
    m_enabledCheckBox->setChecked(false);
    m_pluginPathEdit->clear();
    
    m_configGroup->setEnabled(false);
    m_loadPluginButton->setEnabled(false);
    m_unloadPluginButton->setEnabled(false);
}

QString PluginConfigDialog::getPluginStatusText(IPlugin* plugin) const
{
    if (!plugin) {
        return "Not Loaded";
    }
    
    if (m_pluginManager->isPluginEnabled(plugin->name())) {
        return "Loaded and Enabled";
    } else {
        return "Loaded but Disabled";
    }
}

QString PluginConfigDialog::formatPluginInfo(IPlugin* plugin) const
{
    if (!plugin) {
        return "No plugin information available.";
    }
    
    QString info;
    info += QString("Plugin Name: %1\n").arg(plugin->name());
    info += QString("Version: %1\n").arg(plugin->version());
    info += QString("Supported Client Versions: %1\n").arg(plugin->supportedVersions().join(", "));
    info += QString("Status: %1\n").arg(getPluginStatusText(plugin));
    
    if (m_pluginManager) {
        QString path = m_pluginManager->getPluginPath(plugin->name());
        if (!path.isEmpty()) {
            QFileInfo fileInfo(path);
            info += QString("File: %1\n").arg(fileInfo.fileName());
            info += QString("Path: %1\n").arg(fileInfo.absolutePath());
            info += QString("Size: %1 bytes\n").arg(fileInfo.size());
            info += QString("Modified: %1\n").arg(fileInfo.lastModified().toString());
        }
    }
    
    return info;
}

void PluginConfigDialog::loadPluginConfiguration()
{
    if (m_selectedPluginName.isEmpty()) {
        return;
    }
    
    QSettings settings;
    QString prefix = QString("Plugins/%1/").arg(m_selectedPluginName);
    
    m_timeoutSpinBox->setValue(settings.value(prefix + "Timeout", 30).toInt());
    m_autoLoadCheckBox->setChecked(settings.value(prefix + "AutoLoad", true).toBool());
}

void PluginConfigDialog::savePluginConfiguration()
{
    if (m_selectedPluginName.isEmpty()) {
        return;
    }
    
    QSettings settings;
    QString prefix = QString("Plugins/%1/").arg(m_selectedPluginName);
    
    settings.setValue(prefix + "Enabled", m_enabledCheckBox->isChecked());
    settings.setValue(prefix + "Timeout", m_timeoutSpinBox->value());
    settings.setValue(prefix + "AutoLoad", m_autoLoadCheckBox->isChecked());
    
    settings.sync();
}

bool PluginConfigDialog::validateConfiguration() const
{
    // Validate timeout value
    if (m_timeoutSpinBox->value() < 5 || m_timeoutSpinBox->value() > 300) {
        return false;
    }
    
    return true;
}