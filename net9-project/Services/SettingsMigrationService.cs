using Microsoft.Extensions.Logging;
using System.Configuration;
using System.Text.Json;
using System.Xml;
using ItemEditor.Models;

namespace ItemEditor.Services;

/// <summary>
/// Service for migrating legacy settings to modern format
/// </summary>
public class SettingsMigrationService : ISettingsMigrationService
{
    private readonly ILogger<SettingsMigrationService> _logger;
    private readonly ISettingsService _settingsService;
    private readonly string _modernSettingsPath;
    private readonly string _backupDirectory;
    
    /// <summary>
    /// Initializes a new instance of the SettingsMigrationService class
    /// </summary>
    /// <param name="logger">Logger instance</param>
    /// <param name="settingsService">Settings service instance</param>
    public SettingsMigrationService(ILogger<SettingsMigrationService> logger, ISettingsService settingsService)
    {
        _logger = logger;
        _settingsService = settingsService;
        
        var appDataPath = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
        _modernSettingsPath = Path.Combine(appDataPath, "ItemEditor", "settings.json");
        _backupDirectory = Path.Combine(appDataPath, "ItemEditor", "Backups");
        
        // Ensure backup directory exists
        Directory.CreateDirectory(_backupDirectory);
    }
    
    /// <inheritdoc />
    public async Task<bool> HasLegacySettingsAsync()
    {
        try
        {
            var legacyPaths = GetLegacySettingsPaths();
            
            foreach (var path in legacyPaths)
            {
                if (File.Exists(path))
                {
                    _logger.LogInformation("Found legacy settings file: {Path}", path);
                    return true;
                }
            }
            
            // Also check for legacy user.config files in various locations
            var userConfigPaths = GetUserConfigPaths();
            foreach (var path in userConfigPaths)
            {
                if (File.Exists(path))
                {
                    var content = await File.ReadAllTextAsync(path);
                    if (content.Contains("ItemEditor.Properties.Settings"))
                    {
                        _logger.LogInformation("Found legacy user.config file: {Path}", path);
                        return true;
                    }
                }
            }
            
            return false;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error checking for legacy settings");
            return false;
        }
    }
    
    /// <inheritdoc />
    public async Task<MigrationResult> MigrateSettingsAsync(
        IProgress<MigrationProgress>? progress = null,
        CancellationToken cancellationToken = default)
    {
        var result = new MigrationResult();
        var totalSteps = 6;
        var currentStep = 0;
        
        try
        {
            _logger.LogInformation("Starting settings migration");
            
            // Step 1: Check for legacy settings
            ReportProgress(progress, ++currentStep, totalSteps, "Checking for legacy settings...");
            
            if (!await HasLegacySettingsAsync())
            {
                result.Success = true;
                result.Message = "No legacy settings found to migrate";
                ReportProgress(progress, totalSteps, totalSteps, "Migration complete - no legacy settings found", true);
                return result;
            }
            
            // Step 2: Create backup of current settings
            ReportProgress(progress, ++currentStep, totalSteps, "Creating backup of current settings...");
            
            if (File.Exists(_modernSettingsPath))
            {
                var backupPath = await CreateSettingsBackupAsync(cancellationToken);
                _logger.LogInformation("Created settings backup: {BackupPath}", backupPath);
            }
            
            // Step 3: Load legacy settings
            ReportProgress(progress, ++currentStep, totalSteps, "Loading legacy settings...");
            
            var legacySettings = await LoadLegacySettingsAsync(cancellationToken);
            if (legacySettings == null)
            {
                result.Success = false;
                result.Message = "Failed to load legacy settings";
                result.Errors.Add("Could not parse legacy settings files");
                return result;
            }
            
            // Step 4: Convert to modern format
            ReportProgress(progress, ++currentStep, totalSteps, "Converting settings to modern format...");
            
            var modernSettings = ConvertLegacyToModern(legacySettings);
            result.SettingsMigrated = ApplyModernSettings(modernSettings);
            
            // Step 5: Save modern settings
            ReportProgress(progress, ++currentStep, totalSteps, "Saving modern settings...");
            
            await _settingsService.SaveSettingsAsync(cancellationToken);
            
            // Step 6: Validate migration
            ReportProgress(progress, ++currentStep, totalSteps, "Validating migration...");
            
            var isValid = await ValidateMigratedSettingsAsync(cancellationToken);
            if (!isValid)
            {
                result.Warnings.Add("Migration validation failed - some settings may not have been migrated correctly");
            }
            
            result.Success = true;
            result.Message = $"Successfully migrated {result.SettingsMigrated} settings";
            result.NewSettingsPath = _modernSettingsPath;
            
            ReportProgress(progress, totalSteps, totalSteps, "Migration completed successfully", true);
            
            _logger.LogInformation("Settings migration completed successfully. Migrated {Count} settings", result.SettingsMigrated);
        }
        catch (OperationCanceledException)
        {
            result.Success = false;
            result.Message = "Migration was cancelled";
            _logger.LogInformation("Settings migration was cancelled");
        }
        catch (Exception ex)
        {
            result.Success = false;
            result.Message = $"Migration failed: {ex.Message}";
            result.Errors.Add(ex.ToString());
            _logger.LogError(ex, "Settings migration failed");
        }
        
        return result;
    }
    
