using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using ItemEditor.ViewModels;

namespace ItemEditor.Controls;

/// <summary>
/// Property editor control with modern validation and WPFUI styling
/// </summary>
public partial class PropertyEditor : UserControl, INotifyPropertyChanged
{
    #region Dependency Properties

    /// <summary>
    /// Selected item dependency property
    /// </summary>
    public static readonly DependencyProperty SelectedItemProperty =
        DependencyProperty.Register(
            nameof(SelectedItem),
            typeof(ItemViewModel),
            typeof(PropertyEditor),
            new FrameworkPropertyMetadata(null, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, OnSelectedItemChanged));

    /// <summary>
    /// Read-only mode dependency property
    /// </summary>
    public static readonly DependencyProperty IsReadOnlyProperty =
        DependencyProperty.Register(
            nameof(IsReadOnly),
            typeof(bool),
            typeof(PropertyEditor),
            new PropertyMetadata(false));

    /// <summary>
    /// Show advanced properties dependency property
    /// </summary>
    public static readonly DependencyProperty ShowAdvancedPropertiesProperty =
        DependencyProperty.Register(
            nameof(ShowAdvancedProperties),
            typeof(bool),
            typeof(PropertyEditor),
            new PropertyMetadata(false));

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
    /// Gets or sets whether the editor is in read-only mode
    /// </summary>
    public bool IsReadOnly
    {
        get => (bool)GetValue(IsReadOnlyProperty);
        set => SetValue(IsReadOnlyProperty, value);
    }

    /// <summary>
    /// Gets or sets whether to show advanced properties
    /// </summary>
    public bool ShowAdvancedProperties
    {
        get => (bool)GetValue(ShowAdvancedPropertiesProperty);
        set => SetValue(ShowAdvancedPropertiesProperty, value);
    }

    private string _newPropertyKey = string.Empty;
    /// <summary>
    /// Gets or sets the new property key for adding custom properties
    /// </summary>
    public string NewPropertyKey
    {
        get => _newPropertyKey;
        set => SetProperty(ref _newPropertyKey, value);
    }

    private string _newPropertyValue = string.Empty;
    /// <summary>
    /// Gets or sets the new property value for adding custom properties
    /// </summary>
    public string NewPropertyValue
    {
        get => _newPropertyValue;
        set => SetProperty(ref _newPropertyValue, value);
    }

    private string _spriteIdsText = string.Empty;
    /// <summary>
    /// Gets or sets the sprite IDs as comma-separated text
    /// </summary>
    public string SpriteIdsText
    {
        get => _spriteIdsText;
        set
        {
            if (SetProperty(ref _spriteIdsText, value))
            {
                UpdateSpriteIds();
            }
        }
    }

    /// <summary>
    /// Gets the custom properties collection for display
    /// </summary>
    public ObservableCollection<CustomPropertyViewModel> CustomProperties { get; } = new();

    #endregion

    #region Commands

    /// <summary>
    /// Command to add a custom property
    /// </summary>
    public ICommand AddCustomPropertyCommand { get; }

    /// <summary>
    /// Command to remove a custom property
    /// </summary>
    public ICommand RemoveCustomPropertyCommand { get; }

    /// <summary>
    /// Command to view sprites
    /// </summary>
    public ICommand ViewSpritesCommand { get; }

    /// <summary>
    /// Command to reset all properties
    /// </summary>
    public ICommand ResetAllCommand { get; }

    /// <summary>
    /// Command to validate all properties
    /// </summary>
    public ICommand ValidateAllCommand { get; }

    #endregion

    #region Events

    /// <summary>
    /// Event raised when a property value changes
    /// </summary>
    public event EventHandler<PropertyChangedEventArgs>? PropertyValueChanged;

    /// <summary>
    /// Event raised when sprites are requested to be viewed
    /// </summary>
    public event EventHandler<ItemViewModel>? ViewSpritesRequested;

    /// <summary>
    /// Property changed event
    /// </summary>
    public event PropertyChangedEventHandler? PropertyChanged;

    #endregion

    #region Constructor

    /// <summary>
    /// Initializes a new instance of the PropertyEditor class
    /// </summary>
    public PropertyEditor()
    {
        InitializeComponent();

        // Initialize commands
        AddCustomPropertyCommand = new RelayCommand(AddCustomProperty, CanAddCustomProperty);
        RemoveCustomPropertyCommand = new RelayCommand<string>(RemoveCustomProperty, CanRemoveCustomProperty);
        ViewSpritesCommand = new RelayCommand<ItemViewModel>(ViewSprites, CanViewSprites);
        ResetAllCommand = new RelayCommand(ResetAll, CanResetAll);
        ValidateAllCommand = new RelayCommand(ValidateAll, CanValidateAll);

        // Set data context
        DataContext = this;
    }

    #endregion

    #region Event Handlers

