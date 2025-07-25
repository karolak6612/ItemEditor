using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows.Input;
using ItemEditor.Services;
using Microsoft.Extensions.Logging;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;

namespace ItemEditor.ViewModels;

/// <summary>
/// View model for the help window
/// </summary>
public partial class HelpViewModel : ObservableObject
{
    private readonly IHelpService _helpService;
    private readonly ILogger<HelpViewModel> _logger;
    private readonly Stack<string> _navigationHistory = new();
    private readonly Stack<string> _forwardHistory = new();
    
    [ObservableProperty]
    private string _searchQuery = string.Empty;
    
    [ObservableProperty]
    private HelpContent? _currentContent;
    
    [ObservableProperty]
    private ObservableCollection<HelpSearchResult> _searchResults = new();
    
    [ObservableProperty]
    private ObservableCollection<HelpNavigationItem> _helpCategories = new();
    
    [ObservableProperty]
    private bool _isSearching;
    
    [ObservableProperty]
    private string _statusText = "Ready";
    
    [ObservableProperty]
    private bool _hasContent;
    
    public HelpViewModel(IHelpService helpService, ILogger<HelpViewModel> logger)
    {
        _helpService = helpService ?? throw new ArgumentNullException(nameof(helpService));
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
        
        // Initialize commands
        BackCommand = new RelayCommand(GoBack, CanGoBack);
        ForwardCommand = new RelayCommand(GoForward, CanGoForward);
        HomeCommand = new RelayCommand(GoHome);
        OpenLinkCommand = new RelayCommand<HelpLink>(OpenLink);
        StartTourCommand = new RelayCommand(StartTour);
        CloseCommand = new RelayCommand(Close);
        
        // Subscribe to property changes
        PropertyChanged += OnPropertyChanged;
        
        _logger.LogDebug("HelpViewModel initialized");
    }
    
    #region Commands
    
    public ICommand BackCommand { get; }
    public ICommand ForwardCommand { get; }
    public ICommand HomeCommand { get; }
    public ICommand OpenLinkCommand { get; }
    public ICommand StartTourCommand { get; }
    public ICommand CloseCommand { get; }
    
    #endregion
    
    #region Public Methods
    
    /// <summary>
    /// Initializes the help system
    /// </summary>
    public async Task InitializeAsync()
    {
        try
        {
            _logger.LogDebug("Initializing help system");
            
            // Build navigation tree
            BuildNavigationTree();
            
            // Load default content
            NavigateToContent("main");
            
            StatusText = "Help system ready";
            _logger.LogInformation("Help system initialized successfully");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error initializing help system");
            StatusText = "Error loading help content";
        }
    }
    
    /// <summary>
    /// Navigates to specific help content
    /// </summary>
    /// <param name="context">Help context</param>
    public void NavigateToContent(string context)
    {
        try
        {
            if (string.IsNullOrWhiteSpace(context))
                return;
            
            var content = _helpService.GetHelpContent(context);
            if (content == null)
            {
                _logger.LogWarning("Help content not found for context: {Context}", context);
                StatusText = $"Help content not found: {context}";
                return;
            }
            
            // Add to navigation history
            if (CurrentContent != null)
            {
                _navigationHistory.Push(GetCurrentContext());
                _forwardHistory.Clear(); // Clear forward history when navigating to new content
            }
            
            CurrentContent = content;
            HasContent = true;
            IsSearching = false;
            SearchResults.Clear();
            
            StatusText = $"Viewing: {content.Title}";
            
            // Update command states
            ((RelayCommand)BackCommand).NotifyCanExecuteChanged();
            ((RelayCommand)ForwardCommand).NotifyCanExecuteChanged();
            
            _logger.LogDebug("Navigated to help content: {Context}", context);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error navigating to help content: {Context}", context);
            StatusText = "Error loading help content";
        }
    }
    
    /// <summary>
    /// Performs help search
    /// </summary>
    /// <param name="query">Search query</param>
    public async Task SearchAsync(string query)
    {
        try
        {
            if (string.IsNullOrWhiteSpace(query))
            {
                IsSearching = false;
                SearchResults.Clear();
                StatusText = "Ready";
                return;
            }
            
            _logger.LogDebug("Searching help content: {Query}", query);
            StatusText = "Searching...";
            
            var results = await _helpService.SearchHelpAsync(query);
            
            SearchResults.Clear();
            foreach (var result in results)
            {
                SearchResults.Add(result);
            }
            
            IsSearching = true;
            HasContent = SearchResults.Count > 0;
            StatusText = $"Found {SearchResults.Count} results for '{query}'";
            
            _logger.LogDebug("Search completed: {ResultCount} results", SearchResults.Count);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error searching help content: {Query}", query);
            StatusText = "Search error occurred";
        }
    }
    
    /// <summary>
    /// Cleans up resources
    /// </summary>
    public void Cleanup()
    {
        try
        {
            PropertyChanged -= OnPropertyChanged;
            _logger.LogDebug("HelpViewModel cleaned up");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error cleaning up HelpViewModel");
        }
    }
    
    #endregion
    
    #region Private Methods
    
