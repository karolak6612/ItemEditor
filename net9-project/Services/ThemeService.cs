using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Options;
using System.Windows;
using Wpf.Ui;
using Wpf.Ui.Appearance;
using ItemEditor.Models;

namespace ItemEditor.Services;

/// <summary>
/// Implementation of theme service for managing application themes with WPFUI integration
/// </summary>
public class ThemeService : IThemeService
{
    private readonly ILogger<ThemeService> _logger;
    private readonly ISettingsService _settingsService;
    private readonly ApplicationSettings _applicationSettings;
    private string _currentTheme;
    private string _currentAccentColor;
    private bool _isSystemThemeWatcherEnabled;
    private string? _previewTheme;
    private string? _previewAccentColor;
    
    /// <summary>
    /// Initializes a new instance of the ThemeService class
    /// </summary>
    /// <param name="logger">Logger instance</param>
    public ThemeService(ILogger<ThemeService> logger)
    {
        _logger = logger;
        _settingsService = null!;
        _applicationSettings = new ApplicationSettings();
        _currentTheme = _applicationSettings.Theme;
        _currentAccentColor = _applicationSettings.AccentColor;
        _isSystemThemeWatcherEnabled = false;
    }
    
    /// <summary>
    /// Initializes a new instance of the ThemeService class with full dependencies
    /// </summary>
    /// <param name="logger">Logger instance</param>
    /// <param name="settingsService">Settings service</param>
    /// <param name="applicationSettings">Application settings</param>
    public ThemeService(
        ILogger<ThemeService> logger, 
        ISettingsService settingsService,
        IOptions<ApplicationSettings> applicationSettings)
    {
        _logger = logger;
        _settingsService = settingsService;
        _applicationSettings = applicationSettings.Value;
        _currentTheme = _applicationSettings.Theme;
        _currentAccentColor = _applicationSettings.AccentColor;
        _isSystemThemeWatcherEnabled = false;
    }
    
    /// <inheritdoc />
    public event EventHandler<ThemeChangedEventArgs>? ThemeChanged;
    
    /// <inheritdoc />
    public string GetCurrentTheme()
    {
        return _currentTheme;
    }
    
    /// <inheritdoc />
    public async Task SetThemeAsync(string themeName)
    {
        try
        {
            _logger.LogInformation("Changing theme from {OldTheme} to {NewTheme}", _currentTheme, themeName);
            
            var oldTheme = _currentTheme;
            _currentTheme = themeName;
            
            // Manage system theme watcher based on theme selection
            if (themeName.Equals("Auto", StringComparison.OrdinalIgnoreCase))
            {
                EnableSystemThemeWatcher();
            }
            else
            {
                DisableSystemThemeWatcher();
            }
            
            // Apply theme to WPFUI
            await ApplyThemeToWpfUiAsync(themeName);
            
            // Save theme preference
            _settingsService.SetSetting("Theme", themeName);
            await _settingsService.SaveSettingsAsync();
            
            // Raise theme changed event
            ThemeChanged?.Invoke(this, new ThemeChangedEventArgs(oldTheme, themeName));
            
            _logger.LogInformation("Theme changed successfully to {NewTheme}", themeName);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error changing theme to {ThemeName}", themeName);
            throw;
        }
    }
    
    /// <inheritdoc />
    public IEnumerable<string> GetAvailableThemes()
    {
        // Return available themes - these will be used with WPFUI
        return new[] { "Light", "Dark", "Auto" };
    }
    
    /// <inheritdoc />
    public async Task InitializeAsync()
    {
        try
        {
            _logger.LogInformation("Initializing theme service");
            
            // Load saved theme preference
            var savedTheme = _settingsService.GetSetting("Theme", _applicationSettings.Theme);
            
            if (!string.IsNullOrEmpty(savedTheme) && GetAvailableThemes().Contains(savedTheme))
            {
                _currentTheme = savedTheme;
            }
            
            // Apply initial theme to WPFUI
            await ApplyThemeToWpfUiAsync(_currentTheme);
            
            // Set up system theme watcher if theme is set to Auto
            if (_currentTheme.Equals("Auto", StringComparison.OrdinalIgnoreCase))
            {
                EnableSystemThemeWatcher();
            }
            
            _logger.LogInformation("Theme service initialized with theme: {CurrentTheme}", _currentTheme);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error initializing theme service");
            throw;
        }
    }
    
