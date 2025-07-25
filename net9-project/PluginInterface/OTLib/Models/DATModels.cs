namespace PluginInterface.OTLib.FileFormats;

/// <summary>
/// DAT file header structure
/// </summary>
public class DATHeader
{
    /// <summary>
    /// Header size in bytes
    /// </summary>
    public const int HeaderSize = 12;

    /// <summary>
    /// DAT format version
    /// </summary>
    public uint Version { get; set; }

    /// <summary>
    /// Number of items in the file
    /// </summary>
    public ushort ItemCount { get; set; }

    /// <summary>
    /// Number of outfits in the file
    /// </summary>
    public ushort OutfitCount { get; set; }

    /// <summary>
    /// Number of effects in the file
    /// </summary>
    public ushort EffectCount { get; set; }

    /// <summary>
    /// Number of missiles in the file
    /// </summary>
    public ushort MissileCount { get; set; }
}

/// <summary>
/// DAT item representation
/// </summary>
public class DATItem
{
    /// <summary>
    /// Item ID
    /// </summary>
    public ushort Id { get; set; }

    /// <summary>
    /// Item flags
    /// </summary>
    public List<DATItemFlag> Flags { get; set; } = new();

    /// <summary>
    /// Sprite width
    /// </summary>
    public byte Width { get; set; }

    /// <summary>
    /// Sprite height
    /// </summary>
    public byte Height { get; set; }

    /// <summary>
    /// Number of layers
    /// </summary>
    public byte Layers { get; set; }

    /// <summary>
    /// Pattern X
    /// </summary>
    public byte PatternX { get; set; }

    /// <summary>
    /// Pattern Y
    /// </summary>
    public byte PatternY { get; set; }

    /// <summary>
    /// Pattern Z
    /// </summary>
    public byte PatternZ { get; set; }

    /// <summary>
    /// Number of animations
    /// </summary>
    public byte Animations { get; set; }

    /// <summary>
    /// Sprite IDs
    /// </summary>
    public List<ushort> SpriteIds { get; set; } = new();

    // Flag-specific properties
    /// <summary>
    /// Ground speed (for ground items)
    /// </summary>
    public ushort? GroundSpeed { get; set; }

    /// <summary>
    /// Maximum text length (for writeable items)
    /// </summary>
    public ushort? MaxTextLength { get; set; }

    /// <summary>
    /// Light level
    /// </summary>
    public ushort? LightLevel { get; set; }

    /// <summary>
    /// Light color
    /// </summary>
    public ushort? LightColor { get; set; }

    /// <summary>
    /// Displacement X
    /// </summary>
    public ushort? DisplacementX { get; set; }

    /// <summary>
    /// Displacement Y
    /// </summary>
    public ushort? DisplacementY { get; set; }

    /// <summary>
    /// Elevation
    /// </summary>
    public ushort? Elevation { get; set; }

    /// <summary>
    /// Minimap color
    /// </summary>
    public ushort? MinimapColor { get; set; }

    /// <summary>
    /// Lens help
    /// </summary>
    public ushort? LensHelp { get; set; }

    /// <summary>
    /// Cloth slot
    /// </summary>
    public ushort? ClothSlot { get; set; }

    /// <summary>
    /// Market category
    /// </summary>
    public ushort? MarketCategory { get; set; }

    /// <summary>
    /// Market trade as
    /// </summary>
    public ushort? MarketTradeAs { get; set; }

    /// <summary>
    /// Market show as
    /// </summary>
    public ushort? MarketShowAs { get; set; }

    /// <summary>
    /// Market name
    /// </summary>
    public string? MarketName { get; set; }

    /// <summary>
    /// Market restrict profession
    /// </summary>
    public ushort? MarketRestrictProfession { get; set; }

    /// <summary>
    /// Market restrict level
    /// </summary>
    public ushort? MarketRestrictLevel { get; set; }

    /// <summary>
    /// Check if item has a specific flag
    /// </summary>
    /// <param name="flag">Flag to check</param>
    /// <returns>True if item has the flag</returns>
    public bool HasFlag(DATItemFlag flag) => Flags.Contains(flag);

    /// <summary>
    /// Add a flag to the item
    /// </summary>
    /// <param name="flag">Flag to add</param>
    public void AddFlag(DATItemFlag flag)
    {
        if (!HasFlag(flag))
        {
            Flags.Add(flag);
        }
    }

    /// <summary>
    /// Remove a flag from the item
    /// </summary>
    /// <param name="flag">Flag to remove</param>
    /// <returns>True if flag was removed</returns>
    public bool RemoveFlag(DATItemFlag flag) => Flags.Remove(flag);

    /// <summary>
    /// Get total number of sprites for this item
    /// </summary>
    /// <returns>Total sprite count</returns>
    public int GetTotalSpriteCount()
    {
        return Width * Height * Layers * PatternX * PatternY * PatternZ * Math.Max(Animations, 1);
    }
}

/// <summary>
/// DAT item flags
/// </summary>
public enum DATItemFlag : byte
{
    Ground = 0x00,
    GroundBorder = 0x01,
    OnBottom = 0x02,
    OnTop = 0x03,
    Container = 0x04,
    Stackable = 0x05,
    ForceUse = 0x06,
    MultiUse = 0x07,
    Writeable = 0x08,
    WriteableOnce = 0x09,
    FluidContainer = 0x0A,
    Splash = 0x0B,
    NotWalkable = 0x0C,
    NotMoveable = 0x0D,
    BlockProjectile = 0x0E,
    NotPathable = 0x0F,
    Pickupable = 0x10,
    Hangable = 0x11,
    HookSouth = 0x12,
    HookEast = 0x13,
    Rotatable = 0x14,
    Light = 0x15,
    DontHide = 0x16,
    Translucent = 0x17,
    Displacement = 0x18,
    Elevation = 0x19,
    LyingCorpse = 0x1A,
    AnimateAlways = 0x1B,
    MinimapColor = 0x1C,
    LensHelp = 0x1D,
    FullGround = 0x1E,
    IgnoreLook = 0x1F,
    Cloth = 0x20,
    Market = 0x21,
    LastFlag = 0xFF
}