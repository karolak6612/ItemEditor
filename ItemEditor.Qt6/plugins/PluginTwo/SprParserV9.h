#pragma once

#include "SprParser.h"
#include <QMutex>
#include <QHash>
#include <QList>

/**
 * @brief SPR parser for client versions 8.60-9.86
 * 
 * Handles parsing of SPR files for Tibia client versions 8.60-9.86.
 * Implements native C++ parsing with identical functionality to legacy system.
 */
class SprParserV9 : public SprParser
{
public:
    SprParserV9();
    ~SprParserV9() override;
    bool parseFile(const QString& filePath) override;
    ::SpriteData getSpriteData(quint16 id) const override;
    bool isLoaded() const override;
    void cleanup() override;
    
    // Additional methods for Plugin Two
    quint32 getSprSignature() const;
    QString getClientVersion() const;

private:
    // Helper methods
    bool loadSpriteData(const QString& filePath);
    bool validateSignature(quint32 signature) const;
    QString determineClientVersion(quint32 signature) const;

    // Member variables
    mutable QMutex m_mutex;
    bool m_isLoaded;
    quint32 m_sprSignature;
    quint32 m_totalSprites;
    bool m_transparency;
    QList<quint32> m_spriteIndexes;
    QHash<quint16, ::SpriteData> m_spriteCache;
};