using System.ComponentModel;
using System.ComponentModel.DataAnnotations;
using System.Runtime.CompilerServices;
using System.Windows.Media.Imaging;
using System.Text.Json.Serialization;

namespace ItemEditor.Models;

/// <summary>
/// Enhanced item model with comprehensive properties and validation
/// </summary>
public class Item : INotifyPropertyChanged, IDataErrorInfo, ICloneable
{
    private string _name = string.Empty;
    private ushort _id;
    private ushort _clientId;
    private string _description = string.Empty;
    private ItemType _type = ItemType.None;
    private bool _isStackable;
    private bool _isMoveable = true;
    private bool _isPickupable = true;
    private bool _isRotatable;
    private bool _hasLight;
    private ushort _lightLevel;
    private ushort _lightColor;
    private ushort _speed;
    private ushort _weight;
    private ushort _minimapColor;
    private TileStackOrder _stackOrder = TileStackOrder.None;
    private BitmapSource? _thumbnail;
    private List<ushort> _spriteIds = new();
    private byte[]? _spriteHash;
    private DateTime _lastModified = DateTime.UtcNow;
    private bool _isDirty;
    private Dictionary<string, object> _customProperties = new();
    private readonly Dictionary<string, string> _validationErrors = new();

    /// <summary>
    /// Gets or sets the item name
    /// </summary>
    [Required(ErrorMessage = "Item name is required")]
    [StringLength(100, ErrorMessage = "Item name cannot exceed 100 characters")]
    public string Name
    {
        get => _name;
        set
        {
            if (SetProperty(ref _name, value))
            {
                ValidateProperty(value, nameof(Name));
                MarkAsDirty();
            }
        }
    }
    
    /// <summary>
    /// Gets or sets the server item ID
    /// </summary>
    [Range(1, ushort.MaxValue, ErrorMessage = "Item ID must be between 1 and 65535")]
    public ushort Id
    {
        get => _id;
        set
        {
            if (SetProperty(ref _id, value))
            {
                ValidateProperty(value, nameof(Id));
                MarkAsDirty();
            }
        }
    }
    
    /// <summary>
    /// Gets or sets the client item ID
    /// </summary>
    [Range(0, ushort.MaxValue, ErrorMessage = "Client ID must be between 0 and 65535")]
    public ushort ClientId
    {
        get => _clientId;
        set
        {
            if (SetProperty(ref _clientId, value))
            {
                ValidateProperty(value, nameof(ClientId));
                MarkAsDirty();
            }
        }
    }
    
    /// <summary>
    /// Gets or sets the item description
    /// </summary>
    [StringLength(500, ErrorMessage = "Description cannot exceed 500 characters")]
    public string Description
    {
        get => _description;
        set
        {
            if (SetProperty(ref _description, value))
            {
                ValidateProperty(value, nameof(Description));
                MarkAsDirty();
            }
        }
    }
    
    /// <summary>
    /// Gets or sets the item type
    /// </summary>
    public ItemType Type
    {
        get => _type;
        set
        {
            if (SetProperty(ref _type, value))
            {
                MarkAsDirty();
            }
        }
    }
    
    /// <summary>
    /// Gets or sets whether the item is stackable
    /// </summary>
    public bool IsStackable
    {
        get => _isStackable;
        set
        {
            if (SetProperty(ref _isStackable, value))
            {
                MarkAsDirty();
            }
        }
    }
    
    /// <summary>
    /// Gets or sets whether the item is moveable
    /// </summary>
    public bool IsMoveable
    {
        get => _isMoveable;
        set
        {
            if (SetProperty(ref _isMoveable, value))
            {
                MarkAsDirty();
            }
        }
    }
    
    /// <summary>
    /// Gets or sets whether the item is pickupable
    /// </summary>
    public bool IsPickupable
    {
        get => _isPickupable;
        set
        {
            if (SetProperty(ref _isPickupable, value))
            {
                MarkAsDirty();
            }
        }
    }
    
    /// <summary>
    /// Gets or sets whether the item is rotatable
    /// </summary>
    public bool IsRotatable
    {
        get => _isRotatable;
        set
        {
            if (SetProperty(ref _isRotatable, value))
            {
                MarkAsDirty();
            }
        }
    }
    
    /// <summary>
    /// Gets or sets whether the item has light
    /// </summary>
    public bool HasLight
    {
        get => _hasLight;
        set
        {
            if (SetProperty(ref _hasLight, value))
            {
                MarkAsDirty();
            }
        }
    }
    
    /// <summary>
    /// Gets or sets the light level (0-255)
    /// </summary>
    [Range(0, 255, ErrorMessage = "Light level must be between 0 and 255")]
    public ushort LightLevel
    {
        get => _lightLevel;
        set
        {
            if (SetProperty(ref _lightLevel, value))
            {
                ValidateProperty(value, nameof(LightLevel));
                MarkAsDirty();
            }
        }
    }
    
