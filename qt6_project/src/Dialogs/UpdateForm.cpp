/**
 * Item Editor Qt6 - Update Dialog Implementation
 * Update management dialog for application updates
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "UpdateForm.h"
#include "ui_UpdateForm.h"
#include "../Properties/version.h"

#include <QApplication>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QProcess>

namespace ItemEditor {

UpdateDialog::UpdateDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UpdateDialog)
    , updateAvailable(false)
{
    ui->setupUi(this);
    
    // Setup dialog properties
    setModal(true);
    setFixedSize(500, 400);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Check for Updates"));
    
    // Setup UI components
    setupUi();
    
    // Connect signals
    connect(ui->updateButton, &QPushButton::clicked, this, &UpdateDialog::onUpdateClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &UpdateDialog::onCancelClicked);
    connect(ui->checkUpdatesButton, &QPushButton::clicked, this, &UpdateDialog::onCheckUpdatesClicked);
}

UpdateDialog::~UpdateDialog()
{
    delete ui;
}

void UpdateDialog::setupUi()
{
    // Set current version
    currentVersion = Version::getVersionString();
    ui->currentVersionLabel->setText(tr("Current Version: %1").arg(currentVersion));
    
    // Initialize available version
    ui->availableVersionLabel->setText(tr("Available Version: Checking..."));
    
    // Configure update notes
    ui->updateNotesEdit->setReadOnly(true);
    ui->updateNotesEdit->setPlainText(tr("Click 'Check for Updates' to see if a new version is available."));
    
    // Configure progress bar
    ui->downloadProgressBar->setVisible(false);
    ui->downloadProgressBar->setMinimum(0);
    ui->downloadProgressBar->setMaximum(100);
    ui->downloadProgressBar->setValue(0);
    
    // Configure buttons
    ui->updateButton->setText(tr("Download Update"));
    ui->updateButton->setEnabled(false);
    ui->cancelButton->setText(tr("Close"));
    ui->checkUpdatesButton->setText(tr("Check for Updates"));
    
    // Set initial state
    updateAvailable = false;
    updateButtonStates();
    
    // Center the dialog on parent
    if (parentWidget()) {
        move(parentWidget()->geometry().center() - rect().center());
    }
}

void UpdateDialog::setCurrentVersion(const QString &version)
{
    currentVersion = version;
    ui->currentVersionLabel->setText(tr("Current Version: %1").arg(currentVersion));
}

void UpdateDialog::setAvailableVersion(const QString &version)
{
    availableVersion = version;
    ui->availableVersionLabel->setText(tr("Available Version: %1").arg(availableVersion));
}

void UpdateDialog::setUpdateNotes(const QString &notes)
{
    ui->updateNotesEdit->setPlainText(notes);
}

void UpdateDialog::setUpdateUrl(const QString &url)
{
    updateUrl = url;
}

void UpdateDialog::setUpdateAvailable(bool available)
{
    updateAvailable = available;
    updateButtonStates();
    
    if (available) {
        ui->updateNotesEdit->setPlainText(tr("A new version is available!\n\nClick 'Download Update' to get the latest version."));
    } else {
        ui->updateNotesEdit->setPlainText(tr("You are using the latest version."));
        ui->availableVersionLabel->setText(tr("Available Version: %1 (Latest)").arg(currentVersion));
    }
}

void UpdateDialog::setDownloadProgress(int progress)
{
    ui->downloadProgressBar->setValue(progress);
}

void UpdateDialog::showDownloadProgress(bool show)
{
    ui->downloadProgressBar->setVisible(show);
}

void UpdateDialog::updateButtonStates()
{
    ui->updateButton->setEnabled(updateAvailable);
    ui->checkUpdatesButton->setEnabled(true);
}

void UpdateDialog::checkForUpdates()
{
    // Implement actual update checking logic
    ui->availableVersionLabel->setText(tr("Available Version: Checking..."));
    ui->updateNotesEdit->setPlainText(tr("Checking for updates..."));
    ui->checkUpdatesButton->setEnabled(false);
    
    // Create network manager for update checking
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    
    // Connect to finished signal
    connect(manager, &QNetworkAccessManager::finished, this, [this, manager](QNetworkReply* reply) {
        manager->deleteLater();
        
        if (reply->error() == QNetworkReply::NoError) {
            // Parse response for version information
            QByteArray data = reply->readAll();
            QString response = QString::fromUtf8(data);
            
            // Simple version parsing (would be more sophisticated in real implementation)
            QRegularExpression versionRegex("\"tag_name\":\\s*\"v?([^\"]+)\"");
            QRegularExpressionMatch match = versionRegex.match(response);
            
            if (match.hasMatch()) {
                QString latestVersion = match.captured(1);
                setAvailableVersion(latestVersion);
                
                // Compare versions
                if (isNewerVersion(latestVersion, currentVersion)) {
                    setUpdateAvailable(true);
                    setUpdateUrl("https://github.com/ottools/ItemEditor/releases/latest");
                    
                    // Extract release notes if available
                    QRegularExpression notesRegex("\"body\":\\s*\"([^\"]*)\"");
                    QRegularExpressionMatch notesMatch = notesRegex.match(response);
                    if (notesMatch.hasMatch()) {
                        QString notes = notesMatch.captured(1);
                        notes.replace("\\n", "\n").replace("\\r", "");
                        setUpdateNotes(tr("Release Notes:\n\n%1").arg(notes));
                    }
                } else {
                    setUpdateAvailable(false);
                }
            } else {
                setUpdateAvailable(false);
                ui->updateNotesEdit->setPlainText(tr("Could not parse version information."));
            }
        } else {
            setUpdateAvailable(false);
            ui->updateNotesEdit->setPlainText(tr("Failed to check for updates: %1").arg(reply->errorString()));
        }
        
        ui->checkUpdatesButton->setEnabled(true);
        reply->deleteLater();
    });
    
    // Make request to GitHub API for latest release
    QNetworkRequest request(QUrl("https://api.github.com/repos/ottools/ItemEditor/releases/latest"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "ItemEditor-Qt6");
    manager->get(request);
    
    emit updateRequested();
}

void UpdateDialog::downloadUpdate()
{
    if (!updateUrl.isEmpty()) {
        // Open the download URL in the default browser
        QDesktopServices::openUrl(QUrl(updateUrl));
    }
    
    emit downloadRequested();
}

void UpdateDialog::installUpdate()
{
    // Implement update installation logic
    if (updateUrl.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("No update URL available."));
        return;
    }
    
    // Show download progress
    showDownloadProgress(true);
    setDownloadProgress(0);
    
    // Create network manager for downloading
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    
    // Connect to download progress
    QNetworkReply* reply = nullptr;
    connect(manager, &QNetworkAccessManager::finished, this, [this, manager](QNetworkReply* downloadReply) {
        manager->deleteLater();
        
        if (downloadReply->error() == QNetworkReply::NoError) {
            // Save downloaded file
            QByteArray data = downloadReply->readAll();
            QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
            QString fileName = QUrl(updateUrl).fileName();
            if (fileName.isEmpty()) {
                fileName = "ItemEditor_Update.exe";
            }
            QString filePath = QDir(tempDir).filePath(fileName);
            
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(data);
                file.close();
                
                // Launch installer
                QProcess::startDetached(filePath);
                
                // Close application to allow update
                QMessageBox::information(this, tr("Update"), 
                                       tr("Update downloaded and launched. The application will now close."));
                QApplication::quit();
            } else {
                QMessageBox::warning(this, tr("Error"), 
                                   tr("Failed to save update file: %1").arg(file.errorString()));
            }
        } else {
            QMessageBox::warning(this, tr("Error"), 
                               tr("Failed to download update: %1").arg(downloadReply->errorString()));
        }
        
        showDownloadProgress(false);
        downloadReply->deleteLater();
    });
    
    // Make download request
    auto downloadRequest = QNetworkRequest(QUrl(updateUrl));
    downloadRequest.setRawHeader("User-Agent", "ItemEditor-Qt6");
    downloadReply = manager->get(downloadRequest);
    
    // Connect progress signal
    connect(downloadReply, &QNetworkReply::downloadProgress, this, [this](qint64 received, qint64 total) {
        if (total > 0) {
            int progress = static_cast<int>((received * 100) / total);
            setDownloadProgress(progress);
        }
    });
    
    emit installRequested();
}

bool UpdateDialog::isNewerVersion(const QString& available, const QString& current)
{
    // Simple version comparison - split by dots and compare numerically
    QStringList availableParts = available.split('.');
    QStringList currentParts = current.split('.');
    
    // Pad shorter version with zeros
    while (availableParts.size() < currentParts.size()) {
        availableParts.append("0");
    }
    while (currentParts.size() < availableParts.size()) {
        currentParts.append("0");
    }
    
    // Compare each part
    for (int i = 0; i < availableParts.size(); ++i) {
        int availableNum = availableParts[i].toInt();
        int currentNum = currentParts[i].toInt();
        
        if (availableNum > currentNum) {
            return true;
        } else if (availableNum < currentNum) {
            return false;
        }
    }
    
    return false; // Versions are equal
}

void UpdateDialog::onUpdateClicked()
{
    if (updateAvailable) {
        downloadUpdate();
    }
}

void UpdateDialog::onCancelClicked()
{
    reject();
}

void UpdateDialog::onCheckUpdatesClicked()
{
    checkForUpdates();
}

} // namespace ItemEditor