#include "mainwindow.h"
#include "otb/otbtypes.h"
#include "dialogs/aboutdialog.h"
#include "dialogs/spritecandidatesdialog.h"
#include "otb/otbreader.h"
#include "otb/otbwriter.h"
#include "plugins/dummyplugin.h"
#include "widgets/clientitemview.h"

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
#include <functional> // For std::function


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isModified(false), currentSelectedItem(nullptr), pluginManager(nullptr), currentPlugin(nullptr), loadingItemDetails(false)
{
    setWindowTitle(tr("ItemEditor Qt"));
    setMinimumSize(800, 700);

    pluginManager = new PluginManager(this);
    DummyPlugin* dummy = new DummyPlugin(this);
    pluginManager->registerPlugin(dummy);

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createCentralWidget();

    setCurrentFile(QString());
    clearItemDetailsView();
    editMenu->setEnabled(false);
    viewMenu->setEnabled(false);
    toolsMenu->setEnabled(false);
    statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow()
{
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
    saveAct->setEnabled(false);
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFile);

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the current OTB file under a new name"));
    saveAsAct->setEnabled(false);
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
    createItemAct->setStatusTip(tr("Create a new item"));
    createItemAct->setEnabled(false);
    connect(createItemAct, &QAction::triggered, this, &MainWindow::createNewItem);

    duplicateItemAct = new QAction(tr("&Duplicate Item"), this);
    duplicateItemAct->setStatusTip(tr("Duplicate the selected item"));
    duplicateItemAct->setEnabled(false);
    connect(duplicateItemAct, &QAction::triggered, this, &MainWindow::duplicateCurrentItem);

    reloadItemAct = new QAction(tr("&Reload Item"), this);
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
}

void MainWindow::createStatusBar()
{
}

