#pragma once

#include <QString>
#include <QStringList>

namespace ItemEditor {
namespace Helpers {

/**
 * @brief Utility class for file name operations and validation
 * 
 * Provides file name validation, extension handling, and safe filename
 * generation using Qt6 classes for cross-platform compatibility.
 */
class FileNameHelper
{
public:
    /**
     * @brief Get the settings file path
     * @return QString containing the full path to the settings file
     */
    static QString getSettingData();

    /**
     * @brief Validate if a filename is safe for the current platform
     * @param fileName The filename to validate
     * @return true if filename is valid, false otherwise
     */
    static bool isValidFileName(const QString& fileName);

    /**
     * @brief Generate a safe filename from input string
     * @param input The input string to convert to safe filename
     * @return QString containing a safe filename
     */
    static QString generateSafeFileName(const QString& input);

    /**
     * @brief Get file extension from filename
     * @param fileName The filename to extract extension from
     * @return QString containing the extension (without dot)
     */
    static QString getFileExtension(const QString& fileName);

    /**
     * @brief Check if file has specific extension
     * @param fileName The filename to check
     * @param extension The extension to check for (without dot)
     * @return true if file has the specified extension, false otherwise
     */
    static bool hasExtension(const QString& fileName, const QString& extension);

    /**
     * @brief Get list of invalid characters for filenames on current platform
     * @return QStringList containing invalid characters
     */
    static QStringList getInvalidFileNameCharacters();

private:
    FileNameHelper() = delete; // Static class, no instances allowed
};

} // namespace Helpers
} // namespace ItemEditor