using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Automation.Peers;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using Microsoft.Extensions.Logging;
using Microsoft.Win32;

namespace ItemEditor.Services;

/// <summary>
/// Service for accessibility features and screen reader support
/// </summary>
public class AccessibilityService : IAccessibilityService
{
    private readonly ILogger<AccessibilityService> _logger;
    private readonly Dictionary<FrameworkElement, string> _liveRegions = new();
    private AccessibilitySettings _currentSettings;
    
    // Windows API for screen reader detection
    [DllImport("user32.dll")]
    private static extern bool SystemParametersInfo(int uAction, int uParam, ref bool lpvParam, int flags);
    
    private const int SPI_GETSCREENREADER = 70;
    private const int SPI_GETHIGHCONTRAST = 66;
    
    public AccessibilityService(ILogger<AccessibilityService> logger)
    {
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
        _currentSettings = new AccessibilitySettings();
        
        // Initialize settings
        UpdateAccessibilitySettings();
        
        // Listen for system changes
        SystemEvents.UserPreferenceChanged += OnUserPreferenceChanged;
        
        _logger.LogDebug("AccessibilityService initialized");
    }
    
    /// <inheritdoc />
    public void Announce(string text, AccessibilityPriority priority = AccessibilityPriority.Normal)
    {
        try
        {
            if (string.IsNullOrWhiteSpace(text) || !_currentSettings.IsScreenReaderActive)
                return;
            
            _logger.LogDebug("Announcing to screen reader: {Text} (Priority: {Priority})", text, priority);
            
            // Use UI Automation to announce text
            var automationPeer = new GenericAutomationPeer();
            automationPeer.RaiseNotificationEvent(
                AutomationNotificationKind.ActionCompleted,
                priority switch
                {
                    AccessibilityPriority.Low => AutomationNotificationProcessing.ImportantMostRecent,
                    AccessibilityPriority.Normal => AutomationNotificationProcessing.ImportantAll,
                    AccessibilityPriority.High => AutomationNotificationProcessing.All,
                    AccessibilityPriority.Critical => AutomationNotificationProcessing.All,
                    _ => AutomationNotificationProcessing.ImportantAll
                },
                text,
                text
            );
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error announcing text to screen reader: {Text}", text);
        }
    }
    
    /// <inheritdoc />
    public bool SetFocus(FrameworkElement element, bool announceChange = true)
    {
        try
        {
            if (element == null || !element.IsVisible || !element.IsEnabled)
                return false;
            
            // Ensure element is focusable
            if (!element.Focusable)
            {
                element.Focusable = true;
            }
            
            var result = element.Focus();
            
            if (result && announceChange && _currentSettings.AnnounceStatusChanges)
            {
                var name = AutomationProperties.GetName(element);
                var description = AutomationProperties.GetHelpText(element);
                
                var announcement = !string.IsNullOrEmpty(name) ? name : element.GetType().Name;
                if (!string.IsNullOrEmpty(description))
                {
                    announcement += $", {description}";
                }
                
                Announce($"Focused on {announcement}");
            }
            
            _logger.LogDebug("Set focus to {ElementType} (Success: {Success})", element.GetType().Name, result);
            return result;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error setting focus to element");
            return false;
        }
    }
    
    /// <inheritdoc />
    public FrameworkElement? GetNextFocusableElement(FrameworkElement current, bool forward = true)
    {
        try
        {
            var request = new TraversalRequest(forward ? FocusNavigationDirection.Next : FocusNavigationDirection.Previous);
            var nextElement = current.PredictFocus(request.FocusNavigationDirection) as FrameworkElement;
            
            // Ensure the element is actually focusable and visible
            while (nextElement != null && (!nextElement.IsVisible || !nextElement.IsEnabled || !nextElement.Focusable))
            {
                nextElement = nextElement.PredictFocus(request.FocusNavigationDirection) as FrameworkElement;
            }
            
            return nextElement;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error finding next focusable element");
            return null;
        }
    }
    
