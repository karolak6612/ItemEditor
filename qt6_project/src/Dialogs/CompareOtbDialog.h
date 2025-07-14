/**
 * Item Editor Qt6 - Compare OTB Dialog Header
 *
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_COMPAREOTBDIALOG_H
#define ITEMEDITOR_COMPAREOTBDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class CompareOtbDialog; }
QT_END_NAMESPACE

namespace ItemEditor {

class CompareOtbDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CompareOtbDialog(QWidget *parent = nullptr);
    ~CompareOtbDialog();

private slots:
    void onBrowse1();
    void onBrowse2();
    void onCompare();
    void onTextChanged();

private:
    void setupUi();
    bool compareItems();

    Ui::CompareOtbDialog *ui;

    QLineEdit* m_file1Text;
    QLineEdit* m_file2Text;
    QPushButton* m_browse1Button;
    QPushButton* m_browse2Button;
    QPushButton* m_compareButton;
    QTextEdit* m_resultText;
};

} // namespace ItemEditor

#endif // ITEMEDITOR_COMPAREOTBDIALOG_H
