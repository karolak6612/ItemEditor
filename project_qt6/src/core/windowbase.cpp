#include "core/windowbase.h"
#include "core/applicationbase.h"
#include <QApplication>
#include <QSettings>
#include <QLoggingCategory>
#include <QShowEvent>

Q_LOGGING_CATEGORY(windowLog, "window")

namespace Core {

WindowBase::WindowBase(QWidget *parent)
    : QMainWindow(parent)
    , m_geometryRestored(false)
    , m_stateRestored(false)
{
    // Set default settings key based on class name
    m_settingsKey = metaObject()->className();
    
    // Connect to application signals
    if (auto app = application()) {
        connect(app, &QApplication::aboutToQuit, this, &WindowBase::onApplicationAboutToQuit);
    }
    
    qCDebug(windowLog) << "WindowBase created:" << m_settingsKey;
}

WindowBase::~WindowBase()
{
    qCDebug(windowLog) << "WindowBase destroyed:" << m_settingsKey;
}

ApplicationBase* WindowBase::application() const
{
    return ApplicationBase::instance();
}

QSettings* WindowBase::settings() const
{
    if (auto app = application()) {
        return app->settings();
    }
    return nullptr;
}void WindowBase::logMessage(const QString &message, QtMsgType type)
{
    if (auto app = application()) {
        app->logMessage(type, QString("[%1] %2").arg(m_settingsKey, message));
    }
}

void WindowBase::saveGeometry()
{
    if (auto s = settings()) {
        s->beginGroup(m_settingsKey);
        s->setValue("geometry", QMainWindow::saveGeometry());
        s->setValue("windowState", QMainWindow::saveState());
        s->endGroup();
        qCDebug(windowLog) << "Geometry saved for:" << m_settingsKey;
    }
}

void WindowBase::restoreGeometry()
{
    if (auto s = settings()) {
        s->beginGroup(m_settingsKey);
        QMainWindow::restoreGeometry(s->value("geometry").toByteArray());
        QMainWindow::restoreState(s->value("windowState").toByteArray());
        s->endGroup();
        m_geometryRestored = true;
        qCDebug(windowLog) << "Geometry restored for:" << m_settingsKey;
    }
}

void WindowBase::saveState()
{
    if (auto s = settings()) {
        s->beginGroup(m_settingsKey);
        s->setValue("state", QMainWindow::saveState());
        s->endGroup();
    }
}

void WindowBase::restoreState()
{
    if (auto s = settings()) {
        s->beginGroup(m_settingsKey);
        QMainWindow::restoreState(s->value("state").toByteArray());
        s->endGroup();
        m_stateRestored = true;
    }
}void WindowBase::setupUI()
{
    // Default implementation - override in subclasses
    qCDebug(windowLog) << "Setting up UI for:" << m_settingsKey;
}

void WindowBase::setupConnections()
{
    // Default implementation - override in subclasses
    qCDebug(windowLog) << "Setting up connections for:" << m_settingsKey;
}

void WindowBase::setupMenus()
{
    // Default implementation - override in subclasses
}

void WindowBase::setupToolbars()
{
    // Default implementation - override in subclasses
}

void WindowBase::setupStatusBar()
{
    // Default implementation - override in subclasses
}

// Event handlers
void WindowBase::closeEvent(QCloseEvent *event)
{
    if (canClose()) {
        saveGeometry();
        saveState();
        onWindowClosed();
        event->accept();
        qCDebug(windowLog) << "Window closed:" << m_settingsKey;
    } else {
        event->ignore();
        qCDebug(windowLog) << "Window close rejected:" << m_settingsKey;
    }
}void WindowBase::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    // Auto-save geometry on resize (with some delay to avoid too frequent saves)
}

void WindowBase::moveEvent(QMoveEvent *event)
{
    QMainWindow::moveEvent(event);
    // Auto-save geometry on move (with some delay to avoid too frequent saves)
}

void WindowBase::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    
    // Restore geometry on first show
    if (!m_geometryRestored) {
        restoreGeometry();
    }
    
    // Restore state on first show
    if (!m_stateRestored) {
        restoreState();
    }
    
    onWindowShown();
    qCDebug(windowLog) << "Window shown:" << m_settingsKey;
}

void WindowBase::onApplicationAboutToQuit()
{
    saveGeometry();
    saveState();
}

} // namespace Core

#include "moc_windowbase.cpp"