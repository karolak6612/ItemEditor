#include "PreferencesDialog.h"
#include "ui_PreferencesDialog.h"
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include "../Helpers/Utils.h"
#include "../Host/PluginCollection.h"

extern PluginCollection g_plugins;

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog),
    m_datSignature(0),
    m_sprSignature(0),
    m_plugin(nullptr)
{
    ui->setupUi(this);

    QSettings settings;
    ui->directoryPathTextBox->setText(settings.value("ClientDirectory").toString());
    ui->extendedCheckBox->setChecked(settings.value("Extended").toBool());
    ui->frameDurationsCheckBox->setChecked(settings.value("FrameDurations").toBool());
    ui->transparencyCheckBox->setChecked(settings.value("Transparency").toBool());
    m_datSignature = settings.value("DatSignature").toUInt();
    m_sprSignature = settings.value("SprSignature").toUInt();

    onSelectFiles(ui->directoryPathTextBox->text());
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

Plugin* PreferencesDialog::getPlugin() const
{
    return m_plugin;
}

SupportedClient PreferencesDialog::getClient() const
{
    return m_client;
}

void PreferencesDialog::on_directoryPathTextBox_textChanged(const QString &arg1)
{
    onSelectFiles(arg1);
}

void PreferencesDialog::on_browseButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Client Directory"));
    if (!dir.isEmpty()) {
        onSelectFiles(dir);
    }
}

void PreferencesDialog::on_confirmButton_clicked()
{
    QSettings settings;
    settings.setValue("ClientDirectory", ui->directoryPathTextBox->text());
    settings.setValue("Extended", ui->extendedCheckBox->isChecked());
    settings.setValue("FrameDurations", ui->frameDurationsCheckBox->isChecked());
    settings.setValue("Transparency", ui->transparencyCheckBox->isChecked());
    settings.setValue("DatSignature", m_datSignature);
    settings.setValue("SprSignature", m_sprSignature);

    setResult(QDialog::Accepted);
    close();
}

void PreferencesDialog::on_cancelButton_clicked()
{
    close();
}

void PreferencesDialog::onSelectFiles(const QString &directory)
{
    ui->alertLabel->setText("");

    if (directory.isEmpty() || !QDir(directory).exists())
    {
        ui->alertLabel->setText("Directory not found");
        return;
    }

    QString datPath = Utils::findClientFile(directory, ".dat");
    QString sprPath = Utils::findClientFile(directory, ".spr");

    if (!QFile::exists(datPath) || !QFile::exists(sprPath))
    {
        ui->alertLabel->setText("Client files not found");
        return;
    }

    quint32 datSignature = getSignature(datPath);
    quint32 sprSignature = getSignature(sprPath);

    m_plugin = g_plugins.find(datSignature, sprSignature);
    if (m_plugin == nullptr)
    {
        ui->alertLabel->setText(QString("Unsupported version\nDat Signature: %1\nSpr Signature: %2").arg(datSignature, 8, 16, QLatin1Char('0')).arg(sprSignature, 8, 16, QLatin1Char('0')));
        return;
    }

    m_client = m_plugin->getInstance()->getClientBySignatures(datSignature, sprSignature);
    ui->extendedCheckBox->setChecked(ui->extendedCheckBox->isChecked() || m_client.version >= 960);
    ui->extendedCheckBox->setEnabled(m_client.version < 960);
    ui->frameDurationsCheckBox->setChecked(ui->frameDurationsCheckBox->isChecked() || m_client.version >= 1050);
    ui->frameDurationsCheckBox->setEnabled(m_client.version < 1050);
    m_datSignature = datSignature;
    m_sprSignature = sprSignature;
    ui->directoryPathTextBox->setText(directory);
}

quint32 PreferencesDialog::getSignature(const QString &fileName)
{
    quint32 signature = 0;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        QDataStream in(&file);
        in >> signature;
        file.close();
    }
    return signature;
}

void PreferencesDialog::clear()
{
    ui->directoryPathTextBox->clear();
    ui->extendedCheckBox->setChecked(false);
    ui->frameDurationsCheckBox->setChecked(false);
    ui->transparencyCheckBox->setChecked(false);
    m_datSignature = 0;
    m_sprSignature = 0;
}
