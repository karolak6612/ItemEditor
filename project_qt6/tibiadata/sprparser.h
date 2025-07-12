#ifndef SPPPARSER_H
#define SPPPARSER_H

#include "otb/item.h" // For OTB::Sprite (to populate)
#include <QString>
#include <QFile>
#include <QDataStream>
#include <QVector>
#include <QMap>

namespace TibiaData {

class SprParser
{
public:
    SprParser();

    bool loadSpr(const QString& filePath, QString& errorString);

    // Gets a single sprite by its ID (1-based index from SPR file)
    // Populates the passed OTB::Sprite object.
    // Returns true if sprite found and successfully parsed.
    bool getSprite(quint32 spriteId, OTB::Sprite& outSprite, bool transparent = true) const;

    quint32 getSpriteCount() const;
    quint32 getSignature() const;

private:
    bool parseSpriteData(quint32 spriteId, QDataStream& stream, OTB::Sprite& outSprite, bool transparent) const;

    QFile m_file; // Keep the file open if we need to read sprites on demand
    // QDataStream m_stream; // Or manage stream per operation

    quint32 m_signature;
    quint32 m_spriteCount;
    QVector<quint32> m_spriteAddresses; // Stores file offset for each sprite

    // Potentially cache parsed sprites if memory allows and performance needs it
    // mutable QMap<quint32, OTB::Sprite> m_spriteCache;
};

} // namespace TibiaData

#endif // SPPPARSER_H
