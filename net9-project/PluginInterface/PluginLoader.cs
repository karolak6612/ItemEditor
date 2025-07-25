using Microsoft.Extensions.Logging;
using System.Reflection;
using System.Runtime.Loader;

namespace PluginInterface;

/// <summary>
/// Modern plugin loader with assembly isolation and enhanced error handling
/// </summary>
public class PluginLoader : IDisposable
{
    private readonly ILogger<PluginLoader>? _logger;
    private readonly Dictionary<string, PluginLoadContext> _loadContexts = new();
    private bool _disposed;

    /// <summary>
    /// Initializes a new instance of the PluginLoader class
    /// </summary>
    /// <param name="logger">Logger instance</param>
    public PluginLoader(ILogger<PluginLoader>? logger = null)
    {
        _logger = logger;
    }

    /// <summary>
    /// Load a plugin from the specified assembly file
    /// </summary>
    /// <param name="assemblyPath">Path to the plugin assembly</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Loaded plugin instance or null if loading failed</returns>
    public async Task<IPlugin?> LoadPluginAsync(string assemblyPath, CancellationToken cancellationToken = default)
    {
        if (string.IsNullOrEmpty(assemblyPath))
            throw new ArgumentException("Assembly path cannot be null or empty", nameof(assemblyPath));

        if (!File.Exists(assemblyPath))
            throw new FileNotFoundException($"Plugin assembly not found: {assemblyPath}");

        try
        {
            _logger?.LogInformation("Loading plugin from {AssemblyPath}", assemblyPath);

            // Create isolated load context for the plugin
            var loadContext = new PluginLoadContext(assemblyPath);
            _loadContexts[assemblyPath] = loadContext;

            // Load the assembly
            var assembly = loadContext.LoadFromAssemblyPath(assemblyPath);
            
            // Find plugin types
            var pluginTypes = await FindPluginTypesAsync(assembly, cancellationToken);
            
            if (pluginTypes.Count == 0)
            {
                _logger?.LogWarning("No plugin types found in assembly {AssemblyPath}", assemblyPath);
                return null;
            }

            // Create plugin instance (use first found plugin type)
            var pluginType = pluginTypes[0];
            var plugin = await CreatePluginInstanceAsync(pluginType, assemblyPath, cancellationToken);

            if (plugin != null)
            {
                _logger?.LogInformation("Successfully loaded plugin {PluginName} from {AssemblyPath}", 
                    plugin.Name, assemblyPath);
            }

            return plugin;
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Failed to load plugin from {AssemblyPath}", assemblyPath);
            
            // Clean up load context on failure
            if (_loadContexts.TryGetValue(assemblyPath, out var context))
            {
                context.Unload();
                _loadContexts.Remove(assemblyPath);
            }
            
            throw;
        }
    }

    /// <summary>
    /// Load all plugins from a directory
    /// </summary>
    /// <param name="pluginDirectory">Directory containing plugin assemblies</param>
    /// <param name="searchPattern">File search pattern (default: "*.dll")</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>List of loaded plugins</returns>
    public async Task<List<IPlugin>> LoadPluginsFromDirectoryAsync(
        string pluginDirectory, 
        string searchPattern = "*.dll", 
        CancellationToken cancellationToken = default)
    {
        if (string.IsNullOrEmpty(pluginDirectory))
            throw new ArgumentException("Plugin directory cannot be null or empty", nameof(pluginDirectory));

        if (!Directory.Exists(pluginDirectory))
        {
            _logger?.LogWarning("Plugin directory does not exist: {PluginDirectory}", pluginDirectory);
            return new List<IPlugin>();
        }

        var plugins = new List<IPlugin>();
        var assemblyFiles = Directory.GetFiles(pluginDirectory, searchPattern, SearchOption.TopDirectoryOnly);

        _logger?.LogInformation("Found {AssemblyCount} potential plugin assemblies in {PluginDirectory}", 
            assemblyFiles.Length, pluginDirectory);

        foreach (var assemblyFile in assemblyFiles)
        {
            try
            {
                cancellationToken.ThrowIfCancellationRequested();
                
                var plugin = await LoadPluginAsync(assemblyFile, cancellationToken);
                if (plugin != null)
                {
                    plugins.Add(plugin);
                }
            }
            catch (Exception ex)
            {
                _logger?.LogError(ex, "Failed to load plugin from {AssemblyFile}", assemblyFile);
                // Continue loading other plugins
            }
        }

        _logger?.LogInformation("Successfully loaded {PluginCount} plugins from {PluginDirectory}", 
            plugins.Count, pluginDirectory);

        return plugins;
    }

    /// <summary>
    /// Unload a plugin and its associated assembly context
    /// </summary>
    /// <param name="plugin">Plugin to unload</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task representing the async operation</returns>
    public async Task UnloadPluginAsync(IPlugin plugin, CancellationToken cancellationToken = default)
    {
        if (plugin == null)
            throw new ArgumentNullException(nameof(plugin));

        try
        {
            _logger?.LogInformation("Unloading plugin {PluginName}", plugin.Name);

            // Shutdown the plugin
            if (plugin.IsLoaded)
            {
                await plugin.ShutdownAsync(cancellationToken);
            }

            // Dispose the plugin
            plugin.Dispose();

            // Find and unload the associated load context
            var assemblyPath = plugin.Metadata.FilePath;
            if (!string.IsNullOrEmpty(assemblyPath) && _loadContexts.TryGetValue(assemblyPath, out var context))
            {
                context.Unload();
                _loadContexts.Remove(assemblyPath);
            }

            _logger?.LogInformation("Successfully unloaded plugin {PluginName}", plugin.Name);
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Failed to unload plugin {PluginName}", plugin.Name);
            throw;
        }
    }

