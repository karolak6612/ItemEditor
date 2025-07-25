using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.Animation;
using ItemEditor.ViewModels;
using Microsoft.Extensions.Logging;

namespace ItemEditor.Controls;

/// <summary>
/// Control for displaying undo/redo status and providing visual feedback
/// </summary>
public partial class UndoRedoStatusControl : UserControl
{
    private readonly ILogger<UndoRedoStatusControl>? _logger;
    private Storyboard? _statusChangeAnimation;
    private Storyboard? _autoHideAnimation;
    
    public UndoRedoStatusControl()
    {
        InitializeComponent();
        
        // Get animations from resources
        _statusChangeAnimation = FindResource("StatusChangeAnimation") as Storyboard;
        _autoHideAnimation = FindResource("AutoHideAnimation") as Storyboard;
        
        // Subscribe to data context changes
        DataContextChanged += OnDataContextChanged;
    }
    
    public UndoRedoStatusControl(ILogger<UndoRedoStatusControl> logger) : this()
    {
        _logger = logger;
    }
    
    private void OnDataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
    {
        try
        {
            // Unsubscribe from old view model
            if (e.OldValue is UndoRedoStatusViewModel oldViewModel)
            {
                oldViewModel.StatusChanged -= OnStatusChanged;
            }
            
            // Subscribe to new view model
            if (e.NewValue is UndoRedoStatusViewModel newViewModel)
            {
                newViewModel.StatusChanged += OnStatusChanged;
            }
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Error handling data context change");
        }
    }
    
    private void OnStatusChanged(object? sender, StatusChangedEventArgs e)
    {
        try
        {
            // Update status icon based on operation type
            UpdateStatusIcon(e.OperationType, e.Success);
            
            // Show status animation
            ShowStatusAnimation();
            
            _logger?.LogDebug("Status changed: {OperationType} - {Success}", e.OperationType, e.Success);
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Error handling status change");
        }
    }
    
    private void UpdateStatusIcon(UndoRedoOperationType operationType, bool success)
    {
        try
        {
            var iconSymbol = operationType switch
            {
                UndoRedoOperationType.Execute => success ? "Checkmark16" : "ErrorCircle16",
                UndoRedoOperationType.Undo => success ? "ArrowUndo16" : "ErrorCircle16",
                UndoRedoOperationType.Redo => success ? "ArrowRedo16" : "ErrorCircle16",
                _ => "Info16"
            };
            
            StatusIcon.Symbol = Wpf.Ui.Common.SymbolRegular.Checkmark16; // This would need proper enum conversion
            
            // Update color based on success
            var colorResource = success ? "SystemFillColorSuccessBrush" : "SystemFillColorCriticalBrush";
            StatusIcon.Foreground = (System.Windows.Media.Brush)FindResource(colorResource);
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Error updating status icon");
        }
    }
    
    private void ShowStatusAnimation()
    {
        try
        {
            // Stop any running animations
            _statusChangeAnimation?.Stop();
            _autoHideAnimation?.Stop();
            
            // Reset opacity and scale
            StatusCard.Opacity = 0;
            if (StatusCard.RenderTransform is System.Windows.Media.ScaleTransform scaleTransform)
            {
                scaleTransform.ScaleX = 0.8;
                scaleTransform.ScaleY = 0.8;
            }
            
            // Start show animation
            _statusChangeAnimation?.Begin();
            
            // Start auto-hide animation
            _autoHideAnimation?.Begin();
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Error showing status animation");
        }
    }
    
    /// <summary>
    /// Shows the status with a custom message
    /// </summary>
    /// <param name="message">Status message</param>
    /// <param name="operationType">Operation type</param>
    /// <param name="success">Whether the operation was successful</param>
    public void ShowStatus(string message, UndoRedoOperationType operationType, bool success = true)
    {
        try
        {
            if (DataContext is UndoRedoStatusViewModel viewModel)
            {
                viewModel.ShowStatus(message, operationType, success);
            }
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Error showing status: {Message}", message);
        }
    }
    
    /// <summary>
    /// Hides the status display
    /// </summary>
    public void HideStatus()
    {
        try
        {
            if (DataContext is UndoRedoStatusViewModel viewModel)
            {
                viewModel.HideStatus();
            }
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Error hiding status");
        }
    }
    
    /// <summary>
    /// Shows the command history panel
    /// </summary>
    public void ShowHistory()
    {
        try
        {
            if (DataContext is UndoRedoStatusViewModel viewModel)
            {
                viewModel.ShowHistoryPanel = true;
            }
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Error showing history");
        }
    }
    
    /// <summary>
    /// Hides the command history panel
    /// </summary>
    public void HideHistory()
    {
        try
        {
            if (DataContext is UndoRedoStatusViewModel viewModel)
            {
                viewModel.ShowHistoryPanel = false;
            }
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Error hiding history");
        }
    }
}