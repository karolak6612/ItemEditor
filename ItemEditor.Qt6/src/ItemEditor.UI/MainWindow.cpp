#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "PluginManager.h"
#include "OtbFileManager.h"
#include "ServerItemListWidget.h"
#include "PropertyEditorWidget.h"
#include "ClientItemWidget.h"
#include "FindItemDialog.h"

#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QSplitter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QFileInfo>
#include <QDir>
#include <QMenu>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_pluginManager(nullptr)
    , m_fileManager(nullptr)
    , m_itemListWidget(nullptr)
    , m_propertyEditor(nullptr)
    , m_clientItemWidget(nullptr)
    , m_itemListDock(nullptr)
    , m_propertyDock(nullptr)
    , m_clientViewDock(nullptr)
    , m_statusLabel(nullptr)
    , m_itemCountLabel(nullptr)
    , m_progressBar(nullptr)
    , m_recentFilesMenu(nullptr)
{
    ui->setupUi(this);
    setupUI();
    applyDarkTheme();
    initializePluginSystem();
    initializeFileManager();
    connectSignals();
    loadSettings();
}

MainWindow::~MainWindow()
{
    saveSettings();
    
    if (m_pluginManager) {
        m_pluginManager->cleanup();
    }
    
    if (m_fileManager) {
        m_fileManager->closeFile();
    }
    
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_fileManager && m_fileManager->isModified()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Unsaved Changes",
            "You have unsaved changes. Do you want to save before closing?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );
        
        if (reply == QMessageBox::Save) {
            if (saveFile()) {
                event->accept();
            } else {
                event->ignore();
            }
        } else if (reply == QMessageBox::Discard) {
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void MainWindow::setupUI()
{
    // Set window properties
    setWindowTitle("ItemEditor");
    setMinimumSize(800, 600);
    resize(1024, 768);
    
    // Create menu bar, toolbar, and status bar
    createMenuBar();
    createToolBar();
    createStatusBar();
    createDockWidgets();
    
    // Set central widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Create main layout
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Add placeholder for main content area
    QLabel *placeholderLabel = new QLabel("Open an OTB file to begin editing", this);
    placeholderLabel->setAlignment(Qt::AlignCenter);
    placeholderLabel->setStyleSheet("color: #888888; font-size: 14px;");
    mainLayout->addWidget(placeholderLabel);
}

void MainWindow::createMenuBar()
{
    // File menu - matching legacy MainForm structure
    QMenu *fileMenu = menuBar()->addMenu("&File");
    
    QAction *openAction = fileMenu->addAction("&Open...");
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip("Open an OTB file");
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    
    QAction *saveAction = fileMenu->addAction("&Save");
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setStatusTip("Save the current file");
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    
    QAction *saveAsAction = fileMenu->addAction("Save &As...");
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    saveAsAction->setStatusTip("Save the file with a new name");
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);
    
    fileMenu->addSeparator();
    
    // Recent Files submenu
    m_recentFilesMenu = fileMenu->addMenu("Recent &Files");
    updateRecentFilesMenu();
    
    fileMenu->addSeparator();
    
    QAction *exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    exitAction->setStatusTip("Exit the application");
    connect(exitAction, &QAction::triggered, this, &MainWindow::exitApplication);
    
    // Edit menu - matching legacy structure
    QMenu *editMenu = menuBar()->addMenu("&Edit");
    
    QAction *findAction = editMenu->addAction("&Find Item...");
    findAction->setShortcut(QKeySequence::Find);
    findAction->setStatusTip("Find an item by ID or name");
    connect(findAction, &QAction::triggered, this, &MainWindow::findItem);
    
    editMenu->addSeparator();
    
    QAction *preferencesAction = editMenu->addAction("&Preferences...");
    preferencesAction->setStatusTip("Configure application settings");
    connect(preferencesAction, &QAction::triggered, this, &MainWindow::preferences);
    
    // View menu - matching legacy dock panel structure
    QMenu *viewMenu = menuBar()->addMenu("&View");
    
    QAction *itemListAction = viewMenu->addAction("&Item List");
    itemListAction->setCheckable(true);
    itemListAction->setChecked(true);
    itemListAction->setStatusTip("Show or hide the item list panel");
    connect(itemListAction, &QAction::triggered, this, &MainWindow::toggleItemList);
    
    QAction *propertyAction = viewMenu->addAction("&Properties");
    propertyAction->setCheckable(true);
    propertyAction->setChecked(true);
    propertyAction->setStatusTip("Show or hide the properties panel");
    connect(propertyAction, &QAction::triggered, this, &MainWindow::togglePropertyEditor);
    
    QAction *clientViewAction = viewMenu->addAction("&Client View");
    clientViewAction->setCheckable(true);
    clientViewAction->setChecked(true);
    clientViewAction->setStatusTip("Show or hide the client sprite view");
    connect(clientViewAction, &QAction::triggered, this, &MainWindow::toggleClientView);
    
    viewMenu->addSeparator();
    
    QAction *refreshAction = viewMenu->addAction("&Refresh");
    refreshAction->setShortcut(QKeySequence::Refresh);
    refreshAction->setStatusTip("Refresh the current view");
    connect(refreshAction, &QAction::triggered, [this]() {
        updateStatusBar("View refreshed");
    });
    
    // Tools menu - matching legacy plugin and validation tools
    QMenu *toolsMenu = menuBar()->addMenu("&Tools");
    
    QAction *reloadPluginsAction = toolsMenu->addAction("&Reload Plugins");
    reloadPluginsAction->setShortcut(QKeySequence("F5"));
    reloadPluginsAction->setStatusTip("Reload all client plugins");
    connect(reloadPluginsAction, &QAction::triggered, this, &MainWindow::reloadPlugins);
    
    QAction *validateAction = toolsMenu->addAction("&Validate Data");
    validateAction->setShortcut(QKeySequence("Ctrl+Shift+V"));
    validateAction->setStatusTip("Validate server and client data consistency");
    connect(validateAction, &QAction::triggered, this, &MainWindow::validateData);
    
    toolsMenu->addSeparator();
    
    QAction *compareAction = toolsMenu->addAction("&Compare Items");
    compareAction->setShortcut(QKeySequence("Ctrl+D"));
    compareAction->setStatusTip("Compare server and client items");
    connect(compareAction, &QAction::triggered, [this]() {
        QMessageBox::information(this, "Compare Items", "Item comparison not yet implemented");
    });
    
    // Help menu - matching legacy about and help structure
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    
    QAction *aboutAction = helpMenu->addAction("&About ItemEditor");
    aboutAction->setStatusTip("Show information about ItemEditor");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::aboutApplication);
    
    QAction *aboutQtAction = helpMenu->addAction("About &Qt");
    aboutQtAction->setStatusTip("Show information about Qt");
    connect(aboutQtAction, &QAction::triggered, this, &MainWindow::aboutQt);
}

