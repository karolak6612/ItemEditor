#pragma once

#include <QListWidget>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QScrollBar>
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTreeView>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <functional>

#include "../ItemEditor.Core/ServerItem.h"
#include "../ItemEditor.Core/ServerItemList.h"
#include "../ItemEditor.Core/ItemEnums.h"

class ServerItemModel;
class ServerItemDelegate;

/**
 * @brief Custom widget for displaying server items with filtering and search
 * 
 * Provides a high-performance list widget for displaying server items with:
 * - Virtual scrolling for large datasets
 * - Real-time filtering and search
 * - Multi-column display (ID, Name, Type)
 * - Context menu integration
 * - Sorting capabilities
 */
class ServerItemListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ServerItemListWidget(QWidget *parent = nullptr);
    ~ServerItemListWidget();

    // Data management
    void setServerItemList(ServerItemList* itemList);
    ServerItemList* getServerItemList() const;
    void refreshItems();
    void clearItems();

    // Selection management
    ServerItem* getCurrentItem() const;
    ItemId getCurrentItemId() const;
    void setCurrentItem(ItemId id);
    void selectItem(ItemId id);
    QList<ItemId> getSelectedItemIds() const;

    // Filtering and search
    void setFilter(const QString& filter);
    void setTypeFilter(ServerItemType type);
    void setIdRangeFilter(ItemId minId, ItemId maxId);
    void clearFilters();
    QString getCurrentFilter() const;
    
    // Search functionality
    void findItem(const QString& searchText, bool exactMatch = false);
    void findNext();
    void findPrevious();
    void clearSearch();

    // Display options
    void setShowItemCount(bool show);
    void setShowTypeIcons(bool show);
    void setSortColumn(int column);
    void setSortOrder(Qt::SortOrder order);

    // Performance settings
    void setVirtualScrollingEnabled(bool enabled);
    void setItemCacheSize(int size);

signals:
    void itemSelected(ItemId id);
    void itemDoubleClicked(ItemId id);
    void itemsSelectionChanged();
    void contextMenuRequested(ItemId id, const QPoint& position);
    void filterChanged(const QString& filter);
    void itemCountChanged(int count);

public slots:
    void onItemAdded(ItemId id);
    void onItemRemoved(ItemId id);
    void onItemModified(ItemId id);
    void onItemListChanged();

private slots:
    void onFilterTextChanged();
    void onTypeFilterChanged();
    void onTreeSelectionChanged();
    void onTreeDoubleClicked(const QModelIndex& index);
    void onTreeContextMenu(const QPoint& position);
    void onSearchTimerTimeout();
    void onHeaderSectionClicked(int logicalIndex);

private:
    // UI components
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_filterLayout;
    QLineEdit* m_filterEdit;
    QComboBox* m_typeFilterCombo;
    QPushButton* m_clearFilterButton;
    QLabel* m_itemCountLabel;
    QTreeView* m_treeView;

    // Data and models
    ServerItemList* m_itemList;
    ServerItemModel* m_itemModel;
    QSortFilterProxyModel* m_proxyModel;
    ServerItemDelegate* m_itemDelegate;

    // Search functionality
    QTimer* m_searchTimer;
    QString m_lastSearchText;
    int m_currentSearchIndex;
    QList<QModelIndex> m_searchResults;

    // Settings
    bool m_showItemCount;
    bool m_showTypeIcons;
    bool m_virtualScrollingEnabled;
    int m_itemCacheSize;

    // Performance optimization
    mutable QHash<ItemId, QModelIndex> m_itemIndexCache;
    mutable bool m_indexCacheValid;

    // Private methods
    void setupUI();
    void setupTreeView();
    void setupFilters();
    void setupContextMenu();
    void applyDarkTheme();
    void updateItemCount();
    void updateSearchResults();
    void highlightSearchResult(int index);
    void invalidateIndexCache();
    QModelIndex findItemIndex(ItemId id) const;
    void ensureItemVisible(ItemId id);
    QString getItemTypeDisplayName(ServerItemType type) const;
    QIcon getItemTypeIcon(ServerItemType type) const;
};

/**
 * @brief Custom model for server items with virtual scrolling support
 */
class ServerItemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Column {
        IdColumn = 0,
        NameColumn = 1,
        TypeColumn = 2,
        ClientIdColumn = 3,
        ColumnCount = 4
    };

    explicit ServerItemModel(QObject* parent = nullptr);
    ~ServerItemModel();

    void setServerItemList(ServerItemList* itemList);
    ServerItem* getItem(const QModelIndex& index) const;
    ItemId getItemId(const QModelIndex& index) const;
    QModelIndex getItemIndex(ItemId id) const;

    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Sorting support
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

public slots:
    void onItemAdded(ItemId id);
    void onItemRemoved(ItemId id);
    void onItemModified(ItemId id);
    void refreshModel();

private:
    ServerItemList* m_itemList;
    mutable QHash<ItemId, int> m_itemRowCache;
    mutable bool m_rowCacheValid;

    void invalidateRowCache();
    void ensureRowCacheValid() const;
    int findItemRow(ItemId id) const;
    QString getItemTypeDisplayName(ServerItemType type) const;
};

/**
 * @brief Custom delegate for server item rendering
 */
class ServerItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ServerItemDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    void paintItemId(QPainter* painter, const QStyleOptionViewItem& option, ItemId id) const;
    void paintItemName(QPainter* painter, const QStyleOptionViewItem& option, const QString& name) const;
    void paintItemType(QPainter* painter, const QStyleOptionViewItem& option, ServerItemType type) const;
    void paintClientId(QPainter* painter, const QStyleOptionViewItem& option, ClientId clientId) const;
};