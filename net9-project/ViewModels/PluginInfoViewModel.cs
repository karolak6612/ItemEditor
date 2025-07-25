using System.Collections.ObjectModel;
using System.Windows.Media;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Microsoft.Extensions.Logging;
using ItemEditor.Models;
using ItemEditor.Services;
using PluginInterface;

namespace ItemEditor.ViewModels;

/// <summary>
/// ViewModel for the Plugin Information window
/// </summary>
public partial class PluginInfoViewModel : ObservableObject
{
    private readonly IPluginService _pluginService;
    private readonly ILogger<PluginInfoViewModel> _logger;

    [ObservableProperty]
    private string _name = string.Empty;

    [ObservableProperty]
    private string _description = string.Empty;

    [ObservableProperty]
    private string _version = string.Empty;

    [ObservableProperty]
    private string _author = string.Empty;

    [ObservableProperty]
    private string _category = string.Empty;

    [ObservableProperty]
    private string _pluginType = string.Empty;

    [ObservableProperty]
    private string _filePath = string.Empty;

    [ObservableProperty]
    private DateTime _loadedAt;

    [ObservableProperty]
    private string _statusText = string.Empty;

    [ObservableProperty]
    private Color _statusColor = Colors.Gray;

    [ObservableProperty]
    private bool _hasError;

    [ObservableProperty]
    private string _errorMessage = string.Empty;

    [ObservableProperty]
    private bool _hasConfiguration;

    [ObservableProperty]
    private bool _canReload;

    [ObservableProperty]
    private ObservableCollection<string> _supportedClients = new();

    [ObservableProperty]
    private ObservableCollection<DependencyViewModel> _dependencies = new();

