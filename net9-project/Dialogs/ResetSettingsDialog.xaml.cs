using System.Windows;
using Wpf.Ui.Controls;

namespace ItemEditor.Dialogs;

/// <summary>
/// Reset settings dialog
/// </summary>
public partial class ResetSettingsDialog : FluentWindow
{
    /// <summary>
    /// Initializes a new instance of the ResetSettingsDialog class
    /// </summary>
    public ResetSettingsDialog()
    {
        InitializeComponent();
    }
    
    /// <summary>
    /// Gets whether to reset all settings
    /// </summary>
    public bool ResetAll => ResetAllRadioButton.IsChecked == true;
    
    /// <summary>
    /// Gets whether to create a backup before reset
    /// </summary>
    public bool CreateBackup => CreateBackupCheckBox.IsChecked == true;
    
    /// <summary>
    /// Gets the sections to reset (only valid if ResetAll is false)
    /// </summary>
    public ResetSections SectionsToReset
    {
        get
        {
            var sections = ResetSections.None;
            
            if (ResetGeneralCheckBox.IsChecked == true)
                sections |= ResetSections.General;
            
            if (ResetAppearanceCheckBox.IsChecked == true)
                sections |= ResetSections.Appearance;
            
            if (ResetPluginCheckBox.IsChecked == true)
                sections |= ResetSections.Plugins;
            
            if (ResetAdvancedCheckBox.IsChecked == true)
                sections |= ResetSections.Advanced;
            
            if (ResetUserCheckBox.IsChecked == true)
                sections |= ResetSections.User;
            
            return sections;
        }
    }
    
    private void ResetButton_Click(object sender, RoutedEventArgs e)
    {
        // Validate selection if selective reset
        if (!ResetAll && SectionsToReset == ResetSections.None)
        {
            MessageBox.Show("Please select at least one section to reset.", "No Selection", 
                           MessageBoxButton.OK, MessageBoxImage.Warning);
            return;
        }
        
        DialogResult = true;
        Close();
    }
    
    private void CancelButton_Click(object sender, RoutedEventArgs e)
    {
        DialogResult = false;
        Close();
    }
}

/// <summary>
/// Flags for reset sections
/// </summary>
[Flags]
public enum ResetSections
{
    None = 0,
    General = 1,
    Appearance = 2,
    Plugins = 4,
    Advanced = 8,
    User = 16
}