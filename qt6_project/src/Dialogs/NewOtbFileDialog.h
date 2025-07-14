#pragma once

#include <QDialog>
#include "../PluginInterface/PluginInterface.h"

namespace Ui {
class NewOtbFileDialog;
}

class NewOtbFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewOtbFileDialog(QWidget *parent = nullptr);
    ~NewOtbFileDialog();

    QString getFilePath() const;
    SupportedClient getSelectedClient() const;

private slots:
    void on_clientVersionComboBox_currentIndexChanged(int index);
    void on_createButton_clicked();
    void on_cancelButton_clicked();

private:
    void loadPlugins();

    Ui::NewOtbFileDialog *ui;
    QString m_filePath;
    SupportedClient m_selectedClient;
};
