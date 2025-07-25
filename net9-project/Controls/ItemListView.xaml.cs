using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using ItemEditor.ViewModels;
using ItemEditor.Services;
using ItemEditor.Models;
using Microsoft.Extensions.Logging;

namespace ItemEditor.Controls;

/// <summary>
/// Modern item list control with virtualization and WPFUI styling
/// </summary>
public partial class ItemListView : UserControl
{
    #region Dependency Properties

    /// <summary>
    /// Items source dependency property
    /// </summary>
    public static readonly DependencyProperty ItemsSourceProperty =
        DependencyProperty.Register(
            nameof(ItemsSource),
            typeof(ObservableCollection<ItemViewModel>),
            typeof(ItemListView),
            new PropertyMetadata(null, OnItemsSourceChanged));

    /// <summary>
    /// Selected item dependency property
    /// </summary>
    public static readonly DependencyProperty SelectedItemProperty =
        DependencyProperty.Register(
            nameof(SelectedItem),
            typeof(ItemViewModel),
            typeof(ItemListView),
            new FrameworkPropertyMetadata(null, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, OnSelectedItemChanged));

    /// <summary>
    /// Selected items dependency property
    /// </summary>
    public static readonly DependencyProperty SelectedItemsProperty =
        DependencyProperty.Register(
            nameof(SelectedItems),
            typeof(ObservableCollection<ItemViewModel>),
            typeof(ItemListView),
            new PropertyMetadata(new ObservableCollection<ItemViewModel>()));

    /// <summary>
    /// Search text dependency property
    /// </summary>
    public static readonly DependencyProperty SearchTextProperty =
        DependencyProperty.Register(
            nameof(SearchText),
            typeof(string),
            typeof(ItemListView),
            new PropertyMetadata(string.Empty, OnSearchTextChanged));

    /// <summary>
    /// Show thumbnails dependency property
    /// </summary>
    public static readonly DependencyProperty ShowThumbnailsProperty =
        DependencyProperty.Register(
            nameof(ShowThumbnails),
            typeof(bool),
            typeof(ItemListView),
            new PropertyMetadata(true));

    /// <summary>
    /// Selection mode dependency property
    /// </summary>
    public static readonly DependencyProperty SelectionModeProperty =
        DependencyProperty.Register(
            nameof(SelectionMode),
            typeof(SelectionMode),
            typeof(ItemListView),
            new PropertyMetadata(SelectionMode.Single));

    /// <summary>
    /// Is loading dependency property
    /// </summary>
    public static readonly DependencyProperty IsLoadingProperty =
        DependencyProperty.Register(
            nameof(IsLoading),
            typeof(bool),
            typeof(ItemListView),
            new PropertyMetadata(false));

    /// <summary>
    /// Loading message dependency property
    /// </summary>
    public static readonly DependencyProperty LoadingMessageProperty =
        DependencyProperty.Register(
            nameof(LoadingMessage),
            typeof(string),
            typeof(ItemListView),
            new PropertyMetadata("Loading items..."));

    /// <summary>
    /// Status message dependency property
    /// </summary>
    public static readonly DependencyProperty StatusMessageProperty =
        DependencyProperty.Register(
            nameof(StatusMessage),
            typeof(string),
            typeof(ItemListView),
            new PropertyMetadata(string.Empty));

    #endregion

    #region Properties

    /// <summary>
    /// Gets or sets the items source
    /// </summary>
    public ObservableCollection<ItemViewModel>? ItemsSource
    {
        get => (ObservableCollection<ItemViewModel>?)GetValue(ItemsSourceProperty);
        set => SetValue(ItemsSourceProperty, value);
    }

    /// <summary>
    /// Gets or sets the selected item
    /// </summary>
    public ItemViewModel? SelectedItem
    {
        get => (ItemViewModel?)GetValue(SelectedItemProperty);
        set => SetValue(SelectedItemProperty, value);
    }

    /// <summary>
    /// Gets or sets the selected items
    /// </summary>
    public ObservableCollection<ItemViewModel> SelectedItems
    {
        get => (ObservableCollection<ItemViewModel>)GetValue(SelectedItemsProperty);
        set => SetValue(SelectedItemsProperty, value);
    }

    /// <summary>
    /// Gets or sets the search text
    /// </summary>
    public string SearchText
    {
        get => (string)GetValue(SearchTextProperty);
        set => SetValue(SearchTextProperty, value);
    }

    /// <summary>
    /// Gets or sets whether to show thumbnails
    /// </summary>
    public bool ShowThumbnails
    {
        get => (bool)GetValue(ShowThumbnailsProperty);
        set => SetValue(ShowThumbnailsProperty, value);
    }

