using System.ComponentModel.DataAnnotations;
using System.Text.Json.Serialization;

namespace ItemEditor.Models;

/// <summary>
/// Root configuration model containing all application settings
/// </summary>
public class AppConfiguration
{
    /// <summary>
    /// Gets or sets the configuration version for migration purposes
    /// </summary>
    public string Version { get; set; } = "2.0.0";
    
    /// <summary>
    /// Gets or sets the application-level settings
    /// </summary>
    public ApplicationSettings Application { get; set; } = new();
    
    /// <summary>
    /// Gets or sets the user-level settings
    /// </summary>
    public UserSettings User { get; set; } = new();
    
    /// <summary>
    /// Gets or sets the plugin settings
    /// </summary>
    public PluginSettings Plugins { get; set; } = new();
    
    /// <summary>
    /// Gets or sets the file handling settings
    /// </summary>
    public FileSettings Files { get; set; } = new();
    
    /// <summary>
    /// Gets or sets the UI settings
    /// </summary>
    public UISettings UI { get; set; } = new();
    
    /// <summary>
    /// Gets or sets the performance settings
    /// </summary>
    public PerformanceSettings Performance { get; set; } = new();
}

/// <summary>
/// User-specific settings that can be personalized
/// </summary>
public class UserSettings
{
    /// <summary>
    /// Gets or sets the user's preferred theme
    /// </summary>
    [JsonPropertyName("theme")]
    public string Theme { get; set; } = "Auto";
    
    /// <summary>
    /// Gets or sets the user's preferred accent color
    /// </summary>
    [JsonPropertyName("accentColor")]
    public string AccentColor { get; set; } = "System";
    
    /// <summary>
    /// Gets or sets the user's preferred font size
    /// </summary>
    [JsonPropertyName("fontSize")]
    [Range(8.0, 24.0, ErrorMessage = "Font size must be between 8 and 24")]
    public double FontSize { get; set; } = 12.0;
    
    /// <summary>
    /// Gets or sets the user's preferred language/locale
    /// </summary>
    [JsonPropertyName("language")]
    public string Language { get; set; } = "en-US";
    
    /// <summary>
    /// Gets or sets the recent files list
    /// </summary>
    [JsonPropertyName("recentFiles")]
    public List<RecentFileInfo> RecentFiles { get; set; } = new();
    
    /// <summary>
    /// Gets or sets the window state and position
    /// </summary>
    [JsonPropertyName("windowState")]
    public WindowStateSettings WindowState { get; set; } = new();
    
    /// <summary>
    /// Gets or sets the keyboard shortcuts customization
    /// </summary>
    [JsonPropertyName("keyboardShortcuts")]
    public Dictionary<string, string> KeyboardShortcuts { get; set; } = new();
}

/// <summary>
/// UI-specific settings
/// </summary>
public class UISettings
{
    /// <summary>
    /// Gets or sets whether to show thumbnails
    /// </summary>
    [JsonPropertyName("showThumbnails")]
    public bool ShowThumbnails { get; set; } = true;
    
    /// <summary>
    /// Gets or sets the thumbnail size
    /// </summary>
    [JsonPropertyName("thumbnailSize")]
    [Range(16.0, 128.0, ErrorMessage = "Thumbnail size must be between 16 and 128")]
    public double ThumbnailSize { get; set; } = 32.0;
    
    /// <summary>
    /// Gets or sets whether to show the status bar
    /// </summary>
    [JsonPropertyName("showStatusBar")]
    public bool ShowStatusBar { get; set; } = true;
    
    /// <summary>
    /// Gets or sets whether to show the toolbar
    /// </summary>
    [JsonPropertyName("showToolbar")]
    public bool ShowToolbar { get; set; } = true;
    
    /// <summary>
    /// Gets or sets whether to show line numbers in editors
    /// </summary>
    [JsonPropertyName("showLineNumbers")]
    public bool ShowLineNumbers { get; set; } = false;
    
    /// <summary>
    /// Gets or sets the grid view settings
    /// </summary>
    [JsonPropertyName("gridView")]
    public GridViewSettings GridView { get; set; } = new();
}

/// <summary>
/// Performance-related settings
/// </summary>
public class PerformanceSettings
{
    /// <summary>
    /// Gets or sets the memory cache size in MB
    /// </summary>
    [JsonPropertyName("memoryCacheSize")]
    [Range(64, 2048, ErrorMessage = "Memory cache size must be between 64 and 2048 MB")]
    public int MemoryCacheSize { get; set; } = 256;
    