    /// <summary>
    /// Initializes a new instance of the PluginInfoViewModel class
    /// </summary>
    /// <param name="pluginInfo">Plugin information</param>
    /// <param name="loadedPlugin">Loaded plugin instance</param>
    /// <param name="pluginService">Plugin service</param>
    /// <param name="logger">Logger instance</param>
    public PluginInfoViewModel(
        PluginInfo pluginInfo, 
        IPlugin? loadedPlugin,
        IPluginService pluginService,
        ILogger<PluginInfoViewModel> logger)
    {
        _pluginService = pluginService ?? throw new ArgumentNullException(nameof(pluginService));
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));

        LoadedPlugin = loadedPlugin;
        
        // Initialize commands
        ConfigureCommand = new AsyncRelayCommand(ConfigurePluginAsync);
        ReloadCommand = new AsyncRelayCommand(ReloadPluginAsync);
        CloseCommand = new RelayCommand(Close);

        // Load plugin information
        LoadPluginInfo(pluginInfo, loadedPlugin);
    }

    /// <summary>
    /// Gets the loaded plugin instance
    /// </summary>
    public IPlugin? LoadedPlugin { get; }

    /// <summary>
    /// Gets whether the plugin has supported clients
    /// </summary>
    public bool HasSupportedClients => SupportedClients.Any();

    /// <summary>
    /// Gets whether the plugin has dependencies
    /// </summary>
    public bool HasDependencies => Dependencies.Any();

    /// <summary>
    /// Command to configure the plugin
    /// </summary>
    public IAsyncRelayCommand ConfigureCommand { get; }

    /// <summary>
    /// Command to reload the plugin
    /// </summary>
    public IAsyncRelayCommand ReloadCommand { get; }

    /// <summary>
    /// Command to close the window
    /// </summary>
    public IRelayCommand CloseCommand { get; }

    /// <summary>
    /// Event raised when the window should be closed
    /// </summary>
    public event EventHandler? CloseRequested;

    /// <summary>
    /// Loads plugin information into the view model
    /// </summary>
    /// <param name="pluginInfo">Plugin information</param>
    /// <param name="loadedPlugin">Loaded plugin instance</param>
    private void LoadPluginInfo(PluginInfo pluginInfo, IPlugin? loadedPlugin)
    {
        Name = pluginInfo.Name;
        Description = pluginInfo.Description;
        Version = pluginInfo.Version;
        Author = pluginInfo.Author;
        FilePath = pluginInfo.FilePath;
        LoadedAt = pluginInfo.LoadedAt;

        // Load information from loaded plugin if available
        if (loadedPlugin != null)
        {
            Category = loadedPlugin.Metadata.Category.ToString();
            PluginType = loadedPlugin.Metadata.IsLegacy ? "Legacy Plugin" : "Modern Plugin";
            HasConfiguration = loadedPlugin.GetConfigurationSchema() != null;
            CanReload = true;

            // Load supported clients
            SupportedClients.Clear();
            foreach (var version in loadedPlugin.Metadata.SupportedHostVersions)
            {
                SupportedClients.Add(version.ToString());
            }

            // Load dependencies
            Dependencies.Clear();
            foreach (var dependency in loadedPlugin.Metadata.Dependencies)
            {
                Dependencies.Add(new DependencyViewModel(dependency));
            }

            // Update status
            if (loadedPlugin.IsLoaded)
            {
                StatusText = loadedPlugin.Metadata.IsLegacy ? "Loaded (Legacy)" : "Loaded";
                StatusColor = Colors.Green;
            }
            else
            {
                StatusText = "Not Loaded";
                StatusColor = Colors.Orange;
            }
        }
        else
        {
            Category = "Unknown";
            PluginType = "Unknown";
            HasConfiguration = false;
            CanReload = false;
            StatusText = "Unloaded";
            StatusColor = Colors.Gray;

            // Load supported clients from plugin info
            SupportedClients.Clear();
            foreach (var client in pluginInfo.SupportedClients)
            {
                SupportedClients.Add(client);
            }
        }

        // Update computed properties
        OnPropertyChanged(nameof(HasSupportedClients));
        OnPropertyChanged(nameof(HasDependencies));
    }

    /// <summary>
    /// Configures the plugin
    /// </summary>
    private async Task ConfigurePluginAsync()
    {
        if (LoadedPlugin == null) return;

        try
        {
            _logger.LogInformation("Opening configuration for plugin: {PluginName}", Name);
            
            // Implementation would open plugin configuration dialog
            // For now, just log the action
            _logger.LogInformation("Plugin configuration opened for: {PluginName}", Name);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to configure plugin: {PluginName}", Name);
            
            HasError = true;
            ErrorMessage = ex.Message;
            StatusText = "Configuration Error";
            StatusColor = Colors.Red;
        }
    }

    /// <summary>
    /// Reloads the plugin
    /// </summary>
    private async Task ReloadPluginAsync()
    {
        if (LoadedPlugin == null) return;

        try
        {
            _logger.LogInformation("Reloading plugin: {PluginName}", Name);
            
            var reloadedPlugin = await _pluginService.ReloadPluginAsync(LoadedPlugin);
            
            if (reloadedPlugin != null)
            {
                _logger.LogInformation("Successfully reloaded plugin: {PluginName}", Name);
                StatusText = "Reloaded";
                StatusColor = Colors.Green;
            }
            else
            {
                _logger.LogWarning("Failed to reload plugin: {PluginName}", Name);
                HasError = true;
                ErrorMessage = "Plugin reload failed";
                StatusText = "Reload Failed";
                StatusColor = Colors.Red;
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error reloading plugin: {PluginName}", Name);
            
            HasError = true;
            ErrorMessage = ex.Message;
            StatusText = "Reload Error";
            StatusColor = Colors.Red;
        }
    }

    /// <summary>
    /// Closes the window
    /// </summary>
    private void Close()
    {
        CloseRequested?.Invoke(this, EventArgs.Empty);
    }
}

/// <summary>
/// ViewModel for plugin dependencies
/// </summary>
public class DependencyViewModel
{
    /// <summary>
    /// Initializes a new instance of the DependencyViewModel class
    /// </summary>
    /// <param name="dependency">Plugin dependency</param>
    public DependencyViewModel(PluginDependency dependency)
    {
        Name = dependency.Name;
        Type = dependency.Type.ToString();
        
        // Format version range
        if (dependency.MaxVersion != null)
        {
            VersionRange = $"{dependency.MinVersion} - {dependency.MaxVersion}";
        }
        else
        {
            VersionRange = $">= {dependency.MinVersion}";
        }
        
        if (dependency.IsOptional)
        {
            VersionRange += " (Optional)";
        }
    }

    /// <summary>
    /// Gets the dependency name
    /// </summary>
    public string Name { get; }

    /// <summary>
    /// Gets the dependency type
    /// </summary>
    public string Type { get; }

    /// <summary>
    /// Gets the version range
    /// </summary>
    public string VersionRange { get; }
}