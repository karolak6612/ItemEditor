using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using ItemEditor.Models;
using System.Collections.ObjectModel;
using System.Text.Json;

namespace ItemEditor.ViewModels;

/// <summary>
/// ViewModel for the Find Item dialog
/// </summary>
public partial class FindItemViewModel : ObservableObject
{
    [ObservableProperty]
    private string _searchText = string.Empty;
    
    [ObservableProperty]
    private ushort? _searchId;
    
    [ObservableProperty]
    private string _searchName = string.Empty;
    
    [ObservableProperty]
    private ItemType? _searchType;
    
    [ObservableProperty]
    private bool _searchStackable;
    
    [ObservableProperty]
    private bool _useAdvancedSearch;
    
    [ObservableProperty]
    private bool _isSearching;
    
    [ObservableProperty]
    private ObservableCollection<ItemViewModel> _searchResults = new();
    
    [ObservableProperty]
    private ItemViewModel? _selectedResult;
    
    [ObservableProperty]
    private string _searchStatus = string.Empty;
    
    [ObservableProperty]
    private ObservableCollection<SearchHistoryItem> _searchHistory = new();
    
    [ObservableProperty]
    private ObservableCollection<SavedSearch> _savedSearches = new();
    
    [ObservableProperty]
    private SavedSearch? _selectedSavedSearch;
    
    [ObservableProperty]
    private string _newSavedSearchName = string.Empty;
    
    private readonly List<Item> _allItems = new();
    private readonly string _searchHistoryFile = "search_history.json";
    private readonly string _savedSearchesFile = "saved_searches.json";
    
    public FindItemViewModel()
    {
        // Initialize commands
        SearchCommand = new AsyncRelayCommand(SearchAsync, CanSearch);
        ClearSearchCommand = new RelayCommand(ClearSearch);
        SaveSearchCommand = new RelayCommand(SaveCurrentSearch, CanSaveSearch);
        LoadSavedSearchCommand = new RelayCommand(LoadSavedSearch, () => SelectedSavedSearch != null);
        DeleteSavedSearchCommand = new RelayCommand<SavedSearch>(DeleteSavedSearch);
        SelectResultCommand = new RelayCommand<ItemViewModel>(SelectResult);
        
        // Load saved data
        LoadSearchHistory();
        LoadSavedSearches();
        
        // Watch for property changes to update command states
        PropertyChanged += (s, e) =>
        {
            if (e.PropertyName == nameof(SearchText) || e.PropertyName == nameof(SearchId) || 
                e.PropertyName == nameof(SearchName) || e.PropertyName == nameof(SearchType))
            {
                SearchCommand.NotifyCanExecuteChanged();
            }
            
            if (e.PropertyName == nameof(NewSavedSearchName))
            {
                SaveSearchCommand.NotifyCanExecuteChanged();
            }
            
            if (e.PropertyName == nameof(SelectedSavedSearch))
            {
                LoadSavedSearchCommand.NotifyCanExecuteChanged();
            }
        };
    }
    
    public IAsyncRelayCommand SearchCommand { get; }
    public IRelayCommand ClearSearchCommand { get; }
    public IRelayCommand SaveSearchCommand { get; }
    public IRelayCommand LoadSavedSearchCommand { get; }
    public IRelayCommand<SavedSearch> DeleteSavedSearchCommand { get; }
    public IRelayCommand<ItemViewModel> SelectResultCommand { get; }
    
    /// <summary>
    /// Gets the available item types for filtering
    /// </summary>
    public Array ItemTypes => Enum.GetValues(typeof(ItemType));
    
    /// <summary>
    /// Sets the items to search through
    /// </summary>
    /// <param name="items">Collection of items to search</param>
    public void SetItems(IEnumerable<Item> items)
    {
        _allItems.Clear();
        _allItems.AddRange(items);
    }
    