void MainWindow::createCentralWidget()
{
    QWidget *mainWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(mainWidget);

    // Left Panel
    QVBoxLayout *leftPanelLayout = new QVBoxLayout();
    serverItemListBox = new QListWidget();
    serverItemListBox->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(serverItemListBox, &QListWidget::customContextMenuRequested, this, &MainWindow::showServerListContextMenu);
    connect(serverItemListBox, &QListWidget::currentItemChanged, this, &MainWindow::onServerItemSelectionChanged);
    leftPanelLayout->addWidget(serverItemListBox, 1);

    QHBoxLayout* itemButtonsLayout = new QHBoxLayout();
    newItemButtonMain = new QPushButton(tr("New"));
    connect(newItemButtonMain, &QPushButton::clicked, this, &MainWindow::createNewItem);
    duplicateItemButtonMain = new QPushButton(tr("Duplicate"));
    duplicateItemButtonMain->setEnabled(false);
    connect(duplicateItemButtonMain, &QPushButton::clicked, this, &MainWindow::duplicateCurrentItem);
    reloadItemButtonMain = new QPushButton(tr("Reload"));
    reloadItemButtonMain->setEnabled(false);
    connect(reloadItemButtonMain, &QPushButton::clicked, this, &MainWindow::reloadCurrentItem);
    findItemButtonMain = new QPushButton(tr("Find"));
    // connect(findItemButtonMain, &QPushButton::clicked, this, &MainWindow::findItem);

    itemButtonsLayout->addWidget(newItemButtonMain);
    itemButtonsLayout->addWidget(duplicateItemButtonMain);
    itemButtonsLayout->addWidget(reloadItemButtonMain);
    itemButtonsLayout->addStretch(1);
    itemButtonsLayout->addWidget(findItemButtonMain);
    leftPanelLayout->addLayout(itemButtonsLayout);

    QWidget* leftPanelWidget = new QWidget();
    leftPanelWidget->setLayout(leftPanelLayout);
    leftPanelWidget->setMinimumWidth(200);
    leftPanelWidget->setMaximumWidth(300);

    // Right Panel
    QVBoxLayout *rightPanelLayout = new QVBoxLayout();
    QHBoxLayout *topRightLayout = new QHBoxLayout();

    appearanceGroupBox = new QGroupBox(tr("Appearance"));
    QGridLayout *appearanceLayout = new QGridLayout(appearanceGroupBox);

    previousClientItemViewWidget = new ClientItemView(appearanceGroupBox);
    mainClientItemViewWidget = new ClientItemView(appearanceGroupBox);

    appearanceLayout->addWidget(new QLabel(tr("Previous:")), 0, 0, 1, 2, Qt::AlignCenter);
    appearanceLayout->addWidget(previousClientItemViewWidget, 1, 0, 1, 2, Qt::AlignCenter);
    appearanceLayout->addWidget(new QLabel(tr("Current:")), 2, 0, 1, 2, Qt::AlignCenter);
    appearanceLayout->addWidget(mainClientItemViewWidget, 3, 0, 1, 2, Qt::AlignCenter);

    appearanceLayout->addWidget(new QLabel(tr("Server ID:")), 4, 0);
    serverIDLabel_val = new QLabel(tr("0"));
    appearanceLayout->addWidget(serverIDLabel_val, 4, 1);
    appearanceLayout->addWidget(new QLabel(tr("Client ID:")), 5, 0);
    clientIDSpinBox = new QSpinBox();
    clientIDSpinBox->setRange(0, 65535);
    connect(clientIDSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::onClientIdChanged);
    appearanceLayout->addWidget(clientIDSpinBox, 5, 1);
    candidatesButton = new QPushButton(tr("Candidates"));
    connect(candidatesButton, &QPushButton::clicked, this, &MainWindow::showSpriteCandidates);
    appearanceLayout->addWidget(candidatesButton, 6, 0, 1, 2);
    appearanceLayout->setRowStretch(7,1);
    topRightLayout->addWidget(appearanceGroupBox);

    attributesGroupBox = new QGroupBox(tr("Attributes"));
    QGridLayout *attributesLayout = new QGridLayout(attributesGroupBox);

    int attr_row = 0, attr_col = 0;
    auto addFlagCheckBox = [&](QCheckBox*& checkBox, const QString& label, void (MainWindow::*slot)(bool)) {
        checkBox = new QCheckBox(label);
        connect(checkBox, &QCheckBox::toggled, this, slot);
        attributesLayout->addWidget(checkBox, attr_row, attr_col);
        attr_col++;
        if (attr_col >= 2) { attr_col = 0; attr_row++; }
    };

    addFlagCheckBox(unpassableCheckBox, tr("Unpassable"), &MainWindow::onUnpassableChanged);
    addFlagCheckBox(blockMissilesCheckBox, tr("Block Missiles"), &MainWindow::onBlockMissilesChanged);
    addFlagCheckBox(movableCheckBox, tr("Movable"), &MainWindow::onMovableChanged);
    addFlagCheckBox(blockPathfinderCheckBox, tr("Block Pathfinder"), &MainWindow::onBlockPathfinderChanged);
    addFlagCheckBox(pickupableCheckBox, tr("Pickupable"), &MainWindow::onPickupableChanged);
    addFlagCheckBox(hasElevationCheckBox, tr("Has Elevation"), &MainWindow::onHasElevationChanged);
    addFlagCheckBox(stackableCheckBox, tr("Stackable"), &MainWindow::onStackableChanged);
    addFlagCheckBox(forceUseCheckBox, tr("Force Use"), &MainWindow::onForceUseChanged);
    addFlagCheckBox(readableCheckBox, tr("Readable"), &MainWindow::onReadableChanged);
    addFlagCheckBox(multiUseCheckBox, tr("Multi Use"), &MainWindow::onMultiUseChanged);
    addFlagCheckBox(rotatableCheckBox, tr("Rotatable"), &MainWindow::onRotatableChanged);
    addFlagCheckBox(ignoreLookCheckBox, tr("Ignore Look"), &MainWindow::onIgnoreLookChanged);
    addFlagCheckBox(hangableCheckBox, tr("Hangable"), &MainWindow::onHangableChanged);
    addFlagCheckBox(fullGroundCheckBox, tr("Full Ground"), &MainWindow::onFullGroundChanged);
    addFlagCheckBox(hookSouthCheckBox, tr("Hook South"), &MainWindow::onHookSouthChanged);
    addFlagCheckBox(hookEastCheckBox, tr("Hook East"), &MainWindow::onHookEastChanged);

    if (attr_col != 0) { attr_row++; attr_col = 0; }

    attributesLayout->addWidget(new QLabel(tr("Name:")), attr_row, 0);
    itemNameLineEdit = new QLineEdit();
    connect(itemNameLineEdit, &QLineEdit::textChanged, this, &MainWindow::onItemNameChanged);
    attributesLayout->addWidget(itemNameLineEdit, attr_row, 1);
    attr_row++;

    attributesLayout->addWidget(new QLabel(tr("Item Type:")), attr_row, 0);
    itemTypeComboBox = new QComboBox();
    connect(itemTypeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::onItemTypeChanged);
    attributesLayout->addWidget(itemTypeComboBox, attr_row, 1);
    attr_row++;

    attributesLayout->addWidget(new QLabel(tr("Stack Order:")), attr_row, 0);
    stackOrderComboBox = new QComboBox();
    connect(stackOrderComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::onStackOrderChanged);
    attributesLayout->addWidget(stackOrderComboBox, attr_row, 1);
    attr_row++;

    auto addAttributeLineEdit = [&](QLineEdit*& lineEdit, const QString& label, void (MainWindow::*slot)(const QString&)) {
        attributesLayout->addWidget(new QLabel(label), attr_row, 0);
        lineEdit = new QLineEdit();
        connect(lineEdit, &QLineEdit::textChanged, this, slot);
        attributesLayout->addWidget(lineEdit, attr_row, 1);
        attr_row++;
    };

    addAttributeLineEdit(groundSpeedLineEdit, tr("Ground Speed:"), &MainWindow::onGroundSpeedChanged);
    addAttributeLineEdit(lightLevelLineEdit, tr("Light Level:"), &MainWindow::onLightLevelChanged);
    addAttributeLineEdit(lightColorLineEdit, tr("Light Color:"), &MainWindow::onLightColorChanged);
    addAttributeLineEdit(minimapColorLineEdit, tr("Minimap Color:"), &MainWindow::onMinimapColorChanged);
    addAttributeLineEdit(maxReadCharsLineEdit, tr("Max Read Chars:"), &MainWindow::onMaxReadCharsChanged);
    addAttributeLineEdit(maxReadWriteCharsLineEdit, tr("Max R/W Chars:"), &MainWindow::onMaxReadWriteCharsChanged);
    addAttributeLineEdit(wareIdLineEdit, tr("Ware ID:"), &MainWindow::onWareIdChanged);

    attributesLayout->setRowStretch(attr_row, 1);
    attributesGroupBox->setMinimumWidth(350);
    topRightLayout->addWidget(attributesGroupBox,1);

    rightPanelLayout->addLayout(topRightLayout);

    outputLogView = new QTextEdit();
    outputLogView->setReadOnly(true);
    rightPanelLayout->addWidget(outputLogView, 1);

    QHBoxLayout* bottomStatusLayout = new QHBoxLayout();
    itemsCountLabel = new QLabel(tr("0 Items"));
    loadingProgressBar = new QProgressBar();
    loadingProgressBar->setVisible(false);
    bottomStatusLayout->addWidget(itemsCountLabel);
    bottomStatusLayout->addSpacing(10);
    bottomStatusLayout->addWidget(loadingProgressBar);
    bottomStatusLayout->addStretch(1);
    rightPanelLayout->addLayout(bottomStatusLayout);

    mainLayout->addWidget(leftPanelWidget);
    mainLayout->addLayout(rightPanelLayout, 1);

    setCentralWidget(mainWidget);
    appearanceGroupBox->setEnabled(false);
    attributesGroupBox->setEnabled(false);
}

void MainWindow::createDockWidgets()
{
}

