#include "ServerItemListWidget.h"
#include <QApplication>
#include <QPainter>
#include <QFontMetrics>
#include <QStyle>
#include <QStyleOption>
#include <QDebug>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QScrollArea>
#include <QSplitter>
#include <QRegularExpression>

// ServerItemListWidget Implementation
ServerItemListWidget::ServerItemListWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_filterLayout(nullptr)
    , m_filterEdit(nullptr)
    , m_typeFilterCombo(nullptr)
    , m_clearFilterButton(nullptr)
    , m_itemCountLabel(nullptr)
    , m_treeView(nullptr)
    , m_itemList(nullptr)
    , m_itemModel(nullptr)
    , m_proxyModel(nullptr)
    , m_itemDelegate(nullptr)
    , m_searchTimer(nullptr)
    , m_currentSearchIndex(-1)
    , m_showItemCount(true)
    , m_showTypeIcons(true)
    , m_virtualScrollingEnabled(true)
    , m_itemCacheSize(1000)
    , m_indexCacheValid(false)
{
    setupUI();
    setupTreeView();
    setupFilters();
    setupContextMenu();
    applyDarkTheme();
    
    // Initialize search timer
    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(300); // 300ms delay for search
    connect(m_searchTimer, &QTimer::timeout, this, &ServerItemListWidget::onSearchTimerTimeout);
}

ServerItemListWidget::~ServerItemListWidget()
{
    // Qt's parent-child system will handle cleanup
}

void ServerItemListWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(4, 4, 4, 4);
    m_mainLayout->setSpacing(4);
    
    // Filter layout
    m_filterLayout = new QHBoxLayout();
    m_filterLayout->setSpacing(4);
    
    // Filter edit
    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setPlaceholderText("Filter items...");
    m_filterEdit->setClearButtonEnabled(true);
    connect(m_filterEdit, &QLineEdit::textChanged, this, &ServerItemListWidget::onFilterTextChanged);
    
    // Type filter combo
    m_typeFilterCombo = new QComboBox(this);
    m_typeFilterCombo->addItem("All Types", static_cast<int>(ServerItemType::None));
    m_typeFilterCombo->addItem("Ground", static_cast<int>(ServerItemType::Ground));
    m_typeFilterCombo->addItem("Container", static_cast<int>(ServerItemType::Container));
    m_typeFilterCombo->addItem("Weapon", static_cast<int>(ServerItemType::Weapon));
    m_typeFilterCombo->addItem("Ammunition", static_cast<int>(ServerItemType::Ammunition));
    m_typeFilterCombo->addItem("Armor", static_cast<int>(ServerItemType::Armor));
    m_typeFilterCombo->addItem("Charges", static_cast<int>(ServerItemType::Charges));
    m_typeFilterCombo->addItem("Teleport", static_cast<int>(ServerItemType::Teleport));
    m_typeFilterCombo->addItem("Magic Field", static_cast<int>(ServerItemType::MagicField));
    m_typeFilterCombo->addItem("Writable", static_cast<int>(ServerItemType::Writable));
    m_typeFilterCombo->addItem("Key", static_cast<int>(ServerItemType::Key));
    m_typeFilterCombo->addItem("Splash", static_cast<int>(ServerItemType::Splash));
    m_typeFilterCombo->addItem("Fluid", static_cast<int>(ServerItemType::Fluid));
    m_typeFilterCombo->addItem("Door", static_cast<int>(ServerItemType::Door));
    connect(m_typeFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &ServerItemListWidget::onTypeFilterChanged);
    
    // Clear filter button
    m_clearFilterButton = new QPushButton("Clear", this);
    m_clearFilterButton->setMaximumWidth(60);
    connect(m_clearFilterButton, &QPushButton::clicked, this, &ServerItemListWidget::clearFilters);
    
    // Item count label
    m_itemCountLabel = new QLabel("0 items", this);
    m_itemCountLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    // Add to filter layout
    m_filterLayout->addWidget(m_filterEdit);
    m_filterLayout->addWidget(m_typeFilterCombo);
    m_filterLayout->addWidget(m_clearFilterButton);
    m_filterLayout->addStretch();
    m_filterLayout->addWidget(m_itemCountLabel);
    
    m_mainLayout->addLayout(m_filterLayout);
}

