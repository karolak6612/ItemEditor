#pragma once

#include <QObject>
#include <QUndoStack>
#include <QTimer>
#include <QHash>
#include <QVariant>
#include "ServerItem.h"
#include "ClientItem.h"
#include "ServerItemList.h"
#include "UndoCommands.h"

/**
 * @brief Manages item selection and editing workflow with undo/redo support
 * 
 * This class provides:
 * - Item selection management with property display
 * - Attribute modification with validation
 * - Undo/redo functionality for all changes
 * - Batch editing support
 * - Change tracking and validation
 */
class ItemEditingManager : public QObject
{
    Q_OBJECT

public:
    explicit ItemEditingManager(QObject* parent = nullptr);
    ~ItemEditingManager();

    // Data management
    void setServerItemList(ServerItemList* itemList);
    ServerItemList* getServerItemList() const;
    void setClientItem(const ClientItem* clientItem);
    const ClientItem* getClientItem() const;

    // Item selection
    void selectItem(ItemId id);
    ServerItem* getCurrentItem() const;
    ItemId getCurrentItemId() const;
    bool hasSelection() const;
    void clearSelection();

    // Property editing
    void setProperty(const QString& propertyName, const QVariant& value);
    QVariant getProperty(const QString& propertyName) const;
    void setProperties(const QHash<QString, QVariant>& properties);
    
    // Batch editing
    void beginBatchEdit(const QString& description = "Batch Edit");
    void endBatchEdit();
    bool isBatchEditing() const;
    void cancelBatchEdit();

    // Undo/Redo system
    QUndoStack* getUndoStack() const;
    bool canUndo() const;
    bool canRedo() const;
    QString undoText() const;
    QString redoText() const;
    void clearUndoStack();

    // Item operations
    ItemId createItem(const ServerItem& templateItem = ServerItem());
    bool deleteItem(ItemId id);
    ItemId duplicateItem(ItemId sourceId, ItemId newId = 0);
    
    // Validation and change tracking
    bool hasUnsavedChanges() const;
    bool hasValidationErrors() const;
    QStringList getValidationErrors() const;
    QStringList getModifiedProperties() const;
    
    // Auto-save functionality
    void setAutoSaveEnabled(bool enabled);
    bool isAutoSaveEnabled() const;
    void setAutoSaveInterval(int seconds);
    int getAutoSaveInterval() const;

    // Settings
    void setValidationEnabled(bool enabled);
    bool isValidationEnabled() const;
    void setRealTimeValidation(bool enabled);
    bool isRealTimeValidation() const;

signals:
    void itemSelected(ItemId id);
    void itemDeselected();
    void propertyChanged(const QString& propertyName, const QVariant& oldValue, const QVariant& newValue);
    void itemModified(ItemId id);
    void itemCreated(ItemId id);
    void itemDeleted(ItemId id);
    void itemDuplicated(ItemId sourceId, ItemId newId);
    void batchEditStarted();
    void batchEditEnded();
    void batchEditCancelled();
    void validationStateChanged(bool hasErrors);
    void undoRedoStateChanged();
    void autoSaveTriggered();

public slots:
    void undo();
    void redo();
    void resetToDefaults();
    void copyFromClient();
    void validateCurrentItem();
    void saveChanges();

private slots:
    void onAutoSaveTimer();
    void onUndoStackChanged();
    void onPropertyValidationRequested(const QString& propertyName);

private:
    // Core data
    ServerItemList* m_itemList;
    const ClientItem* m_clientItem;
    ServerItem* m_currentItem;
    ItemId m_currentItemId;

    // Undo/Redo system
    QUndoStack* m_undoStack;
    BatchPropertyChangeCommand* m_currentBatchCommand;
    bool m_batchEditing;
    QString m_batchDescription;

    // Change tracking
    QHash<QString, QVariant> m_originalValues;
    QHash<QString, QVariant> m_pendingChanges;
    QStringList m_validationErrors;
    bool m_hasUnsavedChanges;

    // Auto-save
    QTimer* m_autoSaveTimer;
    bool m_autoSaveEnabled;
    int m_autoSaveInterval;

    // Settings
    bool m_validationEnabled;
    bool m_realTimeValidation;

    // Private methods
    void storeOriginalValues();
    void applyPendingChanges();
    void validateProperty(const QString& propertyName, const QVariant& value);
    void updateValidationState();
    void resetChangeTracking();
    ItemId generateNewItemId() const;
    void connectItemSignals();
    void disconnectItemSignals();
    void emitPropertyChanged(const QString& propertyName, const QVariant& oldValue, const QVariant& newValue);
};