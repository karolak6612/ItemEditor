using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.Windows.Threading;
using ItemEditor.ViewModels;
using ItemEditor.Services;
using Microsoft.Extensions.Logging;

namespace ItemEditor.Controls;

/// <summary>
/// Item preview control for displaying item sprites and animations
/// </summary>
public partial class ItemPreview : UserControl, INotifyPropertyChanged
{
    #region Fields

    private readonly DispatcherTimer _animationTimer;
    private readonly IDragDropService? _dragDropService;
    private readonly ILogger<ItemPreview>? _logger;
    private bool _isDragging;
    private bool _isDragDropOperation;
    private Point _lastMousePosition;
    private Point _dragStartPoint;
    private readonly List<BitmapSource> _spriteFrames = new();

    #endregion

    #region Dependency Properties

    /// <summary>
    /// Selected item dependency property
    /// </summary>
    public static readonly DependencyProperty SelectedItemProperty =
        DependencyProperty.Register(
            nameof(SelectedItem),
            typeof(ItemViewModel),
            typeof(ItemPreview),
            new FrameworkPropertyMetadata(null, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, OnSelectedItemChanged));

    /// <summary>
    /// Comparison item dependency property
    /// </summary>
    public static readonly DependencyProperty ComparisonItemProperty =
        DependencyProperty.Register(
            nameof(ComparisonItem),
            typeof(ItemViewModel),
            typeof(ItemPreview),
            new PropertyMetadata(null, OnComparisonItemChanged));

    /// <summary>
    /// Show grid dependency property
    /// </summary>
    public static readonly DependencyProperty ShowGridProperty =
        DependencyProperty.Register(
            nameof(ShowGrid),
            typeof(bool),
            typeof(ItemPreview),
            new PropertyMetadata(false, OnShowGridChanged));

    /// <summary>
    /// Show checkerboard dependency property
    /// </summary>
    public static readonly DependencyProperty ShowCheckerboardProperty =
        DependencyProperty.Register(
            nameof(ShowCheckerboard),
            typeof(bool),
            typeof(ItemPreview),
            new PropertyMetadata(true));

    #endregion

    #region Properties

    /// <summary>
    /// Gets or sets the selected item
    /// </summary>
    public ItemViewModel? SelectedItem
    {
        get => (ItemViewModel?)GetValue(SelectedItemProperty);
        set => SetValue(SelectedItemProperty, value);
    }

    /// <summary>
    /// Gets or sets the comparison item
    /// </summary>
    public ItemViewModel? ComparisonItem
    {
        get => (ItemViewModel?)GetValue(ComparisonItemProperty);
        set => SetValue(ComparisonItemProperty, value);
    }

    /// <summary>
    /// Gets or sets whether to show the grid
    /// </summary>
    public bool ShowGrid
    {
        get => (bool)GetValue(ShowGridProperty);
        set => SetValue(ShowGridProperty, value);
    }

    /// <summary>
    /// Gets or sets whether to show the checkerboard background
    /// </summary>
    public bool ShowCheckerboard
    {
        get => (bool)GetValue(ShowCheckerboardProperty);
        set => SetValue(ShowCheckerboardProperty, value);
    }

    private double _zoomLevel = 1.0;
    /// <summary>
    /// Gets or sets the zoom level
    /// </summary>
    public double ZoomLevel
    {
        get => _zoomLevel;
        set
        {
            if (SetProperty(ref _zoomLevel, value))
            {
                UpdateZoom();
            }
        }
    }

    private bool _isComparisonMode;
    /// <summary>
    /// Gets or sets whether comparison mode is enabled
    /// </summary>
    public bool IsComparisonMode
    {
        get => _isComparisonMode;
        set
        {
            if (SetProperty(ref _isComparisonMode, value))
            {
                UpdateComparisonMode();
            }
        }
    }

    private bool _isLoading;
    /// <summary>
    /// Gets or sets whether the control is loading
    /// </summary>
    public bool IsLoading
    {
        get => _isLoading;
        set => SetProperty(ref _isLoading, value);
    }

    private BitmapSource? _currentSprite;
    /// <summary>
    /// Gets or sets the current sprite
    /// </summary>
    public BitmapSource? CurrentSprite
    {
        get => _currentSprite;
        set => SetProperty(ref _currentSprite, value);
    }

