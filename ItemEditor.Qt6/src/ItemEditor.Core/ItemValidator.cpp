#include "ItemValidator.h"
#include <QDebug>

bool ItemValidator::validateItem(const ServerItem& item)
{
    return getAllValidationErrors(item).isEmpty();
}

QString ItemValidator::getValidationError(const ServerItem& item)
{
    QStringList errors = getAllValidationErrors(item);
    return errors.isEmpty() ? QString() : errors.first();
}

QStringList ItemValidator::getAllValidationErrors(const ServerItem& item)
{
    QStringList errors;
    
    // Basic property validation
    if (!validateItemId(item.id)) {
        errors << formatValidationError("ID", QString("Invalid item ID: %1").arg(item.id));
    }
    
    if (!validateItemType(item.type)) {
        errors << formatValidationError("Type", "Invalid item type");
    }
    
    if (!validateItemName(item.name)) {
        errors << formatValidationError("Name", "Invalid item name");
    }
    
    if (!validateDimensions(item.width, item.height)) {
        errors << formatValidationError("Dimensions", QString("Invalid dimensions: %1x%2").arg(item.width).arg(item.height));
    }
    
    if (!validateFlags(item.flags)) {
        errors << formatValidationError("Flags", "Invalid flag combination");
    }
    
    if (!validateSpeed(item.speed)) {
        errors << formatValidationError("Speed", QString("Invalid speed: %1").arg(item.speed));
    }
    
    if (!validateLight(item.lightLevel, item.lightColor)) {
        errors << formatValidationError("Light", QString("Invalid light: level=%1, color=%2").arg(item.lightLevel).arg(item.lightColor));
    }
    
    // Advanced consistency validation
    if (!validateItemConsistency(item)) {
        errors << "Item configuration is inconsistent";
    }
    
    return errors;
}

bool ItemValidator::validateClientItem(const ClientItem& item)
{
    return getClientValidationErrors(item).isEmpty();
}

QStringList ItemValidator::getClientValidationErrors(const ClientItem& item)
{
    QStringList errors = getAllValidationErrors(item);
    
    // Client-specific validation
    if (!validateSpriteData(item)) {
        errors << formatValidationError("Sprite Data", "Invalid sprite data");
    }
    
    if (item.animationPhases < 1) {
        errors << formatValidationError("Animation Phases", "Must be at least 1");
    }
    
    if (item.xDiv < 1 || item.yDiv < 1 || item.zDiv < 1) {
        errors << formatValidationError("Division", "Division values must be at least 1");
    }
    
    // Validate sprite count consistency
    if (item.hasSprites()) {
        int expectedCount = item.width * item.height * item.layers * 
                           item.patternX * item.patternY * item.patternZ * 
                           item.frames * item.animationPhases;
        if (item.getSpriteCount() != expectedCount) {
            errors << formatValidationError("Sprite Count", 
                QString("Expected %1 sprites, found %2").arg(expectedCount).arg(item.getSpriteCount()));
        }
    }
    
    return errors;
}

bool ItemValidator::validateItemId(ItemId id)
{
    return id >= getMinItemId() && id <= getMaxItemId();
}

bool ItemValidator::validateItemType(ServerItemType type)
{
    return isValidItemType(type);
}

bool ItemValidator::validateItemName(const QString& name)
{
    return !name.isEmpty() && name.length() <= getMaxNameLength();
}

bool ItemValidator::validateDimensions(quint8 width, quint8 height)
{
    return width >= 1 && width <= getMaxDimension() &&
           height >= 1 && height <= getMaxDimension();
}

bool ItemValidator::validateFlags(quint32 flags)
{
    return hasValidFlagCombination(flags);
}

bool ItemValidator::validateSpeed(quint16 speed)
{
    return speed <= getMaxSpeed();
}

bool ItemValidator::validateLight(quint16 level, quint16 color)
{
    return level <= getMaxLightLevel() && color <= 65535;
}

