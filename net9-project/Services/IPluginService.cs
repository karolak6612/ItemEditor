using PluginInterface;

namespace ItemEditor.Services;

/// <summary>
/// Modern plugin service interface for plugin loading and management with dependency injection
/// </summary>
public interface IPluginService
{
    /// <summary>
    /// Loads plugins from the default plugin directory
    /// </summary>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task representing the asynchronous operation</returns>
    Task LoadPluginsAsync(CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Loads plugins from a specific directory with enhanced error handling
    /// </summary>
    /// <param name="pluginDirectory">Directory containing plugins</param>
    /// <param name="searchPattern">File search pattern (default: "*.dll")</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Collection of loaded plugins</returns>
    Task<IEnumerable<IPlugin>> LoadPluginsAsync(string pluginDirectory, string searchPattern = "*.dll", CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Loads a single plugin from file path
    /// </summary>
    /// <param name="pluginPath">Path to the plugin file</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Loaded plugin or null if loading failed</returns>
    Task<IPlugin?> LoadPluginAsync(string pluginPath, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Validates a plugin file with detailed validation results
    /// </summary>
    /// <param name="pluginPath">Path to the plugin file</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Detailed validation result</returns>
    Task<PluginValidationResult> ValidatePluginAsync(string pluginPath, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Gets all loaded plugins
    /// </summary>
    /// <returns>Collection of loaded plugins</returns>
    IEnumerable<IPlugin> GetLoadedPlugins();
    
    /// <summary>
    /// Gets plugins by category
    /// </summary>
    /// <param name="category">Plugin category</param>
    /// <returns>Collection of plugins in the specified category</returns>
    IEnumerable<IPlugin> GetPluginsByCategory(PluginCategory category);
    
    /// <summary>
    /// Gets plugin by name
    /// </summary>
    /// <param name="name">Plugin name</param>
    /// <returns>Plugin with the specified name or null if not found</returns>
    IPlugin? GetPluginByName(string name);
    
    /// <summary>
    /// Checks if a plugin is loaded
    /// </summary>
    /// <param name="plugin">Plugin to check</param>
    /// <returns>True if the plugin is loaded</returns>
    bool IsPluginLoaded(IPlugin plugin);
    
    /// <summary>
    /// Initializes a loaded plugin
    /// </summary>
    /// <param name="plugin">Plugin to initialize</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task representing the async operation</returns>
    Task InitializePluginAsync(IPlugin plugin, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Configures a plugin with provided settings
    /// </summary>
    /// <param name="plugin">Plugin to configure</param>
    /// <param name="configuration">Plugin configuration</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task representing the async operation</returns>
    Task ConfigurePluginAsync(IPlugin plugin, PluginConfiguration configuration, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Unloads a specific plugin
    /// </summary>
    /// <param name="plugin">Plugin to unload</param>
    /// <param name="reason">Reason for unloading</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task representing the asynchronous operation</returns>
    Task UnloadPluginAsync(IPlugin plugin, PluginUnloadReason reason = PluginUnloadReason.UserRequested, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Unloads all plugins
    /// </summary>
    /// <param name="reason">Reason for unloading</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task representing the asynchronous operation</returns>
    Task UnloadAllPluginsAsync(PluginUnloadReason reason = PluginUnloadReason.ApplicationShutdown, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Reloads a plugin (unload then load)
    /// </summary>
    /// <param name="plugin">Plugin to reload</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Reloaded plugin instance</returns>
    Task<IPlugin?> ReloadPluginAsync(IPlugin plugin, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Gets plugin statistics
    /// </summary>
    /// <returns>Plugin statistics</returns>
    PluginStatistics GetPluginStatistics();
    
    /// <summary>
    /// Checks for plugin dependency conflicts
    /// </summary>
    /// <param name="plugin">Plugin to check</param>
    /// <returns>List of dependency conflicts</returns>
    Task<List<PluginDependencyConflict>> CheckDependencyConflictsAsync(IPlugin plugin);
    
    /// <summary>
    /// Event raised when a plugin is being loaded
    /// </summary>
    event EventHandler<PluginLoadingEventArgs>? PluginLoading;
    
    /// <summary>
    /// Event raised when a plugin is loaded
    /// </summary>
    event EventHandler<PluginLoadedEventArgs>? PluginLoaded;
    
    /// <summary>
    /// Event raised when a plugin is being unloaded
    /// </summary>
    event EventHandler<PluginUnloadingEventArgs>? PluginUnloading;
    
    /// <summary>
    /// Event raised when a plugin is unloaded
    /// </summary>
    event EventHandler<PluginUnloadedEventArgs>? PluginUnloaded;
    
    /// <summary>
    /// Event raised when a plugin error occurs
    /// </summary>
    event EventHandler<PluginErrorEventArgs>? PluginError;
    
    /// <summary>
    /// Gets information about installed plugins
    /// </summary>
    /// <returns>Collection of plugin information</returns>
    Task<IEnumerable<PluginInfo>> GetInstalledPluginsAsync();
    
    /// <summary>
    /// Installs a plugin from file
    /// </summary>
    /// <param name="pluginFilePath">Path to the plugin file</param>
    /// <returns>Task representing the async operation</returns>
    Task InstallPluginAsync(string pluginFilePath);
    
    /// <summary>
    /// Enables a plugin
    /// </summary>
    /// <param name="plugin">Plugin to enable</param>
    void EnablePlugin(PluginInfo plugin);
    
    /// <summary>
    /// Disables a plugin
    /// </summary>
    /// <param name="plugin">Plugin to disable</param>
    void DisablePlugin(PluginInfo plugin);
    
    /// <summary>
    /// Removes a plugin
    /// </summary>
    /// <param name="plugin">Plugin to remove</param>
    /// <returns>Task representing the async operation</returns>
    Task RemovePluginAsync(PluginInfo plugin);
}

/// <summary>
/// Plugin statistics information
/// </summary>
public class PluginStatistics
{
    /// <summary>
    /// Total number of loaded plugins
    /// </summary>
    public int TotalPlugins { get; set; }
    
    /// <summary>
    /// Number of active plugins
    /// </summary>
    public int ActivePlugins { get; set; }
    
    /// <summary>
    /// Number of legacy plugins
    /// </summary>
    public int LegacyPlugins { get; set; }
    
    /// <summary>
    /// Number of modern plugins
    /// </summary>
    public int ModernPlugins { get; set; }
    
    /// <summary>
    /// Plugins by category
    /// </summary>
    public Dictionary<PluginCategory, int> PluginsByCategory { get; set; } = new();
    
    /// <summary>
    /// Average plugin load time in milliseconds
    /// </summary>
    public double AverageLoadTime { get; set; }
    
    /// <summary>
    /// Total memory used by plugins in bytes
    /// </summary>
    public long TotalMemoryUsage { get; set; }
}

/// <summary>
/// Plugin dependency conflict information
/// </summary>
public class PluginDependencyConflict
{
    /// <summary>
    /// Conflicting plugin
    /// </summary>
    public IPlugin ConflictingPlugin { get; set; } = null!;
    
    /// <summary>
    /// Dependency that causes the conflict
    /// </summary>
    public PluginDependency Dependency { get; set; } = null!;
    
    /// <summary>
    /// Conflict type
    /// </summary>
    public DependencyConflictType ConflictType { get; set; }
    
    /// <summary>
    /// Conflict description
    /// </summary>
    public string Description { get; set; } = string.Empty;
    
    /// <summary>
    /// Suggested resolution
    /// </summary>
    public string? SuggestedResolution { get; set; }
}

/// <summary>
/// Types of dependency conflicts
/// </summary>
public enum DependencyConflictType
{
    /// <summary>
    /// Missing dependency
    /// </summary>
    Missing,
    
    /// <summary>
    /// Version mismatch
    /// </summary>
    VersionMismatch,
    
    /// <summary>
    /// Circular dependency
    /// </summary>
    Circular,
    
    /// <summary>
    /// Incompatible dependency
    /// </summary>
    Incompatible
}

/// <summary>
/// Plugin information for UI display
/// </summary>
public class PluginInfo
{
    /// <summary>
    /// Plugin name
    /// </summary>
    public string Name { get; set; } = string.Empty;
    
    /// <summary>
    /// Plugin description
    /// </summary>
    public string Description { get; set; } = string.Empty;
    
    /// <summary>
    /// Plugin version
    /// </summary>
    public string Version { get; set; } = string.Empty;
    
    /// <summary>
    /// Plugin author
    /// </summary>
    public string Author { get; set; } = string.Empty;
    
    /// <summary>
    /// Plugin file path
    /// </summary>
    public string FilePath { get; set; } = string.Empty;
    
    /// <summary>
    /// When the plugin was loaded
    /// </summary>
    public DateTime LoadedAt { get; set; }
    
    /// <summary>
    /// Supported client versions
    /// </summary>
    public List<string> SupportedClients { get; set; } = new();
    
    /// <summary>
    /// Whether the plugin is enabled
    /// </summary>
    public bool IsEnabled { get; set; }
}