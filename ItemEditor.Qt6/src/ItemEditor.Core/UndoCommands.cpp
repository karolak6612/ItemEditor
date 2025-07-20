#include "UndoCommands.h"
#include "ServerItemList.h"
#include <QDebug>

// ItemEditCommand Implementation
ItemEditCommand::ItemEditCommand(ServerItem* item, const QString& description, QUndoCommand* parent)
    : QUndoCommand(description, parent)
    , m_item(item)
    , m_itemId(item ? item->id : 0)
{
}

// PropertyChangeCommand Implementation
PropertyChangeCommand::PropertyChangeCommand(ServerItem* item, const QString& propertyName,
                                           const QVariant& oldValue, const QVariant& newValue,
                                           QUndoCommand* parent)
    : ItemEditCommand(item, QString("Change %1").arg(propertyName), parent)
    , m_propertyName(propertyName)
    , m_oldValue(oldValue)
    , m_newValue(newValue)
{
}

void PropertyChangeCommand::undo()
{
    if (m_item) {
        m_item->setProperty(m_propertyName, m_oldValue);
        m_item->markAsModified();
        qDebug() << "Undo property change:" << m_propertyName << "to" << m_oldValue;
    }
}

void PropertyChangeCommand::redo()
{
    if (m_item) {
        m_item->setProperty(m_propertyName, m_newValue);
        m_item->markAsModified();
        qDebug() << "Redo property change:" << m_propertyName << "to" << m_newValue;
    }
}

int PropertyChangeCommand::id() const
{
    // Use a hash of item ID and property name for merging
    return qHash(QString("%1_%2").arg(m_itemId).arg(m_propertyName));
}

bool PropertyChangeCommand::mergeWith(const QUndoCommand* other)
{
    const PropertyChangeCommand* otherCmd = static_cast<const PropertyChangeCommand*>(other);
    
    if (otherCmd->id() != id() || otherCmd->m_item != m_item) {
        return false;
    }
    
    // Merge by updating the new value
    m_newValue = otherCmd->m_newValue;
    setText(QString("Change %1").arg(m_propertyName));
    
    return true;
}

// BatchPropertyChangeCommand Implementation
BatchPropertyChangeCommand::BatchPropertyChangeCommand(ServerItem* item, const QString& description,
                                                      QUndoCommand* parent)
    : ItemEditCommand(item, description, parent)
{
}

void BatchPropertyChangeCommand::addPropertyChange(const QString& propertyName,
                                                  const QVariant& oldValue, const QVariant& newValue)
{
    PropertyChange change;
    change.propertyName = propertyName;
    change.oldValue = oldValue;
    change.newValue = newValue;
    m_changes.append(change);
}

void BatchPropertyChangeCommand::undo()
{
    if (!m_item) return;
    
    // Apply changes in reverse order
    for (int i = m_changes.size() - 1; i >= 0; --i) {
        const PropertyChange& change = m_changes[i];
        m_item->setProperty(change.propertyName, change.oldValue);
    }
    
    m_item->markAsModified();
    qDebug() << "Undo batch property changes for item" << m_itemId;
}

void BatchPropertyChangeCommand::redo()
{
    if (!m_item) return;
    
    // Apply changes in forward order
    for (const PropertyChange& change : m_changes) {
        m_item->setProperty(change.propertyName, change.newValue);
    }
    
    m_item->markAsModified();
    qDebug() << "Redo batch property changes for item" << m_itemId;
}

int BatchPropertyChangeCommand::id() const
{
    return qHash(QString("batch_%1").arg(m_itemId));
}

// CreateItemCommand Implementation
CreateItemCommand::CreateItemCommand(ServerItemList* itemList, const ServerItem& item,
                                   QUndoCommand* parent)
    : QUndoCommand("Create Item", parent)
    , m_itemList(itemList)
    , m_item(item)
    , m_itemId(item.id)
    , m_itemCreated(false)
{
    setText(QString("Create Item %1").arg(m_itemId));
}

void CreateItemCommand::undo()
{
    if (m_itemList && m_itemCreated) {
        m_itemList->removeItem(m_itemId);
        m_itemCreated = false;
        qDebug() << "Undo create item" << m_itemId;
    }
}

void CreateItemCommand::redo()
{
    if (m_itemList && !m_itemCreated) {
        m_itemList->addItem(m_item);
        m_itemCreated = true;
        qDebug() << "Redo create item" << m_itemId;
    }
}

int CreateItemCommand::id() const
{
    return qHash(QString("create_%1").arg(m_itemId));
}

// DeleteItemCommand Implementation
DeleteItemCommand::DeleteItemCommand(ServerItemList* itemList, ItemId itemId,
                                   QUndoCommand* parent)
    : QUndoCommand("Delete Item", parent)
    , m_itemList(itemList)
    , m_itemId(itemId)
    , m_itemIndex(-1)
    , m_itemDeleted(false)
{
    setText(QString("Delete Item %1").arg(m_itemId));
    
    // Store the item data before deletion
    if (m_itemList) {
        ServerItem* item = m_itemList->findItem(m_itemId);
        if (item) {
            m_item = *item;
            m_itemIndex = m_itemList->findItemIndex(m_itemId);
        }
    }
}

void DeleteItemCommand::undo()
{
    if (m_itemList && m_itemDeleted) {
        // Insert the item back at its original position
        if (m_itemIndex >= 0) {
            m_itemList->insert(m_itemIndex, m_item);
        } else {
            m_itemList->addItem(m_item);
        }
        m_itemDeleted = false;
        qDebug() << "Undo delete item" << m_itemId;
    }
}

void DeleteItemCommand::redo()
{
    if (m_itemList && !m_itemDeleted) {
        m_itemList->removeItem(m_itemId);
        m_itemDeleted = true;
        qDebug() << "Redo delete item" << m_itemId;
    }
}

int DeleteItemCommand::id() const
{
    return qHash(QString("delete_%1").arg(m_itemId));
}

// DuplicateItemCommand Implementation
DuplicateItemCommand::DuplicateItemCommand(ServerItemList* itemList, ItemId sourceId, ItemId newId,
                                         QUndoCommand* parent)
    : QUndoCommand("Duplicate Item", parent)
    , m_itemList(itemList)
    , m_sourceId(sourceId)
    , m_newId(newId)
    , m_itemCreated(false)
{
    setText(QString("Duplicate Item %1 to %2").arg(m_sourceId).arg(m_newId));
    
    // Create the duplicated item
    if (m_itemList) {
        ServerItem* sourceItem = m_itemList->findItem(m_sourceId);
        if (sourceItem) {
            m_duplicatedItem = *sourceItem;
            m_duplicatedItem.id = m_newId;
            m_duplicatedItem.isCustomCreated = true;
            m_duplicatedItem.markAsModified();
        }
    }
}

void DuplicateItemCommand::undo()
{
    if (m_itemList && m_itemCreated) {
        m_itemList->removeItem(m_newId);
        m_itemCreated = false;
        qDebug() << "Undo duplicate item" << m_sourceId << "to" << m_newId;
    }
}

void DuplicateItemCommand::redo()
{
    if (m_itemList && !m_itemCreated) {
        m_itemList->addItem(m_duplicatedItem);
        m_itemCreated = true;
        qDebug() << "Redo duplicate item" << m_sourceId << "to" << m_newId;
    }
}

int DuplicateItemCommand::id() const
{
    return qHash(QString("duplicate_%1_%2").arg(m_sourceId).arg(m_newId));
}