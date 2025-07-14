#pragma once

#include <QDialog>
#include "../Host/Plugin.h"
#include "../PluginInterface/PluginInterface.h"

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();

    Plugin* getPlugin() const;
    SupportedClient getClient() const;

private slots:
    void on_directoryPathTextBox_textChanged(const QString &arg1);
    void on_browseButton_clicked();
    void on_confirmButton_clicked();
    void on_cancelButton_clicked();

private:
    void onSelectFiles(const QString &directory);
    quint32 getSignature(const QString &fileName);
    void clear();

    Ui::PreferencesDialog *ui;
    quint32 m_datSignature;
    quint32 m_sprSignature;
    Plugin* m_plugin;
    SupportedClient m_client;
};
