using System.Text.Json.Serialization;

namespace PluginInterface;

/// <summary>
/// Enhanced plugin metadata with version compatibility checking
/// </summary>
public class PluginMetadata
{
    /// <summary>
    /// Plugin name
    /// </summary>
    public required string Name { get; set; }
    
    /// <summary>
    /// Plugin version
    /// </summary>
    public required Version Version { get; set; }
    
    /// <summary>
    /// Plugin description
    /// </summary>
    public required string Description { get; set; }
    
    /// <summary>
    /// Plugin author
    /// </summary>
    public required string Author { get; set; }
    
    /// <summary>
    /// Plugin website or repository URL
    /// </summary>
    public string? Website { get; set; }
    
    /// <summary>
    /// Plugin license information
    /// </summary>
    public string? License { get; set; }
    
    /// <summary>
    /// Supported host application versions
    /// </summary>
    public Version[] SupportedHostVersions { get; set; } = Array.Empty<Version>();
    
    /// <summary>
    /// Minimum required host version
    /// </summary>
    public Version? MinimumHostVersion { get; set; }
    
    /// <summary>
    /// Maximum supported host version
    /// </summary>
    public Version? MaximumHostVersion { get; set; }
    
    /// <summary>
    /// Plugin dependencies
    /// </summary>
    public PluginDependency[] Dependencies { get; set; } = Array.Empty<PluginDependency>();
    
    /// <summary>
    /// Plugin tags for categorization
    /// </summary>
    public string[] Tags { get; set; } = Array.Empty<string>();
    
    /// <summary>
    /// Plugin category
    /// </summary>
    public PluginCategory Category { get; set; } = PluginCategory.Utility;
    
    /// <summary>
    /// Whether this is a legacy plugin
    /// </summary>
    public bool IsLegacy { get; set; }
    
    /// <summary>
    /// Plugin file path
    /// </summary>
    [JsonIgnore]
    public string? FilePath { get; set; }
    
    /// <summary>
    /// When the plugin was loaded
    /// </summary>
    [JsonIgnore]
    public DateTime LoadedAt { get; set; }
    
    /// <summary>
    /// Plugin load time in milliseconds
    /// </summary>
    [JsonIgnore]
    public long LoadTimeMs { get; set; }
}

/// <summary>
/// Plugin dependency information
/// </summary>
public class PluginDependency
{
    /// <summary>
    /// Dependency name
    /// </summary>
    public required string Name { get; set; }
    
    /// <summary>
    /// Required version or version range
    /// </summary>
    public required string VersionRange { get; set; }
    
    /// <summary>
    /// Whether the dependency is optional
    /// </summary>
    public bool IsOptional { get; set; }
}

/// <summary>
/// Plugin categories for organization
/// </summary>
public enum PluginCategory
{
    /// <summary>
    /// General utility plugins
    /// </summary>
    Utility,
    
    /// <summary>
    /// File format plugins
    /// </summary>
    FileFormat,
    
    /// <summary>
    /// Image processing plugins
    /// </summary>
    ImageProcessing,
    
    /// <summary>
    /// Data analysis plugins
    /// </summary>
    Analysis,
    
    /// <summary>
    /// Import/Export plugins
    /// </summary>
    ImportExport,
    
    /// <summary>
    /// Validation plugins
    /// </summary>
    Validation,
    
    /// <summary>
    /// Development tools
    /// </summary>
    Development
}

/// <summary>
/// Plugin validation result
/// </summary>
public class PluginValidationResult
{
    /// <summary>
    /// Whether the plugin is valid and compatible
    /// </summary>
    public bool IsValid { get; set; }
    
    /// <summary>
    /// Validation message
    /// </summary>
    public string Message { get; set; } = string.Empty;
    
    /// <summary>
    /// Validation warnings
    /// </summary>
    public string[] Warnings { get; set; } = Array.Empty<string>();
    
    /// <summary>
    /// Validation errors
    /// </summary>
    public string[] Errors { get; set; } = Array.Empty<string>();
    
    /// <summary>
    /// Missing dependencies
    /// </summary>
    public PluginDependency[] MissingDependencies { get; set; } = Array.Empty<PluginDependency>();
}