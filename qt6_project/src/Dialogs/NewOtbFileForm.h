/**
 * Item Editor Qt6 - New OTB File Dialog Header
 * Exact mirror of Legacy_App/csharp/Source/Dialogs/NewOtbFileForm.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_NEWOTBFILEFORM_H
#define ITEMEDITOR_NEWOTBFILEFORM_H

#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QString>
#include "../PluginInterface/SupportedClient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class NewOtbFileDialog; }
QT_END_NAMESPACE

namespace ItemEditor {

/**
 * New OTB File Dialog Class
 * Exact mirror of C# NewOtbFileForm : DarkForm
 * 
 * Allows user to create a new OTB file by selecting client version
 */
class NewOtbFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewOtbFileDialog(QWidget *parent = nullptr);
    ~NewOtbFileDialog();

    // Public properties - exact mirror of C# properties
    QString getFilePath() const { return filePath; }
    SupportedClient getSelectedClient() const { return selectedClient; }

private slots:
    void onCreateClicked();
    void onCancelClicked();
    void onClientVersionChanged();

private:
    void setupUi();
    void loadClientVersions();
    void updateCreateButtonState();

private:
    Ui::NewOtbFileDialog *ui;
    
    // Private properties - exact mirror of C# private properties
    QString filePath;
    SupportedClient selectedClient;
};

} // namespace ItemEditor

#endif // ITEMEDITOR_NEWOTBFILEFORM_H