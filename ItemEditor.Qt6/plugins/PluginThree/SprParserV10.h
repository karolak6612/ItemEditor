#pragma once

#include "SprParser.h"
#include <QHash>
#include <QMutex>
#include <QList>

/**
 * @brief SPR parser for client versions 10.00-10.77
 * 
 * Implements native C++ SPR file parsing with exact compatibility to legacy system.
 * Handles binary SPR file format with compressed sprite data and transparency support
 * for client versions 10.00-10.77.
 */
class SprParserV10 : public SprParser
{
public:
    SprParserV10();
    ~SprParserV10() override;

    bool parseFile(const QString& filePath) override;
    ::SpriteData getSpriteData(quint16 id) const override;
    bool isLoaded() const override;
    void cleanup() override;
    
    // Additional methods for version detection and validation
    quint32 getSprSignature() const { return m_sprSignature; }
    quint32 getTotalSprites() const { return m_totalSprites; }
    QString getClientVersion() const;

private:
    bool m_isLoaded;
    quint32 m_sprSignature;
    quint32 m_totalSprites;
    bool m_transparency;
    
    // Cache for parsed sprite data
    QHash<quint32, ::SpriteData> m_spriteCache;
    QList<quint32> m_spriteIndexes;
    mutable QMutex m_mutex;
    
    // Helper methods
    bool validateSignature(quint32 signature) const;
    bool loadSpriteData(const QString& filePath);
    QString determineClientVersion(quint32 signature) const;
};