/**
 * Item Editor Qt6 - New OTB File Dialog Header
 * Exact mirror of Legacy_App/csharp/Source/Dialogs/NewOtbFileForm.cs
 *
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_NEWOTBFILEDIALOG_H
#define ITEMEDITOR_NEWOTBFILEDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QString>
#include "../PluginInterface/SupportedClient.h"
#include "../Host/PluginServices.h"

QT_BEGIN_NAMESPACE
namespace Ui { class NewOtbFileDialog; }
QT_END_NAMESPACE

namespace ItemEditor {

class NewOtbFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewOtbFileDialog(PluginServices* pluginServices, QWidget *parent = nullptr);
    ~NewOtbFileDialog();

    QString getFilePath() const;
    SupportedClient getSelectedClient() const;

private slots:
    void onCreateClicked();
    void onCancelClicked();
    void onClientVersionChanged(int index);

private:
    void setupUi();
    void populateClientVersions();

    Ui::NewOtbFileDialog *ui;
    PluginServices* m_pluginServices;
    QString m_filePath;
    SupportedClient m_selectedClient;

    QComboBox* m_clientVersionComboBox;
    QPushButton* m_createButton;
    QPushButton* m_cancelButton;
};

} // namespace ItemEditor

#endif // ITEMEDITOR_NEWOTBFILEDIALOG_H
