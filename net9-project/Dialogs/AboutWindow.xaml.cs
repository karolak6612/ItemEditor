using ItemEditor.ViewModels;
using System.Text;
using System.Windows;

namespace ItemEditor.Dialogs;

/// <summary>
/// About dialog window with application information, system details, license, and contributors
/// </summary>
public partial class AboutWindow : Wpf.Ui.Controls.FluentWindow
{
    private AboutViewModel ViewModel => (AboutViewModel)DataContext;
    
    /// <summary>
    /// Initializes a new instance of the AboutWindow class
    /// </summary>
    public AboutWindow()
    {
        InitializeComponent();
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
    
    /// <summary>
    /// Handles the Copy System Info button click
    /// </summary>
    /// <param name="sender">Event sender</param>
    /// <param name="e">Event arguments</param>
    private void OnCopySystemInfoClick(object sender, RoutedEventArgs e)
    {
        try
        {
            var systemInfo = BuildSystemInfoString();
            Clipboard.SetText(systemInfo);
            
            // Show a brief confirmation (you could use a toast notification here)
            var button = sender as Wpf.Ui.Controls.Button;
            if (button != null)
            {
                var originalContent = button.Content;
                button.Content = "Copied!";
                button.IsEnabled = false;
                
                // Reset button after 2 seconds
                var timer = new System.Windows.Threading.DispatcherTimer
                {
                    Interval = TimeSpan.FromSeconds(2)
                };
                timer.Tick += (s, args) =>
                {
                    button.Content = originalContent;
                    button.IsEnabled = true;
                    timer.Stop();
                };
                timer.Start();
            }
        }
        catch (Exception ex)
        {
            MessageBox.Show($"Failed to copy system information: {ex.Message}", 
                          "Error", MessageBoxButton.OK, MessageBoxImage.Warning);
        }
    }
    
    /// <summary>
    /// Builds a formatted string with system information
    /// </summary>
    /// <returns>Formatted system information string</returns>
    private string BuildSystemInfoString()
    {
        var sb = new StringBuilder();
        
        sb.AppendLine($"{ViewModel.ApplicationName} - System Information");
        sb.AppendLine(new string('=', 50));
        sb.AppendLine();
        
        sb.AppendLine("Application Information:");
        sb.AppendLine($"  Name: {ViewModel.ApplicationName}");
        sb.AppendLine($"  Version: {ViewModel.Version}");
        sb.AppendLine($"  Build Date: {ViewModel.BuildDate}");
        sb.AppendLine($"  Copyright: {ViewModel.Copyright}");
        sb.AppendLine();
        
        sb.AppendLine("System Information:");
        sb.AppendLine($"  .NET Version: {ViewModel.FrameworkVersion}");
        sb.AppendLine($"  Operating System: {ViewModel.OperatingSystem}");
        sb.AppendLine($"  Architecture: {ViewModel.Architecture}");
        sb.AppendLine();
        
        sb.AppendLine("Key Dependencies:");
        sb.AppendLine("  WPF-UI: 3.0.5");
        sb.AppendLine("  CommunityToolkit.Mvvm: 8.2.2");
        sb.AppendLine("  Microsoft.Extensions.Hosting: 9.0.0");
        sb.AppendLine();
        
        sb.AppendLine($"Generated on: {DateTime.Now:yyyy-MM-dd HH:mm:ss}");
        
        return sb.ToString();
    }
}