    /// <inheritdoc />
    public bool IsAccessible(FrameworkElement element)
    {
        try
        {
            if (element == null)
                return false;
            
            // Check basic accessibility requirements
            var hasName = !string.IsNullOrEmpty(AutomationProperties.GetName(element));
            var hasRole = AutomationProperties.GetAutomationId(element) != null;
            var isVisible = element.IsVisible;
            var isInTabOrder = element.IsTabStop || element.Focusable;
            
            return hasName && isVisible && (isInTabOrder || hasRole);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error checking element accessibility");
            return false;
        }
    }
    
    /// <inheritdoc />
    public void SetAccessibilityProperties(FrameworkElement element, string? name = null, 
        string? description = null, AccessibilityRole? role = null)
    {
        try
        {
            if (element == null)
                return;
            
            if (!string.IsNullOrEmpty(name))
            {
                AutomationProperties.SetName(element, name);
            }
            
            if (!string.IsNullOrEmpty(description))
            {
                AutomationProperties.SetHelpText(element, description);
            }
            
            if (role.HasValue)
            {
                var automationRole = role.Value switch
                {
                    AccessibilityRole.Button => AutomationControlType.Button,
                    AccessibilityRole.TextBox => AutomationControlType.Edit,
                    AccessibilityRole.ComboBox => AutomationControlType.ComboBox,
                    AccessibilityRole.ListBox => AutomationControlType.List,
                    AccessibilityRole.ListItem => AutomationControlType.ListItem,
                    AccessibilityRole.MenuItem => AutomationControlType.MenuItem,
                    AccessibilityRole.TabItem => AutomationControlType.TabItem,
                    AccessibilityRole.TabPanel => AutomationControlType.Tab,
                    AccessibilityRole.Group => AutomationControlType.Group,
                    AccessibilityRole.Heading => AutomationControlType.Text,
                    AccessibilityRole.Image => AutomationControlType.Image,
                    AccessibilityRole.Link => AutomationControlType.Hyperlink,
                    AccessibilityRole.ProgressBar => AutomationControlType.ProgressBar,
                    AccessibilityRole.Slider => AutomationControlType.Slider,
                    AccessibilityRole.StatusBar => AutomationControlType.StatusBar,
                    AccessibilityRole.Table => AutomationControlType.Table,
                    AccessibilityRole.Cell => AutomationControlType.Cell,
                    AccessibilityRole.ColumnHeader => AutomationControlType.HeaderItem,
                    AccessibilityRole.RowHeader => AutomationControlType.HeaderItem,
                    AccessibilityRole.Dialog => AutomationControlType.Window,
                    AccessibilityRole.Alert => AutomationControlType.Text,
                    _ => AutomationControlType.Custom
                };
                
                AutomationProperties.SetAutomationControlType(element, automationRole);
            }
            
            _logger.LogDebug("Set accessibility properties for {ElementType}: Name={Name}, Role={Role}", 
                element.GetType().Name, name, role);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error setting accessibility properties");
        }
    }
    
    /// <inheritdoc />
    public void CreateLiveRegion(FrameworkElement element, LiveRegionPoliteness politeness = LiveRegionPoliteness.Polite)
    {
        try
        {
            if (element == null)
                return;
            
            var liveRegionProperty = politeness switch
            {
                LiveRegionPoliteness.Off => AutomationLiveSetting.Off,
                LiveRegionPoliteness.Polite => AutomationLiveSetting.Polite,
                LiveRegionPoliteness.Assertive => AutomationLiveSetting.Assertive,
                _ => AutomationLiveSetting.Polite
            };
            
            AutomationProperties.SetLiveSetting(element, liveRegionProperty);
            _liveRegions[element] = string.Empty;
            
            _logger.LogDebug("Created live region for {ElementType} with politeness {Politeness}", 
                element.GetType().Name, politeness);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error creating live region");
        }
    }
    
    /// <inheritdoc />
    public void UpdateLiveRegion(FrameworkElement element, string content)
    {
        try
        {
            if (element == null || !_liveRegions.ContainsKey(element))
                return;
            
            // Update the content
            if (element is TextBlock textBlock)
            {
                textBlock.Text = content;
            }
            else if (element is ContentControl contentControl)
            {
                contentControl.Content = content;
            }
            
            // Store the content for reference
            _liveRegions[element] = content;
            
            // Raise automation event
            var peer = UIElementAutomationPeer.FromElement(element);
            peer?.RaiseAutomationEvent(AutomationEvents.LiveRegionChanged);
            
            _logger.LogDebug("Updated live region content: {Content}", content);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error updating live region");
        }
    }
    
