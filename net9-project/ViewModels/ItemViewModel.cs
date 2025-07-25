using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using System.Collections.ObjectModel;
using System.Windows.Media.Imaging;
using ItemEditor.Models;
using ItemEditor.Services;
using System.ComponentModel;

namespace ItemEditor.ViewModels;

/// <summary>
/// ViewModel wrapper for Item with CommunityToolkit.Mvvm source generators
/// </summary>
public partial class ItemViewModel : ObservableObject
{
    private readonly Item _item;
    private readonly IImageService _imageService;
    private readonly IFileService _fileService;

    [ObservableProperty]
    private bool _isSelected;

    [ObservableProperty]
    private bool _isExpanded;

    [ObservableProperty]
    private bool _isLoading;

    [ObservableProperty]
    private string _searchText = string.Empty;

    [ObservableProperty]
    private ObservableCollection<string> _tags = new();

    [ObservableProperty]
    private ObservableCollection<ItemViewModel> _relatedItems = new();

    /// <summary>
    /// Initializes a new instance of the ItemViewModel class
    /// </summary>
    /// <param name="item">The item model</param>
    /// <param name="imageService">Image service for thumbnail operations</param>
    /// <param name="fileService">File service for save operations</param>
    public ItemViewModel(Item item, IImageService imageService, IFileService fileService)
    {
        _item = item ?? throw new ArgumentNullException(nameof(item));
        _imageService = imageService ?? throw new ArgumentNullException(nameof(imageService));
        _fileService = fileService ?? throw new ArgumentNullException(nameof(fileService));

        // Subscribe to item property changes
        _item.PropertyChanged += OnItemPropertyChanged;
        _item.ItemChanged += OnItemChanged;

        // Initialize commands
        RefreshThumbnailCommand = new AsyncRelayCommand(RefreshThumbnailAsync);
        SaveItemCommand = new AsyncRelayCommand(SaveItemAsync, CanSaveItem);
        ResetChangesCommand = new RelayCommand(ResetChanges, CanResetChanges);
        ValidateCommand = new RelayCommand(ValidateItem);
        CloneItemCommand = new RelayCommand(CloneItem);
        AddTagCommand = new RelayCommand<string>(AddTag);
        RemoveTagCommand = new RelayCommand<string>(RemoveTag);
    }

    #region Item Properties (Wrapped)

    /// <summary>
    /// Gets or sets the item name
    /// </summary>
    public string Name
    {
        get => _item.Name;
        set
        {
            if (_item.Name != value)
            {
                _item.Name = value;
                OnPropertyChanged();
                SaveItemCommand.NotifyCanExecuteChanged();
                ResetChangesCommand.NotifyCanExecuteChanged();
            }
        }
    }

    /// <summary>
    /// Gets or sets the item ID
    /// </summary>
    public ushort Id
    {
        get => _item.Id;
        set
        {
            if (_item.Id != value)
            {
                _item.Id = value;
                OnPropertyChanged();
                SaveItemCommand.NotifyCanExecuteChanged();
                ResetChangesCommand.NotifyCanExecuteChanged();
            }
        }
    }

    /// <summary>
    /// Gets or sets the client ID
    /// </summary>
    public ushort ClientId
    {
        get => _item.ClientId;
        set
        {
            if (_item.ClientId != value)
            {
                _item.ClientId = value;
                OnPropertyChanged();
                SaveItemCommand.NotifyCanExecuteChanged();
                ResetChangesCommand.NotifyCanExecuteChanged();
            }
        }
    }

    /// <summary>
    /// Gets or sets the item description
    /// </summary>
    public string Description
    {
        get => _item.Description;
        set
        {
            if (_item.Description != value)
            {
                _item.Description = value;
                OnPropertyChanged();
                SaveItemCommand.NotifyCanExecuteChanged();
                ResetChangesCommand.NotifyCanExecuteChanged();
            }
        }
    }

    /// <summary>
    /// Gets or sets the item type
    /// </summary>
    public ItemType Type
    {
        get => _item.Type;
        set
        {
            if (_item.Type != value)
            {
                _item.Type = value;
                OnPropertyChanged();
                OnPropertyChanged(nameof(TypeDisplayName));
                SaveItemCommand.NotifyCanExecuteChanged();
                ResetChangesCommand.NotifyCanExecuteChanged();
            }
        }
    }

    /// <summary>
    /// Gets or sets whether the item is stackable
    /// </summary>
    public bool IsStackable
    {
        get => _item.IsStackable;
        set
        {
            if (_item.IsStackable != value)
            {
                _item.IsStackable = value;
                OnPropertyChanged();
                SaveItemCommand.NotifyCanExecuteChanged();
                ResetChangesCommand.NotifyCanExecuteChanged();
            }
        }
    }

