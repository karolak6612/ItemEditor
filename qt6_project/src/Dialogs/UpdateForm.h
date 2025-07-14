/**
 * Item Editor Qt6 - Update Dialog Header
 * Update management dialog for application updates
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_UPDATEFORM_H
#define ITEMEDITOR_UPDATEFORM_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QString>
#include <QProgressBar>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QApplication>

QT_BEGIN_NAMESPACE
namespace Ui { class UpdateDialog; }
QT_END_NAMESPACE

namespace ItemEditor {

/**
 * Update Dialog Class
 * Manages application updates and displays update information
 * 
 * Provides update checking, downloading, and installation functionality
 */
class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateDialog(QWidget *parent = nullptr);
    ~UpdateDialog();

    // Update information methods
    void setCurrentVersion(const QString &version);
    void setAvailableVersion(const QString &version);
    void setUpdateNotes(const QString &notes);
    void setUpdateUrl(const QString &url);
    
    // Update process control
    void setUpdateAvailable(bool available);
    void setDownloadProgress(int progress);
    void showDownloadProgress(bool show);

public slots:
    void checkForUpdates();
    void downloadUpdate();
    void installUpdate();

signals:
    void updateRequested();
    void downloadRequested();
    void installRequested();

private slots:
    void onUpdateClicked();
    void onCancelClicked();
    void onCheckUpdatesClicked();

private:
    void setupUi();
    void updateButtonStates();
    bool isNewerVersion(const QString& available, const QString& current);

private:
    Ui::UpdateDialog *ui;
    
    QString currentVersion;
    QString availableVersion;
    QString updateUrl;
    bool updateAvailable;
    
    // Network components for update functionality
    QNetworkAccessManager *manager;
    QNetworkReply *downloadReply;
};

} // namespace ItemEditor

#endif // ITEMEDITOR_UPDATEFORM_H