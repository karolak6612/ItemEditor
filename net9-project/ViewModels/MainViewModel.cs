using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Microsoft.Extensions.Logging;
using System.Collections.ObjectModel;
using ItemEditor.Models;
using ItemEditor.Services;
using Microsoft.Win32;
using ItemEditor.Dialogs;
using System.Windows;

namespace ItemEditor.ViewModels;

/// <summary>
/// Main view model for the application
/// </summary>
public partial class MainViewModel : ObservableObject
{
    private readonly IFileService _fileService;
    private readonly IPluginService _pluginService;
    private readonly ILogger<MainViewModel> _logger;
    
    [ObservableProperty]
    private ObservableCollection<Item> _items = new();
    
    [ObservableProperty]
    private Item? _selectedItem;
    
    [ObservableProperty]
    private bool _isLoading;
    
    [ObservableProperty]
    private string _statusText = "Ready";
    
    [ObservableProperty]
    private string _loadingText = "Loading...";
    
    [ObservableProperty]
    private string _loadingSubText = "";
    
    [ObservableProperty]
    private string _searchText = "";
    
    [ObservableProperty]
    private string? _currentFilePath;
    
    [ObservableProperty]
    private ObservableCollection<string> _recentFiles = new();
    
    [ObservableProperty]
    private ObservableCollection<Item> _filteredItems = new();
    
    /// <summary>
    /// Gets the current file name from the file path
    /// </summary>
    public string? CurrentFileName => string.IsNullOrEmpty(CurrentFilePath) 
        ? null 
        : Path.GetFileName(CurrentFilePath);
    
    /// <summary>
    /// Initializes a new instance of the MainViewModel class
    /// </summary>
    /// <param name="fileService">File service</param>
    /// <param name="pluginService">Plugin service</param>
    /// <param name="logger">Logger instance</param>
    public MainViewModel(
        IFileService fileService,
        IPluginService pluginService,
        ILogger<MainViewModel> logger)
    {
        _fileService = fileService;
        _pluginService = pluginService;
        _logger = logger;
        
        // Initialize filtered items
        FilteredItems = new ObservableCollection<Item>(Items);
        
        // Set up search filtering
        PropertyChanged += OnPropertyChanged;
        
        _logger.LogInformation("MainViewModel initialized");
    }
    
    /// <summary>
    /// Handles property changes for search filtering
    /// </summary>
    private void OnPropertyChanged(object? sender, System.ComponentModel.PropertyChangedEventArgs e)
    {
        if (e.PropertyName == nameof(SearchText) || e.PropertyName == nameof(Items))
        {
            FilterItems();
        }
        
        if (e.PropertyName == nameof(CurrentFilePath))
        {
            OnPropertyChanged(nameof(CurrentFileName));
        }
    }
    
    /// <summary>
    /// Filters items based on search text
    /// </summary>
    private void FilterItems()
    {
        FilteredItems.Clear();
        
        var filteredItems = string.IsNullOrWhiteSpace(SearchText)
            ? Items
            : Items.Where(item => 
                item.Name.Contains(SearchText, StringComparison.OrdinalIgnoreCase) ||
                item.Id.ToString().Contains(SearchText));
        
        foreach (var item in filteredItems)
        {
            FilteredItems.Add(item);
        }
    }
    
