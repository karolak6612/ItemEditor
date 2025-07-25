namespace ItemEditor.Models;

/// <summary>
/// Application settings configuration
/// </summary>
public class ApplicationSettings
{
    /// <summary>
    /// Gets or sets the application name
    /// </summary>
    public string Name { get; set; } = "ItemEditor";
    
    /// <summary>
    /// Gets or sets the application version
    /// </summary>
    public string Version { get; set; } = "2.0.0";
    
    /// <summary>
    /// Gets or sets the theme setting
    /// </summary>
    public string Theme { get; set; } = "Auto";
    
    /// <summary>
    /// Gets or sets the accent color
    /// </summary>
    public string AccentColor { get; set; } = "System";
    
    /// <summary>
    /// Gets or sets the client directory path
    /// </summary>
    public string ClientDirectory { get; set; } = string.Empty;
    
    /// <summary>
    /// Gets or sets whether extended features are enabled
    /// </summary>
    public bool ExtendedFeatures { get; set; } = false;
    
    /// <summary>
    /// Gets or sets whether frame durations are supported
    /// </summary>
    public bool FrameDurations { get; set; } = false;
    
    /// <summary>
    /// Gets or sets whether transparency is supported
    /// </summary>
    public bool TransparencySupport { get; set; } = false;
    
    /// <summary>
    /// Gets or sets whether auto-save is enabled
    /// </summary>
    public bool AutoSave { get; set; } = false;
    
    /// <summary>
    /// Gets or sets whether to create backups
    /// </summary>
    public bool CreateBackups { get; set; } = true;
    
    /// <summary>
    /// Gets or sets the number of recent files to remember
    /// </summary>
    public int RecentFilesCount { get; set; } = 10;
    
    /// <summary>
    /// Gets or sets the application font size
    /// </summary>
    public double FontSize { get; set; } = 12.0;
    
    /// <summary>
    /// Gets or sets whether to show thumbnails
    /// </summary>
    public bool ShowThumbnails { get; set; } = true;
    
    /// <summary>
    /// Gets or sets the thumbnail size
    /// </summary>
    public double ThumbnailSize { get; set; } = 32.0;
    
    /// <summary>
    /// Gets or sets whether to associate .otb files
    /// </summary>
    public bool AssociateOtbFiles { get; set; } = false;
    
    /// <summary>
    /// Gets or sets whether to associate .dat files
    /// </summary>
    public bool AssociateDatFiles { get; set; } = false;
    
    /// <summary>
    /// Gets or sets whether to associate .spr files
    /// </summary>
    public bool AssociateSprFiles { get; set; } = false;
    
    /// <summary>
    /// Gets or sets the memory cache size in MB
    /// </summary>
    public int MemoryCacheSize { get; set; } = 256;
    
    /// <summary>
    /// Gets or sets whether multi-threading is enabled
    /// </summary>
    public bool EnableMultiThreading { get; set; } = true;
    
    /// <summary>
    /// Gets or sets whether hardware acceleration is enabled
    /// </summary>
    public bool HardwareAcceleration { get; set; } = true;
}

/// <summary>
/// Plugin settings configuration
/// </summary>
public class PluginSettings
{
    /// <summary>
    /// Gets or sets the plugin directory
    /// </summary>
    public string Directory { get; set; } = "Plugins";
    
    /// <summary>
    /// Gets or sets whether to auto-load plugins
    /// </summary>
    public bool AutoLoad { get; set; } = true;
    
    /// <summary>
    /// Gets or sets the plugin load timeout in milliseconds
    /// </summary>
    public int LoadTimeout { get; set; } = 30000;
}

/// <summary>
/// File settings configuration
/// </summary>
public class FileSettings
{
    /// <summary>
    /// Gets or sets the number of recent files to remember
    /// </summary>
    public int RecentFilesCount { get; set; } = 10;
    
    /// <summary>
    /// Gets or sets whether to auto-backup files
    /// </summary>
    public bool AutoBackup { get; set; } = true;
    
    /// <summary>
    /// Gets or sets the backup directory
    /// </summary>
    public string BackupDirectory { get; set; } = "Backups";
}