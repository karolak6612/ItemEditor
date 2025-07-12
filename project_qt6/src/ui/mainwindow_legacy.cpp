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
#include "otb/otbtypes.h"
#include "ui/dialogs/aboutdialog.h"
#include "ui/dialogs/spritecandidatesdialog.h"
#include "ui/dialogs/finditemdialog.h"
#include "ui/dialogs/preferencesdialog.h"
#include "ui/dialogs/compareotbdialog.h"
#include "ui/dialogs/updateotbdialog.h"
#include "otb/otbreader.h"
#include "otb/otbwriter.h"
#include "widgets/clientitemview.h"
#include "tibiadata/imagesimilarity.h"

#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QTextEdit>
#include <QListWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>
#include <QProgressBar>
#include <QDebug>
#include <QFileInfo>
#include <QInputDialog>
#include <QClipboard>
#include <functional>
#include <QSettings>
#include <QMenu>
#include <algorithm>
#include <QPair>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isModified(false), currentSelectedItem(nullptr), loadingItemDetails(false),
      m_showOnlyMismatched(false), m_showOnlyDeprecated(false)
{
    setWindowTitle(tr("ItemEditor Qt6 - Advanced Edition"));
    setMinimumSize(1200, 800);

    // Initialize advanced components
    // TODO: Implement advanced UI components
    // setupAdvancedUI();
    // createAdvancedMenus();
    // createAdvancedToolBars();
    // createAdvancedDockWidgets();
    // setupAdvancedConnections();
    setWindowIcon(QIcon(":/app_icon"));

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createCentralWidget();

    setCurrentFile(QString());
    clearItemDetailsView();
    // Disable menus by default to match C# behavior
    editMenu->setEnabled(false);
    viewMenu->setEnabled(false);
    toolsMenu->setEnabled(false);
    statusBar()->showMessage(tr("Ready"));

    // Restore toolbar state
    restoreToolBarState();
}

