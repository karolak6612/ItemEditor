using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using ItemEditor.Models;
using ItemEditor.Services;
using ItemEditor.Dialogs;
using Microsoft.Win32;
using System.Collections.ObjectModel;
using System.IO;
using System.Text.Json;
using System.Windows;
using System.Windows.Media;

namespace ItemEditor.ViewModels;

/// <summary>
/// ViewModel for the Preferences dialog with comprehensive settings management
/// </summary>
public partial class PreferencesViewModel : ObservableObject
{
    private readonly ISettingsService _settingsService;
    private readonly IConfigurationService _configurationService;
    private readonly IThemeService _themeService;
    private readonly IPluginService _pluginService;
    private readonly IFileService _fileService;
    
    [ObservableProperty]
    private string _clientDirectory = string.Empty;
    
    [ObservableProperty]
    private bool _extendedFeatures;
    
    [ObservableProperty]
    private bool _frameDurations;
    
    [ObservableProperty]
    private bool _transparencySupport;
    
    [ObservableProperty]
    private bool _autoSave;
    
    [ObservableProperty]
    private bool _createBackups;
    
    [ObservableProperty]
    private int _recentFilesCount = 10;
    
    [ObservableProperty]
    private ThemeOption _selectedTheme;
    
    [ObservableProperty]
    private AccentColorOption _selectedAccentColor;
    
    [ObservableProperty]
    private double _fontSize = 12;
    
    [ObservableProperty]
    private bool _showThumbnails = true;
    
    [ObservableProperty]
    private double _thumbnailSize = 32;
    
    [ObservableProperty]
    private bool _associateOtbFiles;
    
    [ObservableProperty]
    private bool _associateDatFiles;
    
    [ObservableProperty]
    private bool _associateSprFiles;
    
    [ObservableProperty]
    private int _memoryCacheSize = 256;
    
    [ObservableProperty]
    private bool _enableMultiThreading = true;
    
    [ObservableProperty]
    private bool _hardwareAcceleration = true;
    
    [ObservableProperty]
    private ObservableCollection<PluginInfo> _installedPlugins = new();
    
    [ObservableProperty]
    private PluginInfo? _selectedPlugin;
    
    [ObservableProperty]
    private bool _hasClientInfo;
    
    [ObservableProperty]
    private string _clientInfoMessage = string.Empty;
    
    [ObservableProperty]
    private bool _hasError;
    
    [ObservableProperty]
    private string _errorMessage = string.Empty;
    
    [ObservableProperty]
    private bool _hasUnsavedChanges;
    
    private ApplicationSettings _originalSettings;
    
    /// <summary>
    /// Initializes a new instance of the PreferencesViewModel class
    /// </summary>
    public PreferencesViewModel(ISettingsService settingsService, IConfigurationService configurationService,
                               IThemeService themeService, IPluginService pluginService, IFileService fileService)
    {
        _settingsService = settingsService ?? throw new ArgumentNullException(nameof(settingsService));
        _configurationService = configurationService ?? throw new ArgumentNullException(nameof(configurationService));
        _themeService = themeService ?? throw new ArgumentNullException(nameof(themeService));
        _pluginService = pluginService ?? throw new ArgumentNullException(nameof(pluginService));
        _fileService = fileService ?? throw new ArgumentNullException(nameof(fileService));
        
        // Initialize commands
        BrowseClientDirectoryCommand = new RelayCommand(BrowseClientDirectory);
        RefreshPluginsCommand = new AsyncRelayCommand(RefreshPluginsAsync);
        InstallPluginCommand = new AsyncRelayCommand(InstallPluginAsync);
        TogglePluginCommand = new RelayCommand<PluginInfo>(TogglePlugin);
        RemovePluginCommand = new AsyncRelayCommand<PluginInfo>(RemovePluginAsync);
        ExportSettingsCommand = new AsyncRelayCommand(ExportSettingsAsync);
        ImportSettingsCommand = new AsyncRelayCommand(ImportSettingsAsync);
        ResetSettingsCommand = new RelayCommand(ResetSettings);
        ApplyCommand = new AsyncRelayCommand(ApplyAsync);
        OkCommand = new AsyncRelayCommand(OkAsync);
        CancelCommand = new RelayCommand(Cancel);
        
        // Initialize data
        InitializeThemes();
        InitializeAccentColors();
        LoadSettings();
        
        // Watch for changes
        PropertyChanged += OnPropertyChanged;
    }
    
