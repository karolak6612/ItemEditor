#pragma once

#include <QString>
#include <QByteArray>

namespace ItemEditor {

/**
 * @brief General utility functions for common operations
 * 
 * Provides utility functions for byte array comparison, file searching,
 * string manipulation, and other common operations using Qt6 classes.
 */
class Utils
{
public:
    /**
     * @brief Compare two byte arrays for equality
     * @param a1 First byte array
     * @param a2 Second byte array
     * @return true if arrays are equal, false otherwise
     */
    static bool byteArrayCompare(const QByteArray& a1, const QByteArray& a2);

    /**
     * @brief Find a client file in the specified directory
     * @param directory Directory to search in
     * @param extension File extension to look for (with or without dot)
     * @return QString containing the full path to the found file, empty if not found
     */
    static QString findClientFile(const QString& directory, const QString& extension);

    /**
     * @brief Convert string to safe identifier (alphanumeric + underscore)
     * @param input Input string to convert
     * @return QString containing safe identifier
     */
    static QString toSafeIdentifier(const QString& input);

    /**
     * @brief Check if string is numeric
     * @param str String to check
     * @return true if string represents a number, false otherwise
     */
    static bool isNumeric(const QString& str);

    /**
     * @brief Trim whitespace and normalize line endings
     * @param input Input string to normalize
     * @return QString with normalized whitespace and line endings
     */
    static QString normalizeString(const QString& input);

    /**
     * @brief Convert bytes to human-readable size string
     * @param bytes Number of bytes
     * @return QString containing human-readable size (e.g., "1.5 MB")
     */
    static QString formatFileSize(qint64 bytes);

private:
    Utils() = delete; // Static class, no instances allowed
};

} // namespace ItemEditor