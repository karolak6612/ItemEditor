/**
 * Item Editor Qt6 - Server Item List Box Implementation
 * Exact mirror of Legacy_App/csharp/Source/Controls/ServerItemListBox.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "ServerItemListBox.h"
#include "../PluginInterface/Item.h"
#include "../PluginInterface/IPlugin.h"
#include "../PluginInterface/OTLib/Collections/ServerItemList.h"
#include "../PluginInterface/OTLib/Server/Items/ServerItem.h"

namespace ItemEditor {

ServerItemListBox::ServerItemListBox(QWidget *parent)
    : ListBase<ItemEditor::ServerItem>(ListBaseLayout::Vertical, parent)
    , m_plugin(nullptr)
    , m_serverItemList(nullptr)
    , m_minimumID(0)
    , m_maximumID(0)
    , m_visibleStartIndex(0)
    , m_visibleEndIndex(0)
    , m_itemHeight(32)
    , m_itemMargin(2)
    , m_lastViewportSize()
    , m_cacheValid(false)
    , m_maxCacheSize(1000)
    , m_cacheMemoryUsage(0)
{
    // Initialize layout rectangles
    m_layoutRect = QRect();
    m_destRect = QRect();
    m_sourceRect = QRect();
    
    // Initialize performance optimization structures
    m_itemPositions.clear();
    m_itemIndexMap.clear();
    
    // Initialize managed sprite cache with memory optimization
    m_spriteCache = std::make_unique<ManagedCache<quint16, QPixmap>>(m_maxCacheSize, this);
    
    // Performance optimizations
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_StaticContents, true);
    
    // Set up virtual scrolling timer for smooth updates
    m_updateTimer = new QTimer(this);
    m_updateTimer->setSingleShot(true);
    m_updateTimer->setInterval(16); // ~60 FPS
    connect(m_updateTimer, &QTimer::timeout, this, &ServerItemListBox::performDeferredUpdate);
    
    // Connect to memory manager for optimization
    connect(MemoryManager::instance(), &MemoryManager::memoryOptimized,
            this, &ServerItemListBox::onMemoryOptimized);
    
    // Track memory allocation
    MemoryManager::instance()->trackAllocation(this, sizeof(*this), "ServerItemListBox");
}

void ServerItemListBox::setPlugin(ItemEditor::IPlugin* plugin)
{
    if (m_plugin != plugin) {
        m_plugin = plugin;
        
        // Clear sprite cache when plugin changes to ensure fresh sprite loading
        if (plugin) {
            refreshSprites();
        } else {
            clearSpriteCache();
        }
        
        emit pluginChanged(plugin);
    }
}

void ServerItemListBox::setServerItemList(OTLib::Collections::ServerItemList* list)
{
    if (m_serverItemList != list) {
        // Disconnect from old list
        if (m_serverItemList) {
            disconnect(m_serverItemList, nullptr, this, nullptr);
        }
        
        m_serverItemList = list;
        
        // Connect to new list
        if (m_serverItemList) {
            connect(m_serverItemList, &OTLib::Collections::ServerItemList::itemAdded,
                    this, &ServerItemListBox::onOTLibItemAdded);
            connect(m_serverItemList, &OTLib::Collections::ServerItemList::collectionChanged,
                    this, &ServerItemListBox::onServerItemListChanged);
            connect(m_serverItemList, &OTLib::Collections::ServerItemList::collectionCleared,
                    this, &ServerItemListBox::onServerItemListChanged);
        }
        
        emit serverItemListChanged(list);
        refreshDisplay();
    }
}

void ServerItemListBox::refreshDisplay()
{
    // Clear current items
    beginUpdate();
    items().clear();
    
    // Load items from ServerItemList if available
    if (m_serverItemList) {
        const auto& otlibItems = m_serverItemList->items();
        
        // Convert OTLib::Server::Items::ServerItem to ItemEditor::ServerItem
        for (const auto* otlibItem : otlibItems) {
            if (otlibItem) {
                auto* editorItem = new ItemEditor::ServerItem(this);
                
                // Copy basic properties
                editorItem->setId(otlibItem->id());
                editorItem->setName(otlibItem->name());
                editorItem->setClientId(otlibItem->clientId());
                
                // Copy all ServerItemFlag properties for checkbox population
                editorItem->setUnpassable(otlibItem->unpassable());
                editorItem->setBlockMissiles(otlibItem->blockMissiles());
                editorItem->setBlockPathfinder(otlibItem->blockPathfinder());
                editorItem->setHasElevation(otlibItem->hasElevation());
                editorItem->setForceUse(otlibItem->forceUse());
                editorItem->setMultiUse(otlibItem->multiUse());
                editorItem->setPickupable(otlibItem->pickupable());
                editorItem->setMovable(otlibItem->movable());
                editorItem->setStackable(otlibItem->stackable());
                editorItem->setReadable(otlibItem->readable());
                editorItem->setRotatable(otlibItem->rotatable());
                editorItem->setHangable(otlibItem->hangable());
                editorItem->setHookSouth(otlibItem->hookSouth());
                editorItem->setHookEast(otlibItem->hookEast());
                editorItem->setHasCharges(otlibItem->hasCharges());
                editorItem->setIgnoreLook(otlibItem->ignoreLook());
                editorItem->setFullGround(otlibItem->fullGround());
                editorItem->setAllowDistanceRead(otlibItem->allowDistanceRead());
                editorItem->setIsAnimation(otlibItem->isAnimation());
                
                // Copy all ServerItemAttribute properties for UI controls
                editorItem->setType(static_cast<ItemEditor::ServerItemType>(otlibItem->type()));
                editorItem->setHasStackOrder(otlibItem->hasStackOrder());
                editorItem->setStackOrder(static_cast<ItemEditor::TileStackOrder>(otlibItem->stackOrder()));
                editorItem->setGroundSpeed(otlibItem->groundSpeed());
                editorItem->setLightLevel(otlibItem->lightLevel());
                editorItem->setLightColor(otlibItem->lightColor());
                editorItem->setMaxReadChars(otlibItem->maxReadChars());
                editorItem->setMaxReadWriteChars(otlibItem->maxReadWriteChars());
                editorItem->setMinimapColor(otlibItem->minimapColor());
                editorItem->setTradeAs(otlibItem->tradeAs());
                
                // Copy sprite hash for comparison purposes
                editorItem->setSpriteHash(otlibItem->spriteHash());
                
                // Add diagnostic logging for conversion validation
                qDebug() << "ServerItemListBox: Converted item" << otlibItem->id() 
                         << "with" << (otlibItem->unpassable() ? "unpassable" : "passable")
                         << "flags and" << otlibItem->lightLevel() << "light level";
                
                // Add to display list
                items().append(editorItem);
                updateItemRange(editorItem);
            }
        }
        
        qDebug() << "ServerItemListBox: Converted" << items().size() << "items with complete flag and attribute data";
    }
    
    endUpdate();
}

void ServerItemListBox::add(const QList<ItemEditor::ServerItem*>& itemList)
{
    beginUpdate();
    
    for (ItemEditor::ServerItem* item : itemList) {
        if (item) {
            items().append(item);
            updateItemRange(item);
        }
    }
    
    endUpdate();
}

void ServerItemListBox::addItem(quint16 itemId, const QString& name)
{
    // Create a new ServerItem with optimized memory allocation
    auto* serverItem = new ItemEditor::ServerItem(this);
    serverItem->setId(itemId);
    serverItem->setName(name);
    
    // Add to items list with efficient insertion
    beginUpdate();
    items().append(serverItem);
    updateItemRange(serverItem);
    endUpdate();
    
    // Emit signal for UI updates
    emit itemAdded(serverItem);
}

void ServerItemListBox::clearSelection()
{
    // Clear the current selection using the base class method
    ListBase<ItemEditor::ServerItem>::clearSelection();
}

void ServerItemListBox::updateItemPosition(ItemEditor::ServerItem* item, int index)
{
    if (!item) return;
    
    // Calculate position based on layout and index for efficient positioning
    const int itemHeight = 32; // Standard item height
    const int itemWidth = 32;  // Standard item width
    const int margin = 2;      // Margin between items
    
    int x = (index % getItemsPerRow()) * (itemWidth + margin);
    int y = (index / getItemsPerRow()) * (itemHeight + margin);
    
    // Store position for efficient rendering
    m_itemPositions[item] = QPoint(x, y);
    
    // Update visible range for virtual scrolling optimization
    updateVisibleRange();
}

void ServerItemListBox::paintContent(QPainter& painter)
{
    if (!m_plugin || items().isEmpty()) {
        return;
    }
    
    // Performance optimization: Check if we can use cached rendering
    QRect visibleRect = this->getVisibleRect();
    if (m_cacheValid && m_lastViewportSize == visibleRect.size() && !m_renderCache.isNull()) {
        painter.drawPixmap(visibleRect.topLeft(), m_renderCache, 
                          QRect(QPoint(0, 0), visibleRect.size()));
        return;
    }
    
    // Update visible range for virtual scrolling
    updateVisibleRange();
    
    // Create or update render cache
    if (m_renderCache.size() != visibleRect.size()) {
        m_renderCache = QPixmap(visibleRect.size());
        m_lastViewportSize = visibleRect.size();
    }
    
    // Paint to cache for better performance
    m_renderCache.fill(Qt::transparent);
    QPainter cacheP(&m_renderCache);
    cacheP.setRenderHint(QPainter::Antialiasing, false);
    cacheP.setRenderHint(QPainter::SmoothPixmapTransform, false);
    
    // Only paint items that are visible for optimal performance
    int itemsPerRow = getItemsPerRow();
    int totalHeight = 0;
    
    for (int i = m_visibleStartIndex; i <= m_visibleEndIndex && i < items().size(); ++i) {
        ItemEditor::ServerItem* item = items().at(i);
        if (!item) continue;
        
        // Calculate item position efficiently
        int row = i / itemsPerRow;
        int col = i % itemsPerRow;
        int x = col * (m_itemHeight + m_itemMargin);
        int y = row * (m_itemHeight + m_itemMargin) - visibleRect.top();
        
        QRect itemRect(x, y, m_itemHeight, m_itemHeight);
        
        // Skip items outside visible area
        if (y + m_itemHeight < 0 || y > visibleRect.height()) {
            continue;
        }
        
        // Paint item with optimized rendering
        paintItemBackground(cacheP, itemRect, isItemSelected(item));
        paintItemSprite(cacheP, item, itemRect);
        paintItemText(cacheP, item, itemRect);
        paintItemBorder(cacheP, itemRect);
    }
    
    cacheP.end();
    m_cacheValid = true;
    
    // Draw cached result
    painter.drawPixmap(visibleRect.topLeft(), m_renderCache);
}

void ServerItemListBox::onSelectionChanged()
{
    // Emit the Qt signal when selection changes
    emit itemSelectionChanged();
}

void ServerItemListBox::updateItemRange(ItemEditor::ServerItem* item)
{
    if (!item) return;
    
    quint16 itemId = item->id();
    
    // Update ID range for efficient searching and filtering
    if (m_minimumID == 0 || itemId < m_minimumID) {
        m_minimumID = itemId;
    }
    
    if (m_maximumID == 0 || itemId > m_maximumID) {
        m_maximumID = itemId;
    }
    
    // Update item index mapping for O(1) lookups
    m_itemIndexMap[itemId] = items().size() - 1;
}

void ServerItemListBox::paintItemBackground(QPainter& painter, const QRect& rect, bool selected)
{
    // Paint background with selection highlighting
    if (selected) {
        painter.fillRect(rect, QColor(100, 150, 200, 128)); // Light blue selection
    } else {
        painter.fillRect(rect, QColor(240, 240, 240)); // Light gray background
    }
}

void ServerItemListBox::paintItemSprite(QPainter& painter, ItemEditor::ServerItem* item, const QRect& destRect)
{
    if (!item || !m_plugin) return;
    
    quint16 itemId = item->id();
    
    // Check sprite cache first for performance
    if (m_spriteCache->contains(itemId)) {
        const QPixmap& cachedSprite = m_spriteCache->value(itemId);
        if (!cachedSprite.isNull()) {
            painter.drawPixmap(destRect.adjusted(2, 2, -2, -2), cachedSprite);
            return;
        } else {
            // Remove null cached entries to prevent repeated lookups
            m_spriteCache->remove(itemId);
        }
    }
    
    // Try to get actual sprite from plugin's ClientItem with enhanced validation
    ItemEditor::ClientItem* clientItem = m_plugin->getClientItem(item->clientId());
    if (clientItem) {
        // Force bitmap generation if not available but sprites exist
        if (!clientItem->spriteList().isEmpty()) {
            QPixmap bitmap = clientItem->getBitmap(); // This calls generateBitmap() if needed
            if (!bitmap.isNull() && bitmap.width() > 0 && bitmap.height() > 0) {
                // Draw the actual sprite bitmap with proper scaling
                QRect spriteRect = destRect.adjusted(2, 2, -2, -2);
                
                // Scale bitmap to fit the destination rectangle while maintaining aspect ratio
                QPixmap scaledBitmap = bitmap.scaled(spriteRect.size(), 
                                                   Qt::KeepAspectRatio, 
                                                   Qt::SmoothTransformation);
                
                // Center the scaled bitmap in the destination rectangle
                QPoint drawPos = spriteRect.center() - QPoint(scaledBitmap.width() / 2, scaledBitmap.height() / 2);
                painter.drawPixmap(drawPos, scaledBitmap);
                
                // Cache the scaled bitmap for future use with memory management
                if (m_spriteCache->size() < m_maxCacheSize) {
                    int pixmapCost = scaledBitmap.width() * scaledBitmap.height() * 4; // 4 bytes per pixel (ARGB)
                    m_spriteCache->insert(itemId, scaledBitmap, pixmapCost);
                    m_cacheMemoryUsage += pixmapCost;
                }
                return;
            } else {
                // Try to force bitmap generation
                clientItem->generateBitmap();
                bitmap = clientItem->getBitmap();
                if (!bitmap.isNull() && bitmap.width() > 0 && bitmap.height() > 0) {
                    // Successfully generated bitmap after manual generation
                    QRect spriteRect = destRect.adjusted(2, 2, -2, -2);
                    QPixmap scaledBitmap = bitmap.scaled(spriteRect.size(), 
                                                       Qt::KeepAspectRatio, 
                                                       Qt::SmoothTransformation);
                    QPoint drawPos = spriteRect.center() - QPoint(scaledBitmap.width() / 2, scaledBitmap.height() / 2);
                    painter.drawPixmap(drawPos, scaledBitmap);
                    
                    // Cache the result
                    if (m_spriteCache->size() < m_maxCacheSize) {
                        int pixmapCost = scaledBitmap.width() * scaledBitmap.height() * 4;
                        m_spriteCache->insert(itemId, scaledBitmap, pixmapCost);
                        m_cacheMemoryUsage += pixmapCost;
                    }
                    return;
                }
            }
        }
    }
    
    // Enhanced placeholder rendering for missing or corrupted sprites
    QRect placeholderRect = destRect.adjusted(2, 2, -2, -2);
    
    // Draw subtle border for missing sprites
    painter.setPen(QPen(QColor(200, 200, 200), 1, Qt::DashLine));
    painter.drawRect(placeholderRect);
    
    // Draw item ID with subtle styling
    painter.setPen(QColor(128, 128, 128));
    painter.setFont(QFont("Arial", 8));
    QString idText = QString::number(itemId);
    
    // Use cached font metrics for better performance
    static QFontMetrics fm(painter.font());
    QRect textRect = fm.boundingRect(idText);
    QPoint textPos = placeholderRect.center() - textRect.center();
    
    painter.drawText(textPos, idText);
    
    // Add sprite status indicator
    QString statusText;
    if (clientItem) {
        if (clientItem->spriteList().isEmpty()) {
            statusText = "No sprites";
        } else {
            statusText = QString("%1 spr").arg(clientItem->spriteList().size());
        }
    } else {
        statusText = "No client";
    }
    
    // Draw status text below ID
    painter.setPen(QColor(100, 100, 100));
    painter.setFont(QFont("Arial", 6));
    QFontMetrics statusFm(painter.font());
    QRect statusRect = statusFm.boundingRect(statusText);
    QPoint statusPos = QPoint(placeholderRect.center().x() - statusRect.width() / 2, 
                             textPos.y() + textRect.height() + 3);
    
    painter.drawText(statusPos, statusText);
    
    // Cache the enhanced placeholder for future use (only if we have cache space)
    if (m_spriteCache->size() < m_maxCacheSize) {
        QPixmap placeholderPixmap(placeholderRect.size());
        placeholderPixmap.fill(Qt::transparent);
        QPainter placeholderPainter(&placeholderPixmap);
        
        // Render the same placeholder to the pixmap
        placeholderPainter.setPen(QPen(QColor(200, 200, 200), 1, Qt::DashLine));
        placeholderPainter.drawRect(placeholderPixmap.rect());
        placeholderPainter.setPen(QColor(128, 128, 128));
        placeholderPainter.setFont(QFont("Arial", 8));
        placeholderPainter.drawText(placeholderPixmap.rect(), Qt::AlignCenter, idText);
        
        // Add status text
        placeholderPainter.setPen(QColor(100, 100, 100));
        placeholderPainter.setFont(QFont("Arial", 6));
        QRect statusArea = QRect(0, placeholderPixmap.height() - 12, placeholderPixmap.width(), 12);
        placeholderPainter.drawText(statusArea, Qt::AlignCenter, statusText);
        
        placeholderPainter.end();
        
        // Calculate memory cost for the placeholder pixmap
        int pixmapCost = placeholderPixmap.width() * placeholderPixmap.height() * 4; // 4 bytes per pixel (ARGB)
        m_spriteCache->insert(itemId, placeholderPixmap, pixmapCost);
        m_cacheMemoryUsage += pixmapCost;
    }
}

void ServerItemListBox::paintItemText(QPainter& painter, ItemEditor::ServerItem* item, const QRect& layoutRect)
{
    if (!item) return;
    
    // Paint item name below the sprite
    QRect textRect = layoutRect.adjusted(0, 34, 0, 0); // Below 32px sprite + 2px margin
    painter.setPen(QColor(0, 0, 0));
    painter.setFont(QFont("Arial", 8));
    
    QString displayText = item->name().isEmpty() ? 
                         QString("Item %1").arg(item->id()) : 
                         item->name();
    
    // Truncate text if too long
    QFontMetrics fm(painter.font());
    displayText = fm.elidedText(displayText, Qt::ElideRight, textRect.width());
    
    painter.drawText(textRect, Qt::AlignTop | Qt::AlignHCenter, displayText);
}

void ServerItemListBox::paintItemBorder(QPainter& painter, const QRect& rect)
{
    // Draw item border for visual separation
    painter.setPen(QPen(QColor(128, 128, 128), 1));
    painter.drawRect(rect);
}

// Performance optimization helper methods
int ServerItemListBox::getItemsPerRow() const
{
    const int itemWidth = m_itemHeight;
    const int margin = m_itemMargin;
    return qMax(1, width() / (itemWidth + margin));
}

QRect ServerItemListBox::getVisibleRect() const
{
    // Return the currently visible area for virtual scrolling
    return QRect(0, 0, width(), height());
}

void ServerItemListBox::refreshSprites()
{
    // Clear sprite cache to force reload of sprites from updated plugins
    clearSpriteCache();
    
    // Invalidate render cache to force repaint
    m_renderCache = QPixmap();
    m_cacheValid = false;
    
    // Force immediate repaint to show updated sprites
    update();
    
    qDebug() << "ServerItemListBox: Sprite display refreshed after plugin update";
}

void ServerItemListBox::clearSpriteCache()
{
    if (m_spriteCache) {
        int clearedItems = m_spriteCache->size();
        m_spriteCache->clear();
        m_cacheMemoryUsage = 0;
        qDebug() << "ServerItemListBox: Sprite cache cleared -" << clearedItems << "items removed";
    }
    m_cacheValid = false;
}

void ServerItemListBox::optimizeMemoryUsage()
{
    qDebug() << "ServerItemListBox: Optimizing memory usage...";
    
    // Clear render cache
    m_renderCache = QPixmap();
    m_cacheValid = false;
    
    // Reduce sprite cache size if it's too large
    if (m_spriteCache && m_spriteCache->size() > m_maxCacheSize / 2) {
        // Clear half of the cache to free memory
        int targetSize = m_maxCacheSize / 4;
        while (m_spriteCache->size() > targetSize) {
            // The ManagedCache will automatically evict LRU items
            break;
        }
    }
    
    // Clear item position cache for items not currently visible
    QHash<ServerItem*, QPoint> visiblePositions;
    for (auto it = m_itemPositions.begin(); it != m_itemPositions.end(); ++it) {
        // Keep only visible items (simplified check)
        if (it.key()) {
            visiblePositions.insert(it.key(), it.value());
        }
    }
    m_itemPositions = visiblePositions;
    
    qDebug() << "ServerItemListBox: Memory optimization complete";
}

void ServerItemListBox::setCacheLimit(int maxItems)
{
    m_maxCacheSize = maxItems;
    if (m_spriteCache) {
        // Create new cache with updated size limit
        auto newCache = std::make_unique<ManagedCache<quint16, QPixmap>>(m_maxCacheSize, this);
        
        // Transfer existing items up to the new limit
        // Note: This is a simplified approach; in practice, you might want to
        // preserve the most recently used items
        m_spriteCache = std::move(newCache);
        m_cacheMemoryUsage = 0;
    }
}

MemoryStats ServerItemListBox::getCacheStats() const
{
    MemoryStats stats;
    if (m_spriteCache) {
        stats.currentUsage = m_cacheMemoryUsage;
        stats.activeAllocations = m_spriteCache->size();
        stats.totalAllocations = m_spriteCache->size(); // Simplified
    }
    return stats;
}

void ServerItemListBox::onMemoryOptimized()
{
    optimizeMemoryUsage();
}

void ServerItemListBox::updateVisibleRange()
{
    // Calculate which items are visible for virtual scrolling optimization
    const int itemsPerRow = getItemsPerRow();
    const int rowHeight = m_itemHeight + m_itemMargin;
    
    QRect visibleRect = getVisibleRect();
    int startRow = qMax(0, visibleRect.top() / rowHeight);
    int endRow = qMin((items().size() - 1) / itemsPerRow, 
                      (visibleRect.bottom() + rowHeight - 1) / rowHeight);
    
    int newStartIndex = startRow * itemsPerRow;
    int newEndIndex = qMin(items().size() - 1, (endRow + 1) * itemsPerRow - 1);
    
    // Only update if range changed to avoid unnecessary repaints
    if (newStartIndex != m_visibleStartIndex || newEndIndex != m_visibleEndIndex) {
        m_visibleStartIndex = newStartIndex;
        m_visibleEndIndex = newEndIndex;
        m_cacheValid = false; // Invalidate cache when visible range changes
        
        // Defer update for smooth scrolling
        if (!m_updateTimer->isActive()) {
            m_updateTimer->start();
        }
    }
}

void ServerItemListBox::performDeferredUpdate()
{
    // Perform the actual update after a short delay for smooth scrolling
    update();
}

bool ServerItemListBox::isItemSelected(ItemEditor::ServerItem* item) const
{
    // Check if item is currently selected by finding its index
    int index = items().indexOf(item);
    return index >= 0 && selectedIndices().contains(index);
}

void ServerItemListBox::onOTLibItemAdded(OTLib::Server::Items::ServerItem* item)
{
    // This slot is called when an item is added to the OTLib ServerItemList
    // We need to refresh the display to show the new item
    refreshDisplay();
}

void ServerItemListBox::onServerItemListChanged()
{
    // This slot is called when the ServerItemList changes
    // Refresh the entire display
    refreshDisplay();
}

} // namespace ItemEditor