    /// <summary>
    /// Command to open a file
    /// </summary>
    [RelayCommand]
    private async Task OpenFileAsync(string? filePath = null)
    {
        // If no file path provided, show file dialog
        if (string.IsNullOrEmpty(filePath))
        {
            var openFileDialog = new OpenFileDialog
            {
                Title = "Open ItemEditor File",
                Filter = "All Supported Files (*.otb;*.dat;*.spr)|*.otb;*.dat;*.spr|" +
                        "OpenTibia Binary Files (*.otb)|*.otb|" +
                        "Data Files (*.dat)|*.dat|" +
                        "Sprite Files (*.spr)|*.spr|" +
                        "All Files (*.*)|*.*",
                FilterIndex = 1,
                CheckFileExists = true,
                CheckPathExists = true,
                Multiselect = false
            };
            
            // Set initial directory to recent files location if available
            if (RecentFiles.Count > 0 && File.Exists(RecentFiles[0]))
            {
                openFileDialog.InitialDirectory = Path.GetDirectoryName(RecentFiles[0]);
            }
            
            if (openFileDialog.ShowDialog() != true)
            {
                return;
            }
            
            filePath = openFileDialog.FileName;
        }
        
        if (string.IsNullOrEmpty(filePath))
        {
            _logger.LogWarning("No file path selected");
            return;
        }
        
        // Check file size to determine if we need a progress dialog
        var fileInfo = new FileInfo(filePath);
        var useProgressDialog = fileInfo.Length > 1024 * 1024; // 1MB threshold
        
        ProgressDialog? progressDialog = null;
        CancellationTokenSource? cancellationTokenSource = null;
        
        try
        {
            if (useProgressDialog)
            {
                cancellationTokenSource = new CancellationTokenSource();
                progressDialog = new ProgressDialog(
                    "Loading File", 
                    $"Loading {Path.GetFileName(filePath)}...", 
                    cancellationTokenSource)
                {
                    Owner = Application.Current.MainWindow
                };
                
                // Show progress dialog asynchronously
                progressDialog.Show();
            }
            else
            {
                IsLoading = true;
                LoadingText = "Loading file...";
                LoadingSubText = Path.GetFileName(filePath);
            }
            
            StatusText = $"Loading {Path.GetFileName(filePath)}...";
            _logger.LogInformation("Opening file: {FilePath}", filePath);
            
            var items = await _fileService.LoadItemsAsync(filePath, cancellationTokenSource?.Token ?? CancellationToken.None);
            
            // Check if operation was cancelled
            if (cancellationTokenSource?.Token.IsCancellationRequested == true)
            {
                StatusText = "File loading cancelled";
                return;
            }
            
            Items.Clear();
            foreach (var item in items)
            {
                Items.Add(item);
            }
            
            // Update current file path
            CurrentFilePath = filePath;
            
            // Add to recent files
            AddToRecentFiles(filePath);
            
            StatusText = $"Loaded {Items.Count} items from {Path.GetFileName(filePath)}";
            _logger.LogInformation("Successfully loaded {ItemCount} items", Items.Count);
            
            progressDialog?.CloseWithSuccess();
        }
        catch (OperationCanceledException)
        {
            _logger.LogInformation("File loading cancelled: {FilePath}", filePath);
            StatusText = "File loading cancelled";
            progressDialog?.CloseWithError();
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error opening file: {FilePath}", filePath);
            StatusText = $"Error loading file: {ex.Message}";
            progressDialog?.CloseWithError();
        }
        finally
        {
            IsLoading = false;
            LoadingText = "Loading...";
            LoadingSubText = "";
            cancellationTokenSource?.Dispose();
        }
    }
    
    /// <summary>
    /// Command to save the current file
    /// </summary>
    [RelayCommand]
    private async Task SaveFileAsync(string? filePath = null)
    {
        var targetPath = filePath ?? CurrentFilePath;
        
        // If no file path available, show save dialog
        if (string.IsNullOrEmpty(targetPath))
        {
            var saveFileDialog = new SaveFileDialog
            {
                Title = "Save ItemEditor File",
                Filter = "OpenTibia Binary Files (*.otb)|*.otb|" +
                        "Data Files (*.dat)|*.dat|" +
                        "Sprite Files (*.spr)|*.spr|" +
                        "All Files (*.*)|*.*",
                FilterIndex = 1,
                CheckPathExists = true,
                OverwritePrompt = true,
                AddExtension = true,
                DefaultExt = "otb"
            };
            
            // Set initial directory to recent files location if available
            if (RecentFiles.Count > 0 && File.Exists(RecentFiles[0]))
            {
                saveFileDialog.InitialDirectory = Path.GetDirectoryName(RecentFiles[0]);
            }
            
            if (saveFileDialog.ShowDialog() != true)
            {
                return;
            }
            
            targetPath = saveFileDialog.FileName;
        }
        
        // Use progress dialog for large item collections
        var useProgressDialog = Items.Count > 1000;
        
        ProgressDialog? progressDialog = null;
        CancellationTokenSource? cancellationTokenSource = null;
        
        try
        {
            if (useProgressDialog)
            {
                cancellationTokenSource = new CancellationTokenSource();
                progressDialog = new ProgressDialog(
                    "Saving File", 
                    $"Saving {Path.GetFileName(targetPath)}...", 
                    cancellationTokenSource)
                {
                    Owner = Application.Current.MainWindow
                };
                
                progressDialog.Show();
            }
            else
            {
                IsLoading = true;
                LoadingText = "Saving file...";
                LoadingSubText = Path.GetFileName(targetPath);
            }
            
            StatusText = $"Saving {Path.GetFileName(targetPath)}...";
            _logger.LogInformation("Saving file: {FilePath}", targetPath);
            
            await _fileService.SaveItemsAsync(targetPath, Items, cancellationTokenSource?.Token ?? CancellationToken.None);
            
            // Check if operation was cancelled
            if (cancellationTokenSource?.Token.IsCancellationRequested == true)
            {
                StatusText = "File saving cancelled";
                return;
            }
            
            // Update current file path if it was provided
            if (!string.IsNullOrEmpty(filePath))
            {
                CurrentFilePath = filePath;
                AddToRecentFiles(filePath);
            }
            
            StatusText = $"Saved {Items.Count} items to {Path.GetFileName(targetPath)}";
            _logger.LogInformation("Successfully saved {ItemCount} items", Items.Count);
            
            progressDialog?.CloseWithSuccess();
        }
        catch (OperationCanceledException)
        {
            _logger.LogInformation("File saving cancelled: {FilePath}", targetPath);
            StatusText = "File saving cancelled";
            progressDialog?.CloseWithError();
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error saving file: {FilePath}", targetPath);
            StatusText = $"Error saving file: {ex.Message}";
            progressDialog?.CloseWithError();
        }
        finally
        {
            IsLoading = false;
            LoadingText = "Loading...";
            LoadingSubText = "";
            cancellationTokenSource?.Dispose();
        }
    }
    
