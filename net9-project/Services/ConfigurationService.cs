using Microsoft.Extensions.Logging;
using System.ComponentModel.DataAnnotations;
using System.Text.Json;
using System.Text.Json.Serialization;
using ItemEditor.Models;

namespace ItemEditor.Services;

/// <summary>
/// Modern configuration management service with JSON storage and validation
/// </summary>
public class ConfigurationService : IConfigurationService
{
    private readonly ILogger<ConfigurationService> _logger;
    private readonly JsonSerializerOptions _jsonOptions;
    private readonly ConfigurationPaths _paths;
    private AppConfiguration _currentConfiguration;
    private readonly object _configLock = new();
    
    /// <summary>
    /// Initializes a new instance of the ConfigurationService class
    /// </summary>
    /// <param name="logger">Logger instance</param>
    public ConfigurationService(ILogger<ConfigurationService> logger)
    {
        _logger = logger;
        _currentConfiguration = new AppConfiguration();
        
        // Configure JSON serialization options
        _jsonOptions = new JsonSerializerOptions
        {
            WriteIndented = true,
            PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
            DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull,
            Converters = { new JsonStringEnumConverter() }
        };
        
        // Initialize configuration paths
        var appDataPath = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
        var configDirectory = Path.Combine(appDataPath, "ItemEditor");
        
        _paths = new ConfigurationPaths
        {
            MainConfigPath = Path.Combine(configDirectory, "config.json"),
            UserConfigPath = Path.Combine(configDirectory, "user-config.json"),
            BackupDirectory = Path.Combine(configDirectory, "Backups"),
            SchemaPath = Path.Combine(configDirectory, "config-schema.json")
        };
        
        // Ensure directories exist
        Directory.CreateDirectory(configDirectory);
        Directory.CreateDirectory(_paths.BackupDirectory);
    }
    
    /// <inheritdoc />
    public event EventHandler<ConfigurationChangedEventArgs>? ConfigurationChanged;
    
    /// <inheritdoc />
    public async Task<AppConfiguration> LoadConfigurationAsync(CancellationToken cancellationToken = default)
    {
        try
        {
            _logger.LogDebug("Loading configuration from {MainConfigPath}", _paths.MainConfigPath);
            
            var configuration = new AppConfiguration();
            
            // Load main configuration
            if (File.Exists(_paths.MainConfigPath))
            {
                var mainConfigJson = await File.ReadAllTextAsync(_paths.MainConfigPath, cancellationToken);
                var mainConfig = JsonSerializer.Deserialize<AppConfiguration>(mainConfigJson, _jsonOptions);
                if (mainConfig != null)
                {
                    configuration = mainConfig;
                }
            }
            
            // Load user-specific configuration and merge
            if (File.Exists(_paths.UserConfigPath))
            {
                var userConfigJson = await File.ReadAllTextAsync(_paths.UserConfigPath, cancellationToken);
                var userConfig = JsonSerializer.Deserialize<AppConfiguration>(userConfigJson, _jsonOptions);
                if (userConfig != null)
                {
                    MergeUserConfiguration(configuration, userConfig);
                }
            }
            
            // Validate the loaded configuration
            var validationResult = ValidateConfiguration(configuration);
            if (!validationResult.IsValid)
            {
                _logger.LogWarning("Configuration validation failed: {Errors}", string.Join(", ", validationResult.Errors));
                
                // Use defaults for invalid sections
                configuration = ApplyDefaultsForInvalidSections(configuration, validationResult);
            }
            
            lock (_configLock)
            {
                _currentConfiguration = configuration;
            }
            
            _logger.LogInformation("Configuration loaded successfully");
            return configuration;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to load configuration, using defaults");
            
            var defaultConfig = new AppConfiguration();
            lock (_configLock)
            {
                _currentConfiguration = defaultConfig;
            }
            return defaultConfig;
        }
    }
    
