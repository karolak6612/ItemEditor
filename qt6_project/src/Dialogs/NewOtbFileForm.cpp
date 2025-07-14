/**
 * Item Editor Qt6 - New OTB File Dialog Implementation
 * Exact mirror of Legacy_App/csharp/Source/Dialogs/NewOtbFileForm.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "NewOtbFileForm.h"
#include "ui_NewOtbFileForm.h"
#include "../Host/PluginServices.h"
#include "../PluginInterface/SupportedClient.h"

#include <QStandardPaths>
#include <QDir>
#include <QTemporaryFile>
#include <QMessageBox>
#include <algorithm>

namespace ItemEditor {

NewOtbFileDialog::NewOtbFileDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewOtbFileDialog)
{
    ui->setupUi(this);
    
    // Setup dialog properties - exact mirror of C# NewOtbFileForm properties
    setModal(true);
    setFixedSize(200, 100);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("New OTB"));
    
    // Setup UI components
    setupUi();
    loadClientVersions();
    
    // Connect signals - exact mirror of C# event handlers
    connect(ui->createButton, &QPushButton::clicked, this, &NewOtbFileDialog::onCreateClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &NewOtbFileDialog::onCancelClicked);
    connect(ui->clientVersionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &NewOtbFileDialog::onClientVersionChanged);
}

NewOtbFileDialog::~NewOtbFileDialog()
{
    delete ui;
}

void NewOtbFileDialog::setupUi()
{
    // Generate temporary file path - exact mirror of C# filePath generation
    QTemporaryFile tempFile;
    if (tempFile.open()) {
        filePath = tempFile.fileName();
        tempFile.close();
    } else {
        filePath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/temp.otb";
    }
    
    // Configure combo box - exact mirror of C# clientVersionComboBox setup
    ui->clientVersionComboBox->setEnabled(true);
    
    // Initially disable create button - exact mirror of C# createButton.Enabled = false
    ui->createButton->setEnabled(false);
    
    // Center the dialog on parent
    if (parentWidget()) {
        move(parentWidget()->geometry().center() - rect().center());
    }
}

void NewOtbFileDialog::loadClientVersions()
{
    // Load supported clients from plugin system
    // Exact mirror of C# NewOtbFileForm_Load event handler
    
    QList<SupportedClient> clientList;

    // Get available plugins from PluginServices with enhanced error handling
    PluginServices* pluginServices = PluginServices::getInstance();
    if (!pluginServices) {
        qDebug() << "NewOtbFileForm: PluginServices instance not available";
        // Show fallback message and disable create functionality
        ui->clientVersionComboBox->clear();
        ui->clientVersionComboBox->addItem("Plugin system not initialized", QVariant::fromValue(SupportedClient()));
        ui->clientVersionComboBox->setCurrentIndex(0);
        updateCreateButtonState();
        return;
    }
    
    PluginCollection* plugins = pluginServices->availablePlugins();
    if (!plugins || plugins->isEmpty()) {
        qDebug() << "NewOtbFileForm: No plugins available in PluginServices";
        // Show message indicating no plugins are loaded
        ui->clientVersionComboBox->clear();
        ui->clientVersionComboBox->addItem("No plugins loaded", QVariant::fromValue(SupportedClient()));
        ui->clientVersionComboBox->setCurrentIndex(0);
        updateCreateButtonState();
        return;
    }
    
    qDebug() << QString("NewOtbFileForm: Found %1 available plugins").arg(plugins->size());
    
    // Iterate through all loaded plugins and collect their supported clients
    // Exact mirror of C# foreach (Plugin plugin in Program.plugins.AvailablePlugins)
    int totalClients = 0;
    for (auto it = plugins->constBegin(); it != plugins->constEnd(); ++it) {
        Plugin* plugin = *it;
        if (!plugin) {
            qDebug() << "NewOtbFileForm: Null plugin found in collection";
            continue;
        }
        
        if (!plugin->isLoaded()) {
            qDebug() << QString("NewOtbFileForm: Plugin %1 is not loaded").arg(plugin->name());
            continue;
        }
        
        if (!plugin->instance()) {
            qDebug() << QString("NewOtbFileForm: Plugin %1 has no instance").arg(plugin->name());
            continue;
        }
        
        // Get supported clients from this plugin
        // Exact mirror of C# foreach (SupportedClient client in plugin.Instance.SupportedClients)
        QList<SupportedClient> supportedClients = plugin->instance()->supportedClients();
        qDebug() << QString("NewOtbFileForm: Plugin %1 provides %2 supported clients")
                    .arg(plugin->name()).arg(supportedClients.size());
        
        for (const auto& client : supportedClients) {
            // Validate client data before adding
            if (!client.getName().isEmpty() && client.getOtbVersion() > 0) {
                clientList.append(client);
                totalClients++;
            } else {
                qDebug() << QString("NewOtbFileForm: Invalid client data from plugin %1: name='%2', version=%3")
                            .arg(plugin->name()).arg(client.getName()).arg(client.getOtbVersion());
            }
        }
    }
    
    qDebug() << QString("NewOtbFileForm: Collected %1 valid clients from all plugins").arg(totalClients);

    if (!clientList.isEmpty()) {
        // Sort by OTB version - exact mirror of C# OrderBy(i => i.OtbVersion)
        std::sort(clientList.begin(), clientList.end(), 
                  [](const SupportedClient& a, const SupportedClient& b) {
                      return a.getOtbVersion() < b.getOtbVersion();
                  });

        // Populate combo box
        ui->clientVersionComboBox->clear();
        for (const auto& client : clientList) {
            QString displayText = QString("%1 (v%2)").arg(client.getName()).arg(client.getOtbVersion());
            ui->clientVersionComboBox->addItem(displayText, QVariant::fromValue(client));
        }
        
        // Select the latest version (last item) - exact mirror of C# SelectedIndex = list.Count - 1
        ui->clientVersionComboBox->setCurrentIndex(clientList.size() - 1);
        
        qDebug() << QString("NewOtbFileForm: Successfully populated combo box with %1 clients, selected latest version")
                    .arg(clientList.size());
    } else {
        // Fallback when no valid clients found
        qDebug() << "NewOtbFileForm: No valid clients found from any plugin";
        ui->clientVersionComboBox->clear();
        ui->clientVersionComboBox->addItem("No valid clients available", QVariant::fromValue(SupportedClient()));
        ui->clientVersionComboBox->setCurrentIndex(0);
    }
    
    updateCreateButtonState();
}

void NewOtbFileDialog::updateCreateButtonState()
{
    // Exact mirror of C# createButton.Enabled logic with enhanced validation
    bool hasFilePath = !filePath.isEmpty();
    bool hasSelectedClient = ui->clientVersionComboBox->currentIndex() >= 0;
    
    // Additional validation: check if the selected client is valid
    bool hasValidClient = false;
    if (hasSelectedClient) {
        QVariant clientData = ui->clientVersionComboBox->currentData();
        if (clientData.isValid() && clientData.canConvert<SupportedClient>()) {
            SupportedClient client = clientData.value<SupportedClient>();
            hasValidClient = !client.getName().isEmpty() && 
                           client.getOtbVersion() > 0 &&
                           client.getDatSignature() > 0 &&
                           client.getSprSignature() > 0;
        }
    }
    
    bool enableCreate = hasFilePath && hasSelectedClient && hasValidClient;
    ui->createButton->setEnabled(enableCreate);
    
    // Debug logging for troubleshooting
    if (!enableCreate) {
        qDebug() << QString("NewOtbFileForm: Create button disabled - filePath: %1, selectedClient: %2, validClient: %3")
                    .arg(hasFilePath).arg(hasSelectedClient).arg(hasValidClient);
    }
}

void NewOtbFileDialog::onCreateClicked()
{
    // Exact mirror of C# CreateButton_Click event handler with enhanced validation
    if (ui->clientVersionComboBox->currentIndex() >= 0) {
        // Get selected client from combo box data
        QVariant clientData = ui->clientVersionComboBox->currentData();
        
        // Validate client data exists and is convertible
        if (!clientData.isValid() || !clientData.canConvert<SupportedClient>()) {
            QMessageBox::warning(this, tr("Error"), 
                                tr("Invalid client data. Please select a valid client version."));
            return;
        }
        
        selectedClient = clientData.value<SupportedClient>();
        
        // Comprehensive validation - check if we have a valid client with proper data
        if (selectedClient.getName().isEmpty() || 
            selectedClient.getOtbVersion() <= 0 ||
            selectedClient.getDatSignature() <= 0 ||
            selectedClient.getSprSignature() <= 0) {
            
            QMessageBox::warning(this, tr("Error"), 
                                tr("Please select a valid client version with complete signature data."));
            qDebug() << QString("NewOtbFileForm: Invalid client selected - name: '%1', otbVersion: %2, datSig: %3, sprSig: %4")
                        .arg(selectedClient.getName())
                        .arg(selectedClient.getOtbVersion())
                        .arg(selectedClient.getDatSignature(), 0, 16)
                        .arg(selectedClient.getSprSignature(), 0, 16);
            return;
        }
        
        // Validate file path
        if (filePath.isEmpty()) {
            QMessageBox::warning(this, tr("Error"), 
                                tr("Invalid file path generated."));
            return;
        }
        
        qDebug() << QString("NewOtbFileForm: Creating OTB file with client '%1' (v%2) at path: %3")
                    .arg(selectedClient.getName())
                    .arg(selectedClient.getOtbVersion())
                    .arg(filePath);
        
        accept(); // Equivalent to this.DialogResult = DialogResult.OK; this.Close();
    } else {
        QMessageBox::warning(this, tr("Error"), 
                            tr("No client version selected."));
    }
}

void NewOtbFileDialog::onCancelClicked()
{
    // Exact mirror of C# CancelButton_Click event handler
    reject(); // Equivalent to this.Close() with cancel result
}

void NewOtbFileDialog::onClientVersionChanged()
{
    // Exact mirror of C# ClientVersionComboBox_SelectedIndexChanged event handler
    updateCreateButtonState();
}

} // namespace ItemEditor