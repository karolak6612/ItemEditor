#include "ItemEditingManager.h"
#include "ItemValidator.h"
#include <QDebug>
#include <QRandomGenerator>
#include <QApplication>

ItemEditingManager::ItemEditingManager(QObject* parent)
    : QObject(parent)
    , m_itemList(nullptr)
    , m_clientItem(nullptr)
    , m_currentItem(nullptr)
    , m_currentItemId(0)
    , m_undoStack(new QUndoStack(this))
    , m_currentBatchCommand(nullptr)
    , m_batchEditing(false)
    , m_hasUnsavedChanges(false)
    , m_autoSaveTimer(new QTimer(this))
    , m_autoSaveEnabled(false)
    , m_autoSaveInterval(300) // 5 minutes default
    , m_validationEnabled(true)
    , m_realTimeValidation(true)
{
    // Configure auto-save timer
    m_autoSaveTimer->setSingleShot(false);
    m_autoSaveTimer->setInterval(m_autoSaveInterval * 1000);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &ItemEditingManager::onAutoSaveTimer);

    // Connect undo stack signals
    connect(m_undoStack, &QUndoStack::canUndoChanged, this, &ItemEditingManager::undoRedoStateChanged);
    connect(m_undoStack, &QUndoStack::canRedoChanged, this, &ItemEditingManager::undoRedoStateChanged);
    connect(m_undoStack, &QUndoStack::indexChanged, this, &ItemEditingManager::onUndoStackChanged);
}

ItemEditingManager::~ItemEditingManager()
{
    if (m_batchEditing) {
        cancelBatchEdit();
    }
}

// Data management
void ItemEditingManager::setServerItemList(ServerItemList* itemList)
{
    if (m_itemList == itemList) {
        return;
    }

    // Clear current selection
    clearSelection();

    m_itemList = itemList;
    m_undoStack->clear();
    resetChangeTracking();
}

ServerItemList* ItemEditingManager::getServerItemList() const
{
    return m_itemList;
}

void ItemEditingManager::setClientItem(const ClientItem* clientItem)
{
    m_clientItem = clientItem;
}

const ClientItem* ItemEditingManager::getClientItem() const
{
    return m_clientItem;
}

// Item selection
void ItemEditingManager::selectItem(ItemId id)
{
    if (m_currentItemId == id) {
        return;
    }

    // Save any pending changes before switching items
    if (m_batchEditing) {
        endBatchEdit();
    }

    // Clear previous selection
    if (m_currentItem) {
        disconnectItemSignals();
    }

    m_currentItemId = id;
    m_currentItem = m_itemList ? m_itemList->findItem(id) : nullptr;

    if (m_currentItem) {
        connectItemSignals();
        storeOriginalValues();
        emit itemSelected(id);
        qDebug() << "Selected item" << id;
    } else {
        emit itemDeselected();
        qDebug() << "Deselected item (item not found:" << id << ")";
    }

    resetChangeTracking();
}

ServerItem* ItemEditingManager::getCurrentItem() const
{
    return m_currentItem;
}

ItemId ItemEditingManager::getCurrentItemId() const
{
    return m_currentItemId;
}

bool ItemEditingManager::hasSelection() const
{
    return m_currentItem != nullptr;
}

void ItemEditingManager::clearSelection()
{
    if (m_currentItem) {
        if (m_batchEditing) {
            endBatchEdit();
        }
        
        disconnectItemSignals();
        m_currentItem = nullptr;
        m_currentItemId = 0;
        resetChangeTracking();
        emit itemDeselected();
    }
}

// Property editing
void ItemEditingManager::setProperty(const QString& propertyName, const QVariant& value)
{
    if (!m_currentItem) {
        qWarning() << "Cannot set property: no item selected";
        return;
    }

    QVariant oldValue = m_currentItem->getProperty(propertyName);
    if (oldValue == value) {
        return; // No change
    }

    // Validate the new value if validation is enabled
    if (m_validationEnabled && m_realTimeValidation) {
        validateProperty(propertyName, value);
    }

    // Create undo command
    if (m_batchEditing && m_currentBatchCommand) {
        // Add to batch command
        m_currentBatchCommand->addPropertyChange(propertyName, oldValue, value);
    } else {
        // Create individual command
        PropertyChangeCommand* command = new PropertyChangeCommand(
            m_currentItem, propertyName, oldValue, value);
        m_undoStack->push(command);
    }

    // Apply the change immediately
    m_currentItem->setProperty(propertyName, value);
    m_hasUnsavedChanges = true;

    // Track the change
    if (!m_originalValues.contains(propertyName)) {
        m_originalValues[propertyName] = oldValue;
    }
    m_pendingChanges[propertyName] = value;

    emitPropertyChanged(propertyName, oldValue, value);
    emit itemModified(m_currentItemId);

    // Start auto-save timer if enabled
    if (m_autoSaveEnabled && !m_autoSaveTimer->isActive()) {
        m_autoSaveTimer->start();
    }
}

