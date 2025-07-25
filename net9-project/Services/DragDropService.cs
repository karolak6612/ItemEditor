using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Animation;
using Microsoft.Extensions.Logging;
using ItemEditor.ViewModels;
using ItemEditor.Models;
using Wpf.Ui.Controls;

namespace ItemEditor.Services;

/// <summary>
/// Service for handling drag and drop operations with visual feedback
/// </summary>
public class DragDropService : IDragDropService
{
    private readonly ILogger<DragDropService> _logger;
    private const string ItemDataFormat = "ItemEditor.ItemViewModel";
    private const string ItemsDataFormat = "ItemEditor.ItemViewModels";
    private const string SpriteDataFormat = "ItemEditor.SpriteId";
    private const string SpritePreviewDataFormat = "ItemEditor.SpritePreview";
    
    public DragDropService(ILogger<DragDropService> logger)
    {
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
    }
    
    /// <inheritdoc />
    public DragDropEffects StartDrag(DependencyObject source, ItemViewModel item, DragDropEffects allowedEffects)
    {
        return StartDrag(source, new[] { item }, allowedEffects);
    }
    
    /// <inheritdoc />
    public DragDropEffects StartDrag(DependencyObject source, IEnumerable<ItemViewModel> items, DragDropEffects allowedEffects)
    {
        try
        {
            var itemList = items.ToList();
            if (!itemList.Any())
            {
                _logger.LogWarning("Attempted to start drag with no items");
                return DragDropEffects.None;
            }
            
            _logger.LogDebug("Starting drag operation for {ItemCount} items", itemList.Count);
            
            var dataObject = new DataObject();
            
            // Add items in custom format
            if (itemList.Count == 1)
            {
                dataObject.SetData(ItemDataFormat, itemList.First());
            }
            dataObject.SetData(ItemsDataFormat, itemList);
            
            // Add text representation
            var textData = string.Join(Environment.NewLine, itemList.Select(i => $"{i.DisplayText} (ID: {i.Id})"));
            dataObject.SetText(textData);
            
            // Create drag preview
            var dragPreview = CreateDragPreview(itemList);
            
            // Start drag operation
            var result = DragDrop.DoDragDrop(source, dataObject, allowedEffects);
            
            _logger.LogDebug("Drag operation completed with result: {Result}", result);
            return result;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error starting drag operation");
            return DragDropEffects.None;
        }
    }
    
    /// <summary>
    /// Starts a drag operation for a sprite
    /// </summary>
    /// <param name="source">Source element</param>
    /// <param name="spriteId">Sprite ID to drag</param>
    /// <param name="spritePreview">Sprite preview image</param>
    /// <param name="allowedEffects">Allowed drag effects</param>
    /// <returns>Result of the drag operation</returns>
    public DragDropEffects StartSpriteDrag(DependencyObject source, int spriteId, BitmapSource? spritePreview, DragDropEffects allowedEffects)
    {
        try
        {
            _logger.LogDebug("Starting sprite drag operation for sprite ID: {SpriteId}", spriteId);
            
            var dataObject = new DataObject();
            
            // Add sprite data
            dataObject.SetData(SpriteDataFormat, spriteId);
            if (spritePreview != null)
            {
                dataObject.SetData(SpritePreviewDataFormat, spritePreview);
            }
            
            // Add text representation
            dataObject.SetText($"Sprite ID: {spriteId}");
            
            // Create drag preview for sprite
            var dragPreview = CreateSpriteDragPreview(spriteId, spritePreview);
            
            // Start drag operation
            var result = DragDrop.DoDragDrop(source, dataObject, allowedEffects);
            
            _logger.LogDebug("Sprite drag operation completed with result: {Result}", result);
            return result;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error starting sprite drag operation");
            return DragDropEffects.None;
        }
    }

    /// <inheritdoc />
    public DragDropEffects StartFileDrag(DependencyObject source, IEnumerable<string> filePaths, DragDropEffects allowedEffects)
    {
        try
        {
            var fileList = filePaths.ToList();
            if (!fileList.Any())
            {
                _logger.LogWarning("Attempted to start file drag with no files");
                return DragDropEffects.None;
            }
            
            _logger.LogDebug("Starting file drag operation for {FileCount} files", fileList.Count);
            
            var dataObject = new DataObject();
            dataObject.SetData(DataFormats.FileDrop, fileList.ToArray());
            
            // Add text representation
            var textData = string.Join(Environment.NewLine, fileList);
            dataObject.SetText(textData);
            
            var result = DragDrop.DoDragDrop(source, dataObject, allowedEffects);
            
            _logger.LogDebug("File drag operation completed with result: {Result}", result);
            return result;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error starting file drag operation");
            return DragDropEffects.None;
        }
    }
    