void ServerItemListWidget::setupTreeView()
{
    // Create tree view
    m_treeView = new QTreeView(this);
    m_treeView->setRootIsDecorated(false);
    m_treeView->setAlternatingRowColors(true);
    m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_treeView->setSortingEnabled(true);
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // Create models
    m_itemModel = new ServerItemModel(this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_itemModel);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(-1); // Filter all columns
    
    m_treeView->setModel(m_proxyModel);
    
    // Create delegate
    m_itemDelegate = new ServerItemDelegate(this);
    m_treeView->setItemDelegate(m_itemDelegate);
    
    // Configure headers
    QHeaderView* header = m_treeView->header();
    header->setStretchLastSection(true);
    header->setSectionResizeMode(ServerItemModel::IdColumn, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(ServerItemModel::TypeColumn, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(ServerItemModel::ClientIdColumn, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(ServerItemModel::NameColumn, QHeaderView::Stretch);
    
    // Connect signals
    connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ServerItemListWidget::onTreeSelectionChanged);
    connect(m_treeView, &QTreeView::doubleClicked,
            this, &ServerItemListWidget::onTreeDoubleClicked);
    connect(m_treeView, &QTreeView::customContextMenuRequested,
            this, &ServerItemListWidget::onTreeContextMenu);
    connect(header, &QHeaderView::sectionClicked,
            this, &ServerItemListWidget::onHeaderSectionClicked);
    
    m_mainLayout->addWidget(m_treeView);
}

void ServerItemListWidget::setupFilters()
{
    // Additional filter setup if needed
}

void ServerItemListWidget::setupContextMenu()
{
    // Context menu will be created dynamically in onTreeContextMenu
}

void ServerItemListWidget::applyDarkTheme()
{
    // Apply dark theme styling
    setStyleSheet(R"(
        QWidget {
            background-color: #3C3F41;
            color: #DCDCDC;
        }
        QLineEdit {
            background-color: #45494A;
            border: 1px solid #555555;
            padding: 4px;
            border-radius: 2px;
        }
        QLineEdit:focus {
            border-color: #6897BB;
        }
        QComboBox {
            background-color: #45494A;
            border: 1px solid #555555;
            padding: 4px;
            border-radius: 2px;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 4px solid #DCDCDC;
        }
        QPushButton {
            background-color: #45494A;
            border: 1px solid #555555;
            padding: 4px 8px;
            border-radius: 2px;
        }
        QPushButton:hover {
            background-color: #4C5052;
        }
        QPushButton:pressed {
            background-color: #3A3D3F;
        }
        QTreeView {
            background-color: #2B2B2B;
            alternate-background-color: #313335;
            selection-background-color: #6897BB;
            border: 1px solid #555555;
        }
        QTreeView::item {
            padding: 2px;
            border: none;
        }
        QTreeView::item:selected {
            background-color: #6897BB;
        }
        QTreeView::item:hover {
            background-color: #4C5052;
        }
        QHeaderView::section {
            background-color: #45494A;
            border: 1px solid #555555;
            padding: 4px;
        }
        QScrollBar:vertical {
            background-color: #45494A;
            width: 12px;
        }
        QScrollBar::handle:vertical {
            background-color: #6C6C6C;
            border-radius: 6px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #8C8C8C;
        }
    )");
}

// Data management methods
void ServerItemListWidget::setServerItemList(ServerItemList* itemList)
{
    if (m_itemList == itemList) {
        return;
    }
    
    m_itemList = itemList;
    m_itemModel->setServerItemList(itemList);
    invalidateIndexCache();
    updateItemCount();
    
    if (itemList) {
        // Connect to item list signals if available
        // Note: ServerItemList would need to emit these signals
    }
}

ServerItemList* ServerItemListWidget::getServerItemList() const
{
    return m_itemList;
}

void ServerItemListWidget::refreshItems()
{
    if (m_itemModel) {
        m_itemModel->refreshModel();
        invalidateIndexCache();
        updateItemCount();
    }
}

void ServerItemListWidget::clearItems()
{
    if (m_itemModel) {
        m_itemModel->setServerItemList(nullptr);
        invalidateIndexCache();
        updateItemCount();
    }
}

// Selection management
ServerItem* ServerItemListWidget::getCurrentItem() const
{
    QModelIndex current = m_treeView->currentIndex();
    if (!current.isValid()) {
        return nullptr;
    }
    
    QModelIndex sourceIndex = m_proxyModel->mapToSource(current);
    return m_itemModel->getItem(sourceIndex);
}

ItemId ServerItemListWidget::getCurrentItemId() const
{
    ServerItem* item = getCurrentItem();
    return item ? item->id : 0;
}

void ServerItemListWidget::setCurrentItem(ItemId id)
{
    QModelIndex index = findItemIndex(id);
    if (index.isValid()) {
        m_treeView->setCurrentIndex(index);
        ensureItemVisible(id);
    }
}

void ServerItemListWidget::selectItem(ItemId id)
{
    setCurrentItem(id);
}

QList<ItemId> ServerItemListWidget::getSelectedItemIds() const
{
    QList<ItemId> ids;
    QModelIndexList selected = m_treeView->selectionModel()->selectedRows();
    
    for (const QModelIndex& index : selected) {
        QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
        ItemId id = m_itemModel->getItemId(sourceIndex);
        if (id != 0) {
            ids.append(id);
        }
    }
    
    return ids;
}

// Filtering and search
void ServerItemListWidget::setFilter(const QString& filter)
{
    m_filterEdit->setText(filter);
    m_proxyModel->setFilterFixedString(filter);
    updateItemCount();
}

void ServerItemListWidget::setTypeFilter(ServerItemType type)
{
    int index = m_typeFilterCombo->findData(static_cast<int>(type));
    if (index >= 0) {
        m_typeFilterCombo->setCurrentIndex(index);
    }
}

void ServerItemListWidget::setIdRangeFilter(ItemId minId, ItemId maxId)
{
    // This would require a custom filter implementation
    // For now, we'll use the text filter
    QString rangeFilter = QString("%1-%2").arg(minId).arg(maxId);
    setFilter(rangeFilter);
}

void ServerItemListWidget::clearFilters()
{
    m_filterEdit->clear();
    m_typeFilterCombo->setCurrentIndex(0);
    m_proxyModel->setFilterFixedString("");
    updateItemCount();
}

QString ServerItemListWidget::getCurrentFilter() const
{
    return m_filterEdit->text();
}

void ServerItemListWidget::findItem(const QString& searchText, bool exactMatch)
{
    m_lastSearchText = searchText;
    m_currentSearchIndex = -1;
    m_searchResults.clear();
    
    if (searchText.isEmpty()) {
        return;
    }
    
    // Find all matching items
    for (int row = 0; row < m_proxyModel->rowCount(); ++row) {
        QModelIndex index = m_proxyModel->index(row, ServerItemModel::NameColumn);
        QString itemName = index.data(Qt::DisplayRole).toString();
        
        bool matches = exactMatch ? 
            (itemName.compare(searchText, Qt::CaseInsensitive) == 0) :
            itemName.contains(searchText, Qt::CaseInsensitive);
            
        if (matches) {
            m_searchResults.append(index);
        }
    }
    
    if (!m_searchResults.isEmpty()) {
        m_currentSearchIndex = 0;
        highlightSearchResult(0);
    }
}

void ServerItemListWidget::findNext()
{
    if (m_searchResults.isEmpty()) {
        return;
    }
    
    m_currentSearchIndex = (m_currentSearchIndex + 1) % m_searchResults.size();
    highlightSearchResult(m_currentSearchIndex);
}

void ServerItemListWidget::findPrevious()
{
    if (m_searchResults.isEmpty()) {
        return;
    }
    
    m_currentSearchIndex = (m_currentSearchIndex - 1 + m_searchResults.size()) % m_searchResults.size();
    highlightSearchResult(m_currentSearchIndex);
}

void ServerItemListWidget::clearSearch()
{
    m_lastSearchText.clear();
    m_currentSearchIndex = -1;
    m_searchResults.clear();
}

// Display options
void ServerItemListWidget::setShowItemCount(bool show)
{
    m_showItemCount = show;
    m_itemCountLabel->setVisible(show);
}

void ServerItemListWidget::setShowTypeIcons(bool show)
{
    m_showTypeIcons = show;
    // Trigger view update
    m_treeView->viewport()->update();
}

void ServerItemListWidget::setSortColumn(int column)
{
    m_treeView->sortByColumn(column, m_treeView->header()->sortIndicatorOrder());
}

void ServerItemListWidget::setSortOrder(Qt::SortOrder order)
{
    int column = m_treeView->header()->sortIndicatorSection();
    m_treeView->sortByColumn(column, order);
}

// Performance settings
void ServerItemListWidget::setVirtualScrollingEnabled(bool enabled)
{
    m_virtualScrollingEnabled = enabled;
    // QTreeView has virtual scrolling by default
}

void ServerItemListWidget::setItemCacheSize(int size)
{
    m_itemCacheSize = size;
}

// Slots
void ServerItemListWidget::onItemAdded(ItemId id)
{
    if (m_itemModel) {
        m_itemModel->onItemAdded(id);
        invalidateIndexCache();
        updateItemCount();
    }
}

void ServerItemListWidget::onItemRemoved(ItemId id)
{
    if (m_itemModel) {
        m_itemModel->onItemRemoved(id);
        invalidateIndexCache();
        updateItemCount();
    }
}

void ServerItemListWidget::onItemModified(ItemId id)
{
    if (m_itemModel) {
        m_itemModel->onItemModified(id);
    }
}

void ServerItemListWidget::onItemListChanged()
{
    refreshItems();
}

// Private slots
void ServerItemListWidget::onFilterTextChanged()
{
    m_searchTimer->start(); // Delay the filter application
}

void ServerItemListWidget::onTypeFilterChanged()
{
    ServerItemType type = static_cast<ServerItemType>(
        m_typeFilterCombo->currentData().toInt());
    
    if (type == ServerItemType::None) {
        m_proxyModel->setFilterRegularExpression(QRegularExpression());
    } else {
        // This would require custom filtering logic
        // For now, we'll filter by type name
        QString typeName = getItemTypeDisplayName(type);
        m_proxyModel->setFilterKeyColumn(ServerItemModel::TypeColumn);
        m_proxyModel->setFilterFixedString(typeName);
    }
    
    updateItemCount();
}

void ServerItemListWidget::onTreeSelectionChanged()
{
    ItemId currentId = getCurrentItemId();
    if (currentId != 0) {
        emit itemSelected(currentId);
    }
    emit itemsSelectionChanged();
}

void ServerItemListWidget::onTreeDoubleClicked(const QModelIndex& index)
{
    QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
    ItemId id = m_itemModel->getItemId(sourceIndex);
    if (id != 0) {
        emit itemDoubleClicked(id);
    }
}

void ServerItemListWidget::onTreeContextMenu(const QPoint& position)
{
    QModelIndex index = m_treeView->indexAt(position);
    if (index.isValid()) {
        QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
        ItemId id = m_itemModel->getItemId(sourceIndex);
        if (id != 0) {
            QPoint globalPos = m_treeView->mapToGlobal(position);
            emit contextMenuRequested(id, globalPos);
        }
    }
}

void ServerItemListWidget::onSearchTimerTimeout()
{
    QString filterText = m_filterEdit->text();
    m_proxyModel->setFilterFixedString(filterText);
    updateItemCount();
    emit filterChanged(filterText);
}

void ServerItemListWidget::onHeaderSectionClicked(int logicalIndex)
{
    // Header clicking is handled automatically by QTreeView
}

// Private methods
void ServerItemListWidget::updateItemCount()
{
    if (m_showItemCount && m_itemCountLabel) {
        int count = m_proxyModel ? m_proxyModel->rowCount() : 0;
        m_itemCountLabel->setText(QString("%1 items").arg(count));
        emit itemCountChanged(count);
    }
}

void ServerItemListWidget::updateSearchResults()
{
    // Called when search results need to be updated
}

void ServerItemListWidget::highlightSearchResult(int index)
{
    if (index >= 0 && index < m_searchResults.size()) {
        QModelIndex resultIndex = m_searchResults[index];
        m_treeView->setCurrentIndex(resultIndex);
        m_treeView->scrollTo(resultIndex, QAbstractItemView::EnsureVisible);
    }
}

void ServerItemListWidget::invalidateIndexCache()
{
    m_indexCacheValid = false;
    m_itemIndexCache.clear();
}

QModelIndex ServerItemListWidget::findItemIndex(ItemId id) const
{
    if (!m_itemModel || !m_proxyModel) {
        return QModelIndex();
    }
    
    QModelIndex sourceIndex = m_itemModel->getItemIndex(id);
    if (sourceIndex.isValid()) {
        return m_proxyModel->mapFromSource(sourceIndex);
    }
    
    return QModelIndex();
}

void ServerItemListWidget::ensureItemVisible(ItemId id)
{
    QModelIndex index = findItemIndex(id);
    if (index.isValid()) {
        m_treeView->scrollTo(index, QAbstractItemView::EnsureVisible);
    }
}

QString ServerItemListWidget::getItemTypeDisplayName(ServerItemType type) const
{
    switch (type) {
        case ServerItemType::Ground: return "Ground";
        case ServerItemType::Container: return "Container";
        case ServerItemType::Weapon: return "Weapon";
        case ServerItemType::Ammunition: return "Ammunition";
        case ServerItemType::Armor: return "Armor";
        case ServerItemType::Charges: return "Charges";
        case ServerItemType::Teleport: return "Teleport";
        case ServerItemType::MagicField: return "Magic Field";
        case ServerItemType::Writable: return "Writable";
        case ServerItemType::Key: return "Key";
        case ServerItemType::Splash: return "Splash";
        case ServerItemType::Fluid: return "Fluid";
        case ServerItemType::Door: return "Door";
        case ServerItemType::Deprecated: return "Deprecated";
        default: return "Unknown";
    }
}

QIcon ServerItemListWidget::getItemTypeIcon(ServerItemType type) const
{
    // Return appropriate icons for each type
    // For now, return empty icon
    Q_UNUSED(type)
    return QIcon();
}

//
// ServerItemModel Implementation
ServerItemModel::ServerItemModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_itemList(nullptr)
    , m_rowCacheValid(false)
{
}

ServerItemModel::~ServerItemModel()
{
}

void ServerItemModel::setServerItemList(ServerItemList* itemList)
{
    beginResetModel();
    m_itemList = itemList;
    invalidateRowCache();
    endResetModel();
}

ServerItem* ServerItemModel::getItem(const QModelIndex& index) const
{
    if (!index.isValid() || !m_itemList || index.row() >= m_itemList->size()) {
        return nullptr;
    }
    
    return &(*m_itemList)[index.row()];
}

ItemId ServerItemModel::getItemId(const QModelIndex& index) const
{
    ServerItem* item = getItem(index);
    return item ? item->id : 0;
}

QModelIndex ServerItemModel::getItemIndex(ItemId id) const
{
    if (!m_itemList) {
        return QModelIndex();
    }
    
    ensureRowCacheValid();
    
    if (m_itemRowCache.contains(id)) {
        int row = m_itemRowCache[id];
        return createIndex(row, 0);
    }
    
    return QModelIndex();
}

QModelIndex ServerItemModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid() || !m_itemList || row < 0 || row >= m_itemList->size() || 
        column < 0 || column >= ColumnCount) {
        return QModelIndex();
    }
    
    return createIndex(row, column);
}

