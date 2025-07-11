#include "mainwindow.h"
#include "otb/otbtypes.h"
#include "dialogs/aboutdialog.h"
#include "dialogs/spritecandidatesdialog.h" // Added
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
    // itemsCountLabel and loadingProgressBar are now in createCentralWidget's bottom area
}

void MainWindow::createCentralWidget()
{
    QWidget *mainWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(mainWidget);

    // Left Panel
    QVBoxLayout *leftPanelLayout = new QVBoxLayout();
    serverItemListBox = new QListWidget();
    connect(serverItemListBox, &QListWidget::currentItemChanged, this, &MainWindow::onServerItemSelectionChanged);
    leftPanelLayout->addWidget(serverItemListBox, 1);

    QHBoxLayout* itemButtonsLayout = new QHBoxLayout();
    newItemButtonMain = new QPushButton(tr("New"));
    // connect(newItemButtonMain, &QPushButton::clicked, this, &MainWindow::createNewItem);
    duplicateItemButtonMain = new QPushButton(tr("Duplicate"));
    duplicateItemButtonMain->setEnabled(false);
    // connect(duplicateItemButtonMain, &QPushButton::clicked, this, &MainWindow::duplicateCurrentItem);
    reloadItemButtonMain = new QPushButton(tr("Reload"));
    reloadItemButtonMain->setEnabled(false);
    // connect(reloadItemButtonMain, &QPushButton::clicked, this, &MainWindow::reloadCurrentItem);
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
    connect(candidatesButton, &QPushButton::clicked, this, &MainWindow::showSpriteCandidates); // Connected
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

    if (currentPlugin && currentPlugin->isClientLoaded() && item) {
        OTB::ClientItem clientItem;
        if (currentPlugin->getClientItem(item->clientId, clientItem)) {
            qDebug() << "Selected ServerItem ID:" << item->id << " (ClientID:" << item->clientId << ")";
            qDebug() << "Corresponding ClientItem Name (from plugin):" << clientItem.name;
            mainClientItemViewWidget->setClientItem(&clientItem);
        } else {
            qDebug() << "ClientItem with ID" << item->clientId << "not found in current plugin.";
            mainClientItemViewWidget->setClientItem(nullptr);
        }
    } else {
        mainClientItemViewWidget->setClientItem(nullptr);
    }
    previousClientItemViewWidget->setClientItem(nullptr); // Clear previous for now

    loadingItemDetails = false;
}

void MainWindow::updateClientItemView(OTB::ClientItem* clientItem) {
    mainClientItemViewWidget->setClientItem(clientItem);
}
void MainWindow::updatePreviousClientItemView(OTB::ClientItem* prevClientItem) {
    previousClientItemViewWidget->setClientItem(prevClientItem);
}


