using System.Collections.Concurrent;
using System.Text.Json;
using System.Windows.Input;
using Microsoft.Extensions.Logging;

namespace ItemEditor.Services;

/// <summary>
/// Service for managing keyboard shortcuts
/// </summary>
public class KeyboardShortcutService : IKeyboardShortcutService
{
    private readonly ILogger<KeyboardShortcutService> _logger;
    private readonly ConcurrentDictionary<KeyGesture, KeyboardShortcut> _shortcuts = new();
    private readonly Dictionary<string, List<KeyboardShortcut>> _categorizedShortcuts = new();
    private readonly string _shortcutsFilePath;
    
    public KeyboardShortcutService(ILogger<KeyboardShortcutService> logger)
    {
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
        
        // Set up shortcuts file path
        var appDataPath = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
        var appFolder = Path.Combine(appDataPath, "ItemEditor");
        Directory.CreateDirectory(appFolder);
        _shortcutsFilePath = Path.Combine(appFolder, "shortcuts.json");
        
        // Initialize default categories
        _categorizedShortcuts["File"] = new List<KeyboardShortcut>();
        _categorizedShortcuts["Edit"] = new List<KeyboardShortcut>();
        _categorizedShortcuts["View"] = new List<KeyboardShortcut>();
        _categorizedShortcuts["Navigation"] = new List<KeyboardShortcut>();
        _categorizedShortcuts["General"] = new List<KeyboardShortcut>();
        
        _logger.LogDebug("KeyboardShortcutService initialized");
    }
    
    /// <inheritdoc />
    public void RegisterShortcut(KeyGesture key, ICommand command, string description, string category = "General")
    {
        try
        {
            if (key == null) throw new ArgumentNullException(nameof(key));
            if (command == null) throw new ArgumentNullException(nameof(command));
            if (string.IsNullOrWhiteSpace(description)) throw new ArgumentException("Description cannot be empty", nameof(description));
            
            var shortcut = new KeyboardShortcut
            {
                KeyGesture = key,
                Command = command,
                Description = description,
                Category = category,
                IsCustom = false,
                IsEnabled = true
            };
            
            // Remove existing shortcut if present
            if (_shortcuts.TryRemove(key, out var existingShortcut))
            {
                RemoveFromCategory(existingShortcut);
                _logger.LogDebug("Replaced existing shortcut for {Key}", key.DisplayString);
            }
            
            // Add new shortcut
            _shortcuts[key] = shortcut;
            AddToCategory(shortcut);
            
            _logger.LogDebug("Registered shortcut: {Key} -> {Description} ({Category})", 
                key.DisplayString, description, category);
            
            // Raise event
            ShortcutsChanged?.Invoke(this, new ShortcutsChangedEventArgs(category, shortcut, existingShortcut));
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error registering shortcut {Key}", key?.DisplayString ?? "null");
            throw;
        }
    }
    
    /// <inheritdoc />
    public void RegisterShortcut(Key key, ModifierKeys modifiers, ICommand command, string description, string category = "General")
    {
        var keyGesture = new KeyGesture(key, modifiers);
        RegisterShortcut(keyGesture, command, description, category);
    }
    
