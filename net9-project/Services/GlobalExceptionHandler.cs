using Microsoft.Extensions.Logging;
using System.Windows;
using System.Windows.Threading;

namespace ItemEditor.Services;

/// <summary>
/// Implementation of global exception handler for managing unhandled exceptions
/// </summary>
public class GlobalExceptionHandler : IGlobalExceptionHandler
{
    private readonly ILogger<GlobalExceptionHandler> _logger;
    
    /// <summary>
    /// Initializes a new instance of the GlobalExceptionHandler class
    /// </summary>
    /// <param name="logger">Logger instance</param>
    public GlobalExceptionHandler(ILogger<GlobalExceptionHandler> logger)
    {
        _logger = logger;
    }
    
    /// <inheritdoc />
    public void Initialize()
    {
        _logger.LogInformation("Initializing global exception handler");
        
        // Handle unhandled exceptions in the UI thread
        Application.Current.DispatcherUnhandledException += OnDispatcherUnhandledException;
        
        // Handle unhandled exceptions in background threads
        AppDomain.CurrentDomain.UnhandledException += OnUnhandledException;
        
        // Handle unhandled exceptions in tasks
        TaskScheduler.UnobservedTaskException += OnUnobservedTaskException;
        
        _logger.LogInformation("Global exception handler initialized");
    }
    
    /// <inheritdoc />
    public void HandleException(Exception exception, string? context = null)
    {
        try
        {
            var contextInfo = string.IsNullOrEmpty(context) ? "" : $" in {context}";
            _logger.LogError(exception, "Exception handled{Context}", contextInfo);
            
            // Show user-friendly error dialog
            ShowErrorDialog("An error occurred", exception, context);
        }
        catch (Exception ex)
        {
            // Last resort - log to console if logging fails
            Console.WriteLine($"Critical error in exception handler: {ex}");
            Console.WriteLine($"Original exception: {exception}");
        }
    }
    
    private void OnDispatcherUnhandledException(object sender, DispatcherUnhandledExceptionEventArgs e)
    {
        _logger.LogError(e.Exception, "Unhandled dispatcher exception occurred");
        
        try
        {
            ShowErrorDialog("An unexpected error occurred", e.Exception);
            e.Handled = true; // Prevent application crash
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error handling dispatcher exception");
            // Don't mark as handled if we can't show the dialog
        }
    }
    
    private void OnUnhandledException(object sender, UnhandledExceptionEventArgs e)
    {
        if (e.ExceptionObject is Exception exception)
        {
            _logger.LogError(exception, "Unhandled domain exception occurred. Terminating: {IsTerminating}", e.IsTerminating);
            
            if (!e.IsTerminating)
            {
                try
                {
                    ShowErrorDialog("A critical error occurred", exception);
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, "Error handling domain exception");
                }
            }
        }
    }
    
    private void OnUnobservedTaskException(object? sender, UnobservedTaskExceptionEventArgs e)
    {
        _logger.LogError(e.Exception, "Unobserved task exception occurred");
        
        try
        {
            ShowErrorDialog("A background task error occurred", e.Exception);
            e.SetObserved(); // Prevent application crash
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error handling task exception");
        }
    }
    
    private void ShowErrorDialog(string message, Exception exception, string? context = null)
    {
        try
        {
            // Ensure we're on the UI thread
            if (Application.Current?.Dispatcher.CheckAccess() == false)
            {
                Application.Current.Dispatcher.Invoke(() => ShowErrorDialog(message, exception, context));
                return;
            }
            
            var contextInfo = string.IsNullOrEmpty(context) ? "" : $"\n\nContext: {context}";
            var errorMessage = $"{message}{contextInfo}\n\nError: {exception.Message}";
            
            // TODO: Replace with WPFUI dialog when available
            MessageBox.Show(
                errorMessage,
                "Error",
                MessageBoxButton.OK,
                MessageBoxImage.Error);
        }
        catch (Exception ex)
        {
            // Last resort - log to console
            Console.WriteLine($"Failed to show error dialog: {ex}");
            Console.WriteLine($"Original error: {exception}");
        }
    }
}