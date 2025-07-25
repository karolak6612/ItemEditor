using System.Reflection;
using System.Runtime.Loader;
using System.Security;
using System.Security.Cryptography;
using System.Text;
using Microsoft.Extensions.Logging;
using PluginInterface;
using ItemEditor.Models;

namespace ItemEditor.Services;

/// <summary>
/// Modern plugin loader with assembly isolation and security
/// </summary>
public class ModernPluginLoader : IDisposable
{
    private readonly ILogger<ModernPluginLoader> _logger;
    private readonly Dictionary<string, PluginAssemblyLoadContext> _loadContexts = new();
    private readonly Dictionary<string, IPlugin> _loadedPlugins = new();
    private readonly object _lock = new();
    private bool _disposed;

    /// <summary>
    /// Initializes a new instance of the ModernPluginLoader class
    /// </summary>
    /// <param name="logger">Logger instance</param>
    public ModernPluginLoader(ILogger<ModernPluginLoader> logger)
    {
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
    }

    /// <summary>
    /// Loads a plugin from the specified file path with isolation
    /// </summary>
    /// <param name="pluginPath">Path to the plugin file</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Loaded plugin or null if loading failed</returns>
    public async Task<IPlugin?> LoadPluginAsync(string pluginPath, CancellationToken cancellationToken = default)
    {
        if (string.IsNullOrEmpty(pluginPath))
            throw new ArgumentException("Plugin path cannot be null or empty", nameof(pluginPath));

        if (!File.Exists(pluginPath))
        {
            _logger.LogWarning("Plugin file not found: {PluginPath}", pluginPath);
            return null;
        }

        lock (_lock)
        {
            if (_loadedPlugins.ContainsKey(pluginPath))
            {
                _logger.LogInformation("Plugin already loaded: {PluginPath}", pluginPath);
                return _loadedPlugins[pluginPath];
            }
        }

        try
        {
            _logger.LogInformation("Loading plugin from {PluginPath}", pluginPath);

            // Validate plugin file integrity
            var validationResult = await ValidatePluginFileAsync(pluginPath, cancellationToken);
            if (!validationResult.IsValid)
            {
                _logger.LogError("Plugin validation failed: {PluginPath}. Errors: {Errors}", 
                    pluginPath, string.Join(", ", validationResult.Errors));
                return null;
            }

            // Create isolated assembly load context
            var loadContext = new PluginAssemblyLoadContext(pluginPath, _logger);
            
            // Load the assembly
            var assembly = await Task.Run(() => loadContext.LoadFromAssemblyPath(pluginPath), cancellationToken);
            
            // Find plugin types
            var pluginTypes = FindPluginTypes(assembly);
            if (!pluginTypes.Any())
            {
                _logger.LogWarning("No plugin types found in assembly: {PluginPath}", pluginPath);
                loadContext.Unload();
                return null;
            }

            // Create plugin instance (use first found plugin type)
            var pluginType = pluginTypes.First();
            var plugin = await CreatePluginInstanceAsync(pluginType, pluginPath, cancellationToken);
            
            if (plugin != null)
            {
                lock (_lock)
                {
                    _loadContexts[pluginPath] = loadContext;
                    _loadedPlugins[pluginPath] = plugin;
                }

                _logger.LogInformation("Successfully loaded plugin: {PluginName} from {PluginPath}", 
                    plugin.Name, pluginPath);
            }
            else
            {
                loadContext.Unload();
            }

            return plugin;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to load plugin from {PluginPath}", pluginPath);
            return null;
        }
    }

    /// <summary>
    /// Unloads a plugin and its assembly context
    /// </summary>
    /// <param name="pluginPath">Path to the plugin file</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>True if unloaded successfully</returns>
    public async Task<bool> UnloadPluginAsync(string pluginPath, CancellationToken cancellationToken = default)
    {
        if (string.IsNullOrEmpty(pluginPath))
            return false;

        try
        {
            _logger.LogInformation("Unloading plugin: {PluginPath}", pluginPath);

            IPlugin? plugin = null;
            PluginAssemblyLoadContext? loadContext = null;

            lock (_lock)
            {
                _loadedPlugins.TryGetValue(pluginPath, out plugin);
                _loadContexts.TryGetValue(pluginPath, out loadContext);
            }

            // Shutdown plugin gracefully
            if (plugin != null)
            {
                try
                {
                    await plugin.ShutdownAsync(cancellationToken);
                    plugin.Dispose();
                }
                catch (Exception ex)
                {
                    _logger.LogWarning(ex, "Error during plugin shutdown: {PluginPath}", pluginPath);
                }
            }

            // Unload assembly context
            if (loadContext != null)
            {
                loadContext.Unload();
                
                // Wait for garbage collection to complete unloading
                for (int i = 0; i < 10 && loadContext.IsAlive; i++)
                {
                    GC.Collect();
                    GC.WaitForPendingFinalizers();
                    await Task.Delay(100, cancellationToken);
                }

                if (loadContext.IsAlive)
                {
                    _logger.LogWarning("Plugin assembly context still alive after unload: {PluginPath}", pluginPath);
                }
            }

            lock (_lock)
            {
                _loadedPlugins.Remove(pluginPath);
                _loadContexts.Remove(pluginPath);
            }

            _logger.LogInformation("Successfully unloaded plugin: {PluginPath}", pluginPath);
            return true;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to unload plugin: {PluginPath}", pluginPath);
            return false;
        }
    }

    /// <summary>
    /// Gets all loaded plugins
    /// </summary>
    /// <returns>Collection of loaded plugins</returns>
    public IEnumerable<IPlugin> GetLoadedPlugins()
    {
        lock (_lock)
        {
            return _loadedPlugins.Values.ToList();
        }
    }

    /// <summary>
    /// Validates a plugin file for security and integrity
    /// </summary>
    /// <param name="pluginPath">Path to the plugin file</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Validation result</returns>
    public async Task<PluginValidationResult> ValidatePluginFileAsync(string pluginPath, CancellationToken cancellationToken = default)
    {
        var result = new PluginValidationResult();

        try
        {
            if (!File.Exists(pluginPath))
            {
                result.IsValid = false;
                result.Errors.Add("Plugin file does not exist");
                result.Severity = ValidationSeverity.Error;
                return result;
            }

            var fileInfo = new FileInfo(pluginPath);
            
            // Check file extension
            if (!string.Equals(fileInfo.Extension, ".dll", StringComparison.OrdinalIgnoreCase))
            {
                result.Warnings.Add("Plugin file does not have .dll extension");
            }

            // Check file size (reasonable limits)
            if (fileInfo.Length > 100 * 1024 * 1024) // 100MB limit
            {
                result.Errors.Add("Plugin file is too large (>100MB)");
                result.Severity = ValidationSeverity.Error;
            }

            if (fileInfo.Length == 0)
            {
                result.Errors.Add("Plugin file is empty");
                result.Severity = ValidationSeverity.Error;
            }

            // Validate assembly structure
            try
            {
                var assemblyName = AssemblyName.GetAssemblyName(pluginPath);
                if (assemblyName == null)
                {
                    result.Errors.Add("Invalid assembly structure");
                    result.Severity = ValidationSeverity.Error;
                }
            }
            catch (Exception ex)
            {
                result.Errors.Add($"Assembly validation failed: {ex.Message}");
                result.Severity = ValidationSeverity.Error;
            }

            // Calculate file hash for integrity
            var hash = await CalculateFileHashAsync(pluginPath, cancellationToken);
            result.Message = $"File hash: {hash}";

            result.IsValid = !result.Errors.Any();
            if (result.IsValid && !result.Warnings.Any())
            {
                result.Message = "Plugin file validation passed";
                result.Severity = ValidationSeverity.Info;
            }
        }
        catch (Exception ex)
        {
            result.IsValid = false;
            result.Errors.Add($"Validation error: {ex.Message}");
            result.Severity = ValidationSeverity.Critical;
        }

        return result;
    }

    /// <summary>
    /// Finds plugin types in an assembly
    /// </summary>
    /// <param name="assembly">Assembly to search</param>
    /// <returns>Collection of plugin types</returns>
    private static IEnumerable<Type> FindPluginTypes(Assembly assembly)
    {
        try
        {
            return assembly.GetTypes()
                .Where(type => !type.IsAbstract && !type.IsInterface)
                .Where(type => typeof(IPlugin).IsAssignableFrom(type) || typeof(ILegacyPlugin).IsAssignableFrom(type));
        }
        catch (ReflectionTypeLoadException ex)
        {
            // Return only the types that loaded successfully
            return ex.Types.Where(type => type != null && !type.IsAbstract && !type.IsInterface)
                .Where(type => typeof(IPlugin).IsAssignableFrom(type) || typeof(ILegacyPlugin).IsAssignableFrom(type))!;
        }
    }

    /// <summary>
    /// Creates a plugin instance from a type
    /// </summary>
    /// <param name="pluginType">Plugin type</param>
    /// <param name="pluginPath">Plugin file path</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Plugin instance</returns>
    private async Task<IPlugin?> CreatePluginInstanceAsync(Type pluginType, string pluginPath, CancellationToken cancellationToken)
    {
        try
        {
            var instance = Activator.CreateInstance(pluginType);
            
            if (instance is IPlugin modernPlugin)
            {
                return modernPlugin;
            }
            else if (instance is ILegacyPlugin legacyPlugin)
            {
                // Wrap legacy plugin in adapter
                return new LegacyPluginAdapter(legacyPlugin);
            }
            
            _logger.LogWarning("Plugin type does not implement IPlugin or ILegacyPlugin: {PluginType}", pluginType.FullName);
            return null;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to create plugin instance: {PluginType}", pluginType.FullName);
            return null;
        }
    }

    /// <summary>
    /// Calculates SHA256 hash of a file
    /// </summary>
    /// <param name="filePath">File path</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>File hash</returns>
    private static async Task<string> CalculateFileHashAsync(string filePath, CancellationToken cancellationToken)
    {
        using var sha256 = SHA256.Create();
        using var stream = File.OpenRead(filePath);
        var hashBytes = await sha256.ComputeHashAsync(stream, cancellationToken);
        return Convert.ToHexString(hashBytes);
    }

    /// <summary>
    /// Disposes the plugin loader and unloads all plugins
    /// </summary>
    public void Dispose()
    {
        if (_disposed)
            return;

        try
        {
            var unloadTasks = new List<Task>();
            
            lock (_lock)
            {
                foreach (var pluginPath in _loadedPlugins.Keys.ToList())
                {
                    unloadTasks.Add(UnloadPluginAsync(pluginPath));
                }
            }

            Task.WaitAll(unloadTasks.ToArray(), TimeSpan.FromSeconds(30));
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error during plugin loader disposal");
        }

        _disposed = true;
    }
}

/// <summary>
/// Custom assembly load context for plugin isolation
/// </summary>
internal class PluginAssemblyLoadContext : AssemblyLoadContext
{
    private readonly string _pluginPath;
    private readonly ILogger _logger;
    private readonly AssemblyDependencyResolver _resolver;

    /// <summary>
    /// Initializes a new instance of the PluginAssemblyLoadContext class
    /// </summary>
    /// <param name="pluginPath">Path to the plugin assembly</param>
    /// <param name="logger">Logger instance</param>
    public PluginAssemblyLoadContext(string pluginPath, ILogger logger) : base(isCollectible: true)
    {
        _pluginPath = pluginPath;
        _logger = logger;
        _resolver = new AssemblyDependencyResolver(pluginPath);
    }

    /// <summary>
    /// Gets whether the load context is still alive
    /// </summary>
    public bool IsAlive => !IsCollectible || !IsCollectible;

    /// <summary>
    /// Loads an assembly by name
    /// </summary>
    /// <param name="assemblyName">Assembly name</param>
    /// <returns>Loaded assembly or null</returns>
    protected override Assembly? Load(AssemblyName assemblyName)
    {
        try
        {
            // Try to resolve the assembly path
            var assemblyPath = _resolver.ResolveAssemblyToPath(assemblyName);
            if (assemblyPath != null)
            {
                _logger.LogDebug("Loading dependency assembly: {AssemblyName} from {AssemblyPath}", 
                    assemblyName.Name, assemblyPath);
                return LoadFromAssemblyPath(assemblyPath);
            }

            // Let the default context handle system assemblies
            return null;
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "Failed to load assembly: {AssemblyName}", assemblyName.Name);
            return null;
        }
    }

    /// <summary>
    /// Loads an unmanaged library
    /// </summary>
    /// <param name="unmanagedDllName">Unmanaged DLL name</param>
    /// <returns>Library handle</returns>
    protected override IntPtr LoadUnmanagedDll(string unmanagedDllName)
    {
        try
        {
            var libraryPath = _resolver.ResolveUnmanagedDllToPath(unmanagedDllName);
            if (libraryPath != null)
            {
                _logger.LogDebug("Loading unmanaged library: {LibraryName} from {LibraryPath}", 
                    unmanagedDllName, libraryPath);
                return LoadUnmanagedDllFromPath(libraryPath);
            }

            return IntPtr.Zero;
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "Failed to load unmanaged library: {LibraryName}", unmanagedDllName);
            return IntPtr.Zero;
        }
    }
}