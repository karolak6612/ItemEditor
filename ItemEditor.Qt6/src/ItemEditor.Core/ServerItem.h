#pragma once

#include <QString>
#include <QByteArray>
#include <QDateTime>
#include "ItemTypes.h"
#include "ItemEnums.h"

/**
 * @brief Server item data structure
 * 
 * Represents a server item with all properties from the legacy system.
 * Maintains exact compatibility with the original data structure.
 */
class ServerItem
{
public:
    ServerItem();
    ServerItem(const ServerItem& other);
    ServerItem& operator=(const ServerItem& other);
    virtual ~ServerItem();

    // Core identification
    ItemId id;
    ClientId clientId;
    ClientId previousClientId;
    
    // Item properties
    ServerItemType type;
    TileStackOrder stackOrder;
    QString name;
    QString description;
    QString article;
    QString plural;
    
    // Sprite information
    QByteArray spriteHash;
    quint8 width;
    quint8 height;
    quint8 layers;
    quint8 patternX;
    quint8 patternY;
    quint8 patternZ;
    quint8 frames;
    
    // Item flags and attributes
    quint32 flags;
    quint16 speed;
    quint16 lightLevel;
    quint16 lightColor;
    quint16 minimapColor;
    quint8 elevation;
    
    // Trade and market properties
    quint16 tradeAs;
    bool showAs;
    
    // Weapon properties
    quint8 weaponType;
    quint8 ammoType;
    quint8 shootType;
    quint8 effect;
    quint8 distanceEffect;
    
    // Armor and protection
    quint16 armor;
    quint16 defense;
    quint16 extraDefense;
    quint16 attack;
    quint16 rotateTo;
    
    // Container properties
    quint16 containerSize;
    
    // Fluid properties
    quint8 fluidSource;
    
    // Readable properties
    quint16 maxReadWriteChars;
    quint16 maxReadChars;
    
    // Writable properties
    quint16 maxWriteChars;
    
    // Custom properties
    bool isCustomCreated;
    bool hasClientData;
    QDateTime lastModified;
    QString modifiedBy;
    
    // Validation and comparison
    bool isValid() const;
    bool hasChanges() const;
    void markAsModified();
    void clearModified();
    
    // Property validation
    bool validateProperty(const QString& propertyName, const QVariant& value) const;
    QStringList getValidationErrors() const;
    
    // Comparison with client data
    bool compareWithClient(const ServerItem& clientItem) const;
    QStringList getMismatches(const ServerItem& clientItem) const;
    
    // Serialization support
    QByteArray serialize() const;
    bool deserialize(const QByteArray& data);
    
    // Property access helpers
    QVariant getProperty(const QString& propertyName) const;
    bool setProperty(const QString& propertyName, const QVariant& value);
    QStringList getPropertyNames() const;

private:
    bool m_isModified;
    void copyFrom(const ServerItem& other);
    void initializeDefaults();
};