    /// <summary>
    /// Gets or sets whether the item is moveable
    /// </summary>
    public bool IsMoveable
    {
        get => _item.IsMoveable;
        set
        {
            if (_item.IsMoveable != value)
            {
                _item.IsMoveable = value;
                OnPropertyChanged();
                SaveItemCommand.NotifyCanExecuteChanged();
                ResetChangesCommand.NotifyCanExecuteChanged();
            }
        }
    }

    /// <summary>
    /// Gets or sets whether the item is pickupable
    /// </summary>
    public bool IsPickupable
    {
        get => _item.IsPickupable;
        set
        {
            if (_item.IsPickupable != value)
            {
                _item.IsPickupable = value;
                OnPropertyChanged();
                SaveItemCommand.NotifyCanExecuteChanged();
                ResetChangesCommand.NotifyCanExecuteChanged();
            }
        }
    }

    /// <summary>
    /// Gets or sets the item speed
    /// </summary>
    public ushort Speed
    {
        get => _item.Speed;
        set
        {
            if (_item.Speed != value)
            {
                _item.Speed = value;
                OnPropertyChanged();
                SaveItemCommand.NotifyCanExecuteChanged();
                ResetChangesCommand.NotifyCanExecuteChanged();
            }
        }
    }

    /// <summary>
    /// Gets or sets the item weight
    /// </summary>
    public ushort Weight
    {
        get => _item.Weight;
        set
        {
            if (_item.Weight != value)
            {
                _item.Weight = value;
                OnPropertyChanged();
                SaveItemCommand.NotifyCanExecuteChanged();
                ResetChangesCommand.NotifyCanExecuteChanged();
            }
        }
    }

    /// <summary>
    /// Gets or sets whether the item has light
    /// </summary>
    public bool HasLight
    {
        get => _item.HasLight;
        set
        {
            if (_item.HasLight != value)
            {
                _item.HasLight = value;
                OnPropertyChanged();
                SaveItemCommand.NotifyCanExecuteChanged();
                ResetChangesCommand.NotifyCanExecuteChanged();
            }
        }
    }

    /// <summary>
    /// Gets or sets the light level
    /// </summary>
    public ushort LightLevel
    {
        get => _item.LightLevel;
        set
        {
            if (_item.LightLevel != value)
            {
                _item.LightLevel = value;
                OnPropertyChanged();
                SaveItemCommand.NotifyCanExecuteChanged();
                ResetChangesCommand.NotifyCanExecuteChanged();
            }
        }
    }

    /// <summary>
    /// Gets or sets the light color
    /// </summary>
    public ushort LightColor
    {
        get => _item.LightColor;
        set
        {
            if (_item.LightColor != value)
            {
                _item.LightColor = value;
                OnPropertyChanged();
                SaveItemCommand.NotifyCanExecuteChanged();
                ResetChangesCommand.NotifyCanExecuteChanged();
            }
        }
    }

    /// <summary>
    /// Gets the item thumbnail
    /// </summary>
    public BitmapSource? Thumbnail => _item.Thumbnail;

    /// <summary>
    /// Gets whether the item has unsaved changes
    /// </summary>
    public bool IsDirty => _item.IsDirty;

    /// <summary>
    /// Gets whether the item has validation errors
    /// </summary>
    public bool HasErrors => _item.HasErrors;

    /// <summary>
    /// Gets validation errors
    /// </summary>
    public string ValidationErrors => _item.Error;

    /// <summary>
    /// Gets the last modified date
    /// </summary>
    public DateTime LastModified => _item.LastModified;

    /// <summary>
    /// Gets the underlying item model
    /// </summary>
    public Item Model => _item;

    #endregion

    #region Computed Properties

    /// <summary>
    /// Gets the display name for the item type
    /// </summary>
    public string TypeDisplayName => Type switch
    {
        ItemType.None => "None",
        ItemType.Ground => "Ground",
        ItemType.Container => "Container",
        ItemType.Weapon => "Weapon",
        ItemType.Ammunition => "Ammunition",
        ItemType.Armor => "Armor",
        ItemType.Charges => "Charges",
        ItemType.Teleport => "Teleport",
        ItemType.MagicField => "Magic Field",
        ItemType.Writeable => "Writeable",
        ItemType.Key => "Key",
        ItemType.Splash => "Splash",
        ItemType.Fluid => "Fluid",
        ItemType.Door => "Door",
        _ => "Unknown"
    };

