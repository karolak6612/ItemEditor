using Microsoft.Extensions.Logging;
using System.Text.Json;

namespace ItemEditor.Services;

/// <summary>
/// Implementation of settings service for managing application settings
/// </summary>
public class SettingsService : ISettingsService
{
    private readonly ILogger<SettingsService> _logger;
    private readonly Dictionary<string, object> _settings = new();
    private readonly string _settingsFilePath;
    
    /// <summary>
    /// Initializes a new instance of the SettingsService class
    /// </summary>
    /// <param name="logger">Logger instance</param>
    public SettingsService(ILogger<SettingsService> logger)
    {
        _logger = logger;
        _settingsFilePath = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
            "ItemEditor",
            "settings.json");
    }
    
    /// <inheritdoc />
    public T GetSetting<T>(string key, T defaultValue = default!)
    {
        try
        {
            if (_settings.TryGetValue(key, out var value))
            {
                if (value is JsonElement jsonElement)
                {
                    return JsonSerializer.Deserialize<T>(jsonElement.GetRawText()) ?? defaultValue;
                }
                
                if (value is T typedValue)
                {
                    return typedValue;
                }
                
                // Try to convert the value
                return (T)Convert.ChangeType(value, typeof(T)) ?? defaultValue;
            }
            
            return defaultValue;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error getting setting {Key}", key);
            return defaultValue;
        }
    }
    
    /// <inheritdoc />
    public void SetSetting<T>(string key, T value)
    {
        try
        {
            _settings[key] = value!;
            _logger.LogDebug("Setting {Key} updated", key);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error setting {Key}", key);
            throw;
        }
    }
    
    /// <inheritdoc />
    public async Task SaveSettingsAsync(CancellationToken cancellationToken = default)
    {
        try
        {
            _logger.LogDebug("Saving settings to {SettingsFilePath}", _settingsFilePath);
            
            // Ensure directory exists
            var directory = Path.GetDirectoryName(_settingsFilePath);
            if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
            {
                Directory.CreateDirectory(directory);
            }
            
            // Serialize settings to JSON
            var json = JsonSerializer.Serialize(_settings, new JsonSerializerOptions
            {
                WriteIndented = true
            });
            
            await File.WriteAllTextAsync(_settingsFilePath, json, cancellationToken);
            _logger.LogInformation("Settings saved successfully");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error saving settings");
            throw;
        }
    }
    
    /// <inheritdoc />
    public async Task LoadSettingsAsync(CancellationToken cancellationToken = default)
    {
        try
        {
            _logger.LogDebug("Loading settings from {SettingsFilePath}", _settingsFilePath);
            
            if (!File.Exists(_settingsFilePath))
            {
                _logger.LogInformation("Settings file does not exist, using defaults");
                return;
            }
            
            var json = await File.ReadAllTextAsync(_settingsFilePath, cancellationToken);
            var settings = JsonSerializer.Deserialize<Dictionary<string, JsonElement>>(json);
            
            if (settings != null)
            {
                _settings.Clear();
                foreach (var kvp in settings)
                {
                    _settings[kvp.Key] = kvp.Value;
                }
            }
            
            _logger.LogInformation("Settings loaded successfully");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error loading settings");
            // Don't throw - continue with default settings
        }
    }
    
    /// <inheritdoc />
    public bool RemoveSetting(string key)
    {
        try
        {
            var removed = _settings.Remove(key);
            if (removed)
            {
                _logger.LogDebug("Setting {Key} removed", key);
            }
            return removed;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error removing setting {Key}", key);
            return false;
        }
    }
    
    /// <inheritdoc />
    public bool HasSetting(string key)
    {
        return _settings.ContainsKey(key);
    }
}