    /// <inheritdoc />
    public bool IsHighContrastMode()
    {
        try
        {
            bool isHighContrast = false;
            SystemParametersInfo(SPI_GETHIGHCONTRAST, 0, ref isHighContrast, 0);
            return isHighContrast;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error checking high contrast mode");
            return false;
        }
    }
    
    /// <inheritdoc />
    public bool IsScreenReaderActive()
    {
        try
        {
            bool isScreenReaderActive = false;
            SystemParametersInfo(SPI_GETSCREENREADER, 0, ref isScreenReaderActive, 0);
            return isScreenReaderActive;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error checking screen reader status");
            return false;
        }
    }
    
    /// <inheritdoc />
    public AccessibilitySettings GetAccessibilitySettings()
    {
        return new AccessibilitySettings
        {
            IsScreenReaderActive = _currentSettings.IsScreenReaderActive,
            IsHighContrastMode = _currentSettings.IsHighContrastMode,
            IsKeyboardNavigationEnabled = _currentSettings.IsKeyboardNavigationEnabled,
            IsAnimationEnabled = _currentSettings.IsAnimationEnabled,
            TextScaleFactor = _currentSettings.TextScaleFactor,
            FocusTimeout = _currentSettings.FocusTimeout,
            ShowFocusVisuals = _currentSettings.ShowFocusVisuals,
            AnnounceStatusChanges = _currentSettings.AnnounceStatusChanges
        };
    }
    
    /// <inheritdoc />
    public event EventHandler<AccessibilitySettingsChangedEventArgs>? AccessibilitySettingsChanged;
    
    #region Private Methods
    
    private void OnUserPreferenceChanged(object sender, Microsoft.Win32.UserPreferenceChangedEventArgs e)
    {
        if (e.Category == Microsoft.Win32.UserPreferenceCategory.Accessibility)
        {
            var oldSettings = GetAccessibilitySettings();
            UpdateAccessibilitySettings();
            var newSettings = GetAccessibilitySettings();
            
            AccessibilitySettingsChanged?.Invoke(this, new AccessibilitySettingsChangedEventArgs(oldSettings, newSettings));
        }
    }
    
    private void UpdateAccessibilitySettings()
    {
        try
        {
            _currentSettings.IsScreenReaderActive = IsScreenReaderActive();
            _currentSettings.IsHighContrastMode = IsHighContrastMode();
            _currentSettings.IsKeyboardNavigationEnabled = true; // Always enabled in WPF
            _currentSettings.IsAnimationEnabled = !_currentSettings.IsHighContrastMode; // Disable animations in high contrast
            
            // Get system text scale factor
            var dpi = VisualTreeHelper.GetDpi(Application.Current.MainWindow ?? new Window());
            _currentSettings.TextScaleFactor = dpi.DpiScaleX;
            
            _logger.LogDebug("Updated accessibility settings: ScreenReader={ScreenReader}, HighContrast={HighContrast}", 
                _currentSettings.IsScreenReaderActive, _currentSettings.IsHighContrastMode);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error updating accessibility settings");
        }
    }
    
    #endregion
    
    #region IDisposable
    
    public void Dispose()
    {
        try
        {
            SystemEvents.UserPreferenceChanged -= OnUserPreferenceChanged;
            _liveRegions.Clear();
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error disposing AccessibilityService");
        }
    }
    
    #endregion
}

/// <summary>
/// Generic automation peer for announcements
/// </summary>
internal class GenericAutomationPeer : AutomationPeer
{
    protected override string GetClassNameCore() => "GenericElement";
    protected override AutomationControlType GetAutomationControlTypeCore() => AutomationControlType.Text;
    protected override bool IsContentElementCore() => false;
    protected override bool IsControlElementCore() => false;
}