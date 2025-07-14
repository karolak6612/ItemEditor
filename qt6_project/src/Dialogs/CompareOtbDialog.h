#pragma once

#include <QDialog>

namespace Ui {
class CompareOtbDialog;
}

class CompareOtbDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CompareOtbDialog(QWidget *parent = nullptr);
    ~CompareOtbDialog();

private slots:
    void on_browseButton1_clicked();
    void on_browseButton2_clicked();
    void on_compareButton_clicked();
    void on_file1Text_textChanged(const QString &arg1);
    void on_file2Text_textChanged(const QString &arg1);

private:
    bool compareItems();

    Ui::CompareOtbDialog *ui;
};
