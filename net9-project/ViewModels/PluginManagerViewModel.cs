using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Windows.Data;
using System.Windows.Media;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Microsoft.Extensions.Logging;
using Microsoft.Win32;
using ItemEditor.Models;
using ItemEditor.Services;
using PluginInterface;

namespace ItemEditor.ViewModels;

/// <summary>
/// ViewModel for the Plugin Manager window
/// </summary>
public partial class PluginManagerViewModel : ObservableObject
{
    private readonly IPluginService _pluginService;
    private readonly ILogger<PluginManagerViewModel> _logger;
    private readonly ICollectionView _filteredPluginsView;

    [ObservableProperty]
    private ObservableCollection<PluginItemViewModel> _plugins = new();

    [ObservableProperty]
    private string _searchText = string.Empty;

    [ObservableProperty]
    private string _selectedCategoryFilter = "All Categories";

    [ObservableProperty]
    private string _selectedStatusFilter = "All Status";

    [ObservableProperty]
    private bool _isLoading;

    [ObservableProperty]
    private string _loadingMessage = string.Empty;

    [ObservableProperty]
    private string _statusMessage = string.Empty;

    [ObservableProperty]
    private PluginStatistics _statistics = new();

    /// <summary>
    /// Initializes a new instance of the PluginManagerViewModel class
    /// </summary>
    /// <param name="pluginService">Plugin service</param>
    /// <param name="logger">Logger instance</param>
    public PluginManagerViewModel(IPluginService pluginService, ILogger<PluginManagerViewModel> logger)
    {
        _pluginService = pluginService ?? throw new ArgumentNullException(nameof(pluginService));
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));

        // Set up filtered view
        _filteredPluginsView = CollectionViewSource.GetDefaultView(Plugins);
        _filteredPluginsView.Filter = FilterPlugins;

        // Subscribe to property changes for filtering
        PropertyChanged += OnPropertyChanged;

        // Subscribe to plugin service events
        _pluginService.PluginLoaded += OnPluginLoaded;
        _pluginService.PluginUnloaded += OnPluginUnloaded;
        _pluginService.PluginError += OnPluginError;

        // Initialize commands
        InstallPluginCommand = new AsyncRelayCommand(InstallPluginAsync);
        RefreshPluginsCommand = new AsyncRelayCommand(RefreshPluginsAsync);
        OpenSettingsCommand = new RelayCommand(OpenSettings);
        TogglePluginCommand = new AsyncRelayCommand<PluginItemViewModel>(TogglePluginAsync);
        ConfigurePluginCommand = new AsyncRelayCommand<PluginItemViewModel>(ConfigurePluginAsync);
        RemovePluginCommand = new AsyncRelayCommand<PluginItemViewModel>(RemovePluginAsync);
        ShowPluginInfoCommand = new RelayCommand<PluginItemViewModel>(ShowPluginInfo);
        CloseCommand = new RelayCommand(Close);

        // Initialize data
        _ = Task.Run(LoadPluginsAsync);
    }

    /// <summary>
    /// Gets the filtered plugins view
    /// </summary>
    public ICollectionView FilteredPlugins => _filteredPluginsView;

    /// <summary>
    /// Gets whether the plugin list is empty
    /// </summary>
    public bool IsEmpty => !Plugins.Any();

    /// <summary>
    /// Gets the available category filters
    /// </summary>
    public ObservableCollection<string> CategoryFilters { get; } = new()
    {
        "All Categories",
        "General",
        "File Format",
        "Image Processing",
        "Data Conversion",
        "UI Enhancement",
        "Development Tools"
    };

    /// <summary>
    /// Gets the available status filters
    /// </summary>
    public ObservableCollection<string> StatusFilters { get; } = new()
    {
        "All Status",
        "Loaded",
        "Unloaded",
        "Error",
        "Legacy"
    };

    /// <summary>
    /// Command to install a new plugin
    /// </summary>
    public IAsyncRelayCommand InstallPluginCommand { get; }

    /// <summary>
    /// Command to refresh the plugin list
    /// </summary>
    public IAsyncRelayCommand RefreshPluginsCommand { get; }

    /// <summary>
    /// Command to open plugin settings
    /// </summary>
    public IRelayCommand OpenSettingsCommand { get; }

    /// <summary>
    /// Command to toggle plugin enabled/disabled state
    /// </summary>
    public IAsyncRelayCommand<PluginItemViewModel> TogglePluginCommand { get; }

    /// <summary>
    /// Command to configure a plugin
    /// </summary>
    public IAsyncRelayCommand<PluginItemViewModel> ConfigurePluginCommand { get; }

    /// <summary>
    /// Command to remove a plugin
    /// </summary>
    public IAsyncRelayCommand<PluginItemViewModel> RemovePluginCommand { get; }

    /// <summary>
    /// Command to show plugin information
    /// </summary>
    public IRelayCommand<PluginItemViewModel> ShowPluginInfoCommand { get; }

    /// <summary>
    /// Command to close the window
    /// </summary>
    public IRelayCommand CloseCommand { get; }

    /// <summary>
    /// Event raised when the window should be closed
    /// </summary>
    public event EventHandler? CloseRequested;

    /// <summary>
    /// Loads plugins from the plugin service
    /// </summary>
    private async Task LoadPluginsAsync()
    {
        try
        {
            IsLoading = true;
            LoadingMessage = "Loading plugins...";

            var pluginInfos = await _pluginService.GetInstalledPluginsAsync();
            var loadedPlugins = _pluginService.GetLoadedPlugins();

            await Application.Current.Dispatcher.InvokeAsync(() =>
            {
                Plugins.Clear();

                foreach (var pluginInfo in pluginInfos)
                {
                    var loadedPlugin = loadedPlugins.FirstOrDefault(p => p.Name == pluginInfo.Name);
                    var pluginViewModel = new PluginItemViewModel(pluginInfo, loadedPlugin);
                    Plugins.Add(pluginViewModel);
                }

                UpdateStatistics();
                UpdateStatusMessage();
                OnPropertyChanged(nameof(IsEmpty));
            });
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to load plugins");
            StatusMessage = "Failed to load plugins";
        }
        finally
        {
            IsLoading = false;
            LoadingMessage = string.Empty;
        }
    }

    /// <summary>
    /// Installs a new plugin
    /// </summary>
    private async Task InstallPluginAsync()
    {
        try
        {
            var openFileDialog = new OpenFileDialog
            {
                Title = "Select Plugin File",
                Filter = "Plugin Files (*.dll)|*.dll|All Files (*.*)|*.*",
                Multiselect = false
            };

            if (openFileDialog.ShowDialog() == true)
            {
                IsLoading = true;
                LoadingMessage = "Installing plugin...";

                await _pluginService.InstallPluginAsync(openFileDialog.FileName);
                await RefreshPluginsAsync();

                StatusMessage = "Plugin installed successfully";
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to install plugin");
            StatusMessage = "Failed to install plugin";
        }
        finally
        {
            IsLoading = false;
            LoadingMessage = string.Empty;
        }
    }

    /// <summary>
    /// Refreshes the plugin list
    /// </summary>
    private async Task RefreshPluginsAsync()
    {
        await LoadPluginsAsync();
    }

    /// <summary>
    /// Opens plugin settings
    /// </summary>
    private void OpenSettings()
    {
        // Implementation would open plugin settings dialog
        _logger.LogInformation("Opening plugin settings");
    }

    /// <summary>
    /// Toggles plugin enabled/disabled state
    /// </summary>
    /// <param name="pluginViewModel">Plugin to toggle</param>
    private async Task TogglePluginAsync(PluginItemViewModel? pluginViewModel)
    {
        if (pluginViewModel == null) return;

        try
        {
            IsLoading = true;
            LoadingMessage = pluginViewModel.IsEnabled ? "Disabling plugin..." : "Enabling plugin...";

            if (pluginViewModel.IsEnabled)
            {
                if (pluginViewModel.LoadedPlugin != null)
                {
                    await _pluginService.UnloadPluginAsync(pluginViewModel.LoadedPlugin);
                }
                else
                {
                    _pluginService.DisablePlugin(pluginViewModel.PluginInfo);
                }
            }
            else
            {
                if (pluginViewModel.LoadedPlugin == null)
                {
                    var plugin = await _pluginService.LoadPluginAsync(pluginViewModel.PluginInfo.FilePath);
                    pluginViewModel.LoadedPlugin = plugin;
                }
                else
                {
                    _pluginService.EnablePlugin(pluginViewModel.PluginInfo);
                }
            }

            pluginViewModel.UpdateStatus();
            UpdateStatistics();
            StatusMessage = $"Plugin {(pluginViewModel.IsEnabled ? "enabled" : "disabled")} successfully";
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to toggle plugin: {PluginName}", pluginViewModel.Name);
            StatusMessage = "Failed to toggle plugin";
        }
        finally
        {
            IsLoading = false;
            LoadingMessage = string.Empty;
        }
    }

    /// <summary>
    /// Configures a plugin
    /// </summary>
    /// <param name="pluginViewModel">Plugin to configure</param>
    private async Task ConfigurePluginAsync(PluginItemViewModel? pluginViewModel)
    {
        if (pluginViewModel?.LoadedPlugin == null) return;

        try
        {
            _logger.LogInformation("Configuring plugin: {PluginName}", pluginViewModel.Name);
            
            // Implementation would open plugin configuration dialog
            StatusMessage = "Plugin configuration opened";
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to configure plugin: {PluginName}", pluginViewModel.Name);
            StatusMessage = "Failed to configure plugin";
        }
    }

    /// <summary>
    /// Removes a plugin
    /// </summary>
    /// <param name="pluginViewModel">Plugin to remove</param>
    private async Task RemovePluginAsync(PluginItemViewModel? pluginViewModel)
    {
        if (pluginViewModel == null) return;

        try
        {
            // Show confirmation dialog
            var result = System.Windows.MessageBox.Show(
                $"Are you sure you want to remove the plugin '{pluginViewModel.Name}'?",
                "Confirm Plugin Removal",
                System.Windows.MessageBoxButton.YesNo,
                System.Windows.MessageBoxImage.Question);

            if (result == System.Windows.MessageBoxResult.Yes)
            {
                IsLoading = true;
                LoadingMessage = "Removing plugin...";

                await _pluginService.RemovePluginAsync(pluginViewModel.PluginInfo);
                Plugins.Remove(pluginViewModel);

                UpdateStatistics();
                OnPropertyChanged(nameof(IsEmpty));
                StatusMessage = "Plugin removed successfully";
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to remove plugin: {PluginName}", pluginViewModel.Name);
            StatusMessage = "Failed to remove plugin";
        }
        finally
        {
            IsLoading = false;
            LoadingMessage = string.Empty;
        }
    }

    /// <summary>
    /// Shows plugin information
    /// </summary>
    /// <param name="pluginViewModel">Plugin to show info for</param>
    private void ShowPluginInfo(PluginItemViewModel? pluginViewModel)
    {
        if (pluginViewModel == null) return;

        _logger.LogInformation("Showing plugin info: {PluginName}", pluginViewModel.Name);
        
        // Implementation would show plugin info dialog
        StatusMessage = "Plugin information displayed";
    }

    /// <summary>
    /// Closes the window
    /// </summary>
    private void Close()
    {
        CloseRequested?.Invoke(this, EventArgs.Empty);
    }

    /// <summary>
    /// Filters plugins based on search text and filters
    /// </summary>
    /// <param name="item">Plugin item to filter</param>
    /// <returns>True if item should be shown</returns>
    private bool FilterPlugins(object item)
    {
        if (item is not PluginItemViewModel plugin) return false;

        // Search text filter
        if (!string.IsNullOrEmpty(SearchText))
        {
            var searchLower = SearchText.ToLowerInvariant();
            if (!plugin.Name.ToLowerInvariant().Contains(searchLower) &&
                !plugin.Description.ToLowerInvariant().Contains(searchLower) &&
                !plugin.Author.ToLowerInvariant().Contains(searchLower))
            {
                return false;
            }
        }

        // Category filter
        if (SelectedCategoryFilter != "All Categories")
        {
            if (plugin.Category != SelectedCategoryFilter)
            {
                return false;
            }
        }

        // Status filter
        if (SelectedStatusFilter != "All Status")
        {
            var matchesStatus = SelectedStatusFilter switch
            {
                "Loaded" => plugin.IsEnabled,
                "Unloaded" => !plugin.IsEnabled,
                "Error" => plugin.HasError,
                "Legacy" => plugin.IsLegacy,
                _ => true
            };

            if (!matchesStatus)
            {
                return false;
            }
        }

        return true;
    }

    /// <summary>
    /// Updates plugin statistics
    /// </summary>
    private void UpdateStatistics()
    {
        Statistics = _pluginService.GetPluginStatistics();
    }

    /// <summary>
    /// Updates the status message
    /// </summary>
    private void UpdateStatusMessage()
    {
        StatusMessage = $"Manage your ItemEditor plugins and extensions";
    }

    /// <summary>
    /// Handles property changes for filtering
    /// </summary>
    private void OnPropertyChanged(object? sender, PropertyChangedEventArgs e)
    {
        if (e.PropertyName is nameof(SearchText) or nameof(SelectedCategoryFilter) or nameof(SelectedStatusFilter))
        {
            _filteredPluginsView.Refresh();
        }
    }

    /// <summary>
    /// Handles plugin loaded events
    /// </summary>
    private void OnPluginLoaded(object? sender, PluginLoadedEventArgs e)
    {
        Application.Current.Dispatcher.InvokeAsync(() =>
        {
            var existingPlugin = Plugins.FirstOrDefault(p => p.Name == e.Plugin.Name);
            if (existingPlugin != null)
            {
                existingPlugin.LoadedPlugin = e.Plugin;
                existingPlugin.UpdateStatus();
            }
            else
            {
                // Add new plugin
                var pluginInfo = new PluginInfo
                {
                    Name = e.Plugin.Name,
                    Description = e.Plugin.Description,
                    Version = e.Plugin.Version.ToString(),
                    Author = e.Plugin.Author,
                    LoadedAt = DateTime.UtcNow,
                    IsEnabled = true
                };
                
                var pluginViewModel = new PluginItemViewModel(pluginInfo, e.Plugin);
                Plugins.Add(pluginViewModel);
            }

            UpdateStatistics();
            OnPropertyChanged(nameof(IsEmpty));
        });
    }

    /// <summary>
    /// Handles plugin unloaded events
    /// </summary>
    private void OnPluginUnloaded(object? sender, PluginUnloadedEventArgs e)
    {
        Application.Current.Dispatcher.InvokeAsync(() =>
        {
            var existingPlugin = Plugins.FirstOrDefault(p => p.Name == e.Plugin.Name);
            if (existingPlugin != null)
            {
                existingPlugin.LoadedPlugin = null;
                existingPlugin.UpdateStatus();
            }

            UpdateStatistics();
        });
    }

    /// <summary>
    /// Handles plugin error events
    /// </summary>
    private void OnPluginError(object? sender, PluginErrorEventArgs e)
    {
        Application.Current.Dispatcher.InvokeAsync(() =>
        {
            var existingPlugin = Plugins.FirstOrDefault(p => p.Name == e.Plugin.Name);
            if (existingPlugin != null)
            {
                existingPlugin.HasError = true;
                existingPlugin.ErrorMessage = e.Exception.Message;
                existingPlugin.UpdateStatus();
            }

            StatusMessage = $"Plugin error: {e.Exception.Message}";
        });
    }
}

/// <summary>
/// ViewModel for individual plugin items
/// </summary>
public partial class PluginItemViewModel : ObservableObject
{
    [ObservableProperty]
    private PluginInfo _pluginInfo;

    [ObservableProperty]
    private IPlugin? _loadedPlugin;

    [ObservableProperty]
    private bool _hasError;

    [ObservableProperty]
    private string _errorMessage = string.Empty;

    [ObservableProperty]
    private string _statusText = string.Empty;

    [ObservableProperty]
    private Color _statusColor = Colors.Gray;

    /// <summary>
    /// Initializes a new instance of the PluginItemViewModel class
    /// </summary>
    /// <param name="pluginInfo">Plugin information</param>
    /// <param name="loadedPlugin">Loaded plugin instance</param>
    public PluginItemViewModel(PluginInfo pluginInfo, IPlugin? loadedPlugin = null)
    {
        PluginInfo = pluginInfo ?? throw new ArgumentNullException(nameof(pluginInfo));
        LoadedPlugin = loadedPlugin;
        UpdateStatus();
    }

    /// <summary>
    /// Gets the plugin name
    /// </summary>
    public string Name => PluginInfo.Name;

    /// <summary>
    /// Gets the plugin description
    /// </summary>
    public string Description => PluginInfo.Description;

    /// <summary>
    /// Gets the plugin version
    /// </summary>
    public string Version => PluginInfo.Version;

    /// <summary>
    /// Gets the plugin author
    /// </summary>
    public string Author => PluginInfo.Author;

    /// <summary>
    /// Gets the plugin category
    /// </summary>
    public string Category => LoadedPlugin?.Metadata.Category.ToString() ?? "General";

    /// <summary>
    /// Gets whether the plugin is enabled
    /// </summary>
    public bool IsEnabled => LoadedPlugin?.IsLoaded == true || PluginInfo.IsEnabled;

    /// <summary>
    /// Gets whether the plugin is legacy
    /// </summary>
    public bool IsLegacy => LoadedPlugin?.Metadata.IsLegacy == true;

    /// <summary>
    /// Gets whether the plugin can be configured
    /// </summary>
    public bool CanConfigure => LoadedPlugin?.GetConfigurationSchema() != null;

    /// <summary>
    /// Gets the enable/disable button text
    /// </summary>
    public string EnableDisableText => IsEnabled ? "Disable" : "Enable";

    /// <summary>
    /// Updates the plugin status display
    /// </summary>
    public void UpdateStatus()
    {
        if (HasError)
        {
            StatusText = $"Error: {ErrorMessage}";
            StatusColor = Colors.Red;
        }
        else if (IsEnabled)
        {
            StatusText = IsLegacy ? "Loaded (Legacy)" : "Loaded";
            StatusColor = Colors.Green;
        }
        else
        {
            StatusText = "Unloaded";
            StatusColor = Colors.Gray;
        }

        OnPropertyChanged(nameof(IsEnabled));
        OnPropertyChanged(nameof(EnableDisableText));
        OnPropertyChanged(nameof(CanConfigure));
    }
}