    /// <summary>
    /// Gets or sets the light color
    /// </summary>
    public ushort LightColor
    {
        get => _lightColor;
        set
        {
            if (SetProperty(ref _lightColor, value))
            {
                MarkAsDirty();
            }
        }
    }
    
    /// <summary>
    /// Gets or sets the item speed
    /// </summary>
    [Range(0, ushort.MaxValue, ErrorMessage = "Speed must be between 0 and 65535")]
    public ushort Speed
    {
        get => _speed;
        set
        {
            if (SetProperty(ref _speed, value))
            {
                ValidateProperty(value, nameof(Speed));
                MarkAsDirty();
            }
        }
    }
    
    /// <summary>
    /// Gets or sets the item weight
    /// </summary>
    [Range(0, ushort.MaxValue, ErrorMessage = "Weight must be between 0 and 65535")]
    public ushort Weight
    {
        get => _weight;
        set
        {
            if (SetProperty(ref _weight, value))
            {
                ValidateProperty(value, nameof(Weight));
                MarkAsDirty();
            }
        }
    }
    
    /// <summary>
    /// Gets or sets the minimap color
    /// </summary>
    public ushort MinimapColor
    {
        get => _minimapColor;
        set
        {
            if (SetProperty(ref _minimapColor, value))
            {
                MarkAsDirty();
            }
        }
    }
    
    /// <summary>
    /// Gets or sets the tile stack order
    /// </summary>
    public TileStackOrder StackOrder
    {
        get => _stackOrder;
        set
        {
            if (SetProperty(ref _stackOrder, value))
            {
                MarkAsDirty();
            }
        }
    }
    
    /// <summary>
    /// Gets or sets the item thumbnail
    /// </summary>
    [JsonIgnore]
    public BitmapSource? Thumbnail
    {
        get => _thumbnail;
        set => SetProperty(ref _thumbnail, value);
    }
    
    /// <summary>
    /// Gets or sets the sprite IDs associated with this item
    /// </summary>
    public List<ushort> SpriteIds
    {
        get => _spriteIds;
        set
        {
            if (SetProperty(ref _spriteIds, value))
            {
                MarkAsDirty();
            }
        }
    }
    
    /// <summary>
    /// Gets or sets the sprite hash for comparison
    /// </summary>
    public byte[]? SpriteHash
    {
        get => _spriteHash;
        set
        {
            if (SetProperty(ref _spriteHash, value))
            {
                MarkAsDirty();
            }
        }
    }
    
    /// <summary>
    /// Gets or sets when the item was last modified
    /// </summary>
    public DateTime LastModified
    {
        get => _lastModified;
        set => SetProperty(ref _lastModified, value);
    }
    
    /// <summary>
    /// Gets whether the item has unsaved changes
    /// </summary>
    [JsonIgnore]
    public bool IsDirty
    {
        get => _isDirty;
        private set => SetProperty(ref _isDirty, value);
    }
    
    /// <summary>
    /// Gets whether the item has validation errors
    /// </summary>
    [JsonIgnore]
    public bool HasErrors => _validationErrors.Count > 0;
    
    /// <summary>
    /// Gets custom properties dictionary
    /// </summary>
    public Dictionary<string, object> CustomProperties
    {
        get => _customProperties;
        set => SetProperty(ref _customProperties, value);
    }
    
    /// <summary>
    /// Gets validation error for the entire object
    /// </summary>
    [JsonIgnore]
    public string Error => HasErrors ? string.Join("; ", _validationErrors.Values) : string.Empty;
    
    /// <summary>
    /// Gets validation error for a specific property
    /// </summary>
    /// <param name="columnName">Property name</param>
    /// <returns>Validation error or empty string</returns>
    [JsonIgnore]
    public string this[string columnName] => _validationErrors.TryGetValue(columnName, out var error) ? error : string.Empty;
    
    /// <summary>
    /// Property changed event
    /// </summary>
    public event PropertyChangedEventHandler? PropertyChanged;
    
    /// <summary>
    /// Event raised when the item becomes dirty
    /// </summary>
    public event EventHandler? ItemChanged;
    
    /// <summary>
    /// Sets a property value and raises PropertyChanged if the value changed
    /// </summary>
    /// <typeparam name="T">Property type</typeparam>
    /// <param name="field">Field reference</param>
    /// <param name="value">New value</param>
    /// <param name="propertyName">Property name</param>
    /// <returns>True if the value changed</returns>
    protected virtual bool SetProperty<T>(ref T field, T value, [CallerMemberName] string? propertyName = null)
    {
        if (EqualityComparer<T>.Default.Equals(field, value))
            return false;
        
        field = value;
        OnPropertyChanged(propertyName);
        return true;
    }
    
