using System.Windows;
using System.Windows.Controls;
using ItemEditor.ViewModels;
using ItemEditor.Services;
using Microsoft.Extensions.Logging;

namespace ItemEditor.Dialogs;

/// <summary>
/// Help window for displaying comprehensive help documentation
/// </summary>
public partial class HelpWindow : Wpf.Ui.Controls.FluentWindow
{
    private readonly HelpViewModel _viewModel;
    private readonly ILogger<HelpWindow> _logger;
    
    public HelpWindow(HelpViewModel viewModel, ILogger<HelpWindow> logger)
    {
        _viewModel = viewModel ?? throw new ArgumentNullException(nameof(viewModel));
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
        
        InitializeComponent();
        
        DataContext = _viewModel;
        
        // Subscribe to events
        Loaded += OnWindowLoaded;
        Closing += OnWindowClosing;
        
        _logger.LogDebug("HelpWindow initialized");
    }
    
    private async void OnWindowLoaded(object sender, RoutedEventArgs e)
    {
        try
        {
            await _viewModel.InitializeAsync();
            _logger.LogDebug("HelpWindow loaded and initialized");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error loading help window");
        }
    }
    
    private void OnWindowClosing(object? sender, System.ComponentModel.CancelEventArgs e)
    {
        try
        {
            _viewModel.Cleanup();
            _logger.LogDebug("HelpWindow closing");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error closing help window");
        }
    }
    
    private void OnNavigationSelectionChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
    {
        try
        {
            if (e.NewValue is HelpNavigationItem item)
            {
                _viewModel.NavigateToContent(item.Context);
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error handling navigation selection change");
        }
    }
}