    /// <summary>
    /// Applies the specified theme to WPFUI
    /// </summary>
    /// <param name="themeName">Name of the theme to apply</param>
    private async Task ApplyThemeToWpfUiAsync(string themeName)
    {
        await Application.Current.Dispatcher.InvokeAsync(() =>
        {
            try
            {
                ApplicationTheme wpfuiTheme = themeName.ToLowerInvariant() switch
                {
                    "light" => ApplicationTheme.Light,
                    "dark" => ApplicationTheme.Dark,
                    "auto" => ApplicationTheme.Unknown, // Let system decide
                    _ => ApplicationTheme.Unknown
                };
                
                // Apply theme to application
                ApplicationThemeManager.Apply(wpfuiTheme);
                
                _logger.LogDebug("Applied WPFUI theme: {Theme}", wpfuiTheme);
            }
            catch (Exception ex)
            {
                _logger.LogWarning(ex, "Failed to apply WPFUI theme: {ThemeName}", themeName);
            }
        });
    }
    
    /// <summary>
    /// Enables system theme watcher for automatic theme switching
    /// </summary>
    private void EnableSystemThemeWatcher()
    {
        if (_isSystemThemeWatcherEnabled)
            return;
            
        try
        {
            // The SystemThemeWatcher will be enabled in MainWindow.xaml.cs
            // when the window is loaded to ensure proper window reference
            _isSystemThemeWatcherEnabled = true;
            
            _logger.LogInformation("System theme watcher marked for enabling");
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "Failed to enable system theme watcher");
        }
    }
    
    /// <summary>
    /// Disables system theme watcher
    /// </summary>
    private void DisableSystemThemeWatcher()
    {
        if (!_isSystemThemeWatcherEnabled)
            return;
            
        try
        {
            // The SystemThemeWatcher will be disabled in MainWindow.xaml.cs
            // to ensure proper window reference
            _isSystemThemeWatcherEnabled = false;
            
            _logger.LogInformation("System theme watcher marked for disabling");
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "Failed to disable system theme watcher");
        }
    }
    
    /// <inheritdoc />
    public async Task SetAccentColorAsync(string accentColor)
    {
        try
        {
            _logger.LogInformation("Changing accent color to {AccentColor}", accentColor);
            
            _currentAccentColor = accentColor;
            
            // Apply accent color to WPFUI
            await ApplyAccentColorToWpfUiAsync(accentColor);
            
            // Save accent color preference
            _settingsService.SetSetting("AccentColor", accentColor);
            await _settingsService.SaveSettingsAsync();
            
            _logger.LogInformation("Accent color changed successfully to {AccentColor}", accentColor);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error changing accent color to {AccentColor}", accentColor);
            throw;
        }
    }
    
    /// <inheritdoc />
    public string GetCurrentAccentColor()
    {
        return _currentAccentColor;
    }
    
    /// <inheritdoc />
    public void PreviewTheme(string themeName)
    {
        try
        {
            _previewTheme = themeName;
            _ = ApplyThemeToWpfUiAsync(themeName);
            _logger.LogDebug("Previewing theme: {ThemeName}", themeName);
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "Failed to preview theme: {ThemeName}", themeName);
        }
    }
    
    /// <inheritdoc />
    public void PreviewAccentColor(string accentColor)
    {
        try
        {
            _previewAccentColor = accentColor;
            _ = ApplyAccentColorToWpfUiAsync(accentColor);
            _logger.LogDebug("Previewing accent color: {AccentColor}", accentColor);
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "Failed to preview accent color: {AccentColor}", accentColor);
        }
    }
    
    /// <inheritdoc />
    public void CancelPreview()
    {
        try
        {
            if (_previewTheme != null)
            {
                _ = ApplyThemeToWpfUiAsync(_currentTheme);
                _previewTheme = null;
            }
            
            if (_previewAccentColor != null)
            {
                _ = ApplyAccentColorToWpfUiAsync(_currentAccentColor);
                _previewAccentColor = null;
            }
            
            _logger.LogDebug("Theme preview cancelled");
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "Failed to cancel theme preview");
        }
    }
    
    /// <summary>
    /// Applies the specified accent color to WPFUI
    /// </summary>
    /// <param name="accentColor">Name of the accent color to apply</param>
    private async Task ApplyAccentColorToWpfUiAsync(string accentColor)
    {
        await Application.Current.Dispatcher.InvokeAsync(() =>
        {
            try
            {
                // WPFUI accent color application would go here
                // This is a placeholder for actual accent color implementation
                _logger.LogDebug("Applied WPFUI accent color: {AccentColor}", accentColor);
            }
            catch (Exception ex)
            {
                _logger.LogWarning(ex, "Failed to apply WPFUI accent color: {AccentColor}", accentColor);
            }
        });
    }
}