void MainWindow::createToolBar()
{
    QToolBar *mainToolBar = addToolBar("Main");
    mainToolBar->setObjectName("MainToolBar");
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mainToolBar->setIconSize(QSize(24, 24));
    mainToolBar->setMovable(false);
    
    // File operations - matching legacy toolbar layout
    QAction *openAction = mainToolBar->addAction("Open");
    openAction->setToolTip("Open OTB file (Ctrl+O)");
    openAction->setShortcut(QKeySequence::Open);
    // Icon will be set when resources are available
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    
    QAction *saveAction = mainToolBar->addAction("Save");
    saveAction->setToolTip("Save current file (Ctrl+S)");
    saveAction->setShortcut(QKeySequence::Save);
    // Icon will be set when resources are available
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    
    mainToolBar->addSeparator();
    
    // Search and navigation
    QAction *findAction = mainToolBar->addAction("Find");
    findAction->setToolTip("Find item (Ctrl+F)");
    findAction->setShortcut(QKeySequence::Find);
    // Icon will be set when resources are available
    connect(findAction, &QAction::triggered, this, &MainWindow::findItem);
    
    mainToolBar->addSeparator();
    
    // Plugin and validation tools - matching legacy functionality
    QAction *reloadAction = mainToolBar->addAction("Reload");
    reloadAction->setToolTip("Reload plugins (F5)");
    reloadAction->setShortcut(QKeySequence("F5"));
    // Icon will be set when resources are available
    connect(reloadAction, &QAction::triggered, this, &MainWindow::reloadPlugins);
    
    QAction *validateAction = mainToolBar->addAction("Validate");
    validateAction->setToolTip("Validate data (Ctrl+Shift+V)");
    validateAction->setShortcut(QKeySequence("Ctrl+Shift+V"));
    // Icon will be set when resources are available
    connect(validateAction, &QAction::triggered, this, &MainWindow::validateData);
    
    mainToolBar->addSeparator();
    
    // View controls
    QAction *refreshAction = mainToolBar->addAction("Refresh");
    refreshAction->setToolTip("Refresh view (F5)");
    refreshAction->setShortcut(QKeySequence::Refresh);
    // Icon will be set when resources are available
    connect(refreshAction, &QAction::triggered, [this]() {
        updateStatusBar("View refreshed");
    });
}