    /// <inheritdoc />
    public bool ValidateDrop(DragEventArgs dragEventArgs, Type targetType)
    {
        try
        {
            var dataObject = dragEventArgs.Data;
            
            // Check for file drops
            if (dataObject.GetDataPresent(DataFormats.FileDrop))
            {
                var files = (string[])dataObject.GetData(DataFormats.FileDrop);
                return files.Any(f => IsValidFileExtension(Path.GetExtension(f)));
            }
            
            // Check for item drops
            if (dataObject.GetDataPresent(ItemDataFormat) || dataObject.GetDataPresent(ItemsDataFormat))
            {
                return targetType == typeof(ItemViewModel) || targetType == typeof(IEnumerable<ItemViewModel>);
            }
            
            // Check for sprite drops
            if (dataObject.GetDataPresent(SpriteDataFormat))
            {
                return targetType == typeof(ItemViewModel); // Sprites can be dropped on items
            }
            
            return false;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error validating drop operation");
            return false;
        }
    }
    
    /// <inheritdoc />
    public IEnumerable<ItemViewModel>? ExtractItems(IDataObject dataObject)
    {
        try
        {
            // Try multiple items first
            if (dataObject.GetDataPresent(ItemsDataFormat))
            {
                return (IEnumerable<ItemViewModel>)dataObject.GetData(ItemsDataFormat);
            }
            
            // Try single item
            if (dataObject.GetDataPresent(ItemDataFormat))
            {
                var item = (ItemViewModel)dataObject.GetData(ItemDataFormat);
                return new[] { item };
            }
            
            return null;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error extracting items from drag data");
            return null;
        }
    }
    
    /// <summary>
    /// Extracts sprite ID from drag data
    /// </summary>
    /// <param name="dataObject">Data object</param>
    /// <returns>Sprite ID or null</returns>
    public int? ExtractSpriteId(IDataObject dataObject)
    {
        try
        {
            if (dataObject.GetDataPresent(SpriteDataFormat))
            {
                return (int)dataObject.GetData(SpriteDataFormat);
            }
            
            return null;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error extracting sprite ID from drag data");
            return null;
        }
    }
    
    /// <summary>
    /// Extracts sprite preview from drag data
    /// </summary>
    /// <param name="dataObject">Data object</param>
    /// <returns>Sprite preview or null</returns>
    public BitmapSource? ExtractSpritePreview(IDataObject dataObject)
    {
        try
        {
            if (dataObject.GetDataPresent(SpritePreviewDataFormat))
            {
                return (BitmapSource)dataObject.GetData(SpritePreviewDataFormat);
            }
            
            return null;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error extracting sprite preview from drag data");
            return null;
        }
    }

    /// <inheritdoc />
    public IEnumerable<string>? ExtractFilePaths(IDataObject dataObject)
    {
        try
        {
            if (dataObject.GetDataPresent(DataFormats.FileDrop))
            {
                var files = (string[])dataObject.GetData(DataFormats.FileDrop);
                return files.Where(f => IsValidFileExtension(Path.GetExtension(f)));
            }
            
            return null;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error extracting file paths from drag data");
            return null;
        }
    }
    
