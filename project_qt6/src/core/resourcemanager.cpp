#include "core/resourcemanager.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMutexLocker>
#include <QDir>

namespace ItemEditor {
namespace Core {

ResourceManager* ResourceManager::s_instance = nullptr;

ResourceManager& ResourceManager::instance()
{
    if (!s_instance) {
        s_instance = new ResourceManager();
    }
    return *s_instance;
}

ResourceManager::ResourceManager(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
{
    setupIconPaths();
}

bool ResourceManager::initialize()
{
    if (m_initialized) {
        return true;
    }

    qDebug() << "Initializing ResourceManager...";

    // Verify that Qt resource system is available
    if (!QDir(":/").exists()) {
        qWarning() << "Qt resource system not available";
        return false;
    }

    // Verify critical resources exist
    QStringList criticalResources = {
        ":/dark.qss",
        ":/icons/FormIcon.png",
        ":/icons/NewIcon.png",
        ":/icons/OpenIcon.png",
        ":/icons/SaveIcon.png"
    };

    for (const QString& resource : criticalResources) {
        if (!resourceExists(resource)) {
            qWarning() << "Critical resource missing:" << resource;
            return false;
        }
    }

    m_initialized = true;
    qDebug() << "ResourceManager initialized successfully";
    return true;
}void ResourceManager::setupIconPaths()
{
    // Application icons
    m_iconPaths[IconId::FormIcon] = ":/icons/FormIcon.png";
    
    // Toolbar icons
    m_iconPaths[IconId::NewIcon] = ":/icons/NewIcon.png";
    m_iconPaths[IconId::OpenIcon] = ":/icons/OpenIcon.png";
    m_iconPaths[IconId::SaveIcon] = ":/icons/SaveIcon.png";
    m_iconPaths[IconId::SaveAsIcon] = ":/icons/SaveAsIcon.png";
    m_iconPaths[IconId::ReloadIcon] = ":/icons/ReloadIcon.png";
    m_iconPaths[IconId::DuplicateIcon] = ":/icons/DuplicateIcon.png";
    m_iconPaths[IconId::FindIcon] = ":/icons/FindIcon.png";
    m_iconPaths[IconId::InfoIcon] = ":/icons/InfoIcon.png";
    
    // Dialog icons
    m_iconPaths[IconId::Help] = ":/icons/help.png";
    m_iconPaths[IconId::Find] = ":/icons/find.png";
    m_iconPaths[IconId::AboutBackground] = ":/icons/about_background.png";
    
    // File operation icons
    m_iconPaths[IconId::Disk] = ":/icons/disk.png";
    m_iconPaths[IconId::DiskMultiple] = ":/icons/disk_multiple.png";
    m_iconPaths[IconId::FolderPage] = ":/icons/folder_page.png";
    m_iconPaths[IconId::ApplicationForm] = ":/icons/application_form.png";
    
    // Page icons
    m_iconPaths[IconId::Page] = ":/icons/page.png";
    m_iconPaths[IconId::PageWhite] = ":/icons/page_white.png";
    m_iconPaths[IconId::PageWhite1] = ":/icons/page_white1.png";
    m_iconPaths[IconId::PageWhiteCopy] = ":/icons/page_white_copy.png";
    m_iconPaths[IconId::PageDelete] = ":/icons/page_delete.png";
    m_iconPaths[IconId::Reload] = ":/icons/reload.png";
}

QIcon ResourceManager::getIcon(IconId iconId) const
{
    if (!m_initialized) {
        qWarning() << "ResourceManager not initialized";
        return QIcon();
    }

    QString path = getIconPath(iconId);
    if (path.isEmpty()) {
        qWarning() << "No path found for icon ID:" << static_cast<int>(iconId);
        return QIcon();
    }

    return getIcon(path);
}QIcon ResourceManager::getIcon(const QString& resourcePath) const
{
    if (!m_initialized) {
        qWarning() << "ResourceManager not initialized";
        return QIcon();
    }

    QPixmap pixmap = getPixmap(resourcePath);
    if (pixmap.isNull()) {
        qWarning() << "Failed to load icon:" << resourcePath;
        return QIcon();
    }

    return QIcon(pixmap);
}

QPixmap ResourceManager::getPixmap(IconId iconId) const
{
    QString path = getIconPath(iconId);
    if (path.isEmpty()) {
        qWarning() << "No path found for icon ID:" << static_cast<int>(iconId);
        return QPixmap();
    }

    return getPixmap(path);
}

QPixmap ResourceManager::getPixmap(const QString& resourcePath) const
{
    if (!m_initialized) {
        qWarning() << "ResourceManager not initialized";
        return QPixmap();
    }

    return loadAndCachePixmap(resourcePath);
}

QString ResourceManager::getStylesheet(const QString& stylesheetName) const
{
    if (!m_initialized) {
        qWarning() << "ResourceManager not initialized";
        return QString();
    }

    QMutexLocker locker(&m_cacheMutex);
    
    // Check cache first
    if (m_stylesheetCache.contains(stylesheetName)) {
        return m_stylesheetCache[stylesheetName];
    }

    // Load stylesheet
    QString resourcePath = QString(":/%1.qss").arg(stylesheetName);
    QFile file(resourcePath);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open stylesheet:" << resourcePath;
        return QString();
    }

    QTextStream stream(&file);
    QString content = stream.readAll();
    file.close();

    // Cache the content
    m_stylesheetCache[stylesheetName] = content;
    
    qDebug() << "Loaded stylesheet:" << stylesheetName << "(" << content.length() << "chars)";
    return content;
}bool ResourceManager::resourceExists(const QString& resourcePath) const
{
    return QFile::exists(resourcePath);
}

QString ResourceManager::getIconPath(IconId iconId) const
{
    return m_iconPaths.value(iconId, QString());
}

void ResourceManager::clearCache()
{
    QMutexLocker locker(&m_cacheMutex);
    m_pixmapCache.clear();
    m_stylesheetCache.clear();
    qDebug() << "Resource cache cleared";
}

int ResourceManager::getCacheSize() const
{
    QMutexLocker locker(&m_cacheMutex);
    return m_pixmapCache.size() + m_stylesheetCache.size();
}

QPixmap ResourceManager::loadAndCachePixmap(const QString& resourcePath) const
{
    QMutexLocker locker(&m_cacheMutex);
    
    // Check cache first
    if (m_pixmapCache.contains(resourcePath)) {
        return m_pixmapCache[resourcePath];
    }

    // Load pixmap
    QPixmap pixmap(resourcePath);
    
    if (pixmap.isNull()) {
        qWarning() << "Failed to load pixmap:" << resourcePath;
        return QPixmap();
    }

    // Cache the pixmap
    m_pixmapCache[resourcePath] = pixmap;
    
    qDebug() << "Loaded and cached pixmap:" << resourcePath 
             << "(" << pixmap.width() << "x" << pixmap.height() << ")";
    
    return pixmap;
}

} // namespace Core
} // namespace ItemEditor

#include "moc_resourcemanager.cpp"