    /// <summary>
    /// Gets or sets the selection mode
    /// </summary>
    public SelectionMode SelectionMode
    {
        get => (SelectionMode)GetValue(SelectionModeProperty);
        set => SetValue(SelectionModeProperty, value);
    }

    /// <summary>
    /// Gets or sets whether the control is loading
    /// </summary>
    public bool IsLoading
    {
        get => (bool)GetValue(IsLoadingProperty);
        set => SetValue(IsLoadingProperty, value);
    }

    /// <summary>
    /// Gets or sets the loading message
    /// </summary>
    public string LoadingMessage
    {
        get => (string)GetValue(LoadingMessageProperty);
        set => SetValue(LoadingMessageProperty, value);
    }

    /// <summary>
    /// Gets or sets the status message
    /// </summary>
    public string StatusMessage
    {
        get => (string)GetValue(StatusMessageProperty);
        set => SetValue(StatusMessageProperty, value);
    }

    /// <summary>
    /// Gets the filtered items collection
    /// </summary>
    public ObservableCollection<ItemViewModel> FilteredItems { get; } = new();

    /// <summary>
    /// Gets the total number of items
    /// </summary>
    public int TotalItems => ItemsSource?.Count ?? 0;

    #endregion

    #region Commands

    /// <summary>
    /// Command to refresh all thumbnails
    /// </summary>
    public ICommand RefreshAllCommand { get; }

    /// <summary>
    /// Command to delete an item
    /// </summary>
    public ICommand DeleteItemCommand { get; }

    /// <summary>
    /// Command to show item properties
    /// </summary>
    public ICommand ShowPropertiesCommand { get; }

    #endregion

    #region Events

    /// <summary>
    /// Event raised when an item is double-clicked
    /// </summary>
    public event EventHandler<ItemViewModel>? ItemDoubleClicked;

    /// <summary>
    /// Event raised when the selection changes
    /// </summary>
    public event EventHandler<SelectionChangedEventArgs>? SelectionChanged;

    /// <summary>
    /// Event raised when an item is requested to be deleted
    /// </summary>
    public event EventHandler<ItemViewModel>? ItemDeleteRequested;

    /// <summary>
    /// Event raised when item properties are requested to be shown
    /// </summary>
    public event EventHandler<ItemViewModel>? ItemPropertiesRequested;

    #endregion

    #region Fields

    private readonly IDragDropService? _dragDropService;
    private readonly ILogger<ItemListView>? _logger;
    private Point _dragStartPoint;
    private bool _isDragging;

    #endregion

    #region Constructor

    /// <summary>
    /// Initializes a new instance of the ItemListView class
    /// </summary>
    public ItemListView()
    {
        InitializeComponent();

        // Initialize commands
        RefreshAllCommand = new RelayCommand(RefreshAllThumbnails, CanRefreshAll);
        DeleteItemCommand = new RelayCommand<ItemViewModel>(DeleteItem, CanDeleteItem);
        ShowPropertiesCommand = new RelayCommand<ItemViewModel>(ShowProperties, CanShowProperties);

        // Set up event handlers
        ItemsListView.SelectionChanged += OnListViewSelectionChanged;
        ItemsListView.MouseDoubleClick += OnListViewMouseDoubleClick;
        
        // Set up drag and drop event handlers
        ItemsListView.PreviewMouseLeftButtonDown += OnPreviewMouseLeftButtonDown;
        ItemsListView.PreviewMouseMove += OnPreviewMouseMove;
        ItemsListView.PreviewMouseLeftButtonUp += OnPreviewMouseLeftButtonUp;
        ItemsListView.AllowDrop = true;
        ItemsListView.Drop += OnDrop;
        ItemsListView.DragOver += OnDragOver;
        ItemsListView.DragEnter += OnDragEnter;
        ItemsListView.DragLeave += OnDragLeave;

        // Set data context to self for binding
        DataContext = this;
    }

    /// <summary>
    /// Initializes a new instance of the ItemListView class with dependencies
    /// </summary>
    public ItemListView(IDragDropService dragDropService, ILogger<ItemListView> logger) : this()
    {
        _dragDropService = dragDropService;
        _logger = logger;
    }

    #endregion

    #region Event Handlers

