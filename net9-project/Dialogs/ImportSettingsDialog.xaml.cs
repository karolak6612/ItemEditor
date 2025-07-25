using System.Windows;
using Wpf.Ui.Controls;

namespace ItemEditor.Dialogs;

/// <summary>
/// Import settings dialog
/// </summary>
public partial class ImportSettingsDialog : FluentWindow
{
    /// <summary>
    /// Initializes a new instance of the ImportSettingsDialog class
    /// </summary>
    public ImportSettingsDialog()
    {
        InitializeComponent();
    }
    
    /// <summary>
    /// Gets whether to merge with existing settings (true) or replace all (false)
    /// </summary>
    public bool MergeWithExisting => MergeRadioButton.IsChecked == true;
    
    /// <summary>
    /// Gets whether to create a backup before import
    /// </summary>
    public bool CreateBackup => CreateBackupCheckBox.IsChecked == true;
    
    /// <summary>
    /// Gets whether to validate settings before import
    /// </summary>
    public bool ValidateSettings => ValidateSettingsCheckBox.IsChecked == true;
    
    private void ImportButton_Click(object sender, RoutedEventArgs e)
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