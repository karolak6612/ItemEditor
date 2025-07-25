using System.ComponentModel;
using System.ComponentModel.DataAnnotations;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace PluginInterface;

/// <summary>
/// Plugin configuration container
/// </summary>
public class PluginConfiguration
{
    /// <summary>
    /// Configuration values dictionary
    /// </summary>
    public Dictionary<string, JsonElement> Values { get; set; } = new();
    
    /// <summary>
    /// Get configuration value with type conversion
    /// </summary>
    /// <typeparam name="T">Value type</typeparam>
    /// <param name="key">Configuration key</param>
    /// <param name="defaultValue">Default value if key not found</param>
    /// <returns>Configuration value or default</returns>
    public T GetValue<T>(string key, T defaultValue = default!)
    {
        if (!Values.TryGetValue(key, out var element))
            return defaultValue;
            
        try
        {
            return JsonSerializer.Deserialize<T>(element.GetRawText()) ?? defaultValue;
        }
        catch
        {
            return defaultValue;
        }
    }
    
    /// <summary>
    /// Set configuration value
    /// </summary>
    /// <typeparam name="T">Value type</typeparam>
    /// <param name="key">Configuration key</param>
    /// <param name="value">Configuration value</param>
    public void SetValue<T>(string key, T value)
    {
        var json = JsonSerializer.Serialize(value);
        Values[key] = JsonDocument.Parse(json).RootElement.Clone();
    }
    
    /// <summary>
    /// Check if configuration has a specific key
    /// </summary>
    /// <param name="key">Configuration key</param>
    /// <returns>True if key exists</returns>
    public bool HasValue(string key) => Values.ContainsKey(key);
    
    /// <summary>
    /// Remove configuration value
    /// </summary>
    /// <param name="key">Configuration key</param>
    /// <returns>True if key was removed</returns>
    public bool RemoveValue(string key) => Values.Remove(key);
    
    /// <summary>
    /// Clear all configuration values
    /// </summary>
    public void Clear() => Values.Clear();
}

/// <summary>
/// Plugin configuration schema for UI generation
/// </summary>
public class PluginConfigurationSchema
{
    /// <summary>
    /// Schema title
    /// </summary>
    public string Title { get; set; } = string.Empty;
    
    /// <summary>
    /// Schema description
    /// </summary>
    public string Description { get; set; } = string.Empty;
    
    /// <summary>
    /// Configuration properties
    /// </summary>
    public List<ConfigurationProperty> Properties { get; set; } = new();
    
    /// <summary>
    /// Property groups for organization
    /// </summary>
    public List<PropertyGroup> Groups { get; set; } = new();
}

/// <summary>
/// Configuration property definition
/// </summary>
public class ConfigurationProperty
{
    /// <summary>
    /// Property key
    /// </summary>
    public required string Key { get; set; }
    
    /// <summary>
    /// Property display name
    /// </summary>
    public required string DisplayName { get; set; }
    
    /// <summary>
    /// Property description
    /// </summary>
    public string Description { get; set; } = string.Empty;
    
    /// <summary>
    /// Property type
    /// </summary>
    public ConfigurationPropertyType Type { get; set; }
    
    /// <summary>
    /// Default value
    /// </summary>
    public object? DefaultValue { get; set; }
    
    /// <summary>
    /// Whether the property is required
    /// </summary>
    public bool IsRequired { get; set; }
    
    /// <summary>
    /// Whether the property is read-only
    /// </summary>
    public bool IsReadOnly { get; set; }
    
    /// <summary>
    /// Property validation attributes
    /// </summary>
    public List<ValidationAttribute> Validations { get; set; } = new();
    
    /// <summary>
    /// Available options for enum/select types
    /// </summary>
    public List<ConfigurationOption> Options { get; set; } = new();
    
    /// <summary>
    /// Property group (for UI organization)
    /// </summary>
    public string? Group { get; set; }
    
    /// <summary>
    /// Display order within group
    /// </summary>
    public int Order { get; set; }
}

/// <summary>
/// Configuration property types
/// </summary>
public enum ConfigurationPropertyType
{
    /// <summary>
    /// String text input
    /// </summary>
    String,
    
    /// <summary>
    /// Integer number input
    /// </summary>
    Integer,
    
    /// <summary>
    /// Decimal number input
    /// </summary>
    Decimal,
    
    /// <summary>
    /// Boolean checkbox
    /// </summary>
    Boolean,
    
    /// <summary>
    /// Enum/select dropdown
    /// </summary>
    Enum,
    
    /// <summary>
    /// File path selector
    /// </summary>
    FilePath,
    
    /// <summary>
    /// Directory path selector
    /// </summary>
    DirectoryPath,
    
    /// <summary>
    /// Color picker
    /// </summary>
    Color,
    
    /// <summary>
    /// Multi-line text area
    /// </summary>
    TextArea,
    
    /// <summary>
    /// Password input
    /// </summary>
    Password
}

/// <summary>
/// Configuration option for enum/select properties
/// </summary>
public class ConfigurationOption
{
    /// <summary>
    /// Option value
    /// </summary>
    public required object Value { get; set; }
    
    /// <summary>
    /// Option display text
    /// </summary>
    public required string DisplayText { get; set; }
    
    /// <summary>
    /// Option description
    /// </summary>
    public string Description { get; set; } = string.Empty;
}

/// <summary>
/// Property group for UI organization
/// </summary>
public class PropertyGroup
{
    /// <summary>
    /// Group name
    /// </summary>
    public required string Name { get; set; }
    
    /// <summary>
    /// Group display name
    /// </summary>
    public required string DisplayName { get; set; }
    
    /// <summary>
    /// Group description
    /// </summary>
    public string Description { get; set; } = string.Empty;
    
    /// <summary>
    /// Group display order
    /// </summary>
    public int Order { get; set; }
    
    /// <summary>
    /// Whether the group is collapsible
    /// </summary>
    public bool IsCollapsible { get; set; } = true;
    
    /// <summary>
    /// Whether the group is initially expanded
    /// </summary>
    public bool IsExpanded { get; set; } = true;
}