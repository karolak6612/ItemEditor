using System.Windows;
using ItemEditor.ViewModels;

namespace ItemEditor.Services;

/// <summary>
/// Interface for drag and drop operations with visual feedback
/// </summary>
public interface IDragDropService
{
    /// <summary>
    /// Starts a drag operation for a single item
    /// </summary>
    /// <param name="source">Source element</param>
    /// <param name="item">Item to drag</param>
    /// <param name="allowedEffects">Allowed drag effects</param>
    /// <returns>Result of the drag operation</returns>
    DragDropEffects StartDrag(DependencyObject source, ItemViewModel item, DragDropEffects allowedEffects);
    
    /// <summary>
    /// Starts a drag operation for multiple items
    /// </summary>
    /// <param name="source">Source element</param>
    /// <param name="items">Items to drag</param>
    /// <param name="allowedEffects">Allowed drag effects</param>
    /// <returns>Result of the drag operation</returns>
    DragDropEffects StartDrag(DependencyObject source, IEnumerable<ItemViewModel> items, DragDropEffects allowedEffects);
    
    /// <summary>
    /// Starts a drag operation for files
    /// </summary>
    /// <param name="source">Source element</param>
    /// <param name="filePaths">File paths to drag</param>
    /// <param name="allowedEffects">Allowed drag effects</param>
    /// <returns>Result of the drag operation</returns>
    DragDropEffects StartFileDrag(DependencyObject source, IEnumerable<string> filePaths, DragDropEffects allowedEffects);
    
    /// <summary>
    /// Starts a drag operation for a sprite
    /// </summary>
    /// <param name="source">Source element</param>
    /// <param name="spriteId">Sprite ID to drag</param>
    /// <param name="spritePreview">Sprite preview image</param>
    /// <param name="allowedEffects">Allowed drag effects</param>
    /// <returns>Result of the drag operation</returns>
    DragDropEffects StartSpriteDrag(DependencyObject source, int spriteId, System.Windows.Media.Imaging.BitmapSource? spritePreview, DragDropEffects allowedEffects);
    
    /// <summary>
    /// Validates if a drop operation is valid
    /// </summary>
    /// <param name="dragEventArgs">Drag event arguments</param>
    /// <param name="targetType">Target type</param>
    /// <returns>True if drop is valid</returns>
    bool ValidateDrop(DragEventArgs dragEventArgs, Type targetType);
    
    /// <summary>
    /// Extracts items from drag data
    /// </summary>
    /// <param name="dataObject">Data object</param>
    /// <returns>Extracted items or null</returns>
    IEnumerable<ItemViewModel>? ExtractItems(IDataObject dataObject);
    
    /// <summary>
    /// Extracts file paths from drag data
    /// </summary>
    /// <param name="dataObject">Data object</param>
    /// <returns>Extracted file paths or null</returns>
    IEnumerable<string>? ExtractFilePaths(IDataObject dataObject);
    
    /// <summary>
    /// Extracts sprite ID from drag data
    /// </summary>
    /// <param name="dataObject">Data object</param>
    /// <returns>Sprite ID or null</returns>
    int? ExtractSpriteId(IDataObject dataObject);
    
    /// <summary>
    /// Extracts sprite preview from drag data
    /// </summary>
    /// <param name="dataObject">Data object</param>
    /// <returns>Sprite preview or null</returns>
    System.Windows.Media.Imaging.BitmapSource? ExtractSpritePreview(IDataObject dataObject);
    
    /// <summary>
    /// Creates a drag preview element
    /// </summary>
    /// <param name="items">Items to create preview for</param>
    /// <returns>Preview element</returns>
    FrameworkElement CreateDragPreview(IEnumerable<ItemViewModel> items);
    
    /// <summary>
    /// Creates a drag preview for sprite operations
    /// </summary>
    /// <param name="spriteId">Sprite ID</param>
    /// <param name="spritePreview">Sprite preview image</param>
    /// <returns>Preview element</returns>
    FrameworkElement CreateSpriteDragPreview(int spriteId, System.Windows.Media.Imaging.BitmapSource? spritePreview);
    
    /// <summary>
    /// Shows drop feedback on target element
    /// </summary>
    /// <param name="target">Target element</param>
    /// <param name="isValidDrop">Whether drop is valid</param>
    void ShowDropFeedback(FrameworkElement target, bool isValidDrop);
    
    /// <summary>
    /// Hides drop feedback from target element
    /// </summary>
    /// <param name="target">Target element</param>
    void HideDropFeedback(FrameworkElement target);
}