    /// <summary>
    /// Parameterless constructor for XAML
    /// </summary>
    public PreferencesViewModel()
    {
        // Initialize with null services for design-time
        _settingsService = null!;
        _configurationService = null!;
        _themeService = null!;
        _pluginService = null!;
        _fileService = null!;
        _originalSettings = new ApplicationSettings();
        
        // Initialize commands with empty implementations for design-time
        BrowseClientDirectoryCommand = new RelayCommand(() => { });
        RefreshPluginsCommand = new AsyncRelayCommand(async () => { });
        InstallPluginCommand = new AsyncRelayCommand(async () => { });
        TogglePluginCommand = new RelayCommand<PluginInfo>(_ => { });
        RemovePluginCommand = new AsyncRelayCommand<PluginInfo>(async _ => { });
        ExportSettingsCommand = new AsyncRelayCommand(async () => { });
        ImportSettingsCommand = new AsyncRelayCommand(async () => { });
        ResetSettingsCommand = new RelayCommand(() => { });
        ApplyCommand = new AsyncRelayCommand(async () => { });
        OkCommand = new AsyncRelayCommand(async () => { });
        CancelCommand = new RelayCommand(() => { });
        
        // Initialize data for design-time
        InitializeThemes();
        InitializeAccentColors();
    }
    
    #region Properties
    
    /// <summary>
    /// Gets the available theme options
    /// </summary>
    public ObservableCollection<ThemeOption> AvailableThemes { get; } = new();
    
    /// <summary>
    /// Gets the available accent color options
    /// </summary>
    public ObservableCollection<AccentColorOption> AvailableAccentColors { get; } = new();
    
    #endregion
    
    #region Commands
    
    public IRelayCommand BrowseClientDirectoryCommand { get; }
    public IAsyncRelayCommand RefreshPluginsCommand { get; }
    public IAsyncRelayCommand InstallPluginCommand { get; }
    public IRelayCommand<PluginInfo> TogglePluginCommand { get; }
    public IAsyncRelayCommand<PluginInfo> RemovePluginCommand { get; }
    public IAsyncRelayCommand ExportSettingsCommand { get; }
    public IAsyncRelayCommand ImportSettingsCommand { get; }
    public IRelayCommand ResetSettingsCommand { get; }
    public IAsyncRelayCommand ApplyCommand { get; }
    public IAsyncRelayCommand OkCommand { get; }
    public IRelayCommand CancelCommand { get; }
    
    #endregion
    
    #region Events
    
    /// <summary>
    /// Event raised when the dialog should be closed
    /// </summary>
    public event EventHandler<bool>? CloseRequested;
    
    /// <summary>
    /// Event raised when settings should be applied
    /// </summary>
    public event EventHandler? ApplyRequested;
    
    #endregion
    
    #region Private Methods
    
    private void InitializeThemes()
    {
        AvailableThemes.Clear();
        AvailableThemes.Add(new ThemeOption("Auto", "Automatic", Wpf.Ui.Controls.SymbolRegular.WeatherSunny24));
        AvailableThemes.Add(new ThemeOption("Light", "Light", Wpf.Ui.Controls.SymbolRegular.WeatherSunny24));
        AvailableThemes.Add(new ThemeOption("Dark", "Dark", Wpf.Ui.Controls.SymbolRegular.WeatherMoon24));
        
        _selectedTheme = AvailableThemes.First();
    }
    
    private void InitializeAccentColors()
    {
        AvailableAccentColors.Clear();
        AvailableAccentColors.Add(new AccentColorOption("System", "System Default", SystemParameters.WindowGlassBrush));
        AvailableAccentColors.Add(new AccentColorOption("Blue", "Blue", new SolidColorBrush(Colors.DodgerBlue)));
        AvailableAccentColors.Add(new AccentColorOption("Green", "Green", new SolidColorBrush(Colors.ForestGreen)));
        AvailableAccentColors.Add(new AccentColorOption("Purple", "Purple", new SolidColorBrush(Colors.MediumPurple)));
        AvailableAccentColors.Add(new AccentColorOption("Red", "Red", new SolidColorBrush(Colors.Crimson)));
        AvailableAccentColors.Add(new AccentColorOption("Orange", "Orange", new SolidColorBrush(Colors.DarkOrange)));
        
        _selectedAccentColor = AvailableAccentColors.First();
    }
    
