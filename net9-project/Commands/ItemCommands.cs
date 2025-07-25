using ItemEditor.Models;
using ItemEditor.Services;
using ItemEditor.ViewModels;
using Microsoft.Extensions.Logging;

namespace ItemEditor.Commands;

/// <summary>
/// Command to change an item property
/// </summary>
public class ChangeItemPropertyCommand : UndoableCommandBase
{
    private readonly ItemViewModel _item;
    private readonly string _propertyName;
    private readonly object? _newValue;
    private readonly object? _oldValue;
    private readonly ILogger _logger;
    
    public ChangeItemPropertyCommand(ItemViewModel item, string propertyName, object? newValue, 
        object? oldValue, ILogger logger)
        : base($"Change {propertyName}")
    {
        _item = item ?? throw new ArgumentNullException(nameof(item));
        _propertyName = propertyName ?? throw new ArgumentNullException(nameof(propertyName));
        _newValue = newValue;
        _oldValue = oldValue;
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
        
        // Add metadata
        Metadata["ItemId"] = item.Id;
        Metadata["PropertyName"] = propertyName;
        Metadata["OldValue"] = oldValue;
        Metadata["NewValue"] = newValue;
    }
    
    public override async Task ExecuteAsync()
    {
        try
        {
            await SetPropertyValueAsync(_newValue);
            _logger.LogDebug("Changed property {PropertyName} on item {ItemId} to {NewValue}", 
                _propertyName, _item.Id, _newValue);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error changing property {PropertyName} on item {ItemId}", 
                _propertyName, _item.Id);
            throw;
        }
    }
    
    public override async Task UndoAsync()
    {
        try
        {
            await SetPropertyValueAsync(_oldValue);
            _logger.LogDebug("Reverted property {PropertyName} on item {ItemId} to {OldValue}", 
                _propertyName, _item.Id, _oldValue);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error reverting property {PropertyName} on item {ItemId}", 
                _propertyName, _item.Id);
            throw;
        }
    }
    
    private async Task SetPropertyValueAsync(object? value)
    {
        // Use reflection to set the property value
        var property = _item.GetType().GetProperty(_propertyName);
        if (property == null)
        {
            throw new InvalidOperationException($"Property {_propertyName} not found on item");
        }
        
        if (!property.CanWrite)
        {
            throw new InvalidOperationException($"Property {_propertyName} is read-only");
        }
        
        // Handle async property setters if needed
        if (property.SetMethod?.IsStatic == false)
        {
            property.SetValue(_item, value);
        }
        
        // Mark item as dirty
        _item.IsDirty = true;
        
        await Task.CompletedTask;
    }
}

/// <summary>
/// Command to add a new item
/// </summary>
public class AddItemCommand : UndoableCommandBase
{
    private readonly ItemViewModel _item;
    private readonly IItemService _itemService;
    private readonly ILogger _logger;
    private bool _wasAdded;
    
    public AddItemCommand(ItemViewModel item, IItemService itemService, ILogger logger)
        : base($"Add item {item.DisplayText}")
    {
        _item = item ?? throw new ArgumentNullException(nameof(item));
        _itemService = itemService ?? throw new ArgumentNullException(nameof(itemService));
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
        
        // Add metadata
        Metadata["ItemId"] = item.Id;
        Metadata["ItemName"] = item.DisplayText;
    }
    
    public override async Task ExecuteAsync()
    {
        try
        {
            await _itemService.AddItemAsync(_item);
            _wasAdded = true;
            
            _logger.LogDebug("Added item {ItemId}: {ItemName}", _item.Id, _item.DisplayText);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error adding item {ItemId}: {ItemName}", _item.Id, _item.DisplayText);
            throw;
        }
    }
    
    public override async Task UndoAsync()
    {
        try
        {
            if (_wasAdded)
            {
                await _itemService.RemoveItemAsync(_item);
                _wasAdded = false;
                
                _logger.LogDebug("Removed item {ItemId}: {ItemName}", _item.Id, _item.DisplayText);
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error removing item {ItemId}: {ItemName}", _item.Id, _item.DisplayText);
            throw;
        }
    }
    
    public override bool CanUndo => _wasAdded;
}

/// <summary>
/// Command to remove an item
/// </summary>
public class RemoveItemCommand : UndoableCommandBase
{
    private readonly ItemViewModel _item;
    private readonly IItemService _itemService;
    private readonly ILogger _logger;
    private bool _wasRemoved;
    private int _originalIndex;
    
