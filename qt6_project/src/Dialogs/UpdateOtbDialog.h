/**
 * Item Editor Qt6 - Update OTB Dialog Header
 *
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_UPDATEOTBDIALOG_H
#define ITEMEDITOR_UPDATEOTBDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include "../PluginInterface/SupportedClient.h"
#include "../Host/PluginServices.h"
#include "../Host/Plugin.h"

QT_BEGIN_NAMESPACE
namespace Ui { class UpdateOtbDialog; }
QT_END_NAMESPACE

class MainForm;

namespace ItemEditor {

class UpdateOtbDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateOtbDialog(MainForm* mainForm, PluginServices* pluginServices, QWidget *parent = nullptr);
    ~UpdateOtbDialog();

    Plugin* getSelectedPlugin() const;
    SupportedClient getSelectedClient() const;

private slots:
    void onSelectClicked();
    void onCancelClicked();
    void onClientVersionChanged();

private:
    void setupUi();
    void populateClientVersions();

    Ui::UpdateOtbDialog *ui;
    MainForm* m_mainForm;
    PluginServices* m_pluginServices;
    Plugin* m_selectedPlugin;
    SupportedClient m_selectedClient;

    QListWidget* m_pluginsListBox;
    QPushButton* m_selectButton;
    QPushButton* m_cancelButton;
};

} // namespace ItemEditor

#endif // ITEMEDITOR_UPDATEOTBDIALOG_H