// --- Action Slot Implementations ---
void MainWindow::newFile()
{
    if (maybeSave()) {
        currentOtbItems.clear();
        serverItemListBox->clear();
        listItemToServerItemMap.clear();
        if(currentPlugin) {
            currentPlugin->unloadClient();
            currentPlugin = nullptr;
        }
        clearItemDetailsView();
        setCurrentFile(QString());

        isModified = false;
        setWindowModified(false);

        if (!pluginManager || pluginManager->availablePlugins().isEmpty()) {
            QMessageBox::warning(this, tr("New OTB"), tr("No plugins available to determine client versions. Cannot create new OTB."));
            return;
        }
        IPlugin* chosenPlugin = pluginManager->availablePlugins().first();
        if (chosenPlugin->getSupportedClients().isEmpty()) {
            QMessageBox::warning(this, tr("New OTB"), tr("Selected plugin (%1) has no defined client versions.").arg(chosenPlugin->pluginName()));
            return;
        }

        QStringList clientDescriptions;
        for(const auto& sc : chosenPlugin->getSupportedClients()) {
            clientDescriptions.append(sc.description);
        }

        bool ok;
        QString chosenDesc = QInputDialog::getItem(this, tr("Select Client Version"),
                                                   tr("Choose a client version for the new OTB:"),
                                                   clientDescriptions, 0, false, &ok);
        if (ok && !chosenDesc.isEmpty()) {
            const OTB::SupportedClient* selectedSc = nullptr;
            for(const auto& sc : chosenPlugin->getSupportedClients()) {
                if (sc.description == chosenDesc) {
                    selectedSc = &sc;
                    break;
                }
            }

            if (selectedSc) {
                currentOtbItems.majorVersion = 3;
                currentOtbItems.minorVersion = selectedSc->otbVersion;
                currentOtbItems.buildNumber = 1;
                currentOtbItems.clientVersion = selectedSc->version;
                currentOtbItems.description = QString("OTB for Tibia Client %1").arg(selectedSc->description);

                OTB::ServerItem defaultItem;
                defaultItem.id = 100;
                defaultItem.clientId = 100;
                defaultItem.name = "New Item";
                defaultItem.type = OTB::ServerItemType::None;
                defaultItem.updateFlagsFromProperties();
                currentOtbItems.add(defaultItem);

                OTB::ServerItem* sItemPtr = &currentOtbItems.items[0];
                QListWidgetItem *listItemWidget = new QListWidgetItem(QString("[%1] %2").arg(sItemPtr->id).arg(sItemPtr->name), serverItemListBox);
                listItemToServerItemMap.insert(listItemWidget, sItemPtr);
                serverItemListBox->setCurrentRow(0);

                itemsCountLabel->setText(tr("%1 Items").arg(currentOtbItems.items.count()));
                statusBar()->showMessage(tr("New OTB created for %1. Save to keep changes.").arg(selectedSc->description), 5000);

                saveAct->setEnabled(true);
                saveAsAct->setEnabled(true);
                editMenu->setEnabled(true);
                viewMenu->setEnabled(true);
                toolsMenu->setEnabled(true);
                createItemAct->setEnabled(true);
                findItemAct->setEnabled(true);

                QString errorStr;
                if (chosenPlugin->loadClient(*selectedSc, ".", true, true, true, errorStr)) {
                    currentPlugin = chosenPlugin;
                } else {
                    QMessageBox::warning(this, tr("Plugin Error"), tr("Could not load client %1 with %2 for new OTB:\n%3")
                                         .arg(selectedSc->description, chosenPlugin->pluginName(), errorStr));
                }
                 updateItemDetailsView(sItemPtr);

            } else {
                 QMessageBox::critical(this, tr("Error"), tr("Selected client description not found."));
            }
        } else {
            statusBar()->showMessage(tr("New OTB creation cancelled."), 2000);
        }
        saveAct->setEnabled(true);
        saveAsAct->setEnabled(true);
    }
}

void MainWindow::openFile()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this,
                                   tr("Open OTB File"), currentFile.isEmpty() ? QDir::homePath() : QFileInfo(currentFile).path(),
                                   tr("OTB Files (*.otb);;All Files (*)"));
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

bool MainWindow::saveFile()
{
    if (currentFile.isEmpty()) {
        return saveFileAs();
    } else {
        if (isModified) {
            return saveFile(currentFile);
        }
        return true;
    }
}

bool MainWindow::saveFileAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save OTB As"),
                                                    currentFile.isEmpty() ? QDir::homePath() : QFileInfo(currentFile).path(),
                                                    tr("OTB Files (*.otb);;All Files (*)"));
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

