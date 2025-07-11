#include "mainwindow.h"
#include "otb/otbtypes.h" // For OTB::ServerItem etc. (will be needed in slots)
#include "dialogs/aboutdialog.h" // Include the AboutDialog header
// #include "clientitemview.h" // Will be needed when ClientItemView is created

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


#include "otb/otbreader.h" // For OtbReader
#include "plugins/dummyplugin.h" // Include DummyPlugin for instantiation

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isModified(false), currentSelectedItem(nullptr), pluginManager(nullptr), currentPlugin(nullptr)
{
    setWindowTitle(tr("ItemEditor Qt"));
    setMinimumSize(800, 600); // Initial size, can be adjusted

    pluginManager = new PluginManager(this);
    // Register the dummy plugin for now
    // In future, this would involve scanning a directory for plugin DLLs
    DummyPlugin* dummy = new DummyPlugin(this); // QObject parentage for auto-cleanup if needed
    pluginManager->registerPlugin(dummy);


    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createCentralWidget(); // Setup the main layout and widgets
    // createDockWidgets(); // If needed later

    setCurrentFile(QString()); // No file loaded initially
    clearItemDetailsView(); // Ensure details view is cleared and disabled
    editMenu->setEnabled(false);
    viewMenu->setEnabled(false);
    toolsMenu->setEnabled(false);
    statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow()
{
    // Qt handles child widget deletion automatically
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
    // File Actions
    newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new OTB file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing OTB file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::openFile);

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the current OTB file"));
    saveAct->setEnabled(false); // Initially disabled
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFile);

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the current OTB file under a new name"));
    saveAsAct->setEnabled(false); // Initially disabled
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveFileAs);

    preferencesAct = new QAction(tr("&Preferences..."), this);
    preferencesAct->setStatusTip(tr("Application preferences"));
    connect(preferencesAct, &QAction::triggered, this, &MainWindow::showPreferences);

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, &QAction::triggered, this, &QApplication::closeAllWindows);

    // Edit Actions
    createItemAct = new QAction(tr("&Create Item"), this);
    // createItemAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I)); // Example shortcut
    createItemAct->setStatusTip(tr("Create a new item"));
    createItemAct->setEnabled(false);
    connect(createItemAct, &QAction::triggered, this, &MainWindow::createNewItem);

    duplicateItemAct = new QAction(tr("&Duplicate Item"), this);
    // duplicateItemAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    duplicateItemAct->setStatusTip(tr("Duplicate the selected item"));
    duplicateItemAct->setEnabled(false);
    connect(duplicateItemAct, &QAction::triggered, this, &MainWindow::duplicateCurrentItem);

    reloadItemAct = new QAction(tr("&Reload Item"), this);
    // reloadItemAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    reloadItemAct->setStatusTip(tr("Reload attributes for the selected item"));
    reloadItemAct->setEnabled(false);
    connect(reloadItemAct, &QAction::triggered, this, &MainWindow::reloadCurrentItem);

    findItemAct = new QAction(tr("&Find Item..."), this);
    findItemAct->setShortcuts(QKeySequence::Find);
    findItemAct->setStatusTip(tr("Find an item"));
    findItemAct->setEnabled(false);
    connect(findItemAct, &QAction::triggered, this, &MainWindow::findItem);

    createMissingItemsAct = new QAction(tr("Create Missing Items"), this);
    createMissingItemsAct->setStatusTip(tr("Create items that are in client but not OTB"));
    createMissingItemsAct->setEnabled(false);
    connect(createMissingItemsAct, &QAction::triggered, this, &MainWindow::createMissingItems);


    // View Actions
    showMismatchedAct = new QAction(tr("Show &Mismatched Items"), this);
    showMismatchedAct->setCheckable(true);
    showMismatchedAct->setStatusTip(tr("Toggle display of items that differ from client data"));
    showMismatchedAct->setEnabled(false);
    connect(showMismatchedAct, &QAction::triggered, this, &MainWindow::toggleShowMismatched);

    showDeprecatedAct = new QAction(tr("Show &Deprecated Items"), this);
    showDeprecatedAct->setCheckable(true);
    showDeprecatedAct->setStatusTip(tr("Toggle display of deprecated items"));
    showDeprecatedAct->setEnabled(false);
    connect(showDeprecatedAct, &QAction::triggered, this, &MainWindow::toggleShowDeprecated);

    updateItemsListAct = new QAction(tr("&Update Items List"), this);
    updateItemsListAct->setStatusTip(tr("Refresh the list of items"));
    updateItemsListAct->setEnabled(false);
    connect(updateItemsListAct, &QAction::triggered, this, &MainWindow::updateItemsList);


    // Tools Actions
    reloadAttributesAct = new QAction(tr("&Reload All Item Attributes"), this);
    reloadAttributesAct->setStatusTip(tr("Reload attributes for all items from client data"));
    reloadAttributesAct->setEnabled(false);
    connect(reloadAttributesAct, &QAction::triggered, this, &MainWindow::reloadAllItemAttributes);

    compareOtbAct = new QAction(tr("&Compare OTB Files..."), this);
    compareOtbAct->setStatusTip(tr("Compare two OTB files"));
    connect(compareOtbAct, &QAction::triggered, this, &MainWindow::compareOtbFiles);

    updateVersionAct = new QAction(tr("&Update OTB Version..."), this);
    updateVersionAct->setStatusTip(tr("Update the OTB to a new client version"));
    updateVersionAct->setEnabled(false);
    connect(updateVersionAct, &QAction::triggered, this, &MainWindow::updateOtbVersion);


    // Help Actions
    aboutAct = new QAction(tr("&About ItemEditor"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, &QAction::triggered, qApp, &QApplication::aboutQt);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(preferencesAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(createItemAct);
    editMenu->addAction(duplicateItemAct);
    editMenu->addAction(reloadItemAct);
    editMenu->addSeparator();
    editMenu->addAction(findItemAct);
    editMenu->addSeparator();
    editMenu->addAction(createMissingItemsAct);


    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(showMismatchedAct);
    viewMenu->addAction(showDeprecatedAct);
    viewMenu->addAction(updateItemsListAct);

    toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(reloadAttributesAct);
    toolsMenu->addSeparator();
    toolsMenu->addAction(compareOtbAct);
    toolsMenu->addAction(updateVersionAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
    // Add SaveAs, Compare, Find to toolbar as in C#
    // toolStripSaveAsButton -> saveAsAct (if icon needed)
    // toolStripCompareButton -> compareOtbAct
    // toolStripFindItemButton -> findItemAct

    // editToolBar = addToolBar(tr("Edit"));
    // editToolBar->addAction(createItemAct);
    // editToolBar->addAction(duplicateItemAct);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createCentralWidget()
{
    QWidget *mainWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(mainWidget);

    // Left Panel (Server Item List and buttons below it)
    QVBoxLayout *leftPanelLayout = new QVBoxLayout();
    serverItemListBox = new QListWidget();
    connect(serverItemListBox, &QListWidget::currentItemChanged, this, &MainWindow::onServerItemSelectionChanged);
    leftPanelLayout->addWidget(serverItemListBox, 1); // Stretch factor 1

    QHBoxLayout* itemButtonsLayout = new QHBoxLayout();
    newItemButtonMain = new QPushButton(tr("New"));
    duplicateItemButtonMain = new QPushButton(tr("Duplicate"));
    reloadItemButtonMain = new QPushButton(tr("Reload"));
    findItemButtonMain = new QPushButton(tr("Find"));
    // TODO: Set icons for these buttons later
    // TODO: Enable/disable based on state
    itemButtonsLayout->addWidget(newItemButtonMain);
    itemButtonsLayout->addWidget(duplicateItemButtonMain);
    itemButtonsLayout->addWidget(reloadItemButtonMain);
    itemButtonsLayout->addStretch(1); // Pushes find to the right if needed, or remove for compact
    itemButtonsLayout->addWidget(findItemButtonMain);
    leftPanelLayout->addLayout(itemButtonsLayout);

    QWidget* leftPanelWidget = new QWidget();
    leftPanelWidget->setLayout(leftPanelLayout);
    leftPanelWidget->setMinimumWidth(200); // Example width
    leftPanelWidget->setMaximumWidth(300); // Example max width


    // Right Panel (Appearance, Attributes, Output Log)
    QVBoxLayout *rightPanelLayout = new QVBoxLayout();

    // Top part of Right Panel (Appearance and Attributes side-by-side)
    QHBoxLayout *topRightLayout = new QHBoxLayout();

    appearanceGroupBox = new QGroupBox(tr("Appearance"));
    QGridLayout *appearanceLayout = new QGridLayout(appearanceGroupBox);
    // previousClientItemViewWidget = new ClientItemView(appearanceGroupBox); // Placeholder
    // mainClientItemViewWidget = new ClientItemView(appearanceGroupBox);   // Placeholder
    // appearanceLayout->addWidget(new QLabel(tr("Previous:")), 0, 0);
    // appearanceLayout->addWidget(previousClientItemViewWidget, 1, 0, 1, 2); // Spanning 2 columns for view
    // appearanceLayout->addWidget(new QLabel(tr("Current:")), 2, 0);
    // appearanceLayout->addWidget(mainClientItemViewWidget, 3, 0, 1, 2);
    appearanceLayout->addWidget(new QLabel(tr("Server ID:")), 4, 0);
    serverIDLabel_val = new QLabel(tr("0"));
    appearanceLayout->addWidget(serverIDLabel_val, 4, 1);
    appearanceLayout->addWidget(new QLabel(tr("Client ID:")), 5, 0);
    clientIDSpinBox = new QSpinBox();
    appearanceLayout->addWidget(clientIDSpinBox, 5, 1);
    candidatesButton = new QPushButton(tr("Candidates"));
    appearanceLayout->addWidget(candidatesButton, 6, 0, 1, 2);
    appearanceLayout->setRowStretch(7,1); // Add stretch at the bottom
    topRightLayout->addWidget(appearanceGroupBox);

    attributesGroupBox = new QGroupBox(tr("Attributes"));
    QGridLayout *attributesLayout = new QGridLayout(attributesGroupBox);
    // Add flag checkboxes
    unpassableCheckBox = new QCheckBox(tr("Unpassable"));
    attributesLayout->addWidget(unpassableCheckBox, 0, 0);
    movableCheckBox = new QCheckBox(tr("Movable"));
    attributesLayout->addWidget(movableCheckBox, 1, 0);
    blockMissilesCheckBox = new QCheckBox(tr("Block Missiles"));
    attributesLayout->addWidget(blockMissilesCheckBox, 0, 1);
    // ... (add all other checkboxes and lineedits similarly) ...
    attributesLayout->addWidget(new QLabel(tr("Item Type:")), 10, 0);
    itemTypeComboBox = new QComboBox();
    attributesLayout->addWidget(itemTypeComboBox, 10, 1);
    attributesLayout->addWidget(new QLabel(tr("Stack Order:")), 11, 0);
    stackOrderComboBox = new QComboBox();
    attributesLayout->addWidget(stackOrderComboBox, 11, 1);
    attributesLayout->addWidget(new QLabel(tr("Name:")), 12, 0);
    itemNameLineEdit = new QLineEdit();
    attributesLayout->addWidget(itemNameLineEdit, 12, 1);
    // Add more attributes...
    attributesLayout->setRowStretch(13,1); // Add stretch at the bottom
    attributesGroupBox->setMinimumWidth(350);
    topRightLayout->addWidget(attributesGroupBox,1); // Stretch factor for attributes box

    rightPanelLayout->addLayout(topRightLayout);

    // Output Log View
    outputLogView = new QTextEdit();
    outputLogView->setReadOnly(true);
    rightPanelLayout->addWidget(outputLogView, 1); // Stretch factor 1

    // Status Bar items (not directly in central widget, but related to bottom area)
    QHBoxLayout* statusBarItemsLayout = new QHBoxLayout();
    itemsCountLabel = new QLabel(tr("0 Items"));
    loadingProgressBar = new QProgressBar();
    loadingProgressBar->setVisible(false); // Initially hidden
    statusBarItemsLayout->addWidget(itemsCountLabel);
    statusBarItemsLayout->addSpacing(20);
    statusBarItemsLayout->addWidget(loadingProgressBar);
    statusBarItemsLayout->addStretch(1);
    // This layout would typically be added to a custom status bar widget or managed separately.
    // For simplicity here, we'll add it to the right panel.
    // rightPanelLayout->addLayout(statusBarItemsLayout); // Or add to actual status bar later.


    // Main Layout Assembly
    mainLayout->addWidget(leftPanelWidget);
    mainLayout->addLayout(rightPanelLayout, 1); // Right panel takes more stretch

    setCentralWidget(mainWidget);
}

void MainWindow::createDockWidgets()
{
    // Example:
    // QDockWidget *dock = new QDockWidget(tr("Output Log"), this);
    // dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    // outputLogView = new QTextEdit(dock);
    // outputLogView->setReadOnly(true);
    // dock->setWidget(outputLogView);
    // addDockWidget(Qt::BottomDockWidgetArea, dock);
    // viewMenu->addAction(dock->toggleViewAction());
}


// --- Placeholder Slot Implementations ---
void MainWindow::newFile() { QMessageBox::information(this, "Not Implemented", "New File placeholder."); }

void MainWindow::openFile()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this,
                                   tr("Open OTB File"), currentFile, // Or last opened directory
                                   tr("OTB Files (*.otb);;All Files (*)"));
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

bool MainWindow::saveFile() { QMessageBox::information(this, "Not Implemented", "Save File placeholder."); return false; }
bool MainWindow::saveFileAs() { QMessageBox::information(this, "Not Implemented", "Save File As placeholder."); return false; }
void MainWindow::showPreferences() { QMessageBox::information(this, "Not Implemented", "Preferences placeholder."); }
void MainWindow::createNewItem() { QMessageBox::information(this, "Not Implemented", "Create New Item placeholder."); }
void MainWindow::duplicateCurrentItem() { QMessageBox::information(this, "Not Implemented", "Duplicate Current Item placeholder."); }
void MainWindow::reloadCurrentItem() { QMessageBox::information(this, "Not Implemented", "Reload Current Item placeholder."); }
void MainWindow::findItem() { QMessageBox::information(this, "Not Implemented", "Find Item placeholder."); }
void MainWindow::createMissingItems() { QMessageBox::information(this, "Not Implemented", "Create Missing Items placeholder."); }
void MainWindow::toggleShowMismatched(bool) { QMessageBox::information(this, "Not Implemented", "Toggle Show Mismatched placeholder."); }
void MainWindow::toggleShowDeprecated(bool) { QMessageBox::information(this, "Not Implemented", "Toggle Show Deprecated placeholder."); }
void MainWindow::updateItemsList() { QMessageBox::information(this, "Not Implemented", "Update Items List placeholder."); }
void MainWindow::reloadAllItemAttributes() { QMessageBox::information(this, "Not Implemented", "Reload All Item Attributes placeholder."); }
void MainWindow::compareOtbFiles() { QMessageBox::information(this, "Not Implemented", "Compare OTB Files placeholder."); }
void MainWindow::updateOtbVersion() { QMessageBox::information(this, "Not Implemented", "Update OTB Version placeholder."); }
void MainWindow::about()
{
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}

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
        // Enable/disable relevant actions like duplicate, reload
        duplicateItemAct->setEnabled(true);
        reloadItemAct->setEnabled(true);
        // Enable attribute group boxes
        appearanceGroupBox->setEnabled(true);
        attributesGroupBox->setEnabled(true);

    } else {
        currentSelectedItem = nullptr;
        clearItemDetailsView();
    }
}