    /// <summary>
    /// Command to save the current file with a new name
    /// </summary>
    [RelayCommand]
    private async Task SaveAsAsync()
    {
        var saveFileDialog = new SaveFileDialog
        {
            Title = "Save ItemEditor File As",
            Filter = "OpenTibia Binary Files (*.otb)|*.otb|" +
                    "Data Files (*.dat)|*.dat|" +
                    "Sprite Files (*.spr)|*.spr|" +
                    "All Files (*.*)|*.*",
            FilterIndex = 1,
            CheckPathExists = true,
            OverwritePrompt = true,
            AddExtension = true,
            DefaultExt = "otb"
        };
        
        // Set initial directory and filename
        if (!string.IsNullOrEmpty(CurrentFilePath))
        {
            saveFileDialog.InitialDirectory = Path.GetDirectoryName(CurrentFilePath);
            saveFileDialog.FileName = Path.GetFileName(CurrentFilePath);
        }
        else if (RecentFiles.Count > 0 && File.Exists(RecentFiles[0]))
        {
            saveFileDialog.InitialDirectory = Path.GetDirectoryName(RecentFiles[0]);
        }
        
        if (saveFileDialog.ShowDialog() == true)
        {
            await SaveFileAsync(saveFileDialog.FileName);
        }
    }
    
    /// <summary>
    /// Adds a file path to the recent files list
    /// </summary>
    private void AddToRecentFiles(string filePath)
    {
        try
        {
            // Remove if already exists
            if (RecentFiles.Contains(filePath))
            {
                RecentFiles.Remove(filePath);
            }
            
            // Add to beginning
            RecentFiles.Insert(0, filePath);
            
            // Keep only the most recent 10 files
            while (RecentFiles.Count > 10)
            {
                RecentFiles.RemoveAt(RecentFiles.Count - 1);
            }
            
            _logger.LogDebug("Added {FilePath} to recent files", filePath);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error adding file to recent files: {FilePath}", filePath);
        }
    }
    
    /// <summary>
    /// Command to create a new file
    /// </summary>
    [RelayCommand]
    private void NewFile()
    {
        try
        {
            _logger.LogInformation("Creating new file");
            
            Items.Clear();
            SelectedItem = null;
            CurrentFilePath = null;
            SearchText = "";
            StatusText = "New file created";
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error creating new file");
            StatusText = $"Error creating new file: {ex.Message}";
        }
    }
    
    /// <summary>
    /// Command to exit the application
    /// </summary>
    [RelayCommand]
    private void Exit()
    {
        try
        {
            _logger.LogInformation("Exiting application");
            System.Windows.Application.Current.Shutdown();
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error during application exit");
        }
    }
    
