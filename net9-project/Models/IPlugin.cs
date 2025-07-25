namespace ItemEditor.Models;

/// <summary>
/// Interface for ItemEditor plugins
/// </summary>
public interface IPlugin
{
    /// <summary>
    /// Gets the plugin name
    /// </summary>
    string Name { get; }
    
    /// <summary>
    /// Gets the plugin version
    /// </summary>
    Version Version { get; }
    
    /// <summary>
    /// Gets the plugin description
    /// </summary>
    string Description { get; }
    
    /// <summary>
    /// Gets the plugin author
    /// </summary>
    string Author { get; }
    
    /// <summary>
    /// Initialize the plugin
    /// </summary>
    /// <returns>Task representing the async operation</returns>
    Task InitializeAsync();
    
    /// <summary>
    /// Shutdown the plugin
    /// </summary>
    /// <returns>Task representing the async operation</returns>
    Task ShutdownAsync();
}

/// <summary>
/// Event arguments for plugin events
/// </summary>
public class PluginEventArgs : EventArgs
{
    /// <summary>
    /// Initializes a new instance of the PluginEventArgs class
    /// </summary>
    /// <param name="plugin">The plugin</param>
    public PluginEventArgs(IPlugin plugin)
    {
        Plugin = plugin;
    }
    
    /// <summary>
    /// Gets the plugin
    /// </summary>
    public IPlugin Plugin { get; }
}