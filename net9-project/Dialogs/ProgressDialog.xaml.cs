using System.Windows;

namespace ItemEditor.Dialogs;

/// <summary>
/// Progress dialog for long-running operations
/// </summary>
public partial class ProgressDialog : Wpf.Ui.Controls.FluentWindow
{
    private CancellationTokenSource? _cancellationTokenSource;
    
    /// <summary>
    /// Gets the cancellation token for the operation
    /// </summary>
    public CancellationToken CancellationToken => _cancellationTokenSource?.Token ?? CancellationToken.None;
    
    /// <summary>
    /// Gets or sets whether the operation was cancelled
    /// </summary>
    public bool IsCancelled { get; private set; }
    
    /// <summary>
    /// Initializes a new instance of the ProgressDialog class
    /// </summary>
    /// <param name="title">Dialog title</param>
    /// <param name="message">Initial message</param>
    /// <param name="cancellationTokenSource">Cancellation token source</param>
    public ProgressDialog(string title = "Progress", string message = "Please wait...", CancellationTokenSource? cancellationTokenSource = null)
    {
        InitializeComponent();
        
        _cancellationTokenSource = cancellationTokenSource;
        
        TitleText.Text = title;
        StatusText.Text = message;
        
        // Hide cancel button if no cancellation token provided
        if (_cancellationTokenSource == null)
        {
            CancelButton.Visibility = Visibility.Collapsed;
        }
    }
    
    /// <summary>
    /// Updates the progress dialog status
    /// </summary>
    /// <param name="message">Status message</param>
    /// <param name="progress">Progress percentage (0-100), null for indeterminate</param>
    public void UpdateProgress(string message, double? progress = null)
    {
        Dispatcher.Invoke(() =>
        {
            StatusText.Text = message;
            
            if (progress.HasValue)
            {
                ProgressBar.IsIndeterminate = false;
                ProgressBar.Value = Math.Max(0, Math.Min(100, progress.Value));
            }
            else
            {
                ProgressBar.IsIndeterminate = true;
            }
        });
    }
    
    /// <summary>
    /// Handles the cancel button click
    /// </summary>
    /// <param name="sender">Event sender</param>
    /// <param name="e">Event arguments</param>
    private void OnCancelClick(object sender, RoutedEventArgs e)
    {
        try
        {
            IsCancelled = true;
            _cancellationTokenSource?.Cancel();
            
            CancelButton.IsEnabled = false;
            CancelButton.Content = "Cancelling...";
            
            UpdateProgress("Cancelling operation...");
        }
        catch (Exception ex)
        {
            // Log error but don't throw to avoid crashing the dialog
            System.Diagnostics.Debug.WriteLine($"Error cancelling operation: {ex.Message}");
        }
    }
    
    /// <summary>
    /// Closes the dialog with success result
    /// </summary>
    public void CloseWithSuccess()
    {
        Dispatcher.Invoke(() =>
        {
            DialogResult = true;
            Close();
        });
    }
    
    /// <summary>
    /// Closes the dialog with error result
    /// </summary>
    public void CloseWithError()
    {
        Dispatcher.Invoke(() =>
        {
            DialogResult = false;
            Close();
        });
    }
}