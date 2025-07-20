#include "ItemComparator.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <cmath>

ItemComparator::ItemComparator(QObject* parent)
    : QObject(parent)
    , m_ignoreCase(false)
    , m_ignoreWhitespace(false)
    , m_numericTolerance(0.001)
    , m_totalComparisons(0)
    , m_mismatchCount(0)
    , m_matchCount(0)
{
    initializeColorScheme();
}

ItemComparator::~ItemComparator()
{
}

// Single item comparison
ItemComparison ItemComparator::compareItems(const ServerItem* serverItem, const ClientItem* clientItem)
{
    ItemId id = 0;
    if (serverItem) id = serverItem->id;
    else if (clientItem) id = clientItem->id;
    
    return compareItems(id, serverItem, clientItem);
}

ItemComparison ItemComparator::compareItems(ItemId id, const ServerItem* serverItem, const ClientItem* clientItem)
{
    ItemComparison result;
    result.itemId = id;
    result.hasServerItem = (serverItem != nullptr);
    result.hasClientItem = (clientItem != nullptr);
    
    if (!serverItem && !clientItem) {
        return result; // Both null, nothing to compare
    }
    
    if (!serverItem || !clientItem) {
        result.overallMatch = false;
        return result; // One is null, automatic mismatch
    }
    
    // Compare all relevant properties
    QStringList propertiesToCompare = {
        "id", "name", "type", "width", "height", "speed", "lightLevel", "lightColor",
        "flags", "minimapColor", "elevation", "tradeAs", "showAs", "weaponType",
        "ammoType", "shootType", "effect", "distanceEffect", "armor", "defense",
        "extraDefense", "attack", "rotateTo", "containerSize", "fluidSource",
        "maxReadWriteChars", "maxReadChars", "maxWriteChars"
    };
    
    for (const QString& propertyName : propertiesToCompare) {
        if (m_ignoredProperties.contains(propertyName)) {
            continue;
        }
        
        if (!isComparableProperty(propertyName)) {
            continue;
        }
        
        QVariant serverValue = getServerPropertyValue(serverItem, propertyName);
        QVariant clientValue = getClientPropertyValue(clientItem, propertyName);
        
        PropertyComparison propComparison = compareProperty(propertyName, serverValue, clientValue);
        result.addPropertyComparison(propComparison);
        
        if (!propComparison.matches) {
            emit mismatchDetected(id, propertyName, serverValue, clientValue);
        }
    }
    
    updateStatistics(result.overallMatch);
    return result;
}

// Property-specific comparison
PropertyComparison ItemComparator::compareProperty(const QString& propertyName, 
                                                 const QVariant& serverValue, 
                                                 const QVariant& clientValue)
{
    PropertyComparison result(propertyName, serverValue, clientValue);
    
    if (!serverValue.isValid() && !clientValue.isValid()) {
        result.matches = true;
        return result;
    }
    
    if (!serverValue.isValid() || !clientValue.isValid()) {
        result.matches = false;
        result.mismatchReason = "One value is null/invalid";
        return result;
    }
    
    result.matches = compareValues(serverValue, clientValue, propertyName);
    
    if (!result.matches) {
        result.mismatchReason = QString("Server: %1, Client: %2")
                                .arg(serverValue.toString())
                                .arg(clientValue.toString());
    }
    
    return result;
}

// Batch comparison operations
BatchComparison ItemComparator::compareItemLists(const QList<ServerItem>& serverItems, 
                                                const QList<ClientItem>& clientItems)
{
    BatchComparison result;
    
    // Create hash maps for efficient lookup
    QHash<ItemId, const ServerItem*> serverMap;
    QHash<ItemId, const ClientItem*> clientMap;
    QSet<ItemId> allIds;
    
    for (const ServerItem& item : serverItems) {
        serverMap[item.id] = &item;
        allIds.insert(item.id);
    }
    
    for (const ClientItem& item : clientItems) {
        clientMap[item.id] = &item;
        allIds.insert(item.id);
    }
    
    emit comparisonStarted(allIds.size());
    
    int currentItem = 0;
    for (ItemId id : allIds) {
        const ServerItem* serverItem = serverMap.value(id, nullptr);
        const ClientItem* clientItem = clientMap.value(id, nullptr);
        
        ItemComparison itemComparison = compareItems(id, serverItem, clientItem);
        result.addItemComparison(itemComparison);
        
        emit comparisonProgress(++currentItem, allIds.size());
    }
    
    emit comparisonCompleted(result);
    return result;
}

