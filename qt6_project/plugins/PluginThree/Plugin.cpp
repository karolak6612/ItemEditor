/**
 * Item Editor Qt6 - PluginThree Implementation
 * Exact mirror of Legacy_App/csharp/Source/PluginThree/Plugin.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "Plugin.h"
#include "Item.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>

namespace PluginThree {

Plugin::Plugin(QObject *parent)
    : QObject(parent)
    , m_host(nullptr)
    , m_settings(new ItemEditor::Settings())
    , m_items(new ItemEditor::ClientItems())
    , m_itemCount(0)
    , m_loaded(false)
{
    qDebug() << "PluginThree: Constructor called";
}

Plugin::~Plugin()
{
    dispose();
    delete m_settings;
    delete m_items;
    
    // Clean up sprites
    for (auto it = m_sprites.begin(); it != m_sprites.end(); ++it) {
        delete it.value();
    }
    m_sprites.clear();
    
    qDebug() << "PluginThree: Destructor called";
}

ItemEditor::IPluginHost* Plugin::host() const
{
    return m_host;
}

QString Plugin::name() const
{
    return QString("PluginThree");
}

void Plugin::setHost(ItemEditor::IPluginHost* host)
{
    m_host = host;
}

ItemEditor::ClientItems* Plugin::items() const
{
    return m_items;
}

quint16 Plugin::minItemId() const
{
    return 100;
}

quint16 Plugin::maxItemId() const
{
    return m_itemCount;
}

QList<ItemEditor::SupportedClient> Plugin::supportedClients() const
{
    return m_supportedClients;
}

bool Plugin::loaded() const
{
    return m_loaded;
}

bool Plugin::loadClient(const ItemEditor::SupportedClient& client, 
                       bool extended, 
                       bool frameDurations, 
                       bool transparency,
                       const QString& datFullPath, 
                       const QString& sprFullPath)
{
    qDebug() << "PluginThree: Loading client" << client.description();
    
    if (m_loaded) {
        dispose();
    }

    if (!loadDat(datFullPath, client, extended, frameDurations)) {
        qDebug() << "PluginThree: Failed to load dat file:" << datFullPath;
        return false;
    }

    if (!loadSprites(sprFullPath, client, extended, transparency)) {
        qDebug() << "PluginThree: Failed to load spr file:" << sprFullPath;
        return false;
    }

    m_loaded = true;
    emit clientLoaded(client);
    qDebug() << "PluginThree: Client loaded successfully";
    return true;
}

void Plugin::initialize()
{
    qDebug() << "PluginThree: Initializing plugin";
    
    if (m_settings->load("PluginThree.xml")) {
        m_supportedClients = m_settings->getSupportedClientList();
        qDebug() << "PluginThree: Loaded" << m_supportedClients.size() << "supported clients";
    } else {
        qDebug() << "PluginThree: Failed to load settings, creating default supported clients";
        
        // Create some default supported clients for demonstration
        ItemEditor::SupportedClient client1(900, "Tibia 9.00", 0, 0x4E119CC3, 0x4E119CC3);
        ItemEditor::SupportedClient client2(1000, "Tibia 10.00", 0, 0x4E119CC4, 0x4E119CC4);
        
        m_supportedClients.append(client1);
        m_supportedClients.append(client2);
    }
    
    emit pluginLoaded();
}

ItemEditor::SupportedClient Plugin::getClientBySignatures(quint32 datSignature, quint32 sprSignature)
{
    for (const auto& client : m_supportedClients) {
        if (client.datSignature() == datSignature && client.sprSignature() == sprSignature) {
            return client;
        }
    }
    
    return ItemEditor::SupportedClient(); // Return default constructed client if not found
}

ItemEditor::ClientItem* Plugin::getClientItem(quint16 id)
{
    if (m_loaded && id >= 100 && id <= m_itemCount) {
        return m_items->value(id, nullptr);
    }
    
    return nullptr;
}

bool Plugin::loadSprites(const QString& filename, const ItemEditor::SupportedClient& client, bool extended, bool transparency)
{
    qDebug() << "PluginThree: Loading sprites from" << filename;
    
    // Load sprite data using the complete Sprite::loadSprites implementation
    if (!ItemEditor::Sprite::loadSprites(filename, m_sprites, client, extended, transparency)) {
        qDebug() << "PluginThree: Failed to load sprite data from" << filename;
        return false;
    }
    
    qDebug() << "PluginThree: Loaded" << m_sprites.size() << "sprites, now populating ClientItem bitmaps";
    
    // Populate ClientItem bitmaps with actual sprite data
    int processedItems = 0;
    int itemsWithSprites = 0;
    int itemsWithValidBitmaps = 0;
    
    for (auto it = m_items->begin(); it != m_items->end(); ++it) {
        ItemEditor::ClientItem* clientItem = it.value();
        if (!clientItem || clientItem->spriteList().isEmpty()) {
            continue;
        }
        
        itemsWithSprites++;
        
        // Use the proper generateBitmap() method for multi-sprite composition
        // This handles width, height, layers, patterns, and frames correctly
        clientItem->generateBitmap();
        
        // Verify bitmap was generated successfully
        QPixmap bitmap = clientItem->getBitmap();
        if (!bitmap.isNull()) {
            processedItems++;
            itemsWithValidBitmaps++;
            
            // Debug: Log successful bitmap creation for first few items
            if (processedItems <= 5) {
                qDebug() << QString("PluginThree: Generated bitmap for item %1, size %2x%3, sprites: %4")
                            .arg(clientItem->id()).arg(bitmap.width()).arg(bitmap.height())
                            .arg(clientItem->spriteList().size());
            }
        } else {
            // Debug: Log when bitmap generation failed
            if (itemsWithSprites <= 10) {
                qDebug() << QString("PluginThree: Failed to generate bitmap for item %1 with %2 sprites")
                            .arg(clientItem->id()).arg(clientItem->spriteList().size());
            }
        }
        
        // Emit progress for UI updates (based on items with sprites)
        if (itemsWithSprites % 100 == 0) {
            emit loadingProgress((itemsWithSprites * 100) / m_items->size());
        }
    }
    
    emit loadingProgress(100);
    qDebug() << QString("PluginThree: Successfully generated %1 valid bitmaps out of %2 items with sprites")
                .arg(itemsWithValidBitmaps).arg(itemsWithSprites);
    return true;
}

bool Plugin::loadDat(const QString& filename, const ItemEditor::SupportedClient& client, bool extended, bool frameDurations)
{
    qDebug() << "PluginThree: Loading DAT from" << filename;
    
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "PluginThree: Cannot open DAT file:" << filename;
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    // Read DAT signature
    quint32 datSignature;
    stream >> datSignature;
    
    if (client.datSignature() != datSignature) {
        qDebug() << QString("PluginThree: Bad dat signature. Expected %1, got %2")
                    .arg(client.datSignature(), 0, 16)
                    .arg(datSignature, 0, 16);
        return false;
    }

    // Read item count
    stream >> m_itemCount;
    
    // Skip outfit, effect, and missile counts
    quint16 dummy;
    stream >> dummy; // outfits
    stream >> dummy; // effects  
    stream >> dummy; // missiles

    qDebug() << "PluginThree: Loading" << m_itemCount << "items";

    // Load items (simplified implementation with PluginThree specific flags)
    for (quint16 id = 100; id <= m_itemCount; ++id) {
        ItemEditor::ClientItem* item = new ItemEditor::ClientItem();
        item->setId(id);
        m_items->insert(id, item);

        // Read item flags (includes PluginThree specific flags)
        quint8 flag;
        do {
            stream >> flag;
            
            // Handle specific flags that require additional data
            switch (static_cast<ItemFlag>(flag)) {
                case ItemFlag::Ground:
                    stream >> dummy; // ground speed
                    break;
                case ItemFlag::Writable:
                case ItemFlag::WritableOnce:
                    stream >> dummy; // max chars
                    break;
                case ItemFlag::HasLight:
                    stream >> dummy; // light level
                    stream >> dummy; // light color
                    break;
                case ItemFlag::HasOffset:
                    stream >> dummy; // offset x
                    stream >> dummy; // offset y
                    break;
                case ItemFlag::HasElevation:
                    stream >> dummy; // height
                    break;
                case ItemFlag::Minimap:
                    stream >> dummy; // minimap color
                    break;
                case ItemFlag::LensHelp:
                    stream >> dummy; // lens help
                    break;
                case ItemFlag::Cloth:
                    stream >> dummy; // cloth slot
                    break;
                case ItemFlag::Market:
                    {
                        stream >> dummy; // category
                        stream >> dummy; // trade as
                        stream >> dummy; // show as
                        quint16 nameLength;
                        stream >> nameLength;
                        QByteArray nameData(nameLength, 0);
                        stream.readRawData(nameData.data(), nameLength);
                        // item name would be set here: QString::fromUtf8(nameData)
                        stream >> dummy; // profession
                        stream >> dummy; // level
                    }
                    break;
                case ItemFlag::DefaultAction:
                    stream >> dummy; // default action
                    break;
                case ItemFlag::Wrappable:
                case ItemFlag::Unwrappable:
                case ItemFlag::TopEffect:
                case ItemFlag::Usable:
                    // These flags don't require additional data
                    break;
                default:
                    break;
            }
        } while (static_cast<ItemFlag>(flag) != ItemFlag::LastFlag);

        // Read sprite dimensions and data
        quint8 width, height, layers, patternX, patternY, patternZ, frames;
        stream >> width >> height;
        
        if (width > 1 || height > 1) {
            stream >> dummy; // skip exact size
        }
        
        stream >> layers >> patternX >> patternY >> patternZ >> frames;
        
        quint32 numSprites = width * height * layers * patternX * patternY * patternZ * frames;
        
        if (frames > 1 && frameDurations) {
            // Skip frame duration data
            stream.skipRawData(6 + 8 * frames);
        }

        // Read sprite IDs (PluginThree uses 32-bit sprite IDs)
        for (quint32 i = 0; i < numSprites; ++i) {
            quint32 spriteId;
            stream >> spriteId; // Always 32-bit for PluginThree
            
            // Create sprite if it doesn't exist
            if (!m_sprites.contains(spriteId)) {
                ItemEditor::Sprite* sprite = new ItemEditor::Sprite();
                sprite->setId(spriteId);
                m_sprites.insert(spriteId, sprite);
            }
            
            // CRITICAL FIX: Add sprite to item's sprite list for bitmap generation
            if (m_sprites.contains(spriteId)) {
                item->addSprite(m_sprites[spriteId]);
                // Debug: Log sprite addition for verification
                if (id <= 105) { // Only log first few items to avoid spam
                    qDebug() << QString("PluginThree: Added sprite %1 to item %2, sprite list size: %3")
                                .arg(spriteId).arg(id).arg(item->spriteList().size());
                }
            }
        }
        
        if (id % 1000 == 0) {
            emit loadingProgress((id * 100) / m_itemCount);
        }
    }

    emit loadingProgress(100);
    qDebug() << "PluginThree: DAT loading completed";
    return true;
}

void Plugin::dispose()
{
    if (m_loaded) {
        m_sprites.clear();
        m_items->clear();
        m_itemCount = 0;
        m_loaded = false;
        emit pluginUnloaded();
    }
}

} // namespace PluginThree