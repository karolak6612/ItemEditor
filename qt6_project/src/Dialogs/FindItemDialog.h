#pragma once

#include <QDialog>
#include "../PluginInterface/OTLib/Server/Items/ServerItem.h"

class MainForm;

namespace Ui {
class FindItemDialog;
}

class FindItemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindItemDialog(QWidget *parent = nullptr);
    ~FindItemDialog();

    void setMainForm(MainForm *mainForm);

private slots:
    void on_findBySidButton_toggled(bool checked);
    void on_findByCidButton_toggled(bool checked);
    void on_findByPropertiesButton_toggled(bool checked);
    void on_findItemButton_clicked();
    void on_findIdNumericUpDown_editingFinished();
    void on_serverItemList_itemClicked(QListWidgetItem *item);
    void on_propertyCheckBox_stateChanged(int arg1);
    void mainForm_CleanedHandler();


private:
    void updateProperties();
    void startFind();

    Ui::FindItemDialog *ui;
    MainForm *m_mainForm;
    ServerItemFlag m_properties;
};
