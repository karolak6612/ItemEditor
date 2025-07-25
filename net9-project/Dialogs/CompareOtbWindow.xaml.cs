using ItemEditor.Services;
using ItemEditor.ViewModels;
using System.Windows;

namespace ItemEditor.Dialogs;

/// <summary>
/// Enhanced OTB comparison dialog with side-by-side visualization and export functionality
/// </summary>
public partial class CompareOtbWindow : Wpf.Ui.Controls.FluentWindow
{
    /// <summary>
    /// Initializes a new instance of the CompareOtbWindow class
    /// </summary>
    public CompareOtbWindow()
    {
        InitializeComponent();
    }
    
    /// <summary>
    /// Initializes a new instance of the CompareOtbWindow class with dependency injection
    /// </summary>
    /// <param name="fileService">File service for loading OTB files</param>
    public CompareOtbWindow(IFileService fileService)
    {
        InitializeComponent();
        DataContext = new CompareOtbViewModel(fileService);
    }
    
    /// <summary>
    /// Handles the Close button click
    /// </summary>
    /// <param name="sender">Event sender</param>
    /// <param name="e">Event arguments</param>
    private void OnCloseClick(object sender, RoutedEventArgs e)
    {
        DialogResult = true;
        Close();
    }
}