    /// <inheritdoc />
    public FrameworkElement CreateDragPreview(IEnumerable<ItemViewModel> items)
    {
        try
        {
            var itemList = items.ToList();
            
            var preview = new Border
            {
                Background = new SolidColorBrush(Color.FromArgb(200, 45, 45, 45)),
                BorderBrush = new SolidColorBrush(Color.FromArgb(255, 0, 120, 215)),
                BorderThickness = new Thickness(1),
                CornerRadius = new CornerRadius(4),
                Padding = new Thickness(8),
                MaxWidth = 300
            };
            
            if (itemList.Count == 1)
            {
                // Single item preview
                var item = itemList.First();
                var content = new StackPanel
                {
                    Orientation = Orientation.Horizontal
                };
                
                // Thumbnail
                if (item.Thumbnail != null)
                {
                    var thumbnail = new Image
                    {
                        Source = item.Thumbnail,
                        Width = 24,
                        Height = 24,
                        Margin = new Thickness(0, 0, 8, 0)
                    };
                    content.Children.Add(thumbnail);
                }
                
                // Text
                var textBlock = new TextBlock
                {
                    Text = item.DisplayText,
                    Foreground = Brushes.White,
                    VerticalAlignment = VerticalAlignment.Center
                };
                content.Children.Add(textBlock);
                
                preview.Child = content;
            }
            else
            {
                // Multiple items preview
                var content = new StackPanel();
                
                var headerText = new TextBlock
                {
                    Text = $"{itemList.Count} items",
                    Foreground = Brushes.White,
                    FontWeight = FontWeights.Bold,
                    Margin = new Thickness(0, 0, 0, 4)
                };
                content.Children.Add(headerText);
                
                // Show first few items
                var maxPreviewItems = Math.Min(3, itemList.Count);
                for (int i = 0; i < maxPreviewItems; i++)
                {
                    var item = itemList[i];
                    var itemPanel = new StackPanel
                    {
                        Orientation = Orientation.Horizontal,
                        Margin = new Thickness(0, 2, 0, 2)
                    };
                    
                    if (item.Thumbnail != null)
                    {
                        var thumbnail = new Image
                        {
                            Source = item.Thumbnail,
                            Width = 16,
                            Height = 16,
                            Margin = new Thickness(0, 0, 4, 0)
                        };
                        itemPanel.Children.Add(thumbnail);
                    }
                    
                    var textBlock = new TextBlock
                    {
                        Text = item.DisplayText,
                        Foreground = Brushes.LightGray,
                        FontSize = 11,
                        VerticalAlignment = VerticalAlignment.Center
                    };
                    itemPanel.Children.Add(textBlock);
                    
                    content.Children.Add(itemPanel);
                }
                
                if (itemList.Count > maxPreviewItems)
                {
                    var moreText = new TextBlock
                    {
                        Text = $"... and {itemList.Count - maxPreviewItems} more",
                        Foreground = Brushes.LightGray,
                        FontSize = 10,
                        FontStyle = FontStyles.Italic,
                        Margin = new Thickness(0, 4, 0, 0)
                    };
                    content.Children.Add(moreText);
                }
                
                preview.Child = content;
            }
            
            return preview;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error creating drag preview");
            
            // Fallback preview
            return new TextBlock
            {
                Text = "Dragging items...",
                Foreground = Brushes.White,
                Background = new SolidColorBrush(Color.FromArgb(200, 45, 45, 45)),
                Padding = new Thickness(8)
            };
        }
    }
    
    /// <summary>
    /// Creates a drag preview for sprite operations
    /// </summary>
    /// <param name="spriteId">Sprite ID</param>
    /// <param name="spritePreview">Sprite preview image</param>
    /// <returns>Drag preview element</returns>
    public FrameworkElement CreateSpriteDragPreview(int spriteId, BitmapSource? spritePreview)
    {
        try
        {
            var preview = new Border
            {
                Background = new SolidColorBrush(Color.FromArgb(200, 45, 45, 45)),
                BorderBrush = new SolidColorBrush(Color.FromArgb(255, 0, 120, 215)),
                BorderThickness = new Thickness(1),
                CornerRadius = new CornerRadius(4),
                Padding = new Thickness(8),
                MaxWidth = 200
            };
            
            var content = new StackPanel
            {
                Orientation = Orientation.Horizontal
            };
            
            // Sprite preview
            if (spritePreview != null)
            {
                var image = new Image
                {
                    Source = spritePreview,
                    Width = 32,
                    Height = 32,
                    Margin = new Thickness(0, 0, 8, 0),
                    RenderOptions = { BitmapScalingMode = BitmapScalingMode.NearestNeighbor }
                };
                content.Children.Add(image);
            }
            else
            {
                // Fallback icon
                var icon = new Border
                {
                    Width = 32,
                    Height = 32,
                    Background = new SolidColorBrush(Color.FromArgb(100, 255, 255, 255)),
                    BorderBrush = new SolidColorBrush(Color.FromArgb(150, 255, 255, 255)),
                    BorderThickness = new Thickness(1),
                    CornerRadius = new CornerRadius(2),
                    Margin = new Thickness(0, 0, 8, 0),
                    Child = new TextBlock
                    {
                        Text = "🖼",
                        HorizontalAlignment = HorizontalAlignment.Center,
                        VerticalAlignment = VerticalAlignment.Center,
                        FontSize = 16
                    }
                };
                content.Children.Add(icon);
            }
            
            // Text
            var textBlock = new TextBlock
            {
                Text = $"Sprite {spriteId}",
                Foreground = Brushes.White,
                VerticalAlignment = VerticalAlignment.Center,
                FontWeight = FontWeights.Medium
            };
            content.Children.Add(textBlock);
            
            preview.Child = content;
            return preview;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error creating sprite drag preview");
            
            // Fallback preview
            return new TextBlock
            {
                Text = $"Sprite {spriteId}",
                Foreground = Brushes.White,
                Background = new SolidColorBrush(Color.FromArgb(200, 45, 45, 45)),
                Padding = new Thickness(8)
            };
        }
    }

