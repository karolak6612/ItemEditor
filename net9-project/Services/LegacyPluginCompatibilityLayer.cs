using Microsoft.Extensions.Logging;
using PluginInterface;
using ItemEditor.Models;
using System.Reflection;
using System.Collections.Concurrent;

namespace ItemEditor.Services;

/// <summary>
/// Compatibility layer for legacy plugins to work with modern plugin system
/// </summary>
public class LegacyPluginCompatibilityLayer : ILegacyPluginCompatibilityLayer, IDisposable
{
    private readonly ILogger<LegacyPluginCompatibilityLayer> _logger;
    private readonly ConcurrentDictionary<string, LegacyPluginWrapper> _legacyPlugins = new();
    private readonly ConcurrentDictionary<string, Type> _legacyPluginTypes = new();
    private bool _disposed;

    /// <summary>
    /// Initializes a new instance of the LegacyPluginCompatibilityLayer class
    /// </summary>
    /// <param name="logger">Logger instance</param>
    public LegacyPluginCompatibilityLayer(ILogger<LegacyPluginCompatibilityLayer> logger)
    {
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
    }

    /// <summary>
    /// Wraps a legacy plugin to work with the modern plugin system
    /// </summary>
    /// <param name="legacyPlugin">Legacy plugin instance</param>
    /// <param name="assemblyPath">Path to the plugin assembly</param>
    /// <returns>Modern plugin wrapper</returns>
    public IPlugin WrapLegacyPlugin(ILegacyPlugin legacyPlugin, string assemblyPath)
    {
        if (legacyPlugin == null)
            throw new ArgumentNullException(nameof(legacyPlugin));

        var pluginKey = GetPluginKey(legacyPlugin);

        if (_legacyPlugins.TryGetValue(pluginKey, out var existingWrapper))
        {
            _logger.LogDebug("Returning existing wrapper for legacy plugin: {PluginName}", legacyPlugin.Name);
            return existingWrapper;
        }

        try
        {
            _logger.LogInformation("Wrapping legacy plugin: {PluginName}", legacyPlugin.Name);

            var wrapper = new LegacyPluginWrapper(legacyPlugin, assemblyPath, _logger);
            _legacyPlugins.TryAdd(pluginKey, wrapper);
            _legacyPluginTypes.TryAdd(pluginKey, legacyPlugin.GetType());

            _logger.LogInformation("Successfully wrapped legacy plugin: {PluginName}", legacyPlugin.Name);
            return wrapper;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to wrap legacy plugin: {PluginName}", legacyPlugin.Name);
            throw;
        }
    }