    /// <inheritdoc />
    public async Task SaveConfigurationAsync(AppConfiguration configuration, CancellationToken cancellationToken = default)
    {
        try
        {
            _logger.LogDebug("Saving configuration to {MainConfigPath}", _paths.MainConfigPath);
            
            // Validate before saving
            var validationResult = ValidateConfiguration(configuration);
            if (!validationResult.IsValid)
            {
                throw new InvalidOperationException($"Cannot save invalid configuration: {string.Join(", ", validationResult.Errors)}");
            }
            
            // Create backup before saving
            if (File.Exists(_paths.MainConfigPath))
            {
                await CreateBackupAsync(cancellationToken);
            }
            
            // Split configuration into main and user parts
            var (mainConfig, userConfig) = SplitConfiguration(configuration);
            
            // Save main configuration
            var mainConfigJson = JsonSerializer.Serialize(mainConfig, _jsonOptions);
            await File.WriteAllTextAsync(_paths.MainConfigPath, mainConfigJson, cancellationToken);
            
            // Save user configuration
            var userConfigJson = JsonSerializer.Serialize(userConfig, _jsonOptions);
            await File.WriteAllTextAsync(_paths.UserConfigPath, userConfigJson, cancellationToken);
            
            lock (_configLock)
            {
                _currentConfiguration = configuration;
            }
            
            // Raise configuration changed event
            ConfigurationChanged?.Invoke(this, new ConfigurationChangedEventArgs
            {
                SectionName = "All",
                ChangeType = ConfigurationChangeType.Updated,
                NewValue = configuration
            });
            
            _logger.LogInformation("Configuration saved successfully");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to save configuration");
            throw;
        }
    }
    
    /// <inheritdoc />
    public async Task<T> GetSectionAsync<T>(string sectionName, CancellationToken cancellationToken = default) where T : class, new()
    {
        try
        {
            var configuration = await LoadConfigurationAsync(cancellationToken);
            
            var section = sectionName.ToLowerInvariant() switch
            {
                "application" => configuration.Application as T,
                "user" => configuration.User as T,
                "plugins" => configuration.Plugins as T,
                "files" => configuration.Files as T,
                "ui" => configuration.UI as T,
                "performance" => configuration.Performance as T,
                _ => throw new ArgumentException($"Unknown configuration section: {sectionName}")
            };
            
            return section ?? new T();
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to get configuration section: {SectionName}", sectionName);
            return new T();
        }
    }
    
    /// <inheritdoc />
    public async Task UpdateSectionAsync<T>(string sectionName, T section, CancellationToken cancellationToken = default) where T : class
    {
        try
        {
            var configuration = await LoadConfigurationAsync(cancellationToken);
            var oldValue = await GetSectionAsync<T>(sectionName, cancellationToken);
            
            switch (sectionName.ToLowerInvariant())
            {
                case "application":
                    configuration.Application = section as ApplicationSettings ?? configuration.Application;
                    break;
                case "user":
                    configuration.User = section as UserSettings ?? configuration.User;
                    break;
                case "plugins":
                    configuration.Plugins = section as PluginSettings ?? configuration.Plugins;
                    break;
                case "files":
                    configuration.Files = section as FileSettings ?? configuration.Files;
                    break;
                case "ui":
                    configuration.UI = section as UISettings ?? configuration.UI;
                    break;
                case "performance":
                    configuration.Performance = section as PerformanceSettings ?? configuration.Performance;
                    break;
                default:
                    throw new ArgumentException($"Unknown configuration section: {sectionName}");
            }
            
            await SaveConfigurationAsync(configuration, cancellationToken);
            
            // Raise section-specific change event
            ConfigurationChanged?.Invoke(this, new ConfigurationChangedEventArgs
            {
                SectionName = sectionName,
                ChangeType = ConfigurationChangeType.Updated,
                OldValue = oldValue,
                NewValue = section
            });
            
            _logger.LogDebug("Configuration section {SectionName} updated", sectionName);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to update configuration section: {SectionName}", sectionName);
            throw;
        }
    }
    
