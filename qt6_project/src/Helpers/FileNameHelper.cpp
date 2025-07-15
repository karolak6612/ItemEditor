#include "FileNameHelper.h"
#include "PathHelper.h"
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>

namespace ItemEditor {
namespace Helpers {

QString FileNameHelper::getSettingData()
{
    return PathHelper::getSettingsPath();
}

bool FileNameHelper::isValidFileName(const QString& fileName)
{
    if (fileName.isEmpty() || fileName.trimmed().isEmpty()) {
        return false;
    }

    // Check for invalid characters
    QStringList invalidChars = getInvalidFileNameCharacters();
    for (const QString& invalidChar : invalidChars) {
        if (fileName.contains(invalidChar)) {
            return false;
        }
    }

    // Check for reserved names on Windows
    QStringList reservedNames = {
        "CON", "PRN", "AUX", "NUL",
        "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
        "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
    };

    QString baseName = QFileInfo(fileName).baseName().toUpper();
    if (reservedNames.contains(baseName)) {
        return false;
    }

    return true;
}QString FileNameHelper::generateSafeFileName(const QString& input)
{
    if (input.isEmpty()) {
        return "untitled";
    }

    QString safe = input.trimmed();
    
    // Replace invalid characters with underscores
    QStringList invalidChars = getInvalidFileNameCharacters();
    for (const QString& invalidChar : invalidChars) {
        safe.replace(invalidChar, "_");
    }

    // Remove multiple consecutive underscores
    QRegularExpression multipleUnderscores("_{2,}");
    safe.replace(multipleUnderscores, "_");

    // Remove leading/trailing underscores
    safe = safe.trimmed();
    while (safe.startsWith("_")) {
        safe = safe.mid(1);
    }
    while (safe.endsWith("_")) {
        safe.chop(1);
    }

    // Ensure we have something left
    if (safe.isEmpty()) {
        safe = "untitled";
    }

    // Check if it's a reserved name and modify if needed
    if (!isValidFileName(safe)) {
        safe = "file_" + safe;
    }

    return safe;
}

QString FileNameHelper::getFileExtension(const QString& fileName)
{
    QFileInfo fileInfo(fileName);
    return fileInfo.suffix();
}

bool FileNameHelper::hasExtension(const QString& fileName, const QString& extension)
{
    QString fileExt = getFileExtension(fileName).toLower();
    QString checkExt = extension.toLower();
    
    // Remove leading dot if present
    if (checkExt.startsWith(".")) {
        checkExt = checkExt.mid(1);
    }
    
    return fileExt == checkExt;
}QStringList FileNameHelper::getInvalidFileNameCharacters()
{
    // Common invalid characters across platforms
    QStringList invalidChars = {
        "<", ">", ":", "\"", "|", "?", "*"
    };

#ifdef Q_OS_WIN
    // Additional Windows-specific invalid characters
    invalidChars.append({"/", "\\"});
#else
    // On Unix-like systems, only forward slash is invalid in filenames
    invalidChars.append(QString("/"));
#endif

    // Control characters (0-31)
    for (int i = 0; i < 32; ++i) {
        invalidChars.append(QString(QChar(i)));
    }

    return invalidChars;
}

} // namespace Helpers
} // namespace ItemEditor