/**
 * Item Editor Qt6 - Find Item Dialog Header
 * Exact mirror of Legacy_App/csharp/Source/Dialogs/FindItemForm.cs
 *
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_FINDITEMDIALOG_H
#define ITEMEDITOR_FINDITEMDIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QGroupBox>
#include <QSpinBox>
#include <QPushButton>

#include "Controls/ServerItemListBox.h"
#include "Controls/FlagCheckBox.h"
#include "PluginInterface/OTLib/Server/Items/ServerItem.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FindItemDialog; }
QT_END_NAMESPACE

class MainForm;

namespace ItemEditor {

class FindItemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindItemDialog(MainForm* mainForm, QWidget *parent = nullptr);
    ~FindItemDialog();

private slots:
    void onSearchModeChanged();
    void onFindClicked();
    void onPropertyCheckBoxChanged();
    void onServerItemSelected();

private:
    void setupUi();
    void updateProperties();

    Ui::FindItemDialog *ui;
    MainForm* m_mainForm;

    QGroupBox* m_searchModeGroupBox;
    QRadioButton* m_findBySidRadioButton;
    QRadioButton* m_findByCidRadioButton;
    QRadioButton* m_findByPropertiesRadioButton;

    QGroupBox* m_itemIdGroupBox;
    QSpinBox* m_findIdSpinBox;

    QGroupBox* m_propertiesGroupBox;
    QList<FlagCheckBox*> m_flagCheckBoxes;

    QGroupBox* m_resultGroupBox;
    ServerItemListBox* m_serverItemList;

    QPushButton* m_findButton;

    OTLib::Server::Items::ServerItemFlag m_properties;
};

} // namespace ItemEditor

#endif // ITEMEDITOR_FINDITEMDIALOG_H