    /// <inheritdoc />
    public void ShowDropFeedback(FrameworkElement target, bool isValidDrop)
    {
        try
        {
            // Create animated feedback
            var borderBrush = isValidDrop 
                ? new SolidColorBrush(Color.FromRgb(0, 120, 215))
                : new SolidColorBrush(Color.FromRgb(232, 17, 35));
            
            var borderThickness = new Thickness(2);
            var opacity = isValidDrop ? 0.9 : 0.6;
            
            // Apply properties with animation
            target.SetValue(Border.BorderBrushProperty, borderBrush);
            target.SetValue(Border.BorderThicknessProperty, borderThickness);
            
            // Animate opacity change
            var opacityAnimation = new DoubleAnimation
            {
                To = opacity,
                Duration = TimeSpan.FromMilliseconds(150),
                EasingFunction = new QuadraticEase { EasingMode = EasingMode.EaseOut }
            };
            
            target.BeginAnimation(UIElement.OpacityProperty, opacityAnimation);
            
            // Add subtle scale animation for valid drops
            if (isValidDrop)
            {
                var scaleTransform = target.RenderTransform as ScaleTransform ?? new ScaleTransform();
                target.RenderTransform = scaleTransform;
                target.RenderTransformOrigin = new Point(0.5, 0.5);
                
                var scaleAnimation = new DoubleAnimation
                {
                    To = 1.02,
                    Duration = TimeSpan.FromMilliseconds(150),
                    AutoReverse = true,
                    EasingFunction = new QuadraticEase { EasingMode = EasingMode.EaseInOut }
                };
                
                scaleTransform.BeginAnimation(ScaleTransform.ScaleXProperty, scaleAnimation);
                scaleTransform.BeginAnimation(ScaleTransform.ScaleYProperty, scaleAnimation);
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error showing drop feedback");
        }
    }
    
    /// <inheritdoc />
    public void HideDropFeedback(FrameworkElement target)
    {
        try
        {
            // Animate back to normal state
            var opacityAnimation = new DoubleAnimation
            {
                To = 1.0,
                Duration = TimeSpan.FromMilliseconds(150),
                EasingFunction = new QuadraticEase { EasingMode = EasingMode.EaseOut }
            };
            
            target.BeginAnimation(UIElement.OpacityProperty, opacityAnimation);
            
            // Reset scale transform
            if (target.RenderTransform is ScaleTransform scaleTransform)
            {
                var scaleAnimation = new DoubleAnimation
                {
                    To = 1.0,
                    Duration = TimeSpan.FromMilliseconds(150),
                    EasingFunction = new QuadraticEase { EasingMode = EasingMode.EaseOut }
                };
                
                scaleTransform.BeginAnimation(ScaleTransform.ScaleXProperty, scaleAnimation);
                scaleTransform.BeginAnimation(ScaleTransform.ScaleYProperty, scaleAnimation);
            }
            
            // Clear properties after animation
            var timer = new System.Windows.Threading.DispatcherTimer
            {
                Interval = TimeSpan.FromMilliseconds(200)
            };
            
            timer.Tick += (s, e) =>
            {
                timer.Stop();
                target.ClearValue(Border.BorderBrushProperty);
                target.ClearValue(Border.BorderThicknessProperty);
                target.ClearValue(UIElement.OpacityProperty);
                target.ClearValue(UIElement.RenderTransformProperty);
            };
            
            timer.Start();
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error hiding drop feedback");
        }
    }
    
    private static bool IsValidFileExtension(string extension)
    {
        var validExtensions = new[] { ".otb", ".dat", ".spr" };
        return validExtensions.Contains(extension.ToLowerInvariant());
    }
}