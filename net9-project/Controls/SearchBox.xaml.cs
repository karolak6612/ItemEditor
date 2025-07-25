using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using ItemEditor.Models;
using ItemEditor.ViewModels;

namespace ItemEditor.Controls;

/// <summary>
/// Advanced search and filter control with WPFUI AutoSuggestBox
/// </summary>
public partial class SearchBox : UserControl, INotifyPropertyChanged
{
    #region Dependency Properties

    /// <summary>
    /// Search text dependency property
    /// </summary>
    public static readonly DependencyProperty SearchTextProperty =
        DependencyProperty.Register(
            nameof(SearchText),
            typeof(string),
            typeof(SearchBox),
            new FrameworkPropertyMetadata(string.Empty, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, OnSearchTextChanged));

    /// <summary>
    /// Items source dependency property
    /// </summary>
    public static readonly DependencyProperty ItemsSourceProperty =
        DependencyProperty.Register(
            nameof(ItemsSource),
            typeof(ObservableCollection<ItemViewModel>),
            typeof(SearchBox),
            new PropertyMetadata(null, OnItemsSourceChanged));

    #endregion

    #region Properties

    /// <summary>
    /// Gets or sets the search text
    /// </summary>
    public string SearchText
    {
        get => (string)GetValue(SearchTextProperty);
        set => SetValue(SearchTextProperty, value);
    }

    /// <summary>
    /// Gets or sets the items source
    /// </summary>
    public ObservableCollection<ItemViewModel>? ItemsSource
    {
        get => (ObservableCollection<ItemViewModel>?)GetValue(ItemsSourceProperty);
        set => SetValue(ItemsSourceProperty, value);
    }

    /// <summary>
    /// Gets the search suggestions
    /// </summary>
    public ObservableCollection<SearchSuggestion> SearchSuggestions { get; } = new();

    /// <summary>
    /// Gets the active filters
    /// </summary>
    public ObservableCollection<SearchFilter> ActiveFilters { get; } = new();

    /// <summary>
    /// Gets the saved searches
    /// </summary>
    public ObservableCollection<SavedSearch> SavedSearches { get; } = new();

    /// <summary>
    /// Gets the available item types
    /// </summary>
    public ObservableCollection<ItemType> AvailableItemTypes { get; } = new();

    private bool _showAllTypes = true;
    /// <summary>
    /// Gets or sets whether to show all types
    /// </summary>
    public bool ShowAllTypes
    {
        get => _showAllTypes;
        set => SetProperty(ref _showAllTypes, value);
    }

    private bool _showGroundItems = true;
    /// <summary>
    /// Gets or sets whether to show ground items
    /// </summary>
    public bool ShowGroundItems
    {
        get => _showGroundItems;
        set => SetProperty(ref _showGroundItems, value);
    }

    private bool _showContainerItems = true;
    /// <summary>
    /// Gets or sets whether to show container items
    /// </summary>
    public bool ShowContainerItems
    {
        get => _showContainerItems;
        set => SetProperty(ref _showContainerItems, value);
    }

    private bool _showWeaponItems = true;
    /// <summary>
    /// Gets or sets whether to show weapon items
    /// </summary>
    public bool ShowWeaponItems
    {
        get => _showWeaponItems;
        set => SetProperty(ref _showWeaponItems, value);
    }

    private bool _showArmorItems = true;
    /// <summary>
    /// Gets or sets whether to show armor items
    /// </summary>
    public bool ShowArmorItems
    {
        get => _showArmorItems;
        set => SetProperty(ref _showArmorItems, value);
    }

    private bool _showModifiedOnly;
    /// <summary>
    /// Gets or sets whether to show only modified items
    /// </summary>
    public bool ShowModifiedOnly
    {
        get => _showModifiedOnly;
        set => SetProperty(ref _showModifiedOnly, value);
    }

