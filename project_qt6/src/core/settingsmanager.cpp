#include "core/settingsmanager.h"
#include <QStandardPaths>
#include <QFileInfo>
#include <QDebug>
#include <QMutexLocker>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>

namespace ItemEditor {
namespace Core {

SettingsManager* SettingsManager::s_instance = nullptr;

SettingsManager& SettingsManager::instance()
{
    if (!s_instance) {
        s_instance = new SettingsManager();
    }
    return *s_instance;
}

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
{
    setupKeyMappings();
    setupDefaults();
    setupValidation();
}

bool SettingsManager::initialize(const QString& organizationName, const QString& applicationName)
{
    if (m_initialized) {
        return true;
    }

    qDebug() << "Initializing SettingsManager...";

    // Create QSettings with proper organization and application names
    QCoreApplication::setOrganizationName(organizationName);
    QCoreApplication::setApplicationName(applicationName);
    
    m_settings = std::make_unique<QSettings>();

    // Ensure settings directory exists
    QFileInfo settingsFile(m_settings->fileName());
    QDir settingsDir = settingsFile.dir();
    if (!settingsDir.exists()) {
        if (!settingsDir.mkpath(".")) {
            qWarning() << "Failed to create settings directory:" << settingsDir.path();
            return false;
        }
    }

    // Migrate settings if needed
    migrateSettings();

    m_initialized = true;
    qDebug() << "SettingsManager initialized successfully";
    qDebug() << "Settings file:" << m_settings->fileName();
    
    return true;
}void SettingsManager::setupKeyMappings()
{
    // Client settings (matching C# Settings.cs)
    m_keyMappings[SettingKey::ClientDirectory] = "Client/Directory";
    m_keyMappings[SettingKey::Extended] = "Client/Extended";
    m_keyMappings[SettingKey::Transparency] = "Client/Transparency";
    m_keyMappings[SettingKey::DatSignature] = "Client/DatSignature";
    m_keyMappings[SettingKey::SprSignature] = "Client/SprSignature";
    m_keyMappings[SettingKey::FrameDurations] = "Client/FrameDurations";

    // Application settings
    m_keyMappings[SettingKey::WindowGeometry] = "Application/WindowGeometry";
    m_keyMappings[SettingKey::WindowState] = "Application/WindowState";
    m_keyMappings[SettingKey::RecentFiles] = "Application/RecentFiles";
    m_keyMappings[SettingKey::Language] = "Application/Language";
    m_keyMappings[SettingKey::Theme] = "Application/Theme";

    // UI settings
    m_keyMappings[SettingKey::ShowToolbar] = "UI/ShowToolbar";
    m_keyMappings[SettingKey::ShowStatusbar] = "UI/ShowStatusbar";
    m_keyMappings[SettingKey::AutoSave] = "UI/AutoSave";
    m_keyMappings[SettingKey::AutoSaveInterval] = "UI/AutoSaveInterval";

    // Plugin settings
    m_keyMappings[SettingKey::PluginDirectory] = "Plugins/Directory";
    m_keyMappings[SettingKey::EnabledPlugins] = "Plugins/Enabled";
    m_keyMappings[SettingKey::PluginSettings] = "Plugins/Settings";

    // Advanced settings
    m_keyMappings[SettingKey::LogLevel] = "Advanced/LogLevel";
    m_keyMappings[SettingKey::CacheSize] = "Advanced/CacheSize";
    m_keyMappings[SettingKey::BackupCount] = "Advanced/BackupCount";
    m_keyMappings[SettingKey::DebugMode] = "Advanced/DebugMode";
}void SettingsManager::setupDefaults()
{
    // Client settings defaults (matching C# defaults)
    m_defaultValues[SettingKey::ClientDirectory] = QString();
    m_defaultValues[SettingKey::Extended] = false;
    m_defaultValues[SettingKey::Transparency] = false;
    m_defaultValues[SettingKey::DatSignature] = static_cast<quint32>(0);
    m_defaultValues[SettingKey::SprSignature] = static_cast<quint32>(0);
    m_defaultValues[SettingKey::FrameDurations] = false;

    // Application settings defaults
    m_defaultValues[SettingKey::WindowGeometry] = QByteArray();
    m_defaultValues[SettingKey::WindowState] = QByteArray();
    m_defaultValues[SettingKey::RecentFiles] = QStringList();
    m_defaultValues[SettingKey::Language] = "en";
    m_defaultValues[SettingKey::Theme] = "dark";

    // UI settings defaults
    m_defaultValues[SettingKey::ShowToolbar] = true;
    m_defaultValues[SettingKey::ShowStatusbar] = true;
    m_defaultValues[SettingKey::AutoSave] = false;
    m_defaultValues[SettingKey::AutoSaveInterval] = 300; // 5 minutes

    // Plugin settings defaults
    m_defaultValues[SettingKey::PluginDirectory] = "plugins";
    m_defaultValues[SettingKey::EnabledPlugins] = QStringList();
    m_defaultValues[SettingKey::PluginSettings] = QVariantMap();

    // Advanced settings defaults
    m_defaultValues[SettingKey::LogLevel] = "Info";
    m_defaultValues[SettingKey::CacheSize] = 100; // MB
    m_defaultValues[SettingKey::BackupCount] = 5;
    m_defaultValues[SettingKey::DebugMode] = false;

    // Setup category mappings
    m_keyCategories[SettingKey::ClientDirectory] = Category::Client;
    m_keyCategories[SettingKey::Extended] = Category::Client;
    m_keyCategories[SettingKey::Transparency] = Category::Client;
    m_keyCategories[SettingKey::DatSignature] = Category::Client;
    m_keyCategories[SettingKey::SprSignature] = Category::Client;
    m_keyCategories[SettingKey::FrameDurations] = Category::Client;

    m_keyCategories[SettingKey::WindowGeometry] = Category::Application;
    m_keyCategories[SettingKey::WindowState] = Category::Application;
    m_keyCategories[SettingKey::RecentFiles] = Category::Application;
    m_keyCategories[SettingKey::Language] = Category::Application;
    m_keyCategories[SettingKey::Theme] = Category::Application;

    m_keyCategories[SettingKey::ShowToolbar] = Category::UI;
    m_keyCategories[SettingKey::ShowStatusbar] = Category::UI;
    m_keyCategories[SettingKey::AutoSave] = Category::UI;
    m_keyCategories[SettingKey::AutoSaveInterval] = Category::UI;

    m_keyCategories[SettingKey::PluginDirectory] = Category::Plugin;
    m_keyCategories[SettingKey::EnabledPlugins] = Category::Plugin;
    m_keyCategories[SettingKey::PluginSettings] = Category::Plugin;

    m_keyCategories[SettingKey::LogLevel] = Category::Advanced;
    m_keyCategories[SettingKey::CacheSize] = Category::Advanced;
    m_keyCategories[SettingKey::BackupCount] = Category::Advanced;
    m_keyCategories[SettingKey::DebugMode] = Category::Advanced;
}void SettingsManager::setupValidation()
{
    // Validation rules will be implemented as needed
    // For now, basic type checking is sufficient
}

void SettingsManager::migrateSettings()
{
    // Check for settings version and migrate if needed
    QString currentVersion = m_settings->value("Meta/Version", "1.0.0").toString();
    QString targetVersion = "2.0.0";

    if (currentVersion != targetVersion) {
        qDebug() << "Migrating settings from" << currentVersion << "to" << targetVersion;
        
        // Perform migration logic here if needed
        // For now, just update the version
        m_settings->setValue("Meta/Version", targetVersion);
        m_settings->sync();
    }
}

QVariant SettingsManager::getValue(SettingKey key, const QVariant& defaultValue) const
{
    if (!m_initialized) {
        qWarning() << "SettingsManager not initialized";
        return defaultValue.isValid() ? defaultValue : getDefaultValue(key);
    }

    QMutexLocker locker(&m_mutex);
    QString keyString = getKeyString(key);
    QVariant fallbackDefault = defaultValue.isValid() ? defaultValue : getDefaultValue(key);
    
    return m_settings->value(keyString, fallbackDefault);
}

QVariant SettingsManager::getValue(const QString& key, const QVariant& defaultValue) const
{
    if (!m_initialized) {
        qWarning() << "SettingsManager not initialized";
        return defaultValue;
    }

    QMutexLocker locker(&m_mutex);
    return m_settings->value(key, defaultValue);
}bool SettingsManager::setValue(SettingKey key, const QVariant& value)
{
    if (!m_initialized) {
        qWarning() << "SettingsManager not initialized";
        return false;
    }

    if (!validateValue(key, value)) {
        qWarning() << "Invalid value for setting:" << getKeyString(key);
        return false;
    }

    QMutexLocker locker(&m_mutex);
    QString keyString = getKeyString(key);
    QVariant oldValue = m_settings->value(keyString);
    
    m_settings->setValue(keyString, value);
    
    if (oldValue != value) {
        emit settingChanged(keyString, value, oldValue);
    }
    
    return true;
}

bool SettingsManager::setValue(const QString& key, const QVariant& value)
{
    if (!m_initialized) {
        qWarning() << "SettingsManager not initialized";
        return false;
    }

    QMutexLocker locker(&m_mutex);
    QVariant oldValue = m_settings->value(key);
    
    m_settings->setValue(key, value);
    
    if (oldValue != value) {
        emit settingChanged(key, value, oldValue);
    }
    
    return true;
}

bool SettingsManager::contains(SettingKey key) const
{
    if (!m_initialized) {
        return false;
    }

    QMutexLocker locker(&m_mutex);
    return m_settings->contains(getKeyString(key));
}

bool SettingsManager::contains(const QString& key) const
{
    if (!m_initialized) {
        return false;
    }

    QMutexLocker locker(&m_mutex);
    return m_settings->contains(key);
}void SettingsManager::remove(SettingKey key)
{
    if (!m_initialized) {
        qWarning() << "SettingsManager not initialized";
        return;
    }

    QMutexLocker locker(&m_mutex);
    QString keyString = getKeyString(key);
    QVariant oldValue = m_settings->value(keyString);
    
    m_settings->remove(keyString);
    emit settingChanged(keyString, QVariant(), oldValue);
}

void SettingsManager::remove(const QString& key)
{
    if (!m_initialized) {
        qWarning() << "SettingsManager not initialized";
        return;
    }

    QMutexLocker locker(&m_mutex);
    QVariant oldValue = m_settings->value(key);
    
    m_settings->remove(key);
    emit settingChanged(key, QVariant(), oldValue);
}

void SettingsManager::sync()
{
    if (!m_initialized) {
        qWarning() << "SettingsManager not initialized";
        return;
    }

    QMutexLocker locker(&m_mutex);
    m_settings->sync();
}

QString SettingsManager::getSettingsPath() const
{
    if (!m_initialized || !m_settings) {
        return QString();
    }

    return m_settings->fileName();
}

QString SettingsManager::getKeyString(SettingKey key) const
{
    return m_keyMappings.value(key, QString());
}

QVariant SettingsManager::getDefaultValue(SettingKey key) const
{
    return m_defaultValues.value(key, QVariant());
}bool SettingsManager::validateValue(SettingKey key, const QVariant& value) const
{
    // Basic validation - can be extended as needed
    switch (key) {
        case SettingKey::DatSignature:
        case SettingKey::SprSignature:
            return value.canConvert<quint32>();
            
        case SettingKey::Extended:
        case SettingKey::Transparency:
        case SettingKey::FrameDurations:
        case SettingKey::ShowToolbar:
        case SettingKey::ShowStatusbar:
        case SettingKey::AutoSave:
        case SettingKey::DebugMode:
            return value.canConvert<bool>();
            
        case SettingKey::ClientDirectory:
        case SettingKey::Language:
        case SettingKey::Theme:
        case SettingKey::PluginDirectory:
        case SettingKey::LogLevel:
            return value.canConvert<QString>();
            
        case SettingKey::AutoSaveInterval:
        case SettingKey::CacheSize:
        case SettingKey::BackupCount:
            return value.canConvert<int>() && value.toInt() >= 0;
            
        case SettingKey::RecentFiles:
        case SettingKey::EnabledPlugins:
            return value.canConvert<QStringList>();
            
        case SettingKey::WindowGeometry:
        case SettingKey::WindowState:
            return value.canConvert<QByteArray>();
            
        case SettingKey::PluginSettings:
            return value.canConvert<QVariantMap>();
            
        default:
            return true; // Allow unknown keys
    }
}

SettingsManager::Category SettingsManager::getKeyCategory(SettingKey key) const
{
    return m_keyCategories.value(key, Category::Application);
}

QVariantMap SettingsManager::getCategorySettings(Category category) const
{
    if (!m_initialized) {
        return QVariantMap();
    }

    QMutexLocker locker(&m_mutex);
    QVariantMap result;
    
    for (auto it = m_keyCategories.constBegin(); it != m_keyCategories.constEnd(); ++it) {
        if (it.value() == category) {
            SettingKey key = it.key();
            QString keyString = getKeyString(key);
            QVariant value = m_settings->value(keyString, getDefaultValue(key));
            result[keyString] = value;
        }
    }
    
    return result;
}void SettingsManager::resetCategory(Category category)
{
    if (!m_initialized) {
        qWarning() << "SettingsManager not initialized";
        return;
    }

    QMutexLocker locker(&m_mutex);
    
    for (auto it = m_keyCategories.constBegin(); it != m_keyCategories.constEnd(); ++it) {
        if (it.value() == category) {
            SettingKey key = it.key();
            QString keyString = getKeyString(key);
            QVariant defaultValue = getDefaultValue(key);
            QVariant oldValue = m_settings->value(keyString);
            
            m_settings->setValue(keyString, defaultValue);
            
            if (oldValue != defaultValue) {
                emit settingChanged(keyString, defaultValue, oldValue);
            }
        }
    }
    
    QString categoryName;
    switch (category) {
        case Category::Application: categoryName = "Application"; break;
        case Category::Client: categoryName = "Client"; break;
        case Category::UI: categoryName = "UI"; break;
        case Category::Plugin: categoryName = "Plugin"; break;
        case Category::Advanced: categoryName = "Advanced"; break;
    }
    
    emit settingsReset(categoryName);
}

void SettingsManager::resetAll()
{
    if (!m_initialized) {
        qWarning() << "SettingsManager not initialized";
        return;
    }

    QMutexLocker locker(&m_mutex);
    
    for (auto it = m_keyMappings.constBegin(); it != m_keyMappings.constEnd(); ++it) {
        SettingKey key = it.key();
        QString keyString = it.value();
        QVariant defaultValue = getDefaultValue(key);
        QVariant oldValue = m_settings->value(keyString);
        
        m_settings->setValue(keyString, defaultValue);
        
        if (oldValue != defaultValue) {
            emit settingChanged(keyString, defaultValue, oldValue);
        }
    }
    
    emit settingsReset(QString());
}bool SettingsManager::importSettings(const QString& filePath)
{
    if (!m_initialized) {
        qWarning() << "SettingsManager not initialized";
        return false;
    }

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isReadable()) {
        qWarning() << "Cannot read settings file:" << filePath;
        return false;
    }

