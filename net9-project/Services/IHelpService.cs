using System.Windows;

namespace ItemEditor.Services;

/// <summary>
/// Interface for context-sensitive help and guidance system
/// </summary>
public interface IHelpService
{
    /// <summary>
    /// Shows help for a specific context
    /// </summary>
    /// <param name="context">Help context identifier</param>
    /// <param name="element">UI element requesting help</param>
    Task ShowHelpAsync(string context, FrameworkElement? element = null);
    
    /// <summary>
    /// Shows a tooltip with rich content
    /// </summary>
    /// <param name="element">Element to show tooltip for</param>
    /// <param name="content">Tooltip content</param>
    /// <param name="placement">Tooltip placement</param>
    void ShowTooltip(FrameworkElement element, HelpContent content, TooltipPlacement placement = TooltipPlacement.Auto);
    
    /// <summary>
    /// Hides the current tooltip
    /// </summary>
    void HideTooltip();
    
    /// <summary>
    /// Shows contextual help panel
    /// </summary>
    /// <param name="context">Help context</param>
    /// <param name="targetElement">Target element for positioning</param>
    void ShowContextualHelp(string context, FrameworkElement? targetElement = null);
    
    /// <summary>
    /// Hides the contextual help panel
    /// </summary>
    void HideContextualHelp();
    
    /// <summary>
    /// Starts an onboarding tour
    /// </summary>
    /// <param name="tourId">Tour identifier</param>
    Task StartOnboardingTourAsync(string tourId);
    
    /// <summary>
    /// Stops the current onboarding tour
    /// </summary>
    void StopOnboardingTour();
    
    /// <summary>
    /// Registers help content for a context
    /// </summary>
    /// <param name="context">Context identifier</param>
    /// <param name="content">Help content</param>
    void RegisterHelpContent(string context, HelpContent content);
    
    /// <summary>
    /// Gets help content for a context
    /// </summary>
    /// <param name="context">Context identifier</param>
    /// <returns>Help content or null if not found</returns>
    HelpContent? GetHelpContent(string context);
    
    /// <summary>
    /// Searches help content
    /// </summary>
    /// <param name="query">Search query</param>
    /// <returns>Search results</returns>
    Task<IEnumerable<HelpSearchResult>> SearchHelpAsync(string query);
    
    /// <summary>
    /// Gets all available help contexts
    /// </summary>
    /// <returns>List of help contexts</returns>
    IEnumerable<string> GetAvailableContexts();
    
    /// <summary>
    /// Checks if help is available for a context
    /// </summary>
    /// <param name="context">Context identifier</param>
    /// <returns>True if help is available</returns>
    bool IsHelpAvailable(string context);
    
    /// <summary>
    /// Event raised when help is requested
    /// </summary>
    event EventHandler<HelpRequestedEventArgs>? HelpRequested;
    
    /// <summary>
    /// Event raised when onboarding tour step changes
    /// </summary>
    event EventHandler<OnboardingStepEventArgs>? OnboardingStepChanged;
}

/// <summary>
/// Help content with rich formatting
/// </summary>
public class HelpContent
{
    public string Title { get; set; } = string.Empty;
    public string Description { get; set; } = string.Empty;
    public string DetailedContent { get; set; } = string.Empty;
    public List<HelpLink> Links { get; set; } = new();
    public List<HelpImage> Images { get; set; } = new();
    public List<HelpStep> Steps { get; set; } = new();
    public HelpContentType Type { get; set; } = HelpContentType.General;
    public string Category { get; set; } = "General";
    public List<string> Keywords { get; set; } = new();
    public DateTime LastUpdated { get; set; } = DateTime.Now;
}

/// <summary>
/// Help link
/// </summary>
public class HelpLink
{
    public string Text { get; set; } = string.Empty;
    public string Url { get; set; } = string.Empty;
    public HelpLinkType Type { get; set; } = HelpLinkType.External;
}

/// <summary>
/// Help image
/// </summary>
public class HelpImage
{
    public string Source { get; set; } = string.Empty;
    public string AltText { get; set; } = string.Empty;
    public string Caption { get; set; } = string.Empty;
    public int Width { get; set; }
    public int Height { get; set; }
}

/// <summary>
/// Help step for tutorials
/// </summary>
public class HelpStep
{
    public int StepNumber { get; set; }
    public string Title { get; set; } = string.Empty;
    public string Description { get; set; } = string.Empty;
    public string TargetElement { get; set; } = string.Empty;
    public HelpImage? Image { get; set; }
    public List<HelpAction> Actions { get; set; } = new();
}

/// <summary>
/// Help action
/// </summary>
public class HelpAction
{
    public string Text { get; set; } = string.Empty;
    public string Command { get; set; } = string.Empty;
    public HelpActionType Type { get; set; } = HelpActionType.Button;
}

/// <summary>
/// Help search result
/// </summary>
public class HelpSearchResult
{
    public string Context { get; set; } = string.Empty;
    public HelpContent Content { get; set; } = null!;
    public double Relevance { get; set; }
    public List<string> MatchedKeywords { get; set; } = new();
}

/// <summary>
/// Tooltip placement options
/// </summary>
public enum TooltipPlacement
{
    Auto,
    Top,
    Bottom,
    Left,
    Right,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
}

/// <summary>
/// Help content types
/// </summary>
public enum HelpContentType
{
    General,
    Tutorial,
    Reference,
    Troubleshooting,
    FAQ,
    QuickTip
}

/// <summary>
/// Help link types
/// </summary>
public enum HelpLinkType
{
    External,
    Internal,
    Context,
    Command
}

/// <summary>
/// Help action types
/// </summary>
public enum HelpActionType
{
    Button,
    Link,
    Command,
    Navigation
}

/// <summary>
/// Event arguments for help requested events
/// </summary>
public class HelpRequestedEventArgs : EventArgs
{
    public string Context { get; }
    public FrameworkElement? Element { get; }
    public bool Handled { get; set; }
    
    public HelpRequestedEventArgs(string context, FrameworkElement? element = null)
    {
        Context = context;
        Element = element;
    }
}

/// <summary>
/// Event arguments for onboarding step events
/// </summary>
public class OnboardingStepEventArgs : EventArgs
{
    public string TourId { get; }
    public int StepNumber { get; }
    public HelpStep Step { get; }
    public bool IsCompleted { get; }
    
    public OnboardingStepEventArgs(string tourId, int stepNumber, HelpStep step, bool isCompleted = false)
    {
        TourId = tourId;
        StepNumber = stepNumber;
        Step = step;
        IsCompleted = isCompleted;
    }
}