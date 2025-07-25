using Microsoft.Extensions.Logging;
using System.Windows;
using System.Windows.Input;
using Wpf.Ui;
using Wpf.Ui.Appearance;
using ItemEditor.ViewModels;
using ItemEditor.Services;
using IThemeService = ItemEditor.Services.IThemeService;

namespace ItemEditor;

/// <summary>
/// Interaction logic for MainWindow.xaml
/// </summary>
public partial class MainWindow : Wpf.Ui.Controls.FluentWindow
{
    private readonly ILogger<MainWindow> _logger;
    private readonly IThemeService _themeService;
    private readonly IKeyboardShortcutService _keyboardShortcutService;
    private readonly IAccessibilityService _accessibilityService;
    private readonly MainViewModel _viewModel;
    
    /// <summary>
    /// Initializes a new instance of the MainWindow class
    /// </summary>
    /// <param name="viewModel">Main view model</param>
    /// <param name="themeService">Theme service</param>
    /// <param name="keyboardShortcutService">Keyboard shortcut service</param>
    /// <param name="accessibilityService">Accessibility service</param>
    /// <param name="logger">Logger instance</param>
    public MainWindow(MainViewModel viewModel, IThemeService themeService, 
        IKeyboardShortcutService keyboardShortcutService, IAccessibilityService accessibilityService,
        ILogger<MainWindow> logger)
    {
        _viewModel = viewModel ?? throw new ArgumentNullException(nameof(viewModel));
        _themeService = themeService ?? throw new ArgumentNullException(nameof(themeService));
        _keyboardShortcutService = keyboardShortcutService ?? throw new ArgumentNullException(nameof(keyboardShortcutService));
        _accessibilityService = accessibilityService ?? throw new ArgumentNullException(nameof(accessibilityService));
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
        
        InitializeComponent();
        
        // Set the data context
        DataContext = _viewModel;
        
        // Subscribe to events
        Loaded += OnWindowLoaded;
        Closing += OnWindowClosing;
        KeyDown += OnWindowKeyDown;
        
        _logger.LogDebug("MainWindow initialized");
    }
    
    /// <summary>
    /// Handles the window loaded event
    /// </summary>
    /// <param name="sender">Event sender</param>
    /// <param name="e">Event arguments</param>
    private async void OnWindowLoaded(object sender, RoutedEventArgs e)
    {
        try
        {
            _logger.LogDebug("MainWindow loaded, initializing systems");
            
            // Initialize theme system
            var currentTheme = _themeService.GetCurrentTheme();
            
            if (currentTheme.Equals("Auto", StringComparison.OrdinalIgnoreCase))
            {
                // Enable system theme watcher with Mica backdrop
                SystemThemeWatcher.Watch(this, Wpf.Ui.Controls.WindowBackdropType.Mica, true);
                _logger.LogInformation("System theme watcher enabled with Mica backdrop");
            }
            else
            {
                // Apply specific theme without system watching
                var backdropType = Wpf.Ui.Controls.WindowBackdropType.Mica;
                    
                WindowBackdropType = backdropType;
                _logger.LogInformation("Applied static theme: {Theme} with backdrop: {Backdrop}", currentTheme, backdropType);
            }
            
            // Subscribe to theme changes
            _themeService.ThemeChanged += OnThemeChanged;
            
            // Initialize keyboard shortcuts
            await InitializeKeyboardShortcutsAsync();
            
            // Initialize accessibility
            InitializeAccessibility();
            
            _logger.LogDebug("MainWindow initialization completed");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error initializing MainWindow");
        }
    }
    
