#pragma once

#include <QtGlobal>
#include <QByteArray>
#include <QList>

/**
 * @brief Client data type definitions for DAT/SPR file parsing
 * 
 * These structures match the legacy system's data format for client versions 8.00-8.57
 */

// Sprite data structure for SPR files
struct SpriteData
{
    quint32 id = 0;
    quint32 size = 0;
    QByteArray compressedPixels;
    bool transparent = false;
    
    // Constants from legacy system
    static const quint8 DefaultSize = 32;
    static const quint16 RGBPixelsDataSize = 3072; // 32*32*3
    static const quint16 ARGBPixelsDataSize = 4096; // 32*32*4
    
    // Methods to extract pixel data
    QByteArray getRGBData() const;
    QByteArray getPixels() const;
    bool isValid() const { return id > 0 && size > 0 && !compressedPixels.isEmpty(); }
};

// DAT data structure for client items
struct DatData
{
    quint16 id = 0;
    
    // Item properties from DAT file
    quint8 width = 1;
    quint8 height = 1;
    quint8 layers = 1;
    quint8 patternX = 1;
    quint8 patternY = 1;
    quint8 patternZ = 1;
    quint8 frames = 1;
    quint32 numSprites = 0;
    
    // Item flags and attributes
    quint32 flags = 0;
    quint16 groundSpeed = 0;
    quint16 lightLevel = 0;
    quint16 lightColor = 0;
    quint16 maxReadChars = 0;
    quint16 maxReadWriteChars = 0;
    quint16 minimapColor = 0;
    
    // Sprite IDs for this item
    QList<quint32> spriteIds;
    
    bool isValid() const { return id > 0; }
};

// Client item flags enum matching legacy system
enum class ClientItemFlag : quint8
{
    Ground = 0x00,
    GroundBorder = 0x01,
    OnBottom = 0x02,
    OnTop = 0x03,
    Container = 0x04,
    Stackable = 0x05,
    ForceUse = 0x06,
    MultiUse = 0x07,
    HasCharges = 0x08,
    Writable = 0x09,
    WritableOnce = 0x0A,
    FluidContainer = 0x0B,
    Fluid = 0x0C,
    IsUnpassable = 0x0D,
    IsUnmoveable = 0x0E,
    BlockMissiles = 0x0F,
    BlockPathfinder = 0x0F,
    Pickupable = 0x10,
    Hangable = 0x11,
    IsHorizontal = 0x12,
    IsVertical = 0x13,
    Rotatable = 0x14,
    HasLight = 0x15,
    DontHide = 0x16,
    Translucent = 0x17,
    HasOffset = 0x18,
    HasElevation = 0x19,
    Lying = 0x1A,
    AnimateAlways = 0x1B,
    Minimap = 0x1C,
    LensHelp = 0x1D,
    FullGround = 0x1E,
    IgnoreLook = 0x1F,
    Cloth = 0x20,
    Market = 0x21,
    LastFlag = 0xFF
};