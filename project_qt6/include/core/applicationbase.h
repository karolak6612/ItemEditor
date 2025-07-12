#pragma once

#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QString>
#include <QSettings>
#include <QLoggingCategory>
#include <memory>

Q_DECLARE_LOGGING_CATEGORY(appLog)

namespace Core {

/**
 * @brief Base application class providing common functionality
 * 
 * This class serves as the foundation for the ItemEditor application,
 * providing common services like settings management, logging,
 * and application-wide utilities.
 */
class ApplicationBase : public QApplication
{
    Q_OBJECT

public:
    explicit ApplicationBase(int &argc, char **argv);
    virtual ~ApplicationBase();

    // Application information
    static QString applicationName() { return "ItemEditor Qt6"; }
    static QString applicationVersion() { return "2.0.0"; }
    static QString organizationName() { return "OTTools"; }
    static QString organizationDomain() { return "github.com/ottools"; }

    // Directory management
    QString applicationDirectory() const;
    QString userDataDirectory() const;
    QString pluginsDirectory() const;
    QString resourcesDirectory() const;

    // Settings management
    QSettings* settings() const { return m_settings.get(); }
    void saveSettings();
    void loadSettings();

    // Logging
    void initializeLogging();
    void logMessage(QtMsgType type, const QString &message);

    // Singleton access
    static ApplicationBase* instance();

protected:
    virtual bool initialize();
    virtual void setupDirectories();
    virtual void setupSettings();
    virtual void initializeResources();

private:
    std::unique_ptr<QSettings> m_settings;
    QString m_applicationDir;
    QString m_userDataDir;
    QString m_pluginsDir;
    QString m_resourcesDir;

    static ApplicationBase* s_instance;

private slots:
    void onAboutToQuit();
};

} // namespace Core