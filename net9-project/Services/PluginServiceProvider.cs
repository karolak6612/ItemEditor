using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using PluginInterface;
using ItemEditor.Models;
using System.Collections.Concurrent;

namespace ItemEditor.Services;

/// <summary>
/// Service provider for plugin dependency injection integration
/// </summary>
public class PluginServiceProvider : IPluginServiceProvider, IDisposable
{
    private readonly IServiceProvider _hostServiceProvider;
    private readonly ILogger<PluginServiceProvider> _logger;
    private readonly ConcurrentDictionary<string, IServiceScope> _pluginScopes = new();
    private readonly ConcurrentDictionary<string, IServiceProvider> _pluginServiceProviders = new();
    private readonly object _lock = new();
    private bool _disposed;

    /// <summary>
    /// Initializes a new instance of the PluginServiceProvider class
    /// </summary>
    /// <param name="hostServiceProvider">Host application service provider</param>
    /// <param name="logger">Logger instance</param>
    public PluginServiceProvider(IServiceProvider hostServiceProvider, ILogger<PluginServiceProvider> logger)
    {
        _hostServiceProvider = hostServiceProvider ?? throw new ArgumentNullException(nameof(hostServiceProvider));
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
    }

    /// <summary>
    /// Creates a service scope for a plugin
    /// </summary>
    /// <param name="plugin">Plugin instance</param>
    /// <returns>Service provider for the plugin</returns>
    public IServiceProvider CreatePluginServiceProvider(IPlugin plugin)
    {
        if (plugin == null)
            throw new ArgumentNullException(nameof(plugin));

        var pluginKey = GetPluginKey(plugin);

        if (_pluginServiceProviders.TryGetValue(pluginKey, out var existingProvider))
        {
            _logger.LogDebug("Returning existing service provider for plugin: {PluginName}", plugin.Name);
            return existingProvider;
        }

        lock (_lock)
        {
            // Double-check pattern
            if (_pluginServiceProviders.TryGetValue(pluginKey, out existingProvider))
            {
                return existingProvider;
            }

            try
            {
                _logger.LogInformation("Creating service provider for plugin: {PluginName}", plugin.Name);

                // Create a new service collection for the plugin
                var services = new ServiceCollection();

                // Add host services that plugins can access
                RegisterHostServices(services);

                // Add plugin-specific services
                RegisterPluginServices(services, plugin);

                // Register the plugin itself
                services.AddSingleton(plugin);
                services.AddSingleton<IPlugin>(plugin);

                // Create service provider
                var serviceProvider = services.BuildServiceProvider();

                // Create scope for proper disposal
                var scope = serviceProvider.CreateScope();
                _pluginScopes.TryAdd(pluginKey, scope);
                _pluginServiceProviders.TryAdd(pluginKey, scope.ServiceProvider);

                _logger.LogInformation("Successfully created service provider for plugin: {PluginName}", plugin.Name);
                return scope.ServiceProvider;
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to create service provider for plugin: {PluginName}", plugin.Name);
                throw;
            }
        }
    }

    /// <summary>
    /// Gets the service provider for a plugin
    /// </summary>
    /// <param name="plugin">Plugin instance</param>
    /// <returns>Service provider or null if not found</returns>
    public IServiceProvider? GetPluginServiceProvider(IPlugin plugin)
    {
        if (plugin == null)
            return null;

        var pluginKey = GetPluginKey(plugin);
        _pluginServiceProviders.TryGetValue(pluginKey, out var serviceProvider);
        return serviceProvider;
    }

    /// <summary>
    /// Removes the service provider for a plugin
    /// </summary>
    /// <param name="plugin">Plugin instance</param>
    /// <returns>True if removed successfully</returns>
    public bool RemovePluginServiceProvider(IPlugin plugin)
    {
        if (plugin == null)
            return false;

        var pluginKey = GetPluginKey(plugin);

        try
        {
            _logger.LogInformation("Removing service provider for plugin: {PluginName}", plugin.Name);

            // Remove and dispose scope
            if (_pluginScopes.TryRemove(pluginKey, out var scope))
            {
                scope.Dispose();
            }

            // Remove service provider
            var removed = _pluginServiceProviders.TryRemove(pluginKey, out _);

            if (removed)
            {
                _logger.LogInformation("Successfully removed service provider for plugin: {PluginName}", plugin.Name);
            }

            return removed;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to remove service provider for plugin: {PluginName}", plugin.Name);
            return false;
        }
    }

    /// <summary>
    /// Gets all plugin service providers
    /// </summary>
    /// <returns>Dictionary of plugin keys to service providers</returns>
    public IReadOnlyDictionary<string, IServiceProvider> GetAllPluginServiceProviders()
    {
        return _pluginServiceProviders.ToDictionary(kvp => kvp.Key, kvp => kvp.Value);
    }

