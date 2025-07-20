#pragma once

#include "DatParser.h"
#include <QHash>
#include <QMutex>

/**
 * @brief DAT parser for client versions 10.00-10.77
 * 
 * Implements native C++ DAT file parsing with exact compatibility to legacy system.
 * Handles binary DAT file format with item definitions, flags, and sprite references
 * for client versions 10.00-10.77.
 */
class DatParserV10 : public DatParser
{
public:
    DatParserV10();
    ~DatParserV10() override;

    bool parseFile(const QString& filePath) override;
    DatData getDatData(quint16 id) const override;
    bool isLoaded() const override;
    void cleanup() override;
    
    // Additional methods for version detection and validation
    quint32 getDatSignature() const { return m_datSignature; }
    quint16 getItemCount() const { return m_itemCount; }
    QString getClientVersion() const;

private:
    bool m_isLoaded;
    quint32 m_datSignature;
    quint16 m_itemCount;
    
    // Cache for parsed DAT data
    QHash<quint16, DatData> m_datCache;
    mutable QMutex m_mutex;
    
    // Helper methods
    bool validateSignature(quint32 signature) const;
    bool parseItemData(QDataStream& stream, quint16 itemId);
    QString determineClientVersion(quint32 signature) const;
};