    private bool CanSearch()
    {
        if (UseAdvancedSearch)
        {
            return SearchId.HasValue || !string.IsNullOrWhiteSpace(SearchName) || SearchType.HasValue;
        }
        
        return !string.IsNullOrWhiteSpace(SearchText);
    }
    
    private bool CanSaveSearch()
    {
        return !string.IsNullOrWhiteSpace(NewSavedSearchName) && CanSearch();
    }
    
    private async Task SearchAsync()
    {
        try
        {
            IsSearching = true;
            SearchResults.Clear();
            
            var results = await Task.Run(() => PerformSearch());
            
            foreach (var item in results)
            {
                SearchResults.Add(new ItemViewModel(item, null!)); // ImageService would be injected in real implementation
            }
            
            SearchStatus = $"Found {SearchResults.Count} items";
            
            // Add to search history
            AddToSearchHistory();
        }
        catch (Exception ex)
        {
            SearchStatus = $"Search failed: {ex.Message}";
        }
        finally
        {
            IsSearching = false;
        }
    }
    
    private List<Item> PerformSearch()
    {
        var query = _allItems.AsEnumerable();
        
        if (UseAdvancedSearch)
        {
            // Advanced search with multiple criteria
            if (SearchId.HasValue)
            {
                query = query.Where(i => i.Id == SearchId.Value);
            }
            
            if (!string.IsNullOrWhiteSpace(SearchName))
            {
                query = query.Where(i => i.Name.Contains(SearchName, StringComparison.OrdinalIgnoreCase));
            }
            
            if (SearchType.HasValue)
            {
                query = query.Where(i => i.Type == SearchType.Value);
            }
            
            if (SearchStackable)
            {
                query = query.Where(i => i.IsStackable);
            }
        }
        else
        {
            // Simple text search across multiple fields
            var searchLower = SearchText.ToLowerInvariant();
            
            query = query.Where(i =>
                i.Name.Contains(searchLower, StringComparison.OrdinalIgnoreCase) ||
                i.Id.ToString().Contains(searchLower) ||
                i.Type.ToString().Contains(searchLower, StringComparison.OrdinalIgnoreCase));
        }
        
        return query.OrderBy(i => i.Id).ToList();
    }
    
    private void ClearSearch()
    {
        SearchText = string.Empty;
        SearchId = null;
        SearchName = string.Empty;
        SearchType = null;
        SearchStackable = false;
        SearchResults.Clear();
        SelectedResult = null;
        SearchStatus = string.Empty;
    }
    
    private void SaveCurrentSearch()
    {
        var savedSearch = new SavedSearch
        {
            Name = NewSavedSearchName,
            UseAdvancedSearch = UseAdvancedSearch,
            SearchText = SearchText,
            SearchId = SearchId,
            SearchName = SearchName,
            SearchType = SearchType,
            SearchStackable = SearchStackable,
            CreatedDate = DateTime.Now
        };
        
        SavedSearches.Add(savedSearch);
        SaveSavedSearches();
        
        NewSavedSearchName = string.Empty;
        SearchStatus = $"Search saved as '{savedSearch.Name}'";
    }
    
    private void LoadSavedSearch()
    {
        if (SelectedSavedSearch == null) return;
        
        UseAdvancedSearch = SelectedSavedSearch.UseAdvancedSearch;
        SearchText = SelectedSavedSearch.SearchText;
        SearchId = SelectedSavedSearch.SearchId;
        SearchName = SelectedSavedSearch.SearchName;
        SearchType = SelectedSavedSearch.SearchType;
        SearchStackable = SelectedSavedSearch.SearchStackable;
        
        SearchStatus = $"Loaded search '{SelectedSavedSearch.Name}'";
    }
    
    private void DeleteSavedSearch(SavedSearch? searchToDelete)
    {
        if (searchToDelete == null) return;
        
        var searchName = searchToDelete.Name;
        SavedSearches.Remove(searchToDelete);
        SaveSavedSearches();
        
        if (SelectedSavedSearch == searchToDelete)
        {
            SelectedSavedSearch = null;
        }
        
        SearchStatus = $"Deleted search '{searchName}'";
    }
    
