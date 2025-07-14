#include "PathHelper.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

namespace ItemEditor {
namespace Helpers {

QString PathHelper::getApplicationPath()
{
    QString appPath = QCoreApplication::applicationDirPath();
    
    // Normalize path separators for cross-platform compatibility
    return QDir::toNativeSeparators(appPath);
}

QString PathHelper::getDataPath()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    
    // Ensure the directory exists
    ensureDirectoryExists(dataPath);
    
    // Normalize path separators for cross-platform compatibility
    return QDir::toNativeSeparators(dataPath);
}

QString PathHelper::getPluginsPath()
{
    QString pluginsPath;
    
    // Try different plugin locations based on platform and deployment
    QStringList candidatePaths;
    
    // 1. Relative to application directory (development/portable)
    candidatePaths << QDir(getApplicationPath()).absoluteFilePath("plugins");
    
    // 2. Platform-specific system locations
#ifdef Q_OS_WIN
    // Windows: relative to executable or in Program Files
    candidatePaths << QDir(getApplicationPath()).absoluteFilePath("../plugins");
    candidatePaths << QDir(getApplicationPath()).absoluteFilePath("../../plugins");
#elif defined(Q_OS_MAC)
    // macOS: inside app bundle or relative locations
    candidatePaths << QDir(getApplicationPath()).absoluteFilePath("../PlugIns");
    candidatePaths << QDir(getApplicationPath()).absoluteFilePath("../Resources/plugins");
    candidatePaths << QDir(getApplicationPath()).absoluteFilePath("../../../plugins");
#else
    // Linux: standard locations
    candidatePaths << QDir(getApplicationPath()).absoluteFilePath("../lib/ItemEditor/plugins");
    candidatePaths << "/usr/lib/ItemEditor/plugins";
    candidatePaths << "/usr/local/lib/ItemEditor/plugins";
    candidatePaths << QDir::homePath() + "/.local/lib/ItemEditor/plugins";
#endif
    
    // 3. Environment variable override
    QString envPluginPath = qgetenv("ITEMEDITOR_PLUGIN_PATH");
    if (!envPluginPath.isEmpty()) {
        candidatePaths.prepend(envPluginPath);
    }
    
    // Find the first existing directory or use the first candidate
    for (const QString& path : candidatePaths) {
        QString normalizedPath = QDir::toNativeSeparators(QDir(path).absolutePath());
        if (QDir(normalizedPath).exists()) {
            pluginsPath = normalizedPath;
            break;
        }
    }
    
    // If no existing directory found, use the first candidate (relative to app)
    if (pluginsPath.isEmpty()) {
        pluginsPath = QDir::toNativeSeparators(candidatePaths.first());
    }
    
    // Ensure the plugins directory exists
    ensureDirectoryExists(pluginsPath);
    
    qDebug() << "Plugin path resolved to:" << pluginsPath;
    return pluginsPath;
}

QString PathHelper::getSettingsPath()
{
    QString settingsPath = QDir(getDataPath()).absoluteFilePath("settings.xml");
    
    // Normalize path separators for cross-platform compatibility
    return QDir::toNativeSeparators(settingsPath);
}

bool PathHelper::ensureDirectoryExists(const QString& path)
{
    if (path.isEmpty()) {
        qWarning() << "PathHelper::ensureDirectoryExists: Empty path provided";
        return false;
    }
    
    QDir dir;
    QString normalizedPath = QDir::toNativeSeparators(path);
    
    if (!dir.exists(normalizedPath)) {
        qDebug() << "Creating directory:" << normalizedPath;
        
        if (!dir.mkpath(normalizedPath)) {
            qWarning() << "Failed to create directory:" << normalizedPath;
            return false;
        }
        
        qDebug() << "Successfully created directory:" << normalizedPath;
    }
    
    return true;
}

QString PathHelper::getTemporaryPath()
{
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString appTempPath = QDir(tempPath).absoluteFilePath("ItemEditor");
    
    // Ensure the temporary directory exists
    ensureDirectoryExists(appTempPath);
    
    return QDir::toNativeSeparators(appTempPath);
}

QString PathHelper::getCachePath()
{
    QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    
    // Ensure the cache directory exists
    ensureDirectoryExists(cachePath);
    
    return QDir::toNativeSeparators(cachePath);
}

QString PathHelper::getConfigPath()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    
    // Ensure the config directory exists
    ensureDirectoryExists(configPath);
    
    return QDir::toNativeSeparators(configPath);
}

QString PathHelper::getLogPath()
{
    QString logPath;
    
#ifdef Q_OS_WIN
    // Windows: Use AppData/Local for logs
    logPath = QDir(getDataPath()).absoluteFilePath("logs");
#elif defined(Q_OS_MAC)
    // macOS: Use ~/Library/Logs
    logPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/logs";
#else
    // Linux: Use ~/.local/share or /var/log
    logPath = QDir(getDataPath()).absoluteFilePath("logs");
#endif
    
    // Ensure the log directory exists
    ensureDirectoryExists(logPath);
    
    return QDir::toNativeSeparators(logPath);
}

bool PathHelper::isPortableMode()
{
    // Check if running in portable mode (settings file next to executable)
    QString portableSettings = QDir(getApplicationPath()).absoluteFilePath("portable.ini");
    return QFileInfo::exists(portableSettings);
}

QString PathHelper::normalizePath(const QString& path)
{
    if (path.isEmpty()) {
        return QString();
    }
    
    // Convert to absolute path and normalize separators
    QDir dir(path);
    return QDir::toNativeSeparators(dir.absolutePath());
}

bool PathHelper::isValidPath(const QString& path)
{
    if (path.isEmpty()) {
        return false;
    }
    
    // Check for invalid characters (basic validation)
    QStringList invalidChars;
    
#ifdef Q_OS_WIN
    invalidChars << "<" << ">" << ":" << "\"" << "|" << "?" << "*";
    // Note: Windows also restricts certain names like CON, PRN, etc.
#endif
    
    for (const QString& invalidChar : invalidChars) {
        if (path.contains(invalidChar)) {
            return false;
        }
    }
    
    // Check path length (Windows has 260 character limit for full paths)
#ifdef Q_OS_WIN
    if (path.length() > 259) {
        return false;
    }
#endif
    
    return true;
}

} // namespace Helpers
} // namespace ItemEditor