    private bool _isAdvancedSearchExpanded;
    /// <summary>
    /// Gets or sets whether the advanced search is expanded
    /// </summary>
    public bool IsAdvancedSearchExpanded
    {
        get => _isAdvancedSearchExpanded;
        set => SetProperty(ref _isAdvancedSearchExpanded, value);
    }

    private ushort? _minId;
    /// <summary>
    /// Gets or sets the minimum ID filter
    /// </summary>
    public ushort? MinId
    {
        get => _minId;
        set => SetProperty(ref _minId, value);
    }

    private ushort? _maxId;
    /// <summary>
    /// Gets or sets the maximum ID filter
    /// </summary>
    public ushort? MaxId
    {
        get => _maxId;
        set => SetProperty(ref _maxId, value);
    }

    private ItemType? _selectedItemType;
    /// <summary>
    /// Gets or sets the selected item type filter
    /// </summary>
    public ItemType? SelectedItemType
    {
        get => _selectedItemType;
        set => SetProperty(ref _selectedItemType, value);
    }

    private bool? _filterStackable;
    /// <summary>
    /// Gets or sets the stackable filter
    /// </summary>
    public bool? FilterStackable
    {
        get => _filterStackable;
        set => SetProperty(ref _filterStackable, value);
    }

    private bool? _filterMoveable;
    /// <summary>
    /// Gets or sets the moveable filter
    /// </summary>
    public bool? FilterMoveable
    {
        get => _filterMoveable;
        set => SetProperty(ref _filterMoveable, value);
    }

    private bool? _filterPickupable;
    /// <summary>
    /// Gets or sets the pickupable filter
    /// </summary>
    public bool? FilterPickupable
    {
        get => _filterPickupable;
        set => SetProperty(ref _filterPickupable, value);
    }

    private bool? _filterHasLight;
    /// <summary>
    /// Gets or sets the has light filter
    /// </summary>
    public bool? FilterHasLight
    {
        get => _filterHasLight;
        set => SetProperty(ref _filterHasLight, value);
    }

    private bool? _filterModified;
    /// <summary>
    /// Gets or sets the modified filter
    /// </summary>
    public bool? FilterModified
    {
        get => _filterModified;
        set => SetProperty(ref _filterModified, value);
    }

    private bool? _filterHasErrors;
    /// <summary>
    /// Gets or sets the has errors filter
    /// </summary>
    public bool? FilterHasErrors
    {
        get => _filterHasErrors;
        set => SetProperty(ref _filterHasErrors, value);
    }

    private string _customPropertyKey = string.Empty;
    /// <summary>
    /// Gets or sets the custom property key
    /// </summary>
    public string CustomPropertyKey
    {
        get => _customPropertyKey;
        set => SetProperty(ref _customPropertyKey, value);
    }

    private string _customPropertyValue = string.Empty;
    /// <summary>
    /// Gets or sets the custom property value
    /// </summary>
    public string CustomPropertyValue
    {
        get => _customPropertyValue;
        set => SetProperty(ref _customPropertyValue, value);
    }

    /// <summary>
    /// Gets whether there are active filters
    /// </summary>
    public bool HasActiveFilters => ActiveFilters.Count > 0;

    #endregion

    #region Commands

    /// <summary>
    /// Command to toggle all types
    /// </summary>
    public ICommand ToggleAllTypesCommand { get; }

    /// <summary>
    /// Command to toggle item type
    /// </summary>
    public ICommand ToggleItemTypeCommand { get; }

    /// <summary>
    /// Command to toggle modified only
    /// </summary>
    public ICommand ToggleModifiedOnlyCommand { get; }

    /// <summary>
    /// Command to remove a filter
    /// </summary>
    public ICommand RemoveFilterCommand { get; }

    /// <summary>
    /// Command to clear all filters
    /// </summary>
    public ICommand ClearAllFiltersCommand { get; }

    /// <summary>
    /// Command to save search
    /// </summary>
    public ICommand SaveSearchCommand { get; }

    /// <summary>
    /// Command to load saved search
    /// </summary>
    public ICommand LoadSavedSearchCommand { get; }

