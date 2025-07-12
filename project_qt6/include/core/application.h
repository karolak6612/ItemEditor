#ifndef APPLICATION_H
#define APPLICATION_H

#include "core/applicationbase.h"
#include <QTimer>
#include <QFileSystemWatcher>
#include <QTranslator>
#include <QSystemTrayIcon>
#include <QMenu>
#include <memory>

namespace ItemEditor {
namespace Core {

class MainWindow;
// class PluginManager; // TODO: Uncomment when PluginManager is implemented

/**
 * @brief Main application class for ItemEditor Qt6
 * 
 * This class extends ApplicationBase to provide a complete application framework
 * including window management, plugin system integration, event handling,
 * and application lifecycle management.
 */
class Application : public ::Core::ApplicationBase
{
    Q_OBJECT

public:
    /**
     * @brief Application states
     */
    enum class State {
        Initializing,
        Running,
        Suspended,
        Terminating
    };

    /**
     * @brief Application modes
     */
    enum class Mode {
        Normal,
        Debug,
        Portable,
        Service
    };

    explicit Application(int &argc, char **argv);
    ~Application() override;

    /**
     * @brief Initialize the application
     * @return true if successful
     */
    bool initialize() override;

    /**
     * @brief Get application instance
     * @return Application instance
     */
    static Application* instance();

    /**
     * @brief Get current application state
     * @return Current state
     */
    State getState() const { return m_state; }

    /**
     * @brief Get application mode
     * @return Current mode
     */
    Mode getMode() const { return m_mode; }

    /**
     * @brief Set application mode
     * @param mode Application mode
     */
    void setMode(Mode mode) { m_mode = mode; }

    /**
     * @brief Get main window
     * @return Main window instance
     */
    // MainWindow* getMainWindow() const { return m_mainWindow.get(); } // TODO: Uncomment when MainWindow is implemented

    /**
     * @brief Create and show main window
     * @return true if successful
     */
    bool createMainWindow();

    /**
     * @brief Get plugin manager
     * @return Plugin manager instance
     */
    // PluginManager* getPluginManager() const { return m_pluginManager.get(); } // TODO: Uncomment when PluginManager is implemented

    /**
     * @brief Process command line arguments
     * @param arguments Command line arguments
     * @return true if successful
     */
    bool processCommandLine(const QStringList& arguments);

    /**
     * @brief Check if application is ready
     * @return true if ready
     */
    bool isReady() const { return m_state == State::Running; }

    /**
     * @brief Restart application
     * @param arguments Optional new arguments
     */
    void restart(const QStringList& arguments = QStringList());

    /**
     * @brief Get uptime in seconds
     * @return Uptime in seconds
     */
    qint64 getUptime() const;

    /**
     * @brief Enable/disable auto-save
     * @param enabled Auto-save enabled
     * @param interval Interval in seconds
     */
    void setAutoSave(bool enabled, int interval = 300);

    /**
     * @brief Check for updates
     */
    void checkForUpdates();

signals:
    /**
     * @brief Emitted when application state changes
     * @param newState New state
     * @param oldState Previous state
     */
    void stateChanged(State newState, State oldState);

    /**
     * @brief Emitted when main window is created
     * @param window Main window instance
     */
    void mainWindowCreated(MainWindow* window);

    /**
     * @brief Emitted when application is ready
     */
    void applicationReady();

    /**
     * @brief Emitted when auto-save is triggered
     */
    void autoSaveTriggered();

    /**
     * @brief Emitted when update is available
     * @param version New version
     * @param url Download URL
     */
    void updateAvailable(const QString& version, const QString& url);

protected:
    /**
     * @brief Handle application events
     * @param receiver Event receiver
     * @param event Event object
     * @return true if handled
     */
    bool notify(QObject* receiver, QEvent* event) override;

    /**
     * @brief Setup application framework
     */
    void setupFramework();

    /**
     * @brief Setup event handling
     */
    void setupEventHandling();

    /**
     * @brief Setup auto-save system
     */
    void setupAutoSave();

    /**
     * @brief Setup system tray
     */
    void setupSystemTray();

    /**
     * @brief Setup file monitoring
     */
    void setupFileMonitoring();

    /**
     * @brief Setup internationalization
     */
    void setupInternationalization();

    /**
     * @brief Change application state
     * @param newState New state
     */
    void setState(State newState);

private slots:
    /**
     * @brief Handle auto-save timer
     */
    void onAutoSaveTimer();

    /**
     * @brief Handle file system changes
     * @param path Changed file path
     */
    void onFileChanged(const QString& path);

    /**
     * @brief Handle directory changes
     * @param path Changed directory path
     */
    void onDirectoryChanged(const QString& path);

    /**
     * @brief Handle system tray activation
     * @param reason Activation reason
     */
    void onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason);

    /**
     * @brief Handle language change
     */
    void onLanguageChanged();

private:
    // Application state
    State m_state;
    Mode m_mode;
    QDateTime m_startTime;

    // Core components
    // std::unique_ptr<MainWindow> m_mainWindow; // TODO: Uncomment when MainWindow is implemented
    // std::unique_ptr<PluginManager> m_pluginManager; // TODO: Uncomment when PluginManager is implemented

    // Framework components
    std::unique_ptr<QTimer> m_autoSaveTimer;
    std::unique_ptr<QFileSystemWatcher> m_fileWatcher;
    std::unique_ptr<QTranslator> m_translator;
    std::unique_ptr<QSystemTrayIcon> m_systemTray;
    std::unique_ptr<QMenu> m_trayMenu;

    // Configuration
    bool m_autoSaveEnabled;
    int m_autoSaveInterval;
    bool m_systemTrayEnabled;
    QString m_currentLanguage;

    // Static instance
    static Application* s_instance;
};

} // namespace Core
} // namespace ItemEditor

#endif // APPLICATION_H