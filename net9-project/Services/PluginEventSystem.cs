using Microsoft.Extensions.Logging;
using PluginInterface;
using ItemEditor.Models;
using System.Collections.Concurrent;

namespace ItemEditor.Services;

/// <summary>
/// Modern event system for plugin communication
/// </summary>
public class PluginEventSystem : IPluginEventSystem, IDisposable
{
    private readonly ILogger<PluginEventSystem> _logger;
    private readonly ConcurrentDictionary<string, List<IPluginEventHandler>> _eventHandlers = new();
    private readonly ConcurrentDictionary<string, List<Func<PluginEvent, Task>>> _asyncEventHandlers = new();
    private readonly object _lock = new();
    private bool _disposed;

    /// <summary>
    /// Initializes a new instance of the PluginEventSystem class
    /// </summary>
    /// <param name="logger">Logger instance</param>
    public PluginEventSystem(ILogger<PluginEventSystem> logger)
    {
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
    }

    /// <summary>
    /// Subscribes to a plugin event
    /// </summary>
    /// <param name="eventName">Event name</param>
    /// <param name="handler">Event handler</param>
    public void Subscribe(string eventName, IPluginEventHandler handler)
    {
        if (string.IsNullOrEmpty(eventName))
            throw new ArgumentException("Event name cannot be null or empty", nameof(eventName));
        
        if (handler == null)
            throw new ArgumentNullException(nameof(handler));

        lock (_lock)
        {
            if (!_eventHandlers.ContainsKey(eventName))
            {
                _eventHandlers[eventName] = new List<IPluginEventHandler>();
            }

            _eventHandlers[eventName].Add(handler);
            _logger.LogDebug("Subscribed handler to event: {EventName}", eventName);
        }
    }

    /// <summary>
    /// Subscribes to a plugin event with async handler
    /// </summary>
    /// <param name="eventName">Event name</param>
    /// <param name="handler">Async event handler</param>
    public void SubscribeAsync(string eventName, Func<PluginEvent, Task> handler)
    {
        if (string.IsNullOrEmpty(eventName))
            throw new ArgumentException("Event name cannot be null or empty", nameof(eventName));
        
        if (handler == null)
            throw new ArgumentNullException(nameof(handler));

        lock (_lock)
        {
            if (!_asyncEventHandlers.ContainsKey(eventName))
            {
                _asyncEventHandlers[eventName] = new List<Func<PluginEvent, Task>>();
            }

            _asyncEventHandlers[eventName].Add(handler);
            _logger.LogDebug("Subscribed async handler to event: {EventName}", eventName);
        }
    }

    /// <summary>
    /// Unsubscribes from a plugin event
    /// </summary>
    /// <param name="eventName">Event name</param>
    /// <param name="handler">Event handler</param>
    public void Unsubscribe(string eventName, IPluginEventHandler handler)
    {
        if (string.IsNullOrEmpty(eventName) || handler == null)
            return;

        lock (_lock)
        {
            if (_eventHandlers.TryGetValue(eventName, out var handlers))
            {
                handlers.Remove(handler);
                if (handlers.Count == 0)
                {
                    _eventHandlers.TryRemove(eventName, out _);
                }
                _logger.LogDebug("Unsubscribed handler from event: {EventName}", eventName);
            }
        }
    }

    /// <summary>
    /// Unsubscribes from a plugin event with async handler
    /// </summary>
    /// <param name="eventName">Event name</param>
    /// <param name="handler">Async event handler</param>
    public void UnsubscribeAsync(string eventName, Func<PluginEvent, Task> handler)
    {
        if (string.IsNullOrEmpty(eventName) || handler == null)
            return;

        lock (_lock)
        {
            if (_asyncEventHandlers.TryGetValue(eventName, out var handlers))
            {
                handlers.Remove(handler);
                if (handlers.Count == 0)
                {
                    _asyncEventHandlers.TryRemove(eventName, out _);
                }
                _logger.LogDebug("Unsubscribed async handler from event: {EventName}", eventName);
            }
        }
    }