    /// <summary>
    /// Command to delete saved search
    /// </summary>
    public ICommand DeleteSavedSearchCommand { get; }

    /// <summary>
    /// Command to add custom property filter
    /// </summary>
    public ICommand AddCustomPropertyFilterCommand { get; }

    #endregion

    #region Events

    /// <summary>
    /// Event raised when search criteria changes
    /// </summary>
    public event EventHandler? SearchChanged;

    /// <summary>
    /// Event raised when filters change
    /// </summary>
    public event EventHandler? FiltersChanged;

    /// <summary>
    /// Property changed event
    /// </summary>
    public event PropertyChangedEventHandler? PropertyChanged;

    #endregion

    #region Constructor

    /// <summary>
    /// Initializes a new instance of the SearchBox class
    /// </summary>
    public SearchBox()
    {
        InitializeComponent();

        // Initialize commands
        ToggleAllTypesCommand = new RelayCommand(ToggleAllTypes);
        ToggleItemTypeCommand = new RelayCommand<ItemType>(ToggleItemType);
        ToggleModifiedOnlyCommand = new RelayCommand(ToggleModifiedOnly);
        RemoveFilterCommand = new RelayCommand<SearchFilter>(RemoveFilter);
        ClearAllFiltersCommand = new RelayCommand(ClearAllFilters, CanClearAllFilters);
        SaveSearchCommand = new RelayCommand(SaveSearch, CanSaveSearch);
        LoadSavedSearchCommand = new RelayCommand<SavedSearch>(LoadSavedSearch);
        DeleteSavedSearchCommand = new RelayCommand<SavedSearch>(DeleteSavedSearch);
        AddCustomPropertyFilterCommand = new RelayCommand(AddCustomPropertyFilter, CanAddCustomPropertyFilter);

        // Initialize available item types
        InitializeItemTypes();

        // Load saved searches
        LoadSavedSearches();

        // Set data context
        DataContext = this;
    }

    #endregion

    #region Event Handlers