void MainWindow::clearItemDetailsView() {
    serverIDLabel_val->setText("0");
    clientIDSpinBox->setValue(0); // Or min value
    itemNameLineEdit->clear();
    itemTypeComboBox->setCurrentIndex(-1); // No selection
    stackOrderComboBox->setCurrentIndex(-1);

    // Disable group boxes
    appearanceGroupBox->setEnabled(false);
    attributesGroupBox->setEnabled(false);

    // Disable actions that require a selected item
    duplicateItemAct->setEnabled(false);
    reloadItemAct->setEnabled(false);

    // Clear checkboxes
    unpassableCheckBox->setChecked(false);
    movableCheckBox->setChecked(false);
    blockMissilesCheckBox->setChecked(false);
    hasElevationCheckBox->setChecked(false);
    forceUseCheckBox->setChecked(false);
    multiUseCheckBox->setChecked(false);
    pickupableCheckBox->setChecked(false);
    stackableCheckBox->setChecked(false);
    readableCheckBox->setChecked(false);
    rotatableCheckBox->setChecked(false);
    hangableCheckBox->setChecked(false);
    hookSouthCheckBox->setChecked(false);
    hookEastCheckBox->setChecked(false);
    ignoreLookCheckBox->setChecked(false);
    fullGroundCheckBox->setChecked(false);
    // ... clear all other checkboxes and lineedits in attributesGroupBox ...
    groundSpeedLineEdit->clear();
    lightLevelLineEdit->clear();
    lightColorLineEdit->clear();
    minimapColorLineEdit->clear();
    maxReadCharsLineEdit->clear();
    maxReadWriteCharsLineEdit->clear();
    wareIdLineEdit->clear();

    // Reset colors from potential red highlighting (if implemented)
    // Example: itemNameLineEdit->setStyleSheet("");
}

