#include "ServerItem.h"
#include <QDataStream>
#include <QIODevice>
#include <QVariant>
#include <QMetaProperty>
#include <QDebug>

ServerItem::ServerItem()
    : id(0)
    , clientId(0)
    , previousClientId(0)
    , type(ServerItemType::None)
    , stackOrder(TileStackOrder::None)
    , width(1)
    , height(1)
    , layers(1)
    , patternX(1)
    , patternY(1)
    , patternZ(1)
    , frames(1)
    , flags(0)
    , speed(0)
    , lightLevel(0)
    , lightColor(0)
    , minimapColor(0)
    , elevation(0)
    , tradeAs(0)
    , showAs(false)
    , weaponType(0)
    , ammoType(0)
    , shootType(0)
    , effect(0)
    , distanceEffect(0)
    , armor(0)
    , defense(0)
    , extraDefense(0)
    , attack(0)
    , rotateTo(0)
    , containerSize(0)
    , fluidSource(0)
    , maxReadWriteChars(0)
    , maxReadChars(0)
    , maxWriteChars(0)
    , isCustomCreated(false)
    , hasClientData(false)
    , lastModified(QDateTime::currentDateTime())
    , m_isModified(false)
{
    initializeDefaults();
}

ServerItem::ServerItem(const ServerItem& other)
    : m_isModified(false)
{
    copyFrom(other);
}

ServerItem& ServerItem::operator=(const ServerItem& other)
{
    if (this != &other) {
        copyFrom(other);
    }
    return *this;
}

ServerItem::~ServerItem()
{
}

bool ServerItem::isValid() const
{
    return id > 0 && type != ServerItemType::None;
}

bool ServerItem::hasChanges() const
{
    return m_isModified;
}

void ServerItem::markAsModified()
{
    m_isModified = true;
}

void ServerItem::clearModified()
{
    m_isModified = false;
}

bool ServerItem::validateProperty(const QString& propertyName, const QVariant& value) const
{
    if (propertyName == "id") {
        return value.toUInt() > 0 && value.toUInt() <= 65535;
    } else if (propertyName == "clientId") {
        return value.toUInt() <= 65535;
    } else if (propertyName == "type") {
        quint8 typeVal = value.toUInt();
        return typeVal <= static_cast<quint8>(ServerItemType::Deprecated);
    } else if (propertyName == "stackOrder") {
        quint8 orderVal = value.toUInt();
        return orderVal <= static_cast<quint8>(TileStackOrder::Top);
    } else if (propertyName == "name") {
        return !value.toString().isEmpty() && value.toString().length() <= 255;
    } else if (propertyName == "width" || propertyName == "height") {
        quint8 val = value.toUInt();
        return val >= 1 && val <= 10;
    } else if (propertyName == "speed") {
        return value.toUInt() <= 65535;
    } else if (propertyName == "lightLevel") {
        return value.toUInt() <= 255;
    } else if (propertyName == "lightColor") {
        return value.toUInt() <= 65535;
    }
    
    return true; // Default to valid for unknown properties
}

QStringList ServerItem::getValidationErrors() const
{
    QStringList errors;
    
    if (id == 0) {
        errors << "Item ID must be greater than 0";
    }
    
    if (type == ServerItemType::None) {
        errors << "Item type must be specified";
    }
    
    if (name.isEmpty()) {
        errors << "Item name cannot be empty";
    }
    
    if (name.length() > 255) {
        errors << "Item name cannot exceed 255 characters";
    }
    
    if (width < 1 || width > 10) {
        errors << "Item width must be between 1 and 10";
    }
    
    if (height < 1 || height > 10) {
        errors << "Item height must be between 1 and 10";
    }
    
    if (layers < 1 || layers > 10) {
        errors << "Item layers must be between 1 and 10";
    }
    
    if (frames < 1) {
        errors << "Item frames must be at least 1";
    }
    
    return errors;
}

bool ServerItem::compareWithClient(const ServerItem& clientItem) const
{
    return clientId == clientItem.id &&
           width == clientItem.width &&
           height == clientItem.height &&
           layers == clientItem.layers &&
           patternX == clientItem.patternX &&
           patternY == clientItem.patternY &&
           patternZ == clientItem.patternZ &&
           frames == clientItem.frames &&
           spriteHash == clientItem.spriteHash;
}

QStringList ServerItem::getMismatches(const ServerItem& clientItem) const
{
    QStringList mismatches;
    
    if (clientId != clientItem.id) {
        mismatches << QString("Client ID mismatch: %1 vs %2").arg(clientId).arg(clientItem.id);
    }
    
    if (width != clientItem.width) {
        mismatches << QString("Width mismatch: %1 vs %2").arg(width).arg(clientItem.width);
    }
    
    if (height != clientItem.height) {
        mismatches << QString("Height mismatch: %1 vs %2").arg(height).arg(clientItem.height);
    }
    
    if (layers != clientItem.layers) {
        mismatches << QString("Layers mismatch: %1 vs %2").arg(layers).arg(clientItem.layers);
    }
    
    if (spriteHash != clientItem.spriteHash) {
        mismatches << "Sprite hash mismatch";
    }
    
    return mismatches;
}

