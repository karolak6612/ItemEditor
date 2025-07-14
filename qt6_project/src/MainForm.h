/**
 * Item Editor Qt6 - Main Application Window Header
 * Exact mirror of Legacy_App/csharp/Source/MainForm.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_MAINFORM_H
#define ITEMEDITOR_MAINFORM_H

#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>
#include <QSplitter>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QTimer>
#include <QSettings>

// Forward declarations
QT_BEGIN_NAMESPACE
class QListWidget;
class QTreeWidget;
class QTreeWidgetItem;
class QSpinBox;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QTextEdit;
QT_END_NAMESPACE

// ItemEditor includes
#include "Controls/ServerItemListBox.h"
#include "Controls/ClientItemView.h"
#include "Controls/FlagCheckBox.h"
#include "Host/PluginServices.h"
#include "PluginInterface/SpriteManager.h"
#include "Host/Plugin.h"
#include "PluginInterface/IPlugin.h"
#include "PluginInterface/OTLib/OTB/OtbReader.h"
#include "PluginInterface/OTLib/OTB/OtbWriter.h"
#include "PluginInterface/OTLib/Collections/ServerItemList.h"
#include "PluginInterface/OTLib/Server/Items/ServerItem.h"
#include "PluginInterface/Item.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainForm; }
QT_END_NAMESPACE

namespace ItemEditor {

/**
 * Main application window class
 * Exact mirror of C# MainForm class functionality
 */
class MainForm : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainForm(QWidget *parent = nullptr);
    ~MainForm();
    
    // Test support methods
    bool loadOtbFile(const QString& filePath);
    bool isOtbFileLoaded() const;
    QString getCurrentOtbFilePath() const;
    ServerItemListBox* getServerItemListBox() const;
    ClientItemView* getClientItemView() const;
    int getCurrentSelectedItemId() const;
    void clearLoadedData();
    void resetUIState();

protected:
    // Event handlers - mirroring C# event handling
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void changeEvent(QEvent *event) override;

private slots:
    // File menu actions - exact mirror of C# MainForm menu handlers
    void onFileNew();
    void onFileOpen();
    void onFileOpenSprites();
    void onFileSave();
    void onFileSaveAs();
    void onFileCompareOtb();
    void onFileExit();
    
    // Edit menu actions
    void onEditFind();
    void onEditPreferences();
    
    // View menu actions
    void onViewShowOnlyMismatchedItems(bool checked);
    void onViewShowDecimalItemId(bool checked);
    void onViewShowHexItemId(bool checked);
    
    // Tools menu actions
    void onToolsUpdate();
    void onToolsUpdateSettings();
    void onToolsDiagnostics();
    
    // Help menu actions
    void onHelpAbout();
    
    // Item selection and editing - mirroring C# item handling
    void onServerItemSelectionChanged();
    void onItemPropertyChanged();
    void onItemFlagChanged();
    
    // Plugin management
    void onPluginChanged();
    void refreshPluginList();
    
    // Plugin signal handlers
    void onClientLoaded(const ItemEditor::SupportedClient& client);
    void onLoadingProgress(int percentage);
    void onErrorOccurred(const QString& error);
    
    // UI update methods
    void updateItemDisplay();
    void updateStatusBar();
    void updateWindowTitle();
    void updateMenuStates();
    
    // Timer events
    void onUpdateTimer();

private:
    // UI setup methods - mirroring C# InitializeComponent pattern
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupCentralWidget();
    void setupConnections();
    void setupDragDrop();
    
    // File operations - exact mirror of C# file handling
    bool openOtbFile(const QString& filePath);
    bool saveOtbFile(const QString& filePath);
    bool saveOtbFileAs();
    void newOtbFile();
    bool openSpriteFile(const QString& filePath);
    
    // Sprite operations - sprite file management
    bool openSpriteFile(const QString& filePath);
    void unloadSprites();
    
    // Plugin operations - mirroring C# plugin management
    void loadPlugins();
    void selectPlugin(ItemEditor::IPlugin* plugin);
    ItemEditor::IPlugin* getCurrentPlugin() const;
    
    // Item management - mirroring C# item operations
    void loadServerItems();
    void loadClientItems();
    void selectServerItem(quint16 itemId);
    void updateItemProperties();
    void applyItemChanges();
    
    // UI state management
    void saveSettings();
    void loadSettings();
    void resetToDefaults();
    
    // Additional UI setup methods - exact mirror of C# InitializeComponent pattern
    void setupLeftPanel();
    void setupCenterPanel();
    void setupRightPanel();
    void setupPropertiesGroup();
    void setupFlagsGroup();
    void setupAttributesGroup();
    
    // Utility methods
    QString getOpenFileName();
    QString getSaveFileName();
    bool confirmUnsavedChanges();
    void showErrorMessage(const QString& message);
    void showInfoMessage(const QString& message);
    
    // Diagnostic methods
    void validateDataPipeline();
    void logDiagnosticInfo(const QString& stage, const QString& message);
    bool validateOtbLoading();
    bool validatePluginIntegration();
    bool validateUIControls();
    bool validateDataBinding();

    // Search methods
    OTLib::Collections::ServerItemList* findItems(const QVariantMap& searchParameters);
    
    // Helper methods for enhanced updateItemProperties()
    void clearItemProperties();
    void populateBasicProperties(OTLib::Server::Items::ServerItem* serverItem, ItemEditor::ClientItem* clientItem);
    void populateFlagCheckboxes(OTLib::Server::Items::ServerItem* serverItem, ItemEditor::ClientItem* clientItem);
    void populateAttributeControls(OTLib::Server::Items::ServerItem* serverItem, ItemEditor::ClientItem* clientItem);
    void updateSpriteDisplay(ItemEditor::ClientItem* clientItem);
    
    // Sprite display system methods
    void refreshSpriteDisplay();
    void optimizeSpriteCache();
    