    /// <summary>
    /// Detects if a type is a legacy plugin
    /// </summary>
    /// <param name="type">Type to check</param>
    /// <returns>True if type is a legacy plugin</returns>
    public bool IsLegacyPlugin(Type type)
    {
        if (type == null)
            return false;

        try
        {
            // Check if type implements ILegacyPlugin
            if (typeof(ILegacyPlugin).IsAssignableFrom(type))
                return true;

            // Check for legacy plugin patterns (common legacy interfaces)
            var legacyInterfaces = new[]
            {
                "IPlugin", // Common legacy interface name
                "IItemEditorPlugin",
                "IOTBPlugin",
                "IExtension"
            };

            foreach (var interfaceName in legacyInterfaces)
            {
                var legacyInterface = type.GetInterfaces()
                    .FirstOrDefault(i => i.Name == interfaceName && i != typeof(IPlugin));
                
                if (legacyInterface != null)
                {
                    _logger.LogDebug("Detected legacy plugin interface: {InterfaceName} in type: {TypeName}", 
                        interfaceName, type.FullName);
                    return true;
                }
            }

            // Check for legacy plugin attributes
            var legacyAttributes = new[]
            {
                "PluginAttribute",
                "ItemEditorPluginAttribute",
                "ExtensionAttribute"
            };

            foreach (var attributeName in legacyAttributes)
            {
                if (type.GetCustomAttributes().Any(attr => attr.GetType().Name == attributeName))
                {
                    _logger.LogDebug("Detected legacy plugin attribute: {AttributeName} in type: {TypeName}", 
                        attributeName, type.FullName);
                    return true;
                }
            }

            return false;
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "Error checking if type is legacy plugin: {TypeName}", type.FullName);
            return false;
        }
    }

    /// <summary>
    /// Creates a legacy plugin adapter for types that don't implement ILegacyPlugin
    /// </summary>
    /// <param name="pluginInstance">Plugin instance</param>
    /// <param name="pluginType">Plugin type</param>
    /// <returns>Legacy plugin adapter</returns>
    public ILegacyPlugin CreateLegacyPluginAdapter(object pluginInstance, Type pluginType)
    {
        if (pluginInstance == null)
            throw new ArgumentNullException(nameof(pluginInstance));
        
        if (pluginType == null)
            throw new ArgumentNullException(nameof(pluginType));

        try
        {
            _logger.LogInformation("Creating legacy plugin adapter for type: {TypeName}", pluginType.FullName);

            return new GenericLegacyPluginAdapter(pluginInstance, pluginType, _logger);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to create legacy plugin adapter for type: {TypeName}", pluginType.FullName);
            throw;
        }
    }

    /// <summary>
    /// Validates legacy plugin compatibility
    /// </summary>
    /// <param name="pluginType">Plugin type to validate</param>
    /// <returns>Validation result</returns>
    public PluginValidationResult ValidateLegacyPluginCompatibility(Type pluginType)
    {
        var result = new PluginValidationResult();

        try
        {
            if (pluginType == null)
            {
                result.IsValid = false;
                result.Errors.Add("Plugin type is null");
                result.Severity = ValidationSeverity.Error;
                return result;
            }

            _logger.LogDebug("Validating legacy plugin compatibility for type: {TypeName}", pluginType.FullName);

            // Check if type is abstract or interface
            if (pluginType.IsAbstract || pluginType.IsInterface)
            {
                result.Errors.Add("Plugin type cannot be abstract or interface");
                result.Severity = ValidationSeverity.Error;
            }

            // Check for parameterless constructor
            var constructor = pluginType.GetConstructor(Type.EmptyTypes);
            if (constructor == null)
            {
                result.Errors.Add("Plugin type must have a parameterless constructor");
                result.Severity = ValidationSeverity.Error;
            }

            // Check for required properties/methods
            var requiredMembers = new[]
            {
                ("Name", typeof(string)),
                ("Version", typeof(Version)),
                ("Description", typeof(string)),
                ("Author", typeof(string))
            };

            foreach (var (memberName, memberType) in requiredMembers)
            {
                var property = pluginType.GetProperty(memberName, BindingFlags.Public | BindingFlags.Instance);
                if (property == null || !memberType.IsAssignableFrom(property.PropertyType))
                {
                    result.Warnings.Add($"Plugin should have a public {memberName} property of type {memberType.Name}");
                }
            }

            // Check for initialization methods
            var initMethods = new[] { "Initialize", "Init", "Load", "Start" };
            var hasInitMethod = initMethods.Any(methodName => 
                pluginType.GetMethod(methodName, BindingFlags.Public | BindingFlags.Instance) != null);

            if (!hasInitMethod)
            {
                result.Warnings.Add("Plugin should have an initialization method (Initialize, Init, Load, or Start)");
            }

            // Check for shutdown methods
            var shutdownMethods = new[] { "Shutdown", "Dispose", "Unload", "Stop" };
            var hasShutdownMethod = shutdownMethods.Any(methodName => 
                pluginType.GetMethod(methodName, BindingFlags.Public | BindingFlags.Instance) != null);

            if (!hasShutdownMethod)
            {
                result.Warnings.Add("Plugin should have a shutdown method (Shutdown, Dispose, Unload, or Stop)");
            }

            result.IsValid = !result.Errors.Any();
            
            if (result.IsValid)
            {
                result.Message = "Legacy plugin compatibility validation passed";
                result.Severity = result.Warnings.Any() ? ValidationSeverity.Warning : ValidationSeverity.Info;
            }
            else
            {
                result.Message = "Legacy plugin compatibility validation failed";
                result.Severity = ValidationSeverity.Error;
            }

            _logger.LogDebug("Legacy plugin validation completed for {TypeName}: {IsValid}", 
                pluginType.FullName, result.IsValid);
        }
        catch (Exception ex)
        {
            result.IsValid = false;
            result.Errors.Add($"Validation error: {ex.Message}");
            result.Severity = ValidationSeverity.Critical;
            
            _logger.LogError(ex, "Error validating legacy plugin compatibility for type: {TypeName}", 
                pluginType?.FullName ?? "Unknown");
        }

        return result;
    }

    /// <summary>
    /// Gets all wrapped legacy plugins
    /// </summary>
    /// <returns>Collection of wrapped legacy plugins</returns>
    public IEnumerable<IPlugin> GetWrappedLegacyPlugins()
    {
        return _legacyPlugins.Values.Cast<IPlugin>().ToList();
    }

    /// <summary>
    /// Gets legacy plugin statistics
    /// </summary>
    /// <returns>Legacy plugin statistics</returns>
    public LegacyPluginStatistics GetLegacyPluginStatistics()
    {
        return new LegacyPluginStatistics
        {
            TotalLegacyPlugins = _legacyPlugins.Count,
            WrappedPlugins = _legacyPlugins.Values.Count(p => p.IsLoaded),
            PluginTypesByInterface = _legacyPluginTypes.Values
                .GroupBy(type => type.GetInterfaces().FirstOrDefault()?.Name ?? "Unknown")
                .ToDictionary(g => g.Key, g => g.Count())
        };
    }

    /// <summary>
    /// Gets a unique key for a legacy plugin
    /// </summary>
    /// <param name="legacyPlugin">Legacy plugin instance</param>
    /// <returns>Plugin key</returns>
    private static string GetPluginKey(ILegacyPlugin legacyPlugin)
    {
        return $"{legacyPlugin.Name}_{legacyPlugin.Version}_{legacyPlugin.GetHashCode()}";
    }

    /// <summary>
    /// Disposes the compatibility layer
    /// </summary>
    public void Dispose()
    {
        if (_disposed)
            return;

        try
        {
            _logger.LogInformation("Disposing legacy plugin compatibility layer");

            // Dispose all wrapped plugins
            foreach (var wrapper in _legacyPlugins.Values)
            {
                try
                {
                    wrapper.Dispose();
                }
                catch (Exception ex)
                {
                    _logger.LogWarning(ex, "Error disposing legacy plugin wrapper: {PluginName}", wrapper.Name);
                }
            }

            _legacyPlugins.Clear();
            _legacyPluginTypes.Clear();

            _logger.LogInformation("Legacy plugin compatibility layer disposed");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error during legacy plugin compatibility layer disposal");
        }

        _disposed = true;
    }
}

