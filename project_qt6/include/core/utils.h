#pragma once

#include <QString>
#include <QStringList>
#include <QDir>
#include <QFileInfo>
#include <QPixmap>
#include <QIcon>
#include <QWidget>
#include <QMessageBox>
#include <QProgressDialog>
#include <QApplication>
#include <memory>

namespace Core {

/**
 * @brief Utility class providing common helper functions
 * 
 * This class contains static utility methods used throughout the application
 * for file operations, UI helpers, string manipulation, and other common tasks.
 */
class Utils
{
public:
    // File and path utilities
    static QString getFileExtension(const QString &filePath);
    static QString getFileName(const QString &filePath);
    static QString getFileNameWithoutExtension(const QString &filePath);
    static QString getDirectoryPath(const QString &filePath);
    static bool fileExists(const QString &filePath);
    static bool directoryExists(const QString &dirPath);
    static bool createDirectory(const QString &dirPath);
    static qint64 getFileSize(const QString &filePath);
    static QString formatFileSize(qint64 bytes);

    // String utilities
    static QString trimString(const QString &str);
    static QStringList splitString(const QString &str, const QString &separator);
    static QString joinStrings(const QStringList &strings, const QString &separator);
    static bool isNullOrEmpty(const QString &str);
    static QString capitalizeFirst(const QString &str);

    // UI utilities
    static void centerWidget(QWidget *widget, QWidget *parent = nullptr);
    static QIcon loadIcon(const QString &iconPath);
    static QPixmap loadPixmap(const QString &imagePath);
    static void showErrorMessage(QWidget *parent, const QString &title, const QString &message);
    static void showWarningMessage(QWidget *parent, const QString &title, const QString &message);
    static void showInfoMessage(QWidget *parent, const QString &title, const QString &message);
    static bool showQuestionMessage(QWidget *parent, const QString &title, const QString &message);

    // Progress dialog utilities
    static std::unique_ptr<QProgressDialog> createProgressDialog(
        QWidget *parent, 
        const QString &labelText, 
        int minimum = 0, 
        int maximum = 100
    );

    // Application utilities
    static QString getApplicationVersion();
    static QString getApplicationName();
    static void processEvents();
    static void setWaitCursor();
    static void restoreCursor();

private:
    Utils() = delete; // Static class
};

} // namespace Core