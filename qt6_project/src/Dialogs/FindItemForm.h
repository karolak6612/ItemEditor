/**
 * Item Editor Qt6 - Find Item Dialog Header
 * Exact mirror of Legacy_App/csharp/Source/Dialogs/FindItemForm.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_FINDITEMFORM_H
#define ITEMEDITOR_FINDITEMFORM_H

#include <QDialog>
#include <QRadioButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QDialogButtonBox>

QT_BEGIN_NAMESPACE
namespace Ui { class FindItemDialog; }
QT_END_NAMESPACE

namespace ItemEditor {

/**
 * Find Item Dialog Class
 * Exact mirror of C# FindItemForm : Form
 * 
 * Allows searching for items by ID or name
 */
class FindItemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindItemDialog(QWidget *parent = nullptr);
    ~FindItemDialog();

    // Search criteria - exact mirror of C# properties
    bool searchById() const;
    quint16 itemId() const;
    QString itemName() const;

private slots:
    void onSearchModeChanged();
    void onOkClicked();
    void onCancelClicked();

private:
    void setupUi();
    void updateControlStates();

private:
    Ui::FindItemDialog *ui;
};

} // namespace ItemEditor

#endif // ITEMEDITOR_FINDITEMFORM_H