// --- Slots for Item Property Changes ---
void MainWindow::onClientIdChanged(int value) {
    if (currentSelectedItem && !loadingItemDetails) {
        currentSelectedItem->clientId = static_cast<quint16>(value);
        isModified = true;
        setWindowModified(isModified);
        if (currentPlugin && currentPlugin->isClientLoaded()) {
            OTB::ClientItem cItem;
            if (currentPlugin->getClientItem(currentSelectedItem->clientId, cItem)) {
                mainClientItemViewWidget->setClientItem(&cItem);
            } else {
                mainClientItemViewWidget->setClientItem(nullptr);
            }
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
    }
}

void MainWindow::onItemTypeChanged(int index) {
    if (currentSelectedItem && !loadingItemDetails && index >=0) {
        QVariant typeData = itemTypeComboBox->itemData(index);
        if (typeData.isValid()) {
            currentSelectedItem->type = static_cast<OTB::ServerItemType>(typeData.toInt());
            isModified = true;
            setWindowModified(isModified);
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
        }
    }
}

#define IMPLEMENT_FLAG_SLOT(SLOT_NAME, ITEM_PROPERTY, ITEM_FLAG) \
void MainWindow::SLOT_NAME(bool checked) { \
    if (currentSelectedItem && !loadingItemDetails) { \
        currentSelectedItem->ITEM_PROPERTY = checked; \
        isModified = true; \
        setWindowModified(isModified); \
    } \
}

IMPLEMENT_FLAG_SLOT(onUnpassableChanged, unpassable, OTB::ServerItemFlag::Unpassable)
IMPLEMENT_FLAG_SLOT(onBlockMissilesChanged, blockMissiles, OTB::ServerItemFlag::BlockMissiles)
IMPLEMENT_FLAG_SLOT(onBlockPathfinderChanged, blockPathfinder, OTB::ServerItemFlag::BlockPathfinder)
IMPLEMENT_FLAG_SLOT(onHasElevationChanged, hasElevation, OTB::ServerItemFlag::HasElevation)
IMPLEMENT_FLAG_SLOT(onForceUseChanged, forceUse, OTB::ServerItemFlag::ForceUse)
IMPLEMENT_FLAG_SLOT(onMultiUseChanged, multiUse, OTB::ServerItemFlag::MultiUse)
IMPLEMENT_FLAG_SLOT(onPickupableChanged, pickupable, OTB::ServerItemFlag::Pickupable)
IMPLEMENT_FLAG_SLOT(onMovableChanged, movable, OTB::ServerItemFlag::Movable)
IMPLEMENT_FLAG_SLOT(onStackableChanged, stackable, OTB::ServerItemFlag::Stackable)
IMPLEMENT_FLAG_SLOT(onReadableChanged, readable, OTB::ServerItemFlag::Readable)
IMPLEMENT_FLAG_SLOT(onRotatableChanged, rotatable, OTB::ServerItemFlag::Rotatable)
IMPLEMENT_FLAG_SLOT(onHangableChanged, hangable, OTB::ServerItemFlag::Hangable)
IMPLEMENT_FLAG_SLOT(onHookSouthChanged, hookSouth, OTB::ServerItemFlag::HookSouth)
IMPLEMENT_FLAG_SLOT(onHookEastChanged, hookEast, OTB::ServerItemFlag::HookEast)
IMPLEMENT_FLAG_SLOT(onIgnoreLookChanged, ignoreLook, OTB::ServerItemFlag::IgnoreLook)
IMPLEMENT_FLAG_SLOT(onFullGroundChanged, fullGround, OTB::ServerItemFlag::FullGround)

#define IMPLEMENT_UINT16_ATTR_SLOT(SLOT_NAME, ITEM_PROPERTY) \
void MainWindow::SLOT_NAME(const QString& text) { \
    if (currentSelectedItem && !loadingItemDetails) { \
        bool ok; \
        quint16 value = text.toUShort(&ok); \
        if (ok) { \
            currentSelectedItem->ITEM_PROPERTY = value; \
            isModified = true; \
            setWindowModified(isModified); \
        } \
    } \
}

IMPLEMENT_UINT16_ATTR_SLOT(onGroundSpeedChanged, groundSpeed)
IMPLEMENT_UINT16_ATTR_SLOT(onLightLevelChanged, lightLevel)
IMPLEMENT_UINT16_ATTR_SLOT(onLightColorChanged, lightColor)
IMPLEMENT_UINT16_ATTR_SLOT(onMinimapColorChanged, minimapColor)
IMPLEMENT_UINT16_ATTR_SLOT(onMaxReadCharsChanged, maxReadChars)
IMPLEMENT_UINT16_ATTR_SLOT(onMaxReadWriteCharsChanged, maxReadWriteChars)
IMPLEMENT_UINT16_ATTR_SLOT(onWareIdChanged, tradeAs)

void MainWindow::showSpriteCandidates()
{
    if (!currentPlugin || !currentPlugin->isClientLoaded() || !currentSelectedItem) {
        QMessageBox::information(this, tr("Sprite Candidates"), tr("Please load an OTB and select an item, and ensure a client is active."));
        return;
    }

    QList<const OTB::ClientItem*> candidatesList;
    // Simulate finding candidates: take up to 3 other items from the dummy plugin
    // In a real scenario, this would involve image similarity search.
    int count = 0;
    for (const auto& clientItem : currentPlugin->getClientItems()) {
        if (clientItem.id != currentSelectedItem->clientId) { // Don't list itself
            candidatesList.append(&clientItem); // Need const OTB::ClientItem*
            count++;
            if (count >= 3) break;
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
            currentSelectedItem->clientId = selectedId;
            isModified = true;
            setWindowModified(true);
            // Refresh the main view for the selected item
            updateItemDetailsView(currentSelectedItem);
            // The clientIDSpinBox should also update via updateItemDetailsView
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