bool ItemValidator::validateItemConsistency(const ServerItem& item)
{
    // Type-specific validation
    switch (item.type) {
        case ServerItemType::Weapon:
            return isValidWeaponConfiguration(item);
        case ServerItemType::Container:
            return isValidContainerConfiguration(item);
        case ServerItemType::Fluid:
            return isValidFluidConfiguration(item);
        default:
            return true;
    }
}

bool ItemValidator::validateSpriteData(const ClientItem& item)
{
    if (!item.hasSprites()) {
        return true; // No sprites is valid
    }
    
    // Check if all sprites have data
    for (int i = 0; i < item.getSpriteCount(); ++i) {
        if (item.getSpriteData(i).isEmpty()) {
            return false;
        }
    }
    
    // Verify sprite hash if present
    if (!item.spriteHash.isEmpty()) {
        return item.verifySpriteHash();
    }
    
    return true;
}

bool ItemValidator::validateItemCompatibility(const ServerItem& serverItem, const ClientItem& clientItem)
{
    // Check if server and client items are compatible
    if (serverItem.clientId != clientItem.id) {
        return false;
    }
    
    if (serverItem.width != clientItem.width || serverItem.height != clientItem.height) {
        return false;
    }
    
    if (!serverItem.spriteHash.isEmpty() && !clientItem.spriteHash.isEmpty()) {
        return serverItem.spriteHash == clientItem.spriteHash;
    }
    
    return true;
}

ItemId ItemValidator::getMinItemId()
{
    return 1;
}

ItemId ItemValidator::getMaxItemId()
{
    return 65535;
}

int ItemValidator::getMaxNameLength()
{
    return 255;
}

int ItemValidator::getMaxDescriptionLength()
{
    return 1024;
}

quint8 ItemValidator::getMaxDimension()
{
    return 10;
}

quint16 ItemValidator::getMaxSpeed()
{
    return 65535;
}

quint16 ItemValidator::getMaxLightLevel()
{
    return 255;
}

QString ItemValidator::formatValidationError(const QString& property, const QString& error)
{
    return QString("[ERROR] %1: %2").arg(property, error);
}

QString ItemValidator::formatValidationWarning(const QString& property, const QString& warning)
{
    return QString("[WARNING] %1: %2").arg(property, warning);
}

bool ItemValidator::isValidItemType(ServerItemType type)
{
    return type >= ServerItemType::None && type <= ServerItemType::Deprecated;
}

bool ItemValidator::isValidStackOrder(TileStackOrder order)
{
    return order >= TileStackOrder::None && order <= TileStackOrder::Top;
}

bool ItemValidator::hasValidFlagCombination(quint32 flags)
{
    // Check for conflicting flags
    if ((flags & static_cast<quint32>(ItemFlag::Unpassable)) && 
        (flags & static_cast<quint32>(ItemFlag::Pickupable))) {
        return false; // Can't be both unpassable and pickupable
    }
    
    if ((flags & static_cast<quint32>(ItemFlag::Stackable)) && 
        (flags & static_cast<quint32>(ItemFlag::MultiUse))) {
        return false; // Stackable items shouldn't be multi-use
    }
    
    return true;
}

bool ItemValidator::isValidWeaponConfiguration(const ServerItem& item)
{
    // Weapons should have attack value
    if (item.attack == 0) {
        return false;
    }
    
    // Weapons should have appropriate flags
    if (!(item.flags & static_cast<quint32>(ItemFlag::Pickupable))) {
        return false;
    }
    
    return true;
}

bool ItemValidator::isValidContainerConfiguration(const ServerItem& item)
{
    // Containers should have size
    if (item.containerSize == 0) {
        return false;
    }
    
    // Containers should be pickupable or unpassable
    if (!(item.flags & static_cast<quint32>(ItemFlag::Pickupable)) &&
        !(item.flags & static_cast<quint32>(ItemFlag::Unpassable))) {
        return false;
    }
    
    return true;
}

bool ItemValidator::isValidFluidConfiguration(const ServerItem& item)
{
    // Fluids should have a fluid source
    if (item.fluidSource == 0) {
        return false;
    }
    
    return true;
}