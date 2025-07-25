namespace ItemEditor.Services;

/// <summary>
/// Interface for settings management service
/// </summary>
public interface ISettingsService
{
    /// <summary>
    /// Loads settings from storage
    /// </summary>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task representing the asynchronous operation</returns>
    Task LoadSettingsAsync(CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Saves settings to storage
    /// </summary>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task representing the asynchronous operation</returns>
    Task SaveSettingsAsync(CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Gets a setting value
    /// </summary>
    /// <typeparam name="T">Type of the setting value</typeparam>
    /// <param name="key">Setting key</param>
    /// <param name="defaultValue">Default value if setting doesn't exist</param>
    /// <returns>Setting value</returns>
    T GetSetting<T>(string key, T defaultValue = default!);
    
    /// <summary>
    /// Sets a setting value
    /// </summary>
    /// <typeparam name="T">Type of the setting value</typeparam>
    /// <param name="key">Setting key</param>
    /// <param name="value">Setting value</param>
    void SetSetting<T>(string key, T value);
    
    /// <summary>
    /// Removes a setting
    /// </summary>
    /// <param name="key">Setting key</param>
    /// <returns>True if the setting was removed</returns>
    bool RemoveSetting(string key);
    
    /// <summary>
    /// Checks if a setting exists
    /// </summary>
    /// <param name="key">Setting key</param>
    /// <returns>True if the setting exists</returns>
    bool HasSetting(string key);
}