void MainWindow::createStatusBar()
{
    // Main status message - matches legacy MainForm status display
    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setMinimumWidth(200);
    statusBar()->addWidget(m_statusLabel, 1); // Stretch factor 1
    
    // Separator
    QLabel *separator1 = new QLabel("|", this);
    separator1->setStyleSheet("color: #888888;");
    statusBar()->addPermanentWidget(separator1);
    
    // Item count display - matches legacy item counter
    m_itemCountLabel = new QLabel("Items: 0", this);
    m_itemCountLabel->setMinimumWidth(80);
    m_itemCountLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addPermanentWidget(m_itemCountLabel);
    
    // Separator
    QLabel *separator2 = new QLabel("|", this);
    separator2->setStyleSheet("color: #888888;");
    statusBar()->addPermanentWidget(separator2);
    
    // Progress bar for file operations - matches legacy progress indication
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    m_progressBar->setMaximumWidth(200);
    m_progressBar->setMinimumWidth(200);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFormat("%p%");
    statusBar()->addPermanentWidget(m_progressBar);
    
    // Set status bar height to match legacy system (22px)
    statusBar()->setFixedHeight(22);
}

void MainWindow::createDockWidgets()
{
    // Item List dock widget
    m_itemListDock = new QDockWidget("Item List", this);
    m_itemListWidget = new ServerItemListWidget(this);
    m_itemListDock->setWidget(m_itemListWidget);
    addDockWidget(Qt::LeftDockWidgetArea, m_itemListDock);
    
    // Property Editor dock widget
    m_propertyDock = new QDockWidget("Properties", this);
    m_propertyEditor = new PropertyEditorWidget(this);
    m_propertyDock->setWidget(m_propertyEditor);
    addDockWidget(Qt::RightDockWidgetArea, m_propertyDock);
    
    // Client View dock widget
    m_clientViewDock = new QDockWidget("Client View", this);
    m_clientItemWidget = new ClientItemWidget(this);
    m_clientViewDock->setWidget(m_clientItemWidget);
    addDockWidget(Qt::RightDockWidgetArea, m_clientViewDock);
    
    // Tabify right dock widgets
    tabifyDockWidget(m_propertyDock, m_clientViewDock);
    m_propertyDock->raise();
}

void MainWindow::applyDarkTheme()
{
    // Load dark theme stylesheet from resource file
    QFile themeFile(":/themes/dark.qss");
    if (themeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString darkStyle = QString::fromUtf8(themeFile.readAll());
        setStyleSheet(darkStyle);
        themeFile.close();
        
        qDebug() << "Dark theme applied successfully";
    } else {
        qWarning() << "Failed to load dark theme file, using fallback styling";
        
        // Fallback minimal dark theme if resource file fails to load
        QString fallbackStyle = R"(
            QMainWindow {
                background-color: #3c3f41;
                color: #dcdcdc;
                font-family: "Segoe UI", Arial, sans-serif;
                font-size: 9pt;
            }
            QMenuBar {
                background-color: #45494a;
                color: #dcdcdc;
                border-bottom: 1px solid #555555;
            }
            QMenuBar::item:selected {
                background-color: #6897bb;
                color: #ffffff;
            }
            QToolBar {
                background-color: #45494a;
                border: 1px solid #555555;
            }
            QStatusBar {
                background-color: #45494a;
                color: #dcdcdc;
                border-top: 1px solid #555555;
            }
            QDockWidget {
                background-color: #3c3f41;
                color: #dcdcdc;
            }
            QDockWidget::title {
                background-color: #45494a;
                color: #dcdcdc;
                padding: 4px;
                border-bottom: 1px solid #555555;
            }
        )";
        
        setStyleSheet(fallbackStyle);
    }
    
    // Ensure theme consistency across all components
    ensureThemeConsistency();
}