    /// <summary>
    /// Gets the display text for the item
    /// </summary>
    public string DisplayText => $"{Id}: {Name}";

    /// <summary>
    /// Gets the tooltip text for the item
    /// </summary>
    public string ToolTipText => $"ID: {Id}\nClient ID: {ClientId}\nType: {TypeDisplayName}\nName: {Name}\nDescription: {Description}";

    /// <summary>
    /// Gets whether the item matches the current search
    /// </summary>
    public bool MatchesSearch
    {
        get
        {
            if (string.IsNullOrWhiteSpace(SearchText))
                return true;

            var searchLower = SearchText.ToLowerInvariant();
            return Name.ToLowerInvariant().Contains(searchLower) ||
                   Description.ToLowerInvariant().Contains(searchLower) ||
                   Id.ToString().Contains(searchLower) ||
                   TypeDisplayName.ToLowerInvariant().Contains(searchLower) ||
                   Tags.Any(tag => tag.ToLowerInvariant().Contains(searchLower));
        }
    }

    /// <summary>
    /// Gets the status text for the item
    /// </summary>
    public string StatusText
    {
        get
        {
            if (HasErrors)
                return "Has Errors";
            if (IsDirty)
                return "Modified";
            return "Saved";
        }
    }

    #endregion

    #region Commands

    /// <summary>
    /// Command to refresh the item thumbnail
    /// </summary>
    public IAsyncRelayCommand RefreshThumbnailCommand { get; }

    /// <summary>
    /// Command to save the item
    /// </summary>
    public IAsyncRelayCommand SaveItemCommand { get; }

    /// <summary>
    /// Command to reset changes
    /// </summary>
    public IRelayCommand ResetChangesCommand { get; }

    /// <summary>
    /// Command to validate the item
    /// </summary>
    public IRelayCommand ValidateCommand { get; }

    /// <summary>
    /// Command to clone the item
    /// </summary>
    public IRelayCommand CloneItemCommand { get; }

    /// <summary>
    /// Command to add a tag
    /// </summary>
    public IRelayCommand<string> AddTagCommand { get; }

    /// <summary>
    /// Command to remove a tag
    /// </summary>
    public IRelayCommand<string> RemoveTagCommand { get; }

    #endregion

    #region Command Implementations

    private async Task RefreshThumbnailAsync()
    {
        try
        {
            IsLoading = true;

            if (_item.SpriteIds.Count > 0)
            {
                // This would need to be implemented based on your sprite loading logic
                // var thumbnail = await _imageService.RenderItemAsync(_item);
                // _item.Thumbnail = thumbnail;
                OnPropertyChanged(nameof(Thumbnail));
            }
        }
        catch (Exception ex)
        {
            // Handle error (could raise an event or show notification)
            System.Diagnostics.Debug.WriteLine($"Failed to refresh thumbnail: {ex.Message}");
        }
        finally
        {
            IsLoading = false;
        }
    }

    private async Task SaveItemAsync()
    {
        try
        {
            IsLoading = true;

            // Validate before saving
            if (!_item.ValidateAll())
                return;

            // This would need to be implemented based on your save logic
            // await _fileService.SaveItemAsync(_item);
            _item.MarkAsSaved();

            OnPropertyChanged(nameof(IsDirty));
            OnPropertyChanged(nameof(StatusText));
        }
        catch (Exception ex)
        {
            // Handle error
            System.Diagnostics.Debug.WriteLine($"Failed to save item: {ex.Message}");
        }
        finally
        {
            IsLoading = false;
        }
    }

    private bool CanSaveItem() => _item.IsDirty && !_item.HasErrors;

    private void ResetChanges()
    {
        // This would need to reload the item from the original source
        // For now, just mark as saved
        _item.MarkAsSaved();
        OnPropertyChanged(nameof(IsDirty));
        OnPropertyChanged(nameof(StatusText));
    }

    private bool CanResetChanges() => _item.IsDirty;

    private void ValidateItem()
    {
        _item.ValidateAll();
        OnPropertyChanged(nameof(HasErrors));
        OnPropertyChanged(nameof(ValidationErrors));
        OnPropertyChanged(nameof(StatusText));
    }

    private void CloneItem()
    {
        var clonedItem = (Item)_item.Clone();
        clonedItem.Id = 0; // Reset ID for new item
        clonedItem.Name = $"{clonedItem.Name} (Copy)";
        
        // Raise event to notify that a new item was created
        ItemCloned?.Invoke(this, new ItemClonedEventArgs(clonedItem));
    }