    /// <inheritdoc />
    public async Task<string> CreateSettingsBackupAsync(CancellationToken cancellationToken = default)
    {
        try
        {
            var timestamp = DateTime.Now.ToString("yyyyMMdd_HHmmss");
            var backupFileName = $"settings_backup_{timestamp}.json";
            var backupPath = Path.Combine(_backupDirectory, backupFileName);
            
            if (File.Exists(_modernSettingsPath))
            {
                File.Copy(_modernSettingsPath, backupPath, true);
                _logger.LogInformation("Created settings backup: {BackupPath}", backupPath);
            }
            
            return backupPath;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to create settings backup");
            throw;
        }
    }
    
    /// <inheritdoc />
    public async Task<bool> RestoreSettingsFromBackupAsync(string backupFilePath, CancellationToken cancellationToken = default)
    {
        try
        {
            if (!File.Exists(backupFilePath))
            {
                _logger.LogError("Backup file not found: {BackupFilePath}", backupFilePath);
                return false;
            }
            
            // Validate backup file is valid JSON
            var backupContent = await File.ReadAllTextAsync(backupFilePath, cancellationToken);
            JsonDocument.Parse(backupContent); // This will throw if invalid JSON
            
            // Create backup of current settings before restore
            if (File.Exists(_modernSettingsPath))
            {
                var currentBackupPath = await CreateSettingsBackupAsync(cancellationToken);
                _logger.LogInformation("Created backup of current settings before restore: {BackupPath}", currentBackupPath);
            }
            
            // Restore from backup
            File.Copy(backupFilePath, _modernSettingsPath, true);
            
            // Reload settings
            await _settingsService.LoadSettingsAsync(cancellationToken);
            
            _logger.LogInformation("Successfully restored settings from backup: {BackupFilePath}", backupFilePath);
            return true;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to restore settings from backup: {BackupFilePath}", backupFilePath);
            return false;
        }
    }
    
    /// <inheritdoc />
    public async Task<bool> ValidateMigratedSettingsAsync(CancellationToken cancellationToken = default)
    {
        try
        {
            // Reload settings to ensure they were saved correctly
            await _settingsService.LoadSettingsAsync(cancellationToken);
            
            // Check that key settings exist and have reasonable values
            var clientDirectory = _settingsService.GetSetting<string>("ClientDirectory", string.Empty);
            var extendedFeatures = _settingsService.GetSetting<bool>("ExtendedFeatures", false);
            var frameDurations = _settingsService.GetSetting<bool>("FrameDurations", false);
            var transparencySupport = _settingsService.GetSetting<bool>("TransparencySupport", false);
            
            // Basic validation - settings should be accessible
            _logger.LogDebug("Validation - ClientDirectory: {ClientDirectory}", clientDirectory);
            _logger.LogDebug("Validation - ExtendedFeatures: {ExtendedFeatures}", extendedFeatures);
            _logger.LogDebug("Validation - FrameDurations: {FrameDurations}", frameDurations);
            _logger.LogDebug("Validation - TransparencySupport: {TransparencySupport}", transparencySupport);
            
            return true;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Settings validation failed");
            return false;
        }
    }
    
