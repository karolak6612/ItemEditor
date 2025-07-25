using ItemEditor.Models;

namespace ItemEditor.Services;

/// <summary>
/// Interface for modern configuration management service
/// </summary>
public interface IConfigurationService
{
    /// <summary>
    /// Loads the complete configuration from storage
    /// </summary>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>The loaded configuration</returns>
    Task<AppConfiguration> LoadConfigurationAsync(CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Saves the complete configuration to storage
    /// </summary>
    /// <param name="configuration">Configuration to save</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task representing the asynchronous operation</returns>
    Task SaveConfigurationAsync(AppConfiguration configuration, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Gets a specific configuration section
    /// </summary>
    /// <typeparam name="T">Type of the configuration section</typeparam>
    /// <param name="sectionName">Name of the section</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>The configuration section</returns>
    Task<T> GetSectionAsync<T>(string sectionName, CancellationToken cancellationToken = default) where T : class, new();
    
    /// <summary>
    /// Updates a specific configuration section
    /// </summary>
    /// <typeparam name="T">Type of the configuration section</typeparam>
    /// <param name="sectionName">Name of the section</param>
    /// <param name="section">Section data to update</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task representing the asynchronous operation</returns>
    Task UpdateSectionAsync<T>(string sectionName, T section, CancellationToken cancellationToken = default) where T : class;
    
    /// <summary>
    /// Validates the configuration against schema and business rules
    /// </summary>
    /// <param name="configuration">Configuration to validate</param>
    /// <returns>Validation result</returns>
    ConfigurationValidationResult ValidateConfiguration(AppConfiguration configuration);
    
    /// <summary>
    /// Creates a backup of the current configuration
    /// </summary>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Path to the backup file</returns>
    Task<string> CreateBackupAsync(CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Restores configuration from a backup file
    /// </summary>
    /// <param name="backupFilePath">Path to the backup file</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>True if restore was successful</returns>
    Task<bool> RestoreFromBackupAsync(string backupFilePath, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Resets configuration to default values
    /// </summary>
    /// <param name="sectionName">Optional section name to reset only that section</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task representing the asynchronous operation</returns>
    Task ResetToDefaultsAsync(string? sectionName = null, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Exports configuration to a file
    /// </summary>
    /// <param name="filePath">Path to export to</param>
    /// <param name="includeUserSettings">Whether to include user-specific settings</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task representing the asynchronous operation</returns>
    Task ExportConfigurationAsync(string filePath, bool includeUserSettings = true, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Imports configuration from a file
    /// </summary>
    /// <param name="filePath">Path to import from</param>
    /// <param name="mergeWithExisting">Whether to merge with existing configuration or replace</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>True if import was successful</returns>
    Task<bool> ImportConfigurationAsync(string filePath, bool mergeWithExisting = true, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Gets the configuration file paths
    /// </summary>
    /// <returns>Configuration file information</returns>
    ConfigurationPaths GetConfigurationPaths();
    
    /// <summary>
    /// Event raised when configuration changes
    /// </summary>
    event EventHandler<ConfigurationChangedEventArgs>? ConfigurationChanged;
}

/// <summary>
/// Configuration file paths information
/// </summary>
public class ConfigurationPaths
{
    /// <summary>
    /// Gets or sets the main configuration file path
    /// </summary>
    public string MainConfigPath { get; set; } = string.Empty;
    
    /// <summary>
    /// Gets or sets the user configuration file path
    /// </summary>
    public string UserConfigPath { get; set; } = string.Empty;
    
    /// <summary>
    /// Gets or sets the backup directory path
    /// </summary>
    public string BackupDirectory { get; set; } = string.Empty;
    
    /// <summary>
    /// Gets or sets the schema file path
    /// </summary>
    public string SchemaPath { get; set; } = string.Empty;
}

/// <summary>
/// Configuration changed event arguments
/// </summary>
public class ConfigurationChangedEventArgs : EventArgs
{
    /// <summary>
    /// Gets or sets the section that changed
    /// </summary>
    public string SectionName { get; set; } = string.Empty;
    
    /// <summary>
    /// Gets or sets the type of change
    /// </summary>
    public ConfigurationChangeType ChangeType { get; set; }
    
    /// <summary>
    /// Gets or sets the old value (if applicable)
    /// </summary>
    public object? OldValue { get; set; }
    
    /// <summary>
    /// Gets or sets the new value (if applicable)
    /// </summary>
    public object? NewValue { get; set; }
}

/// <summary>
/// Configuration change types
/// </summary>
public enum ConfigurationChangeType
{
    /// <summary>
    /// Section was added
    /// </summary>
    Added,
    
    /// <summary>
    /// Section was updated
    /// </summary>
    Updated,
    
    /// <summary>
    /// Section was removed
    /// </summary>
    Removed,
    
    /// <summary>
    /// Configuration was reset
    /// </summary>
    Reset
}