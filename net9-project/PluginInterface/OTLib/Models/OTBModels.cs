namespace PluginInterface.OTLib.FileFormats;

/// <summary>
/// OTB file header structure
/// </summary>
public class OTBHeader
{
    /// <summary>
    /// Valid OTB file signature
    /// </summary>
    public const uint ValidSignature = 0x00000000; // Update with actual signature

    /// <summary>
    /// Header size in bytes
    /// </summary>
    public const int HeaderSize = 16;

    /// <summary>
    /// File signature
    /// </summary>
    public uint Signature { get; set; }

    /// <summary>
    /// OTB format version
    /// </summary>
    public uint Version { get; set; }

    /// <summary>
    /// Client version this OTB was created for
    /// </summary>
    public uint ClientVersion { get; set; }

    /// <summary>
    /// Build number
    /// </summary>
    public uint BuildNumber { get; set; }
}

/// <summary>
/// OTB item representation
/// </summary>
public class OTBItem
{
    /// <summary>
    /// Item type
    /// </summary>
    public OTBItemType Type { get; set; }

    /// <summary>
    /// Item ID
    /// </summary>
    public ushort Id { get; set; }

    /// <summary>
    /// Item properties
    /// </summary>
    public Dictionary<OTBItemProperty, object> Properties { get; set; } = new();

    /// <summary>
    /// Get property value with type conversion
    /// </summary>
    /// <typeparam name="T">Property type</typeparam>
    /// <param name="property">Property to get</param>
    /// <param name="defaultValue">Default value if property not found</param>
    /// <returns>Property value or default</returns>
    public T GetProperty<T>(OTBItemProperty property, T defaultValue = default!)
    {
        if (Properties.TryGetValue(property, out var value) && value is T typedValue)
            return typedValue;
        return defaultValue;
    }

    /// <summary>
    /// Set property value
    /// </summary>
    /// <param name="property">Property to set</param>
    /// <param name="value">Property value</param>
    public void SetProperty(OTBItemProperty property, object value)
    {
        Properties[property] = value;
    }

    /// <summary>
    /// Check if item has a specific property
    /// </summary>
    /// <param name="property">Property to check</param>
    /// <returns>True if property exists</returns>
    public bool HasProperty(OTBItemProperty property) => Properties.ContainsKey(property);

    /// <summary>
    /// Remove a property
    /// </summary>
    /// <param name="property">Property to remove</param>
    /// <returns>True if property was removed</returns>
    public bool RemoveProperty(OTBItemProperty property) => Properties.Remove(property);
}

/// <summary>
/// OTB item types
/// </summary>
public enum OTBItemType : byte
{
    None = 0,
    Ground = 1,
    Container = 2,
    Weapon = 3,
    Ammunition = 4,
    Armor = 5,
    Charges = 6,
    Teleport = 7,
    MagicField = 8,
    Writeable = 9,
    Key = 10,
    Splash = 11,
    Fluid = 12,
    Door = 13,
    Deprecated = 14
}

/// <summary>
/// OTB item properties
/// </summary>
public enum OTBItemProperty : byte
{
    ServerID = 0x10,
    ClientID = 0x11,
    Name = 0x12,
    Description = 0x13,
    Speed = 0x14,
    Slot = 0x15,
    MaxItems = 0x16,
    Weight = 0x17,
    Weapon = 0x18,
    Ammunition = 0x19,
    Armor = 0x1A,
    MagicLevel = 0x1B,
    MagicFieldType = 0x1C,
    Writeable = 0x1D,
    RotateTo = 0x1E,
    Decay = 0x1F,
    SpriteHash = 0x20,
    MinimapColor = 0x21,
    MaxTextLen = 0x22,
    MaxTextLenOnce = 0x23,
    Light = 0x24,
    Decay2 = 0x25,
    Weapon2 = 0x26,
    Ammunition2 = 0x27,
    Armor2 = 0x28,
    Writeable2 = 0x29,
    Light2 = 0x2A,
    TopOrder = 0x2B,
    Writeable3 = 0x2C
}

/// <summary>
/// Light information for items
/// </summary>
public class OTBLightInfo
{
    /// <summary>
    /// Light level (0-255)
    /// </summary>
    public ushort Level { get; set; }

    /// <summary>
    /// Light color
    /// </summary>
    public ushort Color { get; set; }
}

/// <summary>
/// File read progress information
/// </summary>
public class FileReadProgress
{
    /// <summary>
    /// Number of items read so far
    /// </summary>
    public int ItemsRead { get; set; }

    /// <summary>
    /// Total bytes read
    /// </summary>
    public long BytesRead { get; set; }

    /// <summary>
    /// Current operation description
    /// </summary>
    public string CurrentOperation { get; set; } = string.Empty;

    /// <summary>
    /// Progress percentage (0-100)
    /// </summary>
    public double ProgressPercentage { get; set; }

    /// <summary>
    /// Estimated time remaining
    /// </summary>
    public TimeSpan? EstimatedTimeRemaining { get; set; }
}