    /// <summary>
    /// Validate a plugin assembly without loading it
    /// </summary>
    /// <param name="assemblyPath">Path to the plugin assembly</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Validation result</returns>
    public async Task<PluginValidationResult> ValidatePluginAsync(string assemblyPath, CancellationToken cancellationToken = default)
    {
        var result = new PluginValidationResult();

        try
        {
            if (!File.Exists(assemblyPath))
            {
                result.IsValid = false;
                result.Errors = new[] { $"Assembly file not found: {assemblyPath}" };
                return result;
            }

            // Try to load assembly metadata without executing code
            var assemblyName = AssemblyName.GetAssemblyName(assemblyPath);
            
            // Create temporary load context for validation
            using var tempContext = new PluginLoadContext(assemblyPath);
            var assembly = tempContext.LoadFromAssemblyPath(assemblyPath);
            
            var pluginTypes = await FindPluginTypesAsync(assembly, cancellationToken);
            
            if (pluginTypes.Count == 0)
            {
                result.IsValid = false;
                result.Errors = new[] { "No plugin types implementing IPlugin interface found" };
                return result;
            }

            result.IsValid = true;
            result.Message = $"Found {pluginTypes.Count} plugin type(s) in assembly";
        }
        catch (Exception ex)
        {
            result.IsValid = false;
            result.Errors = new[] { $"Validation failed: {ex.Message}" };
        }

        return result;
    }

    private async Task<List<Type>> FindPluginTypesAsync(Assembly assembly, CancellationToken cancellationToken)
    {
        return await Task.Run(() =>
        {
            var pluginTypes = new List<Type>();

            try
            {
                var types = assembly.GetTypes();
                
                foreach (var type in types)
                {
                    cancellationToken.ThrowIfCancellationRequested();
                    
                    // Check for modern IPlugin interface
                    if (typeof(IPlugin).IsAssignableFrom(type) && !type.IsInterface && !type.IsAbstract)
                    {
                        pluginTypes.Add(type);
                    }
                    // Check for legacy plugin interface
                    else if (typeof(ILegacyPlugin).IsAssignableFrom(type) && !type.IsInterface && !type.IsAbstract)
                    {
                        pluginTypes.Add(type);
                    }
                }
            }
            catch (ReflectionTypeLoadException ex)
            {
                _logger?.LogWarning("Some types could not be loaded from assembly: {Exceptions}", 
                    string.Join(", ", ex.LoaderExceptions.Select(e => e?.Message)));
                
                // Use successfully loaded types
                var loadedTypes = ex.Types.Where(t => t != null);
                foreach (var type in loadedTypes)
                {
                    if (typeof(IPlugin).IsAssignableFrom(type!) && !type!.IsInterface && !type.IsAbstract)
                    {
                        pluginTypes.Add(type);
                    }
                }
            }

            return pluginTypes;
        }, cancellationToken);
    }

    private async Task<IPlugin?> CreatePluginInstanceAsync(Type pluginType, string assemblyPath, CancellationToken cancellationToken)
    {
        return await Task.Run(() =>
        {
            try
            {
                // Check if it's a legacy plugin
                if (typeof(ILegacyPlugin).IsAssignableFrom(pluginType) && !typeof(IPlugin).IsAssignableFrom(pluginType))
                {
                    var legacyPlugin = (ILegacyPlugin)Activator.CreateInstance(pluginType)!;
                    var adapter = new LegacyPluginAdapter(legacyPlugin);
                    adapter.Metadata.FilePath = assemblyPath;
                    return adapter;
                }

                // Create modern plugin instance
                var plugin = (IPlugin)Activator.CreateInstance(pluginType)!;
                plugin.Metadata.FilePath = assemblyPath;
                return plugin;
            }
            catch (Exception ex)
            {
                _logger?.LogError(ex, "Failed to create instance of plugin type {PluginType}", pluginType.FullName);
                return null;
            }
        }, cancellationToken);
    }

    /// <inheritdoc />
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    /// <summary>
    /// Disposes the plugin loader and unloads all plugin contexts
    /// </summary>
    /// <param name="disposing">True if disposing managed resources</param>
    protected virtual void Dispose(bool disposing)
    {
        if (!_disposed && disposing)
        {
            foreach (var context in _loadContexts.Values)
            {
                try
                {
                    context.Unload();
                }
                catch (Exception ex)
                {
                    _logger?.LogError(ex, "Error unloading plugin context");
                }
            }
            
            _loadContexts.Clear();
            _disposed = true;
        }
    }
}

/// <summary>
/// Plugin assembly load context for isolation
/// </summary>
internal class PluginLoadContext : AssemblyLoadContext
{
    private readonly AssemblyDependencyResolver _resolver;

    public PluginLoadContext(string pluginPath) : base(isCollectible: true)
    {
        _resolver = new AssemblyDependencyResolver(pluginPath);
    }

    protected override Assembly? Load(AssemblyName assemblyName)
    {
        var assemblyPath = _resolver.ResolveAssemblyToPath(assemblyName);
        return assemblyPath != null ? LoadFromAssemblyPath(assemblyPath) : null;
    }

    protected override IntPtr LoadUnmanagedDll(string unmanagedDllName)
    {
        var libraryPath = _resolver.ResolveUnmanagedDllToPath(unmanagedDllName);
        return libraryPath != null ? LoadUnmanagedDllFromPath(libraryPath) : IntPtr.Zero;
    }
}