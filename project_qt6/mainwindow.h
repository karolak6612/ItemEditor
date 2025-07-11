#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "plugins/iplugin.h" // Include IPlugin for PluginManager and currentPlugin

// Forward declarations
QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMenuBar;
class QToolBar;
class QStatusBar;
class QTextEdit;
class QListWidget;
class QGroupBox;
class QLabel;
class QComboBox;
class QSpinBox;
class QCheckBox;
class QPushButton;
class QProgressBar;
QT_END_NAMESPACE

namespace OTB {
    struct ServerItem; // Forward declare from otbtypes.h
    struct ClientItem;
}

// class ClientItemView; // No longer forward-declaring, will include the header
#include "widgets/clientitemview.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // File menu actions
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();
    void showPreferences();
    // Edit menu actions
    void createNewItem();
    void duplicateCurrentItem();
    void reloadCurrentItem();
    void findItem();
    void createMissingItems();
    // View menu actions
    void toggleShowMismatched(bool checked);
    void toggleShowDeprecated(bool checked);
    void updateItemsList();
    // Tools menu actions
    void reloadAllItemAttributes();
    void compareOtbFiles();
    void updateOtbVersion();
    // Help menu actions
    void about();

    // Toolbar button actions (many can reuse menu action slots)
    // ...

    // UI update slots
    void currentServerItemChanged(OTB::ServerItem* item); // When item selection changes in list (conceptual)
    void onServerItemSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous); // Actual slot for QListWidget
    void updateItemDetailsView(OTB::ServerItem* item);
    void updateClientItemView(OTB::ClientItem* clientItem); // For the main sprite view
    void updatePreviousClientItemView(OTB::ClientItem* prevClientItem); // For the 'previous' sprite view

    // Slots for handling item property changes from UI
    void onClientIdChanged(int value);
    void onItemNameChanged(const QString& text);
    void onItemTypeChanged(int index);
    void onStackOrderChanged(int index);
    // Flag checkboxes
    void onUnpassableChanged(bool checked);
    void onBlockMissilesChanged(bool checked);
    void onBlockPathfinderChanged(bool checked);
    void onHasElevationChanged(bool checked);
    void onForceUseChanged(bool checked);
    void onMultiUseChanged(bool checked);
    void onPickupableChanged(bool checked);
    void onMovableChanged(bool checked);
    void onStackableChanged(bool checked);
    void onReadableChanged(bool checked);
    void onRotatableChanged(bool checked);
    void onHangableChanged(bool checked);
    void onHookSouthChanged(bool checked);
    void onHookEastChanged(bool checked);
    void onIgnoreLookChanged(bool checked);
    void onFullGroundChanged(bool checked);
    // Attribute LineEdits/SpinBoxes
    void onGroundSpeedChanged(const QString& text);
    void onLightLevelChanged(const QString& text);
    void onLightColorChanged(const QString& text);
    void onMinimapColorChanged(const QString& text);
    void onMaxReadCharsChanged(const QString& text);
    void onMaxReadWriteCharsChanged(const QString& text);
    void onWareIdChanged(const QString& text);

    void showSpriteCandidates();
    void showServerListContextMenu(const QPoint& pos);
    // Context menu action slots
    void copyServerId();
    void copyClientId();
    void copyItemName();


