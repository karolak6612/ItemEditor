#pragma once

#include "ServerItem.h"
#include "ClientItem.h"
#include <QStringList>

/**
 * @brief Item validation utilities
 * 
 * Provides comprehensive validation functions for data integrity
 * matching the legacy system's validation rules and constraints.
 */
class ItemValidator
{
public:
    // Basic validation
    static bool validateItem(const ServerItem& item);
    static QString getValidationError(const ServerItem& item);
    static QStringList getAllValidationErrors(const ServerItem& item);
    
    // Client item validation
    static bool validateClientItem(const ClientItem& item);
    static QStringList getClientValidationErrors(const ClientItem& item);
    
    // Property-specific validation
    static bool validateItemId(ItemId id);
    static bool validateItemType(ServerItemType type);
    static bool validateItemName(const QString& name);
    static bool validateDimensions(quint8 width, quint8 height);
    static bool validateFlags(quint32 flags);
    static bool validateSpeed(quint16 speed);
    static bool validateLight(quint16 level, quint16 color);
    
    // Advanced validation
    static bool validateItemConsistency(const ServerItem& item);
    static bool validateSpriteData(const ClientItem& item);
    static bool validateItemCompatibility(const ServerItem& serverItem, const ClientItem& clientItem);
    
    // Validation rules and constraints
    static ItemId getMinItemId();
    static ItemId getMaxItemId();
    static int getMaxNameLength();
    static int getMaxDescriptionLength();
    static quint8 getMaxDimension();
    static quint16 getMaxSpeed();
    static quint16 getMaxLightLevel();
    
    // Error message formatting
    static QString formatValidationError(const QString& property, const QString& error);
    static QString formatValidationWarning(const QString& property, const QString& warning);

private:
    ItemValidator() = delete;
    
    // Internal validation helpers
    static bool isValidItemType(ServerItemType type);
    static bool isValidStackOrder(TileStackOrder order);
    static bool hasValidFlagCombination(quint32 flags);
    static bool isValidWeaponConfiguration(const ServerItem& item);
    static bool isValidContainerConfiguration(const ServerItem& item);
    static bool isValidFluidConfiguration(const ServerItem& item);
};