    /// <summary>
    /// Command to refresh the current view
    /// </summary>
    [RelayCommand]
    private async Task RefreshAsync()
    {
        try
        {
            if (!string.IsNullOrEmpty(CurrentFilePath))
            {
                _logger.LogInformation("Refreshing current file");
                await OpenFileAsync(CurrentFilePath);
            }
            else
            {
                StatusText = "No file to refresh";
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error refreshing file");
            StatusText = $"Error refreshing: {ex.Message}";
        }
    }
    
    /// <summary>
    /// Command to show find dialog
    /// </summary>
    [RelayCommand]
    private void Find()
    {
        try
        {
            _logger.LogInformation("Opening find dialog");
            // TODO: Implement find dialog in future task
            StatusText = "Find functionality will be implemented in a future update";
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error opening find dialog");
            StatusText = $"Error opening find: {ex.Message}";
        }
    }
    
    /// <summary>
    /// Command to toggle theme
    /// </summary>
    [RelayCommand]
    private void ToggleTheme()
    {
        try
        {
            _logger.LogInformation("Toggling theme");
            // TODO: Implement theme toggle through theme service
            StatusText = "Theme toggle functionality will be implemented";
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error toggling theme");
            StatusText = $"Error toggling theme: {ex.Message}";
        }
    }
    
    /// <summary>
    /// Command to set specific theme
    /// </summary>
    [RelayCommand]
    private void SetTheme(string theme)
    {
        try
        {
            _logger.LogInformation("Setting theme to {Theme}", theme);
            // TODO: Implement theme setting through theme service
            StatusText = $"Theme set to {theme}";
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error setting theme to {Theme}", theme);
            StatusText = $"Error setting theme: {ex.Message}";
        }
    }
    
    /// <summary>
    /// Command to edit an item
    /// </summary>
    [RelayCommand]
    private void EditItem(Item item)
    {
        try
        {
            if (item != null)
            {
                SelectedItem = item;
                _logger.LogInformation("Editing item: {ItemName} (ID: {ItemId})", item.Name, item.Id);
                StatusText = $"Editing item: {item.Name}";
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error editing item");
            StatusText = $"Error editing item: {ex.Message}";
        }
    }
    
    /// <summary>
    /// Command to duplicate an item
    /// </summary>
    [RelayCommand]
    private void DuplicateItem(Item item)
    {
        try
        {
            if (item != null)
            {
                // Create a copy of the item with a new ID
                var duplicatedItem = new Item
                {
                    Name = $"{item.Name} (Copy)",
                    Id = (ushort)(Items.Count > 0 ? Items.Max(i => i.Id) + 1 : 1),
                    Type = item.Type,
                    IsStackable = item.IsStackable,
                    Speed = item.Speed,
                    LightLevel = item.LightLevel,
                    LightColor = item.LightColor,
                    StackOrder = item.StackOrder
                };
                
                Items.Add(duplicatedItem);
                SelectedItem = duplicatedItem;
                
                _logger.LogInformation("Duplicated item: {ItemName} (ID: {ItemId})", item.Name, item.Id);
                StatusText = $"Duplicated item: {item.Name}";
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error duplicating item");
            StatusText = $"Error duplicating item: {ex.Message}";
        }
    }
    
    /// <summary>
    /// Command to delete an item
    /// </summary>
    [RelayCommand]
    private void DeleteItem(Item item)
    {
        try
        {
            if (item != null)
            {
                Items.Remove(item);
                
                if (SelectedItem == item)
                {
                    SelectedItem = null;
                }
                
                _logger.LogInformation("Deleted item: {ItemName} (ID: {ItemId})", item.Name, item.Id);
                StatusText = $"Deleted item: {item.Name}";
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error deleting item");
            StatusText = $"Error deleting item: {ex.Message}";
        }
    }
    
    /// <summary>
    /// Command to show item properties
    /// </summary>
    [RelayCommand]
    private void ShowItemProperties(Item item)
    {
        try
        {
            if (item != null)
            {
                SelectedItem = item;
                _logger.LogInformation("Showing properties for item: {ItemName} (ID: {ItemId})", item.Name, item.Id);
                StatusText = $"Showing properties for: {item.Name}";
                // TODO: Implement properties dialog in future task
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error showing item properties");
            StatusText = $"Error showing properties: {ex.Message}";
        }
    }
}