    public RemoveItemCommand(ItemViewModel item, IItemService itemService, ILogger logger)
        : base($"Remove item {item.DisplayText}")
    {
        _item = item ?? throw new ArgumentNullException(nameof(item));
        _itemService = itemService ?? throw new ArgumentNullException(nameof(itemService));
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
        
        // Add metadata
        Metadata["ItemId"] = item.Id;
        Metadata["ItemName"] = item.DisplayText;
    }
    
    public override async Task ExecuteAsync()
    {
        try
        {
            // Store the original index for undo
            _originalIndex = await _itemService.GetItemIndexAsync(_item);
            
            await _itemService.RemoveItemAsync(_item);
            _wasRemoved = true;
            
            _logger.LogDebug("Removed item {ItemId}: {ItemName}", _item.Id, _item.DisplayText);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error removing item {ItemId}: {ItemName}", _item.Id, _item.DisplayText);
            throw;
        }
    }
    
    public override async Task UndoAsync()
    {
        try
        {
            if (_wasRemoved)
            {
                await _itemService.InsertItemAsync(_item, _originalIndex);
                _wasRemoved = false;
                
                _logger.LogDebug("Restored item {ItemId}: {ItemName} at index {Index}", 
                    _item.Id, _item.DisplayText, _originalIndex);
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error restoring item {ItemId}: {ItemName}", _item.Id, _item.DisplayText);
            throw;
        }
    }
    
    public override bool CanUndo => _wasRemoved;
}

/// <summary>
/// Command to duplicate an item
/// </summary>
public class DuplicateItemCommand : UndoableCommandBase
{
    private readonly ItemViewModel _originalItem;
    private readonly IItemService _itemService;
    private readonly ILogger _logger;
    private ItemViewModel? _duplicatedItem;
    
    public DuplicateItemCommand(ItemViewModel originalItem, IItemService itemService, ILogger logger)
        : base($"Duplicate item {originalItem.DisplayText}")
    {
        _originalItem = originalItem ?? throw new ArgumentNullException(nameof(originalItem));
        _itemService = itemService ?? throw new ArgumentNullException(nameof(itemService));
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
        
        // Add metadata
        Metadata["OriginalItemId"] = originalItem.Id;
        Metadata["OriginalItemName"] = originalItem.DisplayText;
    }
    
    public override async Task ExecuteAsync()
    {
        try
        {
            _duplicatedItem = await _itemService.DuplicateItemAsync(_originalItem);
            
            _logger.LogDebug("Duplicated item {OriginalId}: {OriginalName} to {NewId}: {NewName}", 
                _originalItem.Id, _originalItem.DisplayText, _duplicatedItem.Id, _duplicatedItem.DisplayText);
            
            // Update metadata
            Metadata["DuplicatedItemId"] = _duplicatedItem.Id;
            Metadata["DuplicatedItemName"] = _duplicatedItem.DisplayText;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error duplicating item {ItemId}: {ItemName}", 
                _originalItem.Id, _originalItem.DisplayText);
            throw;
        }
    }
    
    public override async Task UndoAsync()
    {
        try
        {
            if (_duplicatedItem != null)
            {
                await _itemService.RemoveItemAsync(_duplicatedItem);
                
                _logger.LogDebug("Removed duplicated item {ItemId}: {ItemName}", 
                    _duplicatedItem.Id, _duplicatedItem.DisplayText);
                
                _duplicatedItem = null;
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error removing duplicated item");
            throw;
        }
    }
    
    public override bool CanUndo => _duplicatedItem != null;
}

/// <summary>
/// Command to move an item to a different position
/// </summary>
public class MoveItemCommand : UndoableCommandBase
{
    private readonly ItemViewModel _item;
    private readonly int _newIndex;
    private readonly int _oldIndex;
    private readonly IItemService _itemService;
    private readonly ILogger _logger;
    