QModelIndex ServerItemModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child)
    return QModelIndex(); // Flat list, no parent
}

int ServerItemModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !m_itemList) {
        return 0;
    }
    
    return m_itemList->size();
}

int ServerItemModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return ColumnCount;
}

QVariant ServerItemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    
    ServerItem* item = getItem(index);
    if (!item) {
        return QVariant();
    }
    
    switch (role) {
        case Qt::DisplayRole:
            switch (index.column()) {
                case IdColumn:
                    return item->id;
                case NameColumn:
                    return item->name.isEmpty() ? QString("Item %1").arg(item->id) : item->name;
                case TypeColumn:
                    return getItemTypeDisplayName(item->type);
                case ClientIdColumn:
                    return item->clientId;
                default:
                    return QVariant();
            }
            
        case Qt::ToolTipRole:
            return QString("ID: %1\nName: %2\nType: %3\nClient ID: %4")
                .arg(item->id)
                .arg(item->name.isEmpty() ? "Unnamed" : item->name)
                .arg(getItemTypeDisplayName(item->type))
                .arg(item->clientId);
                
        case Qt::TextAlignmentRole:
            switch (index.column()) {
                case IdColumn:
                case ClientIdColumn:
                    return QVariant(Qt::AlignRight | Qt::AlignVCenter);
                default:
                    return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
            }
            
        case Qt::BackgroundRole:
            if (item->isCustomCreated) {
                return QColor(60, 80, 60); // Slightly green tint for custom items
            }
            break;
            
        case Qt::ForegroundRole:
            if (!item->hasClientData) {
                return QColor(180, 180, 180); // Dimmed for items without client data
            }
            break;
            
        default:
            break;
    }
    
    return QVariant();
}

