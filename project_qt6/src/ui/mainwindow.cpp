#include "mainwindow.h"
#include "ui/widgets/itempropertyeditor.h"
#include "ui/widgets/spritebrowser.h"
#include "ui/dialogs/enhancedfinditems.h"
#include <QDockWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QDebug>
#include <QCloseEvent>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isModified(false), currentSelectedItem(nullptr), loadingItemDetails(false),
      m_showOnlyMismatched(false), m_showOnlyDeprecated(false)
{
    setWindowTitle(tr("ItemEditor Qt6 - Advanced Edition"));
    setMinimumSize(1200, 800);
    
    // Initialize advanced components
    setupAdvancedUI();
    createAdvancedMenus();
    createAdvancedToolBars();
    createAdvancedDockWidgets();
    setupAdvancedConnections();
}

MainWindow::~MainWindow()
{
    // Qt handles cleanup automatically
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // TODO: Implement maybeSave() logic
    event->accept();
}

void MainWindow::setupAdvancedUI()
{
    // Create central splitter for main layout
    auto* centralSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(centralSplitter);
    
    // Left panel - Item list and basic controls
    auto* leftWidget = new QWidget();
    auto* leftLayout = new QVBoxLayout(leftWidget);
    
    // Item list (simplified version of original)
    serverItemListBox = new QListWidget(this);
    serverItemListBox->setMinimumWidth(250);
    leftLayout->addWidget(new QLabel("Server Items:", this));
    leftLayout->addWidget(serverItemListBox);
    
    // Basic item controls
    auto* itemControlsLayout = new QHBoxLayout();
    newItemButtonMain = new QPushButton("New", this);
    duplicateItemButtonMain = new QPushButton("Duplicate", this);
    reloadItemButtonMain = new QPushButton("Reload", this);
    findItemButtonMain = new QPushButton("Find", this);
    
    itemControlsLayout->addWidget(newItemButtonMain);
    itemControlsLayout->addWidget(duplicateItemButtonMain);
    itemControlsLayout->addWidget(reloadItemButtonMain);
    itemControlsLayout->addWidget(findItemButtonMain);
    leftLayout->addLayout(itemControlsLayout);
    
    centralSplitter->addWidget(leftWidget);
    
    // Center panel - Advanced property editor
    m_propertyEditor = new UI::Widgets::ItemPropertyEditor(this);
    centralSplitter->addWidget(m_propertyEditor);
    
    // Right panel - Sprite browser
    m_spriteBrowser = new UI::Widgets::SpriteBrowser(this);
    centralSplitter->addWidget(m_spriteBrowser);
    
    // Set splitter proportions
    centralSplitter->setStretchFactor(0, 1); // Item list
    centralSplitter->setStretchFactor(1, 2); // Property editor
    centralSplitter->setStretchFactor(2, 2); // Sprite browser
}

void MainWindow::createAdvancedMenus()
{
    // Enhanced File menu
    auto* fileMenu = menuBar()->addMenu("&File");
    
    auto* newAction = fileMenu->addAction("&New OTB", this, &MainWindow::newFile);
    newAction->setShortcut(QKeySequence::New);
    newAction->setIcon(QIcon(":/icons/NewIcon.png"));
    
    auto* openAction = fileMenu->addAction("&Open OTB...", this, &MainWindow::openFile);
    openAction->setShortcut(QKeySequence::Open);
    openAction->setIcon(QIcon(":/icons/OpenIcon.png"));
    
    fileMenu->addSeparator();
    
    auto* saveAction = fileMenu->addAction("&Save", this, &MainWindow::saveFile);
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setIcon(QIcon(":/icons/SaveIcon.png"));
    
    auto* saveAsAction = fileMenu->addAction("Save &As...", this, &MainWindow::saveFileAs);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    saveAsAction->setIcon(QIcon(":/icons/SaveAsIcon.png"));
    
    fileMenu->addSeparator();
    fileMenu->addAction("&Preferences", this, &MainWindow::showPreferences);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", this, &QWidget::close);
    
    // Enhanced Edit menu
    auto* editMenu = menuBar()->addMenu("&Edit");
    
    auto* createItemAction = editMenu->addAction("&Create Item", this, &MainWindow::createNewItem);
    createItemAction->setShortcut(QKeySequence("Ctrl+I"));
    
    auto* duplicateAction = editMenu->addAction("&Duplicate Item", this, &MainWindow::duplicateCurrentItem);
    duplicateAction->setShortcut(QKeySequence("Ctrl+D"));
    
    auto* reloadAction = editMenu->addAction("&Reload Item", this, &MainWindow::reloadCurrentItem);
    reloadAction->setShortcut(QKeySequence("Ctrl+R"));
    
    editMenu->addSeparator();
    
    auto* findAction = editMenu->addAction("&Find Items...", this, &MainWindow::showAdvancedFindDialog);
    findAction->setShortcut(QKeySequence::Find);
    findAction->setIcon(QIcon(":/icons/FindIcon.png"));
    
    editMenu->addSeparator();
    editMenu->addAction("Create &Missing Items", this, &MainWindow::createMissingItems);
    
    // Enhanced View menu
    auto* viewMenu = menuBar()->addMenu("&View");
    
    auto* showMismatchedAction = viewMenu->addAction("Show &Mismatched Items");
    showMismatchedAction->setCheckable(true);
    connect(showMismatchedAction, &QAction::toggled, this, &MainWindow::onShowMismatchedToggled);
    
    auto* showDeprecatedAction = viewMenu->addAction("Show &Deprecated Items");
    showDeprecatedAction->setCheckable(true);
    connect(showDeprecatedAction, &QAction::toggled, this, &MainWindow::onShowDeprecatedToggled);
    
    viewMenu->addSeparator();
    viewMenu->addAction("&Update Items List", this, &MainWindow::buildFilteredItemsList);
    
    // Enhanced Tools menu
    auto* toolsMenu = menuBar()->addMenu("&Tools");
    
    toolsMenu->addAction("&Reload All Attributes", this, &MainWindow::reloadAllItemAttributes);
    toolsMenu->addSeparator();
    toolsMenu->addAction("&Compare OTB Files...", this, &MainWindow::compareOtbFiles);
    toolsMenu->addAction("&Update OTB Version...", this, &MainWindow::updateOtbVersion);
    toolsMenu->addSeparator();
    toolsMenu->addAction("&Sprite Analysis...", this, &MainWindow::showSpriteAnalysis);
    toolsMenu->addAction("&Batch Operations...", this, &MainWindow::showBatchOperations);
    
    // Help menu
    auto* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&About ItemEditor Qt6", this, &MainWindow::about);
    helpMenu->addAction("About &Qt", qApp, &QApplication::aboutQt);
}

