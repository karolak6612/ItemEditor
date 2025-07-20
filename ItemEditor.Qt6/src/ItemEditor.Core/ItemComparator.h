#pragma once

#include <QObject>
#include <QHash>
#include <QVariant>
#include <QStringList>
#include <QColor>
#include "ServerItem.h"
#include "ClientItem.h"

/**
 * @brief Comparison result for a single property
 */
struct PropertyComparison {
    QString propertyName;
    QVariant serverValue;
    QVariant clientValue;
    bool matches;
    QString mismatchReason;
    
    PropertyComparison() : matches(true) {}
    PropertyComparison(const QString& name, const QVariant& server, const QVariant& client, bool match = true, const QString& reason = QString())
        : propertyName(name), serverValue(server), clientValue(client), matches(match), mismatchReason(reason) {}
};

/**
 * @brief Complete comparison result between server and client items
 */
struct ItemComparison {
    ItemId itemId;
    bool hasServerItem;
    bool hasClientItem;
    bool overallMatch;
    QList<PropertyComparison> propertyComparisons;
    QStringList mismatchedProperties;
    int mismatchCount;
    
    ItemComparison() : itemId(0), hasServerItem(false), hasClientItem(false), overallMatch(true), mismatchCount(0) {}
    
    void addPropertyComparison(const PropertyComparison& comparison) {
        propertyComparisons.append(comparison);
        if (!comparison.matches) {
            mismatchedProperties.append(comparison.propertyName);
            mismatchCount++;
            overallMatch = false;
        }
    }
};

/**
 * @brief Batch comparison result for multiple items
 */
struct BatchComparison {
    int totalItems;
    int matchingItems;
    int mismatchedItems;
    int serverOnlyItems;
    int clientOnlyItems;
    QList<ItemComparison> itemComparisons;
    QHash<QString, int> propertyMismatchCounts;
    
    BatchComparison() : totalItems(0), matchingItems(0), mismatchedItems(0), serverOnlyItems(0), clientOnlyItems(0) {}
    
    void addItemComparison(const ItemComparison& comparison) {
        itemComparisons.append(comparison);
        totalItems++;
        
        if (!comparison.hasServerItem) {
            clientOnlyItems++;
        } else if (!comparison.hasClientItem) {
            serverOnlyItems++;
        } else if (comparison.overallMatch) {
            matchingItems++;
        } else {
            mismatchedItems++;
            
            // Count property mismatches
            for (const QString& prop : comparison.mismatchedProperties) {
                propertyMismatchCounts[prop]++;
            }
        }
    }
};

/**
 * @brief Item comparison and mismatch detection engine
 * 
 * Provides server/client item comparison with identical logic to legacy system:
 * - Property-by-property comparison
 * - Mismatch highlighting with same color coding
 * - Batch comparison operations
 * - Detailed mismatch reporting
 */
class ItemComparator : public QObject
{
    Q_OBJECT

public:
    explicit ItemComparator(QObject* parent = nullptr);
    ~ItemComparator();

    // Single item comparison
    ItemComparison compareItems(const ServerItem* serverItem, const ClientItem* clientItem);
    ItemComparison compareItems(ItemId id, const ServerItem* serverItem, const ClientItem* clientItem);
    
    // Property-specific comparison
    PropertyComparison compareProperty(const QString& propertyName, 
                                     const QVariant& serverValue, 
                                     const QVariant& clientValue);
    
    // Batch comparison operations
    BatchComparison compareItemLists(const QList<ServerItem>& serverItems, 
                                   const QList<ClientItem>& clientItems);
    BatchComparison compareItemsByIds(const QList<ItemId>& itemIds,
                                    const QHash<ItemId, ServerItem*>& serverItems,
                                    const QHash<ItemId, ClientItem*>& clientItems);
    
    // Mismatch detection
    QStringList detectMismatches(const ServerItem* serverItem, const ClientItem* clientItem);
    QHash<QString, QVariant> getMismatchedProperties(const ServerItem* serverItem, const ClientItem* clientItem);
    bool hasAnyMismatches(const ServerItem* serverItem, const ClientItem* clientItem);
    
    // Color coding for mismatches (matching legacy system)
    QColor getMismatchColor(const QString& propertyName) const;
    QColor getMatchColor() const;
    QColor getServerOnlyColor() const;
    QColor getClientOnlyColor() const;
    
    // Comparison settings
    void setIgnoreCase(bool ignore);
    bool getIgnoreCase() const;
    void setIgnoreWhitespace(bool ignore);
    bool getIgnoreWhitespace() const;
    void setToleranceForNumeric(double tolerance);
    double getToleranceForNumeric() const;
    
    // Property filtering
    void setIgnoredProperties(const QStringList& properties);
    QStringList getIgnoredProperties() const;
    void addIgnoredProperty(const QString& property);
    void removeIgnoredProperty(const QString& property);
    void clearIgnoredProperties();
    
    // Comparison statistics
    int getTotalComparisons() const;
    int getMismatchCount() const;
    int getMatchCount() const;
    double getMismatchPercentage() const;
    
    // Export comparison results
    QString exportComparisonToText(const ItemComparison& comparison) const;
    QString exportBatchComparisonToText(const BatchComparison& batchComparison) const;
    QByteArray exportComparisonToJson(const ItemComparison& comparison) const;
    QByteArray exportBatchComparisonToJson(const BatchComparison& batchComparison) const;

signals:
    void comparisonStarted(int totalItems);
    void comparisonProgress(int currentItem, int totalItems);
    void comparisonCompleted(const BatchComparison& result);
    void mismatchDetected(ItemId itemId, const QString& propertyName, const QVariant& serverValue, const QVariant& clientValue);

public slots:
    void resetStatistics();

private:
    // Comparison settings
    bool m_ignoreCase;
    bool m_ignoreWhitespace;
    double m_numericTolerance;
    QStringList m_ignoredProperties;
    
    // Statistics
    int m_totalComparisons;
    int m_mismatchCount;
    int m_matchCount;
    
    // Color scheme (matching legacy system)
    QHash<QString, QColor> m_propertyColors;
    QColor m_defaultMismatchColor;
    QColor m_matchColor;
    QColor m_serverOnlyColor;
    QColor m_clientOnlyColor;
    
    // Private comparison methods
    bool compareValues(const QVariant& serverValue, const QVariant& clientValue, const QString& propertyName) const;
    bool compareStrings(const QString& str1, const QString& str2) const;
    bool compareNumbers(const QVariant& num1, const QVariant& num2) const;
    QString normalizeString(const QString& str) const;
    
    // Property mapping and validation
    bool isComparableProperty(const QString& propertyName) const;
    QVariant getServerPropertyValue(const ServerItem* item, const QString& propertyName) const;
    QVariant getClientPropertyValue(const ClientItem* item, const QString& propertyName) const;
    
    // Color initialization
    void initializeColorScheme();
    
    // Statistics tracking
    void updateStatistics(bool isMatch);
};