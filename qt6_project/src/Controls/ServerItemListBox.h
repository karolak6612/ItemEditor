/**
 * Item Editor Qt6 - Server Item List Box Header
 * Exact mirror of Legacy_App/csharp/Source/Controls/ServerItemListBox.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_SERVERITEMLISTBOX_H
#define ITEMEDITOR_SERVERITEMLISTBOX_H

#include <QWidget>
#include <QList>
#include <QRect>
#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QScrollBar>
#include <QFont>
#include <QPixmap>
#include <QBrush>
#include <QPen>
#include <QHash>
#include <QPoint>
#include <QTimer>
#include <QPixmap>
#include <memory>

#include "ListBase.h"
#include "../Helpers/MemoryManager.h"

// Forward declarations
namespace ItemEditor {
    class IPlugin;
    class ServerItem;
}

namespace OTLib {
namespace Collections {
    class ServerItemList;
}
namespace Server {
namespace Items {
    class ServerItem;
}
}
}

// Qt MOC declaration for pointer types
Q_DECLARE_OPAQUE_POINTER(ItemEditor::IPlugin*)
Q_DECLARE_OPAQUE_POINTER(OTLib::Collections::ServerItemList*)

namespace ItemEditor {

/**
 * Server Item List Box Widget
 * Exact mirror of C# ServerItemListBox : ListBase<ServerItem>
 * 
 * Custom list widget for displaying server items with sprite thumbnails
 * Inherits from ListBase to maintain same architecture as C# version
 */
class ServerItemListBox : public ListBase<ItemEditor::ServerItem>
{
    Q_OBJECT
    Q_PROPERTY(ItemEditor::IPlugin* plugin READ plugin WRITE setPlugin NOTIFY pluginChanged)
    Q_PROPERTY(quint16 minimumID READ minimumID NOTIFY rangeChanged)
    Q_PROPERTY(quint16 maximumID READ maximumID NOTIFY rangeChanged)
    Q_PROPERTY(OTLib::Collections::ServerItemList* serverItemList READ serverItemList WRITE setServerItemList NOTIFY serverItemListChanged)

public:
    explicit ServerItemListBox(QWidget *parent = nullptr);
    virtual ~ServerItemListBox() = default;

    // Properties - exact mirror of C# properties
    ItemEditor::IPlugin* plugin() const { return m_plugin; }
    void setPlugin(ItemEditor::IPlugin* plugin);
    
    quint16 minimumID() const { return m_minimumID; }
    quint16 maximumID() const { return m_maximumID; }

    // ServerItemList integration
    OTLib::Collections::ServerItemList* serverItemList() const { return m_serverItemList; }
    void setServerItemList(OTLib::Collections::ServerItemList* list);
    void refreshDisplay();

    // Methods - exact mirror of C# methods
    void add(const QList<ItemEditor::ServerItem*>& itemList);
    void addItems(const QList<ItemEditor::ServerItem*>& itemList);
    
    // Convenience methods for plugin integration
    void addItem(quint16 itemId, const QString& name);
    void clearSelection();
    
    // Performance optimization methods
    void clearSpriteCache();
    void optimizeMemoryUsage();
    void setCacheLimit(int maxItems);
    MemoryStats getCacheStats() const;

    // Filtering
    void setShowOnlyMismatchedItems(bool show);

    // ID Display
    enum IdDisplayFormat {
        Decimal,
        Hexadecimal
    };
    void setIdDisplayFormat(IdDisplayFormat format);

public slots:
    void performDeferredUpdate();
    void onMemoryOptimized();
    void onOTLibItemAdded(OTLib::Server::Items::ServerItem* item);
    void onServerItemListChanged();
    void refreshSprites(); // Refresh sprite display after plugin changes

signals:
    void pluginChanged(ItemEditor::IPlugin* plugin);
    void rangeChanged();
    void itemSelectionChanged();
    void itemAdded(ItemEditor::ServerItem* item);
    void serverItemListChanged(OTLib::Collections::ServerItemList* list);

protected:
    // Override methods from ListBase - exact mirror of C# overrides
    void updateItemPosition(ItemEditor::ServerItem* item, int index) override;
    void paintContent(QPainter& painter) override;
    void onSelectionChanged() override;

private:
    // Constants - exact mirror of C# constants
    static const int ITEM_MARGIN = 5;

    // Private fields - exact mirror of C# private fields
    ItemEditor::IPlugin* m_plugin;
    OTLib::Collections::ServerItemList* m_serverItemList;
    quint16 m_minimumID;
    quint16 m_maximumID;
    
    // Layout rectangles - exact mirror of C# Rectangle fields
    QRect m_layoutRect;
    QRect m_destRect;
    QRect m_sourceRect;
    
    // Performance optimization members
    QHash<ItemEditor::ServerItem*, QPoint> m_itemPositions;  // Cache item positions
    QHash<quint16, int> m_itemIndexMap;          // Fast ID to index lookup
    std::unique_ptr<ManagedCache<quint16, QPixmap>> m_spriteCache; // Managed sprite cache
    int m_visibleStartIndex;                     // Virtual scrolling start
    int m_visibleEndIndex;                       // Virtual scrolling end
    
    // Rendering optimization
    int m_itemHeight;                            // Configurable item height
    int m_itemMargin;                            // Configurable item margin
    QPixmap m_renderCache;                       // Cached render result
    bool m_cacheValid;                           // Cache validity flag
    QSize m_lastViewportSize;                    // Last viewport size
    QTimer* m_updateTimer;                       // Deferred update timer
    
    // Memory management
    int m_maxCacheSize;                          // Maximum cache size
    qint64 m_cacheMemoryUsage;                   // Current cache memory usage

    // Helper methods
    void updateItemRange(ItemEditor::ServerItem* item);
    void paintItemBackground(QPainter& painter, const QRect& rect, bool selected);
    void paintItemSprite(QPainter& painter, ItemEditor::ServerItem* item, const QRect& destRect);
    void paintItemText(QPainter& painter, ItemEditor::ServerItem* item, const QRect& layoutRect);
    void paintItemBorder(QPainter& painter, const QRect& rect);
    
    // Performance helper methods
    int getItemsPerRow() const;
    QRect getVisibleRect() const;
    void updateVisibleRange();
    bool isItemSelected(ItemEditor::ServerItem* item) const;
};

} // namespace ItemEditor

#endif // ITEMEDITOR_SERVERITEMLISTBOX_H