#pragma once

#include <QUndoCommand>
#include <QVariant>
#include <QString>
#include "ServerItem.h"

// Forward declarations
class ServerItemList;

/**
 * @brief Base class for item editing undo commands
 */
class ItemEditCommand : public QUndoCommand
{
public:
    ItemEditCommand(ServerItem* item, const QString& description, QUndoCommand* parent = nullptr);
    virtual ~ItemEditCommand() = default;

protected:
    ServerItem* m_item;
    ItemId m_itemId; // Store ID in case item pointer becomes invalid
};

/**
 * @brief Command for property value changes
 */
class PropertyChangeCommand : public ItemEditCommand
{
public:
    PropertyChangeCommand(ServerItem* item, const QString& propertyName, 
                         const QVariant& oldValue, const QVariant& newValue,
                         QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

private:
    QString m_propertyName;
    QVariant m_oldValue;
    QVariant m_newValue;
};

/**
 * @brief Command for multiple property changes (batch edit)
 */
class BatchPropertyChangeCommand : public ItemEditCommand
{
public:
    BatchPropertyChangeCommand(ServerItem* item, const QString& description,
                              QUndoCommand* parent = nullptr);

    void addPropertyChange(const QString& propertyName, 
                          const QVariant& oldValue, const QVariant& newValue);
    
    void undo() override;
    void redo() override;
    int id() const override;

private:
    struct PropertyChange {
        QString propertyName;
        QVariant oldValue;
        QVariant newValue;
    };
    
    QList<PropertyChange> m_changes;
};

/**
 * @brief Command for item creation
 */
class CreateItemCommand : public QUndoCommand
{
public:
    CreateItemCommand(ServerItemList* itemList, const ServerItem& item,
                     QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;
    int id() const override;

private:
    ServerItemList* m_itemList;
    ServerItem m_item;
    ItemId m_itemId;
    bool m_itemCreated;
};

/**
 * @brief Command for item deletion
 */
class DeleteItemCommand : public QUndoCommand
{
public:
    DeleteItemCommand(ServerItemList* itemList, ItemId itemId,
                     QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;
    int id() const override;

private:
    ServerItemList* m_itemList;
    ServerItem m_item;
    ItemId m_itemId;
    int m_itemIndex;
    bool m_itemDeleted;
};

/**
 * @brief Command for item duplication
 */
class DuplicateItemCommand : public QUndoCommand
{
public:
    DuplicateItemCommand(ServerItemList* itemList, ItemId sourceId, ItemId newId,
                        QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;
    int id() const override;

private:
    ServerItemList* m_itemList;
    ItemId m_sourceId;
    ItemId m_newId;
    ServerItem m_duplicatedItem;
    bool m_itemCreated;
};