    private void LoadSettings()
    {
        try
        {
            _originalSettings = _settingsService.LoadSettings();
            
            // Load general settings
            ClientDirectory = _originalSettings.ClientDirectory;
            ExtendedFeatures = _originalSettings.ExtendedFeatures;
            FrameDurations = _originalSettings.FrameDurations;
            TransparencySupport = _originalSettings.TransparencySupport;
            AutoSave = _originalSettings.AutoSave;
            CreateBackups = _originalSettings.CreateBackups;
            RecentFilesCount = _originalSettings.RecentFilesCount;
            
            // Load appearance settings
            SelectedTheme = AvailableThemes.FirstOrDefault(t => t.Key == _originalSettings.Theme) ?? AvailableThemes.First();
            SelectedAccentColor = AvailableAccentColors.FirstOrDefault(c => c.Key == _originalSettings.AccentColor) ?? AvailableAccentColors.First();
            FontSize = _originalSettings.FontSize;
            ShowThumbnails = _originalSettings.ShowThumbnails;
            ThumbnailSize = _originalSettings.ThumbnailSize;
            
            // Load advanced settings
            AssociateOtbFiles = _originalSettings.AssociateOtbFiles;
            AssociateDatFiles = _originalSettings.AssociateDatFiles;
            AssociateSprFiles = _originalSettings.AssociateSprFiles;
            MemoryCacheSize = _originalSettings.MemoryCacheSize;
            EnableMultiThreading = _originalSettings.EnableMultiThreading;
            HardwareAcceleration = _originalSettings.HardwareAcceleration;
            
            // Validate client directory
            ValidateClientDirectory();
            
            // Load plugins
            _ = RefreshPluginsAsync();
            
            HasUnsavedChanges = false;
        }
        catch (Exception ex)
        {
            ShowError($"Failed to load settings: {ex.Message}");
        }
    }
    
    private void ValidateClientDirectory()
    {
        HasError = false;
        HasClientInfo = false;
        
        if (string.IsNullOrWhiteSpace(ClientDirectory))
        {
            return;
        }
        
        if (!Directory.Exists(ClientDirectory))
        {
            ShowError("Client directory does not exist");
            return;
        }
        
        try
        {
            var datFile = Directory.GetFiles(ClientDirectory, "*.dat").FirstOrDefault();
            var sprFile = Directory.GetFiles(ClientDirectory, "*.spr").FirstOrDefault();
            
            if (datFile == null || sprFile == null)
            {
                ShowError("Client files (.dat and .spr) not found in directory");
                return;
            }
            
            // Get file signatures
            var datSignature = GetFileSignature(datFile);
            var sprSignature = GetFileSignature(sprFile);
            
            // Try to find compatible plugin
            var compatiblePlugins = _pluginService.GetLoadedPlugins()
                .Where(p => p.SupportedClients.Any(c => c.DatSignature == datSignature && c.SprSignature == sprSignature))
                .ToList();
            
            if (compatiblePlugins.Any())
            {
                var plugin = compatiblePlugins.First();
                var client = plugin.SupportedClients.First(c => c.DatSignature == datSignature && c.SprSignature == sprSignature);
                
                ClientInfoMessage = $"Compatible client found: {client.Version} (Plugin: {plugin.Name})";
                HasClientInfo = true;
            }
            else
            {
                ShowError($"Unsupported client version\\nDAT Signature: {datSignature:X}\\nSPR Signature: {sprSignature:X}");
            }
        }
        catch (Exception ex)
        {
            ShowError($"Error validating client directory: {ex.Message}");
        }
    }
    
    private uint GetFileSignature(string filePath)
    {
        using var stream = File.OpenRead(filePath);
        using var reader = new BinaryReader(stream);
        return reader.ReadUInt32();
    }
    
    private void ShowError(string message)
    {
        ErrorMessage = message;
        HasError = true;
        HasClientInfo = false;
    }
    
    private void OnPropertyChanged(object? sender, System.ComponentModel.PropertyChangedEventArgs e)
    {
        if (e.PropertyName == nameof(HasUnsavedChanges))
            return;
        
        // Check if any setting has changed
        HasUnsavedChanges = HasSettingsChanged();
        
        // Special handling for client directory changes
        if (e.PropertyName == nameof(ClientDirectory))
        {
            ValidateClientDirectory();
        }
    }
    