void MainWindow::showPreferences() {
    // Placeholder for PreferencesDialog
    // This dialog would allow selecting a client (which means selecting a plugin and one of its SupportedClient versions)
    // For now, let's try to auto-select the first available plugin and its first client for testing.
    if (pluginManager && !pluginManager->availablePlugins().isEmpty()) {
        IPlugin* selectedPlugin = pluginManager->availablePlugins().first();
        if (!selectedPlugin->getSupportedClients().isEmpty()) {
            const OTB::SupportedClient& clientToLoad = selectedPlugin->getSupportedClients().first();
            QString errorStr;
            // In a real scenario, clientDirectoryPath would come from QSettings or a dialog
            QString dummyClientPath = "."; // Dummy path, not used by DummyPlugin

            if (currentPlugin && currentPlugin->isClientLoaded() && currentPlugin->getCurrentLoadedClient().version == clientToLoad.version) {
                 QMessageBox::information(this, tr("Preferences"), tr("Client %1 is already loaded.").arg(clientToLoad.description));
                return;
            }

            if (selectedPlugin->loadClient(clientToLoad, dummyClientPath, true, true, true, errorStr)) {
                currentPlugin = selectedPlugin; // Set the active plugin
                statusBar()->showMessage(tr("Client %1 loaded via %2").arg(clientToLoad.description, currentPlugin->pluginName()), 5000);
                 if (currentSelectedItem) { // Refresh view if an item is selected
                    updateItemDetailsView(currentSelectedItem);
                }
            } else {
                QMessageBox::warning(this, tr("Plugin Error"), tr("Could not load client %1 with %2:\n%3")
                                     .arg(clientToLoad.description, selectedPlugin->pluginName(), errorStr));
                currentPlugin = nullptr;
            }
        } else {
            QMessageBox::information(this, tr("Preferences"), tr("Selected plugin has no supported clients."));
            currentPlugin = nullptr;
        }
    } else {
        QMessageBox::information(this, tr("Preferences"), tr("No plugins available."));
        currentPlugin = nullptr;
    }
}