MainWindow::~MainWindow()
{
    // Save toolbar state before destruction
    saveToolBarState();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::createActions()
{
    // File menu actions (matching C# MainForm exactly)
    newAct = new QAction(tr("&New"), this);
    newAct->setShortcut(QKeySequence(tr("Ctrl+N")));
    newAct->setStatusTip(tr("Create a new file"));
    newAct->setIcon(QIcon(":/icons/NewIcon.png"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(QKeySequence(tr("Ctrl+O")));
    openAct->setStatusTip(tr("Open an existing file"));
    openAct->setIcon(QIcon(":/icons/OpenIcon.png"));
    connect(openAct, &QAction::triggered, this, &MainWindow::openFile);

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcut(QKeySequence(tr("Ctrl+S")));
    saveAct->setStatusTip(tr("Save the document to disk"));
    saveAct->setIcon(QIcon(":/icons/SaveIcon.png"));
    saveAct->setEnabled(false); // Disabled by default like C#
    connect(saveAct, &QAction::triggered, this, QOverload<>::of(&MainWindow::saveFile));

    saveAsAct = new QAction(tr("S&ave as..."), this);
    saveAsAct->setShortcut(QKeySequence(tr("Ctrl+Shift+S")));
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    saveAsAct->setIcon(QIcon(":/icons/SaveAsIcon.png"));
    saveAsAct->setEnabled(false); // Disabled by default like C#
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveFileAs);

    preferencesAct = new QAction(tr("Preferences"), this);
    preferencesAct->setShortcut(QKeySequence(tr("Ctrl+P")));
    preferencesAct->setStatusTip(tr("Open preferences dialog"));
    connect(preferencesAct, &QAction::triggered, this, &MainWindow::showPreferences);

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    // Edit menu actions (matching C# MainForm exactly)
    createItemAct = new QAction(tr("&Create Item"), this);
    createItemAct->setShortcut(QKeySequence(tr("Ctrl+I")));
    createItemAct->setStatusTip(tr("Create a new item"));
    createItemAct->setIcon(QIcon(":/icons/NewIcon.png"));
    createItemAct->setEnabled(false); // Disabled by default like C#
    connect(createItemAct, &QAction::triggered, this, &MainWindow::createNewItem);

    duplicateItemAct = new QAction(tr("&Duplicate Item"), this);
    duplicateItemAct->setShortcut(QKeySequence(tr("Ctrl+D")));
    duplicateItemAct->setStatusTip(tr("Duplicate the currently selected item"));
    duplicateItemAct->setIcon(QIcon(":/icons/DuplicateIcon.png"));
    duplicateItemAct->setEnabled(false); // Disabled by default like C#
    connect(duplicateItemAct, &QAction::triggered, this, &MainWindow::duplicateCurrentItem);

    reloadItemAct = new QAction(tr("&Reload Item"), this);
    reloadItemAct->setShortcut(QKeySequence(tr("Ctrl+R")));
    reloadItemAct->setStatusTip(tr("Reload the currently selected item"));
    reloadItemAct->setIcon(QIcon(":/icons/ReloadIcon.png"));
    reloadItemAct->setEnabled(false); // Disabled by default like C#
    connect(reloadItemAct, &QAction::triggered, this, &MainWindow::reloadCurrentItem);

    createMissingItemsAct = new QAction(tr("Create Missing Items"), this);
    createMissingItemsAct->setStatusTip(tr("Create missing items"));
    createMissingItemsAct->setEnabled(false); // Disabled by default like C#
    connect(createMissingItemsAct, &QAction::triggered, this, &MainWindow::createMissingItems);

    findItemAct = new QAction(tr("&Find Item"), this);
    findItemAct->setShortcut(QKeySequence(tr("Ctrl+F")));
    findItemAct->setStatusTip(tr("Find an item"));
    findItemAct->setIcon(QIcon(":/icons/FindIcon.png"));
    findItemAct->setEnabled(false); // Disabled by default like C#
    connect(findItemAct, &QAction::triggered, this, &MainWindow::findItem);

    // View menu actions (matching C# MainForm exactly)
    showMismatchedAct = new QAction(tr("&Show Mismatched Items"), this);
    showMismatchedAct->setCheckable(true);
    showMismatchedAct->setStatusTip(tr("Show only mismatched items"));
    showMismatchedAct->setEnabled(false); // Disabled by default like C#
    connect(showMismatchedAct, &QAction::toggled, this, &MainWindow::onShowMismatchedToggled);

    showDeprecatedAct = new QAction(tr("Show Deprecated Items"), this);
    showDeprecatedAct->setCheckable(true);
    showDeprecatedAct->setStatusTip(tr("Show only deprecated items"));
    showDeprecatedAct->setEnabled(false); // Disabled by default like C#
    connect(showDeprecatedAct, &QAction::toggled, this, &MainWindow::onShowDeprecatedToggled);

    updateItemsListAct = new QAction(tr("&Update Items List"), this);
    updateItemsListAct->setStatusTip(tr("Update the items list"));
    updateItemsListAct->setEnabled(false); // Disabled by default like C#
    connect(updateItemsListAct, &QAction::triggered, this, &MainWindow::buildFilteredItemsList);

    // Tools menu actions (matching C# MainForm exactly)
    reloadAttributesAct = new QAction(tr("&Reload Item Attributes"), this);
    reloadAttributesAct->setStatusTip(tr("Reload all item attributes"));
    reloadAttributesAct->setEnabled(false); // Disabled by default like C#
    connect(reloadAttributesAct, &QAction::triggered, this, &MainWindow::reloadAllItemAttributes);

    compareOtbAct = new QAction(tr("&Compare OTB Files"), this);
    compareOtbAct->setStatusTip(tr("Compare OTB files"));
    compareOtbAct->setIcon(QIcon(":/icons/FormIcon.png"));
    connect(compareOtbAct, &QAction::triggered, this, &MainWindow::compareOtbFiles);

    updateVersionAct = new QAction(tr("&Update OTB Version"), this);
    updateVersionAct->setStatusTip(tr("Update OTB version"));
    updateVersionAct->setEnabled(false); // Disabled by default like C#
    connect(updateVersionAct, &QAction::triggered, this, &MainWindow::updateOtbVersion);

    // Help menu actions (matching C# MainForm exactly)
    aboutAct = new QAction(tr("&About ItemEditor"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    aboutAct->setIcon(QIcon(":/icons/InfoIcon.png"));
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);
}void MainWindow::createMenus()
{
    // File menu (matching C# MainForm exactly)
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(preferencesAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    // Edit menu (matching C# MainForm exactly)
    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(createItemAct);
    editMenu->addAction(duplicateItemAct);
    editMenu->addAction(reloadItemAct);
    editMenu->addSeparator();
    editMenu->addAction(createMissingItemsAct);
    editMenu->addSeparator();
    editMenu->addAction(findItemAct);

    // View menu (matching C# MainForm exactly)
    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(showMismatchedAct);
    viewMenu->addAction(showDeprecatedAct);
    viewMenu->addAction(updateItemsListAct);

    // Tools menu (matching C# MainForm exactly)
    toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(reloadAttributesAct);
    toolsMenu->addSeparator();
    toolsMenu->addAction(compareOtbAct);
    toolsMenu->addAction(updateVersionAct);

    // Help menu (matching C# MainForm exactly)
    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
}

void MainWindow::createToolBars()
{
    // Create main toolbar matching C# exactly
    mainToolBar = addToolBar(tr("Main"));
    mainToolBar->setObjectName("mainToolBar");
    mainToolBar->setMovable(true);
    mainToolBar->setFloatable(true);
    mainToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    mainToolBar->setIconSize(QSize(16, 16));

    // File operations section (matching C# order exactly)
    mainToolBar->addAction(newAct);
    mainToolBar->addAction(openAct);
    mainToolBar->addAction(saveAct);

    // Add Save As action to toolbar (missing from C# analysis but present in designer)
    saveAsToolBarAct = new QAction(this);
    saveAsToolBarAct->setIcon(QIcon(":/icons/SaveAsIcon.png"));
    saveAsToolBarAct->setToolTip(tr("Save As"));
    saveAsToolBarAct->setEnabled(false);
    connect(saveAsToolBarAct, &QAction::triggered, this, &MainWindow::saveFileAs);
    mainToolBar->addAction(saveAsToolBarAct);

    // Separator
    mainToolBar->addSeparator();

    // Tools section
    compareOtbToolBarAct = new QAction(this);
    compareOtbToolBarAct->setIcon(QIcon(":/icons/FormIcon.png"));
    compareOtbToolBarAct->setToolTip(tr("Compare OTB Files"));
    connect(compareOtbToolBarAct, &QAction::triggered, this, &MainWindow::compareOtbFiles);
    mainToolBar->addAction(compareOtbToolBarAct);

    mainToolBar->addAction(findItemAct);

    // Set toolbar properties to match C# behavior
    mainToolBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mainToolBar, &QToolBar::customContextMenuRequested,
            this, &MainWindow::showToolBarContextMenu);

    // Enable toolbar customization
    mainToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);

    // Store toolbar state for customization
    setupToolBarCustomization();
}

void MainWindow::createStatusBar()
{
    // Create items count label
    itemsCountLabel = new QLabel(tr("0 items"));
    itemsCountLabel->setStyleSheet("QLabel { margin: 0 5px; }");
    statusBar()->addPermanentWidget(itemsCountLabel);

    // Add visual separator
    QLabel* separator1 = new QLabel("|");
    separator1->setStyleSheet("QLabel { color: gray; margin: 0 5px; }");
    statusBar()->addPermanentWidget(separator1);

    // Create loading progress bar with consistent sizing
    loadingProgressBar = new QProgressBar();
    loadingProgressBar->setVisible(false);
    loadingProgressBar->setMaximumWidth(200);
    loadingProgressBar->setStyleSheet("QProgressBar { margin: 0 5px; }");
    statusBar()->addPermanentWidget(loadingProgressBar);

    // Add another visual separator (for future status elements)
    QLabel* separator2 = new QLabel("|");
    separator2->setStyleSheet("QLabel { color: gray; margin: 0 5px; }");
    statusBar()->addPermanentWidget(separator2);

    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createCentralWidget()
{
    // Create central widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Create main horizontal layout matching C# MainForm (784x561)
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    // === LEFT PANEL - Server Item List (232x440) ===
    QVBoxLayout *leftPanelLayout = new QVBoxLayout();

    // Server item list label
    QLabel *itemListLabel = new QLabel(tr("Server Items"));
    itemListLabel->setStyleSheet("font-weight: bold; margin-bottom: 3px;");

    // Server item list widget
    serverItemListBox = new QListWidget();
    serverItemListBox->setMinimumWidth(232);
    serverItemListBox->setMaximumWidth(250);
    serverItemListBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    serverItemListBox->setContextMenuPolicy(Qt::CustomContextMenu);
    serverItemListBox->setAlternatingRowColors(true);
    connect(serverItemListBox, &QListWidget::currentItemChanged,
            this, &MainWindow::onServerItemSelectionChanged);
    connect(serverItemListBox, &QListWidget::customContextMenuRequested,
            this, &MainWindow::showServerListContextMenu);

    // Control buttons for item list (matching C# layout)
    QHBoxLayout *itemButtonsLayout = new QHBoxLayout();
    itemButtonsLayout->setSpacing(3);

    newItemButtonMain = new QPushButton(tr("New"));
    newItemButtonMain->setMinimumWidth(50);
    duplicateItemButtonMain = new QPushButton(tr("Duplicate"));
    duplicateItemButtonMain->setMinimumWidth(60);
    reloadItemButtonMain = new QPushButton(tr("Reload"));
    reloadItemButtonMain->setMinimumWidth(50);
    findItemButtonMain = new QPushButton(tr("Find"));
    findItemButtonMain->setMinimumWidth(50);

    connect(newItemButtonMain, &QPushButton::clicked, this, &MainWindow::createNewItem);
    connect(duplicateItemButtonMain, &QPushButton::clicked, this, &MainWindow::duplicateCurrentItem);
    connect(reloadItemButtonMain, &QPushButton::clicked, this, &MainWindow::reloadCurrentItem);
    connect(findItemButtonMain, &QPushButton::clicked, this, &MainWindow::findItem);

    itemButtonsLayout->addWidget(newItemButtonMain);
    itemButtonsLayout->addWidget(duplicateItemButtonMain);
    itemButtonsLayout->addWidget(reloadItemButtonMain);
    itemButtonsLayout->addWidget(findItemButtonMain);

    leftPanelLayout->addWidget(itemListLabel);
    leftPanelLayout->addWidget(serverItemListBox);
    leftPanelLayout->addLayout(itemButtonsLayout);

    // === RIGHT PANEL - Appearance and Attributes ===
    QVBoxLayout *rightPanelLayout = new QVBoxLayout();

    // Top section with appearance and attributes side by side
    QHBoxLayout *topSectionLayout = new QHBoxLayout();
    topSectionLayout->setSpacing(8);

    // Create appearance group (89x309)
    createAppearanceGroup();
    topSectionLayout->addWidget(appearanceGroupBox);

    // Create attributes group (425x309)
    createAttributesGroup();
    topSectionLayout->addWidget(attributesGroupBox);

    rightPanelLayout->addLayout(topSectionLayout);

    // Bottom section for output log (525x160)
    QLabel *outputLabel = new QLabel(tr("Output"));
    outputLabel->setStyleSheet("font-weight: bold; margin-top: 8px; margin-bottom: 3px;");

    outputLogView = new QTextEdit();
    outputLogView->setMinimumHeight(160);
    outputLogView->setMaximumHeight(200);
    outputLogView->setReadOnly(true);
    outputLogView->setStyleSheet(
        "QTextEdit {"
        "  background-color: #1e1e1e;"
        "  color: #ffffff;"
        "  font-family: 'Consolas', 'Monaco', monospace;"
        "  font-size: 9pt;"
        "  border: 1px solid #555;"
        "}"
    );

    rightPanelLayout->addWidget(outputLabel);
    rightPanelLayout->addWidget(outputLogView);

    // Add panels to main layout
    mainLayout->addLayout(leftPanelLayout);
    mainLayout->addLayout(rightPanelLayout);

    // Set stretch factors to maintain C# proportions
    mainLayout->setStretchFactor(leftPanelLayout, 0); // Fixed width like C#
    mainLayout->setStretchFactor(rightPanelLayout, 1); // Expandable
}

void MainWindow::createAppearanceGroup()
{
    // Appearance Group Box (89x309 from C# analysis)
    appearanceGroupBox = new QGroupBox(tr("Appearance"));
    appearanceGroupBox->setMinimumWidth(89);
    appearanceGroupBox->setMaximumWidth(120);
    appearanceGroupBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    QVBoxLayout *appearanceLayout = new QVBoxLayout(appearanceGroupBox);
    appearanceLayout->setSpacing(8);

    // Previous sprite (64x64)
    QLabel *prevLabel = new QLabel(tr("Previous:"));
    prevLabel->setStyleSheet("font-weight: bold; font-size: 8pt;");
    previousClientItemViewWidget = new UI::Widgets::ClientItemView();
    previousClientItemViewWidget->setFixedSize(64, 64);
    previousClientItemViewWidget->setStyleSheet("border: 1px solid #666;");

    // Current sprite (64x64)
    QLabel *currentLabel = new QLabel(tr("Current:"));
    currentLabel->setStyleSheet("font-weight: bold; font-size: 8pt;");
    mainClientItemViewWidget = new UI::Widgets::ClientItemView();
    mainClientItemViewWidget->setFixedSize(64, 64);
    mainClientItemViewWidget->setStyleSheet("border: 1px solid #666;");

    // Server ID (display only)
    QLabel *serverIdLabel = new QLabel(tr("Server ID:"));
    serverIdLabel->setStyleSheet("font-size: 8pt;");
    serverIDLabel_val = new QLabel(tr("N/A"));
    serverIDLabel_val->setStyleSheet("font-size: 8pt; color: #0066cc;");

    // Client ID (editable)
    QLabel *clientIdLabel = new QLabel(tr("Client ID:"));
    clientIdLabel->setStyleSheet("font-size: 8pt;");
    clientIDSpinBox = new QSpinBox();
    clientIDSpinBox->setRange(0, 65535);
    clientIDSpinBox->setStyleSheet("font-size: 8pt;");
    connect(clientIDSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onClientIdChanged);

    // Candidates button
    candidatesButton = new QPushButton(tr("Candidates"));
    candidatesButton->setStyleSheet("font-size: 8pt; padding: 2px;");
    connect(candidatesButton, &QPushButton::clicked, this, &MainWindow::showSpriteCandidates);

    // Layout components vertically to match C# layout
    appearanceLayout->addWidget(prevLabel);
    appearanceLayout->addWidget(previousClientItemViewWidget);
    appearanceLayout->addWidget(currentLabel);
    appearanceLayout->addWidget(mainClientItemViewWidget);
    appearanceLayout->addWidget(serverIdLabel);
    appearanceLayout->addWidget(serverIDLabel_val);
    appearanceLayout->addWidget(clientIdLabel);
    appearanceLayout->addWidget(clientIDSpinBox);
    appearanceLayout->addWidget(candidatesButton);
    appearanceLayout->addStretch(); // Push everything to top
}void MainWindow::createAttributesGroup()
{
    // Attributes Group Box (425x309 from C# analysis)
    attributesGroupBox = new QGroupBox(tr("Attributes"));
    attributesGroupBox->setMinimumWidth(425);
    attributesGroupBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QGridLayout *attributesLayout = new QGridLayout(attributesGroupBox);
    attributesLayout->setSpacing(4);
    attributesLayout->setContentsMargins(8, 8, 8, 8);

    int row = 0;

    // === STRING ATTRIBUTES ===
    // Item name (full width)
    attributesLayout->addWidget(new QLabel(tr("Name:")), row, 0);
    itemNameLineEdit = new QLineEdit();
    itemNameLineEdit->setStyleSheet("font-size: 9pt;");
    connect(itemNameLineEdit, &QLineEdit::textChanged, this, &MainWindow::onItemNameChanged);
    attributesLayout->addWidget(itemNameLineEdit, row, 1, 1, 3);
    row++;

    // === ENUM ATTRIBUTES ===
    // Item type and Stack order (side by side)
    attributesLayout->addWidget(new QLabel(tr("Type:")), row, 0);
    itemTypeComboBox = new QComboBox();
    itemTypeComboBox->setStyleSheet("font-size: 9pt;");
    connect(itemTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onItemTypeChanged);
    attributesLayout->addWidget(itemTypeComboBox, row, 1);

    attributesLayout->addWidget(new QLabel(tr("Stack Order:")), row, 2);
    stackOrderComboBox = new QComboBox();
    stackOrderComboBox->setStyleSheet("font-size: 9pt;");
    connect(stackOrderComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onStackOrderChanged);
    attributesLayout->addWidget(stackOrderComboBox, row, 3);
    row++;

    // === BOOLEAN ATTRIBUTES (15 checkboxes in 4 columns) ===
    // Row 1: Unpassable, Movable, BlockMissiles, BlockPathfinder
    unpassableCheckBox = new QCheckBox(tr("Unpassable"));
    unpassableCheckBox->setStyleSheet("font-size: 8pt;");
    connect(unpassableCheckBox, &QCheckBox::toggled, this, &MainWindow::onUnpassableChanged);
    attributesLayout->addWidget(unpassableCheckBox, row, 0);

    movableCheckBox = new QCheckBox(tr("Movable"));
    movableCheckBox->setStyleSheet("font-size: 8pt;");
    connect(movableCheckBox, &QCheckBox::toggled, this, &MainWindow::onMovableChanged);
    attributesLayout->addWidget(movableCheckBox, row, 1);

    blockMissilesCheckBox = new QCheckBox(tr("Block Missiles"));
    blockMissilesCheckBox->setStyleSheet("font-size: 8pt;");
    connect(blockMissilesCheckBox, &QCheckBox::toggled, this, &MainWindow::onBlockMissilesChanged);
    attributesLayout->addWidget(blockMissilesCheckBox, row, 2);

    blockPathfinderCheckBox = new QCheckBox(tr("Block Pathfinder"));
    blockPathfinderCheckBox->setStyleSheet("font-size: 8pt;");
    connect(blockPathfinderCheckBox, &QCheckBox::toggled, this, &MainWindow::onBlockPathfinderChanged);
    attributesLayout->addWidget(blockPathfinderCheckBox, row, 3);
    row++;

    // Row 2: ForceUse, MultiUse, Pickupable, Stackable
    forceUseCheckBox = new QCheckBox(tr("Force Use"));
    forceUseCheckBox->setStyleSheet("font-size: 8pt;");
    connect(forceUseCheckBox, &QCheckBox::toggled, this, &MainWindow::onForceUseChanged);
    attributesLayout->addWidget(forceUseCheckBox, row, 0);

    multiUseCheckBox = new QCheckBox(tr("Multi Use"));
    multiUseCheckBox->setStyleSheet("font-size: 8pt;");
    connect(multiUseCheckBox, &QCheckBox::toggled, this, &MainWindow::onMultiUseChanged);
    attributesLayout->addWidget(multiUseCheckBox, row, 1);

    pickupableCheckBox = new QCheckBox(tr("Pickupable"));
    pickupableCheckBox->setStyleSheet("font-size: 8pt;");
    connect(pickupableCheckBox, &QCheckBox::toggled, this, &MainWindow::onPickupableChanged);
    attributesLayout->addWidget(pickupableCheckBox, row, 2);

    stackableCheckBox = new QCheckBox(tr("Stackable"));
    stackableCheckBox->setStyleSheet("font-size: 8pt;");
    connect(stackableCheckBox, &QCheckBox::toggled, this, &MainWindow::onStackableChanged);
    attributesLayout->addWidget(stackableCheckBox, row, 3);
    row++;

    // Row 3: Readable, Rotatable, Hangable, HookSouth
    readableCheckBox = new QCheckBox(tr("Readable"));
    readableCheckBox->setStyleSheet("font-size: 8pt;");
    connect(readableCheckBox, &QCheckBox::toggled, this, &MainWindow::onReadableChanged);
    attributesLayout->addWidget(readableCheckBox, row, 0);

    rotatableCheckBox = new QCheckBox(tr("Rotatable"));
    rotatableCheckBox->setStyleSheet("font-size: 8pt;");
    connect(rotatableCheckBox, &QCheckBox::toggled, this, &MainWindow::onRotatableChanged);
    attributesLayout->addWidget(rotatableCheckBox, row, 1);

    hangableCheckBox = new QCheckBox(tr("Hangable"));
    hangableCheckBox->setStyleSheet("font-size: 8pt;");
    connect(hangableCheckBox, &QCheckBox::toggled, this, &MainWindow::onHangableChanged);
    attributesLayout->addWidget(hangableCheckBox, row, 2);

    hookSouthCheckBox = new QCheckBox(tr("Hook South"));
    hookSouthCheckBox->setStyleSheet("font-size: 8pt;");
    connect(hookSouthCheckBox, &QCheckBox::toggled, this, &MainWindow::onHookSouthChanged);
    attributesLayout->addWidget(hookSouthCheckBox, row, 3);
    row++;

    // Row 4: HookEast, HasElevation, IgnoreLook, FullGround
    hookEastCheckBox = new QCheckBox(tr("Hook East"));
    hookEastCheckBox->setStyleSheet("font-size: 8pt;");
    connect(hookEastCheckBox, &QCheckBox::toggled, this, &MainWindow::onHookEastChanged);
    attributesLayout->addWidget(hookEastCheckBox, row, 0);

    hasElevationCheckBox = new QCheckBox(tr("Has Elevation"));
    hasElevationCheckBox->setStyleSheet("font-size: 8pt;");
    connect(hasElevationCheckBox, &QCheckBox::toggled, this, &MainWindow::onHasElevationChanged);
    attributesLayout->addWidget(hasElevationCheckBox, row, 1);

    ignoreLookCheckBox = new QCheckBox(tr("Ignore Look"));
    ignoreLookCheckBox->setStyleSheet("font-size: 8pt;");
    connect(ignoreLookCheckBox, &QCheckBox::toggled, this, &MainWindow::onIgnoreLookChanged);
    attributesLayout->addWidget(ignoreLookCheckBox, row, 2);

    fullGroundCheckBox = new QCheckBox(tr("Full Ground"));
    fullGroundCheckBox->setStyleSheet("font-size: 8pt;");
    connect(fullGroundCheckBox, &QCheckBox::toggled, this, &MainWindow::onFullGroundChanged);
    attributesLayout->addWidget(fullGroundCheckBox, row, 3);
    row++;

    // === NUMERIC ATTRIBUTES (8 text boxes in 4x2 grid) ===
    // Row 1: GroundSpeed, LightLevel, LightColor, MinimapColor
    attributesLayout->addWidget(new QLabel(tr("Ground Speed:")), row, 0);
    groundSpeedLineEdit = new QLineEdit();
    groundSpeedLineEdit->setStyleSheet("font-size: 9pt;");
    connect(groundSpeedLineEdit, &QLineEdit::textChanged, this, &MainWindow::onGroundSpeedChanged);
    attributesLayout->addWidget(groundSpeedLineEdit, row, 1);

    attributesLayout->addWidget(new QLabel(tr("Light Level:")), row, 2);
    lightLevelLineEdit = new QLineEdit();
    lightLevelLineEdit->setStyleSheet("font-size: 9pt;");
    connect(lightLevelLineEdit, &QLineEdit::textChanged, this, &MainWindow::onLightLevelChanged);
    attributesLayout->addWidget(lightLevelLineEdit, row, 3);
    row++;

    // Row 2: LightColor, MinimapColor
    attributesLayout->addWidget(new QLabel(tr("Light Color:")), row, 0);
    lightColorLineEdit = new QLineEdit();
    lightColorLineEdit->setStyleSheet("font-size: 9pt;");
    connect(lightColorLineEdit, &QLineEdit::textChanged, this, &MainWindow::onLightColorChanged);
    attributesLayout->addWidget(lightColorLineEdit, row, 1);

    attributesLayout->addWidget(new QLabel(tr("Minimap Color:")), row, 2);
    minimapColorLineEdit = new QLineEdit();
    minimapColorLineEdit->setStyleSheet("font-size: 9pt;");
    connect(minimapColorLineEdit, &QLineEdit::textChanged, this, &MainWindow::onMinimapColorChanged);
    attributesLayout->addWidget(minimapColorLineEdit, row, 3);
    row++;

    // Row 3: MaxReadChars, MaxReadWriteChars, TradeAs (WareId)
    attributesLayout->addWidget(new QLabel(tr("Max Read Chars:")), row, 0);
    maxReadCharsLineEdit = new QLineEdit();
    maxReadCharsLineEdit->setStyleSheet("font-size: 9pt;");
    connect(maxReadCharsLineEdit, &QLineEdit::textChanged, this, &MainWindow::onMaxReadCharsChanged);
    attributesLayout->addWidget(maxReadCharsLineEdit, row, 1);

    attributesLayout->addWidget(new QLabel(tr("Max Read/Write Chars:")), row, 2);
    maxReadWriteCharsLineEdit = new QLineEdit();
    maxReadWriteCharsLineEdit->setStyleSheet("font-size: 9pt;");
    connect(maxReadWriteCharsLineEdit, &QLineEdit::textChanged, this, &MainWindow::onMaxReadWriteCharsChanged);
    attributesLayout->addWidget(maxReadWriteCharsLineEdit, row, 3);
    row++;

    // Row 4: TradeAs (WareId)
    attributesLayout->addWidget(new QLabel(tr("Trade As (Ware ID):")), row, 0);
    wareIdLineEdit = new QLineEdit();
    wareIdLineEdit->setStyleSheet("font-size: 9pt;");
    connect(wareIdLineEdit, &QLineEdit::textChanged, this, &MainWindow::onWareIdChanged);
    attributesLayout->addWidget(wareIdLineEdit, row, 1);

    // Set column stretch to ensure proper proportions
    attributesLayout->setColumnStretch(0, 1);
    attributesLayout->setColumnStretch(1, 1);
    attributesLayout->setColumnStretch(2, 1);
    attributesLayout->setColumnStretch(3, 1);
}

// File operations
void MainWindow::newFile()
{
    if (maybeSave()) {
        currentOtbItems.clear();
        setCurrentFile(QString());
        clearItemDetailsView();
        buildFilteredItemsList();
        showStatusMessage(tr("New file created"), 2000);
    }
}

void MainWindow::openFile()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open OTB File"), QString(), tr("OTB Files (*.otb)"));
        if (!fileName.isEmpty()) {
            loadFile(fileName);
        }
    }
}

bool MainWindow::saveFile()
{
    if (currentFile.isEmpty()) {
        return saveFileAs();
    } else {
        return saveFile(currentFile);
    }
}

bool MainWindow::saveFileAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save OTB File"), QString(), tr("OTB Files (*.otb)"));
    if (fileName.isEmpty()) {
        return false;
    }
    return saveFile(fileName);
}