    private BitmapSource? _comparisonSprite;
    /// <summary>
    /// Gets or sets the comparison sprite
    /// </summary>
    public BitmapSource? ComparisonSprite
    {
        get => _comparisonSprite;
        set => SetProperty(ref _comparisonSprite, value);
    }

    private int _currentFrameIndex;
    /// <summary>
    /// Gets or sets the current frame index
    /// </summary>
    public int CurrentFrameIndex
    {
        get => _currentFrameIndex;
        set
        {
            if (SetProperty(ref _currentFrameIndex, value))
            {
                UpdateCurrentFrame();
            }
        }
    }

    private bool _isPlaying;
    /// <summary>
    /// Gets or sets whether animation is playing
    /// </summary>
    public bool IsPlaying
    {
        get => _isPlaying;
        set
        {
            if (SetProperty(ref _isPlaying, value))
            {
                UpdatePlaybackState();
            }
        }
    }

    private bool _isLooping = true;
    /// <summary>
    /// Gets or sets whether animation is looping
    /// </summary>
    public bool IsLooping
    {
        get => _isLooping;
        set => SetProperty(ref _isLooping, value);
    }

    private double _animationSpeed = 1.0;
    /// <summary>
    /// Gets or sets the animation speed multiplier
    /// </summary>
    public double AnimationSpeed
    {
        get => _animationSpeed;
        set
        {
            if (SetProperty(ref _animationSpeed, value))
            {
                UpdateAnimationSpeed();
            }
        }
    }

    private string _mousePosition = string.Empty;
    /// <summary>
    /// Gets or sets the mouse position text
    /// </summary>
    public string MousePosition
    {
        get => _mousePosition;
        set => SetProperty(ref _mousePosition, value);
    }

    /// <summary>
    /// Gets whether there are multiple frames
    /// </summary>
    public bool HasMultipleFrames => _spriteFrames.Count > 1;

    /// <summary>
    /// Gets the maximum frame index
    /// </summary>
    public int MaxFrameIndex => Math.Max(0, _spriteFrames.Count - 1);

    /// <summary>
    /// Gets the total number of frames
    /// </summary>
    public int TotalFrames => _spriteFrames.Count;

    /// <summary>
    /// Gets the sprite width
    /// </summary>
    public int SpriteWidth => CurrentSprite?.PixelWidth ?? 0;

    /// <summary>
    /// Gets the sprite height
    /// </summary>
    public int SpriteHeight => CurrentSprite?.PixelHeight ?? 0;

    #endregion

    #region Commands

    /// <summary>
    /// Command to go to first frame
    /// </summary>
    public ICommand FirstFrameCommand { get; }

    /// <summary>
    /// Command to go to previous frame
    /// </summary>
    public ICommand PreviousFrameCommand { get; }

    /// <summary>
    /// Command to play/pause animation
    /// </summary>
    public ICommand PlayPauseCommand { get; }

    /// <summary>
    /// Command to go to next frame
    /// </summary>
    public ICommand NextFrameCommand { get; }

    /// <summary>
    /// Command to go to last frame
    /// </summary>
    public ICommand LastFrameCommand { get; }

    #endregion

    #region Events

    /// <summary>
    /// Property changed event
    /// </summary>
    public event PropertyChangedEventHandler? PropertyChanged;

    /// <summary>
    /// Event raised when a sprite is clicked
    /// </summary>
    public event EventHandler<Point>? SpriteClicked;

    /// <summary>
    /// Event raised when a sprite is dropped on the preview
    /// </summary>
    public event EventHandler<SpriteDropEventArgs>? SpriteDropped;

    #endregion

    #region Constructor

