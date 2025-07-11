#include "preferencesdialog.h"
#include "plugins/iplugin.h" // For IPlugin, PluginManager, OTB::SupportedClient
#include "otb/item.h"      // For OTB::SupportedClient definition

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QSettings>
#include <QDebug>

PreferencesDialog::PreferencesDialog(PluginManager* pluginManager, QWidget *parent)
    : QDialog(parent), m_pluginManager(pluginManager), m_selectedPlugin(nullptr)
{
    setWindowTitle(tr("Preferences"));
    setMinimumWidth(450);
    setupUi();
    loadSettings(); // Load saved settings when dialog is created
}

PreferencesDialog::~PreferencesDialog()
{
}

void PreferencesDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // --- Client Selection Group ---
    clientSelectionGroupBox = new QGroupBox(tr("Client Version Selection"), this);
    QGridLayout* clientSelectionLayout = new QGridLayout(clientSelectionGroupBox);

    clientVersionLabel = new QLabel(tr("Preferred Client Version:"), this);
    clientVersionComboBox = new QComboBox(this);
    // Populate clientVersionComboBox based on available plugins and their supported clients
    if (m_pluginManager) {
        for (IPlugin* plugin : m_pluginManager->availablePlugins()) {
            for (const OTB::SupportedClient& client : plugin->getSupportedClients()) {
                // Store both description and enough data to retrieve plugin/client later
                QVariant clientData;
                clientData.setValue(client); // Store the whole SupportedClient struct
                clientVersionComboBox->addItem(client.description, clientData);
            }
        }
    }
    connect(clientVersionComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &PreferencesDialog::onClientVersionSelected);
    clientSelectionLayout->addWidget(clientVersionLabel, 0, 0);
    clientSelectionLayout->addWidget(clientVersionComboBox, 0, 1);
    mainLayout->addWidget(clientSelectionGroupBox);


    // --- Client Path Group ---
    clientPathGroupBox = new QGroupBox(tr("Client Files Path"), this);
    QGridLayout* clientPathLayout = new QGridLayout(clientPathGroupBox);
    clientDirectoryLabel = new QLabel(tr("Client Directory:"), this);
    clientDirectoryLineEdit = new QLineEdit(this);
    clientDirectoryLineEdit->setPlaceholderText(tr("Path to Tibia client (e.g., C:/Tibia)"));
    browseDirectoryButton = new QPushButton(tr("Browse..."), this);
    connect(browseDirectoryButton, &QPushButton::clicked, this, &PreferencesDialog::browseClientDirectory);
    clientPathLayout->addWidget(clientDirectoryLabel, 0, 0);
    clientPathLayout->addWidget(clientDirectoryLineEdit, 0, 1);
    clientPathLayout->addWidget(browseDirectoryButton, 0, 2);
    mainLayout->addWidget(clientPathGroupBox);

    // --- Client Options Group ---
    clientOptionsGroupBox = new QGroupBox(tr("Client Loading Options"), this);
    QVBoxLayout* clientOptionsLayout = new QVBoxLayout(clientOptionsGroupBox);
    extendedCheckBox = new QCheckBox(tr("Use Extended Sprite Format (for newer clients, e.g., 9.60+)"), this);
    frameDurationsCheckBox = new QCheckBox(tr("Load Frame Durations (for newer clients, e.g., 10.50+)"), this);
    transparencyCheckBox = new QCheckBox(tr("Enable Sprite Transparency"), this);
    clientOptionsLayout->addWidget(extendedCheckBox);
    clientOptionsLayout->addWidget(frameDurationsCheckBox);
    clientOptionsLayout->addWidget(transparencyCheckBox);
    mainLayout->addWidget(clientOptionsGroupBox);

    clientLoadingInfoLabel = new QLabel(tr("Note: Client data is loaded when an OTB is opened or 'Update OTB Version' is used."), this);
    clientLoadingInfoLabel->setWordWrap(true);
    clientLoadingInfoLabel->setStyleSheet("font-style: italic; color: gray;");
    mainLayout->addWidget(clientLoadingInfoLabel);


    // --- Dialog Buttons ---
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &PreferencesDialog::accept); // accept calls saveSettings
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

void PreferencesDialog::loadSettings()
{
    QSettings settings; // Uses organizationName and applicationName set in main.cpp usually

    // Load Client Directory Path
    clientDirectoryLineEdit->setText(settings.value("Preferences/ClientDirectory", "").toString());

    // Load Client Loading Options
    extendedCheckBox->setChecked(settings.value("Preferences/ExtendedSprites", true).toBool()); // Default true as many clients are newer
    frameDurationsCheckBox->setChecked(settings.value("Preferences/FrameDurations", true).toBool()); // Default true
    transparencyCheckBox->setChecked(settings.value("Preferences/Transparency", true).toBool()); // Default true

    // Load and set preferred client version
    quint32 preferredClientVersion = settings.value("Preferences/PreferredClientVersion", 0).toUInt();
    if (preferredClientVersion != 0) {
        for (int i = 0; i < clientVersionComboBox->count(); ++i) {
            OTB::SupportedClient sc = clientVersionComboBox->itemData(i).value<OTB::SupportedClient>();
            if (sc.version == preferredClientVersion) {
                clientVersionComboBox->setCurrentIndex(i);
                onClientVersionSelected(i); // To set m_selectedClient and m_selectedPlugin
                break;
            }
        }
    } else if (clientVersionComboBox->count() > 0) {
        clientVersionComboBox->setCurrentIndex(0); // Default to first if nothing saved
        onClientVersionSelected(0);
    }
}

void PreferencesDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("Preferences/ClientDirectory", clientDirectoryLineEdit->text());
    settings.setValue("Preferences/ExtendedSprites", extendedCheckBox->isChecked());
    settings.setValue("Preferences/FrameDurations", frameDurationsCheckBox->isChecked());
    settings.setValue("Preferences/Transparency", transparencyCheckBox->isChecked());

    if (m_selectedPlugin && m_selectedClient.version != 0) { // Check if a valid client was actually selected
        settings.setValue("Preferences/PreferredClientPluginName", m_selectedPlugin->pluginName()); // For info/debug
        settings.setValue("Preferences/PreferredClientVersion", m_selectedClient.version);
        settings.setValue("Preferences/PreferredClientDescription", m_selectedClient.description);
        // Store signatures too, as these are key for matching
        settings.setValue("Preferences/DatSignature", m_selectedClient.datSignature);
        settings.setValue("Preferences/SprSignature", m_selectedClient.sprSignature);
        settings.setValue("Preferences/OtbVersion", m_selectedClient.otbVersion);

        // These are used by MainForm C# to check compatibility before loading
        // Storing them directly simplifies MainForm's logic later.
        // Properties.Settings.Default["DatSignature"] = client.DatSignature;
        // Properties.Settings.Default["SprSignature"] = client.SprSignature;
    } else {
        // Clear preferred client if none is validly selected
        settings.remove("Preferences/PreferredClientPluginName");
        settings.remove("Preferences/PreferredClientVersion");
        settings.remove("Preferences/PreferredClientDescription");
        settings.remove("Preferences/DatSignature");
        settings.remove("Preferences/SprSignature");
        settings.remove("Preferences/OtbVersion");
    }
}

void PreferencesDialog::browseClientDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Client Directory"),
                                                    clientDirectoryLineEdit->text().isEmpty() ? QDir::homePath() : clientDirectoryLineEdit->text());
    if (!dir.isEmpty()) {
        clientDirectoryLineEdit->setText(dir);
    }
}

void PreferencesDialog::onPluginSelected(int index)
{
    // This would be used if plugins were listed separately from client versions.
    // For now, clientVersionComboBox handles combined selection.
    Q_UNUSED(index);
}

void PreferencesDialog::onClientVersionSelected(int index)
{
    if (index < 0 || !m_pluginManager) {
        m_selectedPlugin = nullptr;
        m_selectedClient = OTB::SupportedClient(); // Reset
        return;
    }

    QVariant clientData = clientVersionComboBox->itemData(index);
    OTB::SupportedClient sc = clientData.value<OTB::SupportedClient>();

    // Find the plugin that provides this client
    m_selectedPlugin = nullptr; // Reset first
    for (IPlugin* plugin : m_pluginManager->availablePlugins()) {
        for (const OTB::SupportedClient& supported : plugin->getSupportedClients()) {
            if (supported.version == sc.version && supported.description == sc.description) {
                m_selectedPlugin = plugin;
                m_selectedClient = sc; // Store the selected client
                // Update UI based on selected client capabilities (e.g., if extended is mandatory)
                // For dummy plugin, these are just user choices.
                // extendedCheckBox->setEnabled(!m_selectedClient.forceExtended); // Example
                return;
            }
        }
    }
    // Should not happen if combobox is populated correctly
    qWarning() << "Could not find plugin for selected client version:" << sc.description;
}


void PreferencesDialog::accept()
{
    saveSettings();
    QDialog::accept();
}


// --- Getter methods ---
QString PreferencesDialog::getSelectedClientDirectory() const {
    return clientDirectoryLineEdit->text();
}

OTB::SupportedClient PreferencesDialog::getSelectedClient() const {
    // Ensure m_selectedClient is up-to-date if user changed combo box but didn't hit OK yet
    // However, this is typically called after dialog.exec() == QDialog::Accepted, so onClientVersionSelected would have fired.
    return m_selectedClient;
}

IPlugin* PreferencesDialog::getSelectedPlugin() const {
    return m_selectedPlugin;
}

bool PreferencesDialog::isExtendedChecked() const {
    return extendedCheckBox->isChecked();
}

bool PreferencesDialog::isFrameDurationsChecked() const {
    return frameDurationsCheckBox->isChecked();
}

bool PreferencesDialog::isTransparencyChecked() const {
    return transparencyCheckBox->isChecked();
}