QVariant ItemEditingManager::getProperty(const QString& propertyName) const
{
    if (m_currentItem) {
        return m_currentItem->getProperty(propertyName);
    }
    return QVariant();
}

void ItemEditingManager::setProperties(const QHash<QString, QVariant>& properties)
{
    if (!m_currentItem || properties.isEmpty()) {
        return;
    }

    bool wasBatchEditing = m_batchEditing;
    if (!wasBatchEditing) {
        beginBatchEdit("Set Multiple Properties");
    }

    for (auto it = properties.begin(); it != properties.end(); ++it) {
        setProperty(it.key(), it.value());
    }

    if (!wasBatchEditing) {
        endBatchEdit();
    }
}

// Batch editing
void ItemEditingManager::beginBatchEdit(const QString& description)
{
    if (m_batchEditing) {
        qWarning() << "Already in batch edit mode";
        return;
    }

    if (!m_currentItem) {
        qWarning() << "Cannot start batch edit: no item selected";
        return;
    }

    m_batchEditing = true;
    m_batchDescription = description;
    m_currentBatchCommand = new BatchPropertyChangeCommand(m_currentItem, description);

    emit batchEditStarted();
    qDebug() << "Started batch edit:" << description;
}

void ItemEditingManager::endBatchEdit()
{
    if (!m_batchEditing || !m_currentBatchCommand) {
        return;
    }

    // Only push the command if there are actual changes
    if (!m_currentBatchCommand->text().isEmpty()) {
        m_undoStack->push(m_currentBatchCommand);
    } else {
        delete m_currentBatchCommand;
    }

    m_currentBatchCommand = nullptr;
    m_batchEditing = false;
    m_batchDescription.clear();

    emit batchEditEnded();
    qDebug() << "Ended batch edit";
}

bool ItemEditingManager::isBatchEditing() const
{
    return m_batchEditing;
}

void ItemEditingManager::cancelBatchEdit()
{
    if (!m_batchEditing) {
        return;
    }

    // Revert all changes made during batch edit
    for (auto it = m_pendingChanges.begin(); it != m_pendingChanges.end(); ++it) {
        const QString& propertyName = it.key();
        if (m_originalValues.contains(propertyName)) {
            m_currentItem->setProperty(propertyName, m_originalValues[propertyName]);
        }
    }

    delete m_currentBatchCommand;
    m_currentBatchCommand = nullptr;
    m_batchEditing = false;
    m_batchDescription.clear();

    resetChangeTracking();
    emit batchEditCancelled();
    qDebug() << "Cancelled batch edit";
}

// Undo/Redo system
QUndoStack* ItemEditingManager::getUndoStack() const
{
    return m_undoStack;
}

bool ItemEditingManager::canUndo() const
{
    return m_undoStack->canUndo();
}

bool ItemEditingManager::canRedo() const
{
    return m_undoStack->canRedo();
}

QString ItemEditingManager::undoText() const
{
    return m_undoStack->undoText();
}

QString ItemEditingManager::redoText() const
{
    return m_undoStack->redoText();
}

void ItemEditingManager::clearUndoStack()
{
    m_undoStack->clear();
    resetChangeTracking();
}

// Item operations
ItemId ItemEditingManager::createItem(const ServerItem& templateItem)
{
    if (!m_itemList) {
        qWarning() << "Cannot create item: no item list set";
        return 0;
    }

    ItemId newId = generateNewItemId();
    ServerItem newItem = templateItem;
    newItem.id = newId;
    newItem.isCustomCreated = true;
    newItem.markAsModified();

    CreateItemCommand* command = new CreateItemCommand(m_itemList, newItem);
    m_undoStack->push(command);

    m_hasUnsavedChanges = true;
    emit itemCreated(newId);
    qDebug() << "Created item" << newId;

    return newId;
}

