/**
 * Item Editor Qt6 - Compare OTB Form Header
 * Exact mirror of Legacy_App/csharp/Source/Dialogs/CompareOtbForm.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_DIALOGS_COMPAREOTBFORM_H
#define ITEMEDITOR_DIALOGS_COMPAREOTBFORM_H

#include <QDialog>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "../PluginInterface/OTLib/Collections/ServerItemList.h"
#include "../PluginInterface/OTLib/Server/Items/ServerItem.h"

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
QT_END_NAMESPACE

namespace Ui {
class CompareOtbForm;
}

namespace ItemEditor {
namespace Dialogs {

/**
 * Compare OTB Form Dialog
 * Exact mirror of C# CompareOtbForm : DarkForm
 * 
 * Provides functionality to compare two OTB files and display detailed differences
 * including item count differences, sprite changes, and property modifications
 */
class CompareOtbForm : public QDialog
{
    Q_OBJECT

public:
    explicit CompareOtbForm(QWidget *parent = nullptr);
    ~CompareOtbForm();

private slots:
    // Event handlers - exact mirror of C# event handlers
    void onBrowseFile1();
    void onBrowseFile2();
    void onCompare();
    void onFileTextChanged();

private:
    // Private methods - exact mirror of C# private methods
    bool compareItems();
    void setupUI();
    void connectSignals();
    
    // UI components - exact mirror of C# designer components
    Ui::CompareOtbForm *ui;
    
    // File paths
    QString m_file1Path;
    QString m_file2Path;
    
    // UI elements (for programmatic creation if needed)
    QTextEdit *m_resultTextBox;
    QLineEdit *m_file1Text;
    QLineEdit *m_file2Text;
    QPushButton *m_browseButton1;
    QPushButton *m_browseButton2;
    QPushButton *m_compareButton;
    QLabel *m_label1;
    QLabel *m_label2;
};

} // namespace Dialogs
} // namespace ItemEditor

#endif // ITEMEDITOR_DIALOGS_COMPAREOTBFORM_H