#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "otb/otbtypes.h"
#include "otb/item.h"
#include "ui/widgets/clientitemview.h"
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

namespace UI {
namespace Widgets {
    class ClientItemView;
    class ItemPropertyEditor;
    class SpriteBrowser;
}
}

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
    
    // Toolbar customization slots
    void showToolBarContextMenu(const QPoint& pos);
    void customizeToolBar();
    void resetToolBar();
    void toggleToolBarVisibility();


private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createCentralWidget();
    void createDockWidgets();
    void createAppearanceGroup();
    void createAttributesGroup();
    
    // Toolbar customization methods
    void setupToolBarCustomization();
    void saveToolBarState();
    void restoreToolBarState();

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
    QToolBar *mainToolBar;
    QMenu *toolBarContextMenu;
    
    // Actions
    QAction *newAct, *openAct, *saveAct, *saveAsAct, *preferencesAct, *exitAct;
    QAction *createItemAct, *duplicateItemAct, *reloadItemAct, *findItemAct, *createMissingItemsAct;
    QAction *showMismatchedAct, *showDeprecatedAct, *updateItemsListAct; // updateItemsListAct now calls buildFilteredItemsList
    QAction *reloadAttributesAct, *compareOtbAct, *updateVersionAct;
    QAction *aboutAct, *aboutQtAct;
    
    // Toolbar-specific actions
    QAction *saveAsToolBarAct, *compareOtbToolBarAct;
    QAction *customizeToolBarAct, *resetToolBarAct, *toggleToolBarAct;
    // Central Widget Components
    QListWidget *serverItemListBox;
    QGroupBox *appearanceGroupBox, *attributesGroupBox;
    UI::Widgets::ClientItemView *previousClientItemViewWidget, *mainClientItemViewWidget;
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
    bool loadingItemDetails;
    bool m_showOnlyMismatched; // New filter flag
    bool m_showOnlyDeprecated; // New filter flag

    // Advanced UI setup methods
    void setupAdvancedUI();
    void createAdvancedMenus();
    void createAdvancedToolBars();
    void createAdvancedDockWidgets();
    void setupAdvancedConnections();
    
    // Advanced feature methods
    void showAdvancedFindDialog();
    void showSpriteAnalysis();
    void showBatchOperations();
    void analyzeSpriteSignatures();
    void toggleSpriteBrowser();
    void selectItemById(quint16 itemId);
    void updateWindowTitle();
    
    // Advanced slot implementations
    void onItemPropertyChanged();
    void onSpriteSelected(quint32 spriteId);
    void onSpriteAssignmentRequested(quint32 spriteId, ItemEditor::ClientItem* item);
    
    // Helpers
    void clearItemDetailsView();
    bool loadClientForOtb();
    void updatePropertyStyle(QWidget* control, const std::function<bool(const ItemEditor::ClientItem&)>& comparisonLambda);
    void performOtbUpdate(const UpdateOptions& options,
                          const QMap<quint16, ItemEditor::ClientItem>& currentClientItems,
                          const QMap<quint16, ItemEditor::ClientItem>& targetClientItems);

    // Helper for item comparison
    bool compareItems(const OTB::ServerItem* serverItem, const ItemEditor::ClientItem* clientItem, bool compareHash);
    
    // Status bar helper methods
    void updateItemCount(int count);
    void showLoadingProgress(int current, int maximum, const QString& message);
    void hideLoadingProgress();
    void showStatusMessage(const QString& message, int timeout = 0);
    
    // Advanced UI components - TODO: Implement these widgets
    // UI::Widgets::ItemPropertyEditor* m_propertyEditor;
    // UI::Widgets::SpriteBrowser* m_spriteBrowser;
};

#endif // MAINWINDOW_H
