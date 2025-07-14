#include "UpdateDialog.h"
#include "ui_UpdateDialog.h"
#include "../MainForm.h"
#include "../Host/PluginCollection.h"

extern PluginCollection g_plugins;

UpdateDialog::UpdateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDialog),
    m_mainForm(nullptr),
    m_selectedPlugin(nullptr)
{
    ui->setupUi(this);
    loadPlugins();
}

UpdateDialog::~UpdateDialog()
{
    delete ui;
}

void UpdateDialog::setMainForm(MainForm *mainForm)
{
    m_mainForm = mainForm;
}

Plugin* UpdateDialog::getSelectedPlugin() const
{
    return m_selectedPlugin;
}

SupportedClient UpdateDialog::getUpdateClient() const
{
    return m_updateClient;
}

void UpdateDialog::on_pluginsListBox_itemSelectionChanged()
{
    if (ui->pluginsListBox->currentItem() != nullptr && m_mainForm != nullptr /*&& m_mainForm->getCurrentPlugin() != nullptr*/)
    {
        ui->selectBtn->setEnabled(true);
    }
}

void UpdateDialog::on_selectBtn_clicked()
{
    if (ui->pluginsListBox->currentItem() != nullptr)
    {
        m_updateClient = ui->pluginsListBox->currentItem()->data(Qt::UserRole).value<SupportedClient>();
        m_selectedPlugin = g_plugins.find(m_updateClient.clientVersion);
        setResult(QDialog::Accepted);
        close();
    }
}

void UpdateDialog::loadPlugins()
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
            QListWidgetItem *item = new QListWidgetItem(client.clientVersion);
            item->setData(Qt::UserRole, QVariant::fromValue(client));
            ui->pluginsListBox->addItem(item);
        }
        ui->pluginsListBox->setCurrentRow(list.count() - 1);
    }
}
