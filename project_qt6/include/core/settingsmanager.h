#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QVariant>
#include <QMutex>
#include <QDir>
#include <memory>

namespace ItemEditor {
namespace Core {

/**
 * @brief Comprehensive settings management system for Qt6
 * 
 * This class provides centralized settings management with persistent storage,
 * default values, validation, and migration capabilities. It matches the
 * functionality of C# Settings.cs while providing Qt6-specific enhancements.
 */
class SettingsManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Settings categories for organization
     */
    enum class Category {
        Application,
        Client,
        UI,
        Plugin,
        Advanced
    };

    /**
     * @brief Predefined setting keys matching C# implementation
     */
    enum class SettingKey {
        // Client settings
        ClientDirectory,
        Extended,
        Transparency,
        DatSignature,
        SprSignature,
        FrameDurations,
        
        // Application settings
        WindowGeometry,
        WindowState,
        RecentFiles,
        Language,
        Theme,
        
        // UI settings
        ShowToolbar,
        ShowStatusbar,
        AutoSave,
        AutoSaveInterval,
        
        // Plugin settings
        PluginDirectory,
        EnabledPlugins,
        PluginSettings,
        
        // Advanced settings
        LogLevel,
        CacheSize,
        BackupCount,
        DebugMode
    };

    /**
     * @brief Get singleton instance
     */
    static SettingsManager& instance();

    /**
     * @brief Initialize settings system
     * @param organizationName Organization name for settings storage
     * @param applicationName Application name for settings storage
     * @return true if successful
     */
    bool initialize(const QString& organizationName = "OTTools", 
                   const QString& applicationName = "ItemEditor");

    /**
     * @brief Get setting value by key
     * @param key Setting key
     * @param defaultValue Default value if setting doesn't exist
     * @return Setting value
     */
    QVariant getValue(SettingKey key, const QVariant& defaultValue = QVariant()) const;

    /**
     * @brief Get setting value by string key
     * @param key Setting key as string
     * @param defaultValue Default value if setting doesn't exist
     * @return Setting value
     */
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;

    /**
     * @brief Set setting value by key
     * @param key Setting key
     * @param value Value to set
     * @return true if successful
     */
    bool setValue(SettingKey key, const QVariant& value);

    /**
     * @brief Set setting value by string key
     * @param key Setting key as string
     * @param value Value to set
     * @return true if successful
     */
    bool setValue(const QString& key, const QVariant& value);

    /**
     * @brief Check if setting exists
     * @param key Setting key
     * @return true if setting exists
     */
    bool contains(SettingKey key) const;

    /**
     * @brief Check if setting exists by string key
     * @param key Setting key as string
     * @return true if setting exists
     */
    bool contains(const QString& key) const;

    /**
     * @brief Remove setting
     * @param key Setting key
     */
    void remove(SettingKey key);

    /**
     * @brief Remove setting by string key
     * @param key Setting key as string
     */
    void remove(const QString& key);

    /**
     * @brief Get all settings in a category
     * @param category Settings category
     * @return Map of key-value pairs
     */
    QVariantMap getCategorySettings(Category category) const;

    /**
     * @brief Reset category to defaults
     * @param category Settings category
     */
    void resetCategory(Category category);

    /**
     * @brief Reset all settings to defaults
     */
    void resetAll();

    /**
     * @brief Force synchronization with storage
     */
    void sync();

    /**
     * @brief Get settings file path
     * @return Path to settings file
     */
    QString getSettingsPath() const;

    /**
     * @brief Validate setting value
     * @param key Setting key
     * @param value Value to validate
     * @return true if valid
     */
    bool validateValue(SettingKey key, const QVariant& value) const;

    /**
     * @brief Get setting key as string
     * @param key Setting key enum
     * @return String representation
     */
    QString getKeyString(SettingKey key) const;

    /**
     * @brief Get default value for setting
     * @param key Setting key
     * @return Default value
     */
    QVariant getDefaultValue(SettingKey key) const;

    /**
     * @brief Import settings from file
     * @param filePath Path to settings file
     * @return true if successful
     */
    bool importSettings(const QString& filePath);

    /**
     * @brief Export settings to file
     * @param filePath Path to export file
     * @return true if successful
     */
    bool exportSettings(const QString& filePath);

signals:
    /**
     * @brief Emitted when a setting value changes
     * @param key Setting key
     * @param newValue New value
     * @param oldValue Previous value
     */
    void settingChanged(const QString& key, const QVariant& newValue, const QVariant& oldValue);

    /**
     * @brief Emitted when settings are reset
     * @param category Category that was reset (empty for all)
     */
    void settingsReset(const QString& category);

private:
    explicit SettingsManager(QObject* parent = nullptr);
    ~SettingsManager() override = default;

    // Disable copy and assignment
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    /**
     * @brief Setup default values and key mappings
     */
    void setupDefaults();

    /**
     * @brief Setup key mappings
     */
    void setupKeyMappings();

    /**
     * @brief Setup validation rules
     */
    void setupValidation();

    /**
     * @brief Migrate settings from older versions
     */
    void migrateSettings();

    /**
     * @brief Get category for setting key
     * @param key Setting key
     * @return Category
     */
    Category getKeyCategory(SettingKey key) const;

    // Static instance
    static SettingsManager* s_instance;

    // Core settings object
    std::unique_ptr<QSettings> m_settings;

    // Key mappings
    QHash<SettingKey, QString> m_keyMappings;
    QHash<SettingKey, QVariant> m_defaultValues;
    QHash<SettingKey, Category> m_keyCategories;

    // Thread safety
    mutable QMutex m_mutex;

    // Initialization flag
    bool m_initialized;
};

} // namespace Core
} // namespace ItemEditor

#endif // SETTINGSMANAGER_H