// Slot for future use when a ServerItem object itself is passed (e.g. if not using QListWidget directly)
void MainWindow::currentServerItemChanged(OTB::ServerItem* item) {
    currentSelectedItem = item;
    updateItemDetailsView(item);
     // This slot might not be directly connected if QListWidget::currentItemChanged is used primarily.
    // It's here for conceptual similarity to C# or if we change list view implementation.
}

void MainWindow::updateItemDetailsView(OTB::ServerItem* item) {
    if (!item) {
        clearItemDetailsView();
        return;
    }
    serverIDLabel_val->setText(QString::number(item->id));
    clientIDSpinBox->setValue(item->clientId);
    itemNameLineEdit->setText(item->name);

    // Populate itemTypeComboBox and stackOrderComboBox if not already done
    // This should ideally happen once after an OTB is loaded or client data is known.
    if (itemTypeComboBox->count() == 0) {
        // Example: Manually populate from OTB::ServerItemType enum
        itemTypeComboBox->addItem(tr("None"), QVariant::fromValue(static_cast<int>(OTB::ServerItemType::None)));
        itemTypeComboBox->addItem(tr("Ground"), QVariant::fromValue(static_cast<int>(OTB::ServerItemType::Ground)));
        itemTypeComboBox->addItem(tr("Container"), QVariant::fromValue(static_cast<int>(OTB::ServerItemType::Container)));
        itemTypeComboBox->addItem(tr("Splash"), QVariant::fromValue(static_cast<int>(OTB::ServerItemType::Splash)));
        itemTypeComboBox->addItem(tr("Fluid"), QVariant::fromValue(static_cast<int>(OTB::ServerItemType::Fluid)));
        itemTypeComboBox->addItem(tr("Deprecated"), QVariant::fromValue(static_cast<int>(OTB::ServerItemType::Deprecated)));
    }
    int typeIndex = itemTypeComboBox->findData(QVariant::fromValue(static_cast<int>(item->type)));
    itemTypeComboBox->setCurrentIndex(typeIndex);

    if (stackOrderComboBox->count() == 0) {
        // Example: Manually populate from OTB::TileStackOrder enum
        stackOrderComboBox->addItem(tr("None"), QVariant::fromValue(static_cast<int>(OTB::TileStackOrder::None)));
        stackOrderComboBox->addItem(tr("Border"), QVariant::fromValue(static_cast<int>(OTB::TileStackOrder::Border)));
        stackOrderComboBox->addItem(tr("Ground"), QVariant::fromValue(static_cast<int>(OTB::TileStackOrder::Ground)));
        stackOrderComboBox->addItem(tr("Bottom"), QVariant::fromValue(static_cast<int>(OTB::TileStackOrder::Bottom)));
        stackOrderComboBox->addItem(tr("Top"), QVariant::fromValue(static_cast<int>(OTB::TileStackOrder::Top)));
        stackOrderComboBox->addItem(tr("Creature"), QVariant::fromValue(static_cast<int>(OTB::TileStackOrder::Creature)));
    }
    int stackOrderIndex = stackOrderComboBox->findData(QVariant::fromValue(static_cast<int>(item->stackOrder)));
    stackOrderComboBox->setCurrentIndex(stackOrderIndex);


    // Set checkboxes based on item->flags
    // Ensure ServerItem::updatePropertiesFromFlags() was called after loading if you rely on bool members.
    // Or directly use item->hasFlag()
    unpassableCheckBox->setChecked(item->hasFlag(OTB::ServerItemFlag::Unpassable));
    movableCheckBox->setChecked(item->hasFlag(OTB::ServerItemFlag::Movable));
    blockMissilesCheckBox->setChecked(item->hasFlag(OTB::ServerItemFlag::BlockMissiles));
    hasElevationCheckBox->setChecked(item->hasFlag(OTB::ServerItemFlag::HasElevation));
    forceUseCheckBox->setChecked(item->hasFlag(OTB::ServerItemFlag::ForceUse));
    multiUseCheckBox->setChecked(item->hasFlag(OTB::ServerItemFlag::MultiUse));
    pickupableCheckBox->setChecked(item->hasFlag(OTB::ServerItemFlag::Pickupable));
    stackableCheckBox->setChecked(item->hasFlag(OTB::ServerItemFlag::Stackable));
    readableCheckBox->setChecked(item->hasFlag(OTB::ServerItemFlag::Readable));
    rotatableCheckBox->setChecked(item->hasFlag(OTB::ServerItemFlag::Rotatable));
    hangableCheckBox->setChecked(item->hasFlag(OTB::ServerItemFlag::Hangable));
    hookSouthCheckBox->setChecked(item->hasFlag(OTB::ServerItemFlag::HookSouth));
    hookEastCheckBox->setChecked(item->hasFlag(OTB::ServerItemFlag::HookEast));
    ignoreLookCheckBox->setChecked(item->hasFlag(OTB::ServerItemFlag::IgnoreLook));
    fullGroundCheckBox->setChecked(item->hasFlag(OTB::ServerItemFlag::FullGround));


    groundSpeedLineEdit->setText(QString::number(item->groundSpeed));
    lightLevelLineEdit->setText(QString::number(item->lightLevel));
    lightColorLineEdit->setText(QString::number(item->lightColor));
    minimapColorLineEdit->setText(QString::number(item->minimapColor));
    maxReadCharsLineEdit->setText(QString::number(item->maxReadChars));
    maxReadWriteCharsLineEdit->setText(QString::number(item->maxReadWriteChars));
    wareIdLineEdit->setText(QString::number(item->tradeAs));

    // TODO: Handle sprite display in mainClientItemViewWidget and previousClientItemViewWidget

    // If a plugin is loaded, try to get ClientItem info
    if (currentPlugin && currentPlugin->isClientLoaded() && item) {
        OTB::ClientItem clientItem;
        if (currentPlugin->getClientItem(item->clientId, clientItem)) {
            // Successfully got client item, update relevant UI parts
            // Example: Update a label with client item's name or type if different
            // For now, just log it.
            qDebug() << "Selected ServerItem ID:" << item->id << " (ClientID:" << item->clientId << ")";
            qDebug() << "Corresponding ClientItem Name (from plugin):" << clientItem.name;
            // TODO: Update ClientItemView widgets (pictureBox, previousPictureBox equivalents)
            // mainClientItemViewWidget->setClientItem(&clientItem);
        } else {
            qDebug() << "ClientItem with ID" << item->clientId << "not found in current plugin.";
            // mainClientItemViewWidget->setClientItem(nullptr);
        }
    } else {
        // No plugin loaded or no item selected, clear client specific views
        // mainClientItemViewWidget->setClientItem(nullptr);
        // previousClientItemViewWidget->setClientItem(nullptr);
    }
}

