#pragma once

#include <QString>

namespace ItemEditor {
namespace Helpers {

/**
 * @brief Utility class for managing application paths
 * 
 * Provides cross-platform path management using Qt6 QStandardPaths
 * for consistent behavior across Windows, Linux, and macOS.
 */
class PathHelper
{
public:
    /**
     * @brief Get the application directory path
     * @return QString containing the directory where the executable is located
     */
    static QString getApplicationPath();

    /**
     * @brief Get the user's application data directory
     * @return QString containing the user-specific data directory for ItemEditor
     */
    static QString getDataPath();

    /**
     * @brief Get the plugins directory path
     * @return QString containing the path to the plugins directory
     */
    static QString getPluginsPath();

    /**
     * @brief Get the settings file path
     * @return QString containing the full path to the settings file
     */
    static QString getSettingsPath();

    /**
     * @brief Get the temporary directory path for ItemEditor
     * @return QString containing the temporary directory path
     */
    static QString getTemporaryPath();

    /**
     * @brief Get the cache directory path
     * @return QString containing the cache directory path
     */
    static QString getCachePath();

    /**
     * @brief Get the configuration directory path
     * @return QString containing the configuration directory path
     */
    static QString getConfigPath();

    /**
     * @brief Get the log directory path
     * @return QString containing the log directory path
     */
    static QString getLogPath();

    /**
     * @brief Ensure a directory exists, creating it if necessary
     * @param path The directory path to ensure exists
     * @return true if directory exists or was created successfully, false otherwise
     */
    static bool ensureDirectoryExists(const QString& path);

    /**
     * @brief Check if the application is running in portable mode
     * @return true if portable mode is detected, false otherwise
     */
    static bool isPortableMode();

    /**
     * @brief Normalize a path for the current platform
     * @param path The path to normalize
     * @return QString containing the normalized path
     */
    static QString normalizePath(const QString& path);

    /**
     * @brief Validate if a path is valid for the current platform
     * @param path The path to validate
     * @return true if the path is valid, false otherwise
     */
    static bool isValidPath(const QString& path);

private:
    PathHelper() = delete; // Static class, no instances allowed
};

} // namespace Helpers
} // namespace ItemEditor