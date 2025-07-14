/**
 * Item Editor Qt6 - New OTB File Dialog Implementation
 * Exact mirror of Legacy_App/csharp/Source/Dialogs/NewOtbFileForm.cs
 *
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "NewOtbFileDialog.h"
#include "ui_NewOtbFileDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QDir>
#include <algorithm>

namespace ItemEditor {

NewOtbFileDialog::NewOtbFileDialog(PluginServices* pluginServices, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewOtbFileDialog),
    m_pluginServices(pluginServices)
{
    ui->setupUi(this);
    setupUi();
    populateClientVersions();
}

NewOtbFileDialog::~NewOtbFileDialog()
{
    delete ui;
}

void NewOtbFileDialog::setupUi()
{
    setWindowTitle("New OTB");
    setFixedSize(181, 78);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    m_clientVersionComboBox = new QComboBox(this);
    mainLayout->addWidget(m_clientVersionComboBox);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_createButton = new QPushButton("Create", this);
    m_cancelButton = new QPushButton("Cancel", this);
    buttonLayout->addWidget(m_createButton);
    buttonLayout->addWidget(m_cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(m_createButton, &QPushButton::clicked, this, &NewOtbFileDialog::onCreateClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &NewOtbFileDialog::onCancelClicked);
    connect(m_clientVersionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NewOtbFileDialog::onClientVersionChanged);
}

void NewOtbFileDialog::populateClientVersions()
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
            m_clientVersionComboBox->addItem(client.getDescription(), QVariant::fromValue(client));
        }
        m_clientVersionComboBox->setCurrentIndex(clientList.size() - 1);
    }
    onClientVersionChanged(m_clientVersionComboBox->currentIndex());
}

QString NewOtbFileDialog::getFilePath() const
{
    return m_filePath;
}

SupportedClient NewOtbFileDialog::getSelectedClient() const
{
    return m_selectedClient;
}

void NewOtbFileDialog::onCreateClicked()
{
    if (m_clientVersionComboBox->currentIndex() != -1) {
        m_selectedClient = m_clientVersionComboBox->currentData().value<SupportedClient>();
        QTemporaryFile tempFile;
        if (tempFile.open()) {
            m_filePath = tempFile.fileName();
        } else {
            m_filePath = QDir::temp().filePath("new.otb");
        }
        accept();
    }
}

void NewOtbFileDialog::onCancelClicked()
{
    reject();
}

void NewOtbFileDialog::onClientVersionChanged(int index)
{
    m_createButton->setEnabled(index != -1);
}

} // namespace ItemEditor