    private bool HasSettingsChanged()
    {
        if (_originalSettings == null)
            return false;
        
        return ClientDirectory != _originalSettings.ClientDirectory ||
               ExtendedFeatures != _originalSettings.ExtendedFeatures ||
               FrameDurations != _originalSettings.FrameDurations ||
               TransparencySupport != _originalSettings.TransparencySupport ||
               AutoSave != _originalSettings.AutoSave ||
               CreateBackups != _originalSettings.CreateBackups ||
               RecentFilesCount != _originalSettings.RecentFilesCount ||
               SelectedTheme.Key != _originalSettings.Theme ||
               SelectedAccentColor.Key != _originalSettings.AccentColor ||
               Math.Abs(FontSize - _originalSettings.FontSize) > 0.1 ||
               ShowThumbnails != _originalSettings.ShowThumbnails ||
               Math.Abs(ThumbnailSize - _originalSettings.ThumbnailSize) > 0.1 ||
               AssociateOtbFiles != _originalSettings.AssociateOtbFiles ||
               AssociateDatFiles != _originalSettings.AssociateDatFiles ||
               AssociateSprFiles != _originalSettings.AssociateSprFiles ||
               MemoryCacheSize != _originalSettings.MemoryCacheSize ||
               EnableMultiThreading != _originalSettings.EnableMultiThreading ||
               HardwareAcceleration != _originalSettings.HardwareAcceleration;
    }
    
    #endregion
    
    #region Command Implementations
    