    private static void OnSelectedItemChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        if (d is PropertyEditor editor)
        {
            editor.OnSelectedItemChanged(e.OldValue as ItemViewModel, e.NewValue as ItemViewModel);
        }
    }

    private void OnSelectedItemChanged(ItemViewModel? oldItem, ItemViewModel? newItem)
    {
        // Unsubscribe from old item
        if (oldItem != null)
        {
            oldItem.PropertyChanged -= OnItemPropertyChanged;
        }

        // Subscribe to new item
        if (newItem != null)
        {
            newItem.PropertyChanged += OnItemPropertyChanged;
            LoadCustomProperties();
            LoadSpriteIds();
        }
        else
        {
            CustomProperties.Clear();
            SpriteIdsText = string.Empty;
        }

        // Update command states
        CommandManager.InvalidateRequerySuggested();
    }

    private void OnItemPropertyChanged(object? sender, PropertyChangedEventArgs e)
    {
        // Forward property change notification
        PropertyValueChanged?.Invoke(this, e);

        // Update sprite IDs text if sprite IDs changed
        if (e.PropertyName == nameof(ItemViewModel.Model) && sender is ItemViewModel item)
        {
            LoadSpriteIds();
        }
    }

    #endregion

    #region Command Implementations

    private void AddCustomProperty()
    {
        if (SelectedItem == null || string.IsNullOrWhiteSpace(NewPropertyKey))
            return;

        try
        {
            SelectedItem.SetCustomProperty(NewPropertyKey.Trim(), NewPropertyValue?.Trim() ?? string.Empty);
            LoadCustomProperties();

            // Clear input fields
            NewPropertyKey = string.Empty;
            NewPropertyValue = string.Empty;
        }
        catch (Exception ex)
        {
            // Handle error (could show message box or raise event)
            System.Diagnostics.Debug.WriteLine($"Failed to add custom property: {ex.Message}");
        }
    }

    private bool CanAddCustomProperty()
    {
        return SelectedItem != null && 
               !string.IsNullOrWhiteSpace(NewPropertyKey) && 
               !IsReadOnly;
    }

    private void RemoveCustomProperty(string? key)
    {
        if (SelectedItem == null || string.IsNullOrWhiteSpace(key))
            return;

        try
        {
            var customProps = SelectedItem.Model.CustomProperties;
            if (customProps.ContainsKey(key))
            {
                customProps.Remove(key);
                LoadCustomProperties();
            }
        }
        catch (Exception ex)
        {
            System.Diagnostics.Debug.WriteLine($"Failed to remove custom property: {ex.Message}");
        }
    }

    private bool CanRemoveCustomProperty(string? key)
    {
        return SelectedItem != null && 
               !string.IsNullOrWhiteSpace(key) && 
               !IsReadOnly;
    }

    private void ViewSprites(ItemViewModel? item)
    {
        if (item != null)
        {
            ViewSpritesRequested?.Invoke(this, item);
        }
    }

    private bool CanViewSprites(ItemViewModel? item)
    {
        return item?.Model.SpriteIds.Count > 0;
    }

    private void ResetAll()
    {
        if (SelectedItem?.ResetChangesCommand.CanExecute(null) == true)
        {
            SelectedItem.ResetChangesCommand.Execute(null);
        }
    }

    private bool CanResetAll()
    {
        return SelectedItem?.IsDirty == true && !IsReadOnly;
    }

    private void ValidateAll()
    {
        if (SelectedItem?.ValidateCommand.CanExecute(null) == true)
        {
            SelectedItem.ValidateCommand.Execute(null);
        }
    }

    private bool CanValidateAll()
    {
        return SelectedItem != null;
    }

    #endregion

    #region Private Methods

    private void LoadCustomProperties()
    {
        CustomProperties.Clear();

        if (SelectedItem?.Model.CustomProperties != null)
        {
            foreach (var kvp in SelectedItem.Model.CustomProperties)
            {
                CustomProperties.Add(new CustomPropertyViewModel
                {
                    Key = kvp.Key,
                    Value = kvp.Value?.ToString() ?? string.Empty
                });
            }
        }
    }

    private void LoadSpriteIds()
    {
        if (SelectedItem?.Model.SpriteIds != null)
        {
            SpriteIdsText = string.Join(", ", SelectedItem.Model.SpriteIds);
        }
        else
        {
            SpriteIdsText = string.Empty;
        }
    }

    private void UpdateSpriteIds()
    {
        if (SelectedItem?.Model == null)
            return;

        try
        {
            var spriteIds = new List<ushort>();

            if (!string.IsNullOrWhiteSpace(SpriteIdsText))
            {
                var parts = SpriteIdsText.Split(',', StringSplitOptions.RemoveEmptyEntries);
                foreach (var part in parts)
                {
                    if (ushort.TryParse(part.Trim(), out ushort spriteId))
                    {
                        spriteIds.Add(spriteId);
                    }
                }
            }

            SelectedItem.Model.SpriteIds = spriteIds;
        }
        catch (Exception ex)
        {
            System.Diagnostics.Debug.WriteLine($"Failed to update sprite IDs: {ex.Message}");
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
    /// Refreshes the property editor display
    /// </summary>
    public void RefreshDisplay()
    {
        LoadCustomProperties();
        LoadSpriteIds();
        OnPropertyChanged(nameof(SelectedItem));
    }

    /// <summary>
    /// Validates all properties and returns whether they are valid
    /// </summary>
    /// <returns>True if all properties are valid</returns>
    public bool ValidateProperties()
    {
        if (SelectedItem?.Model == null)
            return true;

        return SelectedItem.Model.ValidateAll();
    }

    /// <summary>
    /// Focuses the first editable control
    /// </summary>
    public void FocusFirstControl()
    {
        // This would focus the first TextBox or other input control
        // Implementation depends on the specific UI structure
    }

    #endregion
}

/// <summary>
/// View model for custom properties display
/// </summary>
public class CustomPropertyViewModel
{
    /// <summary>
    /// Gets or sets the property key
    /// </summary>
    public string Key { get; set; } = string.Empty;

    /// <summary>
    /// Gets or sets the property value
    /// </summary>
    public string Value { get; set; } = string.Empty;
}