    private void SelectResult(ItemViewModel? result)
    {
        SelectedResult = result;
    }
    
    private void AddToSearchHistory()
    {
        var historyItem = new SearchHistoryItem
        {
            SearchText = UseAdvancedSearch ? $"Advanced: {SearchName}" : SearchText,
            SearchDate = DateTime.Now,
            ResultCount = SearchResults.Count
        };
        
        // Remove duplicate if exists
        var existing = SearchHistory.FirstOrDefault(h => h.SearchText == historyItem.SearchText);
        if (existing != null)
        {
            SearchHistory.Remove(existing);
        }
        
        // Add to beginning and limit to 20 items
        SearchHistory.Insert(0, historyItem);
        while (SearchHistory.Count > 20)
        {
            SearchHistory.RemoveAt(SearchHistory.Count - 1);
        }
        
        SaveSearchHistory();
    }
    
    private void LoadSearchHistory()
    {
        try
        {
            if (File.Exists(_searchHistoryFile))
            {
                var json = File.ReadAllText(_searchHistoryFile);
                var history = JsonSerializer.Deserialize<List<SearchHistoryItem>>(json);
                if (history != null)
                {
                    SearchHistory.Clear();
                    foreach (var item in history)
                    {
                        SearchHistory.Add(item);
                    }
                }
            }
        }
        catch (Exception ex)
        {
            // Log error but don't throw
            System.Diagnostics.Debug.WriteLine($"Failed to load search history: {ex.Message}");
        }
    }
    
    private void SaveSearchHistory()
    {
        try
        {
            var json = JsonSerializer.Serialize(SearchHistory.ToList(), new JsonSerializerOptions { WriteIndented = true });
            File.WriteAllText(_searchHistoryFile, json);
        }
        catch (Exception ex)
        {
            // Log error but don't throw
            System.Diagnostics.Debug.WriteLine($"Failed to save search history: {ex.Message}");
        }
    }
    
    private void LoadSavedSearches()
    {
        try
        {
            if (File.Exists(_savedSearchesFile))
            {
                var json = File.ReadAllText(_savedSearchesFile);
                var searches = JsonSerializer.Deserialize<List<SavedSearch>>(json);
                if (searches != null)
                {
                    SavedSearches.Clear();
                    foreach (var search in searches)
                    {
                        SavedSearches.Add(search);
                    }
                }
            }
        }
        catch (Exception ex)
        {
            // Log error but don't throw
            System.Diagnostics.Debug.WriteLine($"Failed to load saved searches: {ex.Message}");
        }
    }
    
    private void SaveSavedSearches()
    {
        try
        {
            var json = JsonSerializer.Serialize(SavedSearches.ToList(), new JsonSerializerOptions { WriteIndented = true });
            File.WriteAllText(_savedSearchesFile, json);
        }
        catch (Exception ex)
        {
            // Log error but don't throw
            System.Diagnostics.Debug.WriteLine($"Failed to save searches: {ex.Message}");
        }
    }
}

/// <summary>
/// Represents a search history item
/// </summary>
public class SearchHistoryItem
{
    public string SearchText { get; set; } = string.Empty;
    public DateTime SearchDate { get; set; }
    public int ResultCount { get; set; }
}

/// <summary>
/// Represents a saved search
/// </summary>
public class SavedSearch
{
    public string Name { get; set; } = string.Empty;
    public bool UseAdvancedSearch { get; set; }
    public string SearchText { get; set; } = string.Empty;
    public ushort? SearchId { get; set; }
    public string SearchName { get; set; } = string.Empty;
    public ItemType? SearchType { get; set; }
    public bool SearchStackable { get; set; }
    public DateTime CreatedDate { get; set; }
}