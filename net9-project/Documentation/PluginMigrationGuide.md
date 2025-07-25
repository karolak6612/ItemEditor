# Plugin Migration Guide

## Overview

This guide helps plugin developers migrate their existing plugins to work with the modern ItemEditor .NET 9 application while maintaining backward compatibility with legacy plugins.

## Plugin System Architecture

The new plugin system supports both modern and legacy plugins:

- **Modern Plugins**: Implement the `IPlugin` interface with async support, dependency injection, and advanced features
- **Legacy Plugins**: Existing plugins that implement `ILegacyPlugin` or follow legacy patterns
- **Compatibility Layer**: Automatic wrapping of legacy plugins to work with the modern system

## Legacy Plugin Support

### Automatic Detection

The system automatically detects legacy plugins by checking for:

1. **ILegacyPlugin Interface**: Direct implementation of the legacy interface
2. **Common Legacy Interfaces**: `IPlugin`, `IItemEditorPlugin`, `IOTBPlugin`, `IExtension`
3. **Legacy Attributes**: `PluginAttribute`, `ItemEditorPluginAttribute`, `ExtensionAttribute`
4. **Method Patterns**: Common initialization/shutdown method names

### Legacy Plugin Requirements

For automatic compatibility, legacy plugins should have:

```csharp
public class MyLegacyPlugin : ILegacyPlugin
{
    public string Name { get; } = "My Legacy Plugin";
    public Version Version { get; } = new Version(1, 0, 0);
    public string Description { get; } = "Legacy plugin description";
    public string Author { get; } = "Plugin Author";

    public void Initialize()
    {
        // Plugin initialization code
    }

    public void Shutdown()
    {
        // Plugin cleanup code
    }
}
```

### Generic Legacy Plugin Support

For plugins that don't implement `ILegacyPlugin`, the system uses reflection to detect:

- **Properties**: `Name`, `Version`, `Description`, `Author`
- **Initialization Methods**: `Initialize`, `Init`, `Load`, `Start`
- **Shutdown Methods**: `Shutdown`, `Dispose`, `Unload`, `Stop`

## Modern Plugin Interface

### IPlugin Interface

```csharp
public interface IPlugin : INotifyPropertyChanged, IDisposable
{
    string Name { get; }
    Version Version { get; }
    string Description { get; }
    string Author { get; }
    PluginMetadata Metadata { get; }
    bool IsLoaded { get; }
    bool IsCompatible { get; }

    Task InitializeAsync(ILogger? logger = null, CancellationToken cancellationToken = default);
    Task ShutdownAsync(CancellationToken cancellationToken = default);
    Task<PluginValidationResult> ValidateCompatibilityAsync(Version hostVersion, CancellationToken cancellationToken = default);
    PluginConfigurationSchema? GetConfigurationSchema();
    Task ConfigureAsync(PluginConfiguration configuration, CancellationToken cancellationToken = default);
}
```

### Modern Plugin Implementation

```csharp
public class MyModernPlugin : IPlugin
{
    private bool _isLoaded;
    private bool _disposed;

    public string Name => "My Modern Plugin";
    public Version Version => new Version(2, 0, 0);
    public string Description => "Modern plugin with async support";
    public string Author => "Plugin Author";
    
    public PluginMetadata Metadata { get; }
    public bool IsLoaded => _isLoaded;
    public bool IsCompatible => true;

    public event PropertyChangedEventHandler? PropertyChanged;

    public MyModernPlugin()
    {
        Metadata = new PluginMetadata
        {
            Name = Name,
            Version = Version,
            Description = Description,
            Author = Author,
            Category = PluginCategory.General,
            IsLegacy = false,
            SupportedHostVersions = new List<Version> { new Version(1, 0, 0) },
            Dependencies = new List<PluginDependency>()
        };
    }

    public async Task InitializeAsync(ILogger? logger = null, CancellationToken cancellationToken = default)
    {
        logger?.LogInformation("Initializing plugin: {PluginName}", Name);
        
        // Async initialization code
        await Task.Delay(100, cancellationToken); // Simulate async work
        
        _isLoaded = true;
        OnPropertyChanged(nameof(IsLoaded));
    }

    public async Task ShutdownAsync(CancellationToken cancellationToken = default)
    {
        // Async shutdown code
        await Task.Delay(50, cancellationToken); // Simulate async work
        
        _isLoaded = false;
        OnPropertyChanged(nameof(IsLoaded));
    }

    public Task<PluginValidationResult> ValidateCompatibilityAsync(Version hostVersion, CancellationToken cancellationToken = default)
    {
        var result = new PluginValidationResult
        {
            IsValid = hostVersion >= new Version(1, 0, 0),
            Message = hostVersion >= new Version(1, 0, 0) ? "Compatible" : "Requires host version 1.0.0 or higher"
        };
        
        return Task.FromResult(result);
    }

    public PluginConfigurationSchema? GetConfigurationSchema()
    {
        return new PluginConfigurationSchema
        {
            Properties = new List<ConfigurationProperty>
            {
                new ConfigurationProperty
                {
                    Name = "EnableFeature",
                    DisplayName = "Enable Feature",
                    Description = "Enables the main feature",
                    Type = ConfigurationPropertyType.Boolean,
                    DefaultValue = true
                }
            }
        };
    }

    public async Task ConfigureAsync(PluginConfiguration configuration, CancellationToken cancellationToken = default)
    {
        var enableFeature = configuration.GetValue<bool>("EnableFeature", true);
        // Apply configuration
    }

    protected virtual void OnPropertyChanged([CallerMemberName] string? propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }

    public void Dispose()
    {
        if (_disposed) return;
        
        if (_isLoaded)
        {
            ShutdownAsync().Wait(TimeSpan.FromSeconds(5));
        }
        
        _disposed = true;
    }
}
```