private:
    // UI components - exact mirror of C# MainForm controls
    Ui::MainForm *ui;
    
    // Main layout components
    QSplitter* m_mainSplitter;
    QSplitter* m_rightSplitter;
    
    // Left panel - Server Items (exact mirror of C# layout)
    QWidget* m_leftPanel;
    QVBoxLayout* m_leftLayout;
    QGroupBox* m_serverItemsGroupBox;
    ServerItemListBox* m_serverItemListBox;
    
    // Center panel - Item Appearance (exact mirror of C# layout)
    QWidget* m_centerPanel;
    QVBoxLayout* m_centerLayout;
    QGroupBox* m_appearanceGroupBox;
    ClientItemView* m_clientItemView;
    
    // Right panel - Item Properties (exact mirror of C# layout)
    QWidget* m_rightPanel;
    QVBoxLayout* m_rightLayout;
    
    // Item Properties Group
    QGroupBox* m_propertiesGroupBox;
    QGridLayout* m_propertiesLayout;
    QLabel* m_serverIdLabel;
    QSpinBox* m_serverIdSpinBox;
    QLabel* m_clientIdLabel;
    QSpinBox* m_clientIdSpinBox;
    QLabel* m_nameLabel;
    QLineEdit* m_nameLineEdit;
    QLabel* m_descriptionLabel;
    QTextEdit* m_descriptionTextEdit;
    
    // Item Flags Group - exact mirror of C# flags layout
    QGroupBox* m_flagsGroupBox;
    QGridLayout* m_flagsLayout;
    QList<FlagCheckBox*> m_flagCheckBoxes;
    
    // Item Attributes Group
    QGroupBox* m_attributesGroupBox;
    QGridLayout* m_attributesLayout;
    
    // Menu and toolbar
    QMenuBar* m_menuBar;
    QToolBar* m_toolBar;
    QStatusBar* m_statusBar;
    
    // Menu actions - exact mirror of C# menu structure
    QMenu* m_fileMenu;
    QAction* m_newAction;
    QAction* m_openAction;
    QAction* m_openSpriteAction;
    QAction* m_saveAction;
    QAction* m_saveAsAction;
    QAction* m_compareOtbAction;
    QAction* m_exitAction;
    
    QMenu* m_editMenu;
    QAction* m_findAction;
    QAction* m_preferencesAction;
    
    QMenu* m_viewMenu;
    QAction* m_showOnlyMismatchedAction;
    QActionGroup* m_itemIdDisplayGroup;
    QAction* m_showDecimalIdAction;
    QAction* m_showHexIdAction;
    
    QMenu* m_toolsMenu;
    QAction* m_updateAction;
    QAction* m_updateSettingsAction;
    QAction* m_diagnosticsAction;
    
    QMenu* m_helpMenu;
    QAction* m_aboutAction;
    
    // Status bar components - exact mirror of C# status bar
    QLabel* m_statusLabel;
    QProgressBar* m_progressBar;
    QLabel* m_itemCountLabel;
    QLabel* m_pluginLabel;
    
    // Application state - mirroring C# MainForm state
    QString m_currentFilePath;
    bool m_hasUnsavedChanges;
    bool m_isLoading;
    ItemEditor::IPlugin* m_currentPlugin;
    ItemEditor::PluginServices* m_pluginServices;
    quint16 m_selectedServerId;
    
    // OTB Integration
    std::unique_ptr<OTLib::OTB::OtbReader> m_otbReader;
    std::unique_ptr<OTLib::OTB::OtbWriter> m_otbWriter;
    OTLib::Collections::ServerItemList* m_serverItemList;
    SpriteManager* m_spriteManager;
    
    // Settings
    QSettings* m_settings;
    
    // Timers
    QTimer* m_updateTimer;
    
    // Test support members
    bool m_otbFileLoaded;
    QString m_currentOtbFilePath;
    
    // Constants - exact mirror of C# constants
    static const int ITEM_LIST_WIDTH = 232;
    static const int APPEARANCE_WIDTH = 89;
    static const int PROPERTIES_MIN_WIDTH = 200;
    static const int WINDOW_MIN_WIDTH = 800;
    static const int WINDOW_MIN_HEIGHT = 600;
    static const int UPDATE_INTERVAL_MS = 100;
};

} // namespace ItemEditor

#endif // ITEMEDITOR_MAINFORM_H