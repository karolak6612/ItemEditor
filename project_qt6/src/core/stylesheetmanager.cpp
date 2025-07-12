#include "core/stylesheetmanager.h"
#include "core/resourcemanager.h"
#include <QApplication>
#include <QDebug>

namespace ItemEditor {
namespace Core {

StylesheetManager* StylesheetManager::s_instance = nullptr;

StylesheetManager& StylesheetManager::instance()
{
    if (!s_instance) {
        s_instance = new StylesheetManager();
    }
    return *s_instance;
}

StylesheetManager::StylesheetManager(QObject* parent)
    : QObject(parent)
    , m_currentTheme(Theme::Dark)
    , m_initialized(false)
{
    setupThemes();
}

void StylesheetManager::setupThemes()
{
    // Map themes to resource files
    m_themeFiles[Theme::Dark] = "dark";
    m_themeFiles[Theme::Light] = "light";  // Future implementation
    m_themeFiles[Theme::System] = "system"; // Future implementation

    // Theme display names
    m_themeNames[Theme::Dark] = "Dark";
    m_themeNames[Theme::Light] = "Light";
    m_themeNames[Theme::System] = "System";
}

bool StylesheetManager::initialize()
{
    if (m_initialized) {
        return true;
    }

    qDebug() << "Initializing StylesheetManager...";

    // Ensure ResourceManager is initialized
    if (!ResourceManager::instance().initialize()) {
        qWarning() << "Failed to initialize ResourceManager";
        return false;
    }

    // Apply default theme
    if (!applyTheme(Theme::Dark)) {
        qWarning() << "Failed to apply default theme";
        return false;
    }

    m_initialized = true;
    qDebug() << "StylesheetManager initialized successfully";
    return true;
}bool StylesheetManager::applyTheme(Theme theme)
{
    if (!isThemeAvailable(theme)) {
        qWarning() << "Theme not available:" << getThemeName(theme);
        return false;
    }

    QString stylesheet = getThemeStylesheet(theme);
    if (stylesheet.isEmpty()) {
        qWarning() << "Failed to load stylesheet for theme:" << getThemeName(theme);
        return false;
    }

    // Apply to application
    qApp->setStyleSheet(stylesheet);

    Theme oldTheme = m_currentTheme;
    m_currentTheme = theme;

    qDebug() << "Applied theme:" << getThemeName(theme);
    emit themeChanged(theme, oldTheme);

    return true;
}

StylesheetManager::Theme StylesheetManager::getCurrentTheme() const
{
    return m_currentTheme;
}

QString StylesheetManager::getThemeStylesheet(Theme theme) const
{
    if (!m_themeFiles.contains(theme)) {
        qWarning() << "No stylesheet file for theme:" << getThemeName(theme);
        return QString();
    }

    QString themeFile = m_themeFiles[theme];
    return ResourceManager::instance().getStylesheet(themeFile);
}

bool StylesheetManager::isThemeAvailable(Theme theme) const
{
    // For now, only Dark theme is available
    return theme == Theme::Dark;
}

QString StylesheetManager::getThemeName(Theme theme) const
{
    return m_themeNames.value(theme, "Unknown");
}

} // namespace Core
} // namespace ItemEditor

#include "moc_stylesheetmanager.cpp"