void MainWindow::applyTheme(const QString& themeName)
{
    // Load theme stylesheet from resource file
    QString themeFilePath = QString(":/themes/%1.qss").arg(themeName);
    QFile themeFile(themeFilePath);
    
    if (themeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString themeStyle = QString::fromUtf8(themeFile.readAll());
        setStyleSheet(themeStyle);
        themeFile.close();
        
        qDebug() << "Theme" << themeName << "applied successfully";
        ensureThemeConsistency();
    } else {
        qWarning() << "Failed to load theme file:" << themeFilePath;
        // Fall back to dark theme if requested theme fails
        if (themeName != "dark") {
            applyDarkTheme();
        }
    }
}

void MainWindow::ensureThemeConsistency()
{
    // Ensure all dock widgets inherit the theme properly
    if (m_itemListDock) {
        m_itemListDock->style()->unpolish(m_itemListDock);
        m_itemListDock->style()->polish(m_itemListDock);
    }
    
    if (m_propertyDock) {
        m_propertyDock->style()->unpolish(m_propertyDock);
        m_propertyDock->style()->polish(m_propertyDock);
    }
    
    if (m_clientViewDock) {
        m_clientViewDock->style()->unpolish(m_clientViewDock);
        m_clientViewDock->style()->polish(m_clientViewDock);
    }
    
    // Force update of all child widgets to apply theme
    QList<QWidget*> allWidgets = findChildren<QWidget*>();
    for (QWidget* widget : allWidgets) {
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
        widget->update();
    }
    
    // Update the main window itself
    style()->unpolish(this);
    style()->polish(this);
    update();
}

void MainWindow::initializePluginSystem()
{
    m_pluginManager = new PluginManager(this);
    
    // Initialize plugin manager
    if (m_pluginManager->initialize()) {
        updateStatusBar(QString("Loaded %1 plugins").arg(m_pluginManager->getPluginCount()));
    } else {
        updateStatusBar("Failed to initialize plugin system");
    }
}

void MainWindow::initializeFileManager()
{
    m_fileManager = new OtbFileManager(this);
    
    // Set up progress callback
    m_fileManager->setProgressCallback([this](int current, int total, const QString& status) {
        updateProgressBar(current, status);
    });
}

void MainWindow::connectSignals()
{
    if (m_pluginManager) {
        connect(m_pluginManager, &PluginManager::pluginsLoaded,
                this, &MainWindow::onPluginsLoaded);
        connect(m_pluginManager, &PluginManager::errorOccurred,
                this, &MainWindow::onPluginError);
        connect(m_pluginManager, &PluginManager::loadingProgress,
                this, &MainWindow::updateProgressBar);
    }
    
    if (m_fileManager) {
        connect(m_fileManager, &OtbFileManager::fileOpened,
                this, &MainWindow::onFileOpened);
        connect(m_fileManager, &OtbFileManager::fileSaved,
                this, &MainWindow::onFileSaved);
        connect(m_fileManager, &OtbFileManager::fileClosed,
                this, &MainWindow::onFileClosed);
        connect(m_fileManager, &OtbFileManager::fileModified,
                this, &MainWindow::onFileModified);
        connect(m_fileManager, &OtbFileManager::errorOccurred,
                this, &MainWindow::onFileError);
        connect(m_fileManager, &OtbFileManager::recentFilesChanged,
                this, &MainWindow::updateRecentFilesMenu);
    }
}

void MainWindow::loadSettings()
{
    QSettings settings;
    
    // Restore window geometry
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    
    // Restore recent files
    // Implementation will be added later
}

void MainWindow::saveSettings()
{
    QSettings settings;
    
    // Save window geometry
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
}

// Slot implementations
void MainWindow::openFile()
{
    // Get default directory from file manager settings
    QString defaultDir;
    if (m_fileManager) {
        defaultDir = m_fileManager->getSettings().defaultDirectory;
    }
    
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open OTB File",
        defaultDir,
        "OTB Files (*.otb);;All Files (*)"
    );
    
    if (!fileName.isEmpty() && m_fileManager) {
        if (!m_fileManager->openFile(fileName)) {
            QMessageBox::critical(this, "Error Opening File", 
                QString("Failed to open file:\n%1").arg(m_fileManager->getLastError()));
        }
    }
}

