#pragma once

#include <QDialog>
#include "../Host/Plugin.h"
#include "../PluginInterface/PluginInterface.h"

class MainForm;

namespace Ui {
class UpdateDialog;
}

class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateDialog(QWidget *parent = nullptr);
    ~UpdateDialog();

    void setMainForm(MainForm *mainForm);
    Plugin* getSelectedPlugin() const;
    SupportedClient getUpdateClient() const;

private slots:
    void on_pluginsListBox_itemSelectionChanged();
    void on_selectBtn_clicked();

private:
    void loadPlugins();

    Ui::UpdateDialog *ui;
    MainForm *m_mainForm;
    Plugin *m_selectedPlugin;
    SupportedClient m_updateClient;
};
