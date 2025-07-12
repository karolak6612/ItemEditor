#include "core/applicationbase.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDebug>
#include <QLoggingCategory>
#include "core/resourcemanager.h"
#include "core/stylesheetmanager.h"
#include "core/settingsmanager.h"

Q_LOGGING_CATEGORY(appLog, "app")

namespace Core {

ApplicationBase* ApplicationBase::s_instance = nullptr;

ApplicationBase::ApplicationBase(int &argc, char **argv)
    : QApplication(argc, argv)
{
    s_instance = this;
    
    // Set application properties
    setApplicationName(applicationName());
    setApplicationVersion(applicationVersion());
    setOrganizationName(organizationName());
    setOrganizationDomain(organizationDomain());
    
    // Initialize logging first
    initializeLogging();
    
    // Setup directories and settings
    setupDirectories();
    setupSettings();
    
    // Initialize resource management
    initializeResources();
    
    // Connect cleanup signal
    connect(this, &QApplication::aboutToQuit, this, &ApplicationBase::onAboutToQuit);
    
    qCInfo(appLog) << "ApplicationBase initialized successfully";
}ApplicationBase::~ApplicationBase()
{
    qCInfo(appLog) << "ApplicationBase shutting down";
    s_instance = nullptr;
}

ApplicationBase* ApplicationBase::instance()
{
    return s_instance;
}

QString ApplicationBase::applicationDirectory() const
{
    return m_applicationDir;
}

QString ApplicationBase::userDataDirectory() const
{
    return m_userDataDir;
}

QString ApplicationBase::pluginsDirectory() const
{
    return m_pluginsDir;
}

QString ApplicationBase::resourcesDirectory() const
{
    return m_resourcesDir;
}

void ApplicationBase::setupDirectories()
{
    // Application directory (where executable is located)
    m_applicationDir = QCoreApplication::applicationDirPath();
    
    // User data directory
    m_userDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(m_userDataDir);
    
    // Plugins directory (relative to application)
    m_pluginsDir = QDir(m_applicationDir).absoluteFilePath("plugins");
    
    // Resources directory (relative to application)
    m_resourcesDir = QDir(m_applicationDir).absoluteFilePath("resources");
    
    qCDebug(appLog) << "Application directory:" << m_applicationDir;
    qCDebug(appLog) << "User data directory:" << m_userDataDir;
    qCDebug(appLog) << "Plugins directory:" << m_pluginsDir;
    qCDebug(appLog) << "Resources directory:" << m_resourcesDir;
}void ApplicationBase::setupSettings()
{
    m_settings = std::make_unique<QSettings>();
    loadSettings();
}

void ApplicationBase::saveSettings()
{
    if (m_settings) {
        m_settings->sync();
        qCDebug(appLog) << "Settings saved";
    }
}

void ApplicationBase::loadSettings()
{
    if (m_settings) {
        qCDebug(appLog) << "Settings loaded from:" << m_settings->fileName();
    }
}

void ApplicationBase::initializeLogging()
{
    // Enable all logging categories for debug builds
#ifdef QT_DEBUG
    QLoggingCategory::setFilterRules("*=true");
#else
    QLoggingCategory::setFilterRules("*.debug=false");
#endif
}

void ApplicationBase::logMessage(QtMsgType type, const QString &message)
{
    switch (type) {
    case QtDebugMsg:
        qCDebug(appLog) << message;
        break;
    case QtInfoMsg:
        qCInfo(appLog) << message;
        break;
    case QtWarningMsg:
        qCWarning(appLog) << message;
        break;
    case QtCriticalMsg:
        qCCritical(appLog) << message;
        break;
    case QtFatalMsg:
        qCCritical(appLog) << message;
        break;
    }
}bool ApplicationBase::initialize()
{
    qCInfo(appLog) << "Initializing application...";
    return true;
}

void ApplicationBase::onAboutToQuit()
{
    qCInfo(appLog) << "Application about to quit";
    saveSettings();
}

void ApplicationBase::initializeResources()
{
    qCDebug(appLog) << "Initializing resource management...";
    
    // Initialize settings manager first
    if (!ItemEditor::Core::SettingsManager::instance().initialize(organizationName(), applicationName())) {
        qCCritical(appLog) << "Failed to initialize SettingsManager";
        return;
    }
    
    // Initialize resource manager
    if (!ItemEditor::Core::ResourceManager::instance().initialize()) {
        qCCritical(appLog) << "Failed to initialize ResourceManager";
        return;
    }
    
    // Initialize stylesheet manager
    if (!ItemEditor::Core::StylesheetManager::instance().initialize()) {
        qCCritical(appLog) << "Failed to initialize StylesheetManager";
        return;
    }
    
    qCDebug(appLog) << "Resource management initialized successfully";
}

} // namespace Core

#include "moc_applicationbase.cpp"