using System.Windows.Input;

namespace ItemEditor.Services;

/// <summary>
/// Interface for keyboard shortcut management
/// </summary>
public interface IKeyboardShortcutService
{
    /// <summary>
    /// Registers a keyboard shortcut
    /// </summary>
    /// <param name="key">Key combination</param>
    /// <param name="command">Command to execute</param>
    /// <param name="description">Description of the shortcut</param>
    /// <param name="category">Category for organization</param>
    void RegisterShortcut(KeyGesture key, ICommand command, string description, string category = "General");
    
    /// <summary>
    /// Registers a keyboard shortcut with custom key combination
    /// </summary>
    /// <param name="key">Key</param>
    /// <param name="modifiers">Modifier keys</param>
    /// <param name="command">Command to execute</param>
    /// <param name="description">Description of the shortcut</param>
    /// <param name="category">Category for organization</param>
    void RegisterShortcut(Key key, ModifierKeys modifiers, ICommand command, string description, string category = "General");
    
    /// <summary>
    /// Unregisters a keyboard shortcut
    /// </summary>
    /// <param name="key">Key combination to unregister</param>
    void UnregisterShortcut(KeyGesture key);
    
    /// <summary>
    /// Gets all registered shortcuts
    /// </summary>
    /// <returns>Dictionary of shortcuts grouped by category</returns>
    IReadOnlyDictionary<string, IReadOnlyList<KeyboardShortcut>> GetAllShortcuts();
    
    /// <summary>
    /// Gets shortcuts for a specific category
    /// </summary>
    /// <param name="category">Category name</param>
    /// <returns>List of shortcuts in the category</returns>
    IReadOnlyList<KeyboardShortcut> GetShortcutsForCategory(string category);
    
    /// <summary>
    /// Processes a key event and executes associated command if found
    /// </summary>
    /// <param name="e">Key event arguments</param>
    /// <returns>True if a shortcut was handled</returns>
    bool ProcessKeyEvent(KeyEventArgs e);
    
    /// <summary>
    /// Checks if a key combination is already registered
    /// </summary>
    /// <param name="key">Key combination to check</param>
    /// <returns>True if the combination is registered</returns>
    bool IsShortcutRegistered(KeyGesture key);
    
    /// <summary>
    /// Loads shortcuts from user preferences
    /// </summary>
    Task LoadShortcutsAsync();
    
    /// <summary>
    /// Saves shortcuts to user preferences
    /// </summary>
    Task SaveShortcutsAsync();
    
    /// <summary>
    /// Resets shortcuts to default values
    /// </summary>
    void ResetToDefaults();
    
    /// <summary>
    /// Event raised when shortcuts are changed
    /// </summary>
    event EventHandler<ShortcutsChangedEventArgs>? ShortcutsChanged;
}

/// <summary>
/// Represents a keyboard shortcut
/// </summary>
public class KeyboardShortcut
{
    public KeyGesture KeyGesture { get; set; } = null!;
    public ICommand Command { get; set; } = null!;
    public string Description { get; set; } = string.Empty;
    public string Category { get; set; } = "General";
    public bool IsCustom { get; set; }
    public bool IsEnabled { get; set; } = true;
    
    /// <summary>
    /// Gets the display text for the key gesture
    /// </summary>
    public string DisplayText => KeyGesture.GetDisplayStringForCulture(System.Globalization.CultureInfo.CurrentCulture);
}

/// <summary>
/// Event arguments for shortcuts changed events
/// </summary>
public class ShortcutsChangedEventArgs : EventArgs
{
    public string Category { get; }
    public KeyboardShortcut? AddedShortcut { get; }
    public KeyboardShortcut? RemovedShortcut { get; }
    public KeyboardShortcut? ModifiedShortcut { get; }
    
    public ShortcutsChangedEventArgs(string category, KeyboardShortcut? added = null, 
        KeyboardShortcut? removed = null, KeyboardShortcut? modified = null)
    {
        Category = category;
        AddedShortcut = added;
        RemovedShortcut = removed;
        ModifiedShortcut = modified;
    }
}