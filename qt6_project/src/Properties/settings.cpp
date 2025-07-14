/**
 * Item Editor Qt6 - Application Settings Implementation
 * Exact mirror of Legacy_App/csharp/Source/Properties/Settings.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "settings.h"
#include "version.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>

namespace ItemEditor {

// Settings key constants
const QString Settings::KEY_WINDOW_GEOMETRY = QStringLiteral("Window/%1/Geometry");
const QString Settings::KEY_WINDOW_STATE = QStringLiteral("Window/%1/State");
const QString Settings::KEY_RECENT_FILES = QStringLiteral("RecentFiles");
const QString Settings::KEY_LANGUAGE = QStringLiteral("Language");
const QString Settings::KEY_THEME = QStringLiteral("Theme");
const QString Settings::KEY_AUTO_SAVE = QStringLiteral("AutoSave");
const QString Settings::KEY_AUTO_SAVE_INTERVAL = QStringLiteral("AutoSaveInterval");
const QString Settings::KEY_PLUGIN_ENABLED = QStringLiteral("Plugins/%1/Enabled");
const QString Settings::KEY_PLUGIN_SETTINGS = QStringLiteral("Plugins/%1/Settings");
const QString Settings::KEY_LAST_OPENED_PATH = QStringLiteral("LastOpenedPath");
const QString Settings::KEY_DEFAULT_CLIENT_VERSION = QStringLiteral("DefaultClientVersion");
const QString Settings::KEY_SHOW_STATUS_BAR = QStringLiteral("UI/ShowStatusBar");
const QString Settings::KEY_SHOW_TOOL_BAR = QStringLiteral("UI/ShowToolBar");
const QString Settings::KEY_SHOW_ITEM_PREVIEW = QStringLiteral("UI/ShowItemPreview");
const QString Settings::KEY_COMPARE_SHOW_ONLY_DIFFERENCES = QStringLiteral("Compare/ShowOnlyDifferences");
const QString Settings::KEY_COMPARE_IGNORE_METADATA = QStringLiteral("Compare/IgnoreMetadata");

Settings::Settings(QObject* parent)
    : QObject(parent)
{
    // Initialize QSettings with application information
    QCoreApplication::setOrganizationName(Version::getCompanyName());
    QCoreApplication::setApplicationName(Version::getApplicationName());
    QCoreApplication::setApplicationVersion(Version::getVersionString());
    
    m_settings = new QSettings(this);
}

Settings& Settings::instance()
{
    static Settings instance;
    return instance;
}

void Settings::setValue(const QString& key, const QVariant& value)
{
    m_settings->setValue(key, value);
    emit settingChanged(key, value);
}

QVariant Settings::value(const QString& key, const QVariant& defaultValue) const
{
    return m_settings->value(key, defaultValue);
}

void Settings::setWindowGeometry(const QString& windowName, const QByteArray& geometry)
{
    const QString key = KEY_WINDOW_GEOMETRY.arg(windowName);
    setValue(key, geometry);
}

QByteArray Settings::getWindowGeometry(const QString& windowName) const
{
    const QString key = KEY_WINDOW_GEOMETRY.arg(windowName);
    return value(key).toByteArray();
}

void Settings::setWindowState(const QString& windowName, const QByteArray& state)
{
    const QString key = KEY_WINDOW_STATE.arg(windowName);
    setValue(key, state);
}

QByteArray Settings::getWindowState(const QString& windowName) const
{
    const QString key = KEY_WINDOW_STATE.arg(windowName);
    return value(key).toByteArray();
}

void Settings::addRecentFile(const QString& filePath)
{
    QStringList recentFiles = getRecentFiles();
    
    // Remove if already exists
    recentFiles.removeAll(filePath);
    
    // Add to front
    recentFiles.prepend(filePath);
    
    // Limit to 10 recent files
    while (recentFiles.size() > 10) {
        recentFiles.removeLast();
    }
    
    setValue(KEY_RECENT_FILES, recentFiles);
    emit recentFilesChanged();
}

QStringList Settings::getRecentFiles() const
{
    return value(KEY_RECENT_FILES).toStringList();
}

void Settings::clearRecentFiles()
{
    setValue(KEY_RECENT_FILES, QStringList());
    emit recentFilesChanged();
}

void Settings::setLanguage(const QString& language)
{
    setValue(KEY_LANGUAGE, language);
    emit languageChanged(language);
}

QString Settings::getLanguage() const
{
    return value(KEY_LANGUAGE, QStringLiteral("en")).toString();
}

void Settings::setTheme(const QString& theme)
{
    setValue(KEY_THEME, theme);
    emit themeChanged(theme);
}

QString Settings::getTheme() const
{
    return value(KEY_THEME, QStringLiteral("light")).toString();
}

void Settings::setAutoSave(bool enabled)
{
    setValue(KEY_AUTO_SAVE, enabled);
}

bool Settings::getAutoSave() const
{
    return value(KEY_AUTO_SAVE, false).toBool();
}

void Settings::setAutoSaveInterval(int minutes)
{
    setValue(KEY_AUTO_SAVE_INTERVAL, minutes);
}

int Settings::getAutoSaveInterval() const
{
    return value(KEY_AUTO_SAVE_INTERVAL, 5).toInt();
}

void Settings::setPluginEnabled(const QString& pluginName, bool enabled)
{
    const QString key = KEY_PLUGIN_ENABLED.arg(pluginName);
    setValue(key, enabled);
}

bool Settings::isPluginEnabled(const QString& pluginName) const
{
    const QString key = KEY_PLUGIN_ENABLED.arg(pluginName);
    return value(key, true).toBool(); // Default to enabled
}

void Settings::setPluginSettings(const QString& pluginName, const QVariant& settings)
{
    const QString key = KEY_PLUGIN_SETTINGS.arg(pluginName);
    setValue(key, settings);
}

QVariant Settings::getPluginSettings(const QString& pluginName) const
{
    const QString key = KEY_PLUGIN_SETTINGS.arg(pluginName);
    return value(key);
}

void Settings::setLastOpenedPath(const QString& path)
{
    setValue(KEY_LAST_OPENED_PATH, path);
}

QString Settings::getLastOpenedPath() const
{
    return value(KEY_LAST_OPENED_PATH, 
                QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
}

void Settings::setDefaultClientVersion(const QString& version)
{
    setValue(KEY_DEFAULT_CLIENT_VERSION, version);
}

QString Settings::getDefaultClientVersion() const
{
    return value(KEY_DEFAULT_CLIENT_VERSION, QStringLiteral("10.98")).toString();
}

void Settings::setShowStatusBar(bool show)
{
    setValue(KEY_SHOW_STATUS_BAR, show);
}

bool Settings::getShowStatusBar() const
{
    return value(KEY_SHOW_STATUS_BAR, true).toBool();
}

void Settings::setShowToolBar(bool show)
{
    setValue(KEY_SHOW_TOOL_BAR, show);
}

bool Settings::getShowToolBar() const
{
    return value(KEY_SHOW_TOOL_BAR, true).toBool();
}

void Settings::setShowItemPreview(bool show)
{
    setValue(KEY_SHOW_ITEM_PREVIEW, show);
}

bool Settings::getShowItemPreview() const
{
    return value(KEY_SHOW_ITEM_PREVIEW, true).toBool();
}

void Settings::setCompareShowOnlyDifferences(bool showOnly)
{
    setValue(KEY_COMPARE_SHOW_ONLY_DIFFERENCES, showOnly);
}

bool Settings::getCompareShowOnlyDifferences() const
{
    return value(KEY_COMPARE_SHOW_ONLY_DIFFERENCES, false).toBool();
}

void Settings::setCompareIgnoreMetadata(bool ignore)
{
    setValue(KEY_COMPARE_IGNORE_METADATA, ignore);
}

bool Settings::getCompareIgnoreMetadata() const
{
    return value(KEY_COMPARE_IGNORE_METADATA, false).toBool();
}

void Settings::sync()
{
    m_settings->sync();
}

} // namespace ItemEditor