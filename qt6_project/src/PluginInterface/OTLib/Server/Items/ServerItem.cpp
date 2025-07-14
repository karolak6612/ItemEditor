/**
 * Item Editor Qt6 - Server Item Implementation
 * Complete implementation based on Legacy_App/csharp/Source/PluginInterface/OTLib/Server/Items/ServerItem.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "ServerItem.h"
#include <QMetaProperty>

namespace OTLib {
namespace Server {
namespace Items {

// Item class implementation
Item::Item(QObject *parent)
    : QObject(parent)
    , m_id(0)
    , m_type(ServerItemType::None)
    , m_hasStackOrder(false)
    , m_stackOrder(TileStackOrder::None)
    , m_unpassable(false)
    , m_blockMissiles(false)
    , m_blockPathfinder(false)
    , m_hasElevation(false)
    , m_forceUse(false)
    , m_multiUse(false)
    , m_pickupable(false)
    , m_movable(true)  // Default to true as in C# implementation
    , m_stackable(false)
    , m_readable(false)
    , m_rotatable(false)
    , m_hangable(false)
    , m_hookSouth(false)
    , m_hookEast(false)
    , m_hasCharges(false)
    , m_ignoreLook(false)
    , m_fullGround(false)
    , m_allowDistanceRead(false)
    , m_isAnimation(false)
    , m_groundSpeed(0)
    , m_lightLevel(0)
    , m_lightColor(0)
    , m_maxReadChars(0)
    , m_maxReadWriteChars(0)
    , m_minimapColor(0)
    , m_tradeAs(0)
{
}

bool Item::equals(const Item* item) const
{
    if (!item) return false;
    
    return (m_type == item->m_type &&
            m_stackOrder == item->m_stackOrder &&
            m_unpassable == item->m_unpassable &&
            m_blockMissiles == item->m_blockMissiles &&
            m_blockPathfinder == item->m_blockPathfinder &&
            m_hasElevation == item->m_hasElevation &&
            m_forceUse == item->m_forceUse &&
            m_multiUse == item->m_multiUse &&
            m_pickupable == item->m_pickupable &&
            m_movable == item->m_movable &&
            m_stackable == item->m_stackable &&
            m_readable == item->m_readable &&
            m_rotatable == item->m_rotatable &&
            m_hangable == item->m_hangable &&
            m_hookSouth == item->m_hookSouth &&
            m_hookEast == item->m_hookEast &&
            m_ignoreLook == item->m_ignoreLook &&
            m_fullGround == item->m_fullGround &&
            m_isAnimation == item->m_isAnimation &&
            m_groundSpeed == item->m_groundSpeed &&
            m_lightLevel == item->m_lightLevel &&
            m_lightColor == item->m_lightColor &&
            m_maxReadChars == item->m_maxReadChars &&
            m_maxReadWriteChars == item->m_maxReadWriteChars &&
            m_minimapColor == item->m_minimapColor &&
            m_tradeAs == item->m_tradeAs &&
            m_name.compare(item->m_name) == 0);
}

bool Item::hasProperties(ServerItemFlags flags) const
{
    if (flags == ServerItemFlag::None) return false;
    if (flags.testFlag(ServerItemFlag::Unpassable) && !m_unpassable) return false;
    if (flags.testFlag(ServerItemFlag::BlockMissiles) && !m_blockMissiles) return false;
    if (flags.testFlag(ServerItemFlag::BlockPathfinder) && !m_blockPathfinder) return false;
    if (flags.testFlag(ServerItemFlag::HasElevation) && !m_hasElevation) return false;
    if (flags.testFlag(ServerItemFlag::ForceUse) && !m_forceUse) return false;
    if (flags.testFlag(ServerItemFlag::MultiUse) && !m_multiUse) return false;
    if (flags.testFlag(ServerItemFlag::Pickupable) && !m_pickupable) return false;
    if (flags.testFlag(ServerItemFlag::Movable) && !m_movable) return false;
    if (flags.testFlag(ServerItemFlag::Stackable) && !m_stackable) return false;
    if (flags.testFlag(ServerItemFlag::Readable) && !m_readable) return false;
    if (flags.testFlag(ServerItemFlag::Rotatable) && !m_rotatable) return false;
    if (flags.testFlag(ServerItemFlag::Hangable) && !m_hangable) return false;
    if (flags.testFlag(ServerItemFlag::HookSouth) && !m_hookSouth) return false;
    if (flags.testFlag(ServerItemFlag::HookEast) && !m_hookEast) return false;
    if (flags.testFlag(ServerItemFlag::AllowDistanceRead) && !m_allowDistanceRead) return false;
    if (flags.testFlag(ServerItemFlag::IgnoreLook) && !m_ignoreLook) return false;
    if (flags.testFlag(ServerItemFlag::FullGround) && !m_fullGround) return false;
    if (flags.testFlag(ServerItemFlag::IsAnimation) && !m_isAnimation) return false;
    return true;
}

void Item::copyPropertiesFrom(const Item* item)
{
    if (!item) return;
    
    // Copy all properties using Qt's meta-object system
    const QMetaObject* metaObject = this->metaObject();
    const QMetaObject* sourceMetaObject = item->metaObject();
    
    for (int i = 0; i < sourceMetaObject->propertyCount(); ++i) {
        QMetaProperty sourceProperty = sourceMetaObject->property(i);
        
        // Skip spriteHash as mentioned in C# implementation
        if (QString(sourceProperty.name()) == "spriteHash") {
            continue;
        }
        
        // Find corresponding property in target object
        int targetIndex = metaObject->indexOfProperty(sourceProperty.name());
        if (targetIndex == -1) continue;
        
        QMetaProperty targetProperty = metaObject->property(targetIndex);
        if (!targetProperty.isWritable()) continue;
        
        // Copy the property value
        QVariant value = sourceProperty.read(item);
        targetProperty.write(this, value);
    }
}

// ServerItem class implementation
ServerItem::ServerItem(QObject *parent)
    : Item(parent)
    , m_clientId(0)
    , m_previousClientId(0)
    , m_spriteAssigned(false)
    , m_isCustomCreated(false)
{
}

ServerItem::ServerItem(const Item* item, QObject *parent)
    : Item(parent)
    , m_clientId(0)
    , m_previousClientId(0)
    , m_spriteAssigned(false)
    , m_isCustomCreated(false)
{
    if (item) {
        copyPropertiesFrom(item);
    }
}

QString ServerItem::toString() const
{
    if (!m_nameXml.isEmpty()) {
        return QString("%1 - %2").arg(m_id).arg(m_nameXml);
    } else if (!m_name.isEmpty()) {
        return QString("%1 - %2").arg(m_id).arg(m_name);
    }
    
    return QString::number(m_id);
}

} // namespace Items
} // namespace Server
} // namespace OTLib