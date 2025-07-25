using ItemEditor.ViewModels;
using ItemEditor.Services;
using Microsoft.Extensions.DependencyInjection;
using System.Windows;
using System.Windows.Controls;

namespace ItemEditor.Dialogs;

/// <summary>
/// Modern Preferences dialog with tabbed interface and comprehensive settings management
/// </summary>
public partial class PreferencesWindow : Wpf.Ui.Controls.FluentWindow
{
    private readonly IThemeService _themeService;
    private PreferencesViewModel ViewModel => (PreferencesViewModel)DataContext;
    
    /// <summary>
    /// Gets whether the dialog was accepted
    /// </summary>
    public bool WasAccepted { get; private set; }
    
    /// <summary>
    /// Initializes a new instance of the PreferencesWindow class
    /// </summary>
    public PreferencesWindow()
    {
        InitializeComponent();
        
        // Get theme service for real-time preview
        _themeService = App.Current.Services?.GetService<IThemeService>() ?? new ThemeService(null!);
        
        // Subscribe to ViewModel events
        ViewModel.CloseRequested += OnCloseRequested;
        ViewModel.ApplyRequested += OnApplyRequested;
    }
    
    /// <summary>
    /// Handles theme selection change for real-time preview
    /// </summary>
    private void ThemeComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if (ViewModel?.SelectedTheme != null)
        {
            // Apply theme preview temporarily
            _themeService.PreviewTheme(ViewModel.SelectedTheme.Key);
        }
    }
    
    /// <summary>
    /// Handles accent color selection change for real-time preview
    /// </summary>
    private void AccentColorComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if (ViewModel?.SelectedAccentColor != null)
        {
            // Apply accent color preview temporarily
            _themeService.PreviewAccentColor(ViewModel.SelectedAccentColor.Key);
        }
    }
    
    /// <summary>
    /// Handles the close requested event from the ViewModel
    /// </summary>
    /// <param name="sender">Event sender</param>
    /// <param name="accepted">Whether the dialog was accepted</param>
    private void OnCloseRequested(object? sender, bool accepted)
    {
        WasAccepted = accepted;
        DialogResult = accepted;
        Close();
    }
    
    /// <summary>
    /// Handles the apply requested event from the ViewModel
    /// </summary>
    /// <param name="sender">Event sender</param>
    /// <param name="e">Event arguments</param>
    private void OnApplyRequested(object? sender, EventArgs e)
    {
        // Settings have been applied, but dialog remains open
        // Could show a brief confirmation message if needed
    }
    
    /// <summary>
    /// Override to handle window closing
    /// </summary>
    /// <param name="e">Cancel event arguments</param>
    protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
    {
        // Cancel any active theme preview if dialog is not accepted
        if (!WasAccepted)
        {
            _themeService.CancelPreview();
        }
        
        // Check if there are unsaved changes
        if (ViewModel.HasUnsavedChanges && !WasAccepted)
        {
            var result = MessageBox.Show(
                "You have unsaved changes. Do you want to save them before closing?",
                "Unsaved Changes",
                MessageBoxButton.YesNoCancel,
                MessageBoxImage.Question);
            
            switch (result)
            {
                case MessageBoxResult.Yes:
                    ViewModel.ApplyCommand.Execute(null);
                    break;
                case MessageBoxResult.Cancel:
                    e.Cancel = true;
                    return;
            }
        }
        
        base.OnClosing(e);
    }
}