#include "NewOtbFileDialog.h"
#include "ui_NewOtbFileDialog.h"
#include <QTemporaryDir>
#include "../Host/PluginCollection.h"
#include "../Host/Plugin.h"

extern PluginCollection g_plugins;

NewOtbFileDialog::NewOtbFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewOtbFileDialog)
{
    ui->setupUi(this);
    loadPlugins();
}

NewOtbFileDialog::~NewOtbFileDialog()
{
    delete ui;
}

QString NewOtbFileDialog::getFilePath() const
{
    return m_filePath;
}

SupportedClient NewOtbFileDialog::getSelectedClient() const
{
    return m_selectedClient;
}

void NewOtbFileDialog::on_clientVersionComboBox_currentIndexChanged(int index)
{
    ui->createButton->setEnabled(!m_filePath.isEmpty() && ui->clientVersionComboBox->currentIndex() != -1);
}

void NewOtbFileDialog::on_createButton_clicked()
{
    if (ui->clientVersionComboBox->currentIndex() != -1)
    {
        m_selectedClient = ui->clientVersionComboBox->currentData().value<SupportedClient>();
        QTemporaryDir dir;
        if (dir.isValid()) {
            m_filePath = dir.path() + "/new.otb";
        }
        setResult(QDialog::Accepted);
        close();
    }
}

void NewOtbFileDialog::on_cancelButton_clicked()
{
    close();
}

void NewOtbFileDialog::loadPlugins()
{
    QList<SupportedClient> list;

    for (const auto& plugin : g_plugins.getAvailablePlugins())
    {
        for (const auto& client : plugin->getInstance()->supportedClients())
        {
            list.append(client);
        }
    }

    if (!list.isEmpty())
    {
        std::sort(list.begin(), list.end(), [](const SupportedClient &a, const SupportedClient &b) {
            return a.otbVersion < b.otbVersion;
        });

        for (const auto& client : list) {
            ui->clientVersionComboBox->addItem(client.clientVersion, QVariant::fromValue(client));
        }
        ui->clientVersionComboBox->setCurrentIndex(list.count() - 1);
    }
}