    /// <summary>
    /// Raises the PropertyChanged event
    /// </summary>
    /// <param name="propertyName">Property name</param>
    protected virtual void OnPropertyChanged([CallerMemberName] string? propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
    
    /// <summary>
    /// Marks the item as having unsaved changes
    /// </summary>
    private void MarkAsDirty()
    {
        if (!IsDirty)
        {
            IsDirty = true;
            LastModified = DateTime.UtcNow;
            ItemChanged?.Invoke(this, EventArgs.Empty);
        }
    }
    
    /// <summary>
    /// Marks the item as saved (not dirty)
    /// </summary>
    public void MarkAsSaved()
    {
        IsDirty = false;
    }
    
    /// <summary>
    /// Validates a property value using data annotations
    /// </summary>
    /// <param name="value">Value to validate</param>
    /// <param name="propertyName">Property name</param>
    private void ValidateProperty(object? value, string propertyName)
    {
        var context = new ValidationContext(this) { MemberName = propertyName };
        var results = new List<ValidationResult>();
        
        if (Validator.TryValidateProperty(value, context, results))
        {
            _validationErrors.Remove(propertyName);
        }
        else
        {
            var error = results.FirstOrDefault()?.ErrorMessage ?? "Validation error";
            _validationErrors[propertyName] = error;
        }
        
        OnPropertyChanged(nameof(HasErrors));
        OnPropertyChanged(nameof(Error));
    }
    
    /// <summary>
    /// Validates all properties
    /// </summary>
    /// <returns>True if all properties are valid</returns>
    public bool ValidateAll()
    {
        _validationErrors.Clear();
        
        var context = new ValidationContext(this);
        var results = new List<ValidationResult>();
        
        if (!Validator.TryValidateObject(this, context, results, true))
        {
            foreach (var result in results)
            {
                var propertyName = result.MemberNames.FirstOrDefault() ?? "Unknown";
                _validationErrors[propertyName] = result.ErrorMessage ?? "Validation error";
            }
        }
        
        OnPropertyChanged(nameof(HasErrors));
        OnPropertyChanged(nameof(Error));
        
        return !HasErrors;
    }
    
    /// <summary>
    /// Gets a custom property value
    /// </summary>
    /// <typeparam name="T">Property type</typeparam>
    /// <param name="key">Property key</param>
    /// <param name="defaultValue">Default value if not found</param>
    /// <returns>Property value or default</returns>
    public T GetCustomProperty<T>(string key, T defaultValue = default!)
    {
        if (CustomProperties.TryGetValue(key, out var value) && value is T typedValue)
            return typedValue;
        return defaultValue;
    }
    
    /// <summary>
    /// Sets a custom property value
    /// </summary>
    /// <param name="key">Property key</param>
    /// <param name="value">Property value</param>
    public void SetCustomProperty(string key, object value)
    {
        CustomProperties[key] = value;
        MarkAsDirty();
        OnPropertyChanged(nameof(CustomProperties));
    }
    
    /// <summary>
    /// Creates a deep copy of the item
    /// </summary>
    /// <returns>Cloned item</returns>
    public object Clone()
    {
        var clone = new Item
        {
            Name = Name,
            Id = Id,
            ClientId = ClientId,
            Description = Description,
            Type = Type,
            IsStackable = IsStackable,
            IsMoveable = IsMoveable,
            IsPickupable = IsPickupable,
            IsRotatable = IsRotatable,
            HasLight = HasLight,
            LightLevel = LightLevel,
            LightColor = LightColor,
            Speed = Speed,
            Weight = Weight,
            MinimapColor = MinimapColor,
            StackOrder = StackOrder,
            SpriteIds = new List<ushort>(SpriteIds),
            SpriteHash = SpriteHash != null ? (byte[])SpriteHash.Clone() : null,
            CustomProperties = new Dictionary<string, object>(CustomProperties)
        };
        
        clone.MarkAsSaved(); // Clone starts as saved
        return clone;
    }
    
    /// <summary>
    /// Compares this item with another for equality
    /// </summary>
    /// <param name="other">Other item to compare</param>
    /// <returns>True if items are equal</returns>
    public bool Equals(Item? other)
    {
        if (other == null) return false;
        if (ReferenceEquals(this, other)) return true;
        
        return Id == other.Id &&
               ClientId == other.ClientId &&
               Name == other.Name &&
               Type == other.Type;
    }
    
    /// <summary>
    /// Gets hash code for the item
    /// </summary>
    /// <returns>Hash code</returns>
    public override int GetHashCode()
    {
        return HashCode.Combine(Id, ClientId, Name, Type);
    }
    
    /// <summary>
    /// Returns string representation of the item
    /// </summary>
    /// <returns>String representation</returns>
    public override string ToString()
    {
        return $"Item {Id}: {Name} ({Type})";
    }
}

/// <summary>
/// Item types enumeration
/// </summary>
public enum ItemType
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
    Door = 13
}

/// <summary>
/// Tile stack order enumeration
/// </summary>
public enum TileStackOrder
{
    None = 0,
    Ground = 1,
    Border = 2,
    Bottom = 3,
    Top = 4
}