## Migration Steps

### Step 1: Assessment

1. **Run Compatibility Tests**: Use the built-in compatibility tester
2. **Identify Plugin Type**: Determine if your plugin is legacy or can be modernized
3. **Check Dependencies**: Review external dependencies for compatibility

### Step 2: Choose Migration Path

#### Option A: Keep as Legacy Plugin
- Minimal changes required
- Automatic compatibility layer handles integration
- Limited to legacy features

#### Option B: Migrate to Modern Interface
- Full access to modern features
- Better performance and integration
- Requires code changes

### Step 3: Legacy Plugin Updates (Option A)

```csharp
// Ensure your legacy plugin implements ILegacyPlugin
public class MyPlugin : ILegacyPlugin, IDisposable
{
    public string Name => "My Plugin";
    public Version Version => new Version(1, 0, 0);
    public string Description => "Legacy plugin";
    public string Author => "Author";

    public void Initialize()
    {
        // Initialization code
    }

    public void Shutdown()
    {
        // Cleanup code
    }

    public void Dispose()
    {
        Shutdown();
    }
}
```

### Step 4: Modern Plugin Migration (Option B)

1. **Update Interface**: Implement `IPlugin` instead of legacy interfaces
2. **Add Async Support**: Convert synchronous methods to async
3. **Add Metadata**: Provide rich plugin metadata
4. **Implement Configuration**: Add configuration support if needed
5. **Add Validation**: Implement compatibility validation
6. **Update Dependencies**: Use dependency injection where possible

## Dependency Injection Integration

Modern plugins can access host services through dependency injection:

```csharp
public class MyModernPlugin : IPlugin
{
    private readonly IFileService _fileService;
    private readonly ILogger<MyModernPlugin> _logger;

    // Services are injected automatically when plugin is loaded
    public MyModernPlugin(IFileService fileService, ILogger<MyModernPlugin> logger)
    {
        _fileService = fileService;
        _logger = logger;
    }

    public async Task InitializeAsync(ILogger? logger = null, CancellationToken cancellationToken = default)
    {
        // Use injected services
        _logger.LogInformation("Plugin initializing");
        
        // Access host file service
        var files = await _fileService.GetRecentFilesAsync();
    }
}
```

## Event System

Modern plugins can use the event system for communication:

```csharp
public class MyEventPlugin : IPlugin
{
    private IPluginEventSystem _eventSystem;

    public async Task InitializeAsync(ILogger? logger = null, CancellationToken cancellationToken = default)
    {
        // Subscribe to events
        _eventSystem.SubscribeAsync("FileOpened", OnFileOpened);
        
        // Publish events
        var pluginEvent = new PluginEvent("PluginReady", this);
        await _eventSystem.PublishAsync(pluginEvent);
    }

    private async Task OnFileOpened(PluginEvent pluginEvent)
    {
        // Handle file opened event
        var fileName = pluginEvent.Data as string;
        // Process file
    }
}
```

## Configuration Management

Modern plugins support rich configuration:

```csharp
public PluginConfigurationSchema GetConfigurationSchema()
{
    return new PluginConfigurationSchema
    {
        Groups = new List<ConfigurationGroup>
        {
            new ConfigurationGroup
            {
                Name = "General",
                DisplayName = "General Settings",
                PropertyNames = new List<string> { "EnableFeature", "MaxItems" }
            }
        },
        Properties = new List<ConfigurationProperty>
        {
            new ConfigurationProperty
            {
                Name = "EnableFeature",
                DisplayName = "Enable Main Feature",
                Type = ConfigurationPropertyType.Boolean,
                DefaultValue = true,
                ValidationRules = new List<ValidationRule>
                {
                    new ValidationRule { Type = ValidationRuleType.Required }
                }
            },
            new ConfigurationProperty
            {
                Name = "MaxItems",
                DisplayName = "Maximum Items",
                Type = ConfigurationPropertyType.Integer,
                DefaultValue = 100,
                ValidationRules = new List<ValidationRule>
                {
                    new ValidationRule { Type = ValidationRuleType.MinValue, Value = 1 },
                    new ValidationRule { Type = ValidationRuleType.MaxValue, Value = 1000 }
                }
            }
        }
    };
}
```

