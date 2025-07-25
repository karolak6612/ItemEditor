using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using System.Windows;
using ItemEditor.Services;
using ItemEditor.ViewModels;
using Wpf.Ui.Controls;

namespace ItemEditor.Dialogs;

/// <summary>
/// Settings migration dialog
/// </summary>
public partial class SettingsMigrationDialog : FluentWindow
{
    private readonly ILogger<SettingsMigrationDialog> _logger;
    private readonly ISettingsMigrationService _migrationService;
    private readonly SettingsMigrationViewModel _viewModel;
    private CancellationTokenSource? _cancellationTokenSource;
    
    /// <summary>
    /// Initializes a new instance of the SettingsMigrationDialog class
    /// </summary>
    /// <param name="serviceProvider">Service provider for dependency injection</param>
    public SettingsMigrationDialog(IServiceProvider serviceProvider)
    {
        _logger = serviceProvider.GetRequiredService<ILogger<SettingsMigrationDialog>>();
        _migrationService = serviceProvider.GetRequiredService<ISettingsMigrationService>();
        _viewModel = new SettingsMigrationViewModel();
        
        InitializeComponent();
        DataContext = _viewModel;
        
        Loaded += OnLoaded;
    }
    
    /// <summary>
    /// Gets the migration result
    /// </summary>
    public bool MigrationSuccessful => _viewModel.MigrationSuccessful;
    
    private async void OnLoaded(object sender, RoutedEventArgs e)
    {
        // Start migration automatically when dialog loads
        await StartMigrationAsync();
    }
    
    private async Task StartMigrationAsync()
    {
        try
        {
            _cancellationTokenSource = new CancellationTokenSource();
            
            var progress = new Progress<Models.MigrationProgress>(progressInfo =>
            {
                Dispatcher.Invoke(() => _viewModel.UpdateProgress(progressInfo));
            });
            
            var result = await _migrationService.MigrateSettingsAsync(progress, _cancellationTokenSource.Token);
            
            Dispatcher.Invoke(() =>
            {
                _viewModel.UpdateResult(result);
                _logger.LogInformation("Settings migration completed. Success: {Success}, Message: {Message}", 
                    result.Success, result.Message);
            });
        }
        catch (OperationCanceledException)
        {
            _logger.LogInformation("Settings migration was cancelled");
            Dispatcher.Invoke(() =>
            {
                _viewModel.UpdateResult(new Models.MigrationResult
                {
                    Success = false,
                    Message = "Migration was cancelled by user"
                });
            });
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Unexpected error during settings migration");
            Dispatcher.Invoke(() =>
            {
                _viewModel.UpdateResult(new Models.MigrationResult
                {
                    Success = false,
                    Message = $"Unexpected error: {ex.Message}",
                    Errors = { ex.ToString() }
                });
            });
        }
    }
    
    private void CancelButton_Click(object sender, RoutedEventArgs e)
    {
        try
        {
            _cancellationTokenSource?.Cancel();
            _logger.LogInformation("User cancelled settings migration");
            
            DialogResult = false;
            Close();
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error cancelling migration");
        }
    }
    
    private void CloseButton_Click(object sender, RoutedEventArgs e)
    {
        DialogResult = _viewModel.MigrationSuccessful;
        Close();
    }
    
    private async void RetryButton_Click(object sender, RoutedEventArgs e)
    {
        try
        {
            _logger.LogInformation("User requested migration retry");
            _viewModel.Reset();
            await StartMigrationAsync();
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error during migration retry");
        }
    }
    
    protected override void OnClosed(EventArgs e)
    {
        _cancellationTokenSource?.Cancel();
        _cancellationTokenSource?.Dispose();
        base.OnClosed(e);
    }
}