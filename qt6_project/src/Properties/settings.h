/**
 * Item Editor Qt6 - Application Settings Header
 * Exact mirror of Legacy_App/csharp/Source/Properties/Settings.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_SETTINGS_H
#define ITEMEDITOR_SETTINGS_H

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QSize>
#include <QPoint>
#include <QByteArray>

namespace ItemEditor {

/**
 * Settings class
 * Provides persistent application configuration storage
 * Exact mirror of C# Settings functionality
 */
class Settings : public QObject
{
    Q_OBJECT

public:
    // Singleton access
    static Settings& instance();
    
    // General settings
    void setValue(const QString& key, const QVariant& value);
    QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;
    
    // Window settings
    void setWindowGeometry(const QString& windowName, const QByteArray& geometry);
    QByteArray getWindowGeometry(const QString& windowName) const;
    void setWindowState(const QString& windowName, const QByteArray& state);
    QByteArray getWindowState(const QString& windowName) const;
    
    // Recent files
    void addRecentFile(const QString& filePath);
    QStringList getRecentFiles() const;
    void clearRecentFiles();
    
    // Application preferences
    void setLanguage(const QString& language);
    QString getLanguage() const;
    void setTheme(const QString& theme);
    QString getTheme() const;
    void setAutoSave(bool enabled);
    bool getAutoSave() const;
    void setAutoSaveInterval(int minutes);
    int getAutoSaveInterval() const;
    
    // Plugin settings
    void setPluginEnabled(const QString& pluginName, bool enabled);
    bool isPluginEnabled(const QString& pluginName) const;
    void setPluginSettings(const QString& pluginName, const QVariant& settings);
    QVariant getPluginSettings(const QString& pluginName) const;
    
    // OTB file settings
    void setLastOpenedPath(const QString& path);
    QString getLastOpenedPath() const;
    void setDefaultClientVersion(const QString& version);
    QString getDefaultClientVersion() const;
    
    // UI settings
    void setShowStatusBar(bool show);
    bool getShowStatusBar() const;
    void setShowToolBar(bool show);
    bool getShowToolBar() const;
    void setShowItemPreview(bool show);
    bool getShowItemPreview() const;
    
    // Comparison settings
    void setCompareShowOnlyDifferences(bool showOnly);
    bool getCompareShowOnlyDifferences() const;
    void setCompareIgnoreMetadata(bool ignore);
    bool getCompareIgnoreMetadata() const;
    
    // Synchronization
    void sync();
    
signals:
    void settingChanged(const QString& key, const QVariant& value);
    void recentFilesChanged();
    void themeChanged(const QString& theme);
    void languageChanged(const QString& language);

private:
    explicit Settings(QObject* parent = nullptr);
    ~Settings() override = default;
    
    // Disable copy constructor and assignment operator
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
    
    QSettings* m_settings;
    
    // Constants for settings keys
    static const QString KEY_WINDOW_GEOMETRY;
    static const QString KEY_WINDOW_STATE;
    static const QString KEY_RECENT_FILES;
    static const QString KEY_LANGUAGE;
    static const QString KEY_THEME;
    static const QString KEY_AUTO_SAVE;
    static const QString KEY_AUTO_SAVE_INTERVAL;
    static const QString KEY_PLUGIN_ENABLED;
    static const QString KEY_PLUGIN_SETTINGS;
    static const QString KEY_LAST_OPENED_PATH;
    static const QString KEY_DEFAULT_CLIENT_VERSION;
    static const QString KEY_SHOW_STATUS_BAR;
    static const QString KEY_SHOW_TOOL_BAR;
    static const QString KEY_SHOW_ITEM_PREVIEW;
    static const QString KEY_COMPARE_SHOW_ONLY_DIFFERENCES;
    static const QString KEY_COMPARE_IGNORE_METADATA;
};

} // namespace ItemEditor

#endif // ITEMEDITOR_SETTINGS_H