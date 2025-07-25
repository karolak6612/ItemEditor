using System.Windows;

namespace ItemEditor.Services;

/// <summary>
/// Interface for accessibility features and screen reader support
/// </summary>
public interface IAccessibilityService
{
    /// <summary>
    /// Announces text to screen readers
    /// </summary>
    /// <param name="text">Text to announce</param>
    /// <param name="priority">Priority level</param>
    void Announce(string text, AccessibilityPriority priority = AccessibilityPriority.Normal);
    
    /// <summary>
    /// Sets focus to an element with proper accessibility handling
    /// </summary>
    /// <param name="element">Element to focus</param>
    /// <param name="announceChange">Whether to announce the focus change</param>
    /// <returns>True if focus was set successfully</returns>
    bool SetFocus(FrameworkElement element, bool announceChange = true);
    
    /// <summary>
    /// Gets the next focusable element in tab order
    /// </summary>
    /// <param name="current">Current element</param>
    /// <param name="forward">Direction to search</param>
    /// <returns>Next focusable element or null</returns>
    FrameworkElement? GetNextFocusableElement(FrameworkElement current, bool forward = true);
    
    /// <summary>
    /// Checks if an element is accessible to screen readers
    /// </summary>
    /// <param name="element">Element to check</param>
    /// <returns>True if accessible</returns>
    bool IsAccessible(FrameworkElement element);
    
    /// <summary>
    /// Sets accessibility properties for an element
    /// </summary>
    /// <param name="element">Element to configure</param>
    /// <param name="name">Accessible name</param>
    /// <param name="description">Accessible description</param>
    /// <param name="role">Accessibility role</param>
    void SetAccessibilityProperties(FrameworkElement element, string? name = null, 
        string? description = null, AccessibilityRole? role = null);
    
    /// <summary>
    /// Creates a live region for dynamic content announcements
    /// </summary>
    /// <param name="element">Element to make into a live region</param>
    /// <param name="politeness">Politeness level</param>
    void CreateLiveRegion(FrameworkElement element, LiveRegionPoliteness politeness = LiveRegionPoliteness.Polite);
    
    /// <summary>
    /// Updates a live region with new content
    /// </summary>
    /// <param name="element">Live region element</param>
    /// <param name="content">New content</param>
    void UpdateLiveRegion(FrameworkElement element, string content);
    
    /// <summary>
    /// Checks if high contrast mode is enabled
    /// </summary>
    /// <returns>True if high contrast mode is active</returns>
    bool IsHighContrastMode();
    
    /// <summary>
    /// Checks if screen reader is detected
    /// </summary>
    /// <returns>True if screen reader is running</returns>
    bool IsScreenReaderActive();
    
    /// <summary>
    /// Gets the current accessibility settings
    /// </summary>
    /// <returns>Accessibility settings</returns>
    AccessibilitySettings GetAccessibilitySettings();
    
    /// <summary>
    /// Event raised when accessibility settings change
    /// </summary>
    event EventHandler<AccessibilitySettingsChangedEventArgs>? AccessibilitySettingsChanged;
}

/// <summary>
/// Priority levels for accessibility announcements
/// </summary>
public enum AccessibilityPriority
{
    Low,
    Normal,
    High,
    Critical
}

/// <summary>
/// Accessibility roles for UI elements
/// </summary>
public enum AccessibilityRole
{
    Button,
    TextBox,
    ComboBox,
    ListBox,
    ListItem,
    MenuItem,
    TabItem,
    TabPanel,
    Group,
    Heading,
    Image,
    Link,
    ProgressBar,
    Slider,
    StatusBar,
    Table,
    Cell,
    ColumnHeader,
    RowHeader,
    Dialog,
    Alert,
    Region,
    Navigation,
    Main,
    Banner,
    Complementary,
    ContentInfo
}

/// <summary>
/// Live region politeness levels
/// </summary>
public enum LiveRegionPoliteness
{
    Off,
    Polite,
    Assertive
}

/// <summary>
/// Accessibility settings
/// </summary>
public class AccessibilitySettings
{
    public bool IsScreenReaderActive { get; set; }
    public bool IsHighContrastMode { get; set; }
    public bool IsKeyboardNavigationEnabled { get; set; }
    public bool IsAnimationEnabled { get; set; }
    public double TextScaleFactor { get; set; } = 1.0;
    public TimeSpan FocusTimeout { get; set; } = TimeSpan.FromSeconds(5);
    public bool ShowFocusVisuals { get; set; } = true;
    public bool AnnounceStatusChanges { get; set; } = true;
}

/// <summary>
/// Event arguments for accessibility settings changes
/// </summary>
public class AccessibilitySettingsChangedEventArgs : EventArgs
{
    public AccessibilitySettings OldSettings { get; }
    public AccessibilitySettings NewSettings { get; }
    
    public AccessibilitySettingsChangedEventArgs(AccessibilitySettings oldSettings, AccessibilitySettings newSettings)
    {
        OldSettings = oldSettings ?? throw new ArgumentNullException(nameof(oldSettings));
        NewSettings = newSettings ?? throw new ArgumentNullException(nameof(newSettings));
    }
}