    /// <summary>
    /// Publishes a plugin event
    /// </summary>
    /// <param name="pluginEvent">Plugin event</param>
    public void Publish(PluginEvent pluginEvent)
    {
        if (pluginEvent == null)
            throw new ArgumentNullException(nameof(pluginEvent));

        try
        {
            _logger.LogDebug("Publishing event: {EventName} from plugin: {PluginName}", 
                pluginEvent.EventName, pluginEvent.SourcePlugin?.Name ?? "Unknown");

            // Handle synchronous events
            if (_eventHandlers.TryGetValue(pluginEvent.EventName, out var handlers))
            {
                var handlersCopy = handlers.ToList(); // Create copy to avoid modification during iteration
                
                foreach (var handler in handlersCopy)
                {
                    try
                    {
                        handler.Handle(pluginEvent);
                    }
                    catch (Exception ex)
                    {
                        _logger.LogError(ex, "Error in event handler for event: {EventName}", pluginEvent.EventName);
                    }
                }
            }

            // Handle asynchronous events (fire and forget)
            if (_asyncEventHandlers.TryGetValue(pluginEvent.EventName, out var asyncHandlers))
            {
                var asyncHandlersCopy = asyncHandlers.ToList();
                
                _ = Task.Run(async () =>
                {
                    foreach (var handler in asyncHandlersCopy)
                    {
                        try
                        {
                            await handler(pluginEvent);
                        }
                        catch (Exception ex)
                        {
                            _logger.LogError(ex, "Error in async event handler for event: {EventName}", pluginEvent.EventName);
                        }
                    }
                });
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error publishing event: {EventName}", pluginEvent.EventName);
        }
    }

    /// <summary>
    /// Publishes a plugin event asynchronously
    /// </summary>
    /// <param name="pluginEvent">Plugin event</param>
    /// <returns>Task representing the async operation</returns>
    public async Task PublishAsync(PluginEvent pluginEvent)
    {
        if (pluginEvent == null)
            throw new ArgumentNullException(nameof(pluginEvent));

        try
        {
            _logger.LogDebug("Publishing async event: {EventName} from plugin: {PluginName}", 
                pluginEvent.EventName, pluginEvent.SourcePlugin?.Name ?? "Unknown");

            var tasks = new List<Task>();

            // Handle synchronous events
            if (_eventHandlers.TryGetValue(pluginEvent.EventName, out var handlers))
            {
                var handlersCopy = handlers.ToList();
                
                foreach (var handler in handlersCopy)
                {
                    tasks.Add(Task.Run(() =>
                    {
                        try
                        {
                            handler.Handle(pluginEvent);
                        }
                        catch (Exception ex)
                        {
                            _logger.LogError(ex, "Error in event handler for event: {EventName}", pluginEvent.EventName);
                        }
                    }));
                }
            }

            // Handle asynchronous events
            if (_asyncEventHandlers.TryGetValue(pluginEvent.EventName, out var asyncHandlers))
            {
                var asyncHandlersCopy = asyncHandlers.ToList();
                
                foreach (var handler in asyncHandlersCopy)
                {
                    tasks.Add(Task.Run(async () =>
                    {
                        try
                        {
                            await handler(pluginEvent);
                        }
                        catch (Exception ex)
                        {
                            _logger.LogError(ex, "Error in async event handler for event: {EventName}", pluginEvent.EventName);
                        }
                    }));
                }
            }

            // Wait for all handlers to complete
            if (tasks.Any())
            {
                await Task.WhenAll(tasks);
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error publishing async event: {EventName}", pluginEvent.EventName);
        }
    }

    /// <summary>
    /// Gets all registered event names
    /// </summary>
    /// <returns>Collection of event names</returns>
    public IEnumerable<string> GetRegisteredEvents()
    {
        var events = new HashSet<string>();
        
        foreach (var eventName in _eventHandlers.Keys)
        {
            events.Add(eventName);
        }
        
        foreach (var eventName in _asyncEventHandlers.Keys)
        {
            events.Add(eventName);
        }
        
        return events;
    }

    /// <summary>
    /// Gets the number of handlers for an event
    /// </summary>
    /// <param name="eventName">Event name</param>
    /// <returns>Number of handlers</returns>
    public int GetHandlerCount(string eventName)
    {
        if (string.IsNullOrEmpty(eventName))
            return 0;

        var count = 0;
        
        if (_eventHandlers.TryGetValue(eventName, out var handlers))
        {
            count += handlers.Count;
        }
        
        if (_asyncEventHandlers.TryGetValue(eventName, out var asyncHandlers))
        {
            count += asyncHandlers.Count;
        }
        
        return count;
    }

    /// <summary>
    /// Clears all event handlers
    /// </summary>
    public void Clear()
    {
        lock (_lock)
        {
            _eventHandlers.Clear();
            _asyncEventHandlers.Clear();
            _logger.LogInformation("Cleared all event handlers");
        }
    }

    /// <summary>
    /// Disposes the event system
    /// </summary>
    public void Dispose()
    {
        if (_disposed)
            return;

        Clear();
        _disposed = true;
        
        _logger.LogInformation("Plugin event system disposed");
    }
}

/// <summary>
/// Interface for plugin event system
/// </summary>
public interface IPluginEventSystem
{
    /// <summary>
    /// Subscribes to a plugin event
    /// </summary>
    /// <param name="eventName">Event name</param>
    /// <param name="handler">Event handler</param>
    void Subscribe(string eventName, IPluginEventHandler handler);

    /// <summary>
    /// Subscribes to a plugin event with async handler
    /// </summary>
    /// <param name="eventName">Event name</param>
    /// <param name="handler">Async event handler</param>
    void SubscribeAsync(string eventName, Func<PluginEvent, Task> handler);

    /// <summary>
    /// Unsubscribes from a plugin event
    /// </summary>
    /// <param name="eventName">Event name</param>
    /// <param name="handler">Event handler</param>
    void Unsubscribe(string eventName, IPluginEventHandler handler);

    /// <summary>
    /// Publishes a plugin event
    /// </summary>
    /// <param name="pluginEvent">Plugin event</param>
    void Publish(PluginEvent pluginEvent);

    /// <summary>
    /// Publishes a plugin event asynchronously
    /// </summary>
    /// <param name="pluginEvent">Plugin event</param>
    /// <returns>Task representing the async operation</returns>
    Task PublishAsync(PluginEvent pluginEvent);

    /// <summary>
    /// Gets all registered event names
    /// </summary>
    /// <returns>Collection of event names</returns>
    IEnumerable<string> GetRegisteredEvents();

    /// <summary>
    /// Gets the number of handlers for an event
    /// </summary>
    /// <param name="eventName">Event name</param>
    /// <returns>Number of handlers</returns>
    int GetHandlerCount(string eventName);

    /// <summary>
    /// Clears all event handlers
    /// </summary>
    void Clear();
}

/// <summary>
/// Plugin event handler interface
/// </summary>
public interface IPluginEventHandler
{
    /// <summary>
    /// Handles a plugin event
    /// </summary>
    /// <param name="pluginEvent">Plugin event</param>
    void Handle(PluginEvent pluginEvent);
}

/// <summary>
/// Plugin event data
/// </summary>
public class PluginEvent
{
    /// <summary>
    /// Initializes a new instance of the PluginEvent class
    /// </summary>
    /// <param name="eventName">Event name</param>
    /// <param name="sourcePlugin">Source plugin</param>
    /// <param name="data">Event data</param>
    public PluginEvent(string eventName, IPlugin? sourcePlugin = null, object? data = null)
    {
        EventName = eventName ?? throw new ArgumentNullException(nameof(eventName));
        SourcePlugin = sourcePlugin;
        Data = data;
        Timestamp = DateTime.UtcNow;
        EventId = Guid.NewGuid();
    }

    /// <summary>
    /// Gets the event name
    /// </summary>
    public string EventName { get; }

    /// <summary>
    /// Gets the source plugin
    /// </summary>
    public IPlugin? SourcePlugin { get; }

    /// <summary>
    /// Gets the event data
    /// </summary>
    public object? Data { get; }

    /// <summary>
    /// Gets the event timestamp
    /// </summary>
    public DateTime Timestamp { get; }

    /// <summary>
    /// Gets the unique event ID
    /// </summary>
    public Guid EventId { get; }

    /// <summary>
    /// Gets or sets whether the event was handled
    /// </summary>
    public bool Handled { get; set; }

    /// <summary>
    /// Gets or sets whether the event should be cancelled
    /// </summary>
    public bool Cancel { get; set; }
}