using System.Windows;
using ItemEditor.ViewModels;

namespace ItemEditor.Dialogs;

/// <summary>
/// Plugin Information window for displaying detailed plugin information
/// </summary>
public partial class PluginInfoWindow
{
    /// <summary>
    /// Initializes a new instance of the PluginInfoWindow class
    /// </summary>
    /// <param name="viewModel">Plugin info view model</param>
    public PluginInfoWindow(PluginInfoViewModel viewModel)
    {
        InitializeComponent();
        DataContext = viewModel ?? throw new ArgumentNullException(nameof(viewModel));
        
        // Subscribe to close request
        viewModel.CloseRequested += OnCloseRequested;
        
        // Set window properties
        Owner = Application.Current.MainWindow;
        WindowStartupLocation = WindowStartupLocation.CenterOwner;
    }

    /// <summary>
    /// Handles close request from view model
    /// </summary>
    private void OnCloseRequested(object? sender, EventArgs e)
    {
        Close();
    }

    /// <summary>
    /// Handles window closing
    /// </summary>
    protected override void OnClosed(EventArgs e)
    {
        if (DataContext is PluginInfoViewModel viewModel)
        {
            viewModel.CloseRequested -= OnCloseRequested;
        }
        
        base.OnClosed(e);
    }
}