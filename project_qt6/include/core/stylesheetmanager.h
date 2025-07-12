#ifndef STYLESHEETMANAGER_H
#define STYLESHEETMANAGER_H

#include <QObject>
#include <QString>
#include <QHash>

namespace ItemEditor {
namespace Core {

/**
 * @brief Manages application stylesheets and themes
 * 
 * This class handles loading, applying, and managing stylesheets for the
 * application. It supports multiple themes and provides dynamic theme switching.
 */
class StylesheetManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Available themes
     */
    enum class Theme {
        Dark,
        Light,
        System
    };

    /**
     * @brief Get singleton instance
     */
    static StylesheetManager& instance();

    /**
     * @brief Initialize stylesheet manager
     * @return true if successful
     */
    bool initialize();

    /**
     * @brief Apply theme to application
     * @param theme Theme to apply
     * @return true if successful
     */
    bool applyTheme(Theme theme);

    /**
     * @brief Get current theme
     * @return Current theme
     */
    Theme getCurrentTheme() const;

    /**
     * @brief Get stylesheet for theme
     * @param theme Theme identifier
     * @return Stylesheet content
     */
    QString getThemeStylesheet(Theme theme) const;

    /**
     * @brief Check if theme is available
     * @param theme Theme to check
     * @return true if available
     */
    bool isThemeAvailable(Theme theme) const;

    /**
     * @brief Get theme name as string
     * @param theme Theme identifier
     * @return Theme name
     */
    QString getThemeName(Theme theme) const;

signals:
    /**
     * @brief Emitted when theme changes
     * @param newTheme New theme
     * @param oldTheme Previous theme
     */
    void themeChanged(Theme newTheme, Theme oldTheme);

private:
    explicit StylesheetManager(QObject* parent = nullptr);
    ~StylesheetManager() override = default;

    // Disable copy and assignment
    StylesheetManager(const StylesheetManager&) = delete;
    StylesheetManager& operator=(const StylesheetManager&) = delete;

    /**
     * @brief Setup theme mappings
     */
    void setupThemes();

    // Static instance
    static StylesheetManager* s_instance;

    // Theme mappings
    QHash<Theme, QString> m_themeFiles;
    QHash<Theme, QString> m_themeNames;

    // Current state
    Theme m_currentTheme;
    bool m_initialized;
};

} // namespace Core
} // namespace ItemEditor

#endif // STYLESHEETMANAGER_H