QVariant ServerItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QVariant();
    }
    
    switch (section) {
        case IdColumn:
            return "ID";
        case NameColumn:
            return "Name";
        case TypeColumn:
            return "Type";
        case ClientIdColumn:
            return "Client ID";
        default:
            return QVariant();
    }
}

Qt::ItemFlags ServerItemModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void ServerItemModel::sort(int column, Qt::SortOrder order)
{
    if (!m_itemList) {
        return;
    }
    
    beginResetModel();
    
    switch (column) {
        case IdColumn:
            m_itemList->sortItems(ServerItemList::SortBy::Id, 
                order == Qt::AscendingOrder ? ServerItemList::SortOrder::Ascending : ServerItemList::SortOrder::Descending);
            break;
        case NameColumn:
            m_itemList->sortItems(ServerItemList::SortBy::Name,
                order == Qt::AscendingOrder ? ServerItemList::SortOrder::Ascending : ServerItemList::SortOrder::Descending);
            break;
        case TypeColumn:
            m_itemList->sortItems(ServerItemList::SortBy::Type,
                order == Qt::AscendingOrder ? ServerItemList::SortOrder::Ascending : ServerItemList::SortOrder::Descending);
            break;
        case ClientIdColumn:
            m_itemList->sortItems(ServerItemList::SortBy::ClientId,
                order == Qt::AscendingOrder ? ServerItemList::SortOrder::Ascending : ServerItemList::SortOrder::Descending);
            break;
    }
    
    invalidateRowCache();
    endResetModel();
}