    /// <summary>
    /// Initializes a new instance of the ItemPreview class
    /// </summary>
    public ItemPreview()
    {
        InitializeComponent();

        // Initialize animation timer
        _animationTimer = new DispatcherTimer
        {
            Interval = TimeSpan.FromMilliseconds(200) // Default 5 FPS
        };
        _animationTimer.Tick += OnAnimationTick;

        // Initialize commands
        FirstFrameCommand = new RelayCommand(GoToFirstFrame, CanNavigateFrames);
        PreviousFrameCommand = new RelayCommand(GoToPreviousFrame, CanNavigateFrames);
        PlayPauseCommand = new RelayCommand(TogglePlayback, CanNavigateFrames);
        NextFrameCommand = new RelayCommand(GoToNextFrame, CanNavigateFrames);
        LastFrameCommand = new RelayCommand(GoToLastFrame, CanNavigateFrames);

        // Set data context
        DataContext = this;

        // Initialize grid
        UpdateGrid();
    }

    /// <summary>
    /// Initializes a new instance of the ItemPreview class with dependencies
    /// </summary>
    public ItemPreview(IDragDropService dragDropService, ILogger<ItemPreview> logger) : this()
    {
        _dragDropService = dragDropService;
        _logger = logger;
    }

    #endregion

    #region Event Handlers

