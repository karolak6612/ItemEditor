#pragma once

#include "DatParser.h"
#include <QMutex>
#include <QHash>
#include <QDataStream>

/**
 * @brief DAT parser for client versions 8.60-9.86
 * 
 * Handles parsing of DAT files for Tibia client versions 8.60-9.86.
 * Implements native C++ parsing with identical functionality to legacy system.
 */
class DatParserV9 : public DatParser
{
public:
    DatParserV9();
    ~DatParserV9() override;

    bool parseFile(const QString& filePath) override;
    DatData getDatData(quint16 id) const override;
    bool isLoaded() const override;
    void cleanup() override;
    
    // Additional methods for Plugin Two
    quint32 getDatSignature() const;
    QString getClientVersion() const;

private:
    // Helper methods
    bool parseItemData(QDataStream& stream, quint16 itemId);
    bool validateSignature(quint32 signature) const;
    QString determineClientVersion(quint32 signature) const;

    // Member variables
    mutable QMutex m_mutex;
    bool m_isLoaded;
    quint32 m_datSignature;
    quint16 m_itemCount;
    QHash<quint16, DatData> m_datCache;
};