## Testing Your Plugin

### Compatibility Testing

```csharp
// Use the compatibility tester
var tester = serviceProvider.GetRequiredService<IPluginCompatibilityTester>();
var result = await tester.RunCompatibilityTestsAsync("path/to/your/plugin.dll");

if (result.OverallResult == CompatibilityTestResult.Passed)
{
    Console.WriteLine("Plugin is compatible!");
}
else
{
    Console.WriteLine($"Compatibility issues: {string.Join(", ", result.TestErrors)}");
}
```

### Unit Testing

```csharp
[Test]
public async Task Plugin_Initialize_ShouldSetLoadedState()
{
    // Arrange
    var plugin = new MyModernPlugin();
    
    // Act
    await plugin.InitializeAsync();
    
    // Assert
    Assert.IsTrue(plugin.IsLoaded);
}

[Test]
public async Task Plugin_Shutdown_ShouldClearLoadedState()
{
    // Arrange
    var plugin = new MyModernPlugin();
    await plugin.InitializeAsync();
    
    // Act
    await plugin.ShutdownAsync();
    
    // Assert
    Assert.IsFalse(plugin.IsLoaded);
}
```

## Best Practices

### For All Plugins

1. **Error Handling**: Always handle exceptions gracefully
2. **Resource Management**: Properly dispose of resources
3. **Thread Safety**: Ensure thread-safe operations
4. **Logging**: Use provided logging infrastructure
5. **Cancellation**: Support cancellation tokens

### For Modern Plugins

1. **Async/Await**: Use async patterns consistently
2. **Dependency Injection**: Leverage DI for loose coupling
3. **Configuration**: Provide rich configuration options
4. **Validation**: Implement proper validation
5. **Events**: Use event system for communication

### For Legacy Plugins

1. **Minimal Changes**: Keep changes to minimum for stability
2. **IDisposable**: Implement IDisposable for proper cleanup
3. **Exception Safety**: Ensure Initialize/Shutdown don't throw
4. **State Management**: Track plugin state properly

## Troubleshooting

### Common Issues

1. **Plugin Not Detected**
   - Ensure plugin implements required interface
   - Check file extension is .dll
   - Verify assembly is not corrupted

2. **Initialization Fails**
   - Check for missing dependencies
   - Verify parameterless constructor exists
   - Review exception logs

3. **Legacy Plugin Issues**
   - Ensure required properties exist
   - Check method signatures match expected patterns
   - Verify no abstract/interface types

4. **Modern Plugin Issues**
   - Implement all required interface members
   - Handle async operations properly
   - Provide valid metadata

### Debugging

1. **Enable Detailed Logging**: Set log level to Debug
2. **Use Compatibility Tester**: Run comprehensive tests
3. **Check Plugin Manager**: Review plugin status in UI
4. **Examine Dependencies**: Verify all required assemblies are available

## Migration Checklist

### Legacy Plugin Checklist
- [ ] Implements ILegacyPlugin or detectable pattern
- [ ] Has parameterless constructor
- [ ] Provides Name, Version, Description, Author
- [ ] Initialize/Shutdown methods work correctly
- [ ] Implements IDisposable if needed
- [ ] Passes compatibility tests

### Modern Plugin Checklist
- [ ] Implements IPlugin interface
- [ ] All interface members implemented
- [ ] Async methods use proper async/await
- [ ] Provides rich metadata
- [ ] Implements configuration if needed
- [ ] Supports cancellation tokens
- [ ] Proper error handling
- [ ] Thread-safe operations
- [ ] Passes all compatibility tests

## Support and Resources

- **Documentation**: Check the plugin interface documentation
- **Examples**: Review sample plugins in the SDK
- **Testing Tools**: Use built-in compatibility tester
- **Community**: Join the plugin developer community
- **Issues**: Report bugs through the issue tracker

## Conclusion

The new plugin system provides excellent backward compatibility while offering modern features for new development. Legacy plugins continue to work with minimal changes, while modern plugins can take advantage of advanced features like dependency injection, async operations, and rich configuration.

Choose the migration path that best fits your needs and timeline. The compatibility layer ensures your users can continue using existing plugins while you plan your migration strategy.