    public MoveItemCommand(ItemViewModel item, int newIndex, int oldIndex, 
        IItemService itemService, ILogger logger)
        : base($"Move item {item.DisplayText}")
    {
        _item = item ?? throw new ArgumentNullException(nameof(item));
        _newIndex = newIndex;
        _oldIndex = oldIndex;
        _itemService = itemService ?? throw new ArgumentNullException(nameof(itemService));
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
        
        // Add metadata
        Metadata["ItemId"] = item.Id;
        Metadata["ItemName"] = item.DisplayText;
        Metadata["OldIndex"] = oldIndex;
        Metadata["NewIndex"] = newIndex;
    }
    
    public override async Task ExecuteAsync()
    {
        try
        {
            await _itemService.MoveItemAsync(_item, _newIndex);
            
            _logger.LogDebug("Moved item {ItemId}: {ItemName} from index {OldIndex} to {NewIndex}", 
                _item.Id, _item.DisplayText, _oldIndex, _newIndex);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error moving item {ItemId}: {ItemName}", _item.Id, _item.DisplayText);
            throw;
        }
    }
    
    public override async Task UndoAsync()
    {
        try
        {
            await _itemService.MoveItemAsync(_item, _oldIndex);
            
            _logger.LogDebug("Moved item {ItemId}: {ItemName} back from index {NewIndex} to {OldIndex}", 
                _item.Id, _item.DisplayText, _newIndex, _oldIndex);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error moving item back {ItemId}: {ItemName}", _item.Id, _item.DisplayText);
            throw;
        }
    }
}

/// <summary>
/// Command to assign a sprite to an item
/// </summary>
public class AssignSpriteCommand : UndoableCommandBase
{
    private readonly ItemViewModel _item;
    private readonly int _spriteId;
    private readonly int _frameIndex;
    private readonly int _oldSpriteId;
    private readonly ILogger _logger;
    
    public AssignSpriteCommand(ItemViewModel item, int spriteId, int frameIndex, 
        int oldSpriteId, ILogger logger)
        : base($"Assign sprite {spriteId} to item {item.DisplayText}")
    {
        _item = item ?? throw new ArgumentNullException(nameof(item));
        _spriteId = spriteId;
        _frameIndex = frameIndex;
        _oldSpriteId = oldSpriteId;
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
        
        // Add metadata
        Metadata["ItemId"] = item.Id;
        Metadata["ItemName"] = item.DisplayText;
        Metadata["SpriteId"] = spriteId;
        Metadata["FrameIndex"] = frameIndex;
        Metadata["OldSpriteId"] = oldSpriteId;
    }
    
    public override async Task ExecuteAsync()
    {
        try
        {
            // Ensure the sprite list is large enough
            while (_item.Model.SpriteIds.Count <= _frameIndex)
            {
                _item.Model.SpriteIds.Add(0);
            }
            
            _item.Model.SpriteIds[_frameIndex] = _spriteId;
            _item.IsDirty = true;
            
            // Refresh thumbnail if this is the first frame
            if (_frameIndex == 0)
            {
                await _item.RefreshThumbnailAsync();
            }
            
            _logger.LogDebug("Assigned sprite {SpriteId} to item {ItemId} at frame {FrameIndex}", 
                _spriteId, _item.Id, _frameIndex);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error assigning sprite {SpriteId} to item {ItemId}", _spriteId, _item.Id);
            throw;
        }
    }
    
    public override async Task UndoAsync()
    {
        try
        {
            if (_frameIndex < _item.Model.SpriteIds.Count)
            {
                _item.Model.SpriteIds[_frameIndex] = _oldSpriteId;
                _item.IsDirty = true;
                
                // Refresh thumbnail if this is the first frame
                if (_frameIndex == 0)
                {
                    await _item.RefreshThumbnailAsync();
                }
                
                _logger.LogDebug("Reverted sprite assignment on item {ItemId} at frame {FrameIndex} to {OldSpriteId}", 
                    _item.Id, _frameIndex, _oldSpriteId);
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error reverting sprite assignment on item {ItemId}", _item.Id);
            throw;
        }
    }
}

/// <summary>
/// Placeholder interface for item service - this would be implemented elsewhere
/// </summary>
public interface IItemService
{
    Task AddItemAsync(ItemViewModel item);
    Task RemoveItemAsync(ItemViewModel item);
    Task InsertItemAsync(ItemViewModel item, int index);
    Task<int> GetItemIndexAsync(ItemViewModel item);
    Task<ItemViewModel> DuplicateItemAsync(ItemViewModel item);
    Task MoveItemAsync(ItemViewModel item, int newIndex);
}