    private void BrowseClientDirectory()
    {
        var dialog = new System.Windows.Forms.FolderBrowserDialog
        {
            Description = "Select client directory containing .dat and .spr files",
            ShowNewFolderButton = false
        };
        
        if (!string.IsNullOrEmpty(ClientDirectory) && Directory.Exists(ClientDirectory))
        {
            dialog.SelectedPath = ClientDirectory;
        }
        
        if (dialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
        {
            ClientDirectory = dialog.SelectedPath;
        }
    }
    
    private async Task RefreshPluginsAsync()
    {
        try
        {
            var plugins = await _pluginService.GetInstalledPluginsAsync();
            
            InstalledPlugins.Clear();
            foreach (var plugin in plugins)
            {
                InstalledPlugins.Add(plugin);
            }
        }
        catch (Exception ex)
        {
            ShowError($"Failed to refresh plugins: {ex.Message}");
        }
    }
    
    private async Task InstallPluginAsync()
    {
        var dialog = new OpenFileDialog
        {
            Title = "Select Plugin File",
            Filter = "Plugin Files (*.dll)|*.dll|All Files (*.*)|*.*",
            Multiselect = false
        };
        
        if (dialog.ShowDialog() == true)
        {
            try
            {
                await _pluginService.InstallPluginAsync(dialog.FileName);
                await RefreshPluginsAsync();
            }
            catch (Exception ex)
            {
                ShowError($"Failed to install plugin: {ex.Message}");
            }
        }
    }
    
    private void TogglePlugin(PluginInfo? plugin)
    {
        if (plugin == null) return;
        
        try
        {
            if (plugin.IsEnabled)
            {
                _pluginService.DisablePlugin(plugin);
            }
            else
            {
                _pluginService.EnablePlugin(plugin);
            }
            
            plugin.IsEnabled = !plugin.IsEnabled;
        }
        catch (Exception ex)
        {
            ShowError($"Failed to toggle plugin: {ex.Message}");
        }
    }
    
    private async Task RemovePluginAsync(PluginInfo? plugin)
    {
        if (plugin == null) return;
        
        var result = MessageBox.Show(
            $"Are you sure you want to remove the plugin '{plugin.Name}'?",
            "Remove Plugin",
            MessageBoxButton.YesNo,
            MessageBoxImage.Question);
        
        if (result == MessageBoxResult.Yes)
        {
            try
            {
                await _pluginService.RemovePluginAsync(plugin);
                InstalledPlugins.Remove(plugin);
                
                if (SelectedPlugin == plugin)
                {
                    SelectedPlugin = null;
                }
            }
            catch (Exception ex)
            {
                ShowError($"Failed to remove plugin: {ex.Message}");
            }
        }
    }
    
    private async Task ExportSettingsAsync()
    {
        var dialog = new SaveFileDialog
        {
            Title = "Export Settings",
            Filter = "Settings Files (*.json)|*.json|All Files (*.*)|*.*",
            DefaultExt = "json",
            FileName = "ItemEditor_Settings.json"
        };
        
        if (dialog.ShowDialog() == true)
        {
            try
            {
                // Show options dialog for what to export
                var exportDialog = new ExportSettingsDialog();
                if (exportDialog.ShowDialog() == true)
                {
                    await _configurationService.ExportConfigurationAsync(dialog.FileName, exportDialog.IncludeUserSettings);
                    
                    MessageBox.Show("Settings exported successfully.", "Export Complete", 
                                   MessageBoxButton.OK, MessageBoxImage.Information);
                }
            }
            catch (Exception ex)
            {
                ShowError($"Failed to export settings: {ex.Message}");
            }
        }
    }
    
    private async Task ImportSettingsAsync()
    {
        var dialog = new OpenFileDialog
        {
            Title = "Import Settings",
            Filter = "Settings Files (*.json)|*.json|All Files (*.*)|*.*",
            Multiselect = false
        };
        
        if (dialog.ShowDialog() == true)
        {
            try
            {
                // Show options dialog for how to import
                var importDialog = new ImportSettingsDialog();
                if (importDialog.ShowDialog() == true)
                {
                    var success = await _configurationService.ImportConfigurationAsync(dialog.FileName, importDialog.MergeWithExisting);
                    
                    if (success)
                    {
                        // Reload settings to reflect imported changes
                        LoadSettings();
                        MessageBox.Show("Settings imported successfully.", "Import Complete", 
                                       MessageBoxButton.OK, MessageBoxImage.Information);
                    }
                    else
                    {
                        ShowError("Failed to import settings - invalid file format or validation failed");
                    }
                }
            }
            catch (Exception ex)
            {
                ShowError($"Failed to import settings: {ex.Message}");
            }
        }
    }
    
    private void LoadSettingsFromObject(ApplicationSettings settings)
    {
        ClientDirectory = settings.ClientDirectory;
        ExtendedFeatures = settings.ExtendedFeatures;
        FrameDurations = settings.FrameDurations;
        TransparencySupport = settings.TransparencySupport;
        AutoSave = settings.AutoSave;
        CreateBackups = settings.CreateBackups;
        RecentFilesCount = settings.RecentFilesCount;
        
        SelectedTheme = AvailableThemes.FirstOrDefault(t => t.Key == settings.Theme) ?? AvailableThemes.First();
        SelectedAccentColor = AvailableAccentColors.FirstOrDefault(c => c.Key == settings.AccentColor) ?? AvailableAccentColors.First();
        FontSize = settings.FontSize;
        ShowThumbnails = settings.ShowThumbnails;
        ThumbnailSize = settings.ThumbnailSize;
        
        AssociateOtbFiles = settings.AssociateOtbFiles;
        AssociateDatFiles = settings.AssociateDatFiles;
        AssociateSprFiles = settings.AssociateSprFiles;
        MemoryCacheSize = settings.MemoryCacheSize;
        EnableMultiThreading = settings.EnableMultiThreading;
        HardwareAcceleration = settings.HardwareAcceleration;
    }
    
    private async void ResetSettings()
    {
        var resetDialog = new ResetSettingsDialog();
        if (resetDialog.ShowDialog() == true)
        {
            try
            {
                if (resetDialog.CreateBackup)
                {
                    await _configurationService.CreateBackupAsync();
                }
                
                if (resetDialog.ResetAll)
                {
                    await _configurationService.ResetToDefaultsAsync();
                }
                else
                {
                    var sections = resetDialog.SectionsToReset;
                    
                    if (sections.HasFlag(ResetSections.General))
                        await _configurationService.ResetToDefaultsAsync("application");
                    
                    if (sections.HasFlag(ResetSections.Appearance))
                        await _configurationService.ResetToDefaultsAsync("ui");
                    
                    if (sections.HasFlag(ResetSections.Plugins))
                        await _configurationService.ResetToDefaultsAsync("plugins");
                    
                    if (sections.HasFlag(ResetSections.Advanced))
                        await _configurationService.ResetToDefaultsAsync("performance");
                    
                    if (sections.HasFlag(ResetSections.User))
                        await _configurationService.ResetToDefaultsAsync("user");
                }
                
                // Reload settings to reflect the reset
                LoadSettings();
                
                MessageBox.Show("Settings have been reset successfully.", "Reset Complete", 
                               MessageBoxButton.OK, MessageBoxImage.Information);
            }
            catch (Exception ex)
            {
                ShowError($"Failed to reset settings: {ex.Message}");
            }
        }
    }
    
    private async Task ApplyAsync()
    {
        try
        {
            var settings = CreateSettingsFromCurrent();
            await _settingsService.SaveSettingsAsync(settings);
            
            // Apply theme changes immediately
            _themeService.SetTheme(SelectedTheme.Key);
            _themeService.SetAccentColor(SelectedAccentColor.Key);
            
            _originalSettings = settings;
            HasUnsavedChanges = false;
            
            ApplyRequested?.Invoke(this, EventArgs.Empty);
        }
        catch (Exception ex)
        {
            ShowError($"Failed to apply settings: {ex.Message}");
        }
    }
    
    private async Task OkAsync()
    {
        await ApplyAsync();
        
        if (!HasError)
        {
            CloseRequested?.Invoke(this, true);
        }
    }
    
    private void Cancel()
    {
        CloseRequested?.Invoke(this, false);
    }
    
    private ApplicationSettings CreateSettingsFromCurrent()
    {
        return new ApplicationSettings
        {
            ClientDirectory = ClientDirectory,
            ExtendedFeatures = ExtendedFeatures,
            FrameDurations = FrameDurations,
            TransparencySupport = TransparencySupport,
            AutoSave = AutoSave,
            CreateBackups = CreateBackups,
            RecentFilesCount = RecentFilesCount,
            Theme = SelectedTheme.Key,
            AccentColor = SelectedAccentColor.Key,
            FontSize = FontSize,
            ShowThumbnails = ShowThumbnails,
            ThumbnailSize = ThumbnailSize,
            AssociateOtbFiles = AssociateOtbFiles,
            AssociateDatFiles = AssociateDatFiles,
            AssociateSprFiles = AssociateSprFiles,
            MemoryCacheSize = MemoryCacheSize,
            EnableMultiThreading = EnableMultiThreading,
            HardwareAcceleration = HardwareAcceleration
        };
    }
    
    #endregion
}

/// <summary>
/// Represents a theme option
/// </summary>
public class ThemeOption
{
    public string Key { get; }
    public string Name { get; }
    public Wpf.Ui.Controls.SymbolRegular Icon { get; }
    
