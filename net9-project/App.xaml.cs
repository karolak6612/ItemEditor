using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Options;
using System.Windows;
using ItemEditor.Models;
using ItemEditor.Services;

namespace ItemEditor;

/// <summary>
/// Interaction logic for App.xaml with dependency injection support
/// </summary>
public partial class App : Application
{
    private readonly ILogger<App> _logger;
    private readonly IServiceProvider _serviceProvider;
    private readonly ApplicationSettings _appSettings;
    private bool _isShuttingDown = false;
    
    /// <summary>
    /// Gets the service provider for the application
    /// </summary>
    public IServiceProvider? Services => _serviceProvider;
    
    /// <summary>
    /// Initializes a new instance of the App class
    /// </summary>
    public App()
    {
        // This parameterless constructor is required for XAML
        // The actual initialization happens through DI in Program.cs
        _logger = null!;
        _serviceProvider = null!;
        _appSettings = new ApplicationSettings();
    }
    
    /// <summary>
    /// Initializes a new instance of the App class with dependency injection
    /// </summary>
    /// <param name="serviceProvider">Service provider for dependency injection</param>
    /// <param name="logger">Logger instance</param>
    public App(IServiceProvider serviceProvider, ILogger<App> logger)
    {
        _serviceProvider = serviceProvider ?? throw new ArgumentNullException(nameof(serviceProvider));
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
        
        // Get application settings
        var appSettingsOptions = _serviceProvider.GetRequiredService<IOptions<ApplicationSettings>>();
        _appSettings = appSettingsOptions.Value;
        
        // Configure application-level event handlers
        DispatcherUnhandledException += OnDispatcherUnhandledException;
        
        _logger.LogDebug("App instance created with dependency injection");
    }
    
    /// <summary>
    /// Called when the application starts
    /// </summary>
    /// <param name="e">Startup event arguments</param>
    protected override async void OnStartup(StartupEventArgs e)
    {
        try
        {
            _logger.LogInformation("Application startup initiated. Version: {Version}", _appSettings.Version);
            
            // Initialize services that require startup initialization
            await InitializeServicesAsync();
            
            // Get the main window from DI container
            var mainWindow = _serviceProvider.GetRequiredService<MainWindow>();
            MainWindow = mainWindow;
            
            // Show the main window
            mainWindow.Show();
            
            _logger.LogInformation("Application startup completed successfully");
        }
        catch (Exception ex)
        {
            _logger.LogCritical(ex, "Critical error during application startup");
            
            var errorMessage = $"Application startup failed: {ex.Message}";
            MessageBox.Show(errorMessage, "Critical Startup Error", 
                MessageBoxButton.OK, MessageBoxImage.Error);
            
            Shutdown(-1);
            return;
        }
        
        base.OnStartup(e);
    }
    
    /// <summary>
    /// Initialize services that require async startup
    /// </summary>
    private async Task InitializeServicesAsync()
    {
        _logger.LogDebug("Initializing application services");
        
        try
        {
            // Check for legacy settings migration first
            await CheckAndMigrateLegacySettingsAsync();
            
            // Initialize settings service
            var settingsService = _serviceProvider.GetRequiredService<ISettingsService>();
            await settingsService.LoadSettingsAsync();
            
            // Initialize theme service
            var themeService = _serviceProvider.GetRequiredService<IThemeService>();
            await themeService.InitializeAsync();
            
            // Set up system theme watcher for the main window (will be done after window creation)
            // This is handled in MainWindow.xaml.cs OnLoaded event
            
            // Initialize plugin service if auto-load is enabled
            var pluginSettings = _serviceProvider.GetRequiredService<IOptions<PluginSettings>>().Value;
            if (pluginSettings.AutoLoad)
            {
                var pluginService = _serviceProvider.GetRequiredService<IPluginService>();
                _ = Task.Run(async () =>
                {
                    try
                    {
                        await pluginService.LoadPluginsAsync();
                        _logger.LogInformation("Plugins loaded successfully");
                    }
                    catch (Exception ex)
                    {
                        _logger.LogWarning(ex, "Failed to load some plugins during startup");
                    }
                });
            }
            
            _logger.LogDebug("Service initialization completed");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error during service initialization");
            throw;
        }
    }
    
    /// <summary>
    /// Check for legacy settings and migrate if necessary
    /// </summary>
    private async Task CheckAndMigrateLegacySettingsAsync()
    {
        try
        {
            var migrationService = _serviceProvider.GetRequiredService<ISettingsMigrationService>();
            
            if (await migrationService.HasLegacySettingsAsync())
            {
                _logger.LogInformation("Legacy settings detected, showing migration dialog");
                
                // Show migration dialog on UI thread
                var migrationDialog = new Dialogs.SettingsMigrationDialog(_serviceProvider);
                var result = migrationDialog.ShowDialog();
                
                if (result == true)
                {
                    _logger.LogInformation("Settings migration completed successfully");
                }
                else
                {
                    _logger.LogWarning("Settings migration was cancelled or failed");
                }
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error during legacy settings migration check");
            // Don't fail startup if migration fails - continue with default settings
        }
    }
    
    /// <summary>
    /// Called when the application is shutting down
    /// </summary>
    /// <param name="e">Exit event arguments</param>
    protected override async void OnExit(ExitEventArgs e)
    {
        if (_isShuttingDown)
            return;
            
        _isShuttingDown = true;
        
        try
        {
            _logger.LogInformation("Application shutdown initiated");
            
            // Perform graceful shutdown of services
            await ShutdownServicesAsync();
            
            _logger.LogInformation("Application shutdown completed successfully");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error during application shutdown");
        }
        
        base.OnExit(e);
    }
    
    /// <summary>
    /// Gracefully shutdown application services
    /// </summary>
    private async Task ShutdownServicesAsync()
    {
        _logger.LogDebug("Shutting down application services");
        
        try
        {
            // Save settings before shutdown
            var settingsService = _serviceProvider.GetRequiredService<ISettingsService>();
            await settingsService.SaveSettingsAsync();
            
            // Cleanup plugin service
            var pluginService = _serviceProvider.GetRequiredService<IPluginService>();
            await pluginService.UnloadAllPluginsAsync();
            
            // Cleanup file service (close any open files)
            var fileService = _serviceProvider.GetRequiredService<IFileService>();
            await fileService.CloseAllFilesAsync();
            
            _logger.LogDebug("Service shutdown completed");
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "Error during service shutdown");
        }
    }
    
    /// <summary>
    /// Handles unhandled dispatcher exceptions
    /// </summary>
    /// <param name="sender">Event sender</param>
    /// <param name="e">Exception event arguments</param>
    private void OnDispatcherUnhandledException(object sender, System.Windows.Threading.DispatcherUnhandledExceptionEventArgs e)
    {
        _logger.LogError(e.Exception, "Unhandled dispatcher exception occurred");
        
        // Let the global exception handler deal with it
        var globalHandler = _serviceProvider.GetRequiredService<IGlobalExceptionHandler>();
        globalHandler.HandleException(e.Exception);
        
        // Mark as handled to prevent application crash
        e.Handled = true;
    }
}