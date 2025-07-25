using System.Text.Json;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Media;
using Microsoft.Extensions.Logging;

namespace ItemEditor.Services;

/// <summary>
/// Service for context-sensitive help and guidance system
/// </summary>
public class HelpService : IHelpService
{
    private readonly ILogger<HelpService> _logger;
    private readonly Dictionary<string, HelpContent> _helpContent = new();
    private readonly Dictionary<string, List<HelpStep>> _onboardingTours = new();
    private ToolTip? _currentTooltip;
    private Popup? _contextualHelpPopup;
    private string? _currentTourId;
    private int _currentTourStep;
    private readonly string _helpContentPath;
    
    public HelpService(ILogger<HelpService> logger)
    {
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
        
        // Set up help content path
        var appDataPath = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
        var appFolder = Path.Combine(appDataPath, "ItemEditor");
        Directory.CreateDirectory(appFolder);
        _helpContentPath = Path.Combine(appFolder, "help");
        Directory.CreateDirectory(_helpContentPath);
        
        // Initialize default help content
        InitializeDefaultHelpContent();
        
        _logger.LogDebug("HelpService initialized");
    }
    
    /// <inheritdoc />
    public async Task ShowHelpAsync(string context, FrameworkElement? element = null)
    {
        try
        {
            _logger.LogDebug("Showing help for context: {Context}", context);
            
            // Raise help requested event
            var eventArgs = new HelpRequestedEventArgs(context, element);
            HelpRequested?.Invoke(this, eventArgs);
            
            if (eventArgs.Handled)
                return;
            
            var content = GetHelpContent(context);
            if (content == null)
            {
                _logger.LogWarning("No help content found for context: {Context}", context);
                
                // Show generic help
                content = new HelpContent
                {
                    Title = "Help Not Available",
                    Description = $"No help content is available for '{context}' at this time.",
                    DetailedContent = "Please check the documentation or contact support for assistance.",
                    Type = HelpContentType.General
                };
            }
            
            // Show help window or panel
            await ShowHelpWindowAsync(content, element);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error showing help for context: {Context}", context);
        }
    }
    
    /// <inheritdoc />
    public void ShowTooltip(FrameworkElement element, HelpContent content, TooltipPlacement placement = TooltipPlacement.Auto)
    {
        try
        {
            if (element == null || content == null)
                return;
            
            // Hide existing tooltip
            HideTooltip();
            
            // Create rich tooltip
            var tooltipContent = CreateTooltipContent(content);
            
            _currentTooltip = new ToolTip
            {
                Content = tooltipContent,
                Placement = ConvertPlacement(placement),
                PlacementTarget = element,
                IsOpen = true,
                StaysOpen = false,
                MaxWidth = 400
            };
            
            // Apply styling
            _currentTooltip.Style = Application.Current.FindResource("ModernTooltipStyle") as Style;
            
            _logger.LogDebug("Showing tooltip for element: {ElementType}", element.GetType().Name);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error showing tooltip");
        }
    }
    
