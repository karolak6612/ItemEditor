using ItemEditor.Models;
using ItemEditor.ViewModels;
using System.Windows;

namespace ItemEditor.Dialogs;

/// <summary>
/// Modern Find Item dialog with advanced search capabilities and result navigation
/// </summary>
public partial class FindItemWindow : Wpf.Ui.Controls.FluentWindow
{
    private FindItemViewModel ViewModel => (FindItemViewModel)DataContext;
    
    /// <summary>
    /// Gets the selected item result
    /// </summary>
    public ItemViewModel? SelectedItem { get; private set; }
    
    /// <summary>
    /// Initializes a new instance of the FindItemWindow class
    /// </summary>
    public FindItemWindow()
    {
        InitializeComponent();
    }
    
    /// <summary>
    /// Initializes a new instance of the FindItemWindow class with items to search
    /// </summary>
    /// <param name="items">Items to search through</param>
    public FindItemWindow(IEnumerable<Item> items) : this()
    {
        ViewModel.SetItems(items);
    }
    
    /// <summary>
    /// Handles the Select and Close button click
    /// </summary>
    /// <param name="sender">Event sender</param>
    /// <param name="e">Event arguments</param>
    private void OnSelectAndCloseClick(object sender, RoutedEventArgs e)
    {
        SelectedItem = ViewModel.SelectedResult;
        DialogResult = true;
        Close();
    }
    
    /// <summary>
    /// Handles the Close button click
    /// </summary>
    /// <param name="sender">Event sender</param>
    /// <param name="e">Event arguments</param>
    private void OnCloseClick(object sender, RoutedEventArgs e)
    {
        DialogResult = false;
        Close();
    }
}