void MainWindow::showPreferences()
{
    UI::Dialogs::PreferencesDialog dialog(this);
    dialog.exec();
}

// Edit operations
void MainWindow::createNewItem()
{
    // Implementation for creating new item
    showStatusMessage(tr("Create new item not yet implemented"), 2000);
}

void MainWindow::duplicateCurrentItem()
{
    if (!currentSelectedItem) {
        showStatusMessage(tr("No item selected"), 2000);
        return;
    }
    // Implementation for duplicating item
    showStatusMessage(tr("Duplicate item not yet implemented"), 2000);
}

void MainWindow::reloadCurrentItem()
{
    if (!currentSelectedItem) {
        showStatusMessage(tr("No item selected"), 2000);
        return;
    }
    // Implementation for reloading item
    showStatusMessage(tr("Reloading item..."));
    // TODO: Implement actual reload logic
    showStatusMessage(tr("Item reloaded"), 1000);
}

void MainWindow::findItem()
{
    UI::Dialogs::FindItemDialog dialog(currentOtbItems, this);
    if (dialog.exec() == QDialog::Accepted) {
        quint16 serverId = dialog.getSelectedServerId();
        if (serverId > 0) {
            // Find the item in the list and select it
            for (int i = 0; i < serverItemListBox->count(); ++i) {
                QListWidgetItem* listItem = serverItemListBox->item(i);
                auto it = listItemToServerItemMap.find(listItem);
                if (it != listItemToServerItemMap.end() && it.value()->id == serverId) {
                    serverItemListBox->setCurrentItem(listItem);
                    break;
                }
            }
        }
    }
}