    /// <inheritdoc />
    public void HideTooltip()
    {
        try
        {
            if (_currentTooltip != null)
            {
                _currentTooltip.IsOpen = false;
                _currentTooltip = null;
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error hiding tooltip");
        }
    }
    
    /// <inheritdoc />
    public void ShowContextualHelp(string context, FrameworkElement? targetElement = null)
    {
        try
        {
            var content = GetHelpContent(context);
            if (content == null)
                return;
            
            // Hide existing contextual help
            HideContextualHelp();
            
            // Create contextual help panel
            var helpPanel = CreateContextualHelpPanel(content);
            
            _contextualHelpPopup = new Popup
            {
                Child = helpPanel,
                PlacementTarget = targetElement ?? Application.Current.MainWindow,
                Placement = PlacementMode.Right,
                IsOpen = true,
                StaysOpen = true,
                AllowsTransparency = true
            };
            
            _logger.LogDebug("Showing contextual help for context: {Context}", context);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error showing contextual help");
        }
    }
    
    /// <inheritdoc />
    public void HideContextualHelp()
    {
        try
        {
            if (_contextualHelpPopup != null)
            {
                _contextualHelpPopup.IsOpen = false;
                _contextualHelpPopup = null;
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error hiding contextual help");
        }
    }
    
    /// <inheritdoc />
    public async Task StartOnboardingTourAsync(string tourId)
    {
        try
        {
            if (!_onboardingTours.ContainsKey(tourId))
            {
                _logger.LogWarning("Onboarding tour not found: {TourId}", tourId);
                return;
            }
            
            _currentTourId = tourId;
            _currentTourStep = 0;
            
            _logger.LogInformation("Starting onboarding tour: {TourId}", tourId);
            
            await ShowNextTourStepAsync();
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error starting onboarding tour: {TourId}", tourId);
        }
    }
    
    /// <inheritdoc />
    public void StopOnboardingTour()
    {
        try
        {
            if (_currentTourId != null)
            {
                _logger.LogInformation("Stopping onboarding tour: {TourId}", _currentTourId);
                
                _currentTourId = null;
                _currentTourStep = 0;
                HideContextualHelp();
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error stopping onboarding tour");
        }
    }
    
    /// <inheritdoc />
    public void RegisterHelpContent(string context, HelpContent content)
    {
        try
        {
            if (string.IsNullOrWhiteSpace(context) || content == null)
                return;
            
            _helpContent[context] = content;
            _logger.LogDebug("Registered help content for context: {Context}", context);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error registering help content for context: {Context}", context);
        }
    }
    
    /// <inheritdoc />
    public HelpContent? GetHelpContent(string context)
    {
        return _helpContent.TryGetValue(context, out var content) ? content : null;
    }
    
    /// <inheritdoc />
    public async Task<IEnumerable<HelpSearchResult>> SearchHelpAsync(string query)
    {
        try
        {
            if (string.IsNullOrWhiteSpace(query))
                return Array.Empty<HelpSearchResult>();
            
            var results = new List<HelpSearchResult>();
            var queryLower = query.ToLowerInvariant();
            
            foreach (var kvp in _helpContent)
            {
                var content = kvp.Value;
                var relevance = CalculateRelevance(content, queryLower);
                
                if (relevance > 0)
                {
                    results.Add(new HelpSearchResult
                    {
                        Context = kvp.Key,
                        Content = content,
                        Relevance = relevance,
                        MatchedKeywords = GetMatchedKeywords(content, queryLower)
                    });
                }
            }
            
            return results.OrderByDescending(r => r.Relevance);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error searching help content");
            return Array.Empty<HelpSearchResult>();
        }
    }
    
    /// <inheritdoc />
    public IEnumerable<string> GetAvailableContexts()
    {
        return _helpContent.Keys;
    }
    
    /// <inheritdoc />
    public bool IsHelpAvailable(string context)
    {
        return _helpContent.ContainsKey(context);
    }
    
    /// <inheritdoc />
    public event EventHandler<HelpRequestedEventArgs>? HelpRequested;
    
    /// <inheritdoc />
    public event EventHandler<OnboardingStepEventArgs>? OnboardingStepChanged;
    
    #region Private Methods
    
    private void InitializeDefaultHelpContent()
    {
        try
        {
            // Main application help
            RegisterHelpContent("main", new HelpContent
            {
                Title = "ItemEditor Overview",
                Description = "Learn how to use the ItemEditor application",
                DetailedContent = "ItemEditor is a powerful tool for editing OpenTibia items. Use the file menu to open .otb, .dat, and .spr files, then edit item properties in the properties panel.",
                Type = HelpContentType.General,
                Category = "Getting Started",
                Keywords = new List<string> { "overview", "introduction", "getting started", "basics" }
            });
            
            // File operations help
            RegisterHelpContent("file-operations", new HelpContent
            {
                Title = "File Operations",
                Description = "How to open, save, and manage files",
                DetailedContent = "Use Ctrl+O to open files, Ctrl+S to save, and Ctrl+N to create new files. Supported formats include .otb, .dat, and .spr files.",
                Type = HelpContentType.Reference,
                Category = "File Management",
                Keywords = new List<string> { "file", "open", "save", "new", "otb", "dat", "spr" },
                Steps = new List<HelpStep>
                {
                    new HelpStep
                    {
                        StepNumber = 1,
                        Title = "Opening Files",
                        Description = "Click File > Open or press Ctrl+O to open a file",
                        TargetElement = "FileMenu"
                    },
                    new HelpStep
                    {
                        StepNumber = 2,
                        Title = "Saving Files",
                        Description = "Click File > Save or press Ctrl+S to save changes",
                        TargetElement = "SaveButton"
                    }
                }
            });
            
            // Item editing help
            RegisterHelpContent("item-editing", new HelpContent
            {
                Title = "Item Editing",
                Description = "How to edit item properties and attributes",
                DetailedContent = "Select an item from the list to view and edit its properties. Use the properties panel to modify item attributes like name, type, and flags.",
                Type = HelpContentType.Tutorial,
                Category = "Editing",
                Keywords = new List<string> { "item", "edit", "properties", "attributes", "modify" }
            });
            
            // Keyboard shortcuts help
            RegisterHelpContent("keyboard-shortcuts", new HelpContent
            {
                Title = "Keyboard Shortcuts",
                Description = "Available keyboard shortcuts for faster navigation",
                DetailedContent = "Use keyboard shortcuts to work more efficiently:\n\n" +
                                "File Operations:\n" +
                                "• Ctrl+N - New file\n" +
                                "• Ctrl+O - Open file\n" +
                                "• Ctrl+S - Save file\n" +
                                "• Ctrl+Shift+S - Save as\n\n" +
                                "Navigation:\n" +
                                "• Up/Down arrows - Navigate items\n" +
                                "• Ctrl+Home - First item\n" +
                                "• Ctrl+End - Last item\n" +
                                "• Ctrl+F - Find item\n\n" +
                                "General:\n" +
                                "• F1 - Show help\n" +
                                "• F5 - Refresh\n" +
                                "• Escape - Cancel operation",
                Type = HelpContentType.Reference,
                Category = "Navigation",
                Keywords = new List<string> { "keyboard", "shortcuts", "hotkeys", "navigation", "quick" }
            });
            
            // Initialize onboarding tours
            InitializeOnboardingTours();
            
            _logger.LogDebug("Initialized default help content");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error initializing default help content");
        }
    }
    
    private void InitializeOnboardingTours()
    {
        try
        {
            // First-time user tour
            _onboardingTours["first-time"] = new List<HelpStep>
            {
                new HelpStep
                {
                    StepNumber = 1,
                    Title = "Welcome to ItemEditor",
                    Description = "Let's take a quick tour of the main features",
                    TargetElement = "MainWindow"
                },
                new HelpStep
                {
                    StepNumber = 2,
                    Title = "Opening Files",
                    Description = "Use the File menu or drag and drop to open .otb, .dat, or .spr files",
                    TargetElement = "FileMenu"
                },
                new HelpStep
                {
                    StepNumber = 3,
                    Title = "Item List",
                    Description = "Browse and search through items in this panel",
                    TargetElement = "ItemList"
                },
                new HelpStep
                {
                    StepNumber = 4,
                    Title = "Properties Panel",
                    Description = "Edit item properties and attributes here",
                    TargetElement = "PropertiesPanel"
                },
                new HelpStep
                {
                    StepNumber = 5,
                    Title = "You're Ready!",
                    Description = "You're all set to start editing items. Press F1 anytime for help.",
                    TargetElement = "MainWindow"
                }
            };
            
            _logger.LogDebug("Initialized onboarding tours");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error initializing onboarding tours");
        }
    }
    
    private async Task ShowHelpWindowAsync(HelpContent content, FrameworkElement? element)
    {
        try
        {
            // This would show a help window or dialog
            // For now, we'll show contextual help
            ShowContextualHelp("temp", element);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error showing help window");
        }
    }
    
    private FrameworkElement CreateTooltipContent(HelpContent content)
    {
        var panel = new StackPanel
        {
            MaxWidth = 350,
            Margin = new Thickness(8)
        };
        
        // Title
        if (!string.IsNullOrEmpty(content.Title))
        {
            var title = new TextBlock
            {
                Text = content.Title,
                FontWeight = FontWeights.Bold,
                FontSize = 14,
                Margin = new Thickness(0, 0, 0, 4)
            };
            panel.Children.Add(title);
        }
        
        // Description
        if (!string.IsNullOrEmpty(content.Description))
        {
            var description = new TextBlock
            {
                Text = content.Description,
                TextWrapping = TextWrapping.Wrap,
                Margin = new Thickness(0, 0, 0, 8)
            };
            panel.Children.Add(description);
        }
        
        // Links
        foreach (var link in content.Links.Take(3)) // Limit to 3 links in tooltip
        {
            var linkButton = new Button
            {
                Content = link.Text,
                Style = Application.Current.FindResource("LinkButtonStyle") as Style,
                HorizontalAlignment = HorizontalAlignment.Left,
                Margin = new Thickness(0, 2, 0, 2)
            };
            
            // Add click handler for link
            linkButton.Click += (s, e) => HandleLinkClick(link);
            
            panel.Children.Add(linkButton);
        }
        
        return panel;
    }
    
    private FrameworkElement CreateContextualHelpPanel(HelpContent content)
    {
        var border = new Border
        {
            Background = new SolidColorBrush(Color.FromArgb(240, 45, 45, 45)),
            BorderBrush = new SolidColorBrush(Color.FromArgb(255, 0, 120, 215)),
            BorderThickness = new Thickness(1),
            CornerRadius = new CornerRadius(4),
            MaxWidth = 400,
            Padding = new Thickness(16)
        };
        
        var panel = new StackPanel();
        
        // Close button
        var closeButton = new Button
        {
            Content = "×",
            HorizontalAlignment = HorizontalAlignment.Right,
            Width = 20,
            Height = 20,
            Margin = new Thickness(0, 0, 0, 8)
        };
        closeButton.Click += (s, e) => HideContextualHelp();
        panel.Children.Add(closeButton);
        
        // Title
        if (!string.IsNullOrEmpty(content.Title))
        {
            var title = new TextBlock
            {
                Text = content.Title,
                FontWeight = FontWeights.Bold,
                FontSize = 16,
                Foreground = Brushes.White,
                Margin = new Thickness(0, 0, 0, 8)
            };
            panel.Children.Add(title);
        }
        
        // Description
        if (!string.IsNullOrEmpty(content.Description))
        {
            var description = new TextBlock
            {
                Text = content.Description,
                TextWrapping = TextWrapping.Wrap,
                Foreground = Brushes.LightGray,
                Margin = new Thickness(0, 0, 0, 12)
            };
            panel.Children.Add(description);
        }
        
        // Detailed content
        if (!string.IsNullOrEmpty(content.DetailedContent))
        {
            var scrollViewer = new ScrollViewer
            {
                MaxHeight = 200,
                VerticalScrollBarVisibility = ScrollBarVisibility.Auto
            };
            
            var detailedText = new TextBlock
            {
                Text = content.DetailedContent,
                TextWrapping = TextWrapping.Wrap,
                Foreground = Brushes.White
            };
            
            scrollViewer.Content = detailedText;
            panel.Children.Add(scrollViewer);
        }
        
        border.Child = panel;
        return border;
    }
    
    private async Task ShowNextTourStepAsync()
    {
        try
        {
            if (_currentTourId == null || !_onboardingTours.ContainsKey(_currentTourId))
                return;
            
            var steps = _onboardingTours[_currentTourId];
            if (_currentTourStep >= steps.Count)
            {
                // Tour completed
                StopOnboardingTour();
                return;
            }
            
            var step = steps[_currentTourStep];
            
            // Raise step changed event
            OnboardingStepChanged?.Invoke(this, new OnboardingStepEventArgs(_currentTourId, _currentTourStep, step));
            
            // Show step content
            var content = new HelpContent
            {
                Title = step.Title,
                Description = step.Description,
                Type = HelpContentType.Tutorial
            };
            
            // Find target element
            FrameworkElement? targetElement = null;
            if (!string.IsNullOrEmpty(step.TargetElement))
            {
                targetElement = FindElementByName(step.TargetElement);
            }
            
            ShowContextualHelp("tour-step", targetElement);
            
            _currentTourStep++;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error showing next tour step");
        }
    }
    
    private FrameworkElement? FindElementByName(string elementName)
    {
        try
        {
            var mainWindow = Application.Current.MainWindow;
            if (mainWindow != null)
            {
                return mainWindow.FindName(elementName) as FrameworkElement;
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error finding element by name: {ElementName}", elementName);
        }
        
        return null;
    }
    
    private double CalculateRelevance(HelpContent content, string queryLower)
    {
        double relevance = 0;
        
        // Title match (highest weight)
        if (content.Title.ToLowerInvariant().Contains(queryLower))
            relevance += 10;
        
        // Description match
        if (content.Description.ToLowerInvariant().Contains(queryLower))
            relevance += 5;
        
        // Keyword match
        foreach (var keyword in content.Keywords)
        {
            if (keyword.ToLowerInvariant().Contains(queryLower))
                relevance += 3;
        }
        
        // Content match
        if (content.DetailedContent.ToLowerInvariant().Contains(queryLower))
            relevance += 1;
        
        return relevance;
    }
    
    private List<string> GetMatchedKeywords(HelpContent content, string queryLower)
    {
        return content.Keywords
            .Where(k => k.ToLowerInvariant().Contains(queryLower))
            .ToList();
    }
    
    private void HandleLinkClick(HelpLink link)
    {
        try
        {
            switch (link.Type)
            {
                case HelpLinkType.External:
                    System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo
                    {
                        FileName = link.Url,
                        UseShellExecute = true
                    });
                    break;
                    
                case HelpLinkType.Context:
                    ShowContextualHelp(link.Url);
                    break;
                    
                case HelpLinkType.Internal:
                    // Handle internal navigation
                    break;
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error handling link click: {Url}", link.Url);
        }
    }
    
    private static PlacementMode ConvertPlacement(TooltipPlacement placement)
    {
        return placement switch
        {
            TooltipPlacement.Top => PlacementMode.Top,
            TooltipPlacement.Bottom => PlacementMode.Bottom,
            TooltipPlacement.Left => PlacementMode.Left,
            TooltipPlacement.Right => PlacementMode.Right,
            TooltipPlacement.TopLeft => PlacementMode.Top,
            TooltipPlacement.TopRight => PlacementMode.Top,
            TooltipPlacement.BottomLeft => PlacementMode.Bottom,
            TooltipPlacement.BottomRight => PlacementMode.Bottom,
            _ => PlacementMode.Mouse
        };
    }
    
    #endregion
}