using Microsoft.Extensions.Logging;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace PluginInterface;

/// <summary>
/// Base plugin implementation with common functionality
/// </summary>
public abstract class BasePlugin : IPlugin
{
    private bool _isLoaded;
    private bool _disposed;
    protected ILogger? _logger;

    /// <summary>
    /// Initializes a new instance of the BasePlugin class
    /// </summary>
    /// <param name="metadata">Plugin metadata</param>
    protected BasePlugin(PluginMetadata metadata)
    {
        Metadata = metadata ?? throw new ArgumentNullException(nameof(metadata));
    }

    /// <inheritdoc />
    public abstract string Name { get; }

    /// <inheritdoc />
    public abstract Version Version { get; }

    /// <inheritdoc />
    public abstract string Description { get; }

    /// <inheritdoc />
    public abstract string Author { get; }

    /// <inheritdoc />
    public PluginMetadata Metadata { get; }

    /// <inheritdoc />
    public bool IsLoaded
    {
        get => _isLoaded;
        protected set => SetProperty(ref _isLoaded, value);
    }

    /// <inheritdoc />
    public virtual bool IsCompatible => true;

    /// <inheritdoc />
    public event PropertyChangedEventHandler? PropertyChanged;

    /// <inheritdoc />
    public virtual async Task InitializeAsync(ILogger? logger = null, CancellationToken cancellationToken = default)
    {
        _logger = logger;
        
        try
        {
            _logger?.LogInformation("Initializing plugin {PluginName} v{Version}", Name, Version);
            
            await OnInitializeAsync(cancellationToken);
            
            IsLoaded = true;
            _logger?.LogInformation("Plugin {PluginName} initialized successfully", Name);
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Failed to initialize plugin {PluginName}", Name);
            throw;
        }
    }

    /// <inheritdoc />
    public virtual async Task ShutdownAsync(CancellationToken cancellationToken = default)
    {
        try
        {
            _logger?.LogInformation("Shutting down plugin {PluginName}", Name);
            
            await OnShutdownAsync(cancellationToken);
            
            IsLoaded = false;
            _logger?.LogInformation("Plugin {PluginName} shut down successfully", Name);
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Error during plugin {PluginName} shutdown", Name);
            throw;
        }
    }

    /// <inheritdoc />
    public virtual Task<PluginValidationResult> ValidateCompatibilityAsync(Version hostVersion, CancellationToken cancellationToken = default)
    {
        var result = new PluginValidationResult { IsValid = true };

        // Check minimum version requirement
        if (Metadata.MinimumHostVersion != null && hostVersion < Metadata.MinimumHostVersion)
        {
            result.IsValid = false;
            result.Errors = result.Errors.Append($"Host version {hostVersion} is below minimum required version {Metadata.MinimumHostVersion}").ToArray();
        }

        // Check maximum version requirement
        if (Metadata.MaximumHostVersion != null && hostVersion > Metadata.MaximumHostVersion)
        {
            result.IsValid = false;
            result.Errors = result.Errors.Append($"Host version {hostVersion} is above maximum supported version {Metadata.MaximumHostVersion}").ToArray();
        }

        // Check supported versions list
        if (Metadata.SupportedHostVersions.Length > 0 && !Metadata.SupportedHostVersions.Contains(hostVersion))
        {
            // Check if any supported version is compatible (same major version)
            var compatibleVersions = Metadata.SupportedHostVersions.Where(v => v.Major == hostVersion.Major);
            if (!compatibleVersions.Any())
            {
                result.Warnings = result.Warnings.Append($"Host version {hostVersion} is not in the list of explicitly supported versions").ToArray();
            }
        }

        result.Message = result.IsValid ? "Plugin is compatible" : "Plugin compatibility check failed";
        
        return Task.FromResult(result);
    }

    /// <inheritdoc />
    public virtual PluginConfigurationSchema? GetConfigurationSchema()
    {
        return null; // Override in derived classes if configuration is needed
    }

    /// <inheritdoc />
    public virtual Task ConfigureAsync(PluginConfiguration configuration, CancellationToken cancellationToken = default)
    {
        return Task.CompletedTask; // Override in derived classes if configuration is needed
    }

    /// <summary>
    /// Called during plugin initialization. Override to provide custom initialization logic.
    /// </summary>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task representing the async operation</returns>
    protected virtual Task OnInitializeAsync(CancellationToken cancellationToken = default)
    {
        return Task.CompletedTask;
    }

    /// <summary>
    /// Called during plugin shutdown. Override to provide custom shutdown logic.
    /// </summary>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task representing the async operation</returns>
    protected virtual Task OnShutdownAsync(CancellationToken cancellationToken = default)
    {
        return Task.CompletedTask;
    }

    /// <summary>
    /// Raises the PropertyChanged event
    /// </summary>
    /// <param name="propertyName">Name of the property that changed</param>
    protected virtual void OnPropertyChanged([CallerMemberName] string? propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }

    /// <summary>
    /// Sets a property value and raises PropertyChanged if the value changed
    /// </summary>
    /// <typeparam name="T">Property type</typeparam>
    /// <param name="field">Field backing the property</param>
    /// <param name="value">New value</param>
    /// <param name="propertyName">Property name</param>
    /// <returns>True if the value changed</returns>
    protected bool SetProperty<T>(ref T field, T value, [CallerMemberName] string? propertyName = null)
    {
        if (EqualityComparer<T>.Default.Equals(field, value))
            return false;

        field = value;
        OnPropertyChanged(propertyName);
        return true;
    }

    /// <summary>
    /// Logs a message using the plugin logger
    /// </summary>
    /// <param name="level">Log level</param>
    /// <param name="message">Message to log</param>
    /// <param name="args">Message arguments</param>
    protected void Log(LogLevel level, string message, params object[] args)
    {
        _logger?.Log(level, message, args);
    }

    /// <summary>
    /// Logs an information message
    /// </summary>
    /// <param name="message">Message to log</param>
    /// <param name="args">Message arguments</param>
    protected void LogInformation(string message, params object[] args)
    {
        _logger?.LogInformation(message, args);
    }

    /// <summary>
    /// Logs a warning message
    /// </summary>
    /// <param name="message">Message to log</param>
    /// <param name="args">Message arguments</param>
    protected void LogWarning(string message, params object[] args)
    {
        _logger?.LogWarning(message, args);
    }

    /// <summary>
    /// Logs an error message
    /// </summary>
    /// <param name="exception">Exception to log</param>
    /// <param name="message">Message to log</param>
    /// <param name="args">Message arguments</param>
    protected void LogError(Exception? exception, string message, params object[] args)
    {
        _logger?.LogError(exception, message, args);
    }

    /// <inheritdoc />
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    /// <summary>
    /// Disposes the plugin resources
    /// </summary>
    /// <param name="disposing">True if disposing managed resources</param>
    protected virtual void Dispose(bool disposing)
    {
        if (!_disposed && disposing)
        {
            if (_isLoaded)
            {
                try
                {
                    ShutdownAsync().GetAwaiter().GetResult();
                }
                catch (Exception ex)
                {
                    _logger?.LogError(ex, "Error during plugin disposal");
                }
            }
            _disposed = true;
        }
    }
}