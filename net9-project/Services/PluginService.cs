using Microsoft.Extensions.Logging;
using PluginInterface;
using ItemEditor.Models;
using System.Collections.Concurrent;
using System.Diagnostics;

namespace ItemEditor.Services;

/// <summary>
/// Modern implementation of plugin service with enhanced loading and management
/// </summary>
public class PluginService : IPluginService, IDisposable
{
    private readonly ILogger<PluginService> _logger;
    private readonly ModernPluginLoader _pluginLoader;
    private readonly IPluginServiceProvider _pluginServiceProvider;
    private readonly IPluginEventSystem _pluginEventSystem;
    private readonly ConcurrentDictionary<string, IPlugin> _loadedPlugins = new();
    private readonly ConcurrentDictionary<string, PluginInfo> _pluginInfoCache = new();
    private readonly object _lock = new();
    private bool _disposed;
    
    /// <summary>
    /// Initializes a new instance of the PluginService class
    /// </summary>
    /// <param name="logger">Logger instance</param>
    /// <param name="pluginServiceProvider">Plugin service provider</param>
    /// <param name="pluginEventSystem">Plugin event system</param>
    public PluginService(
        ILogger<PluginService> logger,
        IPluginServiceProvider pluginServiceProvider,
        IPluginEventSystem pluginEventSystem)
    {
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
        _pluginServiceProvider = pluginServiceProvider ?? throw new ArgumentNullException(nameof(pluginServiceProvider));
        _pluginEventSystem = pluginEventSystem ?? throw new ArgumentNullException(nameof(pluginEventSystem));
        _pluginLoader = new ModernPluginLoader(logger);
    }
    
    /// <inheritdoc />
    public event EventHandler<PluginLoadingEventArgs>? PluginLoading;
    
    /// <inheritdoc />
    public event EventHandler<PluginLoadedEventArgs>? PluginLoaded;
    
    /// <inheritdoc />
    public event EventHandler<PluginUnloadingEventArgs>? PluginUnloading;
    
    /// <inheritdoc />
    public event EventHandler<PluginUnloadedEventArgs>? PluginUnloaded;
    
    /// <inheritdoc />
    public event EventHandler<PluginErrorEventArgs>? PluginError;
    