bool MainWindow::saveFile()
{
    if (!m_fileManager) {
        return false;
    }
    
    if (!m_fileManager->hasOpenFile()) {
        return saveFileAs();
    }
    
    if (!m_fileManager->saveFile()) {
        QMessageBox::critical(this, "Error Saving File", 
            QString("Failed to save file:\n%1").arg(m_fileManager->getLastError()));
        return false;
    }
    
    return true;
}

bool MainWindow::saveFileAs()
{
    if (!m_fileManager) {
        return false;
    }
    
    // Get default directory and current filename
    QString defaultDir;
    QString defaultName;
    
    if (m_fileManager->hasOpenFile()) {
        QFileInfo currentFile(m_fileManager->getCurrentFilePath());
        defaultDir = currentFile.absolutePath();
        defaultName = currentFile.fileName();
    } else {
        defaultDir = m_fileManager->getSettings().defaultDirectory;
        defaultName = "items.otb";
    }
    
    QString defaultPath = QDir(defaultDir).filePath(defaultName);
    
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save OTB File",
        defaultPath,
        "OTB Files (*.otb);;All Files (*)"
    );
    
    if (!fileName.isEmpty()) {
        if (!m_fileManager->saveFileAs(fileName)) {
            QMessageBox::critical(this, "Error Saving File", 
                QString("Failed to save file:\n%1").arg(m_fileManager->getLastError()));
            return false;
        }
        return true;
    }
    
    return false;
}

void MainWindow::recentFileTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action && m_fileManager) {
        QString filePath = action->data().toString();
        if (!filePath.isEmpty()) {
            if (!m_fileManager->openFile(filePath)) {
                QMessageBox::critical(this, "Error Opening File", 
                    QString("Failed to open recent file:\n%1\n\nError: %2")
                    .arg(filePath)
                    .arg(m_fileManager->getLastError()));
                
                // Remove from recent files if file doesn't exist
                if (!QFile::exists(filePath)) {
                    m_fileManager->removeRecentFile(filePath);
                }
            }
        }
    }
}

void MainWindow::exitApplication()
{
    close();
}

void MainWindow::findItem()
{
    if (!m_fileManager || !m_fileManager->getServerItemList()) {
        QMessageBox::information(this, "Find Item", "No OTB file is currently loaded.");
        return;
    }
    
    FindItemDialog dialog(m_fileManager->getServerItemList(), this);
    dialog.exec();
}

void MainWindow::preferences()
{
    // Preferences dialog will be implemented later
    QMessageBox::information(this, "Preferences", "Preferences dialog not yet implemented");
}

void MainWindow::toggleItemList()
{
    m_itemListDock->setVisible(!m_itemListDock->isVisible());
}

void MainWindow::togglePropertyEditor()
{
    m_propertyDock->setVisible(!m_propertyDock->isVisible());
}

void MainWindow::toggleClientView()
{
    m_clientViewDock->setVisible(!m_clientViewDock->isVisible());
}

void MainWindow::reloadPlugins()
{
    if (m_pluginManager) {
        updateStatusBar("Reloading plugins...");
        if (m_pluginManager->reloadPlugins()) {
            updateStatusBar(QString("Reloaded %1 plugins").arg(m_pluginManager->getPluginCount()));
        } else {
            updateStatusBar("Failed to reload plugins");
        }
    }
}

void MainWindow::validateData()
{
    // Data validation logic will be implemented later
    QMessageBox::information(this, "Validate Data", "Data validation not yet implemented");
}