    /// <summary>
    /// Gets or sets whether multi-threading is enabled
    /// </summary>
    [JsonPropertyName("enableMultiThreading")]
    public bool EnableMultiThreading { get; set; } = true;
    
    /// <summary>
    /// Gets or sets whether hardware acceleration is enabled
    /// </summary>
    [JsonPropertyName("hardwareAcceleration")]
    public bool HardwareAcceleration { get; set; } = true;
    
    /// <summary>
    /// Gets or sets the maximum number of concurrent operations
    /// </summary>
    [JsonPropertyName("maxConcurrentOperations")]
    [Range(1, 16, ErrorMessage = "Max concurrent operations must be between 1 and 16")]
    public int MaxConcurrentOperations { get; set; } = 4;
    
    /// <summary>
    /// Gets or sets whether to preload thumbnails
    /// </summary>
    [JsonPropertyName("preloadThumbnails")]
    public bool PreloadThumbnails { get; set; } = true;
}

/// <summary>
/// Recent file information
/// </summary>
public class RecentFileInfo
{
    /// <summary>
    /// Gets or sets the file path
    /// </summary>
    [JsonPropertyName("path")]
    public string Path { get; set; } = string.Empty;
    
    /// <summary>
    /// Gets or sets the last accessed time
    /// </summary>
    [JsonPropertyName("lastAccessed")]
    public DateTime LastAccessed { get; set; } = DateTime.Now;
    
    /// <summary>
    /// Gets or sets the file type
    /// </summary>
    [JsonPropertyName("fileType")]
    public string FileType { get; set; } = string.Empty;
    
    /// <summary>
    /// Gets or sets whether the file is pinned
    /// </summary>
    [JsonPropertyName("isPinned")]
    public bool IsPinned { get; set; } = false;
}

/// <summary>
/// Window state settings
/// </summary>
public class WindowStateSettings
{
    /// <summary>
    /// Gets or sets the window width
    /// </summary>
    [JsonPropertyName("width")]
    public double Width { get; set; } = 1200;
    
    /// <summary>
    /// Gets or sets the window height
    /// </summary>
    [JsonPropertyName("height")]
    public double Height { get; set; } = 800;
    
    /// <summary>
    /// Gets or sets the window left position
    /// </summary>
    [JsonPropertyName("left")]
    public double Left { get; set; } = 100;
    
    /// <summary>
    /// Gets or sets the window top position
    /// </summary>
    [JsonPropertyName("top")]
    public double Top { get; set; } = 100;
    
    /// <summary>
    /// Gets or sets whether the window is maximized
    /// </summary>
    [JsonPropertyName("isMaximized")]
    public bool IsMaximized { get; set; } = false;
    
    /// <summary>
    /// Gets or sets the splitter positions
    /// </summary>
    [JsonPropertyName("splitterPositions")]
    public Dictionary<string, double> SplitterPositions { get; set; } = new();
}

/// <summary>
/// Grid view settings
/// </summary>
public class GridViewSettings
{
    /// <summary>
    /// Gets or sets the column widths
    /// </summary>
    [JsonPropertyName("columnWidths")]
    public Dictionary<string, double> ColumnWidths { get; set; } = new();
    
    /// <summary>
    /// Gets or sets the column order
    /// </summary>
    [JsonPropertyName("columnOrder")]
    public List<string> ColumnOrder { get; set; } = new();
    
    /// <summary>
    /// Gets or sets the sort column
    /// </summary>
    [JsonPropertyName("sortColumn")]
    public string SortColumn { get; set; } = "Name";
    
    /// <summary>
    /// Gets or sets the sort direction
    /// </summary>
    [JsonPropertyName("sortDirection")]
    public string SortDirection { get; set; } = "Ascending";
}

/// <summary>
/// Configuration validation result
/// </summary>
public class ConfigurationValidationResult
{
    /// <summary>
    /// Gets or sets whether the configuration is valid
    /// </summary>
    public bool IsValid { get; set; } = true;
    
    /// <summary>
    /// Gets or sets the validation errors
    /// </summary>
    public List<string> Errors { get; set; } = new();
    
    /// <summary>
    /// Gets or sets the validation warnings
    /// </summary>
    public List<string> Warnings { get; set; } = new();
    
    /// <summary>
    /// Gets or sets the section that failed validation
    /// </summary>
    public string? FailedSection { get; set; }
}