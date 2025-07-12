#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QIcon>
#include <QString>
#include <QHash>
#include <QMutex>

namespace ItemEditor {
namespace Core {

/**
 * @brief Central resource manager for handling icons, images, and stylesheets
 * 
 * This class provides centralized access to all application resources including
 * icons, images, stylesheets, and other assets. It implements caching for
 * performance and provides type-safe access methods.
 */
class ResourceManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Resource types for categorization
     */
    enum class ResourceType {
        Icon,
        Image,
        Stylesheet,
        Other
    };

    /**
     * @brief Predefined icon identifiers matching C# resources
     */
    enum class IconId {
        // Application
        FormIcon,
        
        // Toolbar
        NewIcon,
        OpenIcon,
        SaveIcon,
        SaveAsIcon,
        ReloadIcon,
        DuplicateIcon,
        FindIcon,
        InfoIcon,
        
        // Dialog
        Help,
        Find,
        AboutBackground,
        
        // File Operations
        Disk,
        DiskMultiple,
        FolderPage,
        ApplicationForm,
        
        // Pages
        Page,
        PageWhite,
        PageWhite1,
        PageWhiteCopy,
        PageDelete,
        Reload
    };

    /**
     * @brief Get singleton instance
     */
    static ResourceManager& instance();

    /**
     * @brief Initialize resource system
     * @return true if successful
     */
    bool initialize();

    /**
     * @brief Get icon by identifier
     * @param iconId Predefined icon identifier
     * @return QIcon object
     */
    QIcon getIcon(IconId iconId) const;

    /**
     * @brief Get icon by resource path
     * @param resourcePath Path to resource (e.g., ":/icons/NewIcon.png")
     * @return QIcon object
     */
    QIcon getIcon(const QString& resourcePath) const;

    /**
     * @brief Get pixmap by identifier
     * @param iconId Predefined icon identifier
     * @return QPixmap object
     */
    QPixmap getPixmap(IconId iconId) const;

    /**
     * @brief Get pixmap by resource path
     * @param resourcePath Path to resource
     * @return QPixmap object
     */
    QPixmap getPixmap(const QString& resourcePath) const;

    /**
     * @brief Get stylesheet content
     * @param stylesheetName Name of stylesheet (e.g., "dark")
     * @return Stylesheet content as string
     */
    QString getStylesheet(const QString& stylesheetName) const;

    /**
     * @brief Check if resource exists
     * @param resourcePath Path to resource
     * @return true if resource exists
     */
    bool resourceExists(const QString& resourcePath) const;

    /**
     * @brief Get resource path for icon identifier
     * @param iconId Icon identifier
     * @return Resource path string
     */
    QString getIconPath(IconId iconId) const;

    /**
     * @brief Clear resource cache
     */
    void clearCache();

    /**
     * @brief Get cache statistics
     * @return Number of cached items
     */
    int getCacheSize() const;

private:
    explicit ResourceManager(QObject* parent = nullptr);
    ~ResourceManager() override = default;

    // Disable copy and assignment
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    /**
     * @brief Setup icon path mappings
     */
    void setupIconPaths();

    /**
     * @brief Load and cache resource
     * @param resourcePath Path to resource
     * @return QPixmap object
     */
    QPixmap loadAndCachePixmap(const QString& resourcePath) const;

    // Static instance
    static ResourceManager* s_instance;

    // Icon path mappings
    QHash<IconId, QString> m_iconPaths;

    // Resource cache
    mutable QHash<QString, QPixmap> m_pixmapCache;
    mutable QHash<QString, QString> m_stylesheetCache;

    // Thread safety
    mutable QMutex m_cacheMutex;

    // Initialization flag
    bool m_initialized;
};

} // namespace Core
} // namespace ItemEditor

#endif // RESOURCEMANAGER_H