void MainWindow::showPreferences() {
    if (pluginManager && !pluginManager->availablePlugins().isEmpty()) {
        IPlugin* selectedPlugin = pluginManager->availablePlugins().first();
        if (!selectedPlugin->getSupportedClients().isEmpty()) {
            const OTB::SupportedClient& clientToLoad = selectedPlugin->getSupportedClients().first();
            QString errorStr;
            QString dummyClientPath = ".";

            if (currentPlugin && currentPlugin->isClientLoaded() && currentPlugin->getCurrentLoadedClient().version == clientToLoad.version) {
                 QMessageBox::information(this, tr("Preferences"), tr("Client %1 is already loaded.").arg(clientToLoad.description));
                return;
            }

            if (selectedPlugin->loadClient(clientToLoad, dummyClientPath, true, true, true, errorStr)) {
                currentPlugin = selectedPlugin;
                statusBar()->showMessage(tr("Client %1 loaded via %2").arg(clientToLoad.description, currentPlugin->pluginName()), 5000);
                 if (currentSelectedItem) {
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

void MainWindow::createNewItem()
{
    if (currentFile.isEmpty() && currentOtbItems.items.isEmpty()) {
        QMessageBox::warning(this, tr("Create Item"), tr("Please open or create a new OTB file first."));
        return;
    }

    OTB::ServerItem newItem;
    quint16 newServerId = 100;
    if (!currentOtbItems.items.isEmpty()) {
        newServerId = currentOtbItems.maxId + 1;
         if (newServerId < 100) newServerId = 100;
    }
    bool idExists = true;
    while(idExists){
        idExists = false;
        for(const auto& existingItem : currentOtbItems.items){
            if(existingItem.id == newServerId){
                newServerId++;
                idExists = true;
                break;
            }
        }
    }

    newItem.id = newServerId;
    newItem.clientId = 100;
    newItem.name = tr("New Item %1").arg(newItem.id);
    newItem.type = OTB::ServerItemType::None;
    newItem.movable = true;
    newItem.updateFlagsFromProperties();

    currentOtbItems.add(newItem);

    OTB::ServerItem* addedItemPtr = nullptr;
    for(int i=0; i < currentOtbItems.items.count(); ++i) {
        if (currentOtbItems.items[i].id == newItem.id) {
            addedItemPtr = &currentOtbItems.items[i];
            break;
        }
    }
    if (!addedItemPtr) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to get pointer to newly added item."));
        currentOtbItems.items.removeLast();
        return;
    }

    QListWidgetItem *listItemWidget = new QListWidgetItem(QString("[%1] %2").arg(addedItemPtr->id).arg(addedItemPtr->name), serverItemListBox);
    listItemToServerItemMap.insert(listItemWidget, addedItemPtr);
    serverItemListBox->setCurrentItem(listItemWidget);

    isModified = true;
    setWindowModified(true);
    itemsCountLabel->setText(tr("%1 Items").arg(currentOtbItems.items.count()));
    statusBar()->showMessage(tr("Created new item ID %1").arg(newItem.id), 3000);
}

void MainWindow::duplicateCurrentItem()
{
    if (!currentSelectedItem) {
        QMessageBox::warning(this, tr("Duplicate Item"), tr("Please select an item to duplicate."));
        return;
    }
    OTB::ServerItem duplicatedItem = *currentSelectedItem;
    quint16 newServerId = currentOtbItems.maxId + 1;
    if (newServerId < 100) newServerId = 100;
    bool idExists = true;
    while(idExists){
        idExists = false;
        for(const auto& existingItem : currentOtbItems.items){
            if(existingItem.id == newServerId){
                newServerId++;
                idExists = true;
                break;
            }
        }
    }
    duplicatedItem.id = newServerId;
    currentOtbItems.add(duplicatedItem);
    OTB::ServerItem* addedItemPtr = nullptr;
    for(int i=0; i < currentOtbItems.items.count(); ++i) {
        if (currentOtbItems.items[i].id == duplicatedItem.id) {
            addedItemPtr = &currentOtbItems.items[i];
            break;
        }
    }
    if (!addedItemPtr) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to get pointer to duplicated item."));
        currentOtbItems.items.removeLast();
        return;
    }
    QListWidgetItem *listItemWidget = new QListWidgetItem(QString("[%1] %2").arg(addedItemPtr->id).arg(addedItemPtr->name), serverItemListBox);
    listItemToServerItemMap.insert(listItemWidget, addedItemPtr);
    serverItemListBox->setCurrentItem(listItemWidget);
    isModified = true;
    setWindowModified(true);
    itemsCountLabel->setText(tr("%1 Items").arg(currentOtbItems.items.count()));
    statusBar()->showMessage(tr("Duplicated item ID %1 to new ID %2").arg(currentSelectedItem->id).arg(duplicatedItem.id), 3000);
}

void MainWindow::reloadCurrentItem()
{
    if (!currentSelectedItem) {
        QMessageBox::warning(this, tr("Reload Item"), tr("Please select an item to reload."));
        return;
    }
    if (!currentPlugin || !currentPlugin->isClientLoaded()) {
        QMessageBox::information(this, tr("Reload Item"), tr("No client data loaded. Please load a client via Preferences."));
        return;
    }
    OTB::ClientItem clientItemData;
    if (!currentPlugin->getClientItem(currentSelectedItem->clientId, clientItemData)) {
        QMessageBox::warning(this, tr("Reload Item"), tr("Could not find client data for Client ID %1.").arg(currentSelectedItem->clientId));
        return;
    }
    quint16 originalServerId = currentSelectedItem->id;
    currentSelectedItem->copyPropertiesFrom(clientItemData);
    currentSelectedItem->id = originalServerId;
    currentSelectedItem->clientId = clientItemData.id;
    if (clientItemData.spriteHash.size() == 16) {
        currentSelectedItem->spriteHash = clientItemData.spriteHash;
    } else if (!clientItemData.spriteHash.isEmpty()) {
        qWarning() << "ClientItem ID" << clientItemData.id << "has an invalid spriteHash size during reload:" << clientItemData.spriteHash.size();
    }
    currentSelectedItem->updateFlagsFromProperties();
    isModified = true;
    setWindowModified(true);
    updateItemDetailsView(currentSelectedItem);
    statusBar()->showMessage(tr("Item ID %1 reloaded from client data.").arg(currentSelectedItem->id), 3000);
}

void MainWindow::findItem() { QMessageBox::information(this, "Not Implemented", "Find Item placeholder."); }

void MainWindow::createMissingItems()
{
    if (!currentPlugin || !currentPlugin->isClientLoaded()) {
        QMessageBox::information(this, tr("Create Missing Items"), tr("No client data loaded. Please load a client via Preferences."));
        return;
    }
    if (currentOtbItems.items.isEmpty() && currentFile.isEmpty()) {
         QMessageBox::warning(this, tr("Create Missing Items"), tr("Please open or create an OTB file first."));
        return;
    }
    const QMap<quint16, OTB::ClientItem>& clientItemsMap = currentPlugin->getClientItems();
    if (clientItemsMap.isEmpty()) {
        QMessageBox::information(this, tr("Create Missing Items"), tr("The loaded client data contains no items."));
        return;
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);
    int itemsCreatedCount = 0;
    quint16 nextServerId = currentOtbItems.items.isEmpty() ? 100 : (currentOtbItems.maxId + 1);
    if (nextServerId < 100) nextServerId = 100;

    for (const OTB::ClientItem& clientItem : clientItemsMap) {
        bool serverItemExists = false;
        for (const OTB::ServerItem& serverItem : currentOtbItems.items) {
            if (serverItem.clientId == clientItem.id) {
                serverItemExists = true;
                break;
            }
        }
        if (!serverItemExists) {
            bool idCollision = true;
            while(idCollision){
                idCollision = false;
                for(const auto& existingItem : currentOtbItems.items){
                    if(existingItem.id == nextServerId){
                        nextServerId++;
                        idCollision = true;
                        break;
                    }
                }
            }
            OTB::ServerItem newItem;
            newItem.id = nextServerId++;
            newItem.clientId = clientItem.id;
            newItem.copyPropertiesFrom(clientItem);
            if (clientItem.spriteHash.size() == 16) {
                newItem.spriteHash = clientItem.spriteHash;
            }
            newItem.updateFlagsFromProperties();
            currentOtbItems.add(newItem);
            OTB::ServerItem* addedItemPtr = nullptr;
            for(int i=0; i < currentOtbItems.items.count(); ++i) {
                if (currentOtbItems.items[i].id == newItem.id) {
                    addedItemPtr = &currentOtbItems.items[i];
                    break;
                }
            }
            if (addedItemPtr) {
                 QListWidgetItem *listItemWidget = new QListWidgetItem(QString("[%1] %2").arg(addedItemPtr->id).arg(addedItemPtr->name), serverItemListBox);
                 listItemToServerItemMap.insert(listItemWidget, addedItemPtr);
            }
            itemsCreatedCount++;
        }
    }
    QApplication::restoreOverrideCursor();
    if (itemsCreatedCount > 0) {
        isModified = true;
        setWindowModified(true);
        itemsCountLabel->setText(tr("%1 Items").arg(currentOtbItems.items.count()));
        statusBar()->showMessage(tr("Created %1 missing items.").arg(itemsCreatedCount), 3000);
        if (serverItemListBox->count() > 0 && !currentSelectedItem) {
             serverItemListBox->setCurrentRow(0);
        }
    } else {
        statusBar()->showMessage(tr("No missing items found to create."), 3000);
    }
}

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

// --- UI Update Slots ---
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
        duplicateItemAct->setEnabled(true);
        reloadItemAct->setEnabled(true);
        duplicateItemButtonMain->setEnabled(true);
        reloadItemButtonMain->setEnabled(true);
        appearanceGroupBox->setEnabled(true);
        attributesGroupBox->setEnabled(true);
    } else {
        currentSelectedItem = nullptr;
        clearItemDetailsView();
    }
}

void MainWindow::clearItemDetailsView() {
    loadingItemDetails = true;
    serverIDLabel_val->setText("0");
    clientIDSpinBox->setValue(0);
    itemNameLineEdit->clear();
    itemTypeComboBox->setCurrentIndex(-1);
    stackOrderComboBox->setCurrentIndex(-1);
    mainClientItemViewWidget->setClientItem(nullptr);
    previousClientItemViewWidget->setClientItem(nullptr);
    appearanceGroupBox->setEnabled(false);
    attributesGroupBox->setEnabled(false);
    duplicateItemAct->setEnabled(false);
    reloadItemAct->setEnabled(false);
    duplicateItemButtonMain->setEnabled(false);
    reloadItemButtonMain->setEnabled(false);
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
    groundSpeedLineEdit->clear();
    lightLevelLineEdit->clear();
    lightColorLineEdit->clear();
    minimapColorLineEdit->clear();
    maxReadCharsLineEdit->clear();
    maxReadWriteCharsLineEdit->clear();
    wareIdLineEdit->clear();
    // Reset styles of all editable fields
    itemNameLineEdit->setStyleSheet("");
    itemTypeComboBox->setStyleSheet("");
    stackOrderComboBox->setStyleSheet("");
    unpassableCheckBox->setStyleSheet(""); // Checkboxes might not show text color, but good practice
    movableCheckBox->setStyleSheet("");
    blockMissilesCheckBox->setStyleSheet("");
    // ... for all checkboxes and lineedits ...
    groundSpeedLineEdit->setStyleSheet("");
    lightLevelLineEdit->setStyleSheet("");
    // ... etc.
    loadingItemDetails = false;
}

void MainWindow::currentServerItemChanged(OTB::ServerItem* item) {
    currentSelectedItem = item;
    updateItemDetailsView(item);
}

void MainWindow::updateItemDetailsView(OTB::ServerItem* item) {
    loadingItemDetails = true;
    if (!item) {
        clearItemDetailsView();
        loadingItemDetails = false;
        return;
    }
    serverIDLabel_val->setText(QString::number(item->id));
    clientIDSpinBox->setValue(item->clientId);
    itemNameLineEdit->setText(item->name);
    if (itemTypeComboBox->count() == 0) {
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
        stackOrderComboBox->addItem(tr("None"), QVariant::fromValue(static_cast<int>(OTB::TileStackOrder::None)));
        stackOrderComboBox->addItem(tr("Border"), QVariant::fromValue(static_cast<int>(OTB::TileStackOrder::Border)));
        stackOrderComboBox->addItem(tr("Ground"), QVariant::fromValue(static_cast<int>(OTB::TileStackOrder::Ground)));
        stackOrderComboBox->addItem(tr("Bottom"), QVariant::fromValue(static_cast<int>(OTB::TileStackOrder::Bottom)));
        stackOrderComboBox->addItem(tr("Top"), QVariant::fromValue(static_cast<int>(OTB::TileStackOrder::Top)));
        stackOrderComboBox->addItem(tr("Creature"), QVariant::fromValue(static_cast<int>(OTB::TileStackOrder::Creature)));
    }
    int stackOrderIndex = stackOrderComboBox->findData(QVariant::fromValue(static_cast<int>(item->stackOrder)));
    stackOrderComboBox->setCurrentIndex(stackOrderIndex);
    item->updatePropertiesFromFlags();
    unpassableCheckBox->setChecked(item->unpassable);
    movableCheckBox->setChecked(item->movable);
    blockMissilesCheckBox->setChecked(item->blockMissiles);
    hasElevationCheckBox->setChecked(item->hasElevation);
    forceUseCheckBox->setChecked(item->forceUse);
    multiUseCheckBox->setChecked(item->multiUse);
    pickupableCheckBox->setChecked(item->pickupable);
    stackableCheckBox->setChecked(item->stackable);
    readableCheckBox->setChecked(item->readable);
    rotatableCheckBox->setChecked(item->rotatable);
    hangableCheckBox->setChecked(item->hangable);
    hookSouthCheckBox->setChecked(item->hookSouth);
    hookEastCheckBox->setChecked(item->hookEast);
    ignoreLookCheckBox->setChecked(item->ignoreLook);
    fullGroundCheckBox->setChecked(item->fullGround);
    groundSpeedLineEdit->setText(QString::number(item->groundSpeed));
    lightLevelLineEdit->setText(QString::number(item->lightLevel));
    lightColorLineEdit->setText(QString::number(item->lightColor));
    minimapColorLineEdit->setText(QString::number(item->minimapColor));
    maxReadCharsLineEdit->setText(QString::number(item->maxReadChars));
    maxReadWriteCharsLineEdit->setText(QString::number(item->maxReadWriteChars));
    wareIdLineEdit->setText(QString::number(item->tradeAs));
    OTB::ClientItem clientItemForView;
    bool clientDataForViewAvailable = false;
    if (currentPlugin && currentPlugin->isClientLoaded() && item) {
        if (currentPlugin->getClientItem(item->clientId, clientItemForView)) {
            clientDataForViewAvailable = true;
            qDebug() << "Selected ServerItem ID:" << item->id << " (ClientID:" << item->clientId << ")";
            qDebug() << "Corresponding ClientItem Name (from plugin):" << clientItemForView.name;
            mainClientItemViewWidget->setClientItem(&clientItemForView);
        } else {
            qDebug() << "ClientItem with ID" << item->clientId << "not found in current plugin.";
            mainClientItemViewWidget->setClientItem(nullptr);
        }
    } else {
        mainClientItemViewWidget->setClientItem(nullptr);
    }
    previousClientItemViewWidget->setClientItem(nullptr);
    auto setStyleIfDifferent = [&](QWidget* control, const auto& serverValue, const auto& clientValue) {
        if (clientDataForViewAvailable && serverValue != clientValue) {
            control->setStyleSheet("color: red;");
        } else {
            control->setStyleSheet("");
        }
    };
    setStyleIfDifferent(itemNameLineEdit, item->name, clientItemForView.name);
    setStyleIfDifferent(itemTypeComboBox, item->type, clientItemForView.type);
    setStyleIfDifferent(stackOrderComboBox, item->stackOrder, clientItemForView.stackOrder);
    setStyleIfDifferent(unpassableCheckBox, item->unpassable, clientItemForView.unpassable);
    setStyleIfDifferent(movableCheckBox, item->movable, clientItemForView.movable);
    setStyleIfDifferent(blockMissilesCheckBox, item->blockMissiles, clientItemForView.blockMissiles);
    setStyleIfDifferent(blockPathfinderCheckBox, item->blockPathfinder, clientItemForView.blockPathfinder);
    setStyleIfDifferent(hasElevationCheckBox, item->hasElevation, clientItemForView.hasElevation);
    setStyleIfDifferent(forceUseCheckBox, item->forceUse, clientItemForView.forceUse);
    setStyleIfDifferent(multiUseCheckBox, item->multiUse, clientItemForView.multiUse);
    setStyleIfDifferent(pickupableCheckBox, item->pickupable, clientItemForView.pickupable);
    setStyleIfDifferent(stackableCheckBox, item->stackable, clientItemForView.stackable);
    setStyleIfDifferent(readableCheckBox, item->readable, clientItemForView.readable);
    setStyleIfDifferent(rotatableCheckBox, item->rotatable, clientItemForView.rotatable);
    setStyleIfDifferent(hangableCheckBox, item->hangable, clientItemForView.hangable);
    setStyleIfDifferent(hookSouthCheckBox, item->hookSouth, clientItemForView.hookSouth);
    setStyleIfDifferent(hookEastCheckBox, item->hookEast, clientItemForView.hookEast);
    setStyleIfDifferent(ignoreLookCheckBox, item->ignoreLook, clientItemForView.ignoreLook);
    setStyleIfDifferent(fullGroundCheckBox, item->fullGround, clientItemForView.fullGround);
    setStyleIfDifferent(groundSpeedLineEdit, item->groundSpeed, clientItemForView.groundSpeed);
    setStyleIfDifferent(lightLevelLineEdit, item->lightLevel, clientItemForView.lightLevel);
    setStyleIfDifferent(lightColorLineEdit, item->lightColor, clientItemForView.lightColor);
    setStyleIfDifferent(minimapColorLineEdit, item->minimapColor, clientItemForView.minimapColor);
    setStyleIfDifferent(maxReadCharsLineEdit, item->maxReadChars, clientItemForView.maxReadChars);
    setStyleIfDifferent(maxReadWriteCharsLineEdit, item->maxReadWriteChars, clientItemForView.maxReadWriteChars);
    setStyleIfDifferent(wareIdLineEdit, item->tradeAs, clientItemForView.tradeAs);
    if (clientDataForViewAvailable && item->spriteHash != clientItemForView.spriteHash) {
         mainClientItemViewWidget->setStyleSheet("border: 1px solid red;");
    } else {
         mainClientItemViewWidget->setStyleSheet("");
    }
    loadingItemDetails = false;
}

void MainWindow::updateClientItemView(OTB::ClientItem* clientItem) {
    mainClientItemViewWidget->setClientItem(clientItem);
}
void MainWindow::updatePreviousClientItemView(OTB::ClientItem* prevClientItem) {
    previousClientItemViewWidget->setClientItem(prevClientItem);
}

// --- Helper for styling ---
void MainWindow::updatePropertyStyle(QWidget* control, const std::function<bool(const OTB::ClientItem&)>& comparisonLambda) {
    if (!currentPlugin || !currentPlugin->isClientLoaded() || !currentSelectedItem) {
        control->setStyleSheet("");
        return;
    }
    OTB::ClientItem clientItemData;
    if (currentPlugin->getClientItem(currentSelectedItem->clientId, clientItemData)) {
        if (comparisonLambda(clientItemData)) {
            control->setStyleSheet("color: red;");
        } else {
            control->setStyleSheet("");
        }
    } else {
        control->setStyleSheet("");
    }
}


// --- Slots for Item Property Changes ---
void MainWindow::onClientIdChanged(int value) {
    if (currentSelectedItem && !loadingItemDetails) {
        currentSelectedItem->clientId = static_cast<quint16>(value);
        isModified = true;
        setWindowModified(isModified);
        OTB::ClientItem cItem; // For style update
        bool clientDataAvailable = currentPlugin && currentPlugin->isClientLoaded() && currentPlugin->getClientItem(currentSelectedItem->clientId, cItem);
        // ClientID itself isn't styled based on difference, but changing it might affect sprite view
        if (clientDataAvailable) {
            mainClientItemViewWidget->setClientItem(&cItem);
             // Re-check sprite hash match after ClientID change
            if (currentSelectedItem->spriteHash != cItem.spriteHash) {
                 mainClientItemViewWidget->setStyleSheet("border: 1px solid red;");
            } else {
                 mainClientItemViewWidget->setStyleSheet("");
            }
        } else {
            mainClientItemViewWidget->setClientItem(nullptr);
            mainClientItemViewWidget->setStyleSheet("");
        }
    }
}

void MainWindow::onItemNameChanged(const QString& text) {
    if (currentSelectedItem && !loadingItemDetails) {
        currentSelectedItem->name = text;
        isModified = true;
        setWindowModified(isModified);
        QListWidgetItem* listItem = serverItemListBox->currentItem();
        if (listItem) {
            listItem->setText(QString("[%1] %2").arg(currentSelectedItem->id).arg(currentSelectedItem->name));
        }
        updatePropertyStyle(itemNameLineEdit, [&](const OTB::ClientItem& cItem){ return currentSelectedItem->name != cItem.name; });
    }
}

void MainWindow::onItemTypeChanged(int index) {
    if (currentSelectedItem && !loadingItemDetails && index >=0) {
        QVariant typeData = itemTypeComboBox->itemData(index);
        if (typeData.isValid()) {
            currentSelectedItem->type = static_cast<OTB::ServerItemType>(typeData.toInt());
            isModified = true;
            setWindowModified(isModified);
            updatePropertyStyle(itemTypeComboBox, [&](const OTB::ClientItem& cItem){ return currentSelectedItem->type != cItem.type; });
        }
    }
}

void MainWindow::onStackOrderChanged(int index) {
    if (currentSelectedItem && !loadingItemDetails && index >=0) {
        QVariant stackData = stackOrderComboBox->itemData(index);
        if (stackData.isValid()) {
            OTB::TileStackOrder newOrder = static_cast<OTB::TileStackOrder>(stackData.toInt());
            currentSelectedItem->stackOrder = newOrder;
            currentSelectedItem->hasStackOrder = (newOrder != OTB::TileStackOrder::None);
            isModified = true;
            setWindowModified(isModified);
            updatePropertyStyle(stackOrderComboBox, [&](const OTB::ClientItem& cItem){ return currentSelectedItem->stackOrder != cItem.stackOrder; });
        }
    }
}

#define IMPLEMENT_FLAG_SLOT(SLOT_NAME, CONTROL_WIDGET, ITEM_PROPERTY, ITEM_FLAG) \
void MainWindow::SLOT_NAME(bool checked) { \
    if (currentSelectedItem && !loadingItemDetails) { \
        currentSelectedItem->ITEM_PROPERTY = checked; \
        isModified = true; \
        setWindowModified(isModified); \
        updatePropertyStyle(CONTROL_WIDGET, [&](const OTB::ClientItem& cItem){ return currentSelectedItem->ITEM_PROPERTY != cItem.ITEM_PROPERTY; }); \
    } \
}

IMPLEMENT_FLAG_SLOT(onUnpassableChanged, unpassableCheckBox, unpassable, OTB::ServerItemFlag::Unpassable)
IMPLEMENT_FLAG_SLOT(onBlockMissilesChanged, blockMissilesCheckBox, blockMissiles, OTB::ServerItemFlag::BlockMissiles)
IMPLEMENT_FLAG_SLOT(onBlockPathfinderChanged, blockPathfinderCheckBox, blockPathfinder, OTB::ServerItemFlag::BlockPathfinder)
IMPLEMENT_FLAG_SLOT(onHasElevationChanged, hasElevationCheckBox, hasElevation, OTB::ServerItemFlag::HasElevation)
IMPLEMENT_FLAG_SLOT(onForceUseChanged, forceUseCheckBox, forceUse, OTB::ServerItemFlag::ForceUse)
IMPLEMENT_FLAG_SLOT(onMultiUseChanged, multiUseCheckBox, multiUse, OTB::ServerItemFlag::MultiUse)
IMPLEMENT_FLAG_SLOT(onPickupableChanged, pickupableCheckBox, pickupable, OTB::ServerItemFlag::Pickupable)
IMPLEMENT_FLAG_SLOT(onMovableChanged, movableCheckBox, movable, OTB::ServerItemFlag::Movable)
IMPLEMENT_FLAG_SLOT(onStackableChanged, stackableCheckBox, stackable, OTB::ServerItemFlag::Stackable)
IMPLEMENT_FLAG_SLOT(onReadableChanged, readableCheckBox, readable, OTB::ServerItemFlag::Readable)
IMPLEMENT_FLAG_SLOT(onRotatableChanged, rotatableCheckBox, rotatable, OTB::ServerItemFlag::Rotatable)
IMPLEMENT_FLAG_SLOT(onHangableChanged, hangableCheckBox, hangable, OTB::ServerItemFlag::Hangable)
IMPLEMENT_FLAG_SLOT(onHookSouthChanged, hookSouthCheckBox, hookSouth, OTB::ServerItemFlag::HookSouth)
IMPLEMENT_FLAG_SLOT(onHookEastChanged, hookEastCheckBox, hookEast, OTB::ServerItemFlag::HookEast)
IMPLEMENT_FLAG_SLOT(onIgnoreLookChanged, ignoreLookCheckBox, ignoreLook, OTB::ServerItemFlag::IgnoreLook)
IMPLEMENT_FLAG_SLOT(onFullGroundChanged, fullGroundCheckBox, fullGround, OTB::ServerItemFlag::FullGround)

#define IMPLEMENT_UINT16_ATTR_SLOT(SLOT_NAME, CONTROL_WIDGET, ITEM_PROPERTY) \
void MainWindow::SLOT_NAME(const QString& text) { \
    if (currentSelectedItem && !loadingItemDetails) { \
        bool ok; \
        quint16 value = text.toUShort(&ok); \
        if (ok) { \
            currentSelectedItem->ITEM_PROPERTY = value; \
            isModified = true; \
            setWindowModified(isModified); \
            updatePropertyStyle(CONTROL_WIDGET, [&](const OTB::ClientItem& cItem){ return currentSelectedItem->ITEM_PROPERTY != cItem.ITEM_PROPERTY; }); \
        } else { \
            CONTROL_WIDGET->setStyleSheet("color: orange;"); /* Indicate parsing error */ \
        }\
    } \
}

IMPLEMENT_UINT16_ATTR_SLOT(onGroundSpeedChanged, groundSpeedLineEdit, groundSpeed)
IMPLEMENT_UINT16_ATTR_SLOT(onLightLevelChanged, lightLevelLineEdit, lightLevel)
IMPLEMENT_UINT16_ATTR_SLOT(onLightColorChanged, lightColorLineEdit, lightColor)
IMPLEMENT_UINT16_ATTR_SLOT(onMinimapColorChanged, minimapColorLineEdit, minimapColor)
IMPLEMENT_UINT16_ATTR_SLOT(onMaxReadCharsChanged, maxReadCharsLineEdit, maxReadChars)
IMPLEMENT_UINT16_ATTR_SLOT(onMaxReadWriteCharsChanged, maxReadWriteCharsLineEdit, maxReadWriteChars)
IMPLEMENT_UINT16_ATTR_SLOT(onWareIdChanged, wareIdLineEdit, tradeAs)

void MainWindow::showSpriteCandidates()
{
    if (!currentPlugin || !currentPlugin->isClientLoaded() || !currentSelectedItem) {
        QMessageBox::information(this, tr("Sprite Candidates"), tr("Please load an OTB and select an item, and ensure a client is active."));
        return;
    }
    QList<const OTB::ClientItem*> candidatesList;
    int count = 0;
    for (const auto& clientItem : currentPlugin->getClientItems()) { // Iterate over const references
        if (clientItem.id != currentSelectedItem->clientId) {
            // Need a const pointer to an item that lives as long as the dialog.
            // The items in plugin's map are suitable.
            candidatesList.append(&clientItem);
            count++;
            if (count >= 5) break; // Show up to 5 candidates like C#
        }
    }
     if (candidatesList.isEmpty()) {
        QMessageBox::information(this, tr("Sprite Candidates"), tr("No other sprite candidates found in the current dummy client data."));
        return;
    }
    SpriteCandidatesDialog dialog(candidatesList, this);
    if (dialog.exec() == QDialog::Accepted) {
        quint16 selectedId = dialog.getSelectedClientId();
        if (selectedId != 0 && currentSelectedItem) {
            currentSelectedItem->clientId = selectedId; // This will trigger onClientIdChanged if spinbox is focused
            isModified = true;
            setWindowModified(true);
            // Manually update the spinbox to reflect the change if it wasn't focused
            loadingItemDetails = true; // Prevent onClientIdChanged from re-triggering logic redundantly
            clientIDSpinBox->setValue(selectedId);
            loadingItemDetails = false;
            updateItemDetailsView(currentSelectedItem);
        }
    }
}

bool MainWindow::loadClientForOtb() {
    if (currentOtbItems.items.isEmpty()) {
        qWarning() << "loadClientForOtb: No OTB items loaded.";
        return false;
    }
    quint32 otbClientVersionTarget = currentOtbItems.minorVersion;
    if (otbClientVersionTarget == 0 && currentOtbItems.clientVersion != 0) {
         otbClientVersionTarget = currentOtbItems.clientVersion;
    }
    IPlugin* foundPlugin = pluginManager->findPluginForOtbVersion(otbClientVersionTarget);
    if (foundPlugin) {
        const OTB::SupportedClient* clientToLoad = nullptr;
        for(const auto& sc : foundPlugin->getSupportedClients()){
            if(sc.otbVersion == otbClientVersionTarget || sc.version == otbClientVersionTarget){
                clientToLoad = &sc;
                break;
            }
        }
        if (clientToLoad) {
            QString errorStr;
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
    default:
        break;
    }
    return true;
}

void MainWindow::loadFile(const QString &fileName)
{
    OTB::OtbReader reader;
    QString errorString;
    currentOtbItems.clear();
    serverItemListBox->clear();
    listItemToServerItemMap.clear();
    clearItemDetailsView();
    if(currentPlugin) {
        currentPlugin->unloadClient();
        currentPlugin = nullptr;
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool success = reader.read(fileName, currentOtbItems, errorString);
    QApplication::restoreOverrideCursor();
    if (success) {
        setCurrentFile(fileName);
        loadClientForOtb();
        for (int i = 0; i < currentOtbItems.items.size(); ++i) {
            OTB::ServerItem* serverItem = &currentOtbItems.items[i];
            QListWidgetItem *listItemWidget = new QListWidgetItem(QString("[%1] %2").arg(serverItem->id).arg(serverItem->name), serverItemListBox);
            listItemToServerItemMap.insert(listItemWidget, serverItem);
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
        setWindowModified(false);
        itemsCountLabel->setText(tr("%1 Items").arg(currentOtbItems.items.count()));
        if (serverItemListBox->count() > 0) {
            serverItemListBox->setCurrentRow(0);
        } else {
            clearItemDetailsView();
        }
    } else {
        QMessageBox::critical(this, tr("Error Loading File"),
                             tr("Could not load file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), errorString));
        statusBar()->showMessage(tr("Error loading file"), 5000);
        saveAct->setEnabled(false);
        saveAsAct->setEnabled(false);
        editMenu->setEnabled(false);
        viewMenu->setEnabled(false);
        toolsMenu->setEnabled(false);
        clearItemDetailsView();
        itemsCountLabel->setText(tr("0 Items"));
    }
}

bool MainWindow::saveFile(const QString &fileName)
{
    OTB::OtbWriter writer;
    QString errorString;
    for(OTB::ServerItem &item : currentOtbItems.items) {
        item.updateFlagsFromProperties();
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool success = writer.write(fileName, currentOtbItems, errorString);
    QApplication::restoreOverrideCursor();
    if (success) {
        setCurrentFile(fileName);
        statusBar()->showMessage(tr("File saved successfully"), 2000);
        return true;
    } else {
        QMessageBox::critical(this, tr("Error Saving File"),
                             tr("Could not save file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), errorString));
        statusBar()->showMessage(tr("Error saving file"), 5000);
        return false;
    }
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
    saveAct->setEnabled(!currentFile.isEmpty());
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

// --- Context Menu for ServerItemListBox ---
void MainWindow::showServerListContextMenu(const QPoint& pos)
{
    QListWidgetItem* listItem = serverItemListBox->itemAt(pos);
    OTB::ServerItem* itemForContext = nullptr;
    if (listItem) {
        auto it = listItemToServerItemMap.find(listItem);
        if (it != listItemToServerItemMap.end()) {
            itemForContext = it.value();
        }
    }
    if (!itemForContext) {
        itemForContext = currentSelectedItem;
    }
    QMenu contextMenu(this);
    QAction *duplicateActCtx = nullptr;
    QAction *reloadActCtx = nullptr;
    QAction *copyServerIdAct = nullptr;
    QAction *copyClientIdAct = nullptr;
    QAction *copyNameAct = nullptr;

    if (itemForContext) {
        // For simplicity, context menu actions operate on currentSelectedItem
        // A more advanced version might pass itemForContext to the slots
        if (currentSelectedItem == itemForContext) { // Ensure currentSelectedItem is the one right-clicked
            duplicateActCtx = contextMenu.addAction(tr("Duplicate Item"));
            connect(duplicateActCtx, &QAction::triggered, this, &MainWindow::duplicateCurrentItem);
            reloadActCtx = contextMenu.addAction(tr("Reload Item"));
            connect(reloadActCtx, &QAction::triggered, this, &MainWindow::reloadCurrentItem);
            contextMenu.addSeparator();
            copyServerIdAct = contextMenu.addAction(tr("Copy Server ID"));
            connect(copyServerIdAct, &QAction::triggered, this, &MainWindow::copyServerId);
            copyClientIdAct = contextMenu.addAction(tr("Copy Client ID"));
            connect(copyClientIdAct, &QAction::triggered, this, &MainWindow::copyClientId);
            copyNameAct = contextMenu.addAction(tr("Copy Name"));
            connect(copyNameAct, &QAction::triggered, this, &MainWindow::copyItemName);
        } else { // Item under cursor is not the currently globally selected one
            // Offer to select it, or just show copy actions for it?
            // For now, let's just offer copy for the item under cursor
            // (This requires copy actions to take an item or use itemForContext somehow)
            // To keep it simple: user must select an item first for full context menu.
            // If right-click doesn't select, this menu might be less useful.
            // A common behavior: right-click selects the item.
            if(listItem) serverItemListBox->setCurrentItem(listItem); // Make right-clicked item current

            // Now currentSelectedItem should be itemForContext
             if (currentSelectedItem) { // Re-check after potential selection change
                duplicateActCtx = contextMenu.addAction(tr("Duplicate Item"));
                connect(duplicateActCtx, &QAction::triggered, this, &MainWindow::duplicateCurrentItem);
                reloadActCtx = contextMenu.addAction(tr("Reload Item"));
                connect(reloadActCtx, &QAction::triggered, this, &MainWindow::reloadCurrentItem);
                contextMenu.addSeparator();
                copyServerIdAct = contextMenu.addAction(tr("Copy Server ID"));
                connect(copyServerIdAct, &QAction::triggered, this, &MainWindow::copyServerId);
                copyClientIdAct = contextMenu.addAction(tr("Copy Client ID"));
                connect(copyClientIdAct, &QAction::triggered, this, &MainWindow::copyClientId);
                copyNameAct = contextMenu.addAction(tr("Copy Name"));
                connect(copyNameAct, &QAction::triggered, this, &MainWindow::copyItemName);
            }
        }
    } else {
        QAction *createActCtx = contextMenu.addAction(tr("Create New Item"));
        connect(createActCtx, &QAction::triggered, this, &MainWindow::createNewItem);
    }
    if (contextMenu.isEmpty()) return;
    contextMenu.exec(serverItemListBox->mapToGlobal(pos));
}

void MainWindow::copyServerId() {
    if (currentSelectedItem) {
        QApplication::clipboard()->setText(QString::number(currentSelectedItem->id));
        statusBar()->showMessage(tr("Server ID %1 copied to clipboard.").arg(currentSelectedItem->id), 2000);
    }
}

void MainWindow::copyClientId() {
    if (currentSelectedItem) {
        QApplication::clipboard()->setText(QString::number(currentSelectedItem->clientId));
        statusBar()->showMessage(tr("Client ID %1 copied to clipboard.").arg(currentSelectedItem->clientId), 2000);
    }
}

void MainWindow::copyItemName() {
    if (currentSelectedItem) {
        QApplication::clipboard()->setText(currentSelectedItem->name);
        statusBar()->showMessage(tr("Item name '%1' copied to clipboard.").arg(currentSelectedItem->name), 2000);
    }
}