void ServerItemModel::onItemAdded(ItemId id)
{
    Q_UNUSED(id)
    refreshModel();
}

void ServerItemModel::onItemRemoved(ItemId id)
{
    Q_UNUSED(id)
    refreshModel();
}

void ServerItemModel::onItemModified(ItemId id)
{
    QModelIndex itemIndex = getItemIndex(id);
    if (itemIndex.isValid()) {
        QModelIndex topLeft = createIndex(itemIndex.row(), 0);
        QModelIndex bottomRight = createIndex(itemIndex.row(), ColumnCount - 1);
        emit dataChanged(topLeft, bottomRight);
    }
}

void ServerItemModel::refreshModel()
{
    beginResetModel();
    invalidateRowCache();
    endResetModel();
}

void ServerItemModel::invalidateRowCache()
{
    m_rowCacheValid = false;
    m_itemRowCache.clear();
}

void ServerItemModel::ensureRowCacheValid() const
{
    if (m_rowCacheValid || !m_itemList) {
        return;
    }
    
    m_itemRowCache.clear();
    for (int i = 0; i < m_itemList->size(); ++i) {
        const ServerItem& item = m_itemList->at(i);
        m_itemRowCache[item.id] = i;
    }
    
    m_rowCacheValid = true;
}

int ServerItemModel::findItemRow(ItemId id) const
{
    ensureRowCacheValid();
    return m_itemRowCache.value(id, -1);
}