    private void AddTag(string? tag)
    {
        if (!string.IsNullOrWhiteSpace(tag) && !Tags.Contains(tag))
        {
            Tags.Add(tag);
            OnPropertyChanged(nameof(MatchesSearch));
        }
    }

    private void RemoveTag(string? tag)
    {
        if (!string.IsNullOrWhiteSpace(tag) && Tags.Contains(tag))
        {
            Tags.Remove(tag);
            OnPropertyChanged(nameof(MatchesSearch));
        }
    }

    #endregion

    #region Event Handlers

    private void OnItemPropertyChanged(object? sender, PropertyChangedEventArgs e)
    {
        // Forward property changes from the model
        OnPropertyChanged(e.PropertyName);

        // Update computed properties
        switch (e.PropertyName)
        {
            case nameof(Item.Type):
                OnPropertyChanged(nameof(TypeDisplayName));
                OnPropertyChanged(nameof(ToolTipText));
                break;
            case nameof(Item.Name):
            case nameof(Item.Id):
                OnPropertyChanged(nameof(DisplayText));
                OnPropertyChanged(nameof(ToolTipText));
                OnPropertyChanged(nameof(MatchesSearch));
                break;
            case nameof(Item.Description):
                OnPropertyChanged(nameof(ToolTipText));
                OnPropertyChanged(nameof(MatchesSearch));
                break;
            case nameof(Item.IsDirty):
                OnPropertyChanged(nameof(IsDirty));
                OnPropertyChanged(nameof(StatusText));
                SaveItemCommand.NotifyCanExecuteChanged();
                ResetChangesCommand.NotifyCanExecuteChanged();
                break;
            case nameof(Item.HasErrors):
                OnPropertyChanged(nameof(HasErrors));
                OnPropertyChanged(nameof(StatusText));
                SaveItemCommand.NotifyCanExecuteChanged();
                break;
            case nameof(Item.Error):
                OnPropertyChanged(nameof(ValidationErrors));
                break;
            case nameof(Item.Thumbnail):
                OnPropertyChanged(nameof(Thumbnail));
                break;
        }
    }

    private void OnItemChanged(object? sender, EventArgs e)
    {
        // Item has been modified
        ItemChanged?.Invoke(this, EventArgs.Empty);
    }

    partial void OnSearchTextChanged(string value)
    {
        OnPropertyChanged(nameof(MatchesSearch));
    }

    #endregion

    #region Events

    /// <summary>
    /// Event raised when the item is changed
    /// </summary>
    public event EventHandler? ItemChanged;

    /// <summary>
    /// Event raised when the item is cloned
    /// </summary>
    public event EventHandler<ItemClonedEventArgs>? ItemCloned;

    #endregion

    #region Public Methods

    /// <summary>
    /// Updates the search filter
    /// </summary>
    /// <param name="searchText">Search text</param>
    public void UpdateSearch(string searchText)
    {
        SearchText = searchText;
    }

    /// <summary>
    /// Gets a custom property value
    /// </summary>
    /// <typeparam name="T">Property type</typeparam>
    /// <param name="key">Property key</param>
    /// <param name="defaultValue">Default value</param>
    /// <returns>Property value</returns>
    public T GetCustomProperty<T>(string key, T defaultValue = default!)
    {
        return _item.GetCustomProperty(key, defaultValue);
    }

    /// <summary>
    /// Sets a custom property value
    /// </summary>
    /// <param name="key">Property key</param>
    /// <param name="value">Property value</param>
    public void SetCustomProperty(string key, object value)
    {
        _item.SetCustomProperty(key, value);
    }

    /// <summary>
    /// Refreshes all computed properties
    /// </summary>
    public void RefreshComputedProperties()
    {
        OnPropertyChanged(nameof(TypeDisplayName));
        OnPropertyChanged(nameof(DisplayText));
        OnPropertyChanged(nameof(ToolTipText));
        OnPropertyChanged(nameof(MatchesSearch));
        OnPropertyChanged(nameof(StatusText));
    }

    #endregion
}

/// <summary>
/// Event arguments for item cloned event
/// </summary>
public class ItemClonedEventArgs : EventArgs
{
    /// <summary>
    /// Gets the cloned item
    /// </summary>
    public Item ClonedItem { get; }

    /// <summary>
    /// Initializes a new instance of the ItemClonedEventArgs class
    /// </summary>
    /// <param name="clonedItem">The cloned item</param>
    public ItemClonedEventArgs(Item clonedItem)
    {
        ClonedItem = clonedItem;
    }
}