BatchComparison ItemComparator::compareItemsByIds(const QList<ItemId>& itemIds,
                                                 const QHash<ItemId, ServerItem*>& serverItems,
                                                 const QHash<ItemId, ClientItem*>& clientItems)
{
    BatchComparison result;
    
    emit comparisonStarted(itemIds.size());
    
    int currentItem = 0;
    for (ItemId id : itemIds) {
        const ServerItem* serverItem = serverItems.value(id, nullptr);
        const ClientItem* clientItem = clientItems.value(id, nullptr);
        
        ItemComparison itemComparison = compareItems(id, serverItem, clientItem);
        result.addItemComparison(itemComparison);
        
        emit comparisonProgress(++currentItem, itemIds.size());
    }
    
    emit comparisonCompleted(result);
    return result;
}

// Mismatch detection
QStringList ItemComparator::detectMismatches(const ServerItem* serverItem, const ClientItem* clientItem)
{
    ItemComparison comparison = compareItems(serverItem, clientItem);
    return comparison.mismatchedProperties;
}

QHash<QString, QVariant> ItemComparator::getMismatchedProperties(const ServerItem* serverItem, const ClientItem* clientItem)
{
    QHash<QString, QVariant> mismatches;
    ItemComparison comparison = compareItems(serverItem, clientItem);
    
    for (const PropertyComparison& propComp : comparison.propertyComparisons) {
        if (!propComp.matches) {
            mismatches[propComp.propertyName] = propComp.clientValue;
        }
    }
    
    return mismatches;
}

bool ItemComparator::hasAnyMismatches(const ServerItem* serverItem, const ClientItem* clientItem)
{
    ItemComparison comparison = compareItems(serverItem, clientItem);
    return !comparison.overallMatch;
}

// Color coding for mismatches (matching legacy system)
QColor ItemComparator::getMismatchColor(const QString& propertyName) const
{
    return m_propertyColors.value(propertyName, m_defaultMismatchColor);
}

QColor ItemComparator::getMatchColor() const
{
    return m_matchColor;
}

QColor ItemComparator::getServerOnlyColor() const
{
    return m_serverOnlyColor;
}

QColor ItemComparator::getClientOnlyColor() const
{
    return m_clientOnlyColor;
}

// Comparison settings
void ItemComparator::setIgnoreCase(bool ignore)
{
    m_ignoreCase = ignore;
}

bool ItemComparator::getIgnoreCase() const
{
    return m_ignoreCase;
}

void ItemComparator::setIgnoreWhitespace(bool ignore)
{
    m_ignoreWhitespace = ignore;
}

bool ItemComparator::getIgnoreWhitespace() const
{
    return m_ignoreWhitespace;
}

void ItemComparator::setToleranceForNumeric(double tolerance)
{
    m_numericTolerance = tolerance;
}

double ItemComparator::getToleranceForNumeric() const
{
    return m_numericTolerance;
}

// Property filtering
void ItemComparator::setIgnoredProperties(const QStringList& properties)
{
    m_ignoredProperties = properties;
}

QStringList ItemComparator::getIgnoredProperties() const
{
    return m_ignoredProperties;
}

void ItemComparator::addIgnoredProperty(const QString& property)
{
    if (!m_ignoredProperties.contains(property)) {
        m_ignoredProperties.append(property);
    }
}

void ItemComparator::removeIgnoredProperty(const QString& property)
{
    m_ignoredProperties.removeAll(property);
}

void ItemComparator::clearIgnoredProperties()
{
    m_ignoredProperties.clear();
}

// Comparison statistics
int ItemComparator::getTotalComparisons() const
{
    return m_totalComparisons;
}

int ItemComparator::getMismatchCount() const
{
    return m_mismatchCount;
}

int ItemComparator::getMatchCount() const
{
    return m_matchCount;
}

double ItemComparator::getMismatchPercentage() const
{
    if (m_totalComparisons == 0) return 0.0;
    return (double)m_mismatchCount / m_totalComparisons * 100.0;
}

// Export comparison results
QString ItemComparator::exportComparisonToText(const ItemComparison& comparison) const
{
    QString result;
    result += QString("Item ID: %1\n").arg(comparison.itemId);
    result += QString("Overall Match: %1\n").arg(comparison.overallMatch ? "Yes" : "No");
    result += QString("Mismatch Count: %1\n").arg(comparison.mismatchCount);
    result += "\nProperty Comparisons:\n";
    
    for (const PropertyComparison& propComp : comparison.propertyComparisons) {
        result += QString("  %1: %2 (Server: %3, Client: %4)\n")
                  .arg(propComp.propertyName)
                  .arg(propComp.matches ? "MATCH" : "MISMATCH")
                  .arg(propComp.serverValue.toString())
                  .arg(propComp.clientValue.toString());
        
        if (!propComp.matches && !propComp.mismatchReason.isEmpty()) {
            result += QString("    Reason: %1\n").arg(propComp.mismatchReason);
        }
    }
    
    return result;
}

