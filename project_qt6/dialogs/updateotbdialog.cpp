#include "updateotbdialog.h"
#include "plugins/iplugin.h" // For IPlugin, PluginManager

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDebug>

UpdateOtbDialog::UpdateOtbDialog(PluginManager* pluginManager, quint32 currentOtbVersion, QWidget *parent)
    : QDialog(parent), m_pluginManager(pluginManager), m_currentOtbVersion(currentOtbVersion)
{
    setWindowTitle(tr("Update OTB Version"));
    setMinimumWidth(400);
    setupUi();
    populateClientVersions();

    // Set initial options based on C# UpdateSettingsForm defaults
    reassignUnmatchedSpritesCheckBox->setChecked(true);
    generateSignaturesCheckBox->setChecked(false); // Default false, it's slow
    reloadAttributesCheckBox->setChecked(true);
    createNewItemsCheckBox->setChecked(true);
}

UpdateOtbDialog::~UpdateOtbDialog()
{
}

void UpdateOtbDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // --- Target Client Selection ---
    QHBoxLayout* targetClientLayout = new QHBoxLayout();
    targetClientLayout->addWidget(new QLabel(tr("Update to Client Version:"), this));
    targetClientComboBox = new QComboBox(this);
    targetClientLayout->addWidget(targetClientComboBox, 1);
    mainLayout->addLayout(targetClientLayout);
    connect(targetClientComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &UpdateOtbDialog::onClientVersionSelected);


    // --- Update Options ---
    optionsGroupBox = new QGroupBox(tr("Update Options"), this);
    QVBoxLayout* optionsLayout = new QVBoxLayout(optionsGroupBox);

    reassignUnmatchedSpritesCheckBox = new QCheckBox(tr("Reassign Items with Unmatched Sprites"), this);
    generateSignaturesCheckBox = new QCheckBox(tr("Generate Image Signatures (Slow, for better matching)"), this);
    reloadAttributesCheckBox = new QCheckBox(tr("Reload Item Attributes"), this);
    createNewItemsCheckBox = new QCheckBox(tr("Create New Items for Unassigned Sprites"), this);

    optionsLayout->addWidget(reassignUnmatchedSpritesCheckBox);
    optionsLayout->addWidget(generateSignaturesCheckBox);
    optionsLayout->addWidget(reloadAttributesCheckBox);
    optionsLayout->addWidget(createNewItemsCheckBox);

    mainLayout->addWidget(optionsGroupBox);

    // --- Dialog Buttons ---
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Update"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &UpdateOtbDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

void UpdateOtbDialog::populateClientVersions()
{
    targetClientComboBox->clear();
    if (!m_pluginManager) return;

    for (IPlugin* plugin : m_pluginManager->availablePlugins()) {
        for (const OTB::SupportedClient& client : plugin->getSupportedClients()) {
            // Don't list the current version as a target for update
            if (client.otbVersion != m_currentOtbVersion && client.version != m_currentOtbVersion) {
                QVariant clientData;
                clientData.setValue(qMakePair(plugin, client)); // Store both plugin and client info
                targetClientComboBox->addItem(client.description, clientData);
            }
        }
    }

    if (targetClientComboBox->count() > 0) {
        onClientVersionSelected(0); // Select first item by default
    } else {
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false); // No targets, disable OK
    }
}

void UpdateOtbDialog::onClientVersionSelected(int index)
{
    if (index < 0) {
        m_options.targetPlugin = nullptr;
        return;
    }
    QVariant data = targetClientComboBox->itemData(index);
    if (data.canConvert<QPair<IPlugin*, OTB::SupportedClient>>()) {
        QPair<IPlugin*, OTB::SupportedClient> pair = data.value<QPair<IPlugin*, OTB::SupportedClient>>();
        m_options.targetPlugin = pair.first;
        m_options.targetClient = pair.second;
    } else {
         m_options.targetPlugin = nullptr;
         qWarning() << "Could not convert QVariant data in targetClientComboBox.";
    }
}

void UpdateOtbDialog::accept()
{
    // Store checkbox values into the options struct before accepting
    m_options.reassignUnmatchedSprites = reassignUnmatchedSpritesCheckBox->isChecked();
    m_options.generateImageSignatures = generateSignaturesCheckBox->isChecked();
    m_options.reloadItemAttributes = reloadAttributesCheckBox->isChecked();
    m_options.createNewItems = createNewItemsCheckBox->isChecked();

    if (!m_options.targetPlugin || m_options.targetClient.version == 0) {
        QMessageBox::warning(this, tr("No Target Selected"), tr("Please select a valid target client version."));
        return; // Don't close the dialog
    }

    QDialog::accept();
}

UpdateOptions UpdateOtbDialog::getSelectedUpdateOptions() const
{
    return m_options;
}
