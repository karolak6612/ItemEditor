#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QMoveEvent>
#include <QSettings>
#include <QString>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(windowLog)

namespace Core {

class ApplicationBase;

/**
 * @brief Base window class providing common window functionality
 * 
 * This class serves as the foundation for all windows in the application,
 * providing common features like geometry persistence, common styling,
 * and standard window behaviors.
 */
class WindowBase : public QMainWindow
{
    Q_OBJECT

public:
    explicit WindowBase(QWidget *parent = nullptr);
    virtual ~WindowBase();

    // Window state management
    void saveGeometry();
    void restoreGeometry();
    void saveState();
    void restoreState();

    // Common window operations
    virtual void setupUI();
    virtual void setupConnections();
    virtual void setupMenus();
    virtual void setupToolbars();
    virtual void setupStatusBar();

    // Settings management
    QString settingsKey() const { return m_settingsKey; }
    void setSettingsKey(const QString &key) { m_settingsKey = key; }

protected:
    // Event handlers
    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;

    // Virtual methods for subclasses
    virtual bool canClose() { return true; }
    virtual void onWindowShown() {}
    virtual void onWindowClosed() {}

    // Utility methods
    ApplicationBase* application() const;
    QSettings* settings() const;
    void logMessage(const QString &message, QtMsgType type = QtInfoMsg);

private:
    QString m_settingsKey;
    bool m_geometryRestored;
    bool m_stateRestored;

private slots:
    void onApplicationAboutToQuit();
};

} // namespace Core