    private static void OnSelectedItemChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        if (d is ItemPreview preview)
        {
            preview.OnSelectedItemChanged(e.NewValue as ItemViewModel);
        }
    }

    private static void OnComparisonItemChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        if (d is ItemPreview preview)
        {
            preview.OnComparisonItemChanged(e.NewValue as ItemViewModel);
        }
    }

    private static void OnShowGridChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        if (d is ItemPreview preview)
        {
            preview.UpdateGrid();
        }
    }

    private async void OnSelectedItemChanged(ItemViewModel? item)
    {
        IsPlaying = false;
        _spriteFrames.Clear();
        CurrentFrameIndex = 0;

        if (item?.Model.SpriteIds.Count > 0)
        {
            await LoadSpritesAsync(item);
        }
        else
        {
            CurrentSprite = null;
        }

        OnPropertyChanged(nameof(HasMultipleFrames));
        OnPropertyChanged(nameof(MaxFrameIndex));
        OnPropertyChanged(nameof(TotalFrames));
        OnPropertyChanged(nameof(SpriteWidth));
        OnPropertyChanged(nameof(SpriteHeight));
        
        CommandManager.InvalidateRequerySuggested();
    }

    private async void OnComparisonItemChanged(ItemViewModel? item)
    {
        if (item?.Model.SpriteIds.Count > 0)
        {
            // Load comparison sprite (simplified - just first sprite)
            // In a real implementation, this would load from the sprite service
            ComparisonSprite = item.Thumbnail;
        }
        else
        {
            ComparisonSprite = null;
        }
    }

    private void OnAnimationTick(object? sender, EventArgs e)
    {
        if (!HasMultipleFrames) return;

        if (CurrentFrameIndex < MaxFrameIndex)
        {
            CurrentFrameIndex++;
        }
        else if (IsLooping)
        {
            CurrentFrameIndex = 0;
        }
        else
        {
            IsPlaying = false;
        }
    }

    private void OnCanvasMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
    {
        if (sender is Canvas canvas)
        {
            _isDragging = true;
            _isDragDropOperation = false;
            _lastMousePosition = e.GetPosition(canvas);
            _dragStartPoint = _lastMousePosition;
            canvas.CaptureMouse();
            
            // Raise sprite clicked event
            SpriteClicked?.Invoke(this, _lastMousePosition);
        }
    }

    private void OnCanvasMouseMove(object sender, MouseEventArgs e)
    {
        if (sender is Canvas canvas)
        {
            var position = e.GetPosition(canvas);
            
            // Update mouse position display
            MousePosition = $"X: {(int)position.X}, Y: {(int)position.Y}";
            
            // Handle dragging
            if (_isDragging && e.LeftButton == MouseButtonState.Pressed)
            {
                // Check if we should start a drag-drop operation
                if (!_isDragDropOperation && _dragDropService != null && CurrentSprite != null)
                {
                    var diff = position - _dragStartPoint;
                    if (Math.Abs(diff.X) > SystemParameters.MinimumHorizontalDragDistance ||
                        Math.Abs(diff.Y) > SystemParameters.MinimumVerticalDragDistance)
                    {
                        _isDragDropOperation = true;
                        StartSpriteDragOperation();
                        return;
                    }
                }

                if (!_isDragDropOperation)
                {
                    var delta = position - _lastMousePosition;
                    
                    // Pan the view
                    var scrollViewer = PreviewScrollViewer;
                    scrollViewer.ScrollToHorizontalOffset(scrollViewer.HorizontalOffset - delta.X);
                    scrollViewer.ScrollToVerticalOffset(scrollViewer.VerticalOffset - delta.Y);
                }
                
                _lastMousePosition = position;
            }
        }
    }

    private void OnCanvasMouseLeftButtonUp(object sender, MouseButtonEventArgs e)
    {
        if (sender is Canvas canvas)
        {
            _isDragging = false;
            _isDragDropOperation = false;
            canvas.ReleaseMouseCapture();
        }
    }

    private void OnCanvasMouseWheel(object sender, MouseWheelEventArgs e)
    {
        // Zoom with mouse wheel
        var zoomFactor = e.Delta > 0 ? 1.2 : 1.0 / 1.2;
        var newZoom = ZoomLevel * zoomFactor;
        
        // Clamp zoom level
        ZoomLevel = Math.Max(0.1, Math.Min(10.0, newZoom));
        
        e.Handled = true;
    }

    private void OnSpriteDragEnter(object sender, DragEventArgs e)
    {
        try
        {
            if (_dragDropService != null)
            {
                var isValid = _dragDropService.ValidateDrop(e, typeof(ItemViewModel)) ||
                             _dragDropService.ExtractSpriteId(e.Data) != null;
                
                if (isValid)
                {
                    e.Effects = DragDropEffects.Copy;
                    _dragDropService.ShowDropFeedback(SpriteImage, true);
                }
                else
                {
                    e.Effects = DragDropEffects.None;
                    _dragDropService.ShowDropFeedback(SpriteImage, false);
                }
            }
            else
            {
                e.Effects = DragDropEffects.None;
            }
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Error handling sprite drag enter");
            e.Effects = DragDropEffects.None;
        }
        
        e.Handled = true;
    }

    private void OnSpriteDragOver(object sender, DragEventArgs e)
    {
        try
        {
            if (_dragDropService != null)
            {
                var isValid = _dragDropService.ValidateDrop(e, typeof(ItemViewModel)) ||
                             _dragDropService.ExtractSpriteId(e.Data) != null;
                
                e.Effects = isValid ? DragDropEffects.Copy : DragDropEffects.None;
            }
            else
            {
                e.Effects = DragDropEffects.None;
            }
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Error handling sprite drag over");
            e.Effects = DragDropEffects.None;
        }
        
        e.Handled = true;
    }

    private void OnSpriteDragLeave(object sender, DragEventArgs e)
    {
        try
        {
            _dragDropService?.HideDropFeedback(SpriteImage);
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Error handling sprite drag leave");
        }
    }

    private void OnSpriteDrop(object sender, DragEventArgs e)
    {
        try
        {
            _dragDropService?.HideDropFeedback(SpriteImage);

            if (_dragDropService == null)
            {
                e.Effects = DragDropEffects.None;
                return;
            }

            var dropPosition = e.GetPosition(SpriteImage);
            
            // Handle sprite ID drops
            var spriteId = _dragDropService.ExtractSpriteId(e.Data);
            if (spriteId.HasValue)
            {
                var spritePreview = _dragDropService.ExtractSpritePreview(e.Data);
                var eventArgs = new SpriteDropEventArgs(spriteId.Value, spritePreview, dropPosition);
                SpriteDropped?.Invoke(this, eventArgs);
                
                e.Effects = DragDropEffects.Copy;
                _logger?.LogDebug("Sprite {SpriteId} dropped at position {Position}", spriteId.Value, dropPosition);
                return;
            }

            // Handle item drops (extract sprite from item)
            var droppedItems = _dragDropService.ExtractItems(e.Data);
            if (droppedItems != null)
            {
                var firstItem = droppedItems.FirstOrDefault();
                if (firstItem?.Model.SpriteIds.Count > 0)
                {
                    var firstSpriteId = firstItem.Model.SpriteIds[0];
                    var eventArgs = new SpriteDropEventArgs(firstSpriteId, firstItem.Thumbnail, dropPosition);
                    SpriteDropped?.Invoke(this, eventArgs);
                    
                    e.Effects = DragDropEffects.Copy;
                    _logger?.LogDebug("Item sprite {SpriteId} dropped at position {Position}", firstSpriteId, dropPosition);
                    return;
                }
            }

            e.Effects = DragDropEffects.None;
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Error handling sprite drop");
            e.Effects = DragDropEffects.None;
        }
    }

    private void StartSpriteDragOperation()
    {
        try
        {
            if (_dragDropService == null || SelectedItem?.Model.SpriteIds.Count == 0)
                return;

            var spriteId = SelectedItem.Model.SpriteIds[CurrentFrameIndex];
            var spritePreview = CurrentSprite;
            
            _logger?.LogDebug("Starting sprite drag operation for sprite ID: {SpriteId}", spriteId);

            var allowedEffects = DragDropEffects.Copy;
            var result = _dragDropService.StartSpriteDrag(PreviewCanvas, spriteId, spritePreview, allowedEffects);

            _logger?.LogDebug("Sprite drag operation completed with result: {Result}", result);
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Error during sprite drag operation");
        }
        finally
        {
            _isDragDropOperation = false;
            _isDragging = false;
            PreviewCanvas.ReleaseMouseCapture();
        }
    }

    #endregion

    #region Command Implementations

    private void GoToFirstFrame()
    {
        CurrentFrameIndex = 0;
    }

    private void GoToPreviousFrame()
    {
        if (CurrentFrameIndex > 0)
        {
            CurrentFrameIndex--;
        }
        else if (IsLooping)
        {
            CurrentFrameIndex = MaxFrameIndex;
        }
    }

    private void TogglePlayback()
    {
        IsPlaying = !IsPlaying;
    }

    private void GoToNextFrame()
    {
        if (CurrentFrameIndex < MaxFrameIndex)
        {
            CurrentFrameIndex++;
        }
        else if (IsLooping)
        {
            CurrentFrameIndex = 0;
        }
    }

    private void GoToLastFrame()
    {
        CurrentFrameIndex = MaxFrameIndex;
    }

    private bool CanNavigateFrames()
    {
        return HasMultipleFrames;
    }

    #endregion

    #region Private Methods

    private async Task LoadSpritesAsync(ItemViewModel item)
    {
        try
        {
            IsLoading = true;
            _spriteFrames.Clear();

            // In a real implementation, this would load sprites from the sprite service
            // For now, we'll use the thumbnail as a placeholder
            if (item.Thumbnail != null)
            {
                _spriteFrames.Add(item.Thumbnail);
            }

            // Load additional frames if there are multiple sprite IDs
            foreach (var spriteId in item.Model.SpriteIds.Skip(1))
            {
                // This would load each sprite frame
                // For now, just duplicate the thumbnail
                if (item.Thumbnail != null)
                {
                    _spriteFrames.Add(item.Thumbnail);
                }
            }

            UpdateCurrentFrame();
        }
        catch (Exception ex)
        {
            System.Diagnostics.Debug.WriteLine($"Failed to load sprites: {ex.Message}");
        }
        finally
        {
            IsLoading = false;
        }
    }

    private void UpdateCurrentFrame()
    {
        if (_spriteFrames.Count > 0 && CurrentFrameIndex >= 0 && CurrentFrameIndex < _spriteFrames.Count)
        {
            CurrentSprite = _spriteFrames[CurrentFrameIndex];
        }
        else
        {
            CurrentSprite = null;
        }

        OnPropertyChanged(nameof(SpriteWidth));
        OnPropertyChanged(nameof(SpriteHeight));
    }

    private void UpdatePlaybackState()
    {
        if (IsPlaying && HasMultipleFrames)
        {
            _animationTimer.Start();
        }
        else
        {
            _animationTimer.Stop();
        }
    }

    private void UpdateAnimationSpeed()
    {
        // Update timer interval based on speed
        var baseInterval = 200; // 5 FPS base
        var newInterval = baseInterval / AnimationSpeed;
        _animationTimer.Interval = TimeSpan.FromMilliseconds(Math.Max(50, newInterval)); // Min 20 FPS
    }

    private void UpdateZoom()
    {
        SpriteScale.ScaleX = ZoomLevel;
        SpriteScale.ScaleY = ZoomLevel;
        UpdateGrid();
    }

    private void UpdateComparisonMode()
    {
        if (IsComparisonMode)
        {
            ComparisonColumn.Width = new GridLength(4);
            ComparisonPreviewColumn.Width = new GridLength(1, GridUnitType.Star);
        }
        else
        {
            ComparisonColumn.Width = new GridLength(0);
            ComparisonPreviewColumn.Width = new GridLength(0);
        }
    }

    private void UpdateGrid()
    {
        GridOverlay.Children.Clear();

        if (!ShowGrid || CurrentSprite == null) return;

        var gridSize = 32 * ZoomLevel; // 32x32 pixel grid
        var canvasWidth = CurrentSprite.PixelWidth * ZoomLevel;
        var canvasHeight = CurrentSprite.PixelHeight * ZoomLevel;

        // Draw vertical lines
        for (double x = 0; x <= canvasWidth; x += gridSize)
        {
            var line = new Line
            {
                X1 = x,
                Y1 = 0,
                X2 = x,
                Y2 = canvasHeight,
                Stroke = Brushes.Gray,
                StrokeThickness = 0.5,
                Opacity = 0.5
            };
            GridOverlay.Children.Add(line);
        }

        // Draw horizontal lines
        for (double y = 0; y <= canvasHeight; y += gridSize)
        {
            var line = new Line
            {
                X1 = 0,
                Y1 = y,
                X2 = canvasWidth,
                Y2 = y,
                Stroke = Brushes.Gray,
                StrokeThickness = 0.5,
                Opacity = 0.5
            };
            GridOverlay.Children.Add(line);
        }
    }

    private bool SetProperty<T>(ref T field, T value, [CallerMemberName] string? propertyName = null)
    {
        if (EqualityComparer<T>.Default.Equals(field, value))
            return false;

        field = value;
        OnPropertyChanged(propertyName);
        return true;
    }

    private void OnPropertyChanged([CallerMemberName] string? propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }

    #endregion

    #region Public Methods

    /// <summary>
    /// Fits the sprite to the view
    /// </summary>
    public void FitToView()
    {
        if (CurrentSprite == null) return;

        var viewWidth = PreviewScrollViewer.ViewportWidth;
        var viewHeight = PreviewScrollViewer.ViewportHeight;
        var spriteWidth = CurrentSprite.PixelWidth;
        var spriteHeight = CurrentSprite.PixelHeight;

        if (viewWidth > 0 && viewHeight > 0 && spriteWidth > 0 && spriteHeight > 0)
        {
            var scaleX = viewWidth / spriteWidth;
            var scaleY = viewHeight / spriteHeight;
            ZoomLevel = Math.Min(scaleX, scaleY) * 0.9; // 90% to leave some margin
        }
    }

    /// <summary>
    /// Resets the zoom to 100%
    /// </summary>
    public void ResetZoom()
    {
        ZoomLevel = 1.0;
    }

    /// <summary>
    /// Centers the view on the sprite
    /// </summary>
    public void CenterView()
    {
        if (CurrentSprite == null) return;

        var scrollViewer = PreviewScrollViewer;
        var centerX = (CurrentSprite.PixelWidth * ZoomLevel - scrollViewer.ViewportWidth) / 2;
        var centerY = (CurrentSprite.PixelHeight * ZoomLevel - scrollViewer.ViewportHeight) / 2;

        scrollViewer.ScrollToHorizontalOffset(Math.Max(0, centerX));
        scrollViewer.ScrollToVerticalOffset(Math.Max(0, centerY));
    }

    /// <summary>
    /// Exports the current frame as an image
    /// </summary>
    /// <returns>The current frame as a bitmap source</returns>
    public BitmapSource? ExportCurrentFrame()
    {
        return CurrentSprite;
    }

    #endregion
}

/// <summary>
/// Event arguments for sprite drop operations
/// </summary>
public class SpriteDropEventArgs : EventArgs
{
    public int SpriteId { get; }
    public BitmapSource? SpritePreview { get; }
    public Point DropPosition { get; }

    public SpriteDropEventArgs(int spriteId, BitmapSource? spritePreview, Point dropPosition)
    {
        SpriteId = spriteId;
        SpritePreview = spritePreview;
        DropPosition = dropPosition;
    }
}