    private static void OnItemsSourceChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        if (d is ItemListView control)
        {
            control.OnItemsSourceChanged(e.OldValue as ObservableCollection<ItemViewModel>, 
                                       e.NewValue as ObservableCollection<ItemViewModel>);
        }
    }

    private static void OnSelectedItemChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        if (d is ItemListView control)
        {
            control.OnSelectedItemChanged(e.OldValue as ItemViewModel, e.NewValue as ItemViewModel);
        }
    }

    private static void OnSearchTextChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        if (d is ItemListView control)
        {
            control.ApplyFilter();
        }
    }

    private void OnItemsSourceChanged(ObservableCollection<ItemViewModel>? oldValue, ObservableCollection<ItemViewModel>? newValue)
    {
        // Unsubscribe from old collection
        if (oldValue != null)
        {
            oldValue.CollectionChanged -= OnItemsCollectionChanged;
            foreach (var item in oldValue)
            {
                item.PropertyChanged -= OnItemPropertyChanged;
            }
        }

        // Subscribe to new collection
        if (newValue != null)
        {
            newValue.CollectionChanged += OnItemsCollectionChanged;
            foreach (var item in newValue)
            {
                item.PropertyChanged += OnItemPropertyChanged;
            }
        }

        ApplyFilter();
        OnPropertyChanged(nameof(TotalItems));
    }

    private void OnSelectedItemChanged(ItemViewModel? oldValue, ItemViewModel? newValue)
    {
        // Update IsSelected property on view models
        if (oldValue != null)
        {
            oldValue.IsSelected = false;
        }

        if (newValue != null)
        {
            newValue.IsSelected = true;
        }

        // Update selected items collection
        SelectedItems.Clear();
        if (newValue != null)
        {
            SelectedItems.Add(newValue);
        }
    }

    private void OnItemsCollectionChanged(object? sender, NotifyCollectionChangedEventArgs e)
    {
        // Handle items added
        if (e.NewItems != null)
        {
            foreach (ItemViewModel item in e.NewItems)
            {
                item.PropertyChanged += OnItemPropertyChanged;
            }
        }

        // Handle items removed
        if (e.OldItems != null)
        {
            foreach (ItemViewModel item in e.OldItems)
            {
                item.PropertyChanged -= OnItemPropertyChanged;
            }
        }

        ApplyFilter();
        OnPropertyChanged(nameof(TotalItems));
    }

    private void OnItemPropertyChanged(object? sender, PropertyChangedEventArgs e)
    {
        if (sender is ItemViewModel item && e.PropertyName == nameof(ItemViewModel.MatchesSearch))
        {
            ApplyFilter();
        }
    }

    private void OnListViewSelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        // Update SelectedItems collection
        SelectedItems.Clear();
        foreach (ItemViewModel item in ItemsListView.SelectedItems)
        {
            SelectedItems.Add(item);
        }

        // Raise custom selection changed event
        SelectionChanged?.Invoke(this, e);
    }

    private void OnListViewMouseDoubleClick(object sender, MouseButtonEventArgs e)
    {
        if (SelectedItem != null)
        {
            ItemDoubleClicked?.Invoke(this, SelectedItem);
        }
    }

    #endregion

    #region Command Implementations

    private void RefreshAllThumbnails()
    {
        if (ItemsSource == null) return;

        foreach (var item in ItemsSource)
        {
            if (item.RefreshThumbnailCommand.CanExecute(null))
            {
                item.RefreshThumbnailCommand.Execute(null);
            }
        }

        StatusMessage = "Refreshing all thumbnails...";
    }

    private bool CanRefreshAll()
    {
        return ItemsSource?.Count > 0;
    }

    private void DeleteItem(ItemViewModel? item)
    {
        if (item != null)
        {
            ItemDeleteRequested?.Invoke(this, item);
        }
    }

    private bool CanDeleteItem(ItemViewModel? item)
    {
        return item != null;
    }

    private void ShowProperties(ItemViewModel? item)
    {
        if (item != null)
        {
            ItemPropertiesRequested?.Invoke(this, item);
        }
    }

    private bool CanShowProperties(ItemViewModel? item)
    {
        return item != null;
    }

    #endregion

    #region Private Methods

    private void ApplyFilter()
    {
        FilteredItems.Clear();

        if (ItemsSource == null) return;

        foreach (var item in ItemsSource)
        {
            // Update search text on item
            item.UpdateSearch(SearchText);

            // Add to filtered collection if it matches
            if (item.MatchesSearch)
            {
                FilteredItems.Add(item);
            }
        }

        OnPropertyChanged(nameof(FilteredItems));
        UpdateStatusMessage();
    }

    private void UpdateStatusMessage()
    {
        if (string.IsNullOrWhiteSpace(SearchText))
        {
            StatusMessage = string.Empty;
        }
        else
        {
            StatusMessage = $"Filtered by: {SearchText}";
        }
    }

    private void OnPropertyChanged(string propertyName)
    {
        // Simple property changed notification for data binding
        var handler = GetValue(DataContextProperty);
        if (handler is INotifyPropertyChanged notifyPropertyChanged)
        {
            // This is a simplified approach - in a real implementation you'd want proper INPC
        }
    }

    #endregion

    #region Drag and Drop Event Handlers

    private void OnPreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
    {
        _dragStartPoint = e.GetPosition(null);
        _isDragging = false;
    }

    private void OnPreviewMouseMove(object sender, MouseEventArgs e)
    {
        if (e.LeftButton == MouseButtonState.Pressed && !_isDragging && _dragDropService != null)
        {
            var currentPosition = e.GetPosition(null);
            var diff = _dragStartPoint - currentPosition;

            if (Math.Abs(diff.X) > SystemParameters.MinimumHorizontalDragDistance ||
                Math.Abs(diff.Y) > SystemParameters.MinimumVerticalDragDistance)
            {
                _isDragging = true;
                StartDragOperation(e);
            }
        }
    }

    private void OnPreviewMouseLeftButtonUp(object sender, MouseButtonEventArgs e)
    {
        _isDragging = false;
    }

    private void StartDragOperation(MouseEventArgs e)
    {
        try
        {
            var selectedItems = SelectedItems.ToList();
            if (!selectedItems.Any())
            {
                return;
            }

            _logger?.LogDebug("Starting drag operation for {ItemCount} items", selectedItems.Count);

            var allowedEffects = DragDropEffects.Copy | DragDropEffects.Move;
            var result = _dragDropService!.StartDrag(ItemsListView, selectedItems, allowedEffects);

            _logger?.LogDebug("Drag operation completed with result: {Result}", result);
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Error during drag operation");
        }
    }

    private void OnDragEnter(object sender, DragEventArgs e)
    {
        HandleDragEvent(e);
        if (_dragDropService != null)
        {
            var isValid = _dragDropService.ValidateDrop(e, typeof(ItemViewModel));
            _dragDropService.ShowDropFeedback(ItemsListView, isValid);
        }
    }

    private void OnDragOver(object sender, DragEventArgs e)
    {
        HandleDragEvent(e);
    }

    private void OnDragLeave(object sender, DragEventArgs e)
    {
        _dragDropService?.HideDropFeedback(ItemsListView);
    }

    private void OnDrop(object sender, DragEventArgs e)
    {
        try
        {
            _dragDropService?.HideDropFeedback(ItemsListView);

            if (_dragDropService == null)
            {
                e.Effects = DragDropEffects.None;
                return;
            }

            // Handle item drops (for reordering)
            var droppedItems = _dragDropService.ExtractItems(e.Data);
            if (droppedItems != null)
            {
                HandleItemDrop(droppedItems, e);
                return;
            }

            // Handle file drops
            var droppedFiles = _dragDropService.ExtractFilePaths(e.Data);
            if (droppedFiles != null)
            {
                HandleFileDrop(droppedFiles, e);
                return;
            }

            e.Effects = DragDropEffects.None;
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Error handling drop operation");
            e.Effects = DragDropEffects.None;
        }
    }

    private void HandleDragEvent(DragEventArgs e)
    {
        if (_dragDropService?.ValidateDrop(e, typeof(ItemViewModel)) == true)
        {
            e.Effects = DragDropEffects.Copy;
        }
        else
        {
            e.Effects = DragDropEffects.None;
        }
        e.Handled = true;
    }

    private void HandleItemDrop(IEnumerable<ItemViewModel> items, DragEventArgs e)
    {
        var itemList = items.ToList();
        _logger?.LogDebug("Handling item drop for {ItemCount} items", itemList.Count);

        // Find drop target position
        var dropPosition = e.GetPosition(ItemsListView);
        var dropTarget = GetDropTarget(dropPosition);
        var targetIndex = GetDropTargetIndex(dropPosition);
        
        // Raise event for parent to handle reordering
        ItemsReordered?.Invoke(this, new ItemReorderEventArgs(itemList, dropTarget, targetIndex));
        
        e.Effects = DragDropEffects.Move;
    }

    private void HandleFileDrop(IEnumerable<string> filePaths, DragEventArgs e)
    {
        var fileList = filePaths.ToList();
        _logger?.LogDebug("Handling file drop for {FileCount} files", fileList.Count);

        // Raise event for parent to handle file loading
        var dropPosition = e.GetPosition(ItemsListView);
        FilesDropped?.Invoke(this, new FileDragDropEventArgs(fileList, e.Effects, dropPosition));
        
        e.Effects = DragDropEffects.Copy;
    }

    private ItemViewModel? GetDropTarget(Point dropPosition)
    {
        var hitTestResult = VisualTreeHelper.HitTest(ItemsListView, dropPosition);
        if (hitTestResult?.VisualHit != null)
        {
            var listViewItem = FindParent<ListViewItem>(hitTestResult.VisualHit);
            return listViewItem?.DataContext as ItemViewModel;
        }
        return null;
    }

    private int GetDropTargetIndex(Point dropPosition)
    {
        var hitTestResult = VisualTreeHelper.HitTest(ItemsListView, dropPosition);
        if (hitTestResult?.VisualHit != null)
        {
            var listViewItem = FindParent<ListViewItem>(hitTestResult.VisualHit);
            if (listViewItem?.DataContext is ItemViewModel targetItem)
            {
                return FilteredItems.IndexOf(targetItem);
            }
        }
        return -1;
    }

    private static T? FindParent<T>(DependencyObject child) where T : DependencyObject
    {
        var parent = VisualTreeHelper.GetParent(child);
        while (parent != null && parent is not T)
        {
            parent = VisualTreeHelper.GetParent(parent);
        }
        return parent as T;
    }

    #endregion

    #region Events

    /// <summary>
    /// Event raised when items are reordered via drag and drop
    /// </summary>
    public event EventHandler<ItemReorderEventArgs>? ItemsReordered;

    /// <summary>
    /// Event raised when files are dropped on the control
    /// </summary>
    public event EventHandler<FileDragDropEventArgs>? FilesDropped;

    #endregion

    #region Public Methods

    /// <summary>
    /// Scrolls to the specified item
    /// </summary>
    /// <param name="item">Item to scroll to</param>
    public void ScrollToItem(ItemViewModel item)
    {
        if (FilteredItems.Contains(item))
        {
            ItemsListView.ScrollIntoView(item);
        }
    }

    /// <summary>
    /// Selects all items
    /// </summary>
    public void SelectAll()
    {
        if (SelectionMode == SelectionMode.Multiple)
        {
            ItemsListView.SelectAll();
        }
    }

    /// <summary>
    /// Clears the selection
    /// </summary>
    public void ClearSelection()
    {
        ItemsListView.SelectedItems.Clear();
    }

    /// <summary>
    /// Refreshes the view
    /// </summary>
    public void RefreshView()
    {
        ApplyFilter();
    }

    /// <summary>
    /// Sets the loading state
    /// </summary>
    /// <param name="isLoading">Loading state</param>
    /// <param name="message">Loading message</param>
    public void SetLoadingState(bool isLoading, string message = "Loading items...")
    {
        IsLoading = isLoading;
        LoadingMessage = message;
    }

    #endregion
}