    private void OnPropertyChanged(object? sender, PropertyChangedEventArgs e)
    {
        if (e.PropertyName == nameof(SearchQuery))
        {
            // Debounce search
            Task.Delay(300).ContinueWith(async _ =>
            {
                if (SearchQuery == _searchQuery) // Check if query hasn't changed
                {
                    await SearchAsync(SearchQuery);
                }
            });
        }
    }
    
    private void BuildNavigationTree()
    {
        try
        {
            HelpCategories.Clear();
            
            // Getting Started
            var gettingStarted = new HelpNavigationItem
            {
                Title = "Getting Started",
                Icon = "Play16",
                Context = "main"
            };
            gettingStarted.Children.Add(new HelpNavigationItem
            {
                Title = "Overview",
                Icon = "Info16",
                Context = "main"
            });
            gettingStarted.Children.Add(new HelpNavigationItem
            {
                Title = "First Steps",
                Icon = "ArrowRight16",
                Context = "first-steps"
            });
            
            // File Operations
            var fileOps = new HelpNavigationItem
            {
                Title = "File Operations",
                Icon = "Folder16",
                Context = "file-operations"
            };
            fileOps.Children.Add(new HelpNavigationItem
            {
                Title = "Opening Files",
                Icon = "FolderOpen16",
                Context = "file-open"
            });
            fileOps.Children.Add(new HelpNavigationItem
            {
                Title = "Saving Files",
                Icon = "Save16",
                Context = "file-save"
            });
            
            // Item Editing
            var itemEditing = new HelpNavigationItem
            {
                Title = "Item Editing",
                Icon = "Edit16",
                Context = "item-editing"
            };
            itemEditing.Children.Add(new HelpNavigationItem
            {
                Title = "Properties",
                Icon = "Settings16",
                Context = "item-properties"
            });
            itemEditing.Children.Add(new HelpNavigationItem
            {
                Title = "Sprites",
                Icon = "Image16",
                Context = "item-sprites"
            });
            
            // Reference
            var reference = new HelpNavigationItem
            {
                Title = "Reference",
                Icon = "Book16",
                Context = "reference"
            };
            reference.Children.Add(new HelpNavigationItem
            {
                Title = "Keyboard Shortcuts",
                Icon = "Keyboard16",
                Context = "keyboard-shortcuts"
            });
            reference.Children.Add(new HelpNavigationItem
            {
                Title = "File Formats",
                Icon = "Document16",
                Context = "file-formats"
            });
            
            HelpCategories.Add(gettingStarted);
            HelpCategories.Add(fileOps);
            HelpCategories.Add(itemEditing);
            HelpCategories.Add(reference);
            
            _logger.LogDebug("Built navigation tree with {CategoryCount} categories", HelpCategories.Count);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error building navigation tree");
        }
    }
    
    private string GetCurrentContext()
    {
        // This would need to be implemented to track the current context
        // For now, return a default
        return "main";
    }
    
    #endregion
    
    #region Command Implementations
    
    private void GoBack()
    {
        try
        {
            if (_navigationHistory.Count > 0)
            {
                var previousContext = _navigationHistory.Pop();
                if (CurrentContent != null)
                {
                    _forwardHistory.Push(GetCurrentContext());
                }
                NavigateToContent(previousContext);
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error going back in navigation");
        }
    }
    
    private bool CanGoBack()
    {
        return _navigationHistory.Count > 0;
    }
    
    private void GoForward()
    {
        try
        {
            if (_forwardHistory.Count > 0)
            {
                var nextContext = _forwardHistory.Pop();
                if (CurrentContent != null)
                {
                    _navigationHistory.Push(GetCurrentContext());
                }
                NavigateToContent(nextContext);
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error going forward in navigation");
        }
    }
    
    private bool CanGoForward()
    {
        return _forwardHistory.Count > 0;
    }
    
    private void GoHome()
    {
        try
        {
            NavigateToContent("main");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error navigating to home");
        }
    }
    
    private void OpenLink(HelpLink? link)
    {
        try
        {
            if (link == null)
                return;
            
            switch (link.Type)
            {
                case HelpLinkType.Context:
                    NavigateToContent(link.Url);
                    break;
                    
                case HelpLinkType.External:
                    System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo
                    {
                        FileName = link.Url,
                        UseShellExecute = true
                    });
                    break;
                    
                case HelpLinkType.Internal:
                    // Handle internal navigation
                    break;
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error opening link: {Url}", link?.Url);
        }
    }
    
    private async void StartTour()
    {
        try
        {
            await _helpService.StartOnboardingTourAsync("first-time");
            StatusText = "Starting onboarding tour...";
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error starting onboarding tour");
        }
    }
    
    private void Close()
    {
        try
        {
            // Close the window
            System.Windows.Application.Current.Windows
                .OfType<Dialogs.HelpWindow>()
                .FirstOrDefault()?.Close();
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error closing help window");
        }
    }
    
    #endregion
}

/// <summary>
/// Navigation item for help tree
/// </summary>
public class HelpNavigationItem
{
    public string Title { get; set; } = string.Empty;
    public string Icon { get; set; } = "Document16";
    public string Context { get; set; } = string.Empty;
    public ObservableCollection<HelpNavigationItem> Children { get; set; } = new();
}