    private static void OnSearchTextChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        if (d is SearchBox searchBox)
        {
            searchBox.OnSearchTextChanged();
        }
    }

    private static void OnItemsSourceChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        if (d is SearchBox searchBox)
        {
            searchBox.OnItemsSourceChanged();
        }
    }

    private void OnSearchTextChanged()
    {
        UpdateSearchSuggestions();
        SearchChanged?.Invoke(this, EventArgs.Empty);
    }

    private void OnItemsSourceChanged()
    {
        UpdateSearchSuggestions();
    }

    private void OnQuerySubmitted(object sender, EventArgs e)
    {
        if (!string.IsNullOrWhiteSpace(SearchText))
        {
            AddTextFilter(SearchText);
        }
    }

    private void OnTextChanged(object sender, EventArgs e)
    {
        UpdateSearchSuggestions();
    }

    private void OnSuggestionChosen(object sender, EventArgs e)
    {
        // Handle suggestion selection
        if (MainSearchBox.SelectedItem is SearchSuggestion suggestion)
        {
            SearchText = suggestion.Text;
            AddTextFilter(suggestion.Text);
        }
    }

    #endregion

    #region Command Implementations

    private void ToggleAllTypes()
    {
        ShowAllTypes = !ShowAllTypes;
        
        if (ShowAllTypes)
        {
            ShowGroundItems = true;
            ShowContainerItems = true;
            ShowWeaponItems = true;
            ShowArmorItems = true;
        }
        
        UpdateTypeFilters();
    }

    private void ToggleItemType(ItemType? itemType)
    {
        if (itemType == null) return;

        switch (itemType)
        {
            case ItemType.Ground:
                ShowGroundItems = !ShowGroundItems;
                break;
            case ItemType.Container:
                ShowContainerItems = !ShowContainerItems;
                break;
            case ItemType.Weapon:
                ShowWeaponItems = !ShowWeaponItems;
                break;
            case ItemType.Armor:
                ShowArmorItems = !ShowArmorItems;
                break;
        }

        UpdateTypeFilters();
    }

    private void ToggleModifiedOnly()
    {
        ShowModifiedOnly = !ShowModifiedOnly;
        UpdateModifiedFilter();
    }

    private void RemoveFilter(SearchFilter? filter)
    {
        if (filter != null && ActiveFilters.Contains(filter))
        {
            ActiveFilters.Remove(filter);
            OnPropertyChanged(nameof(HasActiveFilters));
            FiltersChanged?.Invoke(this, EventArgs.Empty);
        }
    }

    private void ClearAllFilters()
    {
        ActiveFilters.Clear();
        SearchText = string.Empty;
        
        // Reset all filter properties
        ShowAllTypes = true;
        ShowGroundItems = true;
        ShowContainerItems = true;
        ShowWeaponItems = true;
        ShowArmorItems = true;
        ShowModifiedOnly = false;
        
        MinId = null;
        MaxId = null;
        SelectedItemType = null;
        FilterStackable = null;
        FilterMoveable = null;
        FilterPickupable = null;
        FilterHasLight = null;
        FilterModified = null;
        FilterHasErrors = null;
        
        OnPropertyChanged(nameof(HasActiveFilters));
        FiltersChanged?.Invoke(this, EventArgs.Empty);
    }

    private bool CanClearAllFilters()
    {
        return HasActiveFilters || !string.IsNullOrWhiteSpace(SearchText);
    }

    private void SaveSearch()
    {
        var dialog = new SaveSearchDialog
        {
            Owner = Window.GetWindow(this)
        };

        if (dialog.ShowDialog() == true)
        {
            var savedSearch = new SavedSearch
            {
                Name = dialog.SearchName,
                Description = dialog.SearchDescription,
                SearchText = SearchText,
                Filters = new List<SearchFilter>(ActiveFilters),
                CreatedDate = DateTime.Now
            };

            SavedSearches.Add(savedSearch);
            SaveSearchesToStorage();
        }
    }

    private bool CanSaveSearch()
    {
        return !string.IsNullOrWhiteSpace(SearchText) || HasActiveFilters;
    }

    private void LoadSavedSearch(SavedSearch? savedSearch)
    {
        if (savedSearch == null) return;

        ClearAllFilters();
        SearchText = savedSearch.SearchText;
        
        foreach (var filter in savedSearch.Filters)
        {
            ActiveFilters.Add(filter);
        }

        OnPropertyChanged(nameof(HasActiveFilters));
        FiltersChanged?.Invoke(this, EventArgs.Empty);
    }

    private void DeleteSavedSearch(SavedSearch? savedSearch)
    {
        if (savedSearch != null && SavedSearches.Contains(savedSearch))
        {
            SavedSearches.Remove(savedSearch);
            SaveSearchesToStorage();
        }
    }

    private void AddCustomPropertyFilter()
    {
        if (string.IsNullOrWhiteSpace(CustomPropertyKey)) return;

        var filter = new SearchFilter
        {
            Type = SearchFilterType.CustomProperty,
            Property = CustomPropertyKey,
            Value = CustomPropertyValue,
            DisplayText = $"{CustomPropertyKey}: {CustomPropertyValue}"
        };

        ActiveFilters.Add(filter);
        
        CustomPropertyKey = string.Empty;
        CustomPropertyValue = string.Empty;
        
        OnPropertyChanged(nameof(HasActiveFilters));
        FiltersChanged?.Invoke(this, EventArgs.Empty);
    }

    private bool CanAddCustomPropertyFilter()
    {
        return !string.IsNullOrWhiteSpace(CustomPropertyKey);
    }

    #endregion

    #region Private Methods

    private void InitializeItemTypes()
    {
        AvailableItemTypes.Clear();
        foreach (ItemType itemType in Enum.GetValues<ItemType>())
        {
            AvailableItemTypes.Add(itemType);
        }
    }

    private void UpdateSearchSuggestions()
    {
        SearchSuggestions.Clear();

        if (string.IsNullOrWhiteSpace(SearchText) || ItemsSource == null)
            return;

        var searchLower = SearchText.ToLowerInvariant();
        var suggestions = new HashSet<string>();

        // Add matching item names
        foreach (var item in ItemsSource.Take(10))
        {
            if (item.Name.ToLowerInvariant().Contains(searchLower))
            {
                suggestions.Add(item.Name);
            }
        }

        // Add matching IDs
        if (ushort.TryParse(SearchText, out _))
        {
            foreach (var item in ItemsSource.Where(i => i.Id.ToString().Contains(SearchText)).Take(5))
            {
                suggestions.Add(item.Id.ToString());
            }
        }

        // Add matching types
        foreach (ItemType itemType in Enum.GetValues<ItemType>())
        {
            if (itemType.ToString().ToLowerInvariant().Contains(searchLower))
            {
                suggestions.Add(itemType.ToString());
            }
        }

        // Convert to suggestion objects
        foreach (var suggestion in suggestions.Take(10))
        {
            SearchSuggestions.Add(new SearchSuggestion
            {
                Text = suggestion,
                Category = DetermineSuggestionCategory(suggestion)
            });
        }
    }

    private string DetermineSuggestionCategory(string suggestion)
    {
        if (ushort.TryParse(suggestion, out _))
            return "ID";
        
        if (Enum.TryParse<ItemType>(suggestion, out _))
            return "Type";
        
        return "Name";
    }

    private void AddTextFilter(string text)
    {
        if (string.IsNullOrWhiteSpace(text)) return;

        var filter = new SearchFilter
        {
            Type = SearchFilterType.Text,
            Value = text,
            DisplayText = $"Text: {text}"
        };

        // Don't add duplicate filters
        if (!ActiveFilters.Any(f => f.Type == SearchFilterType.Text && f.Value == text))
        {
            ActiveFilters.Add(filter);
            OnPropertyChanged(nameof(HasActiveFilters));
            FiltersChanged?.Invoke(this, EventArgs.Empty);
        }
    }

    private void UpdateTypeFilters()
    {
        // Remove existing type filters
        var typeFilters = ActiveFilters.Where(f => f.Type == SearchFilterType.ItemType).ToList();
        foreach (var filter in typeFilters)
        {
            ActiveFilters.Remove(filter);
        }

        // Add active type filters
        if (!ShowGroundItems)
            ActiveFilters.Add(new SearchFilter { Type = SearchFilterType.ItemType, Value = ItemType.Ground.ToString(), DisplayText = "Exclude: Ground" });
        
        if (!ShowContainerItems)
            ActiveFilters.Add(new SearchFilter { Type = SearchFilterType.ItemType, Value = ItemType.Container.ToString(), DisplayText = "Exclude: Container" });
        
        if (!ShowWeaponItems)
            ActiveFilters.Add(new SearchFilter { Type = SearchFilterType.ItemType, Value = ItemType.Weapon.ToString(), DisplayText = "Exclude: Weapon" });
        
        if (!ShowArmorItems)
            ActiveFilters.Add(new SearchFilter { Type = SearchFilterType.ItemType, Value = ItemType.Armor.ToString(), DisplayText = "Exclude: Armor" });

        OnPropertyChanged(nameof(HasActiveFilters));
        FiltersChanged?.Invoke(this, EventArgs.Empty);
    }

    private void UpdateModifiedFilter()
    {
        // Remove existing modified filter
        var modifiedFilter = ActiveFilters.FirstOrDefault(f => f.Type == SearchFilterType.Modified);
        if (modifiedFilter != null)
        {
            ActiveFilters.Remove(modifiedFilter);
        }

        // Add modified filter if enabled
        if (ShowModifiedOnly)
        {
            ActiveFilters.Add(new SearchFilter
            {
                Type = SearchFilterType.Modified,
                Value = "true",
                DisplayText = "Modified Only"
            });
        }

        OnPropertyChanged(nameof(HasActiveFilters));
        FiltersChanged?.Invoke(this, EventArgs.Empty);
    }

    private void LoadSavedSearches()
    {
        // In a real implementation, this would load from user settings or a file
        // For now, we'll just initialize with empty collection
        SavedSearches.Clear();
    }

    private void SaveSearchesToStorage()
    {
        // In a real implementation, this would save to user settings or a file
        // For now, this is a placeholder
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
    /// Applies the current filters to an item
    /// </summary>
    /// <param name="item">Item to check</param>
    /// <returns>True if the item matches the filters</returns>
    public bool MatchesFilters(ItemViewModel item)
    {
        // Text search
        if (!string.IsNullOrWhiteSpace(SearchText))
        {
            var searchLower = SearchText.ToLowerInvariant();
            if (!item.Name.ToLowerInvariant().Contains(searchLower) &&
                !item.Description.ToLowerInvariant().Contains(searchLower) &&
                !item.Id.ToString().Contains(SearchText) &&
                !item.TypeDisplayName.ToLowerInvariant().Contains(searchLower))
            {
                return false;
            }
        }

        // Type filters
        if (!ShowAllTypes)
        {
            if (item.Type == ItemType.Ground && !ShowGroundItems) return false;
            if (item.Type == ItemType.Container && !ShowContainerItems) return false;
            if (item.Type == ItemType.Weapon && !ShowWeaponItems) return false;
            if (item.Type == ItemType.Armor && !ShowArmorItems) return false;
        }

        // Modified filter
        if (ShowModifiedOnly && !item.IsDirty) return false;

        // Advanced filters
        if (MinId.HasValue && item.Id < MinId.Value) return false;
        if (MaxId.HasValue && item.Id > MaxId.Value) return false;
        if (SelectedItemType.HasValue && item.Type != SelectedItemType.Value) return false;

        // Property filters
        if (FilterStackable.HasValue && item.IsStackable != FilterStackable.Value) return false;
        if (FilterMoveable.HasValue && item.IsMoveable != FilterMoveable.Value) return false;
        if (FilterPickupable.HasValue && item.IsPickupable != FilterPickupable.Value) return false;
        if (FilterHasLight.HasValue && item.HasLight != FilterHasLight.Value) return false;
        if (FilterModified.HasValue && item.IsDirty != FilterModified.Value) return false;
        if (FilterHasErrors.HasValue && item.HasErrors != FilterHasErrors.Value) return false;

        return true;
    }

    /// <summary>
    /// Clears the search text
    /// </summary>
    public void ClearSearch()
    {
        SearchText = string.Empty;
    }

    /// <summary>
    /// Focuses the search box
    /// </summary>
    public void FocusSearch()
    {
        MainSearchBox.Focus();
    }

    #endregion
}

#region Supporting Classes

/// <summary>
/// Search suggestion item
/// </summary>
public class SearchSuggestion
{
    public string Text { get; set; } = string.Empty;
    public string Category { get; set; } = string.Empty;
}

/// <summary>
/// Search filter item
/// </summary>
public class SearchFilter
{
    public SearchFilterType Type { get; set; }
    public string Property { get; set; } = string.Empty;
    public string Value { get; set; } = string.Empty;
    public string DisplayText { get; set; } = string.Empty;
}

/// <summary>
/// Search filter types
/// </summary>
public enum SearchFilterType
{
    Text,
    ItemType,
    Modified,
    Property,
    CustomProperty
}

/// <summary>
/// Saved search item
/// </summary>
public class SavedSearch
{
    public string Name { get; set; } = string.Empty;
    public string Description { get; set; } = string.Empty;
    public string SearchText { get; set; } = string.Empty;
    public List<SearchFilter> Filters { get; set; } = new();
    public DateTime CreatedDate { get; set; }
}

#endregion

/// <summary>
/// Simple dialog for saving searches (placeholder)
/// </summary>
public class SaveSearchDialog : Window
{
    public string SearchName { get; set; } = string.Empty;
    public string SearchDescription { get; set; } = string.Empty;
}