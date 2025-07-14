#include "Utils.h"
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QLocale>

namespace ItemEditor {

bool Utils::byteArrayCompare(const QByteArray& a1, const QByteArray& a2)
{
    return a1 == a2; // Qt's QByteArray already implements efficient comparison
}

QString Utils::findClientFile(const QString& directory, const QString& extension)
{
    if (!QDir(directory).exists()) {
        return QString();
    }

    QDir dir(directory);
    QString ext = extension;
    
    // Ensure extension starts with dot
    if (!ext.startsWith(".")) {
        ext = "." + ext;
    }

    // First, try to find "Tibia" + extension
    QString tibiaFile = dir.absoluteFilePath("Tibia" + ext);
    if (QFileInfo::exists(tibiaFile)) {
        return tibiaFile;
    }

    // If not found, search for any file with the specified extension
    QStringList filters;
    filters << "*" + ext;
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    if (!files.isEmpty()) {
        return files.first().absoluteFilePath();
    }

    return QString(); // Not found
}

QString Utils::toSafeIdentifier(const QString& input)
{
    if (input.isEmpty()) {
        return "identifier";
    }

    QString safe = input;
    
    // Replace non-alphanumeric characters with underscores
    QRegularExpression nonAlphaNum("[^a-zA-Z0-9_]");
    safe.replace(nonAlphaNum, "_");
    
    // Ensure it doesn't start with a number
    if (!safe.isEmpty() && safe.at(0).isDigit()) {
        safe = "id_" + safe;
    }
    
    // Remove multiple consecutive underscores
    QRegularExpression multipleUnderscores("_{2,}");
    safe.replace(multipleUnderscores, "_");
    
    // Remove leading/trailing underscores
    while (safe.startsWith("_")) {
        safe = safe.mid(1);
    }
    while (safe.endsWith("_")) {
        safe.chop(1);
    }
    
    // Ensure we have something left
    if (safe.isEmpty()) {
        safe = "identifier";
    }
    
    return safe;
}

bool Utils::isNumeric(const QString& str)
{
    if (str.isEmpty()) {
        return false;
    }
    
    bool ok;
    str.toDouble(&ok);
    return ok;
}

QString Utils::normalizeString(const QString& input)
{
    QString normalized = input.trimmed();
    
    // Normalize line endings to \n
    normalized.replace("\r\n", "\n");
    normalized.replace("\r", "\n");
    
    // Remove excessive whitespace
    QRegularExpression multipleSpaces("  +");
    normalized.replace(multipleSpaces, " ");
    
    return normalized;
}

QString Utils::formatFileSize(qint64 bytes)
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    const qint64 TB = GB * 1024;
    
    QLocale locale;
    
    if (bytes >= TB) {
        return locale.toString(static_cast<double>(bytes) / TB, 'f', 2) + " TB";
    } else if (bytes >= GB) {
        return locale.toString(static_cast<double>(bytes) / GB, 'f', 2) + " GB";
    } else if (bytes >= MB) {
        return locale.toString(static_cast<double>(bytes) / MB, 'f', 2) + " MB";
    } else if (bytes >= KB) {
        return locale.toString(static_cast<double>(bytes) / KB, 'f', 2) + " KB";
    } else {
        return locale.toString(bytes) + " bytes";
    }
}

} // namespace ItemEditor