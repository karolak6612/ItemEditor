using Microsoft.Extensions.Logging;
using System.ComponentModel;

namespace PluginInterface;

/// <summary>
/// Modern plugin interface with enhanced async support and cancellation
/// </summary>
public interface IPlugin : INotifyPropertyChanged, IDisposable
{
    /// <summary>
    /// Gets the plugin name
    /// </summary>
    string Name { get; }
    
    /// <summary>
    /// Gets the plugin version with semantic versioning support
    /// </summary>
    Version Version { get; }
    
    /// <summary>
    /// Gets the plugin description
    /// </summary>
    string Description { get; }
    
    /// <summary>
    /// Gets the plugin author
    /// </summary>
    string Author { get; }
    
    /// <summary>
    /// Gets the plugin metadata
    /// </summary>
    PluginMetadata Metadata { get; }
    
    /// <summary>
    /// Gets whether the plugin is currently loaded and active
    /// </summary>
    bool IsLoaded { get; }
    
    /// <summary>
    /// Gets whether the plugin supports the current application version
    /// </summary>
    bool IsCompatible { get; }
    
    /// <summary>
    /// Initialize the plugin with cancellation support
    /// </summary>
    /// <param name="logger">Logger instance for plugin logging</param>
    /// <param name="cancellationToken">Cancellation token for responsive cancellation</param>
    /// <returns>Task representing the async operation</returns>
    Task InitializeAsync(ILogger? logger = null, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Shutdown the plugin gracefully with cancellation support
    /// </summary>
    /// <param name="cancellationToken">Cancellation token for responsive cancellation</param>
    /// <returns>Task representing the async operation</returns>
    Task ShutdownAsync(CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Validate plugin compatibility with the current environment
    /// </summary>
    /// <param name="hostVersion">Host application version</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task with validation result</returns>
    Task<PluginValidationResult> ValidateCompatibilityAsync(Version hostVersion, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Get plugin configuration schema for settings UI
    /// </summary>
    /// <returns>Configuration schema or null if no configuration needed</returns>
    PluginConfigurationSchema? GetConfigurationSchema();
    
    /// <summary>
    /// Configure plugin with provided settings
    /// </summary>
    /// <param name="configuration">Plugin configuration</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task representing the async operation</returns>
    Task ConfigureAsync(PluginConfiguration configuration, CancellationToken cancellationToken = default);
}

/// <summary>
/// Legacy plugin interface for backward compatibility
/// </summary>
public interface ILegacyPlugin
{
    string Name { get; }
    Version Version { get; }
    string Description { get; }
    string Author { get; }
    
    void Initialize();
    void Shutdown();
}

/// <summary>
/// Plugin adapter for legacy plugin compatibility
/// </summary>
public class LegacyPluginAdapter : IPlugin
{
    private readonly ILegacyPlugin _legacyPlugin;
    private bool _isLoaded;
    private bool _disposed;

    public LegacyPluginAdapter(ILegacyPlugin legacyPlugin)
    {
        _legacyPlugin = legacyPlugin ?? throw new ArgumentNullException(nameof(legacyPlugin));
        
        Metadata = new PluginMetadata
        {
            Name = _legacyPlugin.Name,
            Version = _legacyPlugin.Version,
            Description = _legacyPlugin.Description,
            Author = _legacyPlugin.Author,
            IsLegacy = true,
            SupportedHostVersions = new[] { new Version(1, 0, 0) } // Legacy compatibility
        };
    }

    public string Name => _legacyPlugin.Name;
    public Version Version => _legacyPlugin.Version;
    public string Description => _legacyPlugin.Description;
    public string Author => _legacyPlugin.Author;
    public PluginMetadata Metadata { get; }
    public bool IsLoaded => _isLoaded;
    public bool IsCompatible => true; // Legacy plugins are always considered compatible

    public event PropertyChangedEventHandler? PropertyChanged;

    public async Task InitializeAsync(ILogger? logger = null, CancellationToken cancellationToken = default)
    {
        await Task.Run(() =>
        {
            cancellationToken.ThrowIfCancellationRequested();
            _legacyPlugin.Initialize();
            _isLoaded = true;
            OnPropertyChanged(nameof(IsLoaded));
        }, cancellationToken);
    }

    public async Task ShutdownAsync(CancellationToken cancellationToken = default)
    {
        await Task.Run(() =>
        {
            cancellationToken.ThrowIfCancellationRequested();
            _legacyPlugin.Shutdown();
            _isLoaded = false;
            OnPropertyChanged(nameof(IsLoaded));
        }, cancellationToken);
    }

    public Task<PluginValidationResult> ValidateCompatibilityAsync(Version hostVersion, CancellationToken cancellationToken = default)
    {
        return Task.FromResult(new PluginValidationResult
        {
            IsValid = true,
            Message = "Legacy plugin - compatibility assumed"
        });
    }

    public PluginConfigurationSchema? GetConfigurationSchema() => null;

    public Task ConfigureAsync(PluginConfiguration configuration, CancellationToken cancellationToken = default)
    {
        return Task.CompletedTask; // Legacy plugins don't support configuration
    }

    protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string? propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }

    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    protected virtual void Dispose(bool disposing)
    {
        if (!_disposed && disposing)
        {
            if (_isLoaded)
            {
                _legacyPlugin.Shutdown();
            }
            _disposed = true;
        }
    }
}