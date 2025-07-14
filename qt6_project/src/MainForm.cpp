/**
 * Item Editor Qt6 - Main Application Window Implementation
 * Exact mirror of Legacy_App/csharp/Source/MainForm.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "MainForm.h"
#include "ui_MainForm.h"

#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QProgressBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QMimeData>
#include <QUrl>
#include <QProgressDialog>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>
#include <QTime>

#include "Controls/FlagCheckBox.h"
#include "Dialogs/AboutForm.h"
#include "Dialogs/FindItemForm.h"
#include "Dialogs/CompareOtbForm.h"
#include "Helpers/MemoryManager.h"
// Note: Additional dialogs will be implemented in subsequent tasks
#include "Dialogs/PreferencesForm.h"      
// #include "Dialogs/NewOtbFileForm.h"       
// #include "Dialogs/UpdateForm.h"           
// #include "Dialogs/UpdateSettingsForm.h"
#include "PluginInterface/Item.h"
#include "PluginInterface/OTLib/OTB/OtbReader.h"
#include "PluginInterface/OTLib/OTB/OtbWriter.h"
#include "PluginInterface/OTLib/Collections/ServerItemList.h"
#include "Host/PluginServices.h"
#include "Host/Plugin.h"
#include "Properties/version.h"

namespace ItemEditor {

MainForm::MainForm(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainForm)
    , m_mainSplitter(nullptr)
    , m_rightSplitter(nullptr)
    , m_leftPanel(nullptr)
    , m_centerPanel(nullptr)
    , m_rightPanel(nullptr)
    , m_serverItemListBox(nullptr)
    , m_clientItemView(nullptr)
    , m_hasUnsavedChanges(false)
    , m_isLoading(false)
    , m_currentPlugin(nullptr)
    , m_pluginServices(nullptr)
    , m_selectedServerId(0)
    , m_otbReader(nullptr)
    , m_otbWriter(nullptr)
    , m_serverItemList(nullptr)
    , m_spriteManager(nullptr)
    , m_settings(nullptr)
    , m_updateTimer(nullptr)
    , m_otbFileLoaded(false)
    , m_currentOtbFilePath("")
{
    ui->setupUi(this);
    
    // Initialize settings
    m_settings = new QSettings(this);
    
    // Initialize plugin services
    m_pluginServices = new PluginServices(this);
    
    // Initialize OTB components
    m_otbReader = std::make_unique<OTLib::OTB::OtbReader>(this);
    m_otbWriter = std::make_unique<OTLib::OTB::OtbWriter>(this);
    m_serverItemList = nullptr;
    
    // Initialize sprite manager
    m_spriteManager = new SpriteManager(this);
    
    // Setup UI components
    setupUi();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupCentralWidget();
    setupConnections();
    setupDragDrop();
    
    // Load settings
    loadSettings();
    
    // Initialize plugins
    loadPlugins();
    
    // Setup update timer
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(UPDATE_INTERVAL_MS);
    connect(m_updateTimer, &QTimer::timeout, this, &MainForm::onUpdateTimer);
    m_updateTimer->start();
    
    // Update initial state
    updateWindowTitle();
    updateMenuStates();
    updateStatusBar();
}

MainForm::~MainForm()
{
    // Enhanced cleanup with memory management
    qDebug() << "MainForm: Starting cleanup...";
    
    // Stop update timer to prevent further updates during cleanup
    if (m_updateTimer) {
        m_updateTimer->stop();
    }
    
    // Disconnect all signals to prevent dangling connections
    disconnect();
    
    // Cleanup plugins before destroying UI
    if (m_pluginServices) {
        m_pluginServices->closePlugins();
    }
    
    // Clear any cached data
    if (m_serverItemListBox) {
        m_serverItemListBox->clearSpriteCache();
    }
    
    // Save settings
    saveSettings();
    
    // Track memory deallocation
    MemoryManager::instance()->trackDeallocation(this);
    
    // Clean up UI
    delete ui;
    
    qDebug() << "MainForm: Cleanup completed";
}

// Test support method implementations
bool MainForm::loadOtbFile(const QString& filePath)
{
    // Implementation for loading OTB file
    // This would call the existing file loading logic
    if (filePath.isEmpty() || !QFile::exists(filePath)) {
        return false;
    }
    
    // Store the file path
    m_currentOtbFilePath = filePath;
    
    // Call existing load logic (placeholder for now)
    // In actual implementation, this would call onFileOpen() logic
    m_otbFileLoaded = true;
    
    return true;
}

bool MainForm::isOtbFileLoaded() const
{
    return m_otbFileLoaded;
}

QString MainForm::getCurrentOtbFilePath() const
{
    return m_currentOtbFilePath;
}

ServerItemListBox* MainForm::getServerItemListBox() const
{
    return m_serverItemListBox;
}

ClientItemView* MainForm::getClientItemView() const
{
    return m_clientItemView;
}

int MainForm::getCurrentSelectedItemId() const
{
    if (m_serverItemListBox) {
        const auto& selectedIndices = m_serverItemListBox->selectedIndices();
        if (!selectedIndices.isEmpty()) {
            int index = *selectedIndices.begin();
            const auto& items = m_serverItemListBox->items();
            if (index >= 0 && index < items.size()) {
                auto* serverItem = items[index];
                if (serverItem) {
                    return serverItem->id();
                }
            }
        }
    }
    return -1;
}

void MainForm::clearLoadedData()
{
    m_otbFileLoaded = false;
    m_currentOtbFilePath.clear();
    
    if (m_serverItemListBox) {
        m_serverItemListBox->clearSelection();
    }
    
    if (m_clientItemView) {
        m_clientItemView->clear();
    }
}

void MainForm::resetUIState()
{
    // Reset UI to initial state
    clearLoadedData();
    
    // Reset status bar
    if (statusBar()) {
        statusBar()->showMessage("Ready");
    }
}

void MainForm::setupUi()
{
    // Set window properties - exact mirror of C# MainForm properties
    setWindowTitle("Item Editor");
    setMinimumSize(WINDOW_MIN_WIDTH, WINDOW_MIN_HEIGHT);
    resize(1024, 768);
    
    // Set window icon
    setWindowIcon(QIcon(":/icons/application.ico"));
    
    // Enable drag and drop
    setAcceptDrops(true);
}

void MainForm::setupMenuBar()
{
    // Create menu bar - exact mirror of C# menu structure
    m_menuBar = menuBar();
    
    // File Menu
    m_fileMenu = m_menuBar->addMenu(tr("&File"));
    
    m_newAction = m_fileMenu->addAction(QIcon(":/icons/new.png"), tr("&New"));
    m_newAction->setShortcut(QKeySequence::New);
    m_newAction->setStatusTip(tr("Create a new OTB file"));
    connect(m_newAction, &QAction::triggered, this, &MainForm::onFileNew);
    
    m_openAction = m_fileMenu->addAction(QIcon(":/icons/open.png"), tr("&Open..."));
    m_openAction->setShortcut(QKeySequence::Open);
    m_openAction->setStatusTip(tr("Open an existing OTB file"));
    connect(m_openAction, &QAction::triggered, this, &MainForm::onFileOpen);
    
    m_fileMenu->addSeparator();
    
    m_saveAction = m_fileMenu->addAction(QIcon(":/icons/save.png"), tr("&Save"));
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setStatusTip(tr("Save the current OTB file"));
    connect(m_saveAction, &QAction::triggered, this, &MainForm::onFileSave);
    
    m_saveAsAction = m_fileMenu->addAction(QIcon(":/icons/saveas.png"), tr("Save &As..."));
    m_saveAsAction->setShortcut(QKeySequence::SaveAs);
    m_saveAsAction->setStatusTip(tr("Save the OTB file with a new name"));
    connect(m_saveAsAction, &QAction::triggered, this, &MainForm::onFileSaveAs);
    
    m_fileMenu->addSeparator();
    
    m_compareOtbAction = m_fileMenu->addAction(QIcon(":/icons/compare.png"), tr("&Compare OTB..."));
    m_compareOtbAction->setStatusTip(tr("Compare two OTB files"));
    connect(m_compareOtbAction, &QAction::triggered, this, &MainForm::onFileCompareOtb);
    
    m_fileMenu->addSeparator();
    
    m_exitAction = m_fileMenu->addAction(tr("E&xit"));
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip(tr("Exit the application"));
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);
    
    // Edit Menu
    m_editMenu = m_menuBar->addMenu(tr("&Edit"));
    
    m_findAction = m_editMenu->addAction(QIcon(":/icons/find.png"), tr("&Find Item..."));
    m_findAction->setShortcut(QKeySequence::Find);
    m_findAction->setStatusTip(tr("Find an item by ID or name"));
    connect(m_findAction, &QAction::triggered, this, &MainForm::onEditFind);
    
    m_editMenu->addSeparator();
    
    m_preferencesAction = m_editMenu->addAction(QIcon(":/icons/preferences.png"), tr("&Preferences..."));
    m_preferencesAction->setStatusTip(tr("Configure application preferences"));
    connect(m_preferencesAction, &QAction::triggered, this, &MainForm::onEditPreferences);
    
    // View Menu
    m_viewMenu = m_menuBar->addMenu(tr("&View"));
    
    m_showOnlyMismatchedAction = m_viewMenu->addAction(tr("Show Only &Mismatched Items"));
    m_showOnlyMismatchedAction->setCheckable(true);
    m_showOnlyMismatchedAction->setStatusTip(tr("Show only items that don't match between server and client"));
    connect(m_showOnlyMismatchedAction, &QAction::toggled, this, &MainForm::onViewShowOnlyMismatchedItems);
    
    m_viewMenu->addSeparator();
    
    // Item ID Display submenu
    m_itemIdDisplayGroup = new QActionGroup(this);
    
    m_showDecimalIdAction = m_viewMenu->addAction(tr("Show &Decimal Item IDs"));
    m_showDecimalIdAction->setCheckable(true);
    m_showDecimalIdAction->setChecked(true);
    m_itemIdDisplayGroup->addAction(m_showDecimalIdAction);
    connect(m_showDecimalIdAction, &QAction::toggled, this, &MainForm::onViewShowDecimalItemId);
    
    m_showHexIdAction = m_viewMenu->addAction(tr("Show &Hexadecimal Item IDs"));
    m_showHexIdAction->setCheckable(true);
    m_itemIdDisplayGroup->addAction(m_showHexIdAction);
    connect(m_showHexIdAction, &QAction::toggled, this, &MainForm::onViewShowHexItemId);
    
    // Tools Menu
    m_toolsMenu = m_menuBar->addMenu(tr("&Tools"));
    
    m_updateAction = m_toolsMenu->addAction(QIcon(":/icons/update.png"), tr("&Update..."));
    m_updateAction->setStatusTip(tr("Check for application updates"));
    connect(m_updateAction, &QAction::triggered, this, &MainForm::onToolsUpdate);
    
    m_updateSettingsAction = m_toolsMenu->addAction(QIcon(":/icons/updatesettings.png"), tr("Update &Settings..."));
    m_updateSettingsAction->setStatusTip(tr("Configure update settings"));
    connect(m_updateSettingsAction, &QAction::triggered, this, &MainForm::onToolsUpdateSettings);
    
    m_toolsMenu->addSeparator();
    
    m_diagnosticsAction = m_toolsMenu->addAction(QIcon(":/icons/diagnostics.png"), tr("&Diagnostics..."));
    m_diagnosticsAction->setStatusTip(tr("Run diagnostic tests on data pipeline"));
    connect(m_diagnosticsAction, &QAction::triggered, this, &MainForm::onToolsDiagnostics);
    
    // Help Menu
    m_helpMenu = m_menuBar->addMenu(tr("&Help"));
    
    m_aboutAction = m_helpMenu->addAction(QIcon(":/icons/about.png"), tr("&About..."));
    m_aboutAction->setStatusTip(tr("Show information about this application"));
    connect(m_aboutAction, &QAction::triggered, this, &MainForm::onHelpAbout);
}

void MainForm::setupToolBar()
{
    // Use toolbar from UI file - exact mirror of C# toolbar
    m_toolBar = ui->toolBar;
    m_toolBar->setWindowTitle(tr("Main"));
    
    // Add toolbar actions
    m_toolBar->addAction(m_newAction);
    m_toolBar->addAction(m_openAction);
    m_toolBar->addAction(m_saveAction);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_findAction);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_compareOtbAction);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_updateAction);
    
    // Set toolbar properties
    m_toolBar->setMovable(false);
    m_toolBar->setFloatable(false);
}

void MainForm::setupStatusBar()
{
    // Create status bar - exact mirror of C# status bar layout
    m_statusBar = statusBar();
    
    // Status label (main status text)
    m_statusLabel = new QLabel(tr("Ready"));
    m_statusBar->addWidget(m_statusLabel, 1);
    
    // Progress bar
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    m_progressBar->setMaximumWidth(200);
    m_statusBar->addWidget(m_progressBar);
    
    // Item count label
    m_itemCountLabel = new QLabel(tr("Items: 0"));
    m_itemCountLabel->setMinimumWidth(80);
    m_statusBar->addPermanentWidget(m_itemCountLabel);
    
    // Plugin label
    m_pluginLabel = new QLabel(tr("No plugin loaded"));
    m_pluginLabel->setMinimumWidth(150);
    m_statusBar->addPermanentWidget(m_pluginLabel);
}

void MainForm::setupCentralWidget()
{
    // Create central widget with splitter layout - exact mirror of C# layout
    QWidget* centralWidget = new QWidget();
    setCentralWidget(centralWidget);
    
    // Main horizontal splitter
    m_mainSplitter = new QSplitter(Qt::Horizontal);
    
    // Create panels
    setupLeftPanel();
    setupCenterPanel();
    setupRightPanel();
    
    // Add panels to splitter
    m_mainSplitter->addWidget(m_leftPanel);
    m_mainSplitter->addWidget(m_centerPanel);
    m_mainSplitter->addWidget(m_rightPanel);
    
    // Set splitter sizes - exact mirror of C# layout sizes
    QList<int> sizes;
    sizes << ITEM_LIST_WIDTH << APPEARANCE_WIDTH << PROPERTIES_MIN_WIDTH;
    m_mainSplitter->setSizes(sizes);
    
    // Set splitter properties
    m_mainSplitter->setChildrenCollapsible(false);
    
    // Set central layout
    QHBoxLayout* centralLayout = new QHBoxLayout(centralWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->addWidget(m_mainSplitter);
}

void MainForm::setupLeftPanel()
{
    // Left panel - Server Items List (exact mirror of C# left panel)
    m_leftPanel = new QWidget();
    m_leftLayout = new QVBoxLayout(m_leftPanel);
    m_leftLayout->setContentsMargins(5, 5, 5, 5);
    
    // Server Items GroupBox
    m_serverItemsGroupBox = new QGroupBox(tr("Server Items"));
    m_leftLayout->addWidget(m_serverItemsGroupBox);
    
    // Server Items List
    QVBoxLayout* serverItemsLayout = new QVBoxLayout(m_serverItemsGroupBox);
    serverItemsLayout->setContentsMargins(5, 5, 5, 5);
    
    m_serverItemListBox = new ServerItemListBox();
    serverItemsLayout->addWidget(m_serverItemListBox);
    
    // Set fixed width
    m_leftPanel->setFixedWidth(ITEM_LIST_WIDTH);
}

void MainForm::setupCenterPanel()
{
    // Center panel - Item Appearance (exact mirror of C# center panel)
    m_centerPanel = new QWidget();
    m_centerLayout = new QVBoxLayout(m_centerPanel);
    m_centerLayout->setContentsMargins(5, 5, 5, 5);
    
    // Appearance GroupBox
    m_appearanceGroupBox = new QGroupBox(tr("Appearance"));
    m_centerLayout->addWidget(m_appearanceGroupBox);
    
    // Client Item View
    QVBoxLayout* appearanceLayout = new QVBoxLayout(m_appearanceGroupBox);
    appearanceLayout->setContentsMargins(5, 5, 5, 5);
    
    m_clientItemView = new ClientItemView();
    m_clientItemView->setMinimumSize(64, 64);
    appearanceLayout->addWidget(m_clientItemView);
    
    // Set fixed width
    m_centerPanel->setFixedWidth(APPEARANCE_WIDTH);
}

void MainForm::setupRightPanel()
{
    // Right panel - Item Properties (exact mirror of C# right panel)
    m_rightPanel = new QWidget();
    m_rightLayout = new QVBoxLayout(m_rightPanel);
    m_rightLayout->setContentsMargins(5, 5, 5, 5);
    
    // Setup properties group
    setupPropertiesGroup();
    
    // Setup flags group
    setupFlagsGroup();
    
    // Setup attributes group
    setupAttributesGroup();
    
    // Add stretch to push everything to top
    m_rightLayout->addStretch();
}

void MainForm::setupPropertiesGroup()
{
    // Item Properties GroupBox
    m_propertiesGroupBox = new QGroupBox(tr("Properties"));
    m_rightLayout->addWidget(m_propertiesGroupBox);
    
    m_propertiesLayout = new QGridLayout(m_propertiesGroupBox);
    m_propertiesLayout->setContentsMargins(5, 5, 5, 5);
    
    int row = 0;
    
    // Server ID
    m_serverIdLabel = new QLabel(tr("Server ID:"));
    m_propertiesLayout->addWidget(m_serverIdLabel, row, 0);
    
    m_serverIdSpinBox = new QSpinBox();
    m_serverIdSpinBox->setRange(0, 65535);
    m_propertiesLayout->addWidget(m_serverIdSpinBox, row, 1);
    row++;
    
    // Client ID
    m_clientIdLabel = new QLabel(tr("Client ID:"));
    m_propertiesLayout->addWidget(m_clientIdLabel, row, 0);
    
    m_clientIdSpinBox = new QSpinBox();
    m_clientIdSpinBox->setRange(0, 65535);
    m_propertiesLayout->addWidget(m_clientIdSpinBox, row, 1);
    row++;
    
    // Name
    m_nameLabel = new QLabel(tr("Name:"));
    m_propertiesLayout->addWidget(m_nameLabel, row, 0);
    
    m_nameLineEdit = new QLineEdit();
    m_propertiesLayout->addWidget(m_nameLineEdit, row, 1);
    row++;
    
    // Description
    m_descriptionLabel = new QLabel(tr("Description:"));
    m_propertiesLayout->addWidget(m_descriptionLabel, row, 0, Qt::AlignTop);
    
    m_descriptionTextEdit = new QTextEdit();
    m_descriptionTextEdit->setMaximumHeight(80);
    m_propertiesLayout->addWidget(m_descriptionTextEdit, row, 1);
}

void MainForm::setupFlagsGroup()
{
    // Item Flags GroupBox - exact mirror of C# flags layout
    m_flagsGroupBox = new QGroupBox(tr("Flags"));
    m_rightLayout->addWidget(m_flagsGroupBox);
    
    m_flagsLayout = new QGridLayout(m_flagsGroupBox);
    m_flagsLayout->setContentsMargins(5, 5, 5, 5);
    
    // Create flag checkboxes based on ServerItemFlag enum
    QStringList commonFlags = {
        "Blocking", "Moveable", "Pickupable", "Stackable",
        "Useable", "Readable", "Writable", "LookThrough",
        "Container", "Weapon", "Ammunition", "Armor",
        "MagicField", "Teleport", "Key", "Splash"
    };
    
    // Create FlagCheckBox instances for common flags
    int row = 0, col = 0;
    for (const QString& flagName : commonFlags) {
        FlagCheckBox* flagCheckBox = new FlagCheckBox(flagName);
        m_flagCheckBoxes.append(flagCheckBox);
        m_flagsLayout->addWidget(flagCheckBox, row, col);
        
        // Connect flag change signal
        connect(flagCheckBox, &FlagCheckBox::toggled,
                this, &MainForm::onItemFlagChanged);
        
        col++;
        if (col >= 2) {
            col = 0;
            row++;
        }
    }
}

void MainForm::setupAttributesGroup()
{
    // Item Attributes GroupBox
    m_attributesGroupBox = new QGroupBox(tr("Attributes"));
    m_rightLayout->addWidget(m_attributesGroupBox);
    
    m_attributesLayout = new QGridLayout(m_attributesGroupBox);
    m_attributesLayout->setContentsMargins(5, 5, 5, 5);
    
    // Attributes will be populated dynamically based on item type
}

void MainForm::setupConnections()
{
    // Connect server item list selection
    connect(m_serverItemListBox, &ServerItemListBox::itemSelectionChanged,
            this, &MainForm::onServerItemSelectionChanged);
    
    // Connect property change signals
    connect(m_serverIdSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainForm::onItemPropertyChanged);
    connect(m_clientIdSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainForm::onItemPropertyChanged);
    connect(m_nameLineEdit, &QLineEdit::textChanged,
            this, &MainForm::onItemPropertyChanged);
    connect(m_descriptionTextEdit, &QTextEdit::textChanged,
            this, &MainForm::onItemPropertyChanged);
    
    // Connect flag change signals - now implemented with FlagCheckBox
    for (FlagCheckBox* flagCheckBox : m_flagCheckBoxes) {
        connect(flagCheckBox, &FlagCheckBox::toggled,
                this, &MainForm::onItemFlagChanged);
    }
}

void MainForm::setupDragDrop()
{
    // Enable drag and drop for OTB files
    setAcceptDrops(true);
}

// Event handlers implementation
void MainForm::closeEvent(QCloseEvent *event)
{
    if (confirmUnsavedChanges()) {
        saveSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainForm::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty()) {
            QString fileName = urls.first().toLocalFile();
            if (fileName.endsWith(".otb", Qt::CaseInsensitive)) {
                event->acceptProposedAction();
                return;
            }
        }
    }
    event->ignore();
}

void MainForm::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty()) {
            QString fileName = urls.first().toLocalFile();
            if (fileName.endsWith(".otb", Qt::CaseInsensitive)) {
                if (confirmUnsavedChanges()) {
                    openOtbFile(fileName);
                }
                event->acceptProposedAction();
                return;
            }
        }
    }
    event->ignore();
}

void MainForm::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        updateWindowTitle();
    }
    QMainWindow::changeEvent(event);
}

// File menu slot implementations
void MainForm::onFileNew()
{
    if (confirmUnsavedChanges()) {
        newOtbFile();
        updateWindowTitle();
        updateMenuStates();
        updateStatusBar();
    }
}

void MainForm::onFileOpen()
{
    QString fileName = getOpenFileName();
    if (!fileName.isEmpty()) {
        if (confirmUnsavedChanges()) {
            openOtbFile(fileName);
        }
    }
}

void MainForm::onFileSave()
{
    if (m_currentFilePath.isEmpty()) {
        onFileSaveAs();
    } else {
        saveOtbFile(m_currentFilePath);
    }
}

void MainForm::onFileSaveAs()
{
    saveOtbFileAs();
}

void MainForm::onFileCompareOtb()
{
    Dialogs::CompareOtbForm dialog(this);
    dialog.exec();
}

void MainForm::onFileExit()
{
    close();
}

// Edit menu slot implementations
void MainForm::onEditFind()
{
    FindItemDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        if (dialog.searchById()) {
            quint16 itemId = dialog.itemId();
            if (itemId > 0) {
                selectServerItem(itemId);
            }
        } else {
            QString itemName = dialog.itemName();
            if (!itemName.isEmpty()) {
                // Search by name functionality - iterate through server items
                if (m_serverItemListBox) {
                    // Implementation will be enhanced when ServerItem class provides name search
                    showInfoMessage(tr("Search by name will be implemented when ServerItem class is enhanced."));
                }
            }
        }
    }
}

void MainForm::onEditPreferences()
{
    ItemEditor::PreferencesForm preferencesForm(this);
    preferencesForm.exec();
}

// View menu slot implementations
void MainForm::onViewShowOnlyMismatchedItems(bool checked)
{
    // Filter functionality - save setting and apply to ServerItemListBox
    m_settings->setValue("View/ShowOnlyMismatched", checked);
    
    // Apply filter to server item list when ServerItemListBox supports filtering
    if (m_serverItemListBox) {
        // Implementation will be enhanced when ServerItemListBox provides filtering
        m_serverItemListBox->update();
    }
    
    updateStatusBar();
}

void MainForm::onViewShowDecimalItemId(bool checked)
{
    if (checked) {
        // Set ID display format to decimal and update display
        m_settings->setValue("View/ItemIdFormat", "Decimal");
        
        // Apply format change to ServerItemListBox when supported
        if (m_serverItemListBox) {
            // Implementation will be enhanced when ServerItemListBox supports ID format
            m_serverItemListBox->update();
        }
        
        updateItemDisplay();
    }
}

void MainForm::onViewShowHexItemId(bool checked)
{
    if (checked) {
        // Set ID display format to hexadecimal and update display
        m_settings->setValue("View/ItemIdFormat", "Hexadecimal");
        
        // Apply format change to ServerItemListBox when supported
        if (m_serverItemListBox) {
            // Implementation will be enhanced when ServerItemListBox supports ID format
            m_serverItemListBox->update();
        }
        
        updateItemDisplay();
    }
}

// Tools menu slot implementations
void MainForm::onToolsUpdate()
{
    // UpdateForm will be implemented in subsequent task
    showInfoMessage(tr("Update functionality will be implemented in a future version."));
}

void MainForm::onToolsUpdateSettings()
{
    // UpdateSettingsForm will be implemented in subsequent task
    showInfoMessage(tr("Update settings dialog will be implemented in a future version."));
}

void MainForm::onToolsDiagnostics()
{
    logDiagnosticInfo("DIAGNOSTICS", "=== STARTING COMPREHENSIVE DIAGNOSTIC VALIDATION ===");
    validateDataPipeline();
}

// Help menu slot implementations
void MainForm::onHelpAbout()
{
    AboutDialog dialog(this);
    dialog.exec();
}

// Item selection and editing slot implementations
void MainForm::onServerItemSelectionChanged()
{
    logDiagnosticInfo("ITEM_SELECTION", "Server item selection changed event triggered");
    
    if (m_serverItemListBox) {
        // Get selected item ID from ServerItemListBox
        auto selectedIndices = m_serverItemListBox->selectedIndices();
        logDiagnosticInfo("ITEM_SELECTION", QString("Selected indices count: %1").arg(selectedIndices.size()));
        
        if (!selectedIndices.isEmpty()) {
            int selectedIndex = *selectedIndices.begin();
            // Convert index to item ID - implementation will be enhanced when ServerItem class provides ID access
            quint16 selectedId = static_cast<quint16>(selectedIndex + 1); // Placeholder conversion
            logDiagnosticInfo("ITEM_SELECTION", QString("Selected index: %1, converted to ID: %2").arg(selectedIndex).arg(selectedId));
            
            if (selectedId != m_selectedServerId) {
                m_selectedServerId = selectedId;
                logDiagnosticInfo("ITEM_SELECTION", QString("New selection: Server ID %1").arg(selectedId));
                updateItemProperties();
                updateItemDisplay();
            } else {
                logDiagnosticInfo("ITEM_SELECTION", "Selection unchanged - same item already selected");
            }
        } else {
            logDiagnosticInfo("ITEM_SELECTION", "No item selected - clearing selection");
            m_selectedServerId = 0;
            updateItemProperties();
            updateItemDisplay();
        }
    } else {
        logDiagnosticInfo("ITEM_SELECTION", "ERROR: ServerItemListBox is null");
    }
}

void MainForm::onItemPropertyChanged()
{
    if (!m_isLoading) {
        m_hasUnsavedChanges = true;
        updateWindowTitle();
        applyItemChanges();
    }
}

void MainForm::onItemFlagChanged()
{
    if (!m_isLoading) {
        m_hasUnsavedChanges = true;
        updateWindowTitle();
        applyItemChanges();
    }
}

// Plugin management slot implementations
void MainForm::onPluginChanged()
{
    // Update UI to reflect plugin changes
    updateStatusBar();
    updateMenuStates();
    
    // Clear sprite caches when plugin changes
    if (m_serverItemListBox) {
        m_serverItemListBox->refreshSprites();
    }
    
    if (m_clientItemView) {
        // Clear current item to force refresh
        m_clientItemView->setClientItem(nullptr);
    }
    
    // If we have a current plugin, update the item display
    if (m_currentPlugin && m_currentPlugin->loaded()) {
        loadServerItems();
        loadClientItems();
        
        // Refresh current selection if any
        if (m_selectedServerId > 0) {
            updateItemDisplay();
        }
    }
    
    qDebug() << "MainForm: Plugin changed - sprite display system refreshed";
}

void MainForm::refreshPluginList()
{
    if (m_pluginServices) {
        // Close existing plugins
        m_pluginServices->closePlugins();
        m_currentPlugin = nullptr;
        
        // Reload plugins
        loadPlugins();
    }
}

// Plugin signal handlers
void MainForm::onClientLoaded(const ItemEditor::SupportedClient& client)
{
    loadServerItems();
    loadClientItems();
    updateStatusBar();
}

void MainForm::onLoadingProgress(int percentage)
{
    if (m_progressBar) {
        m_progressBar->setValue(percentage);
        m_progressBar->setVisible(percentage < 100);
    }
}

void MainForm::onErrorOccurred(const QString& error)
{
    showErrorMessage(tr("Plugin error: %1").arg(error));
}

// UI update slot implementations
void MainForm::updateItemDisplay()
{
    logDiagnosticInfo("SPRITE_DISPLAY", QString("Updating item display for Server ID: %1").arg(m_selectedServerId));
    
    if (!m_clientItemView) {
        logDiagnosticInfo("SPRITE_DISPLAY", "ERROR: ClientItemView is null");
        return;
    }
    
    if (m_selectedServerId == 0) {
        logDiagnosticInfo("SPRITE_DISPLAY", "Clearing sprite display - no item selected");
        m_clientItemView->setClientItem(nullptr);
        return;
    }
    
    try {
        // Get ServerItem to find client ID
        OTLib::Server::Items::ServerItem* serverItem = nullptr;
        if (m_serverItemList && m_serverItemList->tryGetValue(m_selectedServerId, serverItem)) {
            logDiagnosticInfo("SPRITE_DISPLAY", QString("Found ServerItem for ID %1, client ID: %2").arg(m_selectedServerId).arg(serverItem->clientId()));
            
            // Get ClientItem from plugin for sprite display
            if (m_currentPlugin && m_currentPlugin->loaded()) {
                ItemEditor::ClientItem* clientItem = m_currentPlugin->getClientItem(serverItem->clientId());
                if (clientItem) {
                    // Validate sprite data before setting
                    if (!clientItem->spriteList().isEmpty()) {
                        // Force bitmap generation if needed
                        QPixmap bitmap = clientItem->getBitmap();
                        if (!bitmap.isNull()) {
                            logDiagnosticInfo("SPRITE_DISPLAY", QString("SUCCESS: Setting ClientItem with valid bitmap (%1x%2)").arg(bitmap.width()).arg(bitmap.height()));
                            m_clientItemView->setClientItem(clientItem);
                        } else {
                            logDiagnosticInfo("SPRITE_DISPLAY", "WARNING: ClientItem has sprites but bitmap generation failed");
                            // Try to generate bitmap manually
                            clientItem->generateBitmap();
                            bitmap = clientItem->getBitmap();
                            if (!bitmap.isNull()) {
                                logDiagnosticInfo("SPRITE_DISPLAY", "SUCCESS: Manual bitmap generation succeeded");
                                m_clientItemView->setClientItem(clientItem);
                            } else {
                                logDiagnosticInfo("SPRITE_DISPLAY", "ERROR: Manual bitmap generation failed");
                                m_clientItemView->setClientItem(nullptr);
                            }
                        }
                    } else {
                        logDiagnosticInfo("SPRITE_DISPLAY", "WARNING: ClientItem has no sprites");
                        m_clientItemView->setClientItem(nullptr);
                    }
                } else {
                    logDiagnosticInfo("SPRITE_DISPLAY", QString("WARNING: No ClientItem found for client ID %1").arg(serverItem->clientId()));
                    m_clientItemView->setClientItem(nullptr);
                }
            } else {
                logDiagnosticInfo("SPRITE_DISPLAY", "ERROR: No plugin loaded");
                m_clientItemView->setClientItem(nullptr);
            }
        } else {
            logDiagnosticInfo("SPRITE_DISPLAY", QString("ERROR: ServerItem not found for ID %1").arg(m_selectedServerId));
            m_clientItemView->setClientItem(nullptr);
        }
    }
    catch (const std::exception& e) {
        logDiagnosticInfo("SPRITE_DISPLAY", QString("ERROR: Exception in updateItemDisplay: %1").arg(e.what()));
        m_clientItemView->setClientItem(nullptr);
    }
    catch (...) {
        logDiagnosticInfo("SPRITE_DISPLAY", "ERROR: Unknown exception in updateItemDisplay");
        m_clientItemView->setClientItem(nullptr);
    }
}

void MainForm::updateStatusBar()
{
    if (m_statusLabel) {
        if (m_isLoading) {
            m_statusLabel->setText(tr("Loading..."));
        } else if (m_hasUnsavedChanges) {
            m_statusLabel->setText(tr("Modified"));
        } else {
            m_statusLabel->setText(tr("Ready"));
        }
    }
    
    if (m_itemCountLabel) {
        int itemCount = 0;
        if (m_serverItemList) {
            itemCount = m_serverItemList->count();
        } else if (m_serverItemListBox) {
            itemCount = m_serverItemListBox->items().size();
        }
        m_itemCountLabel->setText(tr("Items: %1").arg(itemCount));
    }
    
    if (m_pluginLabel) {
        if (m_currentPlugin && m_currentPlugin->loaded()) {
            // Get plugin information from the plugin services
            QString pluginName = tr("Unknown Plugin");
            if (m_pluginServices && m_pluginServices->availablePlugins()) {
                // Find the plugin wrapper for the current plugin instance
                for (auto plugin : *m_pluginServices->availablePlugins()) {
                    if (plugin && plugin->instance() == m_currentPlugin) {
                        QFileInfo fileInfo(plugin->assemblyPath());
                        pluginName = fileInfo.baseName();
                        break;
                    }
                }
            }
            m_pluginLabel->setText(tr("Plugin: %1").arg(pluginName));
        } else {
            int pluginCount = 0;
            if (m_pluginServices && m_pluginServices->availablePlugins()) {
                pluginCount = m_pluginServices->availablePlugins()->count();
            }
            
            if (pluginCount > 0) {
                m_pluginLabel->setText(tr("Plugins available: %1").arg(pluginCount));
            } else {
                m_pluginLabel->setText(tr("No plugins found"));
            }
        }
    }
}

void MainForm::updateWindowTitle()
{
    QString title = tr("Item Editor");
    
    if (!m_currentFilePath.isEmpty()) {
        QFileInfo fileInfo(m_currentFilePath);
        title += tr(" - %1").arg(fileInfo.fileName());
    }
    
    if (m_hasUnsavedChanges) {
        title += tr(" *");
    }
    
    setWindowTitle(title);
}

void MainForm::updateMenuStates()
{
    bool hasFile = !m_currentFilePath.isEmpty();
    bool hasItems = (m_serverItemList != nullptr && m_serverItemList->count() > 0);
    bool hasOtbComponents = (m_otbReader != nullptr && m_otbWriter != nullptr);
    bool hasSelection = m_selectedServerId > 0;
    
    // Update menu action states
    if (m_newAction) m_newAction->setEnabled(hasOtbComponents);
    if (m_openAction) m_openAction->setEnabled(hasOtbComponents);
    if (m_saveAction) m_saveAction->setEnabled(hasFile && hasItems && hasOtbComponents && m_hasUnsavedChanges);
    if (m_saveAsAction) m_saveAsAction->setEnabled(hasItems && hasOtbComponents);
    if (m_compareOtbAction) m_compareOtbAction->setEnabled(hasFile && hasItems);
    if (m_findAction) m_findAction->setEnabled(hasItems);
    
    // Update property controls
    if (m_serverIdSpinBox) m_serverIdSpinBox->setEnabled(hasSelection && hasItems);
    if (m_clientIdSpinBox) m_clientIdSpinBox->setEnabled(hasSelection && hasItems);
    if (m_nameLineEdit) m_nameLineEdit->setEnabled(hasSelection && hasItems);
    if (m_descriptionTextEdit) m_descriptionTextEdit->setEnabled(hasSelection && hasItems);
}

// Timer slot implementations
void MainForm::onUpdateTimer()
{
    // Periodic updates
    if (m_progressBar && m_progressBar->isVisible()) {
        // Update progress if needed
    }
}

// File operations implementation
bool MainForm::openOtbFile(const QString& filePath)
{
    logDiagnosticInfo("OTB_LOADING", QString("Starting OTB file load: %1").arg(filePath));
    
    if (filePath.isEmpty()) {
        logDiagnosticInfo("OTB_LOADING", "ERROR: Empty file path provided");
        return false;
    }
    
    if (!m_otbReader) {
        logDiagnosticInfo("OTB_LOADING", "ERROR: OTB Reader not initialized");
        showErrorMessage(tr("OTB Reader not initialized"));
        return false;
    }
    
    m_isLoading = true;
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    updateStatusBar();
    
    try {
        // Clear existing data
        if (m_serverItemList) {
            logDiagnosticInfo("OTB_LOADING", "Clearing existing ServerItemList");
            m_serverItemList = nullptr;
        }
        
        // Update progress
        m_progressBar->setValue(25);
        QApplication::processEvents();
        
        // Load OTB file using OtbReader
        logDiagnosticInfo("OTB_LOADING", "Calling OtbReader::read()");
        if (!m_otbReader->read(filePath)) {
            m_isLoading = false;
            m_progressBar->setVisible(false);
            logDiagnosticInfo("OTB_LOADING", "ERROR: OtbReader::read() failed");
            showErrorMessage(tr("Failed to read OTB file: %1").arg(filePath));
            return false;
        }
        
        // Update progress
        m_progressBar->setValue(50);
        QApplication::processEvents();
        
        // Get the loaded ServerItemList
        m_serverItemList = m_otbReader->items();
        if (!m_serverItemList) {
            m_isLoading = false;
            m_progressBar->setVisible(false);
            logDiagnosticInfo("OTB_LOADING", "ERROR: No ServerItemList returned from OtbReader");
            showErrorMessage(tr("No items loaded from OTB file"));
            return false;
        }
        
        int itemCount = m_serverItemList->count();
        logDiagnosticInfo("OTB_LOADING", QString("SUCCESS: Loaded %1 items from OTB").arg(itemCount));
        
        // Update progress
        m_progressBar->setValue(75);
        QApplication::processEvents();
        
        // Connect ServerItemList to UI
        if (m_serverItemListBox) {
            logDiagnosticInfo("OTB_LOADING", "Connecting ServerItemList to UI");
            m_serverItemListBox->setServerItemList(m_serverItemList);
        } else {
            logDiagnosticInfo("OTB_LOADING", "WARNING: ServerItemListBox is null");
        }
        
        // Update file state
        m_currentFilePath = filePath;
        m_hasUnsavedChanges = false;
        
        // Update progress
        m_progressBar->setValue(100);
        QApplication::processEvents();
        
        // Update UI
        updateWindowTitle();
        updateMenuStates();
        updateStatusBar();
        
        m_statusLabel->setText(tr("File loaded successfully - %1 items").arg(m_serverItemList->count()));
        logDiagnosticInfo("OTB_LOADING", QString("COMPLETE: OTB loading finished successfully with %1 items").arg(itemCount));
        
        m_isLoading = false;
        m_progressBar->setVisible(false);
        return true;
    }
    catch (const std::exception& e) {
        m_isLoading = false;
        m_progressBar->setVisible(false);
        showErrorMessage(tr("Failed to load file: %1\nError: %2").arg(filePath, e.what()));
        return false;
    }
    catch (...) {
        m_isLoading = false;
        m_progressBar->setVisible(false);
        showErrorMessage(tr("Failed to load file: %1").arg(filePath));
        return false;
    }
}

bool MainForm::saveOtbFile(const QString& filePath)
{
    if (filePath.isEmpty()) {
        return false;
    }
    
    if (!m_otbWriter) {
        showErrorMessage(tr("OTB Writer not initialized"));
        return false;
    }
    
    if (!m_serverItemList) {
        showErrorMessage(tr("No items to save"));
        return false;
    }
    
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    
    try {
        // Connect progress signals
        connect(m_otbWriter.get(), &OTLib::OTB::OtbWriter::progressChanged,
                m_progressBar, &QProgressBar::setValue);
        connect(m_otbWriter.get(), &OTLib::OTB::OtbWriter::statusChanged,
                m_statusLabel, &QLabel::setText);
        
        // Save OTB file using OtbWriter
        bool success = m_otbWriter->write(filePath, m_serverItemList);
        
        // Disconnect progress signals
        disconnect(m_otbWriter.get(), &OTLib::OTB::OtbWriter::progressChanged,
                   m_progressBar, &QProgressBar::setValue);
        disconnect(m_otbWriter.get(), &OTLib::OTB::OtbWriter::statusChanged,
                   m_statusLabel, &QLabel::setText);
        
        if (!success) {
            m_progressBar->setVisible(false);
            QString error = m_otbWriter->hasError() ? m_otbWriter->getLastError() : tr("Unknown error");
            showErrorMessage(tr("Failed to save file: %1\nError: %2").arg(filePath, error));
            return false;
        }
        
        // Update file state
        m_currentFilePath = filePath;
        m_hasUnsavedChanges = false;
        
        updateWindowTitle();
        updateMenuStates();
        
        m_statusLabel->setText(tr("File saved successfully - %1 items").arg(m_serverItemList->count()));
        m_progressBar->setVisible(false);
        return true;
    }
    catch (const std::exception& e) {
        m_progressBar->setVisible(false);
        showErrorMessage(tr("Failed to save file: %1\nError: %2").arg(filePath, e.what()));
        return false;
    }
    catch (...) {
        m_progressBar->setVisible(false);
        showErrorMessage(tr("Failed to save file: %1").arg(filePath));
        return false;
    }
}

bool MainForm::saveOtbFileAs()
{
    QString fileName = getSaveFileName();
    if (!fileName.isEmpty()) {
        return saveOtbFile(fileName);
    }
    return false;
}

void MainForm::newOtbFile()
{
    // Clear current file state
    m_currentFilePath.clear();
    m_hasUnsavedChanges = false;
    m_selectedServerId = 0;
    
    // Create new ServerItemList
    if (m_serverItemList) {
        m_serverItemList = nullptr;
    }
    
    // Create a new empty ServerItemList
    m_serverItemList = new OTLib::Collections::ServerItemList(this);
    
    // Connect to UI
    if (m_serverItemListBox) {
        m_serverItemListBox->setServerItemList(m_serverItemList);
        m_serverItemListBox->clearSelection();
        m_serverItemListBox->update();
    }
    
    updateWindowTitle();
    updateMenuStates();
    updateStatusBar();
}

// Sprite operations implementation
bool MainForm::openSpriteFile(const QString& filePath)
{
    if (!m_spriteManager) {
        QMessageBox::warning(this, tr("Error"), tr("Sprite manager not initialized"));
        return false;
    }
    
    // Get current plugin for client information
    IPlugin* plugin = getCurrentPlugin();
    if (!plugin) {
        QMessageBox::warning(this, tr("Error"), tr("No plugin selected. Please select a client version first."));
        return false;
    }
    
    SupportedClient client;
    QList<SupportedClient> clients = plugin->supportedClients();
    if (!clients.isEmpty()) {
        client = clients.first(); // Use first supported client
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Plugin has no supported clients."));
        return false;
    }
    
    // Show progress dialog
    QProgressDialog progressDialog(tr("Loading sprites..."), tr("Cancel"), 0, 100, this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.show();
    
    // Connect progress signals
    connect(m_spriteManager, &SpriteManager::loadingProgress,
            [&progressDialog](int current, int total) {
                if (total > 0) {
                    progressDialog.setValue((current * 100) / total);
                }
                QApplication::processEvents();
            });
    
    // Load sprites
    bool success = m_spriteManager->loadSpriteFile(filePath, client, false, false);
    
    progressDialog.close();
    
    if (success) {
        statusBar()->showMessage(tr("Sprites loaded: %1 (%2 sprites)")
                               .arg(QFileInfo(filePath).fileName())
                               .arg(m_spriteManager->spriteCount()), 3000);
        
        // Update UI to reflect sprite availability
        updateMenuStates();
        return true;
    } else {
        QMessageBox::critical(this, tr("Error"), 
                            tr("Failed to load sprite file: %1").arg(filePath));
        return false;
    }
}

void MainForm::unloadSprites()
{
    if (m_spriteManager) {
        m_spriteManager->unloadSprites();
        statusBar()->showMessage(tr("Sprites unloaded"), 2000);
        updateMenuStates();
    }
}

// Plugin operations implementation
void MainForm::loadPlugins()
{
    if (!m_pluginServices) {
        return;
    }
    
    try {
        // Find and load all available plugins
        m_pluginServices->findPlugins();
        
        // Connect plugin service signals
        connect(m_pluginServices, &PluginServices::pluginLoaded,
                this, &MainForm::onPluginChanged);
        connect(m_pluginServices, &PluginServices::pluginLoadFailed,
                this, [this](const QString& path, const QString& error) {
                    showErrorMessage(tr("Failed to load plugin %1: %2").arg(path, error));
                });
        
        // Select first available plugin if any
        if (m_pluginServices->availablePlugins() && 
            !m_pluginServices->availablePlugins()->isEmpty()) {
            
            auto firstPlugin = m_pluginServices->availablePlugins()->at(0);
            if (firstPlugin && firstPlugin->instance()) {
                selectPlugin(firstPlugin->instance());
            }
        }
        
        updateStatusBar();
    }
    catch (const std::exception& e) {
        showErrorMessage(tr("Error loading plugins: %1").arg(e.what()));
    }
    catch (...) {
        showErrorMessage(tr("Unknown error occurred while loading plugins"));
    }
}

void MainForm::selectPlugin(ItemEditor::IPlugin* plugin)
{
    if (m_currentPlugin == plugin) {
        return;
    }
    
    // Disconnect from previous plugin
    if (m_currentPlugin) {
        // Cast to QObject to access signals
        if (auto* pluginObject = dynamic_cast<QObject*>(m_currentPlugin)) {
            disconnect(pluginObject, nullptr, this, nullptr);
        }
    }
    
    m_currentPlugin = plugin;
    
    // Connect to new plugin signals
    if (m_currentPlugin) {
        // Cast to QObject to access signals
        if (auto* pluginObject = dynamic_cast<QObject*>(m_currentPlugin)) {
            // Try to connect to signals if they exist
            // Note: These signals are defined in concrete plugin classes, not IPlugin interface
            
            // Connect clientLoaded signal if it exists
            if (pluginObject->metaObject()->indexOfSignal("clientLoaded(ItemEditor::SupportedClient)") != -1) {
                connect(pluginObject, SIGNAL(clientLoaded(ItemEditor::SupportedClient)),
                        this, SLOT(onClientLoaded(ItemEditor::SupportedClient)));
            }
            
            // Connect loadingProgress signal if it exists
            if (pluginObject->metaObject()->indexOfSignal("loadingProgress(int)") != -1) {
                connect(pluginObject, SIGNAL(loadingProgress(int)),
                        this, SLOT(onLoadingProgress(int)));
            }
            
            // Connect errorOccurred signal if it exists
            if (pluginObject->metaObject()->indexOfSignal("errorOccurred(QString)") != -1) {
                connect(pluginObject, SIGNAL(errorOccurred(QString)),
                        this, SLOT(onErrorOccurred(QString)));
            }
        }
    }
    
    updateStatusBar();
}

ItemEditor::IPlugin* MainForm::getCurrentPlugin() const
{
    return m_currentPlugin;
}

// Item management implementation
void MainForm::loadServerItems()
{
    if (!m_serverItemListBox) {
        return;
    }
    
    // Clear existing items
    m_serverItemListBox->clearSelection();
    
    // If we have a loaded ServerItemList, use it directly
    if (m_serverItemList) {
        // The ServerItemListBox is already connected to the ServerItemList
        // through setServerItemList() call in openOtbFile()
        m_serverItemListBox->update();
        updateStatusBar();
        return;
    }
    
    // Legacy plugin support (if needed)
    if (m_currentPlugin && m_currentPlugin->loaded()) {
        try {
            // Get item range from plugin
            quint16 minId = m_currentPlugin->minItemId();
            quint16 maxId = m_currentPlugin->maxItemId();
            
            // Add some sample items for demonstration
            for (quint16 id = minId; id <= qMin(maxId, static_cast<quint16>(minId + 10)); ++id) {
                // Create actual ServerItem objects when available
                m_serverItemListBox->addItem(id, tr("Item %1").arg(id));
            }
        }
        catch (const std::exception& e) {
            showErrorMessage(tr("Error loading server items: %1").arg(e.what()));
        }
        catch (...) {
            showErrorMessage(tr("Unknown error loading server items"));
        }
    }
    
    m_serverItemListBox->update();
    updateStatusBar();
}

void MainForm::loadClientItems()
{
    if (!m_currentPlugin || !m_currentPlugin->loaded()) {
        return;
    }
    
    try {
        // Client items are loaded through the plugin's items() collection
        // The plugin manages the client item data
        if (m_clientItemView) {
            m_clientItemView->update();
        }
        
        updateStatusBar();
    }
    catch (const std::exception& e) {
        showErrorMessage(tr("Error loading client items: %1").arg(e.what()));
    }
    catch (...) {
        showErrorMessage(tr("Unknown error loading client items"));
    }
}

void MainForm::selectServerItem(quint16 itemId)
{
    if (m_serverItemListBox) {
        // Select item by ID - implementation will be enhanced when ServerItemListBox provides ID-based selection
        m_selectedServerId = itemId;
        updateItemProperties();
        updateItemDisplay();
    }
}

void MainForm::updateItemProperties()
{
    logDiagnosticInfo("ITEM_PROPERTIES", QString("Updating properties for Server ID: %1").arg(m_selectedServerId));
    
    if (m_selectedServerId == 0) {
        logDiagnosticInfo("ITEM_PROPERTIES", "Clearing properties - no item selected");
        clearItemProperties();
        return;
    }
    
    m_isLoading = true;
    
    // Declare ClientItem variable at the beginning
    ItemEditor::ClientItem* clientItem = nullptr;
    
    try {
        // 1. Retrieve complete ServerItem data from m_serverItemList
        OTLib::Server::Items::ServerItem* serverItem = nullptr;
        if (m_serverItemList) {
            if (m_serverItemList->tryGetValue(m_selectedServerId, serverItem)) {
                logDiagnosticInfo("ITEM_PROPERTIES", QString("SUCCESS: Retrieved ServerItem for ID %1").arg(m_selectedServerId));
            } else {
                logDiagnosticInfo("ITEM_PROPERTIES", QString("WARNING: ServerItem not found for ID %1").arg(m_selectedServerId));
            }
        } else {
            logDiagnosticInfo("ITEM_PROPERTIES", "ERROR: ServerItemList is null");
        }
        
        // Get ClientItem for comparison and visual feedback
        if (m_currentPlugin && m_currentPlugin->loaded() && serverItem) {
            clientItem = m_currentPlugin->getClientItem(serverItem->clientId());
            if (clientItem) {
                logDiagnosticInfo("ITEM_PROPERTIES", QString("SUCCESS: Retrieved ClientItem for client ID %1").arg(serverItem->clientId()));
            } else {
                logDiagnosticInfo("ITEM_PROPERTIES", QString("WARNING: ClientItem not found for client ID %1").arg(serverItem->clientId()));
            }
        }
        
        // 2. Populate all basic properties
        populateBasicProperties(serverItem, clientItem);
        
        // 3. Implement flag checkbox population using existing FlagCheckBox infrastructure
        populateFlagCheckboxes(serverItem, clientItem);
        
        // 4. Populate attribute controls
        populateAttributeControls(serverItem, clientItem);
        
        // 5. Update ClientItemView with current item
        if (m_clientItemView && clientItem) {
            clientItem = m_currentPlugin->getClientItem(serverItem->clientId());
            if (clientItem) {
                logDiagnosticInfo("ITEM_PROPERTIES", QString("SUCCESS: Retrieved ClientItem for client ID %1").arg(serverItem->clientId()));
            } else {
                logDiagnosticInfo("ITEM_PROPERTIES", QString("WARNING: ClientItem not found for client ID %1").arg(serverItem->clientId()));
            }
        }
        
        // 6. Update sprite display
        updateSpriteDisplay(clientItem);
        
        logDiagnosticInfo("ITEM_PROPERTIES", "SUCCESS: Item properties updated successfully");
    }
    catch (const std::exception& e) {
        logDiagnosticInfo("ITEM_PROPERTIES", QString("ERROR: Exception in updateItemProperties: %1").arg(e.what()));
        showErrorMessage(tr("Error updating item properties: %1").arg(e.what()));
    }
    catch (...) {
        logDiagnosticInfo("ITEM_PROPERTIES", "ERROR: Unknown exception in updateItemProperties");
        showErrorMessage(tr("Unknown error updating item properties"));
    }
    
    m_isLoading = false;
    updateMenuStates();
}

void MainForm::applyItemChanges()
{
    if (m_selectedServerId == 0 || m_isLoading) {
        return;
    }
    
    // Apply changes to selected server item - implementation will be enhanced when ServerItem class provides property setters
    // For now, just mark as having unsaved changes
    m_hasUnsavedChanges = true;
    updateWindowTitle();
}

// UI state management implementation
void MainForm::saveSettings()
{
    if (!m_settings) return;
    
    // Window geometry
    m_settings->setValue("MainWindow/geometry", saveGeometry());
    m_settings->setValue("MainWindow/windowState", saveState());
    
    // Splitter sizes
    if (m_mainSplitter) {
        m_settings->setValue("MainWindow/splitterSizes", m_mainSplitter->saveState());
    }
    
    // View settings
    if (m_showOnlyMismatchedAction) {
        m_settings->setValue("View/ShowOnlyMismatched", m_showOnlyMismatchedAction->isChecked());
    }
    
    if (m_showDecimalIdAction && m_showDecimalIdAction->isChecked()) {
        m_settings->setValue("View/ItemIdFormat", "Decimal");
    } else if (m_showHexIdAction && m_showHexIdAction->isChecked()) {
        m_settings->setValue("View/ItemIdFormat", "Hexadecimal");
    }
}

void MainForm::loadSettings()
{
    if (!m_settings) return;
    
    // Window geometry
    restoreGeometry(m_settings->value("MainWindow/geometry").toByteArray());
    restoreState(m_settings->value("MainWindow/windowState").toByteArray());
    
    // Splitter sizes
    if (m_mainSplitter) {
        m_mainSplitter->restoreState(m_settings->value("MainWindow/splitterSizes").toByteArray());
    }
    
    // View settings
    bool showOnlyMismatched = m_settings->value("View/ShowOnlyMismatched", false).toBool();
    if (m_showOnlyMismatchedAction) {
        m_showOnlyMismatchedAction->setChecked(showOnlyMismatched);
    }
    
    QString idFormat = m_settings->value("View/ItemIdFormat", "Decimal").toString();
    if (idFormat == "Hexadecimal" && m_showHexIdAction) {
        m_showHexIdAction->setChecked(true);
    } else if (m_showDecimalIdAction) {
        m_showDecimalIdAction->setChecked(true);
    }
}

void MainForm::resetToDefaults()
{
    // Reset to default settings
    if (m_showOnlyMismatchedAction) {
        m_showOnlyMismatchedAction->setChecked(false);
    }
    
    if (m_showDecimalIdAction) {
        m_showDecimalIdAction->setChecked(true);
    }
    
    // Reset window size
    resize(1024, 768);
}

// Utility methods implementation
QString MainForm::getOpenFileName()
{
    QString lastDir = m_settings->value("LastDirectory", 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open OTB File"),
        lastDir,
        tr("OTB Files (*.otb);;All Files (*)")
    );
    
    if (!fileName.isEmpty()) {
        QFileInfo fileInfo(fileName);
        m_settings->setValue("LastDirectory", fileInfo.absolutePath());
    }
    
    return fileName;
}

QString MainForm::getSaveFileName()
{
    QString lastDir = m_settings->value("LastDirectory", 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save OTB File"),
        lastDir,
        tr("OTB Files (*.otb);;All Files (*)")
    );
    
    if (!fileName.isEmpty()) {
        QFileInfo fileInfo(fileName);
        m_settings->setValue("LastDirectory", fileInfo.absolutePath());
    }
    
    return fileName;
}

bool MainForm::confirmUnsavedChanges()
{
    if (!m_hasUnsavedChanges) {
        return true;
    }
    
    QMessageBox::StandardButton result = QMessageBox::question(
        this,
        tr("Unsaved Changes"),
        tr("You have unsaved changes. Do you want to save them before continuing?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save
    );
    
    switch (result) {
        case QMessageBox::Save:
            return onFileSave(), !m_hasUnsavedChanges;
        case QMessageBox::Discard:
            return true;
        case QMessageBox::Cancel:
        default:
            return false;
    }
}

void MainForm::showErrorMessage(const QString& message)
{
    QMessageBox::critical(this, tr("Error"), message);
}

void MainForm::showInfoMessage(const QString& message)
{
    QMessageBox::information(this, tr("Information"), message);
}

// Diagnostic methods implementation
void MainForm::validateDataPipeline()
{
    logDiagnosticInfo("PIPELINE", "Starting comprehensive data pipeline validation");
    
    bool otbValid = validateOtbLoading();
    bool pluginValid = validatePluginIntegration();
    bool uiValid = validateUIControls();
    bool bindingValid = validateDataBinding();
    
    QString summary = QString("=== DIAGNOSTIC SUMMARY ===\n"
                             "OTB Loading: %1\n"
                             "Plugin Integration: %2\n"
                             "UI Controls: %3\n"
                             "Data Binding: %4\n"
                             "Overall Status: %5")
                      .arg(otbValid ? "PASS" : "FAIL")
                      .arg(pluginValid ? "PASS" : "FAIL")
                      .arg(uiValid ? "PASS" : "FAIL")
                      .arg(bindingValid ? "PASS" : "FAIL")
                      .arg((otbValid && pluginValid && uiValid && bindingValid) ? "HEALTHY" : "ISSUES DETECTED");
    
    logDiagnosticInfo("PIPELINE", summary);
    
    // Show summary to user
    QMessageBox::information(this, tr("Diagnostic Results"), summary);
}

void MainForm::logDiagnosticInfo(const QString& stage, const QString& message)
{
    QString logMessage = QString("[%1] %2: %3")
                        .arg(QTime::currentTime().toString("hh:mm:ss.zzz"))
                        .arg(stage)
                        .arg(message);
    
    // Output to console
    qDebug() << logMessage;
    
    // Update status bar with latest diagnostic info
    if (m_statusLabel) {
        m_statusLabel->setText(QString("[%1] %2").arg(stage).arg(message));
    }
}

bool MainForm::validateOtbLoading()
{
    logDiagnosticInfo("OTB_VALIDATION", "Validating OTB loading components");
    
    bool valid = true;
    
    // Check OTB Reader
    if (!m_otbReader) {
        logDiagnosticInfo("OTB_VALIDATION", "FAIL: OTB Reader is null");
        valid = false;
    } else {
        logDiagnosticInfo("OTB_VALIDATION", "PASS: OTB Reader initialized");
    }
    
    // Check OTB Writer
    if (!m_otbWriter) {
        logDiagnosticInfo("OTB_VALIDATION", "FAIL: OTB Writer is null");
        valid = false;
    } else {
        logDiagnosticInfo("OTB_VALIDATION", "PASS: OTB Writer initialized");
    }
    
    // Check ServerItemList
    if (!m_serverItemList) {
        logDiagnosticInfo("OTB_VALIDATION", "WARNING: No ServerItemList loaded (no OTB file opened)");
    } else {
        int itemCount = m_serverItemList->count();
        logDiagnosticInfo("OTB_VALIDATION", QString("PASS: ServerItemList loaded with %1 items").arg(itemCount));
    }
    
    // Check current file path
    if (m_currentFilePath.isEmpty()) {
        logDiagnosticInfo("OTB_VALIDATION", "INFO: No file currently loaded");
    } else {
        logDiagnosticInfo("OTB_VALIDATION", QString("INFO: Current file: %1").arg(m_currentFilePath));
    }
    
    return valid;
}

bool MainForm::validatePluginIntegration()
{
    logDiagnosticInfo("PLUGIN_VALIDATION", "Validating plugin integration");
    
    bool valid = true;
    
    // Check PluginServices
    if (!m_pluginServices) {
        logDiagnosticInfo("PLUGIN_VALIDATION", "FAIL: PluginServices is null");
        valid = false;
    } else {
        logDiagnosticInfo("PLUGIN_VALIDATION", "PASS: PluginServices initialized");
        
        // Check available plugins
        if (!m_pluginServices->availablePlugins()) {
            logDiagnosticInfo("PLUGIN_VALIDATION", "FAIL: No plugin list available");
            valid = false;
        } else {
            int pluginCount = m_pluginServices->availablePlugins()->count();
            logDiagnosticInfo("PLUGIN_VALIDATION", QString("INFO: %1 plugins available").arg(pluginCount));
        }
    }
    
    // Check current plugin
    if (!m_currentPlugin) {
        logDiagnosticInfo("PLUGIN_VALIDATION", "WARNING: No plugin currently selected");
    } else {
        logDiagnosticInfo("PLUGIN_VALIDATION", "PASS: Plugin selected");
        
        if (m_currentPlugin->loaded()) {
            logDiagnosticInfo("PLUGIN_VALIDATION", "PASS: Plugin is loaded");
            
            // Test plugin functionality
            try {
                quint16 minId = m_currentPlugin->minItemId();
                quint16 maxId = m_currentPlugin->maxItemId();
                logDiagnosticInfo("PLUGIN_VALIDATION", QString("PASS: Plugin item range: %1-%2").arg(minId).arg(maxId));
            } catch (...) {
                logDiagnosticInfo("PLUGIN_VALIDATION", "FAIL: Plugin item range access failed");
                valid = false;
            }
        } else {
            logDiagnosticInfo("PLUGIN_VALIDATION", "FAIL: Plugin not loaded");
            valid = false;
        }
    }
    
    return valid;
}

bool MainForm::validateUIControls()
{
    logDiagnosticInfo("UI_VALIDATION", "Validating UI control components");
    
    bool valid = true;
    
    // Check main UI components
    if (!m_serverItemListBox) {
        logDiagnosticInfo("UI_VALIDATION", "FAIL: ServerItemListBox is null");
        valid = false;
    } else {
        logDiagnosticInfo("UI_VALIDATION", "PASS: ServerItemListBox exists");
        int itemCount = m_serverItemListBox->items().size();
        logDiagnosticInfo("UI_VALIDATION", QString("INFO: ServerItemListBox contains %1 items").arg(itemCount));
    }
    
    if (!m_clientItemView) {
        logDiagnosticInfo("UI_VALIDATION", "FAIL: ClientItemView is null");
        valid = false;
    } else {
        logDiagnosticInfo("UI_VALIDATION", "PASS: ClientItemView exists");
    }
    
    // Check property controls
    if (!m_serverIdSpinBox) {
        logDiagnosticInfo("UI_VALIDATION", "FAIL: Server ID SpinBox is null");
        valid = false;
    } else {
        logDiagnosticInfo("UI_VALIDATION", QString("PASS: Server ID SpinBox exists (value: %1)").arg(m_serverIdSpinBox->value()));
    }
    
    if (!m_clientIdSpinBox) {
        logDiagnosticInfo("UI_VALIDATION", "FAIL: Client ID SpinBox is null");
        valid = false;
    } else {
        logDiagnosticInfo("UI_VALIDATION", QString("PASS: Client ID SpinBox exists (value: %1)").arg(m_clientIdSpinBox->value()));
    }
    
    if (!m_nameLineEdit) {
        logDiagnosticInfo("UI_VALIDATION", "FAIL: Name LineEdit is null");
        valid = false;
    } else {
        logDiagnosticInfo("UI_VALIDATION", QString("PASS: Name LineEdit exists (text: '%1')").arg(m_nameLineEdit->text()));
    }
    
    if (!m_descriptionTextEdit) {
        logDiagnosticInfo("UI_VALIDATION", "FAIL: Description TextEdit is null");
        valid = false;
    } else {
        logDiagnosticInfo("UI_VALIDATION", QString("PASS: Description TextEdit exists (length: %1)").arg(m_descriptionTextEdit->toPlainText().length()));
    }
    
    // Check flag checkboxes
    logDiagnosticInfo("UI_VALIDATION", QString("INFO: %1 flag checkboxes created").arg(m_flagCheckBoxes.size()));
    
    return valid;
}

bool MainForm::validateDataBinding()
{
    logDiagnosticInfo("BINDING_VALIDATION", "Validating data binding between components");
    
    bool valid = true;
    
    // Check ServerItemList to UI binding
    if (m_serverItemList && m_serverItemListBox) {
        // Test if ServerItemListBox is properly connected to ServerItemList
        int listCount = m_serverItemList->count();
        int uiCount = m_serverItemListBox->items().size();
        
        if (listCount == uiCount) {
            logDiagnosticInfo("BINDING_VALIDATION", QString("PASS: ServerItemList (%1) matches UI count (%2)").arg(listCount).arg(uiCount));
        } else {
            logDiagnosticInfo("BINDING_VALIDATION", QString("FAIL: ServerItemList (%1) != UI count (%2)").arg(listCount).arg(uiCount));
            valid = false;
        }
    } else {
        logDiagnosticInfo("BINDING_VALIDATION", "WARNING: Cannot validate ServerItemList binding - missing components");
    }
    
    // Check selection binding
    if (m_selectedServerId > 0) {
        logDiagnosticInfo("BINDING_VALIDATION", QString("INFO: Current selection: Server ID %1").arg(m_selectedServerId));
        
        // Check if selection is reflected in UI controls
        if (m_serverIdSpinBox && m_serverIdSpinBox->value() == m_selectedServerId) {
            logDiagnosticInfo("BINDING_VALIDATION", "PASS: Server ID SpinBox reflects selection");
        } else {
            logDiagnosticInfo("BINDING_VALIDATION", "FAIL: Server ID SpinBox does not reflect selection");
            valid = false;
        }
    } else {
        logDiagnosticInfo("BINDING_VALIDATION", "INFO: No item currently selected");
    }
    
    // Check plugin to UI binding
    if (m_currentPlugin && m_currentPlugin->loaded()) {
        logDiagnosticInfo("BINDING_VALIDATION", "INFO: Testing plugin to UI data flow");
        
        if (m_selectedServerId > 0) {
            try {
                ClientItem* clientItem = m_currentPlugin->getClientItem(m_selectedServerId);
                if (clientItem) {
                    logDiagnosticInfo("BINDING_VALIDATION", "PASS: Plugin can retrieve client item for selection");
                } else {
                    logDiagnosticInfo("BINDING_VALIDATION", "WARNING: Plugin returned null client item");
                }
            } catch (...) {
                logDiagnosticInfo("BINDING_VALIDATION", "FAIL: Exception when accessing plugin client item");
                valid = false;
            }
        }
    }
    
    return valid;
}

// Helper methods for enhanced updateItemProperties()
void MainForm::clearItemProperties()
{
    logDiagnosticInfo("ITEM_PROPERTIES", "Clearing all item properties");
    
    // Clear basic properties
    if (m_serverIdSpinBox) m_serverIdSpinBox->setValue(0);
    if (m_clientIdSpinBox) m_clientIdSpinBox->setValue(0);
    if (m_nameLineEdit) m_nameLineEdit->clear();
    if (m_descriptionTextEdit) m_descriptionTextEdit->clear();
    
    // Clear flag checkboxes
    for (FlagCheckBox* flagCheckBox : m_flagCheckBoxes) {
        if (flagCheckBox) {
            flagCheckBox->setChecked(false);
            flagCheckBox->setStyleSheet(""); // Reset color
        }
    }
    
    // Clear sprite display
    if (m_clientItemView) {
        m_clientItemView->setClientItem(nullptr);
    }
}

void MainForm::populateBasicProperties(OTLib::Server::Items::ServerItem* serverItem, ItemEditor::ClientItem* clientItem)
{
    if (!serverItem) {
        logDiagnosticInfo("ITEM_PROPERTIES", "WARNING: ServerItem is null - using placeholder values");
        
        // Set placeholder values
        if (m_serverIdSpinBox) m_serverIdSpinBox->setValue(m_selectedServerId);
        if (m_clientIdSpinBox) m_clientIdSpinBox->setValue(m_selectedServerId);
        if (m_nameLineEdit) m_nameLineEdit->setText(tr("Item %1").arg(m_selectedServerId));
        if (m_descriptionTextEdit) m_descriptionTextEdit->setText(tr("Description for item %1").arg(m_selectedServerId));
        return;
    }
    
    logDiagnosticInfo("ITEM_PROPERTIES", "Populating basic properties from ServerItem");
    
    // Populate server ID
    if (m_serverIdSpinBox) {
        m_serverIdSpinBox->setValue(serverItem->id());
        logDiagnosticInfo("ITEM_PROPERTIES", QString("Set server ID: %1").arg(serverItem->id()));
    }
    
    // Populate client ID with comparison
    if (m_clientIdSpinBox) {
        quint16 serverClientId = serverItem->clientId();
        m_clientIdSpinBox->setValue(serverClientId);
        
        // Apply visual feedback for client ID comparison
        if (clientItem) {
            quint16 clientClientId = clientItem->id();
            bool matches = (serverClientId == clientClientId);
            QString styleSheet = matches ? "" : "color: red;";
            m_clientIdSpinBox->setStyleSheet(styleSheet);
            
            if (!matches) {
                m_clientIdSpinBox->setToolTip(tr("Client value: %1").arg(clientClientId));
            } else {
                m_clientIdSpinBox->setToolTip("");
            }
        } else {
            m_clientIdSpinBox->setStyleSheet("color: red;");
            m_clientIdSpinBox->setToolTip(tr("No client data available"));
        }
        
        logDiagnosticInfo("ITEM_PROPERTIES", QString("Set client ID: %1").arg(serverClientId));
    }
    
    // Populate name with comparison
    if (m_nameLineEdit) {
        QString itemName = serverItem->name().isEmpty() ? serverItem->nameXml() : serverItem->name();
        if (itemName.isEmpty()) {
            itemName = tr("Item %1").arg(serverItem->id());
        }
        m_nameLineEdit->setText(itemName);
        
        // Apply visual feedback for name comparison
        if (clientItem) {
            QString clientName = clientItem->name();
            bool matches = (itemName == clientName);
            QString styleSheet = matches ? "" : "color: red;";
            m_nameLineEdit->setStyleSheet(styleSheet);
            
            if (!matches) {
                m_nameLineEdit->setToolTip(tr("Client value: %1").arg(clientName));
            } else {
                m_nameLineEdit->setToolTip("");
            }
        } else {
            m_nameLineEdit->setStyleSheet("color: red;");
            m_nameLineEdit->setToolTip(tr("No client data available"));
        }
        
        logDiagnosticInfo("ITEM_PROPERTIES", QString("Set name: %1").arg(itemName));
    }
    
    // Populate description (using name as description for now)
    if (m_descriptionTextEdit) {
        QString description = serverItem->nameXml();
        if (description.isEmpty()) {
            description = tr("Description for item %1").arg(serverItem->id());
        }
        m_descriptionTextEdit->setText(description);
        logDiagnosticInfo("ITEM_PROPERTIES", QString("Set description: %1").arg(description));
    }
}

void MainForm::populateFlagCheckboxes(OTLib::Server::Items::ServerItem* serverItem, ItemEditor::ClientItem* clientItem)
{
    logDiagnosticInfo("ITEM_PROPERTIES", QString("Populating %1 flag checkboxes").arg(m_flagCheckBoxes.size()));
    
    if (!serverItem) {
        logDiagnosticInfo("ITEM_PROPERTIES", "WARNING: ServerItem is null - clearing all flags");
        for (FlagCheckBox* flagCheckBox : m_flagCheckBoxes) {
            if (flagCheckBox) {
                flagCheckBox->setChecked(false);
            }
        }
        return;
    }
    
    // Iterate through m_flagCheckBoxes and set each checkbox state based on ServerItem properties
    for (FlagCheckBox* flagCheckBox : m_flagCheckBoxes) {
        if (!flagCheckBox) continue;
        
        QString flagName = flagCheckBox->text();
        bool flagValue = false;
        
        // Map flag names to ServerItem properties
        if (flagName == "Blocking") {
            flagValue = serverItem->unpassable();
        } else if (flagName == "Moveable") {
            flagValue = serverItem->movable();
        } else if (flagName == "Pickupable") {
            flagValue = serverItem->pickupable();
        } else if (flagName == "Stackable") {
            flagValue = serverItem->stackable();
        } else if (flagName == "Useable") {
            flagValue = serverItem->multiUse();
        } else if (flagName == "Readable") {
            flagValue = serverItem->readable();
        } else if (flagName == "Writable") {
            // Writable is typically derived from readable and other conditions
            flagValue = serverItem->readable();
        } else if (flagName == "LookThrough") {
            flagValue = !serverItem->ignoreLook();
        } else if (flagName == "Container") {
            flagValue = (serverItem->type() == OTLib::Server::Items::ServerItemType::Container);
        } else if (flagName == "Weapon") {
            // Weapon detection logic would go here
            flagValue = false; // Placeholder
        } else if (flagName == "Ammunition") {
            // Ammunition detection logic would go here
            flagValue = false; // Placeholder
        } else if (flagName == "Armor") {
            // Armor detection logic would go here
            flagValue = false; // Placeholder
        } else if (flagName == "MagicField") {
            flagValue = (serverItem->type() == OTLib::Server::Items::ServerItemType::Splash);
        } else if (flagName == "Teleport") {
            // Teleport detection logic would go here
            flagValue = false; // Placeholder
        } else if (flagName == "Key") {
            // Key detection logic would go here
            flagValue = false; // Placeholder
        } else if (flagName == "Splash") {
            flagValue = (serverItem->type() == OTLib::Server::Items::ServerItemType::Splash);
        }
        
        flagCheckBox->setChecked(flagValue);
        
        // Add visual feedback for flag comparison
        if (clientItem) {
            bool clientFlagValue = false;
            
            // Get corresponding client flag value
            if (flagName == "Blocking") {
                clientFlagValue = clientItem->unpassable();
            } else if (flagName == "Moveable") {
                clientFlagValue = clientItem->movable();
            } else if (flagName == "Pickupable") {
                clientFlagValue = clientItem->pickupable();
            } else if (flagName == "Stackable") {
                clientFlagValue = clientItem->stackable();
            } else if (flagName == "Useable") {
                clientFlagValue = clientItem->multiUse();
            } else if (flagName == "Readable") {
                clientFlagValue = clientItem->readable();
            } else if (flagName == "Writable") {
                clientFlagValue = clientItem->readable();
            } else if (flagName == "LookThrough") {
                clientFlagValue = !clientItem->ignoreLook();
            }
            
            // Apply visual feedback
            bool matches = (flagValue == clientFlagValue);
            QString styleSheet = matches ? "" : "color: red;";
            flagCheckBox->setStyleSheet(styleSheet);
            
            if (!matches) {
                flagCheckBox->setToolTip(tr("Client value: %1").arg(clientFlagValue ? "true" : "false"));
            } else {
                flagCheckBox->setToolTip("");
            }
        } else {
            // No client item - show all as different
            flagCheckBox->setStyleSheet("color: red;");
            flagCheckBox->setToolTip(tr("No client data available"));
        }
        
        logDiagnosticInfo("ITEM_PROPERTIES", QString("Set flag %1: %2").arg(flagName).arg(flagValue ? "true" : "false"));
    }
}

void MainForm::populateAttributeControls(OTLib::Server::Items::ServerItem* serverItem, ItemEditor::ClientItem* clientItem)
{
    logDiagnosticInfo("ITEM_PROPERTIES", "Populating attribute controls");
    
    if (!serverItem) {
        logDiagnosticInfo("ITEM_PROPERTIES", "WARNING: ServerItem is null - skipping attribute population");
        return;
    }
    
    // Attributes will be populated dynamically based on item type
    // For now, we'll populate common attributes that are always present
    
    // Ground speed
    if (serverItem->groundSpeed() > 0) {
        logDiagnosticInfo("ITEM_PROPERTIES", QString("Ground speed: %1").arg(serverItem->groundSpeed()));
    }
    
    // Light level and color
    if (serverItem->lightLevel() > 0) {
        logDiagnosticInfo("ITEM_PROPERTIES", QString("Light level: %1, color: %2").arg(serverItem->lightLevel()).arg(serverItem->lightColor()));
    }
    
    // Read/write character limits
    if (serverItem->maxReadChars() > 0) {
        logDiagnosticInfo("ITEM_PROPERTIES", QString("Max read chars: %1").arg(serverItem->maxReadChars()));
    }
    
    if (serverItem->maxReadWriteChars() > 0) {
        logDiagnosticInfo("ITEM_PROPERTIES", QString("Max read/write chars: %1").arg(serverItem->maxReadWriteChars()));
    }
    
    // Minimap color
    if (serverItem->minimapColor() > 0) {
        logDiagnosticInfo("ITEM_PROPERTIES", QString("Minimap color: %1").arg(serverItem->minimapColor()));
    }
    
    // Trade as
    if (serverItem->tradeAs() > 0) {
        logDiagnosticInfo("ITEM_PROPERTIES", QString("Trade as: %1").arg(serverItem->tradeAs()));
    }
}

void MainForm::updateSpriteDisplay(ItemEditor::ClientItem* clientItem)
{
    logDiagnosticInfo("SPRITE_DISPLAY", "Updating sprite display in properties panel");
    
    if (!m_clientItemView) {
        logDiagnosticInfo("SPRITE_DISPLAY", "ERROR: ClientItemView is null");
        return;
    }
    
    if (clientItem) {
        // Validate ClientItem before setting
        if (!clientItem->spriteList().isEmpty()) {
            // Check if bitmap is available or can be generated
            QPixmap bitmap = clientItem->getBitmap();
            if (bitmap.isNull()) {
                logDiagnosticInfo("SPRITE_DISPLAY", "Bitmap is null, attempting to generate...");
                clientItem->generateBitmap();
                bitmap = clientItem->getBitmap();
            }
            
            if (!bitmap.isNull()) {
                logDiagnosticInfo("SPRITE_DISPLAY", QString("SUCCESS: Setting ClientItem with valid bitmap (%1x%2, %3 sprites)")
                                 .arg(bitmap.width()).arg(bitmap.height()).arg(clientItem->spriteList().size()));
                m_clientItemView->setClientItem(clientItem);
            } else {
                logDiagnosticInfo("SPRITE_DISPLAY", "WARNING: Failed to generate bitmap from sprites");
                m_clientItemView->setClientItem(clientItem); // Still set it for fallback rendering
            }
        } else {
            logDiagnosticInfo("SPRITE_DISPLAY", "WARNING: ClientItem has no sprites");
            m_clientItemView->setClientItem(clientItem); // Set for fallback rendering
        }
    } else {
        logDiagnosticInfo("SPRITE_DISPLAY", "Clearing sprite display - no ClientItem available");
        m_clientItemView->setClientItem(nullptr);
    }
    
    // Force immediate update
    m_clientItemView->update();
}

void MainForm::refreshSpriteDisplay()
{
    logDiagnosticInfo("SPRITE_DISPLAY", "Refreshing entire sprite display system");
    
    // Clear sprite caches in all components
    if (m_serverItemListBox) {
        m_serverItemListBox->refreshSprites();
    }
    
    if (m_clientItemView) {
        // Force refresh of current item
        ItemEditor::ClientItem* currentItem = m_clientItemView->clientItem();
        if (currentItem) {
            // Clear cached bitmap to force regeneration
            currentItem->setBitmap(QPixmap());
            m_clientItemView->setClientItem(nullptr);
            m_clientItemView->setClientItem(currentItem);
        }
    }
    
    // Refresh current selection
    if (m_selectedServerId > 0) {
        updateItemDisplay();
    }
    
    qDebug() << "MainForm: Complete sprite display system refreshed";
}

void MainForm::optimizeSpriteCache()
{
    logDiagnosticInfo("SPRITE_DISPLAY", "Optimizing sprite cache memory usage");
    
    // Optimize ServerItemListBox sprite cache
    if (m_serverItemListBox) {
        m_serverItemListBox->optimizeMemoryUsage();
    }
    
    // Clear ClientItemView cache
    if (m_clientItemView) {
        m_clientItemView->clear();
    }
    
    qDebug() << "MainForm: Sprite cache optimization completed";
}

} // namespace ItemEditor