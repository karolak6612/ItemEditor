using System.Windows;
using ItemEditor.ViewModels;

namespace ItemEditor.Dialogs;

/// <summary>
/// Plugin Manager window for managing plugins
/// </summary>
public partial class PluginManagerWindow
{
    /// <summary>
    /// Initializes a new instance of the PluginManagerWindow class
    /// </summary>
    /// <param name="viewModel">Plugin manager view model</param>
    public PluginManagerWindow(PluginManagerViewModel viewModel)
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
        if (DataContext is PluginManagerViewModel viewModel)
        {
            viewModel.CloseRequested -= OnCloseRequested;
        }
        
        base.OnClosed(e);
    }
}