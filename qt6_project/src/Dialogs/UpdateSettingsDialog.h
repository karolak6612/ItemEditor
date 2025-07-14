#pragma once

#include <QDialog>

namespace Ui {
class UpdateSettingsDialog;
}

class UpdateSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateSettingsDialog(QWidget *parent = nullptr);
    ~UpdateSettingsDialog();

    bool reassignUnmatchedSprites() const;
    bool reloadItemAttributes() const;
    bool createNewItems() const;
    bool generateSignature() const;

private slots:
    void on_closeBtn_clicked();

private:
    Ui::UpdateSettingsDialog *ui;
};
