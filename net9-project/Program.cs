using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using System.Windows;
using ItemEditor.Services;
using ItemEditor.ViewModels;
using ItemEditor.Models;

namespace ItemEditor;

/// <summary>
/// Application entry point with modern async Main method and dependency injection
/// </summary>
public class Program
{
    private static IHost? _host;
    private static ILogger<Program>? _logger;

    /// <summary>
    /// The main entry point for the application with async support
    /// </summary>
    /// <param name="args">Command line arguments</param>
    /// <returns>Exit code</returns>
    [STAThread]
    public static async Task<int> Main(string[] args)
    {
        try
        {
            // Create and configure the host builder
            var builder = Host.CreateApplicationBuilder(args);
            
            // Configure services
            ConfigureServices(builder.Services, builder.Configuration);
            
            // Build the host
            _host = builder.Build();
            
            // Get logger after host is built
            _logger = _host.Services.GetRequiredService<ILogger<Program>>();
            _logger.LogInformation("Application host created successfully");
            
            // Start the host services
            await _host.StartAsync();
            _logger.LogInformation("Host services started");
            
            // Configure application shutdown handling
            AppDomain.CurrentDomain.ProcessExit += OnProcessExit;
            Console.CancelKeyPress += OnCancelKeyPress;
            
            // Get the application instance and run it
            var app = _host.Services.GetRequiredService<App>();
            app.InitializeComponent();
            
            // Configure global exception handling
            var exceptionHandler = _host.Services.GetRequiredService<IGlobalExceptionHandler>();
            exceptionHandler.Initialize();
            
            _logger.LogInformation("Starting WPF application");
            
            // Run the WPF application
            var exitCode = app.Run();
            
            _logger.LogInformation("WPF application exited with code: {ExitCode}", exitCode);
            return exitCode;
        }
        catch (Exception ex)
        {
            // Log startup errors
            if (_logger != null)
            {
                _logger.LogCritical(ex, "Application startup failed");
            }
            else
            {
                Console.WriteLine($"Application startup failed: {ex}");
            }
            return -1;
        }
        finally
        {
            // Ensure proper cleanup
            await ShutdownAsync();
        }
    }
    
    /// <summary>
    /// Configure dependency injection services
    /// </summary>
    /// <param name="services">Service collection</param>
    /// <param name="configuration">Application configuration</param>
    private static void ConfigureServices(IServiceCollection services, IConfiguration configuration)
    {
        // Configure logging first for early initialization
        services.AddLogging(builder =>
        {
            builder.AddConfiguration(configuration.GetSection("Logging"));
            builder.AddConsole();
            builder.AddDebug();
            
            // Add file logging for production scenarios
            if (configuration.GetValue<bool>("Logging:EnableFileLogging", false))
            {
                var logPath = configuration.GetValue<string>("Logging:LogFilePath", "logs/itemeditor.log");
                // File logging would be configured here when needed
            }
        });
        
        // Register WPF Application with proper service provider injection
        services.AddSingleton<App>(provider => 
        {
            var logger = provider.GetRequiredService<ILogger<App>>();
            return new App(provider, logger);
        });
        
        // Register main window and view model as singletons
        services.AddSingleton<MainWindow>();
        services.AddSingleton<MainViewModel>();
        
        // Register WPFUI services (will be configured when needed)
        // Note: WPFUI services will be added when specific functionality is implemented
        
        // Register application services with proper interfaces
        services.AddSingleton<IFileService, FileService>();
        services.AddSingleton<IPluginService, PluginService>();
        services.AddSingleton<IImageService, ImageService>();
        services.AddSingleton<ISettingsService, SettingsService>();
        services.AddSingleton<IConfigurationService, ConfigurationService>();
        services.AddSingleton<ISettingsMigrationService, SettingsMigrationService>();
        services.AddSingleton<IGlobalExceptionHandler, GlobalExceptionHandler>();
        services.AddSingleton<IThemeService>(provider =>
        {
            var logger = provider.GetRequiredService<ILogger<ThemeService>>();
            var settingsService = provider.GetRequiredService<ISettingsService>();
            var appSettings = provider.GetRequiredService<IOptions<ApplicationSettings>>();
            return new ThemeService(logger, settingsService, appSettings);
        });
        
        // Configure application settings with validation
        services.Configure<ApplicationSettings>(options =>
        {
            configuration.GetSection("Application").Bind(options);
            ValidateApplicationSettings(options);
        });
        
        services.Configure<PluginSettings>(options =>
        {
            configuration.GetSection("Plugins").Bind(options);
            ValidatePluginSettings(options);
        });
        
        services.Configure<FileSettings>(options =>
        {
            configuration.GetSection("Files").Bind(options);
            ValidateFileSettings(options);
        });
        
        // Add hosted services for background tasks if needed
        // services.AddHostedService<BackgroundTaskService>();
    }
    
    /// <summary>
    /// Validates application settings
    /// </summary>
    /// <param name="settings">Application settings to validate</param>
    private static void ValidateApplicationSettings(ApplicationSettings settings)
    {
        if (string.IsNullOrWhiteSpace(settings.Name))
            throw new InvalidOperationException("Application name cannot be empty");
        
        if (string.IsNullOrWhiteSpace(settings.Version))
            throw new InvalidOperationException("Application version cannot be empty");
    }
    
    /// <summary>
    /// Validates plugin settings
    /// </summary>
    /// <param name="settings">Plugin settings to validate</param>
    private static void ValidatePluginSettings(PluginSettings settings)
    {
        if (string.IsNullOrWhiteSpace(settings.Directory))
            throw new InvalidOperationException("Plugin directory cannot be empty");
        
        if (settings.LoadTimeout <= 0)
            throw new InvalidOperationException("Plugin load timeout must be positive");
    }
    
    /// <summary>
    /// Validates file settings
    /// </summary>
    /// <param name="settings">File settings to validate</param>
    private static void ValidateFileSettings(FileSettings settings)
    {
        if (settings.RecentFilesCount < 0)
            throw new InvalidOperationException("Recent files count cannot be negative");
        
        if (string.IsNullOrWhiteSpace(settings.BackupDirectory))
            throw new InvalidOperationException("Backup directory cannot be empty");
    }
    
    /// <summary>
    /// Handles process exit event for graceful shutdown
    /// </summary>
    /// <param name="sender">Event sender</param>
    /// <param name="e">Event arguments</param>
    private static async void OnProcessExit(object? sender, EventArgs e)
    {
        _logger?.LogInformation("Process exit requested, initiating shutdown");
        await ShutdownAsync();
    }
    
    /// <summary>
    /// Handles console cancel key press for graceful shutdown
    /// </summary>
    /// <param name="sender">Event sender</param>
    /// <param name="e">Event arguments</param>
    private static async void OnCancelKeyPress(object? sender, ConsoleCancelEventArgs e)
    {
        _logger?.LogInformation("Cancel key pressed, initiating shutdown");
        e.Cancel = true; // Prevent immediate termination
        await ShutdownAsync();
    }
    
    /// <summary>
    /// Performs graceful application shutdown
    /// </summary>
    private static async Task ShutdownAsync()
    {
        if (_host != null)
        {
            try
            {
                _logger?.LogInformation("Shutting down application host");
                
                // Stop the host with timeout
                using var cts = new CancellationTokenSource(TimeSpan.FromSeconds(10));
                await _host.StopAsync(cts.Token);
                
                _logger?.LogInformation("Host shutdown completed");
            }
            catch (Exception ex)
            {
                _logger?.LogError(ex, "Error during host shutdown");
            }
            finally
            {
                _host.Dispose();
                _host = null;
            }
        }
    }
}