#ifndef DATPARSER_H
#define DATPARSER_H

#include "otb/item.h" // For ItemEditor::ClientItem (to populate)
#include <QString>
#include <QFile>
#include <QDataStream>
#include <QMap>
#include <QVector> // For attributes if needed

// Forward declare if Tibia specific enums are in a different header
// enum class DatThingType : quint8 { ... };
// enum class DatItemFlags : quint32 { ... };

namespace TibiaData {

// Based on OTLib.Client.Things.ThingFlag / OTLib.Dat.Enums.DatItemFlags
// This might need to be more extensive based on actual DAT format for various versions
namespace DatItemFlag {
    const quint32 BlockSolid        = 1 << 0;  // from ThingFlag.BlockSolid
    const quint32 BlockProjectile   = 1 << 1;  // from ThingFlag.BlockProjectile
    const quint32 BlockPath         = 1 << 2;  // from ThingFlag.BlockPath
    const quint32 HasElevation      = 1 << 3;  // from ThingFlag.HasElevation
    const quint32 IsUsable          = 1 << 4;  // from ThingFlag.IsUsable
    const quint32 IsPickupable      = 1 << 5;  // from ThingFlag.IsPickupable
    const quint32 IsMovable         = 1 << 6;  // from ThingFlag.IsMovable
    const quint32 IsStackable       = 1 << 7;  // from ThingFlag.IsStackable
    const quint32 FloorChange       = 1 << 8;  // from ThingFlag.FloorChange (down, north, south etc. need more bits)
    const quint32 FullGround        = 1 << 12; // from ThingFlag.FullGround (OTLib uses this)
    const quint32 IsReadable        = 1 << 13; // from ThingFlag.IsReadable
    const quint32 IsRotatable       = 1 << 14; // from ThingFlag.IsRotatable
    const quint32 IsHangable        = 1 << 15; // from ThingFlag.IsHangable
    const quint32 IsHookSouth       = 1 << 16; // from ThingFlag.HookSouth
    const quint32 IsHookEast        = 1 << 17; // from ThingFlag.HookEast
    const quint32 IgnoreLook        = 1 << 18; // from ThingFlag.IgnoreLook
    const quint32 IsAnimation       = 1 << 20; // from ThingFlag.IsAnimation (OTLib uses this)
    // ... Add all flags from C# analysis, values might differ based on client version
    // Example: ClientCharges, MultiUse, ForceUse are often part of OTB flags, derived from DAT or script
}

// Attributes read from DAT file for each item
// Based on OTLib.Client.Things.ThingAttribute / OTLib.Dat.Enums.DatAttribute
enum class DatAttribute : quint8 {
    Ground              = 0x00, // Ground item
    GroundBorder        = 0x01, // Ground border
    OnBottom            = 0x02, // Placed on bottom (e.g. carpets)
    OnTop               = 0x03, // Placed on top (e.g. tables)
    Container           = 0x04, // Is a container
    Stackable           = 0x05, // Can be stacked
    ForceUse            = 0x06, // Usable with 'use' action (multiuse)
    MultiUse            = 0x07, // Usable with 'use with...' action
    Writable            = 0x08, // Is writable (deprecated?)
    WritableOnce        = 0x09, // Is writable once (deprecated?)
    FluidContainer      = 0x0A, // Is a fluid container
    Splash              = 0x0B, // Is a splash
    Unpassable          = 0x0C, // Not walkable
    Unmovable           = 0x0D, // Not movable
    Unsight             = 0x0E, // Blocks line of sight
    Avoid               = 0x0F, // Avoid this item (monsters)
    NoMovementAnimation = 0x10, // No movement animation
    Take                = 0x11, // Can be taken
    LiquidPool          = 0x12, // Is a liquid pool
    Hangable            = 0x13, // Can be hanged
    HookSouth           = 0x14, // Hookable from south
    HookEast            = 0x15, // Hookable from east
    Rotatable           = 0x16, // Can be rotated
    Light               = 0x17, // Emits light
    DontHide            = 0x18, // Don't hide when player is behind
    Translucent         = 0x19, // Translucent
    Shift               = 0x1A, // Shift when player is behind
    Height              = 0x1B, // Has elevation
    LyingObject         = 0x1C, // Is a lying object (corpse)
    AnimateAlways       = 0x1D, // Always animate
    MinimapColor        = 0x1E, // Minimap color
    LensHelp            = 0x1F, // Lens help (wot?)
    FullGround          = 0x20, // Full ground tile
    Look                = 0x21, // Look through
    Cloth               = 0x22, // Is a cloth
    Market              = 0x23, // Market category
    Usable              = 0x24, // Usable (deprecated by ForceUse/MultiUse?)
    Wrapable            = 0x25, // Can be wrapped
    Unwrapable          = 0x26, // Can be unwrapped
    TopEffect           = 0x27, // Has top effect

    // These are often not direct DAT attributes but derived or set by scripts in OT servers
    // Name, Description, MaxTextLen, MaxTextLenOnce, Attack, Defense, Armor, etc.
    // For ItemEditor, these are mostly from OTB. ClientItem from DAT usually focuses on appearance and basic flags.
    Unknown             = 0xFF
};


class DatParser
{
public:
    DatParser();

    // clientVersion is important as DAT format changes significantly
    bool loadDat(const QString& filePath, quint32 clientVersion, QString& errorString);

    // Populates a map of ClientID -> ClientItem
    // Sprites for these client items would be loaded separately by SprParser
    // and then associated (e.g., by a plugin).
    bool getAllClientItems(QMap<quint16, ItemEditor::ClientItem>& outItemsMap, bool extendedFormat) const;

    quint32 getSignature() const;
    quint16 getItemCount() const; // Number of "things" (items, outfits, effects, missiles)
    quint16 getOutfitCount() const;
    quint16 getEffectCount() const;
    quint16 getMissileCount() const;

private:
    bool parseThing(QDataStream& stream, ItemEditor::ClientItem& outItem, bool extendedFormat) const;

    QFile m_file; // Keep open if reading on demand, or read all into memory
    quint32 m_signature;
    quint16 m_itemCount;
    quint16 m_outfitCount;
    quint16 m_effectCount;
    quint16 m_missileCount;
    quint32 m_clientVersion; // Store the client version this DAT was parsed for

    // Store raw item data blocks if not parsing all immediately
    // QMap<quint16, QByteArray> m_rawItemData;
    // Or parse all into ClientItems if memory allows
    QMap<quint16, ItemEditor::ClientItem> m_parsedClientItems;
};

} // namespace TibiaData

#endif // DATPARSER_H