    /// <summary>
    /// Configures plugin services with the host dependency injection
    /// </summary>
    /// <param name="plugin">Plugin instance</param>
    /// <param name="configuration">Plugin configuration</param>
    /// <returns>Task representing the async operation</returns>
    public async Task ConfigurePluginServicesAsync(IPlugin plugin, PluginConfiguration configuration)
    {
        if (plugin == null)
            throw new ArgumentNullException(nameof(plugin));

        try
        {
            _logger.LogInformation("Configuring services for plugin: {PluginName}", plugin.Name);

            var serviceProvider = GetPluginServiceProvider(plugin);
            if (serviceProvider == null)
            {
                _logger.LogWarning("No service provider found for plugin: {PluginName}", plugin.Name);
                return;
            }

            // Configure the plugin with its service provider
            await plugin.ConfigureAsync(configuration);

            _logger.LogInformation("Successfully configured services for plugin: {PluginName}", plugin.Name);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to configure services for plugin: {PluginName}", plugin.Name);
            throw;
        }
    }

    /// <summary>
    /// Registers host services that plugins can access
    /// </summary>
    /// <param name="services">Service collection</param>
    private void RegisterHostServices(IServiceCollection services)
    {
        // Register logging
        services.AddSingleton(typeof(ILogger<>), typeof(Logger<>));
        services.AddSingleton<ILoggerFactory>(_hostServiceProvider.GetRequiredService<ILoggerFactory>());

        // Register host services that plugins might need
        var fileService = _hostServiceProvider.GetService<IFileService>();
        if (fileService != null)
        {
            services.AddSingleton(fileService);
        }

        var imageService = _hostServiceProvider.GetService<IImageService>();
        if (imageService != null)
        {
            services.AddSingleton(imageService);
        }

        // Add other host services as needed
        _logger.LogDebug("Registered host services for plugin");
    }

    /// <summary>
    /// Registers plugin-specific services
    /// </summary>
    /// <param name="services">Service collection</param>
    /// <param name="plugin">Plugin instance</param>
    private void RegisterPluginServices(IServiceCollection services, IPlugin plugin)
    {
        // Register plugin metadata
        services.AddSingleton(plugin.Metadata);

        // Register plugin configuration if available
        var configSchema = plugin.GetConfigurationSchema();
        if (configSchema != null)
        {
            services.AddSingleton(configSchema);
        }

        // Register plugin event system
        services.AddSingleton<IPluginEventSystem, PluginEventSystem>();

        // Register plugin settings manager
        services.AddSingleton<IPluginSettingsManager>(provider => 
            new PluginSettingsManager(plugin.Name, _logger));

        _logger.LogDebug("Registered plugin-specific services for: {PluginName}", plugin.Name);
    }

    /// <summary>
    /// Gets a unique key for a plugin
    /// </summary>
    /// <param name="plugin">Plugin instance</param>
    /// <returns>Plugin key</returns>
    private static string GetPluginKey(IPlugin plugin)
    {
        return $"{plugin.Name}_{plugin.Version}_{plugin.GetHashCode()}";
    }

    /// <summary>
    /// Disposes the plugin service provider
    /// </summary>
    public void Dispose()
    {
        if (_disposed)
            return;

        try
        {
            _logger.LogInformation("Disposing plugin service provider");

            // Dispose all plugin scopes
            foreach (var scope in _pluginScopes.Values)
            {
                try
                {
                    scope.Dispose();
                }
                catch (Exception ex)
                {
                    _logger.LogWarning(ex, "Error disposing plugin scope");
                }
            }

            _pluginScopes.Clear();
            _pluginServiceProviders.Clear();

            _logger.LogInformation("Plugin service provider disposed");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error during plugin service provider disposal");
        }

        _disposed = true;
    }
}

/// <summary>
/// Interface for plugin service provider
/// </summary>
public interface IPluginServiceProvider
{
    /// <summary>
    /// Creates a service provider for a plugin
    /// </summary>
    /// <param name="plugin">Plugin instance</param>
    /// <returns>Service provider for the plugin</returns>
    IServiceProvider CreatePluginServiceProvider(IPlugin plugin);

    /// <summary>
    /// Gets the service provider for a plugin
    /// </summary>
    /// <param name="plugin">Plugin instance</param>
    /// <returns>Service provider or null if not found</returns>
    IServiceProvider? GetPluginServiceProvider(IPlugin plugin);

    /// <summary>
    /// Removes the service provider for a plugin
    /// </summary>
    /// <param name="plugin">Plugin instance</param>
    /// <returns>True if removed successfully</returns>
    bool RemovePluginServiceProvider(IPlugin plugin);

    /// <summary>
    /// Configures plugin services with the host dependency injection
    /// </summary>
    /// <param name="plugin">Plugin instance</param>
    /// <param name="configuration">Plugin configuration</param>
    /// <returns>Task representing the async operation</returns>
    Task ConfigurePluginServicesAsync(IPlugin plugin, PluginConfiguration configuration);
}