private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createCentralWidget(); // Main layout of the application
    void createDockWidgets(); // Or just arrange within central widget for now

    bool maybeSave(); // Prompts to save if document is modified
    void loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

    // --- UI Elements ---
    // Menus
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *toolsMenu;
    QMenu *helpMenu;

    // Toolbars
    QToolBar *fileToolBar;
    QToolBar *editToolBar; // Or combine as needed

    // Actions
    // File
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *preferencesAct;
    QAction *exitAct;
    // Edit
    QAction *createItemAct;
    QAction *duplicateItemAct;
    QAction *reloadItemAct;
    QAction *findItemAct;
    QAction *createMissingItemsAct;
    // View
    QAction *showMismatchedAct;
    QAction *showDeprecatedAct;
    QAction *updateItemsListAct;
    // Tools
    QAction *reloadAttributesAct;
    QAction *compareOtbAct;
    QAction *updateVersionAct;
    // Help
    QAction *aboutAct;
    QAction *aboutQtAct; // Standard Qt about dialog

    // Central Widget Components (mirroring C# MainForm)
    QListWidget *serverItemListBox; // Placeholder for custom ServerItemListBox

    // Appearance GroupBox
    QGroupBox *appearanceGroupBox;
    ClientItemView *previousClientItemViewWidget; // Custom widget for previous sprite
    ClientItemView *mainClientItemViewWidget;     // Custom widget for current sprite
    QLabel *serverIDLabel_val; // Value for Server ID
    QSpinBox *clientIDSpinBox;
    QPushButton *candidatesButton; // For sprite candidates

    // Attributes GroupBox
    QGroupBox *attributesGroupBox;
    QComboBox *itemTypeComboBox;
    QComboBox *stackOrderComboBox;
    QLineEdit *itemNameLineEdit;
    // ... many QCheckBoxes for flags ...
    QCheckBox *unpassableCheckBox;
    QCheckBox *blockMissilesCheckBox;
    QCheckBox *blockPathfinderCheckBox;
    QCheckBox *hasElevationCheckBox;
    QCheckBox *forceUseCheckBox;
    QCheckBox *multiUseCheckBox;
    QCheckBox *pickupableCheckBox;
    QCheckBox *movableCheckBox;
    QCheckBox *stackableCheckBox;
    QCheckBox *readableCheckBox;
    QCheckBox *rotatableCheckBox;
    QCheckBox *hangableCheckBox;
    QCheckBox *hookSouthCheckBox;
    QCheckBox *hookEastCheckBox;
    QCheckBox *ignoreLookCheckBox;
    QCheckBox *fullGroundCheckBox;

    // ... QLineEdits for attributes ...
    QLineEdit *groundSpeedLineEdit;
    QLineEdit *lightLevelLineEdit;
    QLineEdit *lightColorLineEdit;
    QLineEdit *minimapColorLineEdit;
    QLineEdit *maxReadCharsLineEdit;
    QLineEdit *maxReadWriteCharsLineEdit;
    QLineEdit *wareIdLineEdit;


    // Bottom area
    QTextEdit *outputLogView;
    QLabel *itemsCountLabel;
    QProgressBar *loadingProgressBar;

    // Buttons below serverItemListBox (from C# form)
    QPushButton *newItemButtonMain;
    QPushButton *duplicateItemButtonMain;
    QPushButton *reloadItemButtonMain;
    QPushButton *findItemButtonMain;


    // --- Data members ---
    QString currentFile;
    bool isModified; // To track unsaved changes

    OTB::ServerItemList currentOtbItems; // The loaded OTB data
    OTB::ServerItem* currentSelectedItem; // Pointer to the currently selected item in currentOtbItems.list
    QMap<QListWidgetItem*, OTB::ServerItem*> listItemToServerItemMap; // Map QListWidgetItems to ServerItems

    PluginManager* pluginManager;
    IPlugin* currentPlugin; // The currently active plugin instance
    bool loadingItemDetails; // Guard flag

    void clearItemDetailsView();
    bool loadClientForOtb(); // Helper to load client data via plugin

    // Helper for styling changed properties
    void updatePropertyStyle(QWidget* control, const std::function<bool(const OTB::ClientItem&)>& comparisonLambda);
    template<typename T_Server, typename T_Client> // Not used yet, but could be for simpler cases
    void updatePropertyStyleSimple(QWidget* control, const T_Server& serverValue, const T_Client& clientValue, bool clientDataAvailable);
};

#endif // MAINWINDOW_H
