#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "core/settingsmanager.h"
#include <QString>
#include <QStringList>
#include <QByteArray>

namespace ItemEditor {
namespace Core {

/**
 * @brief Convenience wrapper for application settings
 * 
 * This class provides easy access to commonly used settings with
 * type-safe methods and meaningful names matching the C# implementation.
 */
class AppSettings
{
public:
    // Client settings (matching C# Properties.Settings)
    static QString clientDirectory() {
        return SettingsManager::instance().getValue(SettingsManager::SettingKey::ClientDirectory).toString();
    }
    
    static void setClientDirectory(const QString& path) {
        SettingsManager::instance().setValue(SettingsManager::SettingKey::ClientDirectory, path);
    }
    
    static bool extended() {
        return SettingsManager::instance().getValue(SettingsManager::SettingKey::Extended).toBool();
    }
    
    static void setExtended(bool value) {
        SettingsManager::instance().setValue(SettingsManager::SettingKey::Extended, value);
    }
    
    static bool transparency() {
        return SettingsManager::instance().getValue(SettingsManager::SettingKey::Transparency).toBool();
    }
    
    static void setTransparency(bool value) {
        SettingsManager::instance().setValue(SettingsManager::SettingKey::Transparency, value);
    }
    
    static quint32 datSignature() {
        return SettingsManager::instance().getValue(SettingsManager::SettingKey::DatSignature).toUInt();
    }
    
    static void setDatSignature(quint32 signature) {
        SettingsManager::instance().setValue(SettingsManager::SettingKey::DatSignature, signature);
    }
    
    static quint32 sprSignature() {
        return SettingsManager::instance().getValue(SettingsManager::SettingKey::SprSignature).toUInt();
    }
    
    static void setSprSignature(quint32 signature) {
        SettingsManager::instance().setValue(SettingsManager::SettingKey::SprSignature, signature);
    }
    
    static bool frameDurations() {
        return SettingsManager::instance().getValue(SettingsManager::SettingKey::FrameDurations).toBool();
    }
    
    static void setFrameDurations(bool value) {
        SettingsManager::instance().setValue(SettingsManager::SettingKey::FrameDurations, value);
    }

    // Application settings
    static QByteArray windowGeometry() {
        return SettingsManager::instance().getValue(SettingsManager::SettingKey::WindowGeometry).toByteArray();
    }
    
    static void setWindowGeometry(const QByteArray& geometry) {
        SettingsManager::instance().setValue(SettingsManager::SettingKey::WindowGeometry, geometry);
    }
    
    static QByteArray windowState() {
        return SettingsManager::instance().getValue(SettingsManager::SettingKey::WindowState).toByteArray();
    }
    
    static void setWindowState(const QByteArray& state) {
        SettingsManager::instance().setValue(SettingsManager::SettingKey::WindowState, state);
    }
    
    static QStringList recentFiles() {
        return SettingsManager::instance().getValue(SettingsManager::SettingKey::RecentFiles).toStringList();
    }
    
    static void setRecentFiles(const QStringList& files) {
        SettingsManager::instance().setValue(SettingsManager::SettingKey::RecentFiles, files);
    }
    
    static QString language() {
        return SettingsManager::instance().getValue(SettingsManager::SettingKey::Language).toString();
    }
    
    static void setLanguage(const QString& lang) {
        SettingsManager::instance().setValue(SettingsManager::SettingKey::Language, lang);
    }
    
    static QString theme() {
        return SettingsManager::instance().getValue(SettingsManager::SettingKey::Theme).toString();
    }
    
    static void setTheme(const QString& themeName) {
        SettingsManager::instance().setValue(SettingsManager::SettingKey::Theme, themeName);
    }

    // UI settings
    static bool showToolbar() {
        return SettingsManager::instance().getValue(SettingsManager::SettingKey::ShowToolbar).toBool();
    }
    
    static void setShowToolbar(bool show) {
        SettingsManager::instance().setValue(SettingsManager::SettingKey::ShowToolbar, show);
    }
    
    static bool showStatusbar() {
        return SettingsManager::instance().getValue(SettingsManager::SettingKey::ShowStatusbar).toBool();
    }
    
    static void setShowStatusbar(bool show) {
        SettingsManager::instance().setValue(SettingsManager::SettingKey::ShowStatusbar, show);
    }
    
    static bool autoSave() {
        return SettingsManager::instance().getValue(SettingsManager::SettingKey::AutoSave).toBool();
    }
    
    static void setAutoSave(bool enabled) {
        SettingsManager::instance().setValue(SettingsManager::SettingKey::AutoSave, enabled);
    }
    
    static int autoSaveInterval() {
        return SettingsManager::instance().getValue(SettingsManager::SettingKey::AutoSaveInterval).toInt();
    }
    
    static void setAutoSaveInterval(int seconds) {
        SettingsManager::instance().setValue(SettingsManager::SettingKey::AutoSaveInterval, seconds);
    }

    // Plugin settings
    static QString pluginDirectory() {
        return SettingsManager::instance().getValue(SettingsManager::SettingKey::PluginDirectory).toString();
    }
    
    static void setPluginDirectory(const QString& path) {
        SettingsManager::instance().setValue(SettingsManager::SettingKey::PluginDirectory, path);
    }
    
    static QStringList enabledPlugins() {
        return SettingsManager::instance().getValue(SettingsManager::SettingKey::EnabledPlugins).toStringList();
    }
    
    static void setEnabledPlugins(const QStringList& plugins) {
        SettingsManager::instance().setValue(SettingsManager::SettingKey::EnabledPlugins, plugins);
    }

    // Utility methods
    static void sync() {
        SettingsManager::instance().sync();
    }
    
    static void resetToDefaults() {
        SettingsManager::instance().resetAll();
    }
    
    static QString settingsPath() {
        return SettingsManager::instance().getSettingsPath();
    }

private:
    AppSettings() = delete; // Static class
};

} // namespace Core
} // namespace ItemEditor

#endif // APPSETTINGS_H