QVariant ServerItem::getProperty(const QString& propertyName) const
{
    if (propertyName == "id") return id;
    if (propertyName == "clientId") return clientId;
    if (propertyName == "previousClientId") return previousClientId;
    if (propertyName == "type") return static_cast<quint8>(type);
    if (propertyName == "stackOrder") return static_cast<quint8>(stackOrder);
    if (propertyName == "name") return name;
    if (propertyName == "description") return description;
    if (propertyName == "article") return article;
    if (propertyName == "plural") return plural;
    if (propertyName == "width") return width;
    if (propertyName == "height") return height;
    if (propertyName == "layers") return layers;
    if (propertyName == "patternX") return patternX;
    if (propertyName == "patternY") return patternY;
    if (propertyName == "patternZ") return patternZ;
    if (propertyName == "frames") return frames;
    if (propertyName == "flags") return flags;
    if (propertyName == "speed") return speed;
    if (propertyName == "lightLevel") return lightLevel;
    if (propertyName == "lightColor") return lightColor;
    if (propertyName == "minimapColor") return minimapColor;
    if (propertyName == "elevation") return elevation;
    if (propertyName == "tradeAs") return tradeAs;
    if (propertyName == "showAs") return showAs;
    if (propertyName == "weaponType") return weaponType;
    if (propertyName == "ammoType") return ammoType;
    if (propertyName == "shootType") return shootType;
    if (propertyName == "effect") return effect;
    if (propertyName == "distanceEffect") return distanceEffect;
    if (propertyName == "armor") return armor;
    if (propertyName == "defense") return defense;
    if (propertyName == "extraDefense") return extraDefense;
    if (propertyName == "attack") return attack;
    if (propertyName == "rotateTo") return rotateTo;
    if (propertyName == "containerSize") return containerSize;
    if (propertyName == "fluidSource") return fluidSource;
    if (propertyName == "maxReadWriteChars") return maxReadWriteChars;
    if (propertyName == "maxReadChars") return maxReadChars;
    if (propertyName == "maxWriteChars") return maxWriteChars;
    if (propertyName == "isCustomCreated") return isCustomCreated;
    if (propertyName == "hasClientData") return hasClientData;
    if (propertyName == "lastModified") return lastModified;
    if (propertyName == "modifiedBy") return modifiedBy;
    
    return QVariant();
}

bool ServerItem::setProperty(const QString& propertyName, const QVariant& value)
{
    if (!validateProperty(propertyName, value)) {
        return false;
    }
    
    bool changed = false;
    
    if (propertyName == "id" && value.toUInt() != id) {
        id = value.toUInt();
        changed = true;
    } else if (propertyName == "clientId" && value.toUInt() != clientId) {
        clientId = value.toUInt();
        changed = true;
    } else if (propertyName == "previousClientId" && value.toUInt() != previousClientId) {
        previousClientId = value.toUInt();
        changed = true;
    } else if (propertyName == "type") {
        ServerItemType newType = static_cast<ServerItemType>(value.toUInt());
        if (newType != type) {
            type = newType;
            changed = true;
        }
    } else if (propertyName == "stackOrder") {
        TileStackOrder newOrder = static_cast<TileStackOrder>(value.toUInt());
        if (newOrder != stackOrder) {
            stackOrder = newOrder;
            changed = true;
        }
    } else if (propertyName == "name" && value.toString() != name) {
        name = value.toString();
        changed = true;
    } else if (propertyName == "description" && value.toString() != description) {
        description = value.toString();
        changed = true;
    } else if (propertyName == "article" && value.toString() != article) {
        article = value.toString();
        changed = true;
    } else if (propertyName == "plural" && value.toString() != plural) {
        plural = value.toString();
        changed = true;
    } else if (propertyName == "width" && value.toUInt() != width) {
        width = value.toUInt();
        changed = true;
    } else if (propertyName == "height" && value.toUInt() != height) {
        height = value.toUInt();
        changed = true;
    } else if (propertyName == "layers" && value.toUInt() != layers) {
        layers = value.toUInt();
        changed = true;
    } else if (propertyName == "patternX" && value.toUInt() != patternX) {
        patternX = value.toUInt();
        changed = true;
    } else if (propertyName == "patternY" && value.toUInt() != patternY) {
        patternY = value.toUInt();
        changed = true;
    } else if (propertyName == "patternZ" && value.toUInt() != patternZ) {
        patternZ = value.toUInt();
        changed = true;
    } else if (propertyName == "frames" && value.toUInt() != frames) {
        frames = value.toUInt();
        changed = true;
    } else if (propertyName == "flags" && value.toUInt() != flags) {
        flags = value.toUInt();
        changed = true;
    } else if (propertyName == "speed" && value.toUInt() != speed) {
        speed = value.toUInt();
        changed = true;
    } else if (propertyName == "lightLevel" && value.toUInt() != lightLevel) {
        lightLevel = value.toUInt();
        changed = true;
    } else if (propertyName == "lightColor" && value.toUInt() != lightColor) {
        lightColor = value.toUInt();
        changed = true;
    }
    
    if (changed) {
        markAsModified();
        lastModified = QDateTime::currentDateTime();
    }
    
    return true;
}

