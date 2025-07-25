using System.Windows;
using ItemEditor.ViewModels;

namespace ItemEditor.Models;

/// <summary>
/// Event arguments for file drag and drop operations
/// </summary>
public class FileDragDropEventArgs : EventArgs
{
    /// <summary>
    /// Initializes a new instance of the FileDragDropEventArgs class
    /// </summary>
    /// <param name="filePaths">File paths that were dropped</param>
    /// <param name="effects">Drag drop effects</param>
    public FileDragDropEventArgs(IEnumerable<string> filePaths, DragDropEffects effects)
    {
        FilePaths = filePaths?.ToList() ?? new List<string>();
        Effects = effects;
    }
    
    /// <summary>
    /// Gets the file paths that were dropped
    /// </summary>
    public IReadOnlyList<string> FilePaths { get; }
    
    /// <summary>
    /// Gets the drag drop effects
    /// </summary>
    public DragDropEffects Effects { get; }
}

/// <summary>
/// Event arguments for item reorder operations
/// </summary>
public class ItemReorderEventArgs : EventArgs
{
    /// <summary>
    /// Initializes a new instance of the ItemReorderEventArgs class
    /// </summary>
    /// <param name="items">Items being reordered</param>
    /// <param name="dropTarget">Target item for drop operation</param>
    public ItemReorderEventArgs(IEnumerable<ItemViewModel> items, ItemViewModel? dropTarget)
    {
        Items = items?.ToList() ?? new List<ItemViewModel>();
        DropTarget = dropTarget;
    }
    
    /// <summary>
    /// Gets the items being reordered
    /// </summary>
    public IReadOnlyList<ItemViewModel> Items { get; }
    
    /// <summary>
    /// Gets the target item for the drop operation
    /// </summary>
    public ItemViewModel? DropTarget { get; }
}

/// <summary>
/// Event arguments for sprite drag and drop operations
/// </summary>
public class SpriteDragDropEventArgs : EventArgs
{
    /// <summary>
    /// Initializes a new instance of the SpriteDragDropEventArgs class
    /// </summary>
    /// <param name="spriteId">Sprite ID being dragged</param>
    /// <param name="targetItem">Target item for sprite assignment</param>
    /// <param name="effects">Drag drop effects</param>
    public SpriteDragDropEventArgs(int spriteId, ItemViewModel? targetItem, DragDropEffects effects)
    {
        SpriteId = spriteId;
        TargetItem = targetItem;
        Effects = effects;
    }
    
    /// <summary>
    /// Gets the sprite ID being dragged
    /// </summary>
    public int SpriteId { get; }
    
    /// <summary>
    /// Gets the target item for sprite assignment
    /// </summary>
    public ItemViewModel? TargetItem { get; }
    
    /// <summary>
    /// Gets the drag drop effects
    /// </summary>
    public DragDropEffects Effects { get; }
}