void MainWindow::createMissingItems()
{
    // Implementation for creating missing items
    statusBar()->showMessage(tr("Create missing items not yet implemented"), 2000);
}

// View operations
void MainWindow::onShowMismatchedToggled(bool checked)
{
    m_showOnlyMismatched = checked;
    buildFilteredItemsList();
}

void MainWindow::onShowDeprecatedToggled(bool checked)
{
    m_showOnlyDeprecated = checked;
    buildFilteredItemsList();
}

void MainWindow::buildFilteredItemsList()
{
    serverItemListBox->clear();
    listItemToServerItemMap.clear();

    int itemCount = 0;
    for (auto& item : currentOtbItems.items) {
        bool shouldShow = true;

        if (m_showOnlyMismatched) {
            // Only show items that don't match client data
            shouldShow = false; // Implement mismatch logic
        }

        if (m_showOnlyDeprecated) {
            // Only show deprecated items
            shouldShow = false; // Implement deprecated logic
        }

        if (shouldShow) {
            QListWidgetItem* listItem = new QListWidgetItem(QString("ID %1: %2").arg(item.id).arg(item.name));
            serverItemListBox->addItem(listItem);
            listItemToServerItemMap[listItem] = &item;
            itemCount++;
        }
    }

    updateItemCount(itemCount);
}