void MainWindow::updateClientItemView(OTB::ClientItem* clientItem) { /* ... */ }
void MainWindow::updatePreviousClientItemView(OTB::ClientItem* prevClientItem) { /* ... */ }


bool MainWindow::loadClientForOtb() {
    if (currentOtbItems.items.isEmpty()) {
        qWarning() << "loadClientForOtb: No OTB items loaded.";
        return false; // No OTB loaded, or it's empty
    }

    // Try to find a plugin that supports the OTB's client version
    // OTB stores client version in currentOtbItems.minorVersion (as per C# OtbReader)
    // or currentOtbItems.clientVersion (which is set from minorVersion in C# MainForm.LoadClient)
    // Let's assume currentOtbItems.minorVersion is the OTB's target client version identifier for plugins.
    quint32 otbClientVersionTarget = currentOtbItems.minorVersion;
    if (otbClientVersionTarget == 0 && currentOtbItems.clientVersion != 0) {
         otbClientVersionTarget = currentOtbItems.clientVersion; // Fallback if minorVersion wasn't the primary one
    }


    IPlugin* foundPlugin = pluginManager->findPluginForOtbVersion(otbClientVersionTarget);

    if (foundPlugin) {
        const OTB::SupportedClient* clientToLoad = nullptr;
        for(const auto& sc : foundPlugin->getSupportedClients()){
            if(sc.otbVersion == otbClientVersionTarget || sc.version == otbClientVersionTarget){ // Match OTB or direct client version
                clientToLoad = &sc;
                break;
            }
        }

        if (clientToLoad) {
            QString errorStr;
            // Dummy path, actual path would come from settings/dialog
            if (foundPlugin->loadClient(*clientToLoad, ".", true, true, true, errorStr)) {
                currentPlugin = foundPlugin;
                statusBar()->showMessage(tr("Client %1 automatically loaded for OTB via %2")
                                         .arg(clientToLoad->description, currentPlugin->pluginName()), 5000);
                return true;
            } else {
                QMessageBox::warning(this, tr("Plugin Error"), tr("Auto-load failed for client %1 with %2:\n%3")
                                     .arg(clientToLoad->description, foundPlugin->pluginName(), errorStr));
                currentPlugin = nullptr;
            }
        } else {
             QMessageBox::information(this, tr("Plugin Info"), tr("Plugin %1 supports OTB version %2, but no exact client match found in its list.")
                                     .arg(foundPlugin->pluginName()).arg(otbClientVersionTarget));
            currentPlugin = nullptr;
        }
    } else {
        QMessageBox::information(this, tr("Plugin Info"), tr("No plugin found that supports client version %1 (from OTB). Please check Preferences.").arg(otbClientVersionTarget));
        currentPlugin = nullptr;
    }
    return false;
}


