/**
 * Item Editor Qt6 - Update OTB Dialog Implementation
 *
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "UpdateOtbDialog.h"
#include "ui_UpdateOtbDialog.h"
#include "../MainForm.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <algorithm>

namespace ItemEditor {

UpdateOtbDialog::UpdateOtbDialog(MainForm* mainForm, PluginServices* pluginServices, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateOtbDialog),
    m_mainForm(mainForm),
    m_pluginServices(pluginServices)
{
    ui->setupUi(this);
    setupUi();
    populateClientVersions();
}

UpdateOtbDialog::~UpdateOtbDialog()
{
    delete ui;
}

void UpdateOtbDialog::setupUi()
{
    setWindowTitle("Update OTB");
    setFixedSize(312, 178);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    m_pluginsListBox = new QListWidget(this);
    mainLayout->addWidget(m_pluginsListBox);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_selectButton = new QPushButton("Select", this);
    m_cancelButton = new QPushButton("Cancel", this);
    buttonLayout->addWidget(m_selectButton);
    buttonLayout->addWidget(m_cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(m_selectButton, &QPushButton::clicked, this, &UpdateOtbDialog::onSelectClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &UpdateOtbDialog::onCancelClicked);
    connect(m_pluginsListBox, &QListWidget::itemSelectionChanged, this, &UpdateOtbDialog::onClientVersionChanged);
}

void UpdateOtbDialog::populateClientVersions()
{
    QList<SupportedClient> clientList;
    if (m_pluginServices) {
        for (Plugin* plugin : *m_pluginServices->availablePlugins()) {
            if (plugin && plugin->instance()) {
                for (const SupportedClient& client : plugin->instance()->supportedClients()) {
                    clientList.append(client);
                }
            }
        }
    }

    if (!clientList.isEmpty()) {
        std::sort(clientList.begin(), clientList.end(), [](const SupportedClient& a, const SupportedClient& b) {
            return a.getOtbVersion() < b.getOtbVersion();
        });

        for (const SupportedClient& client : clientList) {
            QListWidgetItem* item = new QListWidgetItem(client.getDescription());
            item->setData(Qt::UserRole, QVariant::fromValue(client));
            m_pluginsListBox->addItem(item);
        }
        m_pluginsListBox->setCurrentRow(clientList.size() - 1);
    }
    onClientVersionChanged();
}

Plugin* UpdateOtbDialog::getSelectedPlugin() const
{
    return m_selectedPlugin;
}

SupportedClient UpdateOtbDialog::getSelectedClient() const
{
    return m_selectedClient;
}

void UpdateOtbDialog::onSelectClicked()
{
    if (m_pluginsListBox->currentItem()) {
        m_selectedClient = m_pluginsListBox->currentItem()->data(Qt::UserRole).value<SupportedClient>();
        m_selectedPlugin = m_pluginServices->findPlugin(m_selectedClient.getName());
        accept();
    }
}

void UpdateOtbDialog::onCancelClicked()
{
    reject();
}

void UpdateOtbDialog::onClientVersionChanged()
{
    m_selectButton->setEnabled(m_pluginsListBox->currentItem() != nullptr);
}

} // namespace ItemEditor