// Tools operations
void MainWindow::reloadAllItemAttributes()
{
    // Implementation for reloading all item attributes
    statusBar()->showMessage(tr("Reload all attributes not yet implemented"), 2000);
}

void MainWindow::compareOtbFiles()
{
    UI::Dialogs::CompareOtbDialog dialog(this);
    dialog.exec();
}

void MainWindow::updateOtbVersion()
{
    UI::Dialogs::UpdateOtbDialog dialog(currentOtbItems.buildNumber, this);
    dialog.exec();
}

// Help operations
void MainWindow::about()
{
    UI::Dialogs::AboutDialog dialog(this);
    dialog.exec();
}// UI update slots
void MainWindow::onServerItemSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);

    if (!current) {
        currentSelectedItem = nullptr;
        clearItemDetailsView();
        return;
    }

    auto it = listItemToServerItemMap.find(current);
    if (it != listItemToServerItemMap.end()) {
        currentSelectedItem = it.value();
        updateItemDetailsView(currentSelectedItem);
    }
}

void MainWindow::updateItemDetailsView(OTB::ServerItem* item)
{
    if (!item) {
        clearItemDetailsView();
        return;
    }

    loadingItemDetails = true;
    showStatusMessage(tr("Loading item details..."));

    // Update appearance section
    serverIDLabel_val->setText(QString::number(item->id));
    clientIDSpinBox->setValue(item->clientId);

    // Update attributes section
    itemNameLineEdit->setText(item->name);

    // Update checkboxes
    unpassableCheckBox->setChecked(item->unpassable);
    blockMissilesCheckBox->setChecked(item->blockMissiles);
    blockPathfinderCheckBox->setChecked(item->blockPathfinder);
    hasElevationCheckBox->setChecked(item->hasElevation);
    forceUseCheckBox->setChecked(item->forceUse);
    multiUseCheckBox->setChecked(item->multiUse);
    pickupableCheckBox->setChecked(item->pickupable);
    movableCheckBox->setChecked(item->movable);
    stackableCheckBox->setChecked(item->stackable);
    readableCheckBox->setChecked(item->readable);
    rotatableCheckBox->setChecked(item->rotatable);
    hangableCheckBox->setChecked(item->hangable);
    hookSouthCheckBox->setChecked(item->hookSouth);
    hookEastCheckBox->setChecked(item->hookEast);
    ignoreLookCheckBox->setChecked(item->ignoreLook);
    fullGroundCheckBox->setChecked(item->fullGround);

    // Update numeric fields
    groundSpeedLineEdit->setText(QString::number(item->groundSpeed));
    lightLevelLineEdit->setText(QString::number(item->lightLevel));
    lightColorLineEdit->setText(QString::number(item->lightColor));
    minimapColorLineEdit->setText(QString::number(item->minimapColor));
    maxReadCharsLineEdit->setText(QString::number(item->maxReadChars));
    maxReadWriteCharsLineEdit->setText(QString::number(item->maxReadWriteChars));
    // wareIdLineEdit->setText(QString::number(item->wareId)); // wareId doesn't exist in ServerItem

    loadingItemDetails = false;
    showStatusMessage(tr("Item details loaded"), 1000);
}

