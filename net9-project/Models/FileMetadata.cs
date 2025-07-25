using System.ComponentModel;
using System.ComponentModel.DataAnnotations;
using System.Runtime.CompilerServices;

namespace ItemEditor.Models;

/// <summary>
/// Enhanced file metadata with comprehensive information and validation
/// </summary>
public class FileMetadata : INotifyPropertyChanged, IDataErrorInfo
{
    private string _filePath = string.Empty;
    private FileType _type;
    private long _fileSize;
    private DateTime _lastModified;
    private DateTime _lastAccessed;
    private uint _signature;
    private Version _clientVersion = new();
    private int _itemCount;
    private int _spriteCount;
    private bool _isValid;
    private bool _isReadOnly;
    private bool _isCompressed;
    private string _checksum = string.Empty;
    private string? _errorMessage;
    private Dictionary<string, object> _additionalProperties = new();
    private readonly Dictionary<string, string> _validationErrors = new();

    /// <summary>
    /// Gets or sets the file path
    /// </summary>
    [Required(ErrorMessage = "File path is required")]
    public string FilePath
    {
        get => _filePath;
        set
        {
            if (SetProperty(ref _filePath, value))
            {
                ValidateProperty(value, nameof(FilePath));
                OnPropertyChanged(nameof(FileName));
                OnPropertyChanged(nameof(Directory));
                OnPropertyChanged(nameof(FileExists));
            }
        }
    }
    
    /// <summary>
    /// Gets or sets the file type
    /// </summary>
    public FileType Type
    {
        get => _type;
        set
        {
            if (SetProperty(ref _type, value))
            {
                OnPropertyChanged(nameof(TypeDisplayName));
                OnPropertyChanged(nameof(FileExtension));
            }
        }
    }
    
    /// <summary>
    /// Gets or sets the file size in bytes
    /// </summary>
    [Range(0, long.MaxValue, ErrorMessage = "File size must be non-negative")]
    public long FileSize
    {
        get => _fileSize;
        set
        {
            if (SetProperty(ref _fileSize, value))
            {
                ValidateProperty(value, nameof(FileSize));
                OnPropertyChanged(nameof(FileSizeFormatted));
            }
        }
    }
    
    /// <summary>
    /// Gets or sets the last modified date
    /// </summary>
    public DateTime LastModified
    {
        get => _lastModified;
        set
        {
            if (SetProperty(ref _lastModified, value))
            {
                OnPropertyChanged(nameof(FileAge));
            }
        }
    }
    
    /// <summary>
    /// Gets or sets the last accessed date
    /// </summary>
    public DateTime LastAccessed
    {
        get => _lastAccessed;
        set => SetProperty(ref _lastAccessed, value);
    }
    
    /// <summary>
    /// Gets or sets the file signature
    /// </summary>
    public uint Signature
    {
        get => _signature;
        set
        {
            if (SetProperty(ref _signature, value))
            {
                OnPropertyChanged(nameof(SignatureHex));
            }
        }
    }
    
    /// <summary>
    /// Gets or sets the client version
    /// </summary>
    public Version ClientVersion
    {
        get => _clientVersion;
        set => SetProperty(ref _clientVersion, value);
    }
    
    /// <summary>
    /// Gets or sets the item count
    /// </summary>
    [Range(0, int.MaxValue, ErrorMessage = "Item count must be non-negative")]
    public int ItemCount
    {
        get => _itemCount;
        set
        {
            if (SetProperty(ref _itemCount, value))
            {
                ValidateProperty(value, nameof(ItemCount));
            }
        }
    }
    
    /// <summary>
    /// Gets or sets the sprite count
    /// </summary>
    [Range(0, int.MaxValue, ErrorMessage = "Sprite count must be non-negative")]
    public int SpriteCount
    {
        get => _spriteCount;
        set
        {
            if (SetProperty(ref _spriteCount, value))
            {
                ValidateProperty(value, nameof(SpriteCount));
            }
        }
    }
    
    /// <summary>
    /// Gets or sets whether the file is valid
    /// </summary>
    public bool IsValid
    {
        get => _isValid;
        set => SetProperty(ref _isValid, value);
    }
    
    /// <summary>
    /// Gets or sets whether the file is read-only
    /// </summary>
    public bool IsReadOnly
    {
        get => _isReadOnly;
        set => SetProperty(ref _isReadOnly, value);
    }
    
    /// <summary>
    /// Gets or sets whether the file is compressed
    /// </summary>
    public bool IsCompressed
    {
        get => _isCompressed;
        set => SetProperty(ref _isCompressed, value);
    }
    
    /// <summary>
    /// Gets or sets the file checksum
    /// </summary>
    public string Checksum
    {
        get => _checksum;
        set => SetProperty(ref _checksum, value);
    }
    
    /// <summary>
    /// Gets or sets the error message if validation failed
    /// </summary>
    public string? ErrorMessage
    {
        get => _errorMessage;
        set => SetProperty(ref _errorMessage, value);
    }
    
    /// <summary>
    /// Gets or sets additional properties
    /// </summary>
    public Dictionary<string, object> AdditionalProperties
    {
        get => _additionalProperties;
        set => SetProperty(ref _additionalProperties, value);
    }

    #region Computed Properties

    /// <summary>
    /// Gets the formatted file size
    /// </summary>
    public string FileSizeFormatted
    {
        get
        {
            if (FileSize == 0) return "0 bytes";
            
            string[] sizes = { "bytes", "KB", "MB", "GB", "TB" };
            double len = FileSize;
            int order = 0;
            
            while (len >= 1024 && order < sizes.Length - 1)
            {
                order++;
                len /= 1024;
            }
            
            return $"{len:0.##} {sizes[order]}";
        }
    }
    
    /// <summary>
    /// Gets the display name for the file type
    /// </summary>
    public string TypeDisplayName => Type switch
    {
        FileType.Unknown => "Unknown",
        FileType.OTB => "OpenTibia Binary",
        FileType.DAT => "Data File",
        FileType.SPR => "Sprite File",
        _ => Type.ToString()
    };
    
    /// <summary>
    /// Gets the file extension based on type
    /// </summary>
    public string FileExtension => Type switch
    {
        FileType.OTB => ".otb",
        FileType.DAT => ".dat",
        FileType.SPR => ".spr",
        _ => string.Empty
    };
    
    /// <summary>
    /// Gets the signature as hexadecimal string
    /// </summary>
    public string SignatureHex => $"0x{Signature:X8}";
    
    /// <summary>
    /// Gets the file name without path
    /// </summary>
    public string FileName => Path.GetFileName(FilePath);
    
    /// <summary>
    /// Gets the file directory
    /// </summary>
    public string Directory => Path.GetDirectoryName(FilePath) ?? string.Empty;
    
    /// <summary>
    /// Gets whether the file exists
    /// </summary>
    public bool FileExists => File.Exists(FilePath);
    
    /// <summary>
    /// Gets the age of the file
    /// </summary>
    public TimeSpan FileAge => DateTime.Now - LastModified;
    
    /// <summary>
    /// Gets whether the file has validation errors
    /// </summary>
    public bool HasErrors => _validationErrors.Count > 0;

    #endregion

    #region IDataErrorInfo Implementation

    /// <summary>
    /// Gets validation error for the entire object
    /// </summary>
    public string Error => HasErrors ? string.Join("; ", _validationErrors.Values) : string.Empty;
    
    /// <summary>
    /// Gets validation error for a specific property
    /// </summary>
    /// <param name="columnName">Property name</param>
    /// <returns>Validation error or empty string</returns>
    public string this[string columnName] => _validationErrors.TryGetValue(columnName, out var error) ? error : string.Empty;

    #endregion

    #region INotifyPropertyChanged Implementation

    /// <summary>
    /// Property changed event
    /// </summary>
    public event PropertyChangedEventHandler? PropertyChanged;
    
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

    #endregion

    #region Validation

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

    #endregion

    #region Public Methods

    /// <summary>
    /// Gets an additional property value
    /// </summary>
    /// <typeparam name="T">Property type</typeparam>
    /// <param name="key">Property key</param>
    /// <param name="defaultValue">Default value if not found</param>
    /// <returns>Property value or default</returns>
    public T GetAdditionalProperty<T>(string key, T defaultValue = default!)
    {
        if (AdditionalProperties.TryGetValue(key, out var value) && value is T typedValue)
            return typedValue;
        return defaultValue;
    }
    
    /// <summary>
    /// Sets an additional property value
    /// </summary>
    /// <param name="key">Property key</param>
    /// <param name="value">Property value</param>
    public void SetAdditionalProperty(string key, object value)
    {
        AdditionalProperties[key] = value;
        OnPropertyChanged(nameof(AdditionalProperties));
    }
    
    /// <summary>
    /// Refreshes file system information
    /// </summary>
    public void RefreshFileInfo()
    {
        if (File.Exists(FilePath))
        {
            var fileInfo = new FileInfo(FilePath);
            FileSize = fileInfo.Length;
            LastModified = fileInfo.LastWriteTime;
            LastAccessed = fileInfo.LastAccessTime;
            IsReadOnly = fileInfo.IsReadOnly;
            
            OnPropertyChanged(nameof(FileExists));
            OnPropertyChanged(nameof(FileAge));
        }
    }
    
    /// <summary>
    /// Creates a summary string of the metadata
    /// </summary>
    /// <returns>Summary string</returns>
    public string CreateSummary()
    {
        var summary = new List<string>
        {
            $"File: {FileName}",
            $"Type: {TypeDisplayName}",
            $"Size: {FileSizeFormatted}",
            $"Modified: {LastModified:yyyy-MM-dd HH:mm:ss}"
        };
        
        if (ItemCount > 0)
            summary.Add($"Items: {ItemCount:N0}");
        
        if (SpriteCount > 0)
            summary.Add($"Sprites: {SpriteCount:N0}");
        
        if (ClientVersion != null && ClientVersion.Major > 0)
            summary.Add($"Version: {ClientVersion}");
        
        if (!IsValid && !string.IsNullOrEmpty(ErrorMessage))
            summary.Add($"Error: {ErrorMessage}");
        
        return string.Join(Environment.NewLine, summary);
    }

    #endregion
}

/// <summary>
/// Enumeration of supported file types
/// </summary>
public enum FileType
{
    /// <summary>
    /// Unknown file type
    /// </summary>
    Unknown = 0,
    
    /// <summary>
    /// OpenTibia Binary file
    /// </summary>
    OTB = 1,
    
    /// <summary>
    /// Data file
    /// </summary>
    DAT = 2,
    
    /// <summary>
    /// Sprite file
    /// </summary>
    SPR = 3
}