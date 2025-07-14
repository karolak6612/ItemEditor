#pragma once

#include <QDialog>

namespace Ui {
class ProgressDialog;
}

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = nullptr);
    ~ProgressDialog();

    void setProgress(int value);
    void setLabelText(const QString &text);

private:
    Ui::ProgressDialog *ui;
};