    // For now, implement basic QSettings import
    // Could be extended to support JSON or other formats
    QSettings importSettings(filePath, QSettings::IniFormat);
    
    QMutexLocker locker(&m_mutex);
    
    QStringList keys = importSettings.allKeys();
    for (const QString& key : keys) {
        QVariant value = importSettings.value(key);
        QVariant oldValue = m_settings->value(key);
        
        m_settings->setValue(key, value);
        
        if (oldValue != value) {
            emit settingChanged(key, value, oldValue);
        }
    }
    
    m_settings->sync();
    qDebug() << "Imported" << keys.size() << "settings from" << filePath;
    
    return true;
}

bool SettingsManager::exportSettings(const QString& filePath)
{
    if (!m_initialized) {
        qWarning() << "SettingsManager not initialized";
        return false;
    }

    QMutexLocker locker(&m_mutex);
    
    QSettings exportSettings(filePath, QSettings::IniFormat);
    
    QStringList keys = m_settings->allKeys();
    for (const QString& key : keys) {
        QVariant value = m_settings->value(key);
        exportSettings.setValue(key, value);
    }
    
    exportSettings.sync();
    
    if (exportSettings.status() != QSettings::NoError) {
        qWarning() << "Failed to export settings to" << filePath;
        return false;
    }
    
    qDebug() << "Exported" << keys.size() << "settings to" << filePath;
    return true;
}

} // namespace Core
} // namespace ItemEditor

#include "moc_settingsmanager.cpp"