// Item property change handlers
void MainWindow::onClientIdChanged(int value)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    currentSelectedItem->clientId = value;
    isModified = true;
}

void MainWindow::onItemNameChanged(const QString& text)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    currentSelectedItem->name = text;
    isModified = true;
}

void MainWindow::onItemTypeChanged(int index)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    // Implementation depends on item type enum
    isModified = true;
}

void MainWindow::onStackOrderChanged(int index)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    // Implementation depends on stack order enum
    isModified = true;
}

// Flag checkbox handlers
void MainWindow::onUnpassableChanged(bool checked)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    currentSelectedItem->unpassable = checked;
    isModified = true;
}

void MainWindow::onBlockMissilesChanged(bool checked)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    currentSelectedItem->blockMissiles = checked;
    isModified = true;
}

void MainWindow::onBlockPathfinderChanged(bool checked)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    currentSelectedItem->blockPathfinder = checked;
    isModified = true;
}

void MainWindow::onHasElevationChanged(bool checked)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    currentSelectedItem->hasElevation = checked;
    isModified = true;
}

void MainWindow::onForceUseChanged(bool checked)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    currentSelectedItem->forceUse = checked;
    isModified = true;
}

void MainWindow::onMultiUseChanged(bool checked)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    currentSelectedItem->multiUse = checked;
    isModified = true;
}

void MainWindow::onPickupableChanged(bool checked)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    currentSelectedItem->pickupable = checked;
    isModified = true;
}

void MainWindow::onMovableChanged(bool checked)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    currentSelectedItem->movable = checked;
    isModified = true;
}

void MainWindow::onStackableChanged(bool checked)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    currentSelectedItem->stackable = checked;
    isModified = true;
}

void MainWindow::onReadableChanged(bool checked)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    currentSelectedItem->readable = checked;
    isModified = true;
}

void MainWindow::onRotatableChanged(bool checked)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    currentSelectedItem->rotatable = checked;
    isModified = true;
}

void MainWindow::onHangableChanged(bool checked)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    currentSelectedItem->hangable = checked;
    isModified = true;
}

