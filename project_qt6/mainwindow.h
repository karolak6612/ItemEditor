#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "plugins/iplugin.h"
#include "widgets/clientitemview.h"
#include <functional> // For std::function

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMenuBar;
class QToolBar;
class QStatusBar;
class QTextEdit;
class QListWidget;
class QListWidgetItem;
class QGroupBox;
class QLabel;
class QComboBox;
class QSpinBox;
class QCheckBox;
class QPushButton;
class QLineEdit;
class QProgressBar;
QT_END_NAMESPACE

namespace OTB {
    struct ServerItem;
    struct ClientItem;
}

struct UpdateOptions; // Forward declaration from updateotbdialog.h

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
    void onShowMismatchedToggled(bool checked);
    void onShowDeprecatedToggled(bool checked);
    void buildFilteredItemsList(); // Renamed from updateItemsList
    // Tools menu actions
    void reloadAllItemAttributes();
    void compareOtbFiles();
    void updateOtbVersion();
    // Help menu actions
    void about();

    // UI update slots
    void onServerItemSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void updateItemDetailsView(OTB::ServerItem* item);

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

    // Other UI slots
    void showSpriteCandidates();
    void showServerListContextMenu(const QPoint& pos);
    void copyServerId();
    void copyClientId();
    void copyItemName();


private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createCentralWidget();
    void createDockWidgets();

    bool maybeSave();
    void loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

    // --- UI Elements ---
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *toolsMenu;
    QMenu *helpMenu;
    QToolBar *fileToolBar;
    // Actions
    QAction *newAct, *openAct, *saveAct, *saveAsAct, *preferencesAct, *exitAct;
    QAction *createItemAct, *duplicateItemAct, *reloadItemAct, *findItemAct, *createMissingItemsAct;
    QAction *showMismatchedAct, *showDeprecatedAct, *updateItemsListAct; // updateItemsListAct now calls buildFilteredItemsList
    QAction *reloadAttributesAct, *compareOtbAct, *updateVersionAct;
    QAction *aboutAct, *aboutQtAct;
    // Central Widget Components
    QListWidget *serverItemListBox;
    QGroupBox *appearanceGroupBox, *attributesGroupBox;
    ClientItemView *previousClientItemViewWidget, *mainClientItemViewWidget;
    QLabel *serverIDLabel_val;
    QSpinBox *clientIDSpinBox;
    QPushButton *candidatesButton;
    QComboBox *itemTypeComboBox, *stackOrderComboBox;
    QLineEdit *itemNameLineEdit, *groundSpeedLineEdit, *lightLevelLineEdit, *lightColorLineEdit, *minimapColorLineEdit, *maxReadCharsLineEdit, *maxReadWriteCharsLineEdit, *wareIdLineEdit;
    QCheckBox *unpassableCheckBox, *blockMissilesCheckBox, *blockPathfinderCheckBox, *hasElevationCheckBox, *forceUseCheckBox, *multiUseCheckBox, *pickupableCheckBox, *movableCheckBox, *stackableCheckBox, *readableCheckBox, *rotatableCheckBox, *hangableCheckBox, *hookSouthCheckBox, *hookEastCheckBox, *ignoreLookCheckBox, *fullGroundCheckBox;
    QTextEdit *outputLogView;
    QLabel *itemsCountLabel;
    QProgressBar *loadingProgressBar;
    QPushButton *newItemButtonMain, *duplicateItemButtonMain, *reloadItemButtonMain, *findItemButtonMain;

    // --- Data members ---
    QString currentFile;
    bool isModified;
    OTB::ServerItemList currentOtbItems;
    OTB::ServerItem* currentSelectedItem;
    QMap<QListWidgetItem*, OTB::ServerItem*> listItemToServerItemMap;
    PluginManager* pluginManager;
    IPlugin* currentPlugin;
    bool loadingItemDetails;
    bool m_showOnlyMismatched; // New filter flag
    bool m_showOnlyDeprecated; // New filter flag

    // Helpers
    void clearItemDetailsView();
    bool loadClientForOtb();
    void updatePropertyStyle(QWidget* control, const std::function<bool(const OTB::ClientItem&)>& comparisonLambda);
    void performOtbUpdate(const UpdateOptions& options,
                          const QMap<quint16, OTB::ClientItem>& currentClientItems,
                          const QMap<quint16, OTB::ClientItem>& targetClientItems);

    // Helper for item comparison
    bool compareItems(const OTB::ServerItem* serverItem, const OTB::ClientItem* clientItem, bool compareHash);
};

#endif // MAINWINDOW_H