    public ThemeOption(string key, string name, Wpf.Ui.Controls.SymbolRegular icon)
    {
        Key = key;
        Name = name;
        Icon = icon;
    }
}

/// <summary>
/// Represents an accent color option
/// </summary>
public class AccentColorOption
{
    public string Key { get; }
    public string Name { get; }
    public Brush Brush { get; }
    
    public AccentColorOption(string key, string name, Brush brush)
    {
        Key = key;
        Name = name;
        Brush = brush;
    }
}

/// <summary>
/// Represents plugin information for the UI
/// </summary>
public class PluginInfo : ObservableObject
{
    private bool _isEnabled;
    
    public string Name { get; set; } = string.Empty;
    public string Description { get; set; } = string.Empty;
    public string Version { get; set; } = string.Empty;
    public string Author { get; set; } = string.Empty;
    public string FilePath { get; set; } = string.Empty;
    public DateTime LoadedAt { get; set; }
    public List<string> SupportedClients { get; set; } = new();
    
    public bool IsEnabled
    {
        get => _isEnabled;
        set
        {
            if (SetProperty(ref _isEnabled, value))
            {
                OnPropertyChanged(nameof(StatusIcon));
                OnPropertyChanged(nameof(StatusBrush));
            }
        }
    }
    
    public Wpf.Ui.Controls.SymbolRegular StatusIcon => IsEnabled 
        ? Wpf.Ui.Controls.SymbolRegular.CheckmarkCircle24 
        : Wpf.Ui.Controls.SymbolRegular.DismissCircle24;
    
    public Brush StatusBrush => IsEnabled 
        ? new SolidColorBrush(Colors.Green) 
        : new SolidColorBrush(Colors.Red);
}