bool ItemEditingManager::deleteItem(ItemId id)
{
    if (!m_itemList || !m_itemList->findItem(id)) {
        qWarning() << "Cannot delete item: item not found" << id;
        return false;
    }

    // Clear selection if deleting current item
    if (m_currentItemId == id) {
        clearSelection();
    }

    DeleteItemCommand* command = new DeleteItemCommand(m_itemList, id);
    m_undoStack->push(command);

    m_hasUnsavedChanges = true;
    emit itemDeleted(id);
    qDebug() << "Deleted item" << id;

    return true;
}

ItemId ItemEditingManager::duplicateItem(ItemId sourceId, ItemId newId)
{
    if (!m_itemList) {
        qWarning() << "Cannot duplicate item: no item list set";
        return 0;
    }

    ServerItem* sourceItem = m_itemList->findItem(sourceId);
    if (!sourceItem) {
        qWarning() << "Cannot duplicate item: source item not found" << sourceId;
        return 0;
    }

    if (newId == 0) {
        newId = generateNewItemId();
    }

    DuplicateItemCommand* command = new DuplicateItemCommand(m_itemList, sourceId, newId);
    m_undoStack->push(command);

    m_hasUnsavedChanges = true;
    emit itemDuplicated(sourceId, newId);
    qDebug() << "Duplicated item" << sourceId << "to" << newId;

    return newId;
}

// Validation and change tracking
bool ItemEditingManager::hasUnsavedChanges() const
{
    return m_hasUnsavedChanges || !m_undoStack->isClean();
}

bool ItemEditingManager::hasValidationErrors() const
{
    return !m_validationErrors.isEmpty();
}

QStringList ItemEditingManager::getValidationErrors() const
{
    return m_validationErrors;
}

QStringList ItemEditingManager::getModifiedProperties() const
{
    return m_pendingChanges.keys();
}

// Auto-save functionality
void ItemEditingManager::setAutoSaveEnabled(bool enabled)
{
    m_autoSaveEnabled = enabled;
    if (!enabled && m_autoSaveTimer->isActive()) {
        m_autoSaveTimer->stop();
    }
}

bool ItemEditingManager::isAutoSaveEnabled() const
{
    return m_autoSaveEnabled;
}

void ItemEditingManager::setAutoSaveInterval(int seconds)
{
    m_autoSaveInterval = seconds;
    m_autoSaveTimer->setInterval(seconds * 1000);
}

int ItemEditingManager::getAutoSaveInterval() const
{
    return m_autoSaveInterval;
}

// Settings
void ItemEditingManager::setValidationEnabled(bool enabled)
{
    m_validationEnabled = enabled;
    if (enabled && m_currentItem) {
        validateCurrentItem();
    } else {
        m_validationErrors.clear();
        emit validationStateChanged(false);
    }
}

bool ItemEditingManager::isValidationEnabled() const
{
    return m_validationEnabled;
}

void ItemEditingManager::setRealTimeValidation(bool enabled)
{
    m_realTimeValidation = enabled;
}

bool ItemEditingManager::isRealTimeValidation() const
{
    return m_realTimeValidation;
}

// Public slots
void ItemEditingManager::undo()
{
    if (m_batchEditing) {
        cancelBatchEdit();
    } else {
        m_undoStack->undo();
    }
}

void ItemEditingManager::redo()
{
    m_undoStack->redo();
}

void ItemEditingManager::resetToDefaults()
{
    if (!m_currentItem) {
        return;
    }

    ServerItem defaultItem;
    QHash<QString, QVariant> defaultProperties;
    
    QStringList propertyNames = m_currentItem->getPropertyNames();
    for (const QString& propertyName : propertyNames) {
        QVariant defaultValue = defaultItem.getProperty(propertyName);
        if (defaultValue != m_currentItem->getProperty(propertyName)) {
            defaultProperties[propertyName] = defaultValue;
        }
    }

    if (!defaultProperties.isEmpty()) {
        setProperties(defaultProperties);
    }
}

void ItemEditingManager::copyFromClient()
{
    if (!m_currentItem || !m_clientItem) {
        return;
    }

    QHash<QString, QVariant> clientProperties;
    QStringList clientPropertyNames = m_clientItem->getPropertyNames();
    QStringList serverPropertyNames = m_currentItem->getPropertyNames();

    for (const QString& propertyName : clientPropertyNames) {
        if (serverPropertyNames.contains(propertyName)) {
            QVariant clientValue = m_clientItem->getProperty(propertyName);
            if (clientValue != m_currentItem->getProperty(propertyName)) {
                clientProperties[propertyName] = clientValue;
            }
        }
    }

    if (!clientProperties.isEmpty()) {
        beginBatchEdit("Copy from Client");
        setProperties(clientProperties);
        endBatchEdit();
    }
}