    /// <inheritdoc />
    public ConfigurationValidationResult ValidateConfiguration(AppConfiguration configuration)
    {
        var result = new ConfigurationValidationResult();
        var context = new ValidationContext(configuration);
        var validationResults = new List<ValidationResult>();
        
        try
        {
            // Validate the entire configuration object
            if (!Validator.TryValidateObject(configuration, context, validationResults, true))
            {
                result.IsValid = false;
                result.Errors.AddRange(validationResults.Select(vr => vr.ErrorMessage ?? "Unknown validation error"));
            }
            
            // Validate individual sections
            ValidateSection(configuration.Application, "Application", result);
            ValidateSection(configuration.User, "User", result);
            ValidateSection(configuration.Plugins, "Plugins", result);
            ValidateSection(configuration.Files, "Files", result);
            ValidateSection(configuration.UI, "UI", result);
            ValidateSection(configuration.Performance, "Performance", result);
            
            // Business rule validations
            ValidateBusinessRules(configuration, result);
            
            if (result.Errors.Count > 0)
            {
                result.IsValid = false;
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error during configuration validation");
            result.IsValid = false;
            result.Errors.Add($"Validation error: {ex.Message}");
        }
        
        return result;
    }
    
    /// <inheritdoc />
    public async Task<string> CreateBackupAsync(CancellationToken cancellationToken = default)
    {
        try
        {
            var timestamp = DateTime.Now.ToString("yyyyMMdd_HHmmss");
            var backupFileName = $"config_backup_{timestamp}.json";
            var backupPath = Path.Combine(_paths.BackupDirectory, backupFileName);
            
            if (File.Exists(_paths.MainConfigPath))
            {
                File.Copy(_paths.MainConfigPath, backupPath, true);
                _logger.LogInformation("Configuration backup created: {BackupPath}", backupPath);
            }
            
            // Also backup user config if it exists
            if (File.Exists(_paths.UserConfigPath))
            {
                var userBackupPath = Path.Combine(_paths.BackupDirectory, $"user_config_backup_{timestamp}.json");
                File.Copy(_paths.UserConfigPath, userBackupPath, true);
            }
            
            // Clean up old backups (keep last 10)
            await CleanupOldBackupsAsync(cancellationToken);
            
            return backupPath;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to create configuration backup");
            throw;
        }
    }
    
    /// <inheritdoc />
    public async Task<bool> RestoreFromBackupAsync(string backupFilePath, CancellationToken cancellationToken = default)
    {
        try
        {
            if (!File.Exists(backupFilePath))
            {
                _logger.LogError("Backup file not found: {BackupFilePath}", backupFilePath);
                return false;
            }
            
            // Validate backup file
            var backupContent = await File.ReadAllTextAsync(backupFilePath, cancellationToken);
            var backupConfig = JsonSerializer.Deserialize<AppConfiguration>(backupContent, _jsonOptions);
            
            if (backupConfig == null)
            {
                _logger.LogError("Invalid backup file format: {BackupFilePath}", backupFilePath);
                return false;
            }
            
            var validationResult = ValidateConfiguration(backupConfig);
            if (!validationResult.IsValid)
            {
                _logger.LogWarning("Backup configuration is invalid, but proceeding with restore");
            }
            
            // Create backup of current configuration before restore
            await CreateBackupAsync(cancellationToken);
            
            // Restore configuration
            await SaveConfigurationAsync(backupConfig, cancellationToken);
            
            _logger.LogInformation("Configuration restored from backup: {BackupFilePath}", backupFilePath);
            return true;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to restore configuration from backup: {BackupFilePath}", backupFilePath);
            return false;
        }
    }
    
    /// <inheritdoc />
    public async Task ResetToDefaultsAsync(string? sectionName = null, CancellationToken cancellationToken = default)
    {
        try
        {
            var configuration = await LoadConfigurationAsync(cancellationToken);
            
            if (string.IsNullOrEmpty(sectionName))
            {
                // Reset entire configuration
                configuration = new AppConfiguration();
                _logger.LogInformation("Configuration reset to defaults");
            }
            else
            {
                // Reset specific section
                switch (sectionName.ToLowerInvariant())
                {
                    case "application":
                        configuration.Application = new ApplicationSettings();
                        break;
                    case "user":
                        configuration.User = new UserSettings();
                        break;
                    case "plugins":
                        configuration.Plugins = new PluginSettings();
                        break;
                    case "files":
                        configuration.Files = new FileSettings();
                        break;
                    case "ui":
                        configuration.UI = new UISettings();
                        break;
                    case "performance":
                        configuration.Performance = new PerformanceSettings();
                        break;
                    default:
                        throw new ArgumentException($"Unknown configuration section: {sectionName}");
                }
                
                _logger.LogInformation("Configuration section {SectionName} reset to defaults", sectionName);
            }
            
            await SaveConfigurationAsync(configuration, cancellationToken);
            
            ConfigurationChanged?.Invoke(this, new ConfigurationChangedEventArgs
            {
                SectionName = sectionName ?? "All",
                ChangeType = ConfigurationChangeType.Reset,
                NewValue = configuration
            });
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to reset configuration to defaults");
            throw;
        }
    }
    
    /// <inheritdoc />
    public async Task ExportConfigurationAsync(string filePath, bool includeUserSettings = true, CancellationToken cancellationToken = default)
    {
        try
        {
            var configuration = await LoadConfigurationAsync(cancellationToken);
            
            if (!includeUserSettings)
            {
                // Create a copy without user-specific settings
                configuration = new AppConfiguration
                {
                    Version = configuration.Version,
                    Application = configuration.Application,
                    Plugins = configuration.Plugins,
                    Files = configuration.Files,
                    UI = configuration.UI,
                    Performance = configuration.Performance,
                    User = new UserSettings() // Default user settings
                };
            }
            
            var json = JsonSerializer.Serialize(configuration, _jsonOptions);
            await File.WriteAllTextAsync(filePath, json, cancellationToken);
            
            _logger.LogInformation("Configuration exported to: {FilePath}", filePath);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to export configuration to: {FilePath}", filePath);
            throw;
        }
    }
    
    /// <inheritdoc />
    public async Task<bool> ImportConfigurationAsync(string filePath, bool mergeWithExisting = true, CancellationToken cancellationToken = default)
    {
        try
        {
            if (!File.Exists(filePath))
            {
                _logger.LogError("Import file not found: {FilePath}", filePath);
                return false;
            }
            
            var importContent = await File.ReadAllTextAsync(filePath, cancellationToken);
            var importConfig = JsonSerializer.Deserialize<AppConfiguration>(importContent, _jsonOptions);
            
            if (importConfig == null)
            {
                _logger.LogError("Invalid import file format: {FilePath}", filePath);
                return false;
            }
            
            var validationResult = ValidateConfiguration(importConfig);
            if (!validationResult.IsValid)
            {
                _logger.LogError("Import configuration is invalid: {Errors}", string.Join(", ", validationResult.Errors));
                return false;
            }
            
            AppConfiguration finalConfig;
            
            if (mergeWithExisting)
            {
                var currentConfig = await LoadConfigurationAsync(cancellationToken);
                finalConfig = MergeConfigurations(currentConfig, importConfig);
            }
            else
            {
                finalConfig = importConfig;
            }
            
            await SaveConfigurationAsync(finalConfig, cancellationToken);
            
            _logger.LogInformation("Configuration imported from: {FilePath}", filePath);
            return true;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to import configuration from: {FilePath}", filePath);
            return false;
        }
    }
    
    /// <inheritdoc />
    public ConfigurationPaths GetConfigurationPaths()
    {
        return _paths;
    }
    
    private void ValidateSection<T>(T section, string sectionName, ConfigurationValidationResult result)
    {
        var context = new ValidationContext(section!);
        var validationResults = new List<ValidationResult>();
        
        if (!Validator.TryValidateObject(section!, context, validationResults, true))
        {
            result.IsValid = false;
            result.FailedSection = sectionName;
            result.Errors.AddRange(validationResults.Select(vr => $"{sectionName}: {vr.ErrorMessage}"));
        }
    }
    
    private void ValidateBusinessRules(AppConfiguration configuration, ConfigurationValidationResult result)
    {
        // Example business rule validations
        if (configuration.Performance.MemoryCacheSize > 1024 && !configuration.Performance.EnableMultiThreading)
        {
            result.Warnings.Add("Large memory cache without multi-threading may cause performance issues");
        }
        
        if (configuration.Files.RecentFilesCount > 50)
        {
            result.Warnings.Add("Large recent files count may impact startup performance");
        }
        
        // Validate client directory exists if specified
        if (!string.IsNullOrEmpty(configuration.Application.ClientDirectory) && 
            !Directory.Exists(configuration.Application.ClientDirectory))
        {
            result.Warnings.Add($"Client directory does not exist: {configuration.Application.ClientDirectory}");
        }
    }
    
    private AppConfiguration ApplyDefaultsForInvalidSections(AppConfiguration configuration, ConfigurationValidationResult validationResult)
    {
        if (validationResult.FailedSection != null)
        {
            switch (validationResult.FailedSection.ToLowerInvariant())
            {
                case "application":
                    configuration.Application = new ApplicationSettings();
                    break;
                case "user":
                    configuration.User = new UserSettings();
                    break;
                case "plugins":
                    configuration.Plugins = new PluginSettings();
                    break;
                case "files":
                    configuration.Files = new FileSettings();
                    break;
                case "ui":
                    configuration.UI = new UISettings();
                    break;
                case "performance":
                    configuration.Performance = new PerformanceSettings();
                    break;
            }
        }
        
        return configuration;
    }
    
    private void MergeUserConfiguration(AppConfiguration mainConfig, AppConfiguration userConfig)
    {
        // Merge user-specific settings
        if (userConfig.User != null)
        {
            mainConfig.User = userConfig.User;
        }
        
        // Merge UI settings that are user-specific
        if (userConfig.UI != null)
        {
            mainConfig.UI.ShowThumbnails = userConfig.UI.ShowThumbnails;
            mainConfig.UI.ThumbnailSize = userConfig.UI.ThumbnailSize;
            mainConfig.UI.ShowStatusBar = userConfig.UI.ShowStatusBar;
            mainConfig.UI.ShowToolbar = userConfig.UI.ShowToolbar;
        }
    }
    
    private (AppConfiguration mainConfig, AppConfiguration userConfig) SplitConfiguration(AppConfiguration configuration)
    {
        var mainConfig = new AppConfiguration
        {
            Version = configuration.Version,
            Application = configuration.Application,
            Plugins = configuration.Plugins,
            Files = configuration.Files,
            Performance = configuration.Performance,
            UI = new UISettings
            {
                ShowLineNumbers = configuration.UI.ShowLineNumbers,
                GridView = configuration.UI.GridView
            }
        };
        
        var userConfig = new AppConfiguration
        {
            Version = configuration.Version,
            User = configuration.User,
            UI = new UISettings
            {
                ShowThumbnails = configuration.UI.ShowThumbnails,
                ThumbnailSize = configuration.UI.ThumbnailSize,
                ShowStatusBar = configuration.UI.ShowStatusBar,
                ShowToolbar = configuration.UI.ShowToolbar
            }
        };
        
        return (mainConfig, userConfig);
    }
    
    private AppConfiguration MergeConfigurations(AppConfiguration current, AppConfiguration import)
    {
        // Simple merge strategy - import overwrites current
        // In a more sophisticated implementation, you might want to merge at property level
        return import;
    }
    
    private async Task CleanupOldBackupsAsync(CancellationToken cancellationToken)
    {
        try
        {
            var backupFiles = Directory.GetFiles(_paths.BackupDirectory, "config_backup_*.json")
                .OrderByDescending(f => File.GetCreationTime(f))
                .Skip(10); // Keep last 10 backups
            
            foreach (var file in backupFiles)
            {
                File.Delete(file);
            }
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "Failed to cleanup old backup files");
        }
    }
}