QStringList ServerItem::getPropertyNames() const
{
    return QStringList() 
        << "id" << "clientId" << "previousClientId" << "type" << "stackOrder"
        << "name" << "description" << "article" << "plural"
        << "width" << "height" << "layers" << "patternX" << "patternY" << "patternZ" << "frames"
        << "flags" << "speed" << "lightLevel" << "lightColor" << "minimapColor" << "elevation"
        << "tradeAs" << "showAs" << "weaponType" << "ammoType" << "shootType" << "effect" << "distanceEffect"
        << "armor" << "defense" << "extraDefense" << "attack" << "rotateTo"
        << "containerSize" << "fluidSource" << "maxReadWriteChars" << "maxReadChars" << "maxWriteChars"
        << "isCustomCreated" << "hasClientData" << "lastModified" << "modifiedBy";
}

QByteArray ServerItem::serialize() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    
    // Serialize all properties in order
    stream << id << clientId << previousClientId;
    stream << static_cast<quint8>(type);
    stream << static_cast<quint8>(stackOrder);
    stream << name << description << article << plural;
    stream << spriteHash;
    stream << width << height << layers << patternX << patternY << patternZ << frames;
    stream << flags << speed << lightLevel << lightColor << minimapColor << elevation;
    stream << tradeAs << showAs;
    stream << weaponType << ammoType << shootType << effect << distanceEffect;
    stream << armor << defense << extraDefense << attack << rotateTo;
    stream << containerSize << fluidSource;
    stream << maxReadWriteChars << maxReadChars << maxWriteChars;
    stream << isCustomCreated << hasClientData;
    stream << lastModified << modifiedBy;
    
    return data;
}

bool ServerItem::deserialize(const QByteArray& data)
{
    QDataStream stream(data);
    
    quint8 typeValue, stackOrderValue;
    
    stream >> id >> clientId >> previousClientId;
    stream >> typeValue >> stackOrderValue;
    stream >> name >> description >> article >> plural;
    stream >> spriteHash;
    stream >> width >> height >> layers >> patternX >> patternY >> patternZ >> frames;
    stream >> flags >> speed >> lightLevel >> lightColor >> minimapColor >> elevation;
    stream >> tradeAs >> showAs;
    stream >> weaponType >> ammoType >> shootType >> effect >> distanceEffect;
    stream >> armor >> defense >> extraDefense >> attack >> rotateTo;
    stream >> containerSize >> fluidSource;
    stream >> maxReadWriteChars >> maxReadChars >> maxWriteChars;
    stream >> isCustomCreated >> hasClientData;
    stream >> lastModified >> modifiedBy;
    
    type = static_cast<ServerItemType>(typeValue);
    stackOrder = static_cast<TileStackOrder>(stackOrderValue);
    
    return stream.status() == QDataStream::Ok;
}

void ServerItem::copyFrom(const ServerItem& other)
{
    id = other.id;
    clientId = other.clientId;
    previousClientId = other.previousClientId;
    type = other.type;
    stackOrder = other.stackOrder;
    name = other.name;
    description = other.description;
    article = other.article;
    plural = other.plural;
    spriteHash = other.spriteHash;
    width = other.width;
    height = other.height;
    layers = other.layers;
    patternX = other.patternX;
    patternY = other.patternY;
    patternZ = other.patternZ;
    frames = other.frames;
    flags = other.flags;
    speed = other.speed;
    lightLevel = other.lightLevel;
    lightColor = other.lightColor;
    minimapColor = other.minimapColor;
    elevation = other.elevation;
    tradeAs = other.tradeAs;
    showAs = other.showAs;
    weaponType = other.weaponType;
    ammoType = other.ammoType;
    shootType = other.shootType;
    effect = other.effect;
    distanceEffect = other.distanceEffect;
    armor = other.armor;
    defense = other.defense;
    extraDefense = other.extraDefense;
    attack = other.attack;
    rotateTo = other.rotateTo;
    containerSize = other.containerSize;
    fluidSource = other.fluidSource;
    maxReadWriteChars = other.maxReadWriteChars;
    maxReadChars = other.maxReadChars;
    maxWriteChars = other.maxWriteChars;
    isCustomCreated = other.isCustomCreated;
    hasClientData = other.hasClientData;
    lastModified = other.lastModified;
    modifiedBy = other.modifiedBy;
    m_isModified = other.m_isModified;
}

void ServerItem::initializeDefaults()
{
    // Set default values based on legacy system behavior
    name = "New Item";
    description = "";
    article = "a";
    plural = "";
    modifiedBy = "System";
}