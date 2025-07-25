namespace ItemEditor.Services;

/// <summary>
/// Event arguments for theme change events
/// </summary>
public class ThemeChangedEventArgs : EventArgs
{
    /// <summary>
    /// Gets the previous theme name
    /// </summary>
    public string OldTheme { get; }
    
    /// <summary>
    /// Gets the new theme name
    /// </summary>
    public string NewTheme { get; }
    
    /// <summary>
    /// Initializes a new instance of the ThemeChangedEventArgs class
    /// </summary>
    /// <param name="oldTheme">The previous theme name</param>
    /// <param name="newTheme">The new theme name</param>
    public ThemeChangedEventArgs(string oldTheme, string newTheme)
    {
        OldTheme = oldTheme;
        NewTheme = newTheme;
    }
}

/// <summary>
/// Interface for theme management service
/// </summary>
public interface IThemeService
{
    /// <summary>
    /// Event raised when the theme changes
    /// </summary>
    event EventHandler<ThemeChangedEventArgs>? ThemeChanged;
    
    /// <summary>
    /// Gets the current theme name
    /// </summary>
    /// <returns>The current theme name</returns>
    string GetCurrentTheme();
    
    /// <summary>
    /// Sets the application theme
    /// </summary>
    /// <param name="themeName">Name of the theme to set</param>
    /// <returns>A task representing the asynchronous operation</returns>
    Task SetThemeAsync(string themeName);
    
    /// <summary>
    /// Gets the list of available themes
    /// </summary>
    /// <returns>Collection of available theme names</returns>
    IEnumerable<string> GetAvailableThemes();
    
    /// <summary>
    /// Initializes the theme service
    /// </summary>
    /// <returns>A task representing the asynchronous operation</returns>
    Task InitializeAsync();
    
    /// <summary>
    /// Sets the accent color
    /// </summary>
    /// <param name="accentColor">Name of the accent color to set</param>
    /// <returns>A task representing the asynchronous operation</returns>
    Task SetAccentColorAsync(string accentColor);
    
    /// <summary>
    /// Gets the current accent color
    /// </summary>
    /// <returns>The current accent color name</returns>
    string GetCurrentAccentColor();
    
    /// <summary>
    /// Previews a theme temporarily without saving
    /// </summary>
    /// <param name="themeName">Name of the theme to preview</param>
    void PreviewTheme(string themeName);
    
    /// <summary>
    /// Previews an accent color temporarily without saving
    /// </summary>
    /// <param name="accentColor">Name of the accent color to preview</param>
    void PreviewAccentColor(string accentColor);
    
    /// <summary>
    /// Cancels any active preview and restores the original theme
    /// </summary>
    void CancelPreview();
}