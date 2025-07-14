/**
 * Item Editor Qt6 - Sprite Class Header
 * Exact mirror of Legacy_App/csharp/Source/PluginInterface/Sprite.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_SPRITE_H
#define ITEMEDITOR_SPRITE_H

#include <QByteArray>
#include <QPixmap>
#include <QMap>
#include "IPlugin.h"

namespace ItemEditor {

/**
 * Sprite Class
 * Exact mirror of C# Sprite class
 * Handles sprite data compression and rendering
 */
class Sprite
{
public:
    /**
     * Constructor
     */
    Sprite();
    
    /**
     * Destructor
     */
    virtual ~Sprite() = default;

    // Constants - exact mirror of C# constants
    static const quint8 DEFAULT_SIZE = 32;
    static const quint16 RGB_PIXELS_DATA_SIZE = 3072;  // 32*32*3
    static const quint16 ARGB_PIXELS_DATA_SIZE = 4096; // 32*32*4

    // Properties - exact mirror of C# properties
    quint32 id() const { return m_id; }
    void setId(quint32 id) { m_id = id; }
    
    quint32 size() const { return m_size; }
    void setSize(quint32 size) { m_size = size; }
    
    QByteArray compressedPixels() const { return m_compressedPixels; }
    void setCompressedPixels(const QByteArray& pixels) { m_compressedPixels = pixels; }
    
    bool transparent() const { return m_transparent; }
    void setTransparent(bool transparent) { m_transparent = transparent; }

    // Methods - exact mirror of C# methods
    QByteArray getRGBData();
    QByteArray getPixels();
    QPixmap getPixmap();

    // Static methods - exact mirror of C# static methods
    static void createBlankSprite();
    static bool loadSprites(const QString& filename, 
                           QMap<quint32, Sprite*>& sprites, 
                           const SupportedClient& client, 
                           bool extended, 
                           bool transparency);

private:
    QByteArray getRGBData(quint8 transparentColor);

private:
    quint32 m_id;
    quint32 m_size;
    QByteArray m_compressedPixels;
    bool m_transparent;
    
    // Static blank sprites - exact mirror of C# static fields
    static QByteArray s_blankRGBSprite;
    static QByteArray s_blankARGBSprite;
};

} // namespace ItemEditor

#endif // ITEMEDITOR_SPRITE_H