QString ItemComparator::exportBatchComparisonToText(const BatchComparison& batchComparison) const
{
    QString result;
    result += "Batch Comparison Results\n";
    result += "========================\n";
    result += QString("Total Items: %1\n").arg(batchComparison.totalItems);
    result += QString("Matching Items: %1\n").arg(batchComparison.matchingItems);
    result += QString("Mismatched Items: %1\n").arg(batchComparison.mismatchedItems);
    result += QString("Server Only Items: %1\n").arg(batchComparison.serverOnlyItems);
    result += QString("Client Only Items: %1\n").arg(batchComparison.clientOnlyItems);
    result += "\nProperty Mismatch Counts:\n";
    
    for (auto it = batchComparison.propertyMismatchCounts.begin(); 
         it != batchComparison.propertyMismatchCounts.end(); ++it) {
        result += QString("  %1: %2\n").arg(it.key()).arg(it.value());
    }
    
    return result;
}

QByteArray ItemComparator::exportComparisonToJson(const ItemComparison& comparison) const
{
    QJsonObject json;
    json["itemId"] = static_cast<int>(comparison.itemId);
    json["hasServerItem"] = comparison.hasServerItem;
    json["hasClientItem"] = comparison.hasClientItem;
    json["overallMatch"] = comparison.overallMatch;
    json["mismatchCount"] = comparison.mismatchCount;
    
    QJsonArray propertyArray;
    for (const PropertyComparison& propComp : comparison.propertyComparisons) {
        QJsonObject propJson;
        propJson["propertyName"] = propComp.propertyName;
        propJson["matches"] = propComp.matches;
        propJson["serverValue"] = propComp.serverValue.toString();
        propJson["clientValue"] = propComp.clientValue.toString();
        propJson["mismatchReason"] = propComp.mismatchReason;
        propertyArray.append(propJson);
    }
    json["propertyComparisons"] = propertyArray;
    
    return QJsonDocument(json).toJson();
}

QByteArray ItemComparator::exportBatchComparisonToJson(const BatchComparison& batchComparison) const
{
    QJsonObject json;
    json["totalItems"] = batchComparison.totalItems;
    json["matchingItems"] = batchComparison.matchingItems;
    json["mismatchedItems"] = batchComparison.mismatchedItems;
    json["serverOnlyItems"] = batchComparison.serverOnlyItems;
    json["clientOnlyItems"] = batchComparison.clientOnlyItems;
    
    QJsonObject mismatchCounts;
    for (auto it = batchComparison.propertyMismatchCounts.begin(); 
         it != batchComparison.propertyMismatchCounts.end(); ++it) {
        mismatchCounts[it.key()] = it.value();
    }
    json["propertyMismatchCounts"] = mismatchCounts;
    
    return QJsonDocument(json).toJson();
}

// Public slots
void ItemComparator::resetStatistics()
{
    m_totalComparisons = 0;
    m_mismatchCount = 0;
    m_matchCount = 0;
}

// Private comparison methods
bool ItemComparator::compareValues(const QVariant& serverValue, const QVariant& clientValue, const QString& propertyName) const
{
    Q_UNUSED(propertyName)
    
    if (serverValue.type() != clientValue.type()) {
        // Try to convert to same type for comparison
        if (serverValue.canConvert(clientValue.type())) {
            QVariant convertedServer = serverValue;
            convertedServer.convert(clientValue.type());
            return compareValues(convertedServer, clientValue, propertyName);
        } else if (clientValue.canConvert(serverValue.type())) {
            QVariant convertedClient = clientValue;
            convertedClient.convert(serverValue.type());
            return compareValues(serverValue, convertedClient, propertyName);
        }
        return false;
    }
    
    switch (serverValue.type()) {
        case QVariant::String:
            return compareStrings(serverValue.toString(), clientValue.toString());
        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::Double:
        case QVariant::LongLong:
        case QVariant::ULongLong:
            return compareNumbers(serverValue, clientValue);
        default:
            return serverValue == clientValue;
    }
}

bool ItemComparator::compareStrings(const QString& str1, const QString& str2) const
{
    QString s1 = normalizeString(str1);
    QString s2 = normalizeString(str2);
    
    if (m_ignoreCase) {
        return s1.compare(s2, Qt::CaseInsensitive) == 0;
    } else {
        return s1 == s2;
    }
}

