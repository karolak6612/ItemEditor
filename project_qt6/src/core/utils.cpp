#include "core/utils.h"
#include "core/applicationbase.h"
#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QProgressDialog>
#include <QCursor>
#include <QPixmap>
#include <QIcon>

namespace Core {

// File and path utilities
QString Utils::getFileExtension(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.suffix().toLower();
}

QString Utils::getFileName(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.fileName();
}

QString Utils::getFileNameWithoutExtension(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.baseName();
}

QString Utils::getDirectoryPath(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.absolutePath();
}

bool Utils::fileExists(const QString &filePath)
{
    return QFileInfo::exists(filePath) && QFileInfo(filePath).isFile();
}bool Utils::directoryExists(const QString &dirPath)
{
    return QFileInfo::exists(dirPath) && QFileInfo(dirPath).isDir();
}

bool Utils::createDirectory(const QString &dirPath)
{
    QDir dir;
    return dir.mkpath(dirPath);
}

qint64 Utils::getFileSize(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.size();
}

QString Utils::formatFileSize(qint64 bytes)
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;

    if (bytes >= GB) {
        return QString::number(bytes / GB, 'f', 2) + " GB";
    } else if (bytes >= MB) {
        return QString::number(bytes / MB, 'f', 2) + " MB";
    } else if (bytes >= KB) {
        return QString::number(bytes / KB, 'f', 2) + " KB";
    } else {
        return QString::number(bytes) + " bytes";
    }
}

// String utilities
QString Utils::trimString(const QString &str)
{
    return str.trimmed();
}

QStringList Utils::splitString(const QString &str, const QString &separator)
{
    return str.split(separator, Qt::SkipEmptyParts);
}QString Utils::joinStrings(const QStringList &strings, const QString &separator)
{
    return strings.join(separator);
}

bool Utils::isNullOrEmpty(const QString &str)
{
    return str.isNull() || str.isEmpty();
}

QString Utils::capitalizeFirst(const QString &str)
{
    if (str.isEmpty()) return str;
    return str.at(0).toUpper() + str.mid(1);
}

// UI utilities
void Utils::centerWidget(QWidget *widget, QWidget *parent)
{
    if (!widget) return;

    if (parent) {
        QRect parentGeometry = parent->geometry();
        int x = parentGeometry.x() + (parentGeometry.width() - widget->width()) / 2;
        int y = parentGeometry.y() + (parentGeometry.height() - widget->height()) / 2;
        widget->move(x, y);
    } else {
        // Center on screen
        QScreen *screen = QApplication::primaryScreen();
        if (screen) {
            QRect screenGeometry = screen->geometry();
            int x = (screenGeometry.width() - widget->width()) / 2;
            int y = (screenGeometry.height() - widget->height()) / 2;
            widget->move(x, y);
        }
    }
}

QIcon Utils::loadIcon(const QString &iconPath)
{
    return QIcon(iconPath);
}

QPixmap Utils::loadPixmap(const QString &imagePath)
{
    return QPixmap(imagePath);
}// Message box utilities
void Utils::showErrorMessage(QWidget *parent, const QString &title, const QString &message)
{
    QMessageBox::critical(parent, title, message);
}

void Utils::showWarningMessage(QWidget *parent, const QString &title, const QString &message)
{
    QMessageBox::warning(parent, title, message);
}

void Utils::showInfoMessage(QWidget *parent, const QString &title, const QString &message)
{
    QMessageBox::information(parent, title, message);
}

bool Utils::showQuestionMessage(QWidget *parent, const QString &title, const QString &message)
{
    int result = QMessageBox::question(parent, title, message, 
                                      QMessageBox::Yes | QMessageBox::No, 
                                      QMessageBox::No);
    return result == QMessageBox::Yes;
}

// Progress dialog utilities
std::unique_ptr<QProgressDialog> Utils::createProgressDialog(
    QWidget *parent, 
    const QString &labelText, 
    int minimum, 
    int maximum)
{
    auto progress = std::make_unique<QProgressDialog>(labelText, "Cancel", minimum, maximum, parent);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(1000); // Show after 1 second
    return progress;
}

// Application utilities
QString Utils::getApplicationVersion()
{
    return QApplication::applicationVersion();
}

QString Utils::getApplicationName()
{
    return QApplication::applicationName();
}void Utils::processEvents()
{
    QApplication::processEvents();
}

void Utils::setWaitCursor()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

void Utils::restoreCursor()
{
    QApplication::restoreOverrideCursor();
}

} // namespace Core