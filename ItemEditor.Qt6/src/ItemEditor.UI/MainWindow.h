#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QSplitter>
#include <QLabel>
#include <QProgressBar>

// Forward declarations
class PluginManager;
class OtbFileManager;
class ServerItemListWidget;
class PropertyEditorWidget;
class ClientItemWidget;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief Main application window
 * 
 * Provides the primary user interface for ItemEditor, maintaining
 * exact visual and functional parity with the legacy Windows Forms application.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // File menu actions
    void openFile();
    bool saveFile();
    bool saveFileAs();
    void recentFileTriggered();
    void exitApplication();
    
    // Edit menu actions
    void findItem();
    void preferences();
    
    // View menu actions
    void toggleItemList();
    void togglePropertyEditor();
    void toggleClientView();
    
    // Tools menu actions
    void reloadPlugins();
    void validateData();
    
    // Help menu actions
    void aboutApplication();
    void aboutQt();
    
    // Plugin manager slots
    void onPluginsLoaded(int count);
    void onPluginError(const QString& error);
    
    // Status updates
    void updateStatusBar(const QString& message);
    void updateProgressBar(int value, const QString& text = QString());
    void updateItemCount(int count);
    void showProgressBar(const QString& operation);
    void hideProgressBar();
    void setStatusMessage(const QString& message, int timeout = 0);
    
    // File manager slots
    void onFileOpened(const QString& filePath);
    void onFileSaved(const QString& filePath);
    void onFileClosed();
    void onFileModified(bool modified);
    void onFileError(const QString& error);
    void updateRecentFilesMenu();

private:
    void setupUI();
    void createMenuBar();
    void createToolBar();
    void createStatusBar();
    void createDockWidgets();
    void applyDarkTheme();
    void applyTheme(const QString& themeName);
    void ensureThemeConsistency();
    void initializePluginSystem();
    void initializeFileManager();
    void connectSignals();
    void loadSettings();
    void saveSettings();
    
    Ui::MainWindow *ui;
    
    // Core components
    PluginManager *m_pluginManager;
    OtbFileManager *m_fileManager;
    
    // UI components
    ServerItemListWidget *m_itemListWidget;
    PropertyEditorWidget *m_propertyEditor;
    ClientItemWidget *m_clientItemWidget;
    
    // Dock widgets
    QDockWidget *m_itemListDock;
    QDockWidget *m_propertyDock;
    QDockWidget *m_clientViewDock;
    
    // Status bar components
    QLabel *m_statusLabel;
    QLabel *m_itemCountLabel;
    QProgressBar *m_progressBar;
    
    // Recent files menu
    QMenu *m_recentFilesMenu;
};