void MainWindow::createAdvancedToolBars()
{
    // Main toolbar with enhanced functionality
    auto* mainToolBar = addToolBar("Main");
    
    mainToolBar->addAction(QIcon(":/icons/NewIcon.png"), "New", this, &MainWindow::newFile);
    mainToolBar->addAction(QIcon(":/icons/OpenIcon.png"), "Open", this, &MainWindow::openFile);
    mainToolBar->addAction(QIcon(":/icons/SaveIcon.png"), "Save", this, &MainWindow::saveFile);
    mainToolBar->addAction(QIcon(":/icons/SaveAsIcon.png"), "Save As", this, &MainWindow::saveFileAs);
    
    mainToolBar->addSeparator();
    
    mainToolBar->addAction(QIcon(":/icons/FindIcon.png"), "Advanced Find", this, &MainWindow::showAdvancedFindDialog);
    mainToolBar->addAction(QIcon(":/icons/ReloadIcon.png"), "Reload", this, &MainWindow::reloadCurrentItem);
    mainToolBar->addAction(QIcon(":/icons/DuplicateIcon.png"), "Duplicate", this, &MainWindow::duplicateCurrentItem);
    
    mainToolBar->addSeparator();
    
    // Sprite toolbar
    auto* spriteToolBar = addToolBar("Sprites");
    spriteToolBar->addAction("Show Candidates", this, &MainWindow::showSpriteCandidates);
    spriteToolBar->addAction("Analyze Similarity", this, &MainWindow::analyzeSpriteSignatures);
    spriteToolBar->addAction("Sprite Browser", this, &MainWindow::toggleSpriteBrowser);
}

void MainWindow::createAdvancedDockWidgets()
{
    // Output log dock widget
    auto* outputDock = new QDockWidget("Output Log", this);
    outputLogView = new QTextEdit(this);
    outputLogView->setReadOnly(true);
    outputLogView->setMaximumHeight(200);
    outputDock->setWidget(outputLogView);
    addDockWidget(Qt::BottomDockWidgetArea, outputDock);
    
    // Item statistics dock widget
    auto* statsDock = new QDockWidget("Statistics", this);
    auto* statsWidget = new QWidget();
    auto* statsLayout = new QVBoxLayout(statsWidget);
    
    itemsCountLabel = new QLabel("0 Items", this);
    statsLayout->addWidget(itemsCountLabel);
    
    loadingProgressBar = new QProgressBar(this);
    loadingProgressBar->setVisible(false);
    statsLayout->addWidget(loadingProgressBar);
    
    statsLayout->addStretch();
    statsDock->setWidget(statsWidget);
    addDockWidget(Qt::BottomDockWidgetArea, statsDock);
}