// --- Helper Methods ---
bool MainWindow::maybeSave()
{
    if (!isModified)
        return true;
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Application"),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return saveFile();
    case QMessageBox::Cancel:
        return false;
    default: // QMessageBox::Discard
        break;
    }
    return true;
}

void MainWindow::loadFile(const QString &fileName)
{
    // Placeholder for actual file loading logic using OtbReader
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) { // Adjust for binary OTB
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }
    // QTextStream in(&file); // Replace with OTB parsing
    // QApplication::setOverrideCursor(Qt::WaitCursor);
    // textEdit->setPlainText(in.readAll()); // Replace with populating item list
    // QApplication::restoreOverrideCursor();

    OTB::OtbReader reader;
    QString errorString;
    currentOtbItems.clear(); // Clear previous data
    serverItemListBox->clear();
    listItemToServerItemMap.clear();
    clearItemDetailsView();
    if(currentPlugin) { // Unload previously loaded client if any
        currentPlugin->unloadClient();
        currentPlugin = nullptr;
    }


    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool success = reader.read(fileName, currentOtbItems, errorString);
    QApplication::restoreOverrideCursor();

    if (success) {
        setCurrentFile(fileName); // Set current file early
        loadClientForOtb(); // Attempt to load a client based on OTB version

        for (int i = 0; i < currentOtbItems.items.size(); ++i) {
            OTB::ServerItem* serverItem = &currentOtbItems.items[i];
            QListWidgetItem *listItem = new QListWidgetItem(QString("[%1] %2").arg(serverItem->id).arg(serverItem->name), serverItemListBox);
            listItemToServerItemMap.insert(listItem, serverItem);
        }

        statusBar()->showMessage(tr("File loaded: %1 items").arg(currentOtbItems.items.count()), 5000);
        saveAct->setEnabled(true);
        saveAsAct->setEnabled(true);
        editMenu->setEnabled(true);
        viewMenu->setEnabled(true);
        toolsMenu->setEnabled(true);
        createItemAct->setEnabled(true);
        findItemAct->setEnabled(true);
        createMissingItemsAct->setEnabled(true);
        showMismatchedAct->setEnabled(true);
        showDeprecatedAct->setEnabled(true);
        updateItemsListAct->setEnabled(true);
        reloadAttributesAct->setEnabled(true);
        updateVersionAct->setEnabled(true);

        isModified = false;
        itemsCountLabel->setText(tr("%1 Items").arg(currentOtbItems.items.count()));
        if (serverItemListBox->count() > 0) {
            serverItemListBox->setCurrentRow(0);
        } else {
            clearItemDetailsView(); // Ensure view is clear if OTB is empty
        }
    } else {
        QMessageBox::critical(this, tr("Error Loading File"),
                             tr("Could not load file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), errorString));
        statusBar()->showMessage(tr("Error loading file"), 5000);
        saveAct->setEnabled(false);
        saveAsAct->setEnabled(false);
        // Disable relevant actions
        editMenu->setEnabled(false);
        viewMenu->setEnabled(false);
        toolsMenu->setEnabled(false);
        clearItemDetailsView();
        itemsCountLabel->setText(tr("0 Items"));
    }
}

bool MainWindow::saveFile(const QString &fileName)
{
    // Placeholder for actual file saving logic using OtbWriter
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) { // Adjust for binary OTB
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }
    // QTextStream out(&file); // Replace with OTB serialization
    // QApplication::setOverrideCursor(Qt::WaitCursor);
    // out << textEdit->toPlainText(); // Replace with serializing item data
    // QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    isModified = false;
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    currentFile = fileName;
    isModified = false;
    setWindowModified(false);

    QString shownName = currentFile;
    if (currentFile.isEmpty())
        shownName = "untitled.otb";
    setWindowFilePath(shownName);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}