void ItemEditingManager::validateCurrentItem()
{
    if (!m_currentItem || !m_validationEnabled) {
        return;
    }

    m_validationErrors.clear();
    QStringList propertyNames = m_currentItem->getPropertyNames();
    
    for (const QString& propertyName : propertyNames) {
        QVariant value = m_currentItem->getProperty(propertyName);
        validateProperty(propertyName, value);
    }

    updateValidationState();
}

void ItemEditingManager::saveChanges()
{
    if (m_itemList) {
        m_itemList->markAsModified();
        m_undoStack->setClean();
        m_hasUnsavedChanges = false;
        
        if (m_autoSaveTimer->isActive()) {
            m_autoSaveTimer->stop();
        }
    }
}

// Private slots
void ItemEditingManager::onAutoSaveTimer()
{
    if (hasUnsavedChanges()) {
        emit autoSaveTriggered();
    }
    m_autoSaveTimer->stop();
}

void ItemEditingManager::onUndoStackChanged()
{
    // Update change tracking based on undo stack state
    m_hasUnsavedChanges = !m_undoStack->isClean();
}

void ItemEditingManager::onPropertyValidationRequested(const QString& propertyName)
{
    if (m_currentItem && m_validationEnabled) {
        QVariant value = m_currentItem->getProperty(propertyName);
        validateProperty(propertyName, value);
        updateValidationState();
    }
}

// Private methods
void ItemEditingManager::storeOriginalValues()
{
    m_originalValues.clear();
    if (m_currentItem) {
        QStringList propertyNames = m_currentItem->getPropertyNames();
        for (const QString& propertyName : propertyNames) {
            m_originalValues[propertyName] = m_currentItem->getProperty(propertyName);
        }
    }
}

void ItemEditingManager::validateProperty(const QString& propertyName, const QVariant& value)
{
    if (!m_validationEnabled) {
        return;
    }

    bool isValid = true;
    QString errorKey = QString("%1_error").arg(propertyName);

    // Remove previous error for this property
    m_validationErrors.removeAll(errorKey);

    // Validate based on property type
    if (propertyName == "id") {
        isValid = ItemValidator::validateItemId(value.toUInt());
    } else if (propertyName == "name") {
        isValid = ItemValidator::validateItemName(value.toString());
    } else if (propertyName == "width" || propertyName == "height") {
        quint8 width = (propertyName == "width") ? value.toUInt() : m_currentItem->width;
        quint8 height = (propertyName == "height") ? value.toUInt() : m_currentItem->height;
        isValid = ItemValidator::validateDimensions(width, height);
    } else if (propertyName == "speed") {
        isValid = ItemValidator::validateSpeed(value.toUInt());
    } else if (propertyName == "lightLevel") {
        isValid = ItemValidator::validateLight(value.toUInt(), m_currentItem->lightColor);
    } else if (propertyName == "flags") {
        isValid = ItemValidator::validateFlags(value.toUInt());
    }

    if (!isValid) {
        m_validationErrors.append(errorKey);
    }
}

void ItemEditingManager::updateValidationState()
{
    emit validationStateChanged(hasValidationErrors());
}

void ItemEditingManager::resetChangeTracking()
{
    m_originalValues.clear();
    m_pendingChanges.clear();
    m_validationErrors.clear();
    m_hasUnsavedChanges = false;
}

ItemId ItemEditingManager::generateNewItemId() const
{
    if (!m_itemList) {
        return 1;
    }

    // Find the next available ID
    ItemId newId = m_itemList->getNextAvailableId();

    return newId;
}

void ItemEditingManager::connectItemSignals()
{
    // Connect to item modification signals if the item supports them
    // This would depend on the ServerItem implementation
}

void ItemEditingManager::disconnectItemSignals()
{
    // Disconnect from item modification signals
}

void ItemEditingManager::emitPropertyChanged(const QString& propertyName, const QVariant& oldValue, const QVariant& newValue)
{
    emit propertyChanged(propertyName, oldValue, newValue);
}