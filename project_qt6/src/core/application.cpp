#include "core/application.h"
#include "core/settingsmanager.h"
#include "core/appsettings.h"
#include <QCommandLineParser>
#include <QStandardPaths>
#include <QDir>
#include <QEvent>
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>
#include <QCoreApplication>
#include <QProcess>

namespace ItemEditor {
namespace Core {

Application* Application::s_instance = nullptr;

Application::Application(int &argc, char **argv)
    : ApplicationBase(argc, argv)
    , m_state(State::Initializing)
    , m_mode(Mode::Normal)
    , m_startTime(QDateTime::currentDateTime())
    , m_autoSaveEnabled(false)
    , m_autoSaveInterval(300)
    , m_systemTrayEnabled(false)
    , m_currentLanguage("en")
{
    s_instance = this;
    
    // Set application properties
    setApplicationDisplayName(applicationName());
    setApplicationVersion(applicationVersion());
    setOrganizationName(organizationName());
    setOrganizationDomain(organizationDomain());
    
    // Enable high DPI support
    setAttribute(Qt::AA_EnableHighDpiScaling, true);
    setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    
    qDebug() << "Application created with PID:" << applicationPid();
}

Application::~Application()
{
    setState(State::Terminating);
    
    // Save settings before shutdown
    if (isReady()) {
        AppSettings::sync();
    }
    
    qDebug() << "Application destroyed";
}

Application* Application::instance()
{
    return s_instance;
}bool Application::initialize()
{
    qDebug() << "Initializing Application framework...";
    
    // Initialize base application
    if (!ApplicationBase::initialize()) {
        qCritical() << "Failed to initialize ApplicationBase";
        return false;
    }
    
    // Setup framework components
    setupFramework();
    
    // Setup event handling
    setupEventHandling();
    
    // Setup internationalization
    setupInternationalization();
    
    // Setup auto-save system
    setupAutoSave();
    
    // Setup file monitoring
    setupFileMonitoring();
    
    // Setup system tray if available
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        setupSystemTray();
    }
    
    // Change state to running
    setState(State::Running);
    
    qDebug() << "Application framework initialized successfully";
    emit applicationReady();
    