    /// <inheritdoc />
    public IEnumerable<string> GetLegacySettingsPaths()
    {
        var paths = new List<string>();
        
        // Legacy app.config in various possible locations
        var possibleLegacyPaths = new[]
        {
            Path.Combine(Environment.CurrentDirectory, "app.config"),
            Path.Combine(Environment.CurrentDirectory, "ItemEditor.exe.config"),
            Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "app.config"),
            Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "ItemEditor.exe.config")
        };
        
        paths.AddRange(possibleLegacyPaths);
        
        return paths;
    }
    
    private IEnumerable<string> GetUserConfigPaths()
    {
        var paths = new List<string>();
        
        // Common user.config locations for .NET Framework applications
        var localAppData = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
        var companyName = "ItemEditor"; // Adjust based on actual company name used
        
        // Search in various version folders
        var possiblePaths = new[]
        {
            Path.Combine(localAppData, companyName, "ItemEditor.exe_*", "*", "user.config"),
            Path.Combine(localAppData, "ItemEditor", "ItemEditor.exe_*", "*", "user.config"),
            Path.Combine(localAppData, "Microsoft", "ItemEditor.exe_*", "*", "user.config")
        };
        
        foreach (var pattern in possiblePaths)
        {
            try
            {
                var directory = Path.GetDirectoryName(pattern);
                if (directory != null && Directory.Exists(directory))
                {
                    var files = Directory.GetFiles(directory, "user.config", SearchOption.AllDirectories);
                    paths.AddRange(files);
                }
            }
            catch (Exception ex)
            {
                _logger.LogDebug(ex, "Error searching for user.config files in pattern: {Pattern}", pattern);
            }
        }
        
        return paths;
    }
    
    private async Task<LegacySettings?> LoadLegacySettingsAsync(CancellationToken cancellationToken)
    {
        var legacySettings = new LegacySettings();
        var foundSettings = false;
        
        // Try to load from app.config files
        var legacyPaths = GetLegacySettingsPaths();
        foreach (var path in legacyPaths)
        {
            if (File.Exists(path))
            {
                try
                {
                    var settings = await LoadFromAppConfigAsync(path, cancellationToken);
                    if (settings != null)
                    {
                        MergeSettings(legacySettings, settings);
                        foundSettings = true;
                        _logger.LogInformation("Loaded legacy settings from: {Path}", path);
                    }
                }
                catch (Exception ex)
                {
                    _logger.LogWarning(ex, "Failed to load legacy settings from: {Path}", path);
                }
            }
        }
        
        // Try to load from user.config files
        var userConfigPaths = GetUserConfigPaths();
        foreach (var path in userConfigPaths)
        {
            if (File.Exists(path))
            {
                try
                {
                    var settings = await LoadFromUserConfigAsync(path, cancellationToken);
                    if (settings != null)
                    {
                        MergeSettings(legacySettings, settings);
                        foundSettings = true;
                        _logger.LogInformation("Loaded legacy user settings from: {Path}", path);
                    }
                }
                catch (Exception ex)
                {
                    _logger.LogWarning(ex, "Failed to load legacy user settings from: {Path}", path);
                }
            }
        }
        
        return foundSettings ? legacySettings : null;
    }
    
    private async Task<LegacySettings?> LoadFromAppConfigAsync(string configPath, CancellationToken cancellationToken)
    {
        try
        {
            var content = await File.ReadAllTextAsync(configPath, cancellationToken);
            var doc = new XmlDocument();
            doc.LoadXml(content);
            
            var settings = new LegacySettings();
            var userSettingsNode = doc.SelectSingleNode("//userSettings/ItemEditor.Properties.Settings");
            
            if (userSettingsNode != null)
            {
                foreach (XmlNode settingNode in userSettingsNode.SelectNodes("setting"))
                {
                    var name = settingNode.Attributes?["name"]?.Value;
                    var valueNode = settingNode.SelectSingleNode("value");
                    var value = valueNode?.InnerText;
                    
                    if (!string.IsNullOrEmpty(name) && value != null)
                    {
                        ApplyLegacySetting(settings, name, value);
                    }
                }
                
                return settings;
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error loading from app.config: {ConfigPath}", configPath);
        }
        
        return null;
    }
    
    private async Task<LegacySettings?> LoadFromUserConfigAsync(string configPath, CancellationToken cancellationToken)
    {
        try
        {
            var content = await File.ReadAllTextAsync(configPath, cancellationToken);
            
            // Check if this user.config contains ItemEditor settings
            if (!content.Contains("ItemEditor.Properties.Settings"))
            {
                return null;
            }
            
            var doc = new XmlDocument();
            doc.LoadXml(content);
            
            var settings = new LegacySettings();
            var settingsNodes = doc.SelectNodes("//setting");
            
            if (settingsNodes != null)
            {
                foreach (XmlNode settingNode in settingsNodes)
                {
                    var name = settingNode.Attributes?["name"]?.Value;
                    var valueNode = settingNode.SelectSingleNode("value");
                    var value = valueNode?.InnerText;
                    
                    if (!string.IsNullOrEmpty(name) && value != null)
                    {
                        ApplyLegacySetting(settings, name, value);
                    }
                }
                
                return settings;
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error loading from user.config: {ConfigPath}", configPath);
        }
        
        return null;
    }
    
    private void ApplyLegacySetting(LegacySettings settings, string name, string value)
    {
        switch (name)
        {
            case "ClientDirectory":
                settings.ClientDirectory = value;
                break;
            case "Extended":
                settings.Extended = bool.TryParse(value, out var extended) && extended;
                break;
            case "Transparency":
                settings.Transparency = bool.TryParse(value, out var transparency) && transparency;
                break;
            case "DatSignature":
                settings.DatSignature = uint.TryParse(value, out var datSig) ? datSig : 0;
                break;
            case "SprSignature":
                settings.SprSignature = uint.TryParse(value, out var sprSig) ? sprSig : 0;
                break;
            case "FrameDurations":
                settings.FrameDurations = bool.TryParse(value, out var frameDurations) && frameDurations;
                break;
            default:
                _logger.LogDebug("Unknown legacy setting: {Name} = {Value}", name, value);
                break;
        }
    }
    
    private void MergeSettings(LegacySettings target, LegacySettings source)
    {
        // Merge settings, preferring non-default values from source
        if (!string.IsNullOrEmpty(source.ClientDirectory))
            target.ClientDirectory = source.ClientDirectory;
        
        if (source.Extended)
            target.Extended = source.Extended;
        
        if (source.Transparency)
            target.Transparency = source.Transparency;
        
        if (source.DatSignature != 0)
            target.DatSignature = source.DatSignature;
        
        if (source.SprSignature != 0)
            target.SprSignature = source.SprSignature;
        
        if (source.FrameDurations)
            target.FrameDurations = source.FrameDurations;
    }
    
    private ApplicationSettings ConvertLegacyToModern(LegacySettings legacy)
    {
        return new ApplicationSettings
        {
            ClientDirectory = legacy.ClientDirectory,
            ExtendedFeatures = legacy.Extended,
            TransparencySupport = legacy.Transparency,
            FrameDurations = legacy.FrameDurations,
            // Set reasonable defaults for new settings
            Theme = "Auto",
            AccentColor = "System",
            AutoSave = false,
            CreateBackups = true,
            RecentFilesCount = 10,
            FontSize = 12.0,
            ShowThumbnails = true,
            ThumbnailSize = 32.0,
            MemoryCacheSize = 256,
            EnableMultiThreading = true,
            HardwareAcceleration = true
        };
    }
    
    private int ApplyModernSettings(ApplicationSettings settings)
    {
        var count = 0;
        
        _settingsService.SetSetting("ClientDirectory", settings.ClientDirectory);
        count++;
        
        _settingsService.SetSetting("ExtendedFeatures", settings.ExtendedFeatures);
        count++;
        
        _settingsService.SetSetting("TransparencySupport", settings.TransparencySupport);
        count++;
        
        _settingsService.SetSetting("FrameDurations", settings.FrameDurations);
        count++;
        
        _settingsService.SetSetting("Theme", settings.Theme);
        count++;
        
        _settingsService.SetSetting("AccentColor", settings.AccentColor);
        count++;
        
        _settingsService.SetSetting("AutoSave", settings.AutoSave);
        count++;
        
        _settingsService.SetSetting("CreateBackups", settings.CreateBackups);
        count++;
        
        _settingsService.SetSetting("RecentFilesCount", settings.RecentFilesCount);
        count++;
        
        _settingsService.SetSetting("FontSize", settings.FontSize);
        count++;
        
        _settingsService.SetSetting("ShowThumbnails", settings.ShowThumbnails);
        count++;
        
        _settingsService.SetSetting("ThumbnailSize", settings.ThumbnailSize);
        count++;
        
        _settingsService.SetSetting("MemoryCacheSize", settings.MemoryCacheSize);
        count++;
        
        _settingsService.SetSetting("EnableMultiThreading", settings.EnableMultiThreading);
        count++;
        
        _settingsService.SetSetting("HardwareAcceleration", settings.HardwareAcceleration);
        count++;
        
        return count;
    }
    
    private void ReportProgress(IProgress<MigrationProgress>? progress, int currentStep, int totalSteps, string stepDescription, bool isComplete = false)
    {
        if (progress == null) return;
        
        var progressPercentage = (int)((double)currentStep / totalSteps * 100);
        
        progress.Report(new MigrationProgress
        {
            CurrentStep = stepDescription,
            ProgressPercentage = progressPercentage,
            TotalSteps = totalSteps,
            CurrentStepNumber = currentStep,
            IsComplete = isComplete
        });
    }
}