void MainWindow::setupAdvancedConnections()
{
    // Connect property editor signals
    connect(m_propertyEditor, &UI::Widgets::ItemPropertyEditor::itemPropertyChanged,
            this, &MainWindow::onItemPropertyChanged);
    connect(m_propertyEditor, &UI::Widgets::ItemPropertyEditor::clientIdChanged,
            this, &MainWindow::onClientIdChanged);
    
    // Connect sprite browser signals
    connect(m_spriteBrowser, &UI::Widgets::SpriteBrowser::spriteSelected,
            this, &MainWindow::onSpriteSelected);
    connect(m_spriteBrowser, &UI::Widgets::SpriteBrowser::spriteAssignmentRequested,
            this, &MainWindow::onSpriteAssignmentRequested);
    
    // Connect item list signals
    connect(serverItemListBox, &QListWidget::currentItemChanged,
            this, &MainWindow::onServerItemSelectionChanged);
    
    // Connect item control buttons
    connect(newItemButtonMain, &QPushButton::clicked, this, &MainWindow::createNewItem);
    connect(duplicateItemButtonMain, &QPushButton::clicked, this, &MainWindow::duplicateCurrentItem);
    connect(reloadItemButtonMain, &QPushButton::clicked, this, &MainWindow::reloadCurrentItem);
    connect(findItemButtonMain, &QPushButton::clicked, this, &MainWindow::showAdvancedFindDialog);
}

// Advanced feature implementations
void MainWindow::showAdvancedFindDialog()
{
    auto* dialog = new UI::Dialogs::EnhancedFindItems(currentOtbItems, this);
    
    connect(dialog, &UI::Dialogs::EnhancedFindItems::itemSelected,
            this, &MainWindow::selectItemById);
    
    if (dialog->exec() == QDialog::Accepted) {
        auto* selectedItem = dialog->getFirstSelectedItem();
        if (selectedItem) {
            selectItemById(selectedItem->id);
        }
    }
    
    dialog->deleteLater();
}

void MainWindow::onItemPropertyChanged()
{
    isModified = true;
    updateWindowTitle();
}

void MainWindow::onSpriteSelected(quint32 spriteId)
{
    // Update sprite details or perform sprite-related operations
    outputLogView->append(QString("Sprite selected: %1").arg(spriteId));
}

void MainWindow::onSpriteAssignmentRequested(quint32 spriteId, ItemEditor::ClientItem* item)
{
    // Handle sprite assignment
    if (item) {
        // TODO: Implement sprite assignment logic
        outputLogView->append(QString("Assigning sprite %1 to item %2").arg(spriteId).arg(item->ID));
        QMessageBox::information(this, "Sprite Assignment",
                               QString("Sprite %1 assigned to item %2").arg(spriteId).arg(item->ID));
    }
}

void MainWindow::selectItemById(quint16 itemId)
{
    // Find and select item in the list
    for (int i = 0; i < serverItemListBox->count(); ++i) {
        auto* listItem = serverItemListBox->item(i);
        auto* serverItem = listItem->data(Qt::UserRole).value<OTB::ServerItem*>();
        if (serverItem && serverItem->id == itemId) {
            serverItemListBox->setCurrentItem(listItem);
            break;
        }
    }
}

void MainWindow::updateWindowTitle()
{
    QString title = "ItemEditor Qt6 - Advanced Edition";
    if (!currentFile.isEmpty()) {
        title += QString(" - %1").arg(QFileInfo(currentFile).fileName());
    }
    if (isModified) {
        title += " *";
    }
    setWindowTitle(title);
}

// Placeholder implementations for new menu actions
void MainWindow::showSpriteAnalysis()
{
    QMessageBox::information(this, "Sprite Analysis",
                           "Advanced sprite analysis tools will be implemented in the next phase.");
}

void MainWindow::showBatchOperations()
{
    QMessageBox::information(this, "Batch Operations",
                           "Batch editing operations will be implemented in the next phase.");
}

void MainWindow::analyzeSpriteSignatures()
{
    QMessageBox::information(this, "Sprite Signatures",
                           "FFT-based sprite signature analysis will be implemented in the next phase.");
}

void MainWindow::toggleSpriteBrowser()
{
    // Toggle sprite browser visibility
    bool isVisible = m_spriteBrowser->isVisible();
    m_spriteBrowser->setVisible(!isVisible);
}

// Existing method stubs that need to be implemented
void MainWindow::newFile() { /* TODO */ }
void MainWindow::openFile() { /* TODO */ }
bool MainWindow::saveFile() { return false; /* TODO */ }
bool MainWindow::saveFileAs() { return false; /* TODO */ }
void MainWindow::showPreferences() { /* TODO */ }
void MainWindow::createNewItem() { /* TODO */ }
void MainWindow::duplicateCurrentItem() { /* TODO */ }
void MainWindow::reloadCurrentItem() { /* TODO */ }
void MainWindow::createMissingItems() { /* TODO */ }
void MainWindow::onShowMismatchedToggled(bool checked) { /* TODO */ }
void MainWindow::onShowDeprecatedToggled(bool checked) { /* TODO */ }
void MainWindow::buildFilteredItemsList() { /* TODO */ }
void MainWindow::reloadAllItemAttributes() { /* TODO */ }
void MainWindow::compareOtbFiles() { /* TODO */ }
void MainWindow::updateOtbVersion() { /* TODO */ }
void MainWindow::about() { /* TODO */ }
void MainWindow::showSpriteCandidates() { /* TODO */ }
void MainWindow::onServerItemSelectionChanged(QListWidgetItem* current, QListWidgetItem* previous) { /* TODO */ }
void MainWindow::onClientIdChanged(int newId) { /* TODO */ }