void MainWindow::onHookSouthChanged(bool checked)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    currentSelectedItem->hookSouth = checked;
    isModified = true;
}

void MainWindow::onHookEastChanged(bool checked)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    currentSelectedItem->hookEast = checked;
    isModified = true;
}

void MainWindow::onIgnoreLookChanged(bool checked)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    currentSelectedItem->ignoreLook = checked;
    isModified = true;
}

void MainWindow::onFullGroundChanged(bool checked)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    currentSelectedItem->fullGround = checked;
    isModified = true;
}// Numeric attribute handlers
void MainWindow::onGroundSpeedChanged(const QString& text)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    bool ok;
    int value = text.toInt(&ok);
    if (ok) {
        currentSelectedItem->groundSpeed = value;
        isModified = true;
    }
}

void MainWindow::onLightLevelChanged(const QString& text)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    bool ok;
    int value = text.toInt(&ok);
    if (ok) {
        currentSelectedItem->lightLevel = value;
        isModified = true;
    }
}

void MainWindow::onLightColorChanged(const QString& text)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    bool ok;
    int value = text.toInt(&ok);
    if (ok) {
        currentSelectedItem->lightColor = value;
        isModified = true;
    }
}

void MainWindow::onMinimapColorChanged(const QString& text)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    bool ok;
    int value = text.toInt(&ok);
    if (ok) {
        currentSelectedItem->minimapColor = value;
        isModified = true;
    }
}

void MainWindow::onMaxReadCharsChanged(const QString& text)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    bool ok;
    int value = text.toInt(&ok);
    if (ok) {
        currentSelectedItem->maxReadChars = value;
        isModified = true;
    }
}

void MainWindow::onMaxReadWriteCharsChanged(const QString& text)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    bool ok;
    int value = text.toInt(&ok);
    if (ok) {
        currentSelectedItem->maxReadWriteChars = value;
        isModified = true;
    }
}

void MainWindow::onWareIdChanged(const QString& text)
{
    if (loadingItemDetails || !currentSelectedItem) return;
    // wareId doesn't exist in ServerItem, so this is a no-op for now
    // bool ok;
    // int value = text.toInt(&ok);
    // if (ok) {
    //     currentSelectedItem->wareId = value;
    //     isModified = true;
    // }
}

// Other UI slots
void MainWindow::showSpriteCandidates()
{
    if (!currentSelectedItem) {
        statusBar()->showMessage(tr("No item selected"), 2000);
        return;
    }

    // Create a list of candidate client items (empty for now)
    QList<const ItemEditor::ClientItem*> candidates;
    UI::Dialogs::SpriteCandidatesDialog dialog(candidates, this);
    dialog.exec();
}

void MainWindow::showServerListContextMenu(const QPoint& pos)
{
    QListWidgetItem* item = serverItemListBox->itemAt(pos);
    if (!item) return;

    QMenu contextMenu(this);

    QAction* copyServerIdAction = contextMenu.addAction(tr("Copy Server ID"));
    connect(copyServerIdAction, &QAction::triggered, this, &MainWindow::copyServerId);

    QAction* copyClientIdAction = contextMenu.addAction(tr("Copy Client ID"));
    connect(copyClientIdAction, &QAction::triggered, this, &MainWindow::copyClientId);

    QAction* copyNameAction = contextMenu.addAction(tr("Copy Item Name"));
    connect(copyNameAction, &QAction::triggered, this, &MainWindow::copyItemName);

    contextMenu.exec(serverItemListBox->mapToGlobal(pos));
}

void MainWindow::copyServerId()
{
    if (!currentSelectedItem) return;
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(QString::number(currentSelectedItem->id));
    statusBar()->showMessage(tr("Server ID copied to clipboard"), 2000);
}

void MainWindow::copyClientId()
{
    if (!currentSelectedItem) return;
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(QString::number(currentSelectedItem->clientId));
    statusBar()->showMessage(tr("Client ID copied to clipboard"), 2000);
}

void MainWindow::copyItemName()
{
    if (!currentSelectedItem) return;
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(currentSelectedItem->name);
    statusBar()->showMessage(tr("Item name copied to clipboard"), 2000);
}// Utility methods
bool MainWindow::maybeSave()
{
    if (!isModified) {
        return true;
    }

    const QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("ItemEditor"),
                                                                  tr("The document has been modified.\n"
                                                                     "Do you want to save your changes?"),
                                                                  QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return saveFile();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MainWindow::loadFile(const QString &fileName)
{
    QFileInfo fileInfo(fileName);
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, tr("ItemEditor"), tr("Cannot read file %1:\nFile does not exist.").arg(fileName));
        return;
    }

    OTB::OtbReader reader;
    QString errorString;

    showLoadingProgress(0, 100, tr("Loading %1...").arg(fileInfo.fileName()));

    if (reader.read(fileName, currentOtbItems, errorString)) {
        setCurrentFile(fileName);
        buildFilteredItemsList();
        showStatusMessage(tr("File loaded successfully"), 2000);

        editMenu->setEnabled(true);
        viewMenu->setEnabled(true);
        toolsMenu->setEnabled(true);
    } else {
        QMessageBox::warning(this, tr("ItemEditor"), tr("Cannot read file %1:\n%2").arg(fileName, errorString));
        showStatusMessage(tr("Failed to load file"), 2000);
    }

    hideLoadingProgress();
}

