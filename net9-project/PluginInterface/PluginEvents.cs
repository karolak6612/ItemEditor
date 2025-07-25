namespace PluginInterface;

/// <summary>
/// Event arguments for plugin events with enhanced information
/// </summary>
public class PluginEventArgs : EventArgs
{
    /// <summary>
    /// Initializes a new instance of the PluginEventArgs class
    /// </summary>
    /// <param name="plugin">The plugin</param>
    public PluginEventArgs(IPlugin plugin)
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
    
    /// <summary>
    /// Gets or sets additional event data
    /// </summary>
    public Dictionary<string, object> Data { get; set; } = new();
}

/// <summary>
/// Event arguments for plugin loading events
/// </summary>
public class PluginLoadingEventArgs : PluginEventArgs
{
    /// <summary>
    /// Initializes a new instance of the PluginLoadingEventArgs class
    /// </summary>
    /// <param name="plugin">The plugin being loaded</param>
    /// <param name="filePath">Plugin file path</param>
    public PluginLoadingEventArgs(IPlugin plugin, string filePath) : base(plugin)
    {
        FilePath = filePath;
    }
    
    /// <summary>
    /// Gets the plugin file path
    /// </summary>
    public string FilePath { get; }
    
    /// <summary>
    /// Gets or sets whether to cancel the loading operation
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
public class PluginLoadedEventArgs : PluginEventArgs
{
    /// <summary>
    /// Initializes a new instance of the PluginLoadedEventArgs class
    /// </summary>
    /// <param name="plugin">The loaded plugin</param>
    /// <param name="loadTimeMs">Load time in milliseconds</param>
    public PluginLoadedEventArgs(IPlugin plugin, long loadTimeMs) : base(plugin)
    {
        LoadTimeMs = loadTimeMs;
    }
    
    /// <summary>
    /// Gets the plugin load time in milliseconds
    /// </summary>
    public long LoadTimeMs { get; }
}

/// <summary>
/// Event arguments for plugin error events
/// </summary>
public class PluginErrorEventArgs : PluginEventArgs
{
    /// <summary>
    /// Initializes a new instance of the PluginErrorEventArgs class
    /// </summary>
    /// <param name="plugin">The plugin that caused the error</param>
    /// <param name="exception">The exception that occurred</param>
    /// <param name="operation">The operation that was being performed</param>
    public PluginErrorEventArgs(IPlugin plugin, Exception exception, string operation) : base(plugin)
    {
        Exception = exception;
        Operation = operation;
    }
    
    /// <summary>
    /// Gets the exception that occurred
    /// </summary>
    public Exception Exception { get; }
    
    /// <summary>
    /// Gets the operation that was being performed when the error occurred
    /// </summary>
    public string Operation { get; }
    
    /// <summary>
    /// Gets or sets whether the error was handled
    /// </summary>
    public bool Handled { get; set; }
}

/// <summary>
/// Event arguments for plugin unloading events
/// </summary>
public class PluginUnloadingEventArgs : PluginEventArgs
{
    /// <summary>
    /// Initializes a new instance of the PluginUnloadingEventArgs class
    /// </summary>
    /// <param name="plugin">The plugin being unloaded</param>
    /// <param name="reason">The reason for unloading</param>
    public PluginUnloadingEventArgs(IPlugin plugin, PluginUnloadReason reason) : base(plugin)
    {
        Reason = reason;
    }
    
    /// <summary>
    /// Gets the reason for unloading the plugin
    /// </summary>
    public PluginUnloadReason Reason { get; }
    
    /// <summary>
    /// Gets or sets whether to cancel the unloading operation
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
public class PluginUnloadedEventArgs : PluginEventArgs
{
    /// <summary>
    /// Initializes a new instance of the PluginUnloadedEventArgs class
    /// </summary>
    /// <param name="plugin">The unloaded plugin</param>
    /// <param name="reason">The reason for unloading</param>
    public PluginUnloadedEventArgs(IPlugin plugin, PluginUnloadReason reason) : base(plugin)
    {
        Reason = reason;
    }
    
    /// <summary>
    /// Gets the reason for unloading the plugin
    /// </summary>
    public PluginUnloadReason Reason { get; }
}

/// <summary>
/// Reasons for plugin unloading
/// </summary>
public enum PluginUnloadReason
{
    /// <summary>
    /// User requested unload
    /// </summary>
    UserRequested,
    
    /// <summary>
    /// Application shutdown
    /// </summary>
    ApplicationShutdown,
    
    /// <summary>
    /// Plugin error
    /// </summary>
    Error,
    
    /// <summary>
    /// Plugin incompatibility
    /// </summary>
    Incompatible,
    
    /// <summary>
    /// Plugin update
    /// </summary>
    Update,
    
    /// <summary>
    /// Dependency missing
    /// </summary>
    DependencyMissing
}