    return true;
}

void Application::setupFramework()
{
    qDebug() << "Setting up application framework...";
    
    // Load application mode from settings
    QString modeString = SettingsManager::instance().getValue("Application/Mode", "Normal").toString();
    if (modeString == "Debug") {
        m_mode = Mode::Debug;
    } else if (modeString == "Portable") {
        m_mode = Mode::Portable;
    } else if (modeString == "Service") {
        m_mode = Mode::Service;
    } else {
        m_mode = Mode::Normal;
    }
    
    // Setup application paths based on mode
    if (m_mode == Mode::Portable) {
        // In portable mode, use application directory for all data
        QDir appDir(applicationDirPath());
        appDir.mkpath("data");
        appDir.mkpath("plugins");
        appDir.mkpath("logs");
    }
    
    qDebug() << "Application mode:" << static_cast<int>(m_mode);
}void Application::setupEventHandling()
{
    qDebug() << "Setting up event handling...";
    
    // Install event filter for global event handling
    installEventFilter(this);
    
    // Connect to application signals
    connect(this, &QApplication::aboutToQuit, this, [this]() {
        qDebug() << "Application about to quit";
        setState(State::Terminating);
        
        // Save current state
        // TODO: Uncomment when MainWindow is implemented
        // if (m_mainWindow) {
        //     AppSettings::setWindowGeometry(m_mainWindow->saveGeometry());
        //     AppSettings::setWindowState(m_mainWindow->saveState());
        // }
        
        AppSettings::sync();
    });
    
    connect(this, &QApplication::applicationStateChanged, this, [this](Qt::ApplicationState state) {
        qDebug() << "Application state changed to:" << state;
        
        if (state == Qt::ApplicationSuspended) {
            setState(State::Suspended);
        } else if (state == Qt::ApplicationActive && m_state == State::Suspended) {
            setState(State::Running);
        }
    });
}

void Application::setupAutoSave()
{
    qDebug() << "Setting up auto-save system...";
    
    m_autoSaveTimer = std::make_unique<QTimer>(this);
    m_autoSaveTimer->setSingleShot(false);
    
    connect(m_autoSaveTimer.get(), &QTimer::timeout, this, &Application::onAutoSaveTimer);
    
    // Load auto-save settings
    m_autoSaveEnabled = AppSettings::autoSave();
    m_autoSaveInterval = AppSettings::autoSaveInterval();
    
    if (m_autoSaveEnabled) {
        m_autoSaveTimer->start(m_autoSaveInterval * 1000);
        qDebug() << "Auto-save enabled with interval:" << m_autoSaveInterval << "seconds";
    }
}void Application::setupFileMonitoring()
{
    qDebug() << "Setting up file monitoring...";
    
    m_fileWatcher = std::make_unique<QFileSystemWatcher>(this);
    
    connect(m_fileWatcher.get(), &QFileSystemWatcher::fileChanged, 
            this, &Application::onFileChanged);
    connect(m_fileWatcher.get(), &QFileSystemWatcher::directoryChanged, 
            this, &Application::onDirectoryChanged);
    
    // Monitor important directories
    QStringList watchPaths;
    watchPaths << userDataDirectory();
    watchPaths << pluginsDirectory();
    
    for (const QString& path : watchPaths) {
        if (QDir(path).exists()) {
            m_fileWatcher->addPath(path);
            qDebug() << "Monitoring directory:" << path;
        }
    }
}

void Application::setupSystemTray()
{
    qDebug() << "Setting up system tray...";
    
    m_systemTray = std::make_unique<QSystemTrayIcon>(this);
    m_trayMenu = std::make_unique<QMenu>();
    
    // Setup tray menu
    m_trayMenu->addAction("Show", this, [this]() {
        // TODO: Uncomment when MainWindow is implemented
        // if (m_mainWindow) {
        //     m_mainWindow->show();
        //     m_mainWindow->raise();
        //     m_mainWindow->activateWindow();
        // }
    });
    
    m_trayMenu->addSeparator();
    
    m_trayMenu->addAction("Exit", this, [this]() {
        quit();
    });
    
    m_systemTray->setContextMenu(m_trayMenu.get());
    
    connect(m_systemTray.get(), &QSystemTrayIcon::activated, 
            this, &Application::onSystemTrayActivated);
    
    // Set tray icon (will be set when resources are available)
    m_systemTray->setToolTip(applicationDisplayName());
    
    m_systemTrayEnabled = true;
}void Application::setupInternationalization()
{
    qDebug() << "Setting up internationalization...";
    
    m_translator = std::make_unique<QTranslator>(this);
    
    // Load language from settings
    m_currentLanguage = AppSettings::language();
    
    // Load translation file
    QString translationFile = QString("itemeditor_%1").arg(m_currentLanguage);
    QString translationPath = QStandardPaths::locate(QStandardPaths::AppDataLocation, 
                                                     QString("translations/%1.qm").arg(translationFile));
    
    if (!translationPath.isEmpty() && m_translator->load(translationPath)) {
        installTranslator(m_translator.get());
        qDebug() << "Loaded translation:" << translationFile;
    } else {
        qDebug() << "No translation found for language:" << m_currentLanguage;
    }
}

bool Application::processCommandLine(const QStringList& arguments)
{
    QCommandLineParser parser;
    parser.setApplicationDescription("ItemEditor Qt6 - OTB File Editor");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // Add command line options
    QCommandLineOption debugOption(QStringList() << "d" << "debug", "Enable debug mode");
    parser.addOption(debugOption);
    
    QCommandLineOption portableOption(QStringList() << "p" << "portable", "Run in portable mode");
    parser.addOption(portableOption);
    
    QCommandLineOption configOption("config", "Configuration file path", "file");
    parser.addOption(configOption);
    
    QCommandLineOption logLevelOption("log-level", "Log level (Debug, Info, Warning, Critical)", "level");
    parser.addOption(logLevelOption);
    
    // Parse arguments
    parser.process(arguments);
    
    // Process options
    if (parser.isSet(debugOption)) {
        setMode(Mode::Debug);
        qDebug() << "Debug mode enabled via command line";
    }
    
    if (parser.isSet(portableOption)) {
        setMode(Mode::Portable);
        qDebug() << "Portable mode enabled via command line";
    }
    
    if (parser.isSet(configOption)) {
        QString configFile = parser.value(configOption);
        qDebug() << "Custom config file:" << configFile;
        // TODO: Load custom configuration
    }
    
    if (parser.isSet(logLevelOption)) {
        QString logLevel = parser.value(logLevelOption);
        qDebug() << "Log level set to:" << logLevel;
        // TODO: Set log level
    }
    
    return true;
}bool Application::createMainWindow()
{
    // TODO: Implement when MainWindow is available
    // if (m_mainWindow) {
    //     qWarning() << "Main window already exists";
    //     return true;
    // }
    
    qDebug() << "Creating main window...";
    
    // Create main window (placeholder for now)
    // m_mainWindow = std::make_unique<MainWindow>();
    
    // For now, create a simple test window
    // This will be replaced with the actual MainWindow implementation
    
    // emit mainWindowCreated(m_mainWindow.get()); // TODO: Uncomment when MainWindow is implemented
    
    qDebug() << "Main window created successfully";
    return true;
}

void Application::setState(State newState)
{
    if (m_state == newState) {
        return;
    }
    
    State oldState = m_state;
    m_state = newState;
    
    qDebug() << "Application state changed from" << static_cast<int>(oldState) 
             << "to" << static_cast<int>(newState);
    
    emit stateChanged(newState, oldState);
}

qint64 Application::getUptime() const
{
    return m_startTime.secsTo(QDateTime::currentDateTime());
}

void Application::setAutoSave(bool enabled, int interval)
{
    m_autoSaveEnabled = enabled;
    m_autoSaveInterval = interval;
    
    if (m_autoSaveTimer) {
        if (enabled) {
            m_autoSaveTimer->start(interval * 1000);
        } else {
            m_autoSaveTimer->stop();
        }
    }
    
    // Save settings
    AppSettings::setAutoSave(enabled);
    AppSettings::setAutoSaveInterval(interval);
    
    qDebug() << "Auto-save" << (enabled ? "enabled" : "disabled") 
             << "with interval:" << interval << "seconds";
}void Application::restart(const QStringList& arguments)
{
    qDebug() << "Restarting application with arguments:" << arguments;
    
    // Save current state
    AppSettings::sync();
    
    // Prepare restart
    QStringList args = arguments.isEmpty() ? QCoreApplication::arguments() : arguments;
    args.removeFirst(); // Remove program name
    
    // Schedule restart
    QTimer::singleShot(100, this, [this, args]() {
        QProcess::startDetached(applicationFilePath(), args);
        quit();
    });
}

void Application::checkForUpdates()
{
    qDebug() << "Checking for updates...";
    // TODO: Implement update checking
    // This would typically involve checking a remote server for new versions
}

bool Application::notify(QObject* receiver, QEvent* event)
{
    try {
        return ApplicationBase::notify(receiver, event);
    } catch (const std::exception& e) {
        qCritical() << "Exception in event handling:" << e.what();
        return false;
    } catch (...) {
        qCritical() << "Unknown exception in event handling";
        return false;
    }
}

// Slot implementations
void Application::onAutoSaveTimer()
{
    qDebug() << "Auto-save triggered";
    emit autoSaveTriggered();
    
    // Auto-save logic would go here
    // For now, just sync settings
    AppSettings::sync();
}

void Application::onFileChanged(const QString& path)
{
    qDebug() << "File changed:" << path;
    // Handle file changes as needed
}

void Application::onDirectoryChanged(const QString& path)
{
    qDebug() << "Directory changed:" << path;
    // Handle directory changes as needed
}void Application::onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::DoubleClick:
            // TODO: Uncomment when MainWindow is implemented
            // if (m_mainWindow) {
            //     if (m_mainWindow->isVisible()) {
            //         m_mainWindow->hide();
            //     } else {
            //         m_mainWindow->show();
            //         m_mainWindow->raise();
            //         m_mainWindow->activateWindow();
            //     }
            // }
            break;
            
        case QSystemTrayIcon::MiddleClick:
            // Handle middle click if needed
            break;
            
        default:
            break;
    }
}

void Application::onLanguageChanged()
{
    qDebug() << "Language changed to:" << m_currentLanguage;
    
    // Reload translation
    if (m_translator) {
        removeTranslator(m_translator.get());
    }
    
    setupInternationalization();
    
    // Notify UI components to retranslate
    // This would typically involve emitting a signal that UI components listen to
}

} // namespace Core
} // namespace ItemEditor

#include "moc_application.cpp"