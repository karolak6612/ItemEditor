using PluginInterface;

namespace ItemEditor.Models;

/// <summary>
/// Base class for plugin event arguments
/// </summary>
public abstract class PluginEventArgsBase : EventArgs
{
    /// <summary>
    /// Initializes a new instance of the PluginEventArgsBase class
    /// </summary>
    /// <param name="plugin">The plugin</param>
    protected PluginEventArgsBase(IPlugin plugin)
    {
        Plugin = plugin ?? throw new ArgumentNullException(nameof(plugin));
        Timestamp = DateTime.UtcNow;
    }

    /// <summary>
    /// Gets the plugin
    /// </summary>
    public IPlugin Plugin { get; }

    /// <summary>
    /// Gets the event timestamp
    /// </summary>
    public DateTime Timestamp { get; }
}

/// <summary>
/// Event arguments for plugin loading events
/// </summary>
public class PluginLoadingEventArgs : PluginEventArgsBase
{
    /// <summary>
    /// Initializes a new instance of the PluginLoadingEventArgs class
    /// </summary>
    /// <param name="plugin">The plugin being loaded</param>
    /// <param name="filePath">Path to the plugin file</param>
    public PluginLoadingEventArgs(IPlugin plugin, string filePath) : base(plugin)
    {
        FilePath = filePath ?? throw new ArgumentNullException(nameof(filePath));
    }

    /// <summary>
    /// Gets the plugin file path
    /// </summary>
    public string FilePath { get; }

    /// <summary>
    /// Gets or sets whether the loading should be cancelled
    /// </summary>
    public bool Cancel { get; set; }

    /// <summary>
    /// Gets or sets the cancellation reason
    /// </summary>
    public string? CancellationReason { get; set; }
}

/// <summary>
/// Event arguments for plugin loaded events
/// </summary>
public class PluginLoadedEventArgs : PluginEventArgsBase
{
    /// <summary>
    /// Initializes a new instance of the PluginLoadedEventArgs class
    /// </summary>
    /// <param name="plugin">The loaded plugin</param>
    /// <param name="loadTime">Time taken to load the plugin</param>
    public PluginLoadedEventArgs(IPlugin plugin, TimeSpan loadTime) : base(plugin)
    {
        LoadTime = loadTime;
    }

    /// <summary>
    /// Gets the time taken to load the plugin
    /// </summary>
    public TimeSpan LoadTime { get; }

    /// <summary>
    /// Gets or sets whether the plugin was loaded successfully
    /// </summary>
    public bool Success { get; set; } = true;

    /// <summary>
    /// Gets or sets any warnings that occurred during loading
    /// </summary>
    public List<string> Warnings { get; set; } = new();
}

/// <summary>
/// Event arguments for plugin unloading events
/// </summary>
public class PluginUnloadingEventArgs : PluginEventArgsBase
{
    /// <summary>
    /// Initializes a new instance of the PluginUnloadingEventArgs class
    /// </summary>
    /// <param name="plugin">The plugin being unloaded</param>
    /// <param name="reason">Reason for unloading</param>
    public PluginUnloadingEventArgs(IPlugin plugin, PluginUnloadReason reason) : base(plugin)
    {
        Reason = reason;
    }

    /// <summary>
    /// Gets the reason for unloading
    /// </summary>
    public PluginUnloadReason Reason { get; }

    /// <summary>
    /// Gets or sets whether the unloading should be cancelled
    /// </summary>
    public bool Cancel { get; set; }

    /// <summary>
    /// Gets or sets the cancellation reason
    /// </summary>
    public string? CancellationReason { get; set; }
}

/// <summary>
/// Event arguments for plugin unloaded events
/// </summary>
public class PluginUnloadedEventArgs : PluginEventArgsBase
{
    /// <summary>
    /// Initializes a new instance of the PluginUnloadedEventArgs class
    /// </summary>
    /// <param name="plugin">The unloaded plugin</param>
    /// <param name="reason">Reason for unloading</param>
    /// <param name="unloadTime">Time taken to unload the plugin</param>
    public PluginUnloadedEventArgs(IPlugin plugin, PluginUnloadReason reason, TimeSpan unloadTime) : base(plugin)
    {
        Reason = reason;
        UnloadTime = unloadTime;
    }

    /// <summary>
    /// Gets the reason for unloading
    /// </summary>
    public PluginUnloadReason Reason { get; }

    /// <summary>
    /// Gets the time taken to unload the plugin
    /// </summary>
    public TimeSpan UnloadTime { get; }

    /// <summary>
    /// Gets or sets whether the plugin was unloaded successfully
    /// </summary>
    public bool Success { get; set; } = true;

    /// <summary>
    /// Gets or sets any errors that occurred during unloading
    /// </summary>
    public List<string> Errors { get; set; } = new();
}

/// <summary>
/// Event arguments for plugin error events
/// </summary>
public class PluginErrorEventArgs : PluginEventArgsBase
{
    /// <summary>
    /// Initializes a new instance of the PluginErrorEventArgs class
    /// </summary>
    /// <param name="plugin">The plugin that caused the error</param>
    /// <param name="exception">The exception that occurred</param>
    /// <param name="errorType">Type of error</param>
    public PluginErrorEventArgs(IPlugin plugin, Exception exception, PluginErrorType errorType) : base(plugin)
    {
        Exception = exception ?? throw new ArgumentNullException(nameof(exception));
        ErrorType = errorType;
    }

    /// <summary>
    /// Gets the exception that occurred
    /// </summary>
    public Exception Exception { get; }

    /// <summary>
    /// Gets the type of error
    /// </summary>
    public PluginErrorType ErrorType { get; }

    /// <summary>
    /// Gets or sets whether the error was handled
    /// </summary>
    public bool Handled { get; set; }

    /// <summary>
    /// Gets or sets the recovery action taken
    /// </summary>
    public PluginErrorRecoveryAction RecoveryAction { get; set; } = PluginErrorRecoveryAction.None;
}

/// <summary>
/// Types of plugin errors
/// </summary>
public enum PluginErrorType
{
    /// <summary>
    /// Error during plugin loading
    /// </summary>
    LoadError,

    /// <summary>
    /// Error during plugin initialization
    /// </summary>
    InitializationError,

    /// <summary>
    /// Error during plugin execution
    /// </summary>
    ExecutionError,

    /// <summary>
    /// Error during plugin unloading
    /// </summary>
    UnloadError,

    /// <summary>
    /// Configuration error
    /// </summary>
    ConfigurationError,

    /// <summary>
    /// Dependency error
    /// </summary>
    DependencyError,

    /// <summary>
    /// Security error
    /// </summary>
    SecurityError
}

/// <summary>
/// Plugin error recovery actions
/// </summary>
public enum PluginErrorRecoveryAction
{
    /// <summary>
    /// No recovery action taken
    /// </summary>
    None,

    /// <summary>
    /// Plugin was disabled
    /// </summary>
    Disabled,

    /// <summary>
    /// Plugin was unloaded
    /// </summary>
    Unloaded,

    /// <summary>
    /// Plugin was reloaded
    /// </summary>
    Reloaded,

    /// <summary>
    /// Application was restarted
    /// </summary>
    ApplicationRestarted
}