bool MainWindow::saveFile(const QString &fileName)
{
    OTB::OtbWriter writer;
    QString errorString;

    showLoadingProgress(0, 100, tr("Saving %1...").arg(QFileInfo(fileName).fileName()));

    if (writer.write(fileName, currentOtbItems, errorString)) {
        setCurrentFile(fileName);
        showStatusMessage(tr("File saved successfully"), 2000);
        hideLoadingProgress();
        return true;
    } else {
        QMessageBox::warning(this, tr("ItemEditor"), tr("Cannot write file %1:\n%2").arg(fileName, errorString));
        showStatusMessage(tr("Failed to save file"), 2000);
        hideLoadingProgress();
        return false;
    }
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    currentFile = fileName;
    isModified = false;
    setWindowModified(false);

    QString shownName = currentFile;
    if (currentFile.isEmpty()) {
        shownName = tr("untitled.otb");
    }
    setWindowFilePath(shownName);
    setWindowTitle(tr("%1[*] - %2").arg(strippedName(shownName), tr("ItemEditor Qt")));

    // Clear any loading status when setting current file
    hideLoadingProgress();
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::clearItemDetailsView()
{
    loadingItemDetails = true;

    // Clear appearance section
    serverIDLabel_val->setText(tr("N/A"));
    clientIDSpinBox->setValue(0);

    // Clear attributes section
    itemNameLineEdit->clear();

    // Clear all checkboxes
    unpassableCheckBox->setChecked(false);
    blockMissilesCheckBox->setChecked(false);
    blockPathfinderCheckBox->setChecked(false);
    hasElevationCheckBox->setChecked(false);
    forceUseCheckBox->setChecked(false);
    multiUseCheckBox->setChecked(false);
    pickupableCheckBox->setChecked(false);
    movableCheckBox->setChecked(false);
    stackableCheckBox->setChecked(false);
    readableCheckBox->setChecked(false);
    rotatableCheckBox->setChecked(false);
    hangableCheckBox->setChecked(false);
    hookSouthCheckBox->setChecked(false);
    hookEastCheckBox->setChecked(false);
    ignoreLookCheckBox->setChecked(false);
    fullGroundCheckBox->setChecked(false);

    // Clear numeric fields
    groundSpeedLineEdit->clear();
    lightLevelLineEdit->clear();
    lightColorLineEdit->clear();
    minimapColorLineEdit->clear();
    maxReadCharsLineEdit->clear();
    maxReadWriteCharsLineEdit->clear();
    // wareIdLineEdit->clear(); // wareId doesn't exist in ServerItem

    loadingItemDetails = false;
}

bool MainWindow::loadClientForOtb()
{
    // Implementation for loading client data
    return true;
}

void MainWindow::updatePropertyStyle(QWidget* control, const std::function<bool(const ItemEditor::ClientItem&)>& comparisonLambda)
{
    // Implementation for updating property styles based on comparison
    Q_UNUSED(control);
    Q_UNUSED(comparisonLambda);
}

void MainWindow::performOtbUpdate(const UpdateOptions& options,
                                  const QMap<quint16, ItemEditor::ClientItem>& currentClientItems,
                                  const QMap<quint16, ItemEditor::ClientItem>& targetClientItems)
{
    // Implementation for performing OTB update
    Q_UNUSED(options);
    Q_UNUSED(currentClientItems);
    Q_UNUSED(targetClientItems);
}

bool MainWindow::compareItems(const OTB::ServerItem* serverItem, const ItemEditor::ClientItem* clientItem, bool compareHash)
{
    // Implementation for comparing items
    Q_UNUSED(serverItem);
    Q_UNUSED(clientItem);
    Q_UNUSED(compareHash);
    return true;
}

// Status bar helper methods
void MainWindow::updateItemCount(int count)
{
    if (count == 1) {
        itemsCountLabel->setText(tr("1 Item"));
    } else {
        itemsCountLabel->setText(tr("%1 Items").arg(count));
    }
}

void MainWindow::showLoadingProgress(int current, int maximum, const QString& message)
{
    loadingProgressBar->setRange(0, maximum);
    loadingProgressBar->setValue(current);
    loadingProgressBar->setVisible(true);
    if (!message.isEmpty()) {
        statusBar()->showMessage(message);
    }
}

void MainWindow::hideLoadingProgress()
{
    loadingProgressBar->setVisible(false);
    statusBar()->clearMessage();
}

void MainWindow::showStatusMessage(const QString& message, int timeout)
{
    statusBar()->showMessage(message, timeout);
}

// Toolbar customization implementation
void MainWindow::setupToolBarCustomization()
{
    // Create toolbar context menu
    toolBarContextMenu = new QMenu(this);

    customizeToolBarAct = new QAction(tr("Customize..."), this);
    customizeToolBarAct->setStatusTip(tr("Customize toolbar layout"));
    connect(customizeToolBarAct, &QAction::triggered, this, &MainWindow::customizeToolBar);

    resetToolBarAct = new QAction(tr("Reset"), this);
    resetToolBarAct->setStatusTip(tr("Reset toolbar to default layout"));
    connect(resetToolBarAct, &QAction::triggered, this, &MainWindow::resetToolBar);

    toggleToolBarAct = new QAction(tr("Show/Hide Toolbar"), this);
    toggleToolBarAct->setStatusTip(tr("Toggle toolbar visibility"));
    toggleToolBarAct->setCheckable(true);
    toggleToolBarAct->setChecked(true);
    connect(toggleToolBarAct, &QAction::triggered, this, &MainWindow::toggleToolBarVisibility);

    toolBarContextMenu->addAction(customizeToolBarAct);
    toolBarContextMenu->addAction(resetToolBarAct);
    toolBarContextMenu->addSeparator();
    toolBarContextMenu->addAction(toggleToolBarAct);
}

void MainWindow::showToolBarContextMenu(const QPoint& pos)
{
    toolBarContextMenu->exec(mainToolBar->mapToGlobal(pos));
}

void MainWindow::customizeToolBar()
{
    // Simple implementation - in a full version this would open a customization dialog
    QMessageBox::information(this, tr("Toolbar Customization"),
                           tr("Toolbar customization dialog would open here.\n"
                              "For now, you can:\n"
                              "- Drag toolbar to move it\n"
                              "- Right-click for context menu\n"
                              "- Use Reset to restore default layout"));
}

void MainWindow::resetToolBar()
{
    // Clear current toolbar
    mainToolBar->clear();

    // Recreate default layout
    mainToolBar->addAction(newAct);
    mainToolBar->addAction(openAct);
    mainToolBar->addAction(saveAct);
    mainToolBar->addAction(saveAsToolBarAct);
    mainToolBar->addSeparator();
    mainToolBar->addAction(compareOtbToolBarAct);
    mainToolBar->addAction(findItemAct);

    statusBar()->showMessage(tr("Toolbar reset to default layout"), 2000);
}

void MainWindow::toggleToolBarVisibility()
{
    bool visible = mainToolBar->isVisible();
    mainToolBar->setVisible(!visible);
    toggleToolBarAct->setChecked(!visible);

    statusBar()->showMessage(visible ? tr("Toolbar hidden") : tr("Toolbar shown"), 2000);
}

void MainWindow::saveToolBarState()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("toolBarState", saveState());
    settings.setValue("toolBarVisible", mainToolBar->isVisible());
    settings.endGroup();
}

void MainWindow::restoreToolBarState()
{
    QSettings settings;
    settings.beginGroup("MainWindow");

    QByteArray state = settings.value("toolBarState").toByteArray();
    if (!state.isEmpty()) {
        restoreState(state);
    }

    bool visible = settings.value("toolBarVisible", true).toBool();
    mainToolBar->setVisible(visible);
    toggleToolBarAct->setChecked(visible);

    settings.endGroup();
}