    /// <inheritdoc />
    public async Task<IEnumerable<IPlugin>> LoadPluginsAsync(string pluginDirectory, string searchPattern = "*.dll", CancellationToken cancellationToken = default)
    {
        try
        {
            _logger.LogInformation("Loading plugins from directory {PluginDirectory} with pattern {SearchPattern}", 
                pluginDirectory, searchPattern);
            
            if (!Directory.Exists(pluginDirectory))
            {
                _logger.LogWarning("Plugin directory does not exist: {PluginDirectory}", pluginDirectory);
                return Enumerable.Empty<IPlugin>();
            }
            
            var pluginFiles = Directory.GetFiles(pluginDirectory, searchPattern, SearchOption.TopDirectoryOnly);
            var loadedPlugins = new List<IPlugin>();
            
            foreach (var pluginFile in pluginFiles)
            {
                cancellationToken.ThrowIfCancellationRequested();
                
                try
                {
                    var plugin = await LoadPluginAsync(pluginFile, cancellationToken);
                    if (plugin != null)
                    {
                        loadedPlugins.Add(plugin);
                    }
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, "Failed to load plugin from {PluginFile}", pluginFile);
                    // Continue loading other plugins
                }
            }
            
            _logger.LogInformation("Successfully loaded {PluginCount} plugins from {PluginDirectory}", 
                loadedPlugins.Count, pluginDirectory);
            return loadedPlugins;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error loading plugins from {PluginDirectory}", pluginDirectory);
            throw;
        }
    }
    
    /// <inheritdoc />
    public async Task<PluginValidationResult> ValidatePluginAsync(string pluginPath, CancellationToken cancellationToken = default)
    {
        try
        {
            _logger.LogDebug("Validating plugin {PluginPath}", pluginPath);
            
            return await _pluginLoader.ValidatePluginFileAsync(pluginPath, cancellationToken);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error validating plugin {PluginPath}", pluginPath);
            return new PluginValidationResult
            {
                IsValid = false,
                Message = $"Validation error: {ex.Message}",
                Severity = ValidationSeverity.Critical,
                Errors = { ex.Message }
            };
        }
    }

    /// <inheritdoc />
    public async Task<IPlugin?> LoadPluginAsync(string pluginPath, CancellationToken cancellationToken = default)
    {
        if (string.IsNullOrEmpty(pluginPath))
            throw new ArgumentException("Plugin path cannot be null or empty", nameof(pluginPath));

        try
        {
            var stopwatch = Stopwatch.StartNew();
            
            // Check if already loaded
            if (_loadedPlugins.TryGetValue(pluginPath, out var existingPlugin))
            {
                _logger.LogInformation("Plugin already loaded: {PluginPath}", pluginPath);
                return existingPlugin;
            }

            // Validate plugin first
            var validationResult = await ValidatePluginAsync(pluginPath, cancellationToken);
            if (!validationResult.IsValid)
            {
                _logger.LogError("Plugin validation failed: {PluginPath}. Errors: {Errors}", 
                    pluginPath, string.Join(", ", validationResult.Errors));
                return null;
            }

            // Create temporary plugin instance for loading event
            var tempPlugin = new PluginPlaceholder(Path.GetFileNameWithoutExtension(pluginPath));
            var loadingArgs = new PluginLoadingEventArgs(tempPlugin, pluginPath);
            PluginLoading?.Invoke(this, loadingArgs);
            
            if (loadingArgs.Cancel)
            {
                _logger.LogInformation("Plugin loading cancelled: {PluginPath}. Reason: {Reason}", 
                    pluginPath, loadingArgs.CancellationReason);
                return null;
            }

            // Load plugin using modern loader
            var plugin = await _pluginLoader.LoadPluginAsync(pluginPath, cancellationToken);
            stopwatch.Stop();

            if (plugin != null)
            {
                // Create service provider for plugin
                var pluginServiceProvider = _pluginServiceProvider.CreatePluginServiceProvider(plugin);
                
                // Initialize plugin with dependency injection
                await InitializePluginAsync(plugin, cancellationToken);
                
                // Subscribe to plugin events
                SubscribeToPluginEvents(plugin);
                
                // Cache plugin
                _loadedPlugins.TryAdd(pluginPath, plugin);
                
                // Cache plugin info
                var pluginInfo = CreatePluginInfo(plugin, pluginPath);
                _pluginInfoCache.TryAdd(pluginPath, pluginInfo);

                // Publish plugin loaded event
                var pluginEvent = new PluginEvent("PluginLoaded", plugin, new { LoadTime = stopwatch.Elapsed });
                await _pluginEventSystem.PublishAsync(pluginEvent);

                // Raise loaded event
                var loadedArgs = new PluginLoadedEventArgs(plugin, stopwatch.Elapsed);
                if (validationResult.Warnings.Any())
                {
                    loadedArgs.Warnings.AddRange(validationResult.Warnings);
                }
                PluginLoaded?.Invoke(this, loadedArgs);

                _logger.LogInformation("Successfully loaded and initialized plugin: {PluginName} from {PluginPath} in {LoadTime}ms", 
                    plugin.Name, pluginPath, stopwatch.ElapsedMilliseconds);
            }

            return plugin;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to load plugin from {PluginPath}", pluginPath);
            
            // Raise error event
            var errorArgs = new PluginErrorEventArgs(
                new PluginPlaceholder(Path.GetFileNameWithoutExtension(pluginPath)), 
                ex, 
                PluginErrorType.LoadError);
            PluginError?.Invoke(this, errorArgs);
            
            return null;
        }
    }
    
    /// <inheritdoc />
    public IEnumerable<IPlugin> GetLoadedPlugins()
    {
        return _loadedPlugins.Values.ToList();
    }

    /// <inheritdoc />
    public IEnumerable<IPlugin> GetPluginsByCategory(PluginCategory category)
    {
        return _loadedPlugins.Values
            .Where(plugin => plugin.Metadata.Category == category)
            .ToList();
    }

    /// <inheritdoc />
    public IPlugin? GetPluginByName(string name)
    {
        return _loadedPlugins.Values
            .FirstOrDefault(plugin => string.Equals(plugin.Name, name, StringComparison.OrdinalIgnoreCase));
    }

    /// <inheritdoc />
    public bool IsPluginLoaded(IPlugin plugin)
    {
        return _loadedPlugins.Values.Contains(plugin);
    }

    /// <inheritdoc />
    public async Task InitializePluginAsync(IPlugin plugin, CancellationToken cancellationToken = default)
    {
        try
        {
            _logger.LogDebug("Initializing plugin: {PluginName}", plugin.Name);
            
            await plugin.InitializeAsync(_logger, cancellationToken);
            
            _logger.LogDebug("Successfully initialized plugin: {PluginName}", plugin.Name);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to initialize plugin: {PluginName}", plugin.Name);
            
            var errorArgs = new PluginErrorEventArgs(plugin, ex, PluginErrorType.InitializationError);
            PluginError?.Invoke(this, errorArgs);
            
            throw;
        }
    }

    /// <inheritdoc />
    public async Task ConfigurePluginAsync(IPlugin plugin, PluginConfiguration configuration, CancellationToken cancellationToken = default)
    {
        try
        {
            _logger.LogDebug("Configuring plugin: {PluginName}", plugin.Name);
            
            await plugin.ConfigureAsync(configuration, cancellationToken);
            
            _logger.LogDebug("Successfully configured plugin: {PluginName}", plugin.Name);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to configure plugin: {PluginName}", plugin.Name);
            
            var errorArgs = new PluginErrorEventArgs(plugin, ex, PluginErrorType.ConfigurationError);
            PluginError?.Invoke(this, errorArgs);
            
            throw;
        }
    }
    
    /// <inheritdoc />
    public async Task UnloadPluginAsync(IPlugin plugin, PluginUnloadReason reason = PluginUnloadReason.UserRequested, CancellationToken cancellationToken = default)
    {
        if (plugin == null)
            throw new ArgumentNullException(nameof(plugin));

        try
        {
            var stopwatch = Stopwatch.StartNew();
            
            // Find plugin path
            var pluginPath = _loadedPlugins.FirstOrDefault(kvp => kvp.Value == plugin).Key;
            if (string.IsNullOrEmpty(pluginPath))
            {
                _logger.LogWarning("Plugin not found in loaded plugins: {PluginName}", plugin.Name);
                return;
            }

            _logger.LogInformation("Unloading plugin {PluginName} for reason: {Reason}", plugin.Name, reason);
            
            // Raise unloading event
            var unloadingArgs = new PluginUnloadingEventArgs(plugin, reason);
            PluginUnloading?.Invoke(this, unloadingArgs);
            
            if (unloadingArgs.Cancel)
            {
                _logger.LogInformation("Plugin unloading cancelled: {PluginName}. Reason: {Reason}", 
                    plugin.Name, unloadingArgs.CancellationReason);
                return;
            }

            // Unsubscribe from plugin events
            UnsubscribeFromPluginEvents(plugin);
            
            // Remove plugin service provider
            _pluginServiceProvider.RemovePluginServiceProvider(plugin);
            
            // Publish plugin unloading event
            var pluginEvent = new PluginEvent("PluginUnloading", plugin, new { Reason = reason });
            await _pluginEventSystem.PublishAsync(pluginEvent);
            
            // Unload using modern loader
            var success = await _pluginLoader.UnloadPluginAsync(pluginPath, cancellationToken);
            stopwatch.Stop();

            if (success)
            {
                // Remove from caches
                _loadedPlugins.TryRemove(pluginPath, out _);
                _pluginInfoCache.TryRemove(pluginPath, out _);

                // Publish plugin unloaded event
                var unloadedEvent = new PluginEvent("PluginUnloaded", plugin, new { Reason = reason, UnloadTime = stopwatch.Elapsed });
                await _pluginEventSystem.PublishAsync(unloadedEvent);

                // Raise unloaded event
                var unloadedArgs = new PluginUnloadedEventArgs(plugin, reason, stopwatch.Elapsed);
                PluginUnloaded?.Invoke(this, unloadedArgs);

                _logger.LogInformation("Successfully unloaded plugin {PluginName} in {UnloadTime}ms", 
                    plugin.Name, stopwatch.ElapsedMilliseconds);
            }
            else
            {
                _logger.LogError("Failed to unload plugin: {PluginName}", plugin.Name);
                
                var errorArgs = new PluginErrorEventArgs(plugin, 
                    new InvalidOperationException("Plugin unload failed"), 
                    PluginErrorType.UnloadError);
                PluginError?.Invoke(this, errorArgs);
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error unloading plugin {PluginName}", plugin.Name);
            
            var errorArgs = new PluginErrorEventArgs(plugin, ex, PluginErrorType.UnloadError);
            PluginError?.Invoke(this, errorArgs);
            
            throw;
        }
    }
    
    /// <inheritdoc />
    public async Task LoadPluginsAsync(CancellationToken cancellationToken = default)
    {
        try
        {
            _logger.LogInformation("Loading plugins from default directory");
            
            // Use default plugin directory
            var pluginDirectory = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "Plugins");
            
            var plugins = await LoadPluginsAsync(pluginDirectory, "*.dll", cancellationToken);
            
            _logger.LogInformation("Successfully loaded {PluginCount} plugins from default directory", plugins.Count());
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error loading plugins from default directory");
            throw;
        }
    }
    
    /// <inheritdoc />
    public async Task UnloadAllPluginsAsync(PluginUnloadReason reason = PluginUnloadReason.ApplicationShutdown, CancellationToken cancellationToken = default)
    {
        try
        {
            _logger.LogInformation("Unloading all plugins for reason: {Reason}", reason);
            
            var pluginsToUnload = _loadedPlugins.Values.ToList();
            
            foreach (var plugin in pluginsToUnload)
            {
                await UnloadPluginAsync(plugin, reason, cancellationToken);
            }
            
            _logger.LogInformation("Successfully unloaded all plugins");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error unloading all plugins");
            throw;
        }
    }

    /// <inheritdoc />
    public async Task<IPlugin?> ReloadPluginAsync(IPlugin plugin, CancellationToken cancellationToken = default)
    {
        if (plugin == null)
            throw new ArgumentNullException(nameof(plugin));

        try
        {
            _logger.LogInformation("Reloading plugin: {PluginName}", plugin.Name);
            
            // Find plugin path
            var pluginPath = _loadedPlugins.FirstOrDefault(kvp => kvp.Value == plugin).Key;
            if (string.IsNullOrEmpty(pluginPath))
            {
                _logger.LogWarning("Plugin not found for reload: {PluginName}", plugin.Name);
                return null;
            }

            // Unload first
            await UnloadPluginAsync(plugin, PluginUnloadReason.Update, cancellationToken);
            
            // Load again
            var reloadedPlugin = await LoadPluginAsync(pluginPath, cancellationToken);
            
            _logger.LogInformation("Successfully reloaded plugin: {PluginName}", plugin.Name);
            return reloadedPlugin;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to reload plugin: {PluginName}", plugin.Name);
            
            var errorArgs = new PluginErrorEventArgs(plugin, ex, PluginErrorType.LoadError);
            PluginError?.Invoke(this, errorArgs);
            
            return null;
        }
    }

    /// <inheritdoc />
    public PluginStatistics GetPluginStatistics()
    {
        var plugins = _loadedPlugins.Values.ToList();
        
        return new PluginStatistics
        {
            TotalPlugins = plugins.Count,
            ActivePlugins = plugins.Count(p => p.IsLoaded),
            LegacyPlugins = plugins.Count(p => p.Metadata.IsLegacy),
            ModernPlugins = plugins.Count(p => !p.Metadata.IsLegacy),
            PluginsByCategory = plugins.GroupBy(p => p.Metadata.Category)
                .ToDictionary(g => g.Key, g => g.Count()),
            AverageLoadTime = 0, // Would need to track load times
            TotalMemoryUsage = 0 // Would need to track memory usage
        };
    }

    /// <inheritdoc />
    public async Task<List<PluginDependencyConflict>> CheckDependencyConflictsAsync(IPlugin plugin)
    {
        var conflicts = new List<PluginDependencyConflict>();
        
        try
        {
            // Check each dependency
            foreach (var dependency in plugin.Metadata.Dependencies)
            {
                var dependentPlugin = GetPluginByName(dependency.Name);
                
                if (dependentPlugin == null && !dependency.IsOptional)
                {
                    conflicts.Add(new PluginDependencyConflict
                    {
                        ConflictingPlugin = plugin,
                        Dependency = dependency,
                        ConflictType = DependencyConflictType.Missing,
                        Description = $"Required dependency '{dependency.Name}' is not loaded",
                        SuggestedResolution = $"Load plugin '{dependency.Name}' version {dependency.MinVersion} or higher"
                    });
                }
                else if (dependentPlugin != null)
                {
                    // Check version compatibility
                    if (dependentPlugin.Version < dependency.MinVersion)
                    {
                        conflicts.Add(new PluginDependencyConflict
                        {
                            ConflictingPlugin = plugin,
                            Dependency = dependency,
                            ConflictType = DependencyConflictType.VersionMismatch,
                            Description = $"Dependency '{dependency.Name}' version {dependentPlugin.Version} is below minimum required version {dependency.MinVersion}",
                            SuggestedResolution = $"Update plugin '{dependency.Name}' to version {dependency.MinVersion} or higher"
                        });
                    }
                    
                    if (dependency.MaxVersion != null && dependentPlugin.Version > dependency.MaxVersion)
                    {
                        conflicts.Add(new PluginDependencyConflict
                        {
                            ConflictingPlugin = plugin,
                            Dependency = dependency,
                            ConflictType = DependencyConflictType.VersionMismatch,
                            Description = $"Dependency '{dependency.Name}' version {dependentPlugin.Version} is above maximum supported version {dependency.MaxVersion}",
                            SuggestedResolution = $"Downgrade plugin '{dependency.Name}' to version {dependency.MaxVersion} or lower"
                        });
                    }
                }
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error checking dependency conflicts for plugin: {PluginName}", plugin.Name);
        }
        
        return conflicts;
    }

    /// <inheritdoc />
    public async Task<IEnumerable<PluginInfo>> GetInstalledPluginsAsync()
    {
        return _pluginInfoCache.Values.ToList();
    }

    /// <inheritdoc />
    public async Task InstallPluginAsync(string pluginFilePath)
    {
        // Implementation would copy plugin to plugins directory and load it
        throw new NotImplementedException("Plugin installation not yet implemented");
    }

    /// <inheritdoc />
    public void EnablePlugin(PluginInfo plugin)
    {
        // Implementation would enable a disabled plugin
        throw new NotImplementedException("Plugin enable/disable not yet implemented");
    }

    /// <inheritdoc />
    public void DisablePlugin(PluginInfo plugin)
    {
        // Implementation would disable an enabled plugin
        throw new NotImplementedException("Plugin enable/disable not yet implemented");
    }

    /// <inheritdoc />
    public async Task RemovePluginAsync(PluginInfo plugin)
    {
        // Implementation would unload and delete plugin
        throw new NotImplementedException("Plugin removal not yet implemented");
    }

    /// <summary>
    /// Creates plugin info from a loaded plugin
    /// </summary>
    /// <param name="plugin">Plugin instance</param>
    /// <param name="filePath">Plugin file path</param>
    /// <returns>Plugin info</returns>
    private static PluginInfo CreatePluginInfo(IPlugin plugin, string filePath)
    {
        return new PluginInfo
        {
            Name = plugin.Name,
            Description = plugin.Description,
            Version = plugin.Version.ToString(),
            Author = plugin.Author,
            FilePath = filePath,
            LoadedAt = DateTime.UtcNow,
            SupportedClients = plugin.Metadata.SupportedHostVersions.Select(v => v.ToString()).ToList(),
            IsEnabled = plugin.IsLoaded
        };
    }

    /// <summary>
    /// Subscribes to plugin events
    /// </summary>
    /// <param name="plugin">Plugin to subscribe to</param>
    private void SubscribeToPluginEvents(IPlugin plugin)
    {
        try
        {
            // Subscribe to plugin property changes
            plugin.PropertyChanged += (sender, e) =>
            {
                var pluginEvent = new PluginEvent("PluginPropertyChanged", plugin, new { PropertyName = e.PropertyName });
                _pluginEventSystem.Publish(pluginEvent);
            };

            _logger.LogDebug("Subscribed to events for plugin: {PluginName}", plugin.Name);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to subscribe to events for plugin: {PluginName}", plugin.Name);
        }
    }

    /// <summary>
    /// Unsubscribes from plugin events
    /// </summary>
    /// <param name="plugin">Plugin to unsubscribe from</param>
    private void UnsubscribeFromPluginEvents(IPlugin plugin)
    {
        try
        {
            // Plugin events will be automatically cleaned up when plugin is disposed
            _logger.LogDebug("Unsubscribed from events for plugin: {PluginName}", plugin.Name);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to unsubscribe from events for plugin: {PluginName}", plugin.Name);
        }
    }

    /// <summary>
    /// Disposes the plugin service and unloads all plugins
    /// </summary>
    public void Dispose()
    {
        if (_disposed)
            return;

        try
        {
            UnloadAllPluginsAsync().Wait(TimeSpan.FromSeconds(30));
            _pluginLoader?.Dispose();
            _pluginServiceProvider?.Dispose();
            _pluginEventSystem?.Dispose();
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error during plugin service disposal");
        }

        _disposed = true;
    }
}

/// <summary>
/// Placeholder plugin for events when actual plugin is not yet available
/// </summary>
internal class PluginPlaceholder : IPlugin
{
    public PluginPlaceholder(string name)
    {
        Name = name;
        Metadata = new PluginMetadata { Name = name };
    }

    public string Name { get; }
    public Version Version { get; } = new();
    public string Description { get; } = string.Empty;
    public string Author { get; } = string.Empty;
    public PluginMetadata Metadata { get; }
    public bool IsLoaded { get; } = false;
    public bool IsCompatible { get; } = true;

    public event PropertyChangedEventHandler? PropertyChanged;

    public Task InitializeAsync(ILogger? logger = null, CancellationToken cancellationToken = default) => Task.CompletedTask;
    public Task ShutdownAsync(CancellationToken cancellationToken = default) => Task.CompletedTask;
    public Task<PluginValidationResult> ValidateCompatibilityAsync(Version hostVersion, CancellationToken cancellationToken = default) => 
        Task.FromResult(new PluginValidationResult { IsValid = true });
    public PluginConfigurationSchema? GetConfigurationSchema() => null;
    public Task ConfigureAsync(PluginConfiguration configuration, CancellationToken cancellationToken = default) => Task.CompletedTask;
    public void Dispose() { }
}
}