void MainWindow::aboutApplication()
{
    QMessageBox::about(this, "About ItemEditor",
        "ItemEditor Qt6\n"
        "Version 1.0.0\n\n"
        "A specialized tool for editing OTB (Open Tibia Binary) data files.\n"
        "Migrated from Windows Forms to Qt6 for improved cross-platform support."
    );
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::onPluginsLoaded(int count)
{
    updateStatusBar(QString("Loaded %1 plugins").arg(count));
}

void MainWindow::onPluginError(const QString& error)
{
    QMessageBox::warning(this, "Plugin Error", error);
    updateStatusBar("Plugin error occurred");
}

void MainWindow::updateStatusBar(const QString& message)
{
    if (m_statusLabel) {
        m_statusLabel->setText(message);
    }
}

void MainWindow::updateProgressBar(int value, const QString& text)
{
    if (m_progressBar) {
        if (value >= 0 && value <= 100) {
            m_progressBar->setValue(value);
            m_progressBar->setVisible(true);
            
            if (!text.isEmpty()) {
                updateStatusBar(text);
            }
            
            if (value == 100) {
                // Hide progress bar after a short delay
                QTimer::singleShot(1000, [this]() {
                    m_progressBar->setVisible(false);
                });
            }
        } else {
            m_progressBar->setVisible(false);
        }
    }
}

void MainWindow::updateItemCount(int count)
{
    if (m_itemCountLabel) {
        m_itemCountLabel->setText(QString("Items: %1").arg(count));
    }
}

void MainWindow::showProgressBar(const QString& operation)
{
    if (m_progressBar) {
        m_progressBar->setVisible(true);
        m_progressBar->setValue(0);
        m_progressBar->setFormat(QString("%1 - %p%").arg(operation));
        updateStatusBar(QString("%1...").arg(operation));
    }
}

void MainWindow::hideProgressBar()
{
    if (m_progressBar) {
        m_progressBar->setVisible(false);
        m_progressBar->setFormat("%p%"); // Reset to default format
    }
}

void MainWindow::setStatusMessage(const QString& message, int timeout)
{
    if (m_statusLabel) {
        m_statusLabel->setText(message);
        
        if (timeout > 0) {
            // Clear the message after the specified timeout
            QTimer::singleShot(timeout, [this]() {
                m_statusLabel->setText("Ready");
            });
        }
    }
}

void MainWindow::onFileOpened(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    setWindowTitle(QString("ItemEditor - %1").arg(fileInfo.fileName()));
    updateStatusBar(QString("Opened: %1").arg(fileInfo.fileName()));
    
    // Update item count
    if (m_fileManager) {
        updateItemCount(m_fileManager->getItems().size());
        
        // Update item list widget
        if (m_itemListWidget) {
            m_itemListWidget->setServerItemList(m_fileManager->getServerItemList());
        }
    }
    
    hideProgressBar();
}

void MainWindow::onFileSaved(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    setWindowTitle(QString("ItemEditor - %1").arg(fileInfo.fileName()));
    updateStatusBar(QString("Saved: %1").arg(fileInfo.fileName()));
    hideProgressBar();
}

void MainWindow::onFileClosed()
{
    setWindowTitle("ItemEditor");
    updateStatusBar("Ready");
    updateItemCount(0);
    
    // Clear item list widget
    if (m_itemListWidget) {
        m_itemListWidget->clearItems();
    }
    
    // Clear property editor
    if (m_propertyEditor) {
        m_propertyEditor->clearEditor();
    }
    
    // Clear client view
    if (m_clientItemWidget) {
        m_clientItemWidget->clearWidget();
    }
}

void MainWindow::onFileModified(bool modified)
{
    QString title = windowTitle();
    
    if (modified && !title.endsWith(" *")) {
        setWindowTitle(title + " *");
    } else if (!modified && title.endsWith(" *")) {
        title.chop(2); // Remove " *"
        setWindowTitle(title);
    }
}

void MainWindow::onFileError(const QString& error)
{
    updateStatusBar(QString("Error: %1").arg(error));
    hideProgressBar();
}

void MainWindow::updateRecentFilesMenu()
{
    if (!m_recentFilesMenu || !m_fileManager) {
        return;
    }
    
    m_recentFilesMenu->clear();
    
    QStringList recentFiles = m_fileManager->getRecentFiles();
    
    if (recentFiles.isEmpty()) {
        QAction* noFilesAction = m_recentFilesMenu->addAction("(No recent files)");
        noFilesAction->setEnabled(false);
    } else {
        for (int i = 0; i < recentFiles.size(); ++i) {
            const QString& filePath = recentFiles[i];
            QFileInfo fileInfo(filePath);
            
            QString actionText = QString("&%1 %2").arg(i + 1).arg(fileInfo.fileName());
            QAction* action = m_recentFilesMenu->addAction(actionText);
            action->setData(filePath);
            action->setToolTip(filePath);
            action->setStatusTip(QString("Open %1").arg(filePath));
            
            connect(action, &QAction::triggered, this, &MainWindow::recentFileTriggered);
        }
        
        m_recentFilesMenu->addSeparator();
        
        QAction* clearAction = m_recentFilesMenu->addAction("&Clear Recent Files");
        connect(clearAction, &QAction::triggered, [this]() {
            if (m_fileManager) {
                m_fileManager->clearRecentFiles();
            }
        });
    }
}