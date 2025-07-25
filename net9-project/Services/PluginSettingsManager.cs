using Microsoft.Extensions.Logging;
using ItemEditor.Models;
using System.Text.Json;
using System.Collections.Concurrent;

namespace ItemEditor.Services;

/// <summary>
/// Manager for plugin settings persistence
/// </summary>
public class PluginSettingsManager : IPluginSettingsManager, IDisposable
{
    private readonly string _pluginName;
    private readonly ILogger _logger;
    private readonly string _settingsDirectory;
    private readonly string _settingsFilePath;
    private readonly ConcurrentDictionary<string, object?> _settings = new();
    private readonly object _lock = new();
    private bool _disposed;
    private bool _isDirty;

    /// <summary>
    /// Initializes a new instance of the PluginSettingsManager class
    /// </summary>
    /// <param name="pluginName">Plugin name</param>
    /// <param name="logger">Logger instance</param>
    public PluginSettingsManager(string pluginName, ILogger logger)
    {
        _pluginName = pluginName ?? throw new ArgumentNullException(nameof(pluginName));
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));

        // Create settings directory path
        _settingsDirectory = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
            "ItemEditor",
            "Plugins",
            _pluginName);

        _settingsFilePath = Path.Combine(_settingsDirectory, "settings.json");

        // Load existing settings
        LoadSettings();
    }

    /// <summary>
    /// Gets a setting value
    /// </summary>
    /// <typeparam name="T">Setting value type</typeparam>
    /// <param name="key">Setting key</param>
    /// <param name="defaultValue">Default value if key not found</param>
    /// <returns>Setting value</returns>
    public T? GetSetting<T>(string key, T? defaultValue = default)
    {
        if (string.IsNullOrEmpty(key))
            throw new ArgumentException("Key cannot be null or empty", nameof(key));

        try
        {
            if (_settings.TryGetValue(key, out var value))
            {
                if (value is JsonElement jsonElement)
                {
                    return JsonSerializer.Deserialize<T>(jsonElement.GetRawText());
                }
                else if (value is T typedValue)
                {
                    return typedValue;
                }
                else if (value != null)
                {
                    // Try to convert the value
                    return (T?)Convert.ChangeType(value, typeof(T));
                }
            }

            return defaultValue;
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "Failed to get setting {Key} for plugin {PluginName}, returning default value", 
                key, _pluginName);
            return defaultValue;
        }
    }

    /// <summary>
    /// Sets a setting value
    /// </summary>
    /// <param name="key">Setting key</param>
    /// <param name="value">Setting value</param>
    public void SetSetting(string key, object? value)
    {
        if (string.IsNullOrEmpty(key))
            throw new ArgumentException("Key cannot be null or empty", nameof(key));

        lock (_lock)
        {
            var oldValue = _settings.TryGetValue(key, out var existing) ? existing : null;
            
            if (!Equals(oldValue, value))
            {
                _settings[key] = value;
                _isDirty = true;
                
                _logger.LogDebug("Setting {Key} updated for plugin {PluginName}", key, _pluginName);
                
                // Raise setting changed event
                SettingChanged?.Invoke(this, new SettingChangedEventArgs(key, oldValue, value));
            }
        }
    }

    /// <summary>
    /// Removes a setting
    /// </summary>
    /// <param name="key">Setting key</param>
    /// <returns>True if setting was removed</returns>
    public bool RemoveSetting(string key)
    {
        if (string.IsNullOrEmpty(key))
            return false;

        lock (_lock)
        {
            if (_settings.TryRemove(key, out var oldValue))
            {
                _isDirty = true;
                _logger.LogDebug("Setting {Key} removed for plugin {PluginName}", key, _pluginName);
                
                // Raise setting changed event
                SettingChanged?.Invoke(this, new SettingChangedEventArgs(key, oldValue, null));
                return true;
            }
        }

        return false;
    }

    /// <summary>
    /// Checks if a setting exists
    /// </summary>
    /// <param name="key">Setting key</param>
    /// <returns>True if setting exists</returns>
    public bool HasSetting(string key)
    {
        if (string.IsNullOrEmpty(key))
            return false;

        return _settings.ContainsKey(key);
    }

    /// <summary>
    /// Gets all setting keys
    /// </summary>
    /// <returns>Collection of setting keys</returns>
    public IEnumerable<string> GetAllKeys()
    {
        return _settings.Keys.ToList();
    }

    /// <summary>
    /// Gets all settings as a dictionary
    /// </summary>
    /// <returns>Dictionary of all settings</returns>
    public IReadOnlyDictionary<string, object?> GetAllSettings()
    {
        return _settings.ToDictionary(kvp => kvp.Key, kvp => kvp.Value);
    }

    /// <summary>
    /// Clears all settings
    /// </summary>
    public void ClearSettings()
    {
        lock (_lock)
        {
            if (_settings.Any())
            {
                _settings.Clear();
                _isDirty = true;
                _logger.LogInformation("All settings cleared for plugin {PluginName}", _pluginName);
                
                // Raise settings cleared event
                SettingsCleared?.Invoke(this, EventArgs.Empty);
            }
        }
    }

    /// <summary>
    /// Saves settings to disk
    /// </summary>
    /// <returns>Task representing the async operation</returns>
    public async Task SaveSettingsAsync()
    {
        if (!_isDirty)
            return;

        try
        {
            _logger.LogDebug("Saving settings for plugin {PluginName}", _pluginName);

            // Ensure directory exists
            Directory.CreateDirectory(_settingsDirectory);

            // Convert settings to serializable format
            var settingsToSave = _settings.ToDictionary(kvp => kvp.Key, kvp => kvp.Value);

            // Serialize to JSON
            var json = JsonSerializer.Serialize(settingsToSave, new JsonSerializerOptions
            {
                WriteIndented = true,
                PropertyNamingPolicy = JsonNamingPolicy.CamelCase
            });

            // Write to file
            await File.WriteAllTextAsync(_settingsFilePath, json);

            lock (_lock)
            {
                _isDirty = false;
            }

            _logger.LogInformation("Settings saved for plugin {PluginName}", _pluginName);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to save settings for plugin {PluginName}", _pluginName);
            throw;
        }
    }

    /// <summary>
    /// Loads settings from disk
    /// </summary>
    /// <returns>Task representing the async operation</returns>
    public async Task LoadSettingsAsync()
    {
        try
        {
            if (!File.Exists(_settingsFilePath))
            {
                _logger.LogDebug("No settings file found for plugin {PluginName}", _pluginName);
                return;
            }

            _logger.LogDebug("Loading settings for plugin {PluginName}", _pluginName);

            var json = await File.ReadAllTextAsync(_settingsFilePath);
            
            if (string.IsNullOrWhiteSpace(json))
                return;

            var loadedSettings = JsonSerializer.Deserialize<Dictionary<string, JsonElement>>(json);
            
            if (loadedSettings != null)
            {
                lock (_lock)
                {
                    _settings.Clear();
                    
                    foreach (var kvp in loadedSettings)
                    {
                        _settings[kvp.Key] = kvp.Value;
                    }
                    
                    _isDirty = false;
                }

                _logger.LogInformation("Settings loaded for plugin {PluginName} ({Count} settings)", 
                    _pluginName, loadedSettings.Count);
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to load settings for plugin {PluginName}", _pluginName);
            // Don't throw - continue with empty settings
        }
    }

    /// <summary>
    /// Resets settings to default values
    /// </summary>
    /// <param name="defaultSettings">Default settings</param>
    public void ResetToDefaults(IReadOnlyDictionary<string, object?>? defaultSettings = null)
    {
        lock (_lock)
        {
            _settings.Clear();
            
            if (defaultSettings != null)
            {
                foreach (var kvp in defaultSettings)
                {
                    _settings[kvp.Key] = kvp.Value;
                }
            }
            
            _isDirty = true;
            _logger.LogInformation("Settings reset to defaults for plugin {PluginName}", _pluginName);
            
            // Raise settings reset event
            SettingsReset?.Invoke(this, EventArgs.Empty);
        }
    }

    /// <summary>
    /// Event raised when a setting changes
    /// </summary>
    public event EventHandler<SettingChangedEventArgs>? SettingChanged;

    /// <summary>
    /// Event raised when all settings are cleared
    /// </summary>
    public event EventHandler? SettingsCleared;

    /// <summary>
    /// Event raised when settings are reset
    /// </summary>
    public event EventHandler? SettingsReset;

    /// <summary>
    /// Loads settings synchronously
    /// </summary>
    private void LoadSettings()
    {
        try
        {
            LoadSettingsAsync().Wait();
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to load settings synchronously for plugin {PluginName}", _pluginName);
        }
    }

    /// <summary>
    /// Disposes the settings manager
    /// </summary>
    public void Dispose()
    {
        if (_disposed)
            return;

        try
        {
            // Save any pending changes
            if (_isDirty)
            {
                SaveSettingsAsync().Wait(TimeSpan.FromSeconds(5));
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error saving settings during disposal for plugin {PluginName}", _pluginName);
        }

        _disposed = true;
    }
}

/// <summary>
/// Interface for plugin settings manager
/// </summary>
public interface IPluginSettingsManager
{
    /// <summary>
    /// Gets a setting value
    /// </summary>
    /// <typeparam name="T">Setting value type</typeparam>
    /// <param name="key">Setting key</param>
    /// <param name="defaultValue">Default value if key not found</param>
    /// <returns>Setting value</returns>
    T? GetSetting<T>(string key, T? defaultValue = default);

    /// <summary>
    /// Sets a setting value
    /// </summary>
    /// <param name="key">Setting key</param>
    /// <param name="value">Setting value</param>
    void SetSetting(string key, object? value);

    /// <summary>
    /// Removes a setting
    /// </summary>
    /// <param name="key">Setting key</param>
    /// <returns>True if setting was removed</returns>
    bool RemoveSetting(string key);

    /// <summary>
    /// Checks if a setting exists
    /// </summary>
    /// <param name="key">Setting key</param>
    /// <returns>True if setting exists</returns>
    bool HasSetting(string key);

    /// <summary>
    /// Gets all setting keys
    /// </summary>
    /// <returns>Collection of setting keys</returns>
    IEnumerable<string> GetAllKeys();

    /// <summary>
    /// Gets all settings as a dictionary
    /// </summary>
    /// <returns>Dictionary of all settings</returns>
    IReadOnlyDictionary<string, object?> GetAllSettings();

    /// <summary>
    /// Clears all settings
    /// </summary>
    void ClearSettings();

    /// <summary>
    /// Saves settings to disk
    /// </summary>
    /// <returns>Task representing the async operation</returns>
    Task SaveSettingsAsync();

    /// <summary>
    /// Loads settings from disk
    /// </summary>
    /// <returns>Task representing the async operation</returns>
    Task LoadSettingsAsync();

    /// <summary>
    /// Resets settings to default values
    /// </summary>
    /// <param name="defaultSettings">Default settings</param>
    void ResetToDefaults(IReadOnlyDictionary<string, object?>? defaultSettings = null);

    /// <summary>
    /// Event raised when a setting changes
    /// </summary>
    event EventHandler<SettingChangedEventArgs>? SettingChanged;

    /// <summary>
    /// Event raised when all settings are cleared
    /// </summary>
    event EventHandler? SettingsCleared;

    /// <summary>
    /// Event raised when settings are reset
    /// </summary>
    event EventHandler? SettingsReset;
}

/// <summary>
/// Event arguments for setting changed events
/// </summary>
public class SettingChangedEventArgs : EventArgs
{
    /// <summary>
    /// Initializes a new instance of the SettingChangedEventArgs class
    /// </summary>
    /// <param name="key">Setting key</param>
    /// <param name="oldValue">Old value</param>
    /// <param name="newValue">New value</param>
    public SettingChangedEventArgs(string key, object? oldValue, object? newValue)
    {
        Key = key ?? throw new ArgumentNullException(nameof(key));
        OldValue = oldValue;
        NewValue = newValue;
    }

    /// <summary>
    /// Gets the setting key
    /// </summary>
    public string Key { get; }

    /// <summary>
    /// Gets the old value
    /// </summary>
    public object? OldValue { get; }

    /// <summary>
    /// Gets the new value
    /// </summary>
    public object? NewValue { get; }
}