bool ItemComparator::compareNumbers(const QVariant& num1, const QVariant& num2) const
{
    bool ok1, ok2;
    double d1 = num1.toDouble(&ok1);
    double d2 = num2.toDouble(&ok2);
    
    if (!ok1 || !ok2) {
        return num1 == num2; // Fallback to direct comparison
    }
    
    return std::abs(d1 - d2) <= m_numericTolerance;
}

QString ItemComparator::normalizeString(const QString& str) const
{
    if (m_ignoreWhitespace) {
        return str.simplified();
    }
    return str;
}

// Property mapping and validation
bool ItemComparator::isComparableProperty(const QString& propertyName) const
{
    // Define which properties can be compared between server and client
    static QStringList comparableProperties = {
        "id", "name", "type", "width", "height", "speed", "lightLevel", "lightColor",
        "flags", "minimapColor", "elevation", "tradeAs", "showAs", "weaponType",
        "ammoType", "shootType", "effect", "distanceEffect", "armor", "defense",
        "extraDefense", "attack", "rotateTo", "containerSize", "fluidSource",
        "maxReadWriteChars", "maxReadChars", "maxWriteChars"
    };
    
    return comparableProperties.contains(propertyName);
}

QVariant ItemComparator::getServerPropertyValue(const ServerItem* item, const QString& propertyName) const
{
    if (!item) return QVariant();
    
    // Map property names to ServerItem fields
    if (propertyName == "id") return item->id;
    if (propertyName == "name") return item->name;
    if (propertyName == "type") return static_cast<int>(item->type);
    if (propertyName == "width") return item->width;
    if (propertyName == "height") return item->height;
    if (propertyName == "speed") return item->speed;
    if (propertyName == "lightLevel") return item->lightLevel;
    if (propertyName == "lightColor") return item->lightColor;
    if (propertyName == "flags") return item->flags;
    if (propertyName == "minimapColor") return item->minimapColor;
    if (propertyName == "elevation") return item->elevation;
    if (propertyName == "tradeAs") return item->tradeAs;
    if (propertyName == "showAs") return item->showAs;
    if (propertyName == "weaponType") return item->weaponType;
    if (propertyName == "ammoType") return item->ammoType;
    if (propertyName == "shootType") return item->shootType;
    if (propertyName == "effect") return item->effect;
    if (propertyName == "distanceEffect") return item->distanceEffect;
    if (propertyName == "armor") return item->armor;
    if (propertyName == "defense") return item->defense;
    if (propertyName == "extraDefense") return item->extraDefense;
    if (propertyName == "attack") return item->attack;
    if (propertyName == "rotateTo") return item->rotateTo;
    if (propertyName == "containerSize") return item->containerSize;
    if (propertyName == "fluidSource") return item->fluidSource;
    if (propertyName == "maxReadWriteChars") return item->maxReadWriteChars;
    if (propertyName == "maxReadChars") return item->maxReadChars;
    if (propertyName == "maxWriteChars") return item->maxWriteChars;
    
    return QVariant();
}

QVariant ItemComparator::getClientPropertyValue(const ClientItem* item, const QString& propertyName) const
{
    if (!item) return QVariant();
    
    // Since ClientItem inherits from ServerItem, we can use the same property mapping
    return getServerPropertyValue(item, propertyName);
}

// Color initialization
void ItemComparator::initializeColorScheme()
{
    // Initialize colors matching the legacy system
    m_defaultMismatchColor = QColor(255, 100, 100); // Light red
    m_matchColor = QColor(100, 255, 100); // Light green
    m_serverOnlyColor = QColor(100, 100, 255); // Light blue
    m_clientOnlyColor = QColor(255, 255, 100); // Light yellow
    
    // Property-specific colors
    m_propertyColors["id"] = QColor(255, 150, 150);
    m_propertyColors["name"] = QColor(255, 200, 150);
    m_propertyColors["type"] = QColor(200, 150, 255);
    m_propertyColors["width"] = QColor(150, 255, 200);
    m_propertyColors["height"] = QColor(150, 255, 200);
    m_propertyColors["speed"] = QColor(255, 255, 150);
    m_propertyColors["flags"] = QColor(200, 200, 255);
}

// Statistics tracking
void ItemComparator::updateStatistics(bool isMatch)
{
    m_totalComparisons++;
    if (isMatch) {
        m_matchCount++;
    } else {
        m_mismatchCount++;
    }
}