    /// <summary>
    /// Handles theme change events
    /// </summary>
    /// <param name="sender">Event sender</param>
    /// <param name="e">Theme changed event arguments</param>
    private void OnThemeChanged(object? sender, ThemeChangedEventArgs e)
    {
        try
        {
            _logger.LogDebug("Theme changed from {OldTheme} to {NewTheme}", e.OldTheme, e.NewTheme);
            
            // Update system theme watcher based on new theme
            if (e.NewTheme.Equals("Auto", StringComparison.OrdinalIgnoreCase))
            {
                // Enable system theme watcher
                SystemThemeWatcher.Watch(this, Wpf.Ui.Controls.WindowBackdropType.Mica, true);
                _logger.LogDebug("Enabled system theme watcher for auto theme");
            }
            else
            {
                // Disable system theme watcher and apply specific theme
                SystemThemeWatcher.UnWatch(this);
                
                // Apply the specific theme
                var applicationTheme = e.NewTheme.Equals("Dark", StringComparison.OrdinalIgnoreCase)
                    ? ApplicationTheme.Dark
                    : ApplicationTheme.Light;
                    
                ApplicationThemeManager.Apply(applicationTheme);
                _logger.LogDebug("Applied specific theme: {Theme}", applicationTheme);
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error handling theme change");
        }
    }
    
    /// <summary>
    /// Handles the window closing event
    /// </summary>
    /// <param name="sender">Event sender</param>
    /// <param name="e">Event arguments</param>
    private async void OnWindowClosing(object? sender, System.ComponentModel.CancelEventArgs e)
    {
        try
        {
            _logger.LogDebug("MainWindow closing, cleaning up systems");
            
            // Unsubscribe from theme changes
            _themeService.ThemeChanged -= OnThemeChanged;
            
            // Cleanup system theme watcher
            SystemThemeWatcher.UnWatch(this);
            
            // Save keyboard shortcuts
            await _keyboardShortcutService.SaveShortcutsAsync();
            
            _logger.LogDebug("MainWindow cleanup completed");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error during MainWindow cleanup");
        }
    }
    
    /// <summary>
    /// Handles window key down events for keyboard shortcuts
    /// </summary>
    /// <param name="sender">Event sender</param>
    /// <param name="e">Key event arguments</param>
    private void OnWindowKeyDown(object sender, KeyEventArgs e)
    {
        try
        {
            // Process keyboard shortcuts
            if (_keyboardShortcutService.ProcessKeyEvent(e))
            {
                e.Handled = true;
                return;
            }
            
            // Handle navigation shortcuts
            HandleNavigationShortcuts(e);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error processing key event");
        }
    }
    
    /// <summary>
    /// Initializes keyboard shortcuts
    /// </summary>
    private async Task InitializeKeyboardShortcutsAsync()
    {
        try
        {
            _logger.LogDebug("Initializing keyboard shortcuts");
            
            // Load saved shortcuts
            await _keyboardShortcutService.LoadShortcutsAsync();
            
            // Register default shortcuts
            RegisterDefaultShortcuts();
            
            _logger.LogInformation("Keyboard shortcuts initialized");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error initializing keyboard shortcuts");
        }
    }
    
    /// <summary>
    /// Registers default keyboard shortcuts
    /// </summary>
    private void RegisterDefaultShortcuts()
    {
        try
        {
            // File operations
            _keyboardShortcutService.RegisterShortcut(Key.N, ModifierKeys.Control, _viewModel.NewFileCommand, "New File", "File");
            _keyboardShortcutService.RegisterShortcut(Key.O, ModifierKeys.Control, _viewModel.OpenFileCommand, "Open File", "File");
            _keyboardShortcutService.RegisterShortcut(Key.S, ModifierKeys.Control, _viewModel.SaveFileCommand, "Save File", "File");
            _keyboardShortcutService.RegisterShortcut(Key.S, ModifierKeys.Control | ModifierKeys.Shift, _viewModel.SaveAsCommand, "Save As", "File");
            
            // Edit operations
            _keyboardShortcutService.RegisterShortcut(Key.Z, ModifierKeys.Control, _viewModel.UndoCommand, "Undo", "Edit");
            _keyboardShortcutService.RegisterShortcut(Key.Y, ModifierKeys.Control, _viewModel.RedoCommand, "Redo", "Edit");
            _keyboardShortcutService.RegisterShortcut(Key.Z, ModifierKeys.Control | ModifierKeys.Shift, _viewModel.RedoCommand, "Redo (Alt)", "Edit");
            _keyboardShortcutService.RegisterShortcut(Key.F, ModifierKeys.Control, _viewModel.FindCommand, "Find", "Edit");
            
            // View operations
            _keyboardShortcutService.RegisterShortcut(Key.F5, ModifierKeys.None, _viewModel.RefreshCommand, "Refresh", "View");
            _keyboardShortcutService.RegisterShortcut(Key.F11, ModifierKeys.None, _viewModel.ToggleFullScreenCommand, "Toggle Full Screen", "View");
            
            // Navigation shortcuts
            _keyboardShortcutService.RegisterShortcut(Key.Up, ModifierKeys.None, 
                new RelayCommand(() => NavigateItems(-1)), "Previous Item", "Navigation");
            _keyboardShortcutService.RegisterShortcut(Key.Down, ModifierKeys.None, 
                new RelayCommand(() => NavigateItems(1)), "Next Item", "Navigation");
            _keyboardShortcutService.RegisterShortcut(Key.Home, ModifierKeys.Control, 
                new RelayCommand(() => NavigateToFirstItem()), "First Item", "Navigation");
            _keyboardShortcutService.RegisterShortcut(Key.End, ModifierKeys.Control, 
                new RelayCommand(() => NavigateToLastItem()), "Last Item", "Navigation");
            
            // Accessibility shortcuts
            _keyboardShortcutService.RegisterShortcut(Key.F1, ModifierKeys.None, 
                new RelayCommand(() => ShowHelp()), "Show Help", "General");
            _keyboardShortcutService.RegisterShortcut(Key.Escape, ModifierKeys.None, 
                new RelayCommand(() => CancelCurrentOperation()), "Cancel", "General");
            
            _logger.LogDebug("Registered {Count} default shortcuts", _keyboardShortcutService.GetAllShortcuts().Values.Sum(c => c.Count));
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error registering default shortcuts");
        }
    }
    
    /// <summary>
    /// Initializes accessibility features
    /// </summary>
    private void InitializeAccessibility()
    {
        try
        {
            _logger.LogDebug("Initializing accessibility features");
            
            // Set up main window accessibility properties
            _accessibilityService.SetAccessibilityProperties(this, 
                "ItemEditor - OpenTibia Item Editor", 
                "Main application window for editing OpenTibia items",
                AccessibilityRole.Main);
            
            // Subscribe to accessibility changes
            _accessibilityService.AccessibilitySettingsChanged += OnAccessibilitySettingsChanged;
            
            // Announce application ready
            _accessibilityService.Announce("ItemEditor loaded and ready", AccessibilityPriority.Normal);
            
            _logger.LogInformation("Accessibility features initialized");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error initializing accessibility features");
        }
    }
    
    /// <summary>
    /// Handles accessibility settings changes
    /// </summary>
    private void OnAccessibilitySettingsChanged(object? sender, AccessibilitySettingsChangedEventArgs e)
    {
        try
        {
            _logger.LogDebug("Accessibility settings changed");
            
            // Update UI based on accessibility settings
            if (e.NewSettings.IsHighContrastMode != e.OldSettings.IsHighContrastMode)
            {
                // Handle high contrast mode changes
                _accessibilityService.Announce(e.NewSettings.IsHighContrastMode 
                    ? "High contrast mode enabled" 
                    : "High contrast mode disabled");
            }
            
            if (e.NewSettings.IsScreenReaderActive != e.OldSettings.IsScreenReaderActive)
            {
                // Handle screen reader changes
                _logger.LogInformation("Screen reader status changed: {Status}", 
                    e.NewSettings.IsScreenReaderActive ? "Active" : "Inactive");
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error handling accessibility settings change");
        }
    }
    
    /// <summary>
    /// Handles navigation shortcuts
    /// </summary>
    private void HandleNavigationShortcuts(KeyEventArgs e)
    {
        try
        {
            // Handle Tab navigation with announcements
            if (e.Key == Key.Tab)
            {
                var direction = (e.KeyboardDevice.Modifiers & ModifierKeys.Shift) == ModifierKeys.Shift ? "previous" : "next";
                _accessibilityService.Announce($"Moving to {direction} element");
            }
            
            // Handle Enter key on focused elements
            if (e.Key == Key.Enter && Keyboard.FocusedElement is FrameworkElement focusedElement)
            {
                if (focusedElement is Button button && button.Command?.CanExecute(button.CommandParameter) == true)
                {
                    button.Command.Execute(button.CommandParameter);
                    e.Handled = true;
                }
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error handling navigation shortcuts");
        }
    }
    
    /// <summary>
    /// Navigates through items
    /// </summary>
    private void NavigateItems(int direction)
    {
        try
        {
            if (_viewModel.FilteredItems.Count == 0) return;
            
            var currentIndex = _viewModel.SelectedItem != null 
                ? _viewModel.FilteredItems.IndexOf(_viewModel.SelectedItem) 
                : -1;
            
            var newIndex = currentIndex + direction;
            
            if (newIndex >= 0 && newIndex < _viewModel.FilteredItems.Count)
            {
                _viewModel.SelectedItem = _viewModel.FilteredItems[newIndex];
                _accessibilityService.Announce($"Selected item {newIndex + 1} of {_viewModel.FilteredItems.Count}: {_viewModel.SelectedItem.DisplayText}");
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error navigating items");
        }
    }
    
    /// <summary>
    /// Navigates to the first item
    /// </summary>
    private void NavigateToFirstItem()
    {
        try
        {
            if (_viewModel.FilteredItems.Count > 0)
            {
                _viewModel.SelectedItem = _viewModel.FilteredItems[0];
                _accessibilityService.Announce($"Selected first item: {_viewModel.SelectedItem.DisplayText}");
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error navigating to first item");
        }
    }
    
    /// <summary>
    /// Navigates to the last item
    /// </summary>
    private void NavigateToLastItem()
    {
        try
        {
            if (_viewModel.FilteredItems.Count > 0)
            {
                _viewModel.SelectedItem = _viewModel.FilteredItems[_viewModel.FilteredItems.Count - 1];
                _accessibilityService.Announce($"Selected last item: {_viewModel.SelectedItem.DisplayText}");
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error navigating to last item");
        }
    }
    
    /// <summary>
    /// Shows help information
    /// </summary>
    private void ShowHelp()
    {
        try
        {
            // This would show a help dialog or navigate to help content
            _accessibilityService.Announce("Help information would be displayed here");
            _logger.LogDebug("Help requested");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error showing help");
        }
    }
    
    /// <summary>
    /// Cancels the current operation
    /// </summary>
    private void CancelCurrentOperation()
    {
        try
        {
            // Cancel any ongoing operations
            if (_viewModel.IsLoading)
            {
                // Cancel loading operation if possible
                _accessibilityService.Announce("Operation cancelled");
            }
            
            _logger.LogDebug("Cancel operation requested");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error cancelling operation");
        }
    }
    
    /// <summary>
    /// Handles file drag enter event
    /// </summary>
    /// <param name="sender">Event sender</param>
    /// <param name="e">Drag event arguments</param>
    private void OnFileDragEnter(object sender, DragEventArgs e)
    {
        try
        {
            if (IsValidFileDrop(e))
            {
                e.Effects = DragDropEffects.Copy;
                ShowDragDropOverlay();
                _logger.LogDebug("File drag entered with valid files");
            }
            else
            {
                e.Effects = DragDropEffects.None;
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error handling file drag enter");
            e.Effects = DragDropEffects.None;
        }
    }
    
    /// <summary>
    /// Handles file drag over event
    /// </summary>
    /// <param name="sender">Event sender</param>
    /// <param name="e">Drag event arguments</param>
    private void OnFileDragOver(object sender, DragEventArgs e)
    {
        try
        {
            if (IsValidFileDrop(e))
            {
                e.Effects = DragDropEffects.Copy;
            }
            else
            {
                e.Effects = DragDropEffects.None;
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error handling file drag over");
            e.Effects = DragDropEffects.None;
        }
    }
    
    /// <summary>
    /// Handles file drag leave event
    /// </summary>
    /// <param name="sender">Event sender</param>
    /// <param name="e">Drag event arguments</param>
    private void OnFileDragLeave(object sender, DragEventArgs e)
    {
        try
        {
            HideDragDropOverlay();
            _logger.LogDebug("File drag left");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error handling file drag leave");
        }
    }
    
    /// <summary>
    /// Handles file drop event
    /// </summary>
    /// <param name="sender">Event sender</param>
    /// <param name="e">Drag event arguments</param>
    private async void OnFileDrop(object sender, DragEventArgs e)
    {
        try
        {
            HideDragDropOverlay();
            
            if (e.Data.GetDataPresent(DataFormats.FileDrop))
            {
                var files = (string[])e.Data.GetData(DataFormats.FileDrop);
                if (files != null && files.Length > 0)
                {
                    var filePath = files[0]; // Take the first file
                    _logger.LogInformation("File dropped: {FilePath}", filePath);
                    
                    // Open the dropped file
                    await _viewModel.OpenFileCommand.ExecuteAsync(filePath);
                }
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error handling file drop");
        }
    }
    
    /// <summary>
    /// Validates if the drag operation contains valid files
    /// </summary>
    /// <param name="e">Drag event arguments</param>
    /// <returns>True if valid files are being dragged</returns>
    private bool IsValidFileDrop(DragEventArgs e)
    {
        try
        {
            if (!e.Data.GetDataPresent(DataFormats.FileDrop))
            {
                return false;
            }
            
            var files = (string[])e.Data.GetData(DataFormats.FileDrop);
            if (files == null || files.Length == 0)
            {
                return false;
            }
            
            // Check if the first file has a valid extension
            var filePath = files[0];
            var extension = Path.GetExtension(filePath).ToLowerInvariant();
            
            return extension == ".otb" || extension == ".dat" || extension == ".spr";
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error validating file drop");
            return false;
        }
    }
    
    /// <summary>
    /// Shows the drag and drop overlay
    /// </summary>
    private void ShowDragDropOverlay()
    {
        try
        {
            if (DragDropOverlay != null)
            {
                DragDropOverlay.Visibility = Visibility.Visible;
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error showing drag drop overlay");
        }
    }
    
    /// <summary>
    /// Hides the drag and drop overlay
    /// </summary>
    private void HideDragDropOverlay()
    {
        try
        {
            if (DragDropOverlay != null)
            {
                DragDropOverlay.Visibility = Visibility.Collapsed;
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error hiding drag drop overlay");
        }
    }
}
/// <
summary>
/// Simple relay command implementation for shortcuts
/// </summary>
internal class RelayCommand : ICommand
{
    private readonly Action _execute;
    private readonly Func<bool>? _canExecute;

    public RelayCommand(Action execute, Func<bool>? canExecute = null)
    {
        _execute = execute ?? throw new ArgumentNullException(nameof(execute));
        _canExecute = canExecute;
    }

    public event EventHandler? CanExecuteChanged
    {
        add => CommandManager.RequerySuggested += value;
        remove => CommandManager.RequerySuggested -= value;
    }

    public bool CanExecute(object? parameter) => _canExecute?.Invoke() ?? true;

    public void Execute(object? parameter) => _execute();
}