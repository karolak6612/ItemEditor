/**
 * Item Editor Qt6 - About Dialog Header
 * Exact mirror of Legacy_App/csharp/Source/Dialogs/AboutForm.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_ABOUTFORM_H
#define ITEMEDITOR_ABOUTFORM_H

#include <QDialog>
#include <QLabel>
#include <QDialogButtonBox>

QT_BEGIN_NAMESPACE
namespace Ui { class AboutDialog; }
QT_END_NAMESPACE

namespace ItemEditor {

/**
 * About Dialog Class
 * Exact mirror of C# AboutForm : Form
 * 
 * Displays application information including version, copyright, and website
 */
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

private slots:
    void onOkClicked();

private:
    void setupUi();
    void updateVersionInfo();
    void updateApplicationInfo();

private:
    Ui::AboutDialog *ui;
};

} // namespace ItemEditor

#endif // ITEMEDITOR_ABOUTFORM_H