using System.Windows;
using Wpf.Ui.Controls;

namespace ItemEditor.Dialogs;

/// <summary>
/// Export settings dialog
/// </summary>
public partial class ExportSettingsDialog : FluentWindow
{
    /// <summary>
    /// Initializes a new instance of the ExportSettingsDialog class
    /// </summary>
    public ExportSettingsDialog()
    {
        InitializeComponent();
    }
    
    /// <summary>
    /// Gets whether to include user settings in the export
    /// </summary>
    public bool IncludeUserSettings => IncludeUserSettingsCheckBox.IsChecked == true;
    
    /// <summary>
    /// Gets whether to include plugin settings in the export
    /// </summary>
    public bool IncludePluginSettings => IncludePluginSettingsCheckBox.IsChecked == true;
    
    /// <summary>
    /// Gets whether to include advanced settings in the export
    /// </summary>
    public bool IncludeAdvancedSettings => IncludeAdvancedSettingsCheckBox.IsChecked == true;
    
    private void ExportButton_Click(object sender, RoutedEventArgs e)
    {
        DialogResult = true;
        Close();
    }
    
    private void CancelButton_Click(object sender, RoutedEventArgs e)
    {
        DialogResult = false;
        Close();
    }
}