QString ServerItemModel::getItemTypeDisplayName(ServerItemType type) const
{
    switch (type) {
        case ServerItemType::Ground: return "Ground";
        case ServerItemType::Container: return "Container";
        case ServerItemType::Weapon: return "Weapon";
        case ServerItemType::Ammunition: return "Ammunition";
        case ServerItemType::Armor: return "Armor";
        case ServerItemType::Charges: return "Charges";
        case ServerItemType::Teleport: return "Teleport";
        case ServerItemType::MagicField: return "Magic Field";
        case ServerItemType::Writable: return "Writable";
        case ServerItemType::Key: return "Key";
        case ServerItemType::Splash: return "Splash";
        case ServerItemType::Fluid: return "Fluid";
        case ServerItemType::Door: return "Door";
        case ServerItemType::Deprecated: return "Deprecated";
        default: return "Unknown";
    }
}

// ServerItemDelegate Implementation
ServerItemDelegate::ServerItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void ServerItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.isValid()) {
        return;
    }
    
    // Use default painting for most cases
    QStyledItemDelegate::paint(painter, option, index);
    
    // Add custom painting for specific columns if needed
    switch (index.column()) {
        case ServerItemModel::IdColumn:
            // Custom ID painting could be added here
            break;
        case ServerItemModel::NameColumn:
            // Custom name painting could be added here
            break;
        case ServerItemModel::TypeColumn:
            // Custom type painting could be added here
            break;
        case ServerItemModel::ClientIdColumn:
            // Custom client ID painting could be added here
            break;
    }
}

QSize ServerItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    
    // Ensure minimum height for readability
    size.setHeight(qMax(size.height(), 20));
    
    return size;
}

void ServerItemDelegate::paintItemId(QPainter* painter, const QStyleOptionViewItem& option, ItemId id) const
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(id)
    // Custom ID painting implementation
}

void ServerItemDelegate::paintItemName(QPainter* painter, const QStyleOptionViewItem& option, const QString& name) const
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(name)
    // Custom name painting implementation
}

void ServerItemDelegate::paintItemType(QPainter* painter, const QStyleOptionViewItem& option, ServerItemType type) const
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(type)
    // Custom type painting implementation
}

void ServerItemDelegate::paintClientId(QPainter* painter, const QStyleOptionViewItem& option, ClientId clientId) const
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(clientId)
    // Custom client ID painting implementation
}

#include "ServerItemListWidget.moc"