/// <summary>
/// Event arguments for item reorder operations
/// </summary>
public class ItemReorderEventArgs : EventArgs
{
    public IEnumerable<ItemViewModel> Items { get; }
    public ItemViewModel? DropTarget { get; }
    public int TargetIndex { get; }

    public ItemReorderEventArgs(IEnumerable<ItemViewModel> items, ItemViewModel? dropTarget, int targetIndex = -1)
    {
        Items = items ?? throw new ArgumentNullException(nameof(items));
        DropTarget = dropTarget;
        TargetIndex = targetIndex;
    }
}

/// <summary>
/// Event arguments for file drag and drop operations
/// </summary>
public class FileDragDropEventArgs : EventArgs
{
    public IEnumerable<string> FilePaths { get; }
    public DragDropEffects Effects { get; }
    public Point DropPosition { get; }

    public FileDragDropEventArgs(IEnumerable<string> filePaths, DragDropEffects effects, Point dropPosition = default)
    {
        FilePaths = filePaths ?? throw new ArgumentNullException(nameof(filePaths));
        Effects = effects;
        DropPosition = dropPosition;
    }
}

/// <summary>
/// Simple relay command implementation
/// </summary>
public class RelayCommand : ICommand
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

/// <summary>
/// Generic relay command implementation
/// </summary>
public class RelayCommand<T> : ICommand
{
    private readonly Action<T?> _execute;
    private readonly Func<T?, bool>? _canExecute;

    public RelayCommand(Action<T?> execute, Func<T?, bool>? canExecute = null)
    {
        _execute = execute ?? throw new ArgumentNullException(nameof(execute));
        _canExecute = canExecute;
    }

    public event EventHandler? CanExecuteChanged
    {
        add => CommandManager.RequerySuggested += value;
        remove => CommandManager.RequerySuggested -= value;
    }

    public bool CanExecute(object? parameter) => _canExecute?.Invoke((T?)parameter) ?? true;

    public void Execute(object? parameter) => _execute((T?)parameter);
}