    /// <inheritdoc />
    public void UnregisterShortcut(KeyGesture key)
    {
        try
        {
            if (_shortcuts.TryRemove(key, out var shortcut))
            {
                RemoveFromCategory(shortcut);
                _logger.LogDebug("Unregistered shortcut: {Key}", key.DisplayString);
                
                // Raise event
                ShortcutsChanged?.Invoke(this, new ShortcutsChangedEventArgs(shortcut.Category, removed: shortcut));
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error unregistering shortcut {Key}", key?.DisplayString ?? "null");
        }
    }
    
    /// <inheritdoc />
    public IReadOnlyDictionary<string, IReadOnlyList<KeyboardShortcut>> GetAllShortcuts()
    {
        return _categorizedShortcuts.ToDictionary(
            kvp => kvp.Key,
            kvp => (IReadOnlyList<KeyboardShortcut>)kvp.Value.AsReadOnly()
        );
    }
    
    /// <inheritdoc />
    public IReadOnlyList<KeyboardShortcut> GetShortcutsForCategory(string category)
    {
        return _categorizedShortcuts.TryGetValue(category, out var shortcuts) 
            ? shortcuts.AsReadOnly() 
            : Array.Empty<KeyboardShortcut>();
    }
    
    /// <inheritdoc />
    public bool ProcessKeyEvent(KeyEventArgs e)
    {
        try
        {
            // Create key gesture from event
            var modifiers = Keyboard.Modifiers;
            var key = e.Key == Key.System ? e.SystemKey : e.Key;
            
            // Handle special cases
            if (key == Key.LeftCtrl || key == Key.RightCtrl ||
                key == Key.LeftAlt || key == Key.RightAlt ||
                key == Key.LeftShift || key == Key.RightShift ||
                key == Key.LWin || key == Key.RWin)
            {
                return false; // Don't handle modifier keys alone
            }
            
            var keyGesture = new KeyGesture(key, modifiers);
            
            // Find matching shortcut
            if (_shortcuts.TryGetValue(keyGesture, out var shortcut) && shortcut.IsEnabled)
            {
                if (shortcut.Command.CanExecute(null))
                {
                    _logger.LogDebug("Executing shortcut: {Key} -> {Description}", 
                        keyGesture.DisplayString, shortcut.Description);
                    
                    shortcut.Command.Execute(null);
                    e.Handled = true;
                    return true;
                }
            }
            
            return false;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error processing key event: {Key}", e.Key);
            return false;
        }
    }
    
    /// <inheritdoc />
    public bool IsShortcutRegistered(KeyGesture key)
    {
        return _shortcuts.ContainsKey(key);
    }
    
    /// <inheritdoc />
    public async Task LoadShortcutsAsync()
    {
        try
        {
            if (!File.Exists(_shortcutsFilePath))
            {
                _logger.LogDebug("Shortcuts file not found, using defaults");
                return;
            }
            
            var json = await File.ReadAllTextAsync(_shortcutsFilePath);
            var shortcutData = JsonSerializer.Deserialize<ShortcutData[]>(json);
            
            if (shortcutData != null)
            {
                foreach (var data in shortcutData)
                {
                    try
                    {
                        var keyGesture = new KeyGesture((Key)data.Key, (ModifierKeys)data.Modifiers);
                        
                        // Only load custom shortcuts - default shortcuts are registered by the application
                        if (data.IsCustom && _shortcuts.TryGetValue(keyGesture, out var existingShortcut))
                        {
                            existingShortcut.IsEnabled = data.IsEnabled;
                            existingShortcut.IsCustom = true;
                        }
                    }
                    catch (Exception ex)
                    {
                        _logger.LogWarning(ex, "Failed to load shortcut: {Key}+{Modifiers}", data.Key, data.Modifiers);
                    }
                }
                
                _logger.LogInformation("Loaded {Count} custom shortcuts", shortcutData.Length);
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error loading shortcuts from {FilePath}", _shortcutsFilePath);
        }
    }
    
    /// <inheritdoc />
    public async Task SaveShortcutsAsync()
    {
        try
        {
            var shortcutData = _shortcuts.Values
                .Where(s => s.IsCustom || !s.IsEnabled) // Save custom shortcuts and disabled defaults
                .Select(s => new ShortcutData
                {
                    Key = (int)s.KeyGesture.Key,
                    Modifiers = (int)s.KeyGesture.Modifiers,
                    Description = s.Description,
                    Category = s.Category,
                    IsCustom = s.IsCustom,
                    IsEnabled = s.IsEnabled
                })
                .ToArray();
            
            var json = JsonSerializer.Serialize(shortcutData, new JsonSerializerOptions
            {
                WriteIndented = true
            });
            
            await File.WriteAllTextAsync(_shortcutsFilePath, json);
            _logger.LogInformation("Saved {Count} shortcuts to {FilePath}", shortcutData.Length, _shortcutsFilePath);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error saving shortcuts to {FilePath}", _shortcutsFilePath);
        }
    }
    
    /// <inheritdoc />
    public void ResetToDefaults()
    {
        try
        {
            // Remove custom shortcuts and re-enable all defaults
            var customShortcuts = _shortcuts.Values.Where(s => s.IsCustom).ToList();
            foreach (var shortcut in customShortcuts)
            {
                UnregisterShortcut(shortcut.KeyGesture);
            }
            
            // Re-enable all default shortcuts
            foreach (var shortcut in _shortcuts.Values.Where(s => !s.IsCustom))
            {
                shortcut.IsEnabled = true;
            }
            
            _logger.LogInformation("Reset shortcuts to defaults");
            
            // Raise event for all categories
            foreach (var category in _categorizedShortcuts.Keys)
            {
                ShortcutsChanged?.Invoke(this, new ShortcutsChangedEventArgs(category));
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error resetting shortcuts to defaults");
        }
    }
    
    /// <inheritdoc />
    public event EventHandler<ShortcutsChangedEventArgs>? ShortcutsChanged;
    
    #region Private Methods
    
    private void AddToCategory(KeyboardShortcut shortcut)
    {
        if (!_categorizedShortcuts.ContainsKey(shortcut.Category))
        {
            _categorizedShortcuts[shortcut.Category] = new List<KeyboardShortcut>();
        }
        
        _categorizedShortcuts[shortcut.Category].Add(shortcut);
    }
    
    private void RemoveFromCategory(KeyboardShortcut shortcut)
    {
        if (_categorizedShortcuts.TryGetValue(shortcut.Category, out var categoryShortcuts))
        {
            categoryShortcuts.Remove(shortcut);
        }
    }
    
    #endregion
    
    #region Data Classes
    
    private class ShortcutData
    {
        public int Key { get; set; }
        public int Modifiers { get; set; }
        public string Description { get; set; } = string.Empty;
        public string Category { get; set; } = "General";
        public bool IsCustom { get; set; }
        public bool IsEnabled { get; set; } = true;
    }
    
    #endregion
}