/// <summary>
/// Interface for legacy plugin compatibility layer
/// </summary>
public interface ILegacyPluginCompatibilityLayer
{
    /// <summary>
    /// Wraps a legacy plugin to work with the modern plugin system
    /// </summary>
    /// <param name="legacyPlugin">Legacy plugin instance</param>
    /// <param name="assemblyPath">Path to the plugin assembly</param>
    /// <returns>Modern plugin wrapper</returns>
    IPlugin WrapLegacyPlugin(ILegacyPlugin legacyPlugin, string assemblyPath);

    /// <summary>
    /// Detects if a type is a legacy plugin
    /// </summary>
    /// <param name="type">Type to check</param>
    /// <returns>True if type is a legacy plugin</returns>
    bool IsLegacyPlugin(Type type);

    /// <summary>
    /// Creates a legacy plugin adapter for types that don't implement ILegacyPlugin
    /// </summary>
    /// <param name="pluginInstance">Plugin instance</param>
    /// <param name="pluginType">Plugin type</param>
    /// <returns>Legacy plugin adapter</returns>
    ILegacyPlugin CreateLegacyPluginAdapter(object pluginInstance, Type pluginType);

    /// <summary>
    /// Validates legacy plugin compatibility
    /// </summary>
    /// <param name="pluginType">Plugin type to validate</param>
    /// <returns>Validation result</returns>
    PluginValidationResult ValidateLegacyPluginCompatibility(Type pluginType);

    /// <summary>
    /// Gets all wrapped legacy plugins
    /// </summary>
    /// <returns>Collection of wrapped legacy plugins</returns>
    IEnumerable<IPlugin> GetWrappedLegacyPlugins();

    /// <summary>
    /// Gets legacy plugin statistics
    /// </summary>
    /// <returns>Legacy plugin statistics</returns>
    LegacyPluginStatistics GetLegacyPluginStatistics();
}

/// <summary>
/// Enhanced legacy plugin wrapper with modern features
/// </summary>
public class LegacyPluginWrapper : IPlugin
{
    private readonly ILegacyPlugin _legacyPlugin;
    private readonly string _assemblyPath;
    private readonly ILogger _logger;
    private bool _isLoaded;
    private bool _disposed;

    /// <summary>
    /// Initializes a new instance of the LegacyPluginWrapper class
    /// </summary>
    /// <param name="legacyPlugin">Legacy plugin instance</param>
    /// <param name="assemblyPath">Assembly path</param>
    /// <param name="logger">Logger instance</param>
    public LegacyPluginWrapper(ILegacyPlugin legacyPlugin, string assemblyPath, ILogger logger)
    {
        _legacyPlugin = legacyPlugin ?? throw new ArgumentNullException(nameof(legacyPlugin));
        _assemblyPath = assemblyPath ?? throw new ArgumentNullException(nameof(assemblyPath));
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));

        // Create metadata for legacy plugin
        Metadata = new PluginMetadata
        {
            Name = _legacyPlugin.Name,
            Version = _legacyPlugin.Version,
            Description = _legacyPlugin.Description,
            Author = _legacyPlugin.Author,
            IsLegacy = true,
            Category = PluginCategory.General,
            FilePath = assemblyPath,
            SupportedHostVersions = new List<Version> { new(1, 0, 0) }, // Default compatibility
            Dependencies = new List<PluginDependency>(), // Legacy plugins typically have no declared dependencies
            CreatedAt = File.GetCreationTime(assemblyPath)
        };
    }

    /// <inheritdoc />
    public string Name => _legacyPlugin.Name;

    /// <inheritdoc />
    public Version Version => _legacyPlugin.Version;

    /// <inheritdoc />
    public string Description => _legacyPlugin.Description;

    /// <inheritdoc />
    public string Author => _legacyPlugin.Author;

    /// <inheritdoc />
    public PluginMetadata Metadata { get; }

    /// <inheritdoc />
    public bool IsLoaded => _isLoaded;

    /// <inheritdoc />
    public bool IsCompatible => true; // Legacy plugins are assumed compatible

    /// <inheritdoc />
    public event PropertyChangedEventHandler? PropertyChanged;

    /// <inheritdoc />
    public async Task InitializeAsync(ILogger? logger = null, CancellationToken cancellationToken = default)
    {
        try
        {
            await Task.Run(() =>
            {
                cancellationToken.ThrowIfCancellationRequested();
                _legacyPlugin.Initialize();
                _isLoaded = true;
                OnPropertyChanged(nameof(IsLoaded));
            }, cancellationToken);

            _logger.LogInformation("Legacy plugin initialized: {PluginName}", Name);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to initialize legacy plugin: {PluginName}", Name);
            throw;
        }
    }

    /// <inheritdoc />
    public async Task ShutdownAsync(CancellationToken cancellationToken = default)
    {
        try
        {
            await Task.Run(() =>
            {
                cancellationToken.ThrowIfCancellationRequested();
                _legacyPlugin.Shutdown();
                _isLoaded = false;
                OnPropertyChanged(nameof(IsLoaded));
            }, cancellationToken);

            _logger.LogInformation("Legacy plugin shutdown: {PluginName}", Name);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to shutdown legacy plugin: {PluginName}", Name);
            throw;
        }
    }

    /// <inheritdoc />
    public Task<PluginValidationResult> ValidateCompatibilityAsync(Version hostVersion, CancellationToken cancellationToken = default)
    {
        return Task.FromResult(new PluginValidationResult
        {
            IsValid = true,
            Message = "Legacy plugin - compatibility assumed",
            Severity = ValidationSeverity.Info
        });
    }

    /// <inheritdoc />
    public PluginConfigurationSchema? GetConfigurationSchema() => null; // Legacy plugins don't support configuration

    /// <inheritdoc />
    public Task ConfigureAsync(PluginConfiguration configuration, CancellationToken cancellationToken = default)
    {
        return Task.CompletedTask; // Legacy plugins don't support configuration
    }

    /// <summary>
    /// Raises the PropertyChanged event
    /// </summary>
    /// <param name="propertyName">Property name</param>
    protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string? propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }

    /// <inheritdoc />
    public void Dispose()
    {
        if (_disposed)
            return;

        try
        {
            if (_isLoaded)
            {
                _legacyPlugin.Shutdown();
                _isLoaded = false;
            }

            if (_legacyPlugin is IDisposable disposableLegacyPlugin)
            {
                disposableLegacyPlugin.Dispose();
            }
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "Error disposing legacy plugin: {PluginName}", Name);
        }

        _disposed = true;
    }
}

/// <summary>
/// Generic adapter for legacy plugins that don't implement ILegacyPlugin
/// </summary>
public class GenericLegacyPluginAdapter : ILegacyPlugin, IDisposable
{
    private readonly object _pluginInstance;
    private readonly Type _pluginType;
    private readonly ILogger _logger;
    private readonly MethodInfo? _initializeMethod;
    private readonly MethodInfo? _shutdownMethod;

    /// <summary>
    /// Initializes a new instance of the GenericLegacyPluginAdapter class
    /// </summary>
    /// <param name="pluginInstance">Plugin instance</param>
    /// <param name="pluginType">Plugin type</param>
    /// <param name="logger">Logger instance</param>
    public GenericLegacyPluginAdapter(object pluginInstance, Type pluginType, ILogger logger)
    {
        _pluginInstance = pluginInstance ?? throw new ArgumentNullException(nameof(pluginInstance));
        _pluginType = pluginType ?? throw new ArgumentNullException(nameof(pluginType));
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));

        // Extract plugin information using reflection
        Name = GetPropertyValue<string>("Name") ?? _pluginType.Name;
        Version = GetPropertyValue<Version>("Version") ?? new Version(1, 0, 0);
        Description = GetPropertyValue<string>("Description") ?? "Legacy plugin";
        Author = GetPropertyValue<string>("Author") ?? "Unknown";

        // Find initialization and shutdown methods
        var initMethods = new[] { "Initialize", "Init", "Load", "Start" };
        _initializeMethod = initMethods
            .Select(name => _pluginType.GetMethod(name, BindingFlags.Public | BindingFlags.Instance))
            .FirstOrDefault(method => method != null);

        var shutdownMethods = new[] { "Shutdown", "Dispose", "Unload", "Stop" };
        _shutdownMethod = shutdownMethods
            .Select(name => _pluginType.GetMethod(name, BindingFlags.Public | BindingFlags.Instance))
            .FirstOrDefault(method => method != null);
    }

    /// <inheritdoc />
    public string Name { get; }

    /// <inheritdoc />
    public Version Version { get; }

    /// <inheritdoc />
    public string Description { get; }

    /// <inheritdoc />
    public string Author { get; }

    /// <inheritdoc />
    public void Initialize()
    {
        try
        {
            _initializeMethod?.Invoke(_pluginInstance, null);
            _logger.LogDebug("Generic legacy plugin initialized: {PluginName}", Name);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to initialize generic legacy plugin: {PluginName}", Name);
            throw;
        }
    }

    /// <inheritdoc />
    public void Shutdown()
    {
        try
        {
            _shutdownMethod?.Invoke(_pluginInstance, null);
            _logger.LogDebug("Generic legacy plugin shutdown: {PluginName}", Name);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to shutdown generic legacy plugin: {PluginName}", Name);
            throw;
        }
    }

    /// <summary>
    /// Gets a property value using reflection
    /// </summary>
    /// <typeparam name="T">Property type</typeparam>
    /// <param name="propertyName">Property name</param>
    /// <returns>Property value or default</returns>
    private T? GetPropertyValue<T>(string propertyName)
    {
        try
        {
            var property = _pluginType.GetProperty(propertyName, BindingFlags.Public | BindingFlags.Instance);
            if (property != null && typeof(T).IsAssignableFrom(property.PropertyType))
            {
                return (T?)property.GetValue(_pluginInstance);
            }
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "Failed to get property {PropertyName} from plugin {PluginName}", 
                propertyName, _pluginType.Name);
        }

        return default;
    }

    /// <inheritdoc />
    public void Dispose()
    {
        try
        {
            Shutdown();

            if (_pluginInstance is IDisposable disposableInstance)
            {
                disposableInstance.Dispose();
            }
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "Error disposing generic legacy plugin: {PluginName}", Name);
        }
    }
}

/// <summary>
/// Statistics for legacy plugins
/// </summary>
public class LegacyPluginStatistics
{
    /// <summary>
    /// Total number of legacy plugins
    /// </summary>
    public int TotalLegacyPlugins { get; set; }

    /// <summary>
    /// Number of wrapped plugins
    /// </summary>
    public int WrappedPlugins { get; set; }

    /// <summary>
    /// Plugin types by interface
    /// </summary>
    public Dictionary<string, int> PluginTypesByInterface { get; set; } = new();
}