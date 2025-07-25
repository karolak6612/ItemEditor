using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace ItemEditor.Models;

/// <summary>
/// Plugin metadata containing detailed information about a plugin
/// </summary>
public class PluginMetadata : INotifyPropertyChanged
{
    private string _name = string.Empty;
    private Version _version = new();
    private string _description = string.Empty;
    private string _author = string.Empty;
    private bool _isLegacy;
    private PluginCategory _category = PluginCategory.General;
    private List<Version> _supportedHostVersions = new();
    private List<PluginDependency> _dependencies = new();
    private DateTime _createdAt = DateTime.UtcNow;
    private string _filePath = string.Empty;
    private long _fileSize;
    private string _fileHash = string.Empty;

    /// <summary>
    /// Gets or sets the plugin name
    /// </summary>
    public string Name
    {
        get => _name;
        set => SetProperty(ref _name, value);
    }

    /// <summary>
    /// Gets or sets the plugin version
    /// </summary>
    public Version Version
    {
        get => _version;
        set => SetProperty(ref _version, value);
    }

    /// <summary>
    /// Gets or sets the plugin description
    /// </summary>
    public string Description
    {
        get => _description;
        set => SetProperty(ref _description, value);
    }

    /// <summary>
    /// Gets or sets the plugin author
    /// </summary>
    public string Author
    {
        get => _author;
        set => SetProperty(ref _author, value);
    }

    /// <summary>
    /// Gets or sets whether this is a legacy plugin
    /// </summary>
    public bool IsLegacy
    {
        get => _isLegacy;
        set => SetProperty(ref _isLegacy, value);
    }

    /// <summary>
    /// Gets or sets the plugin category
    /// </summary>
    public PluginCategory Category
    {
        get => _category;
        set => SetProperty(ref _category, value);
    }

    /// <summary>
    /// Gets or sets the supported host application versions
    /// </summary>
    public List<Version> SupportedHostVersions
    {
        get => _supportedHostVersions;
        set => SetProperty(ref _supportedHostVersions, value);
    }

    /// <summary>
    /// Gets or sets the plugin dependencies
    /// </summary>
    public List<PluginDependency> Dependencies
    {
        get => _dependencies;
        set => SetProperty(ref _dependencies, value);
    }

    /// <summary>
    /// Gets or sets when the plugin was created
    /// </summary>
    public DateTime CreatedAt
    {
        get => _createdAt;
        set => SetProperty(ref _createdAt, value);
    }

    /// <summary>
    /// Gets or sets the plugin file path
    /// </summary>
    public string FilePath
    {
        get => _filePath;
        set => SetProperty(ref _filePath, value);
    }

    /// <summary>
    /// Gets or sets the plugin file size in bytes
    /// </summary>
    public long FileSize
    {
        get => _fileSize;
        set => SetProperty(ref _fileSize, value);
    }

    /// <summary>
    /// Gets or sets the plugin file hash for integrity verification
    /// </summary>
    public string FileHash
    {
        get => _fileHash;
        set => SetProperty(ref _fileHash, value);
    }

    public event PropertyChangedEventHandler? PropertyChanged;

    protected virtual void SetProperty<T>(ref T field, T value, [CallerMemberName] string? propertyName = null)
    {
        if (!EqualityComparer<T>.Default.Equals(field, value))
        {
            field = value;
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}

/// <summary>
/// Plugin categories for organization
/// </summary>
public enum PluginCategory
{
    /// <summary>
    /// General purpose plugins
    /// </summary>
    General,

    /// <summary>
    /// File format plugins
    /// </summary>
    FileFormat,

    /// <summary>
    /// Image processing plugins
    /// </summary>
    ImageProcessing,

    /// <summary>
    /// Data conversion plugins
    /// </summary>
    DataConversion,

    /// <summary>
    /// UI enhancement plugins
    /// </summary>
    UIEnhancement,

    /// <summary>
    /// Development tools plugins
    /// </summary>
    DevelopmentTools
}

/// <summary>
/// Plugin dependency information
/// </summary>
public class PluginDependency
{
    /// <summary>
    /// Gets or sets the dependency name
    /// </summary>
    public string Name { get; set; } = string.Empty;

    /// <summary>
    /// Gets or sets the minimum required version
    /// </summary>
    public Version MinVersion { get; set; } = new();

    /// <summary>
    /// Gets or sets the maximum supported version
    /// </summary>
    public Version? MaxVersion { get; set; }

    /// <summary>
    /// Gets or sets whether this dependency is optional
    /// </summary>
    public bool IsOptional { get; set; }

    /// <summary>
    /// Gets or sets the dependency type
    /// </summary>
    public DependencyType Type { get; set; } = DependencyType.Plugin;
}

/// <summary>
/// Types of plugin dependencies
/// </summary>
public enum DependencyType
{
    /// <summary>
    /// Another plugin dependency
    /// </summary>
    Plugin,

    /// <summary>
    /// .NET assembly dependency
    /// </summary>
    Assembly,

    /// <summary>
    /// System component dependency
    /// </summary>
    System
}

/// <summary>
/// Plugin validation result
/// </summary>
public class PluginValidationResult
{
    /// <summary>
    /// Gets or sets whether the plugin is valid
    /// </summary>
    public bool IsValid { get; set; }

    /// <summary>
    /// Gets or sets the validation message
    /// </summary>
    public string Message { get; set; } = string.Empty;

    /// <summary>
    /// Gets or sets detailed validation errors
    /// </summary>
    public List<string> Errors { get; set; } = new();

    /// <summary>
    /// Gets or sets validation warnings
    /// </summary>
    public List<string> Warnings { get; set; } = new();

    /// <summary>
    /// Gets or sets the validation severity
    /// </summary>
    public ValidationSeverity Severity { get; set; } = ValidationSeverity.Info;
}

/// <summary>
/// Validation severity levels
/// </summary>
public enum ValidationSeverity
{
    /// <summary>
    /// Information only
    /// </summary>
    Info,

    /// <summary>
    /// Warning - plugin may work but with issues
    /// </summary>
    Warning,

    /// <summary>
    /// Error - plugin cannot be loaded
    /// </summary>
    Error,

    /// <summary>
    /// Critical - plugin poses security risk
    /// </summary>
    Critical
}

/// <summary>
/// Plugin configuration schema for settings UI
/// </summary>
public class PluginConfigurationSchema
{
    /// <summary>
    /// Gets or sets the configuration properties
    /// </summary>
    public List<ConfigurationProperty> Properties { get; set; } = new();

    /// <summary>
    /// Gets or sets the configuration groups
    /// </summary>
    public List<ConfigurationGroup> Groups { get; set; } = new();
}

/// <summary>
/// Configuration property definition
/// </summary>
public class ConfigurationProperty
{
    /// <summary>
    /// Gets or sets the property name
    /// </summary>
    public string Name { get; set; } = string.Empty;

    /// <summary>
    /// Gets or sets the property display name
    /// </summary>
    public string DisplayName { get; set; } = string.Empty;

    /// <summary>
    /// Gets or sets the property description
    /// </summary>
    public string Description { get; set; } = string.Empty;

    /// <summary>
    /// Gets or sets the property type
    /// </summary>
    public ConfigurationPropertyType Type { get; set; }

    /// <summary>
    /// Gets or sets the default value
    /// </summary>
    public object? DefaultValue { get; set; }

    /// <summary>
    /// Gets or sets whether the property is required
    /// </summary>
    public bool IsRequired { get; set; }

    /// <summary>
    /// Gets or sets the property validation rules
    /// </summary>
    public List<ValidationRule> ValidationRules { get; set; } = new();
}

/// <summary>
/// Configuration property types
/// </summary>
public enum ConfigurationPropertyType
{
    String,
    Integer,
    Boolean,
    Decimal,
    FilePath,
    DirectoryPath,
    Color,
    Enum
}

/// <summary>
/// Configuration group for organizing properties
/// </summary>
public class ConfigurationGroup
{
    /// <summary>
    /// Gets or sets the group name
    /// </summary>
    public string Name { get; set; } = string.Empty;

    /// <summary>
    /// Gets or sets the group display name
    /// </summary>
    public string DisplayName { get; set; } = string.Empty;

    /// <summary>
    /// Gets or sets the group description
    /// </summary>
    public string Description { get; set; } = string.Empty;

    /// <summary>
    /// Gets or sets the properties in this group
    /// </summary>
    public List<string> PropertyNames { get; set; } = new();
}

/// <summary>
/// Validation rule for configuration properties
/// </summary>
public class ValidationRule
{
    /// <summary>
    /// Gets or sets the rule type
    /// </summary>
    public ValidationRuleType Type { get; set; }

    /// <summary>
    /// Gets or sets the rule value
    /// </summary>
    public object? Value { get; set; }

    /// <summary>
    /// Gets or sets the error message
    /// </summary>
    public string ErrorMessage { get; set; } = string.Empty;
}

/// <summary>
/// Validation rule types
/// </summary>
public enum ValidationRuleType
{
    Required,
    MinLength,
    MaxLength,
    MinValue,
    MaxValue,
    Regex,
    Custom
}

/// <summary>
/// Plugin configuration container
/// </summary>
public class PluginConfiguration
{
    /// <summary>
    /// Gets or sets the configuration values
    /// </summary>
    public Dictionary<string, object?> Values { get; set; } = new();

    /// <summary>
    /// Gets a configuration value
    /// </summary>
    /// <typeparam name="T">Value type</typeparam>
    /// <param name="key">Configuration key</param>
    /// <param name="defaultValue">Default value if key not found</param>
    /// <returns>Configuration value</returns>
    public T? GetValue<T>(string key, T? defaultValue = default)
    {
        if (Values.TryGetValue(key, out var value) && value is T typedValue)
        {
            return typedValue;
        }
        return defaultValue;
    }

    /// <summary>
    /// Sets a configuration value
    /// </summary>
    /// <param name="key">Configuration key</param>
    /// <param name="value">Configuration value</param>
    public void SetValue(string key, object? value)
    {
        Values[key] = value;
    }
}

/// <summary>
/// Plugin unload reasons
/// </summary>
public enum PluginUnloadReason
{
    /// <summary>
    /// User requested unload
    /// </summary>
    UserRequested,

    /// <summary>
    /// Application shutdown
    /// </summary>
    ApplicationShutdown,

    /// <summary>
    /// Plugin error
    /// </summary>
    Error,

    /// <summary>
    /// Plugin update
    /// </summary>
    Update,

    /// <summary>
    /// Dependency conflict
    /// </summary>
    DependencyConflict
}