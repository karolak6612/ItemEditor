# Design Document

## Overview

This document outlines the comprehensive design for migrating the Legacy ItemEditor from .NET Framework 4.8 with Windows Forms to a modern .NET 9 application using WPF and WPFUI. The design maintains 100% functional parity while implementing contemporary architecture patterns, modern UI design, and enhanced performance characteristics.

The ItemEditor is a specialized tool for editing OpenTibia Binary (OTB) files, sprites (.spr), and data files (.dat) used in OpenTibia server development. The migration will transform a Windows Forms application with DarkUI theming into a modern WPF application with WPFUI design system while preserving all existing functionality and plugin compatibility.

## Architecture

### High-Level Architecture

The new application follows a modern layered architecture with clear separation of concerns:

```
┌─────────────────────────────────────────────────────────────┐
│                    Presentation Layer                       │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────┐ │
│  │   WPF Views     │  │   WPFUI Themes  │  │  Converters │ │
│  │   (XAML)        │  │   & Styles      │  │             │ │
│  └─────────────────┘  └─────────────────┘  └─────────────┘ │
└─────────────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│                   ViewModel Layer (MVVM)                    │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────┐ │
│  │  MainViewModel  │  │ DialogViewModels│  │   Commands  │ │
│  │                 │  │                 │  │             │ │
│  └─────────────────┘  └─────────────────┘  └─────────────┘ │
└─────────────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│                    Service Layer                            │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────┐ │
│  │  File Services  │  │ Plugin Services │  │Image Service│ │
│  │                 │  │                 │  │             │ │
│  └─────────────────┘  └─────────────────┘  └─────────────┘ │
└─────────────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│                    Core Library Layer                       │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────┐ │
│  │   OTLib Core    │  │ Plugin Interface│  │ Data Models │ │
│  │   (Modernized)  │  │   (Enhanced)    │  │             │ │
│  └─────────────────┘  └─────────────────┘  └─────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

### Technology Stack

**Framework & Runtime:**
- .NET 9 with C# 12 language features
- Windows Presentation Foundation (WPF) for UI framework
- WPFUI library for modern Fluent Design components

**MVVM Framework:**
- CommunityToolkit.Mvvm for source-generated MVVM patterns
- ObservableObject base classes with source generators
- RelayCommand implementations for command binding

**Dependency Injection:**
- Microsoft.Extensions.DependencyInjection for IoC container
- Microsoft.Extensions.Hosting for application lifecycle management
- Service-based architecture with proper abstractions

**Modern Patterns:**
- Async/await throughout for non-blocking operations
- Nullable reference types for improved type safety
- Source generators for performance-critical code paths
- Modern file I/O with System.IO.Pipelines for large files

## Components and Interfaces

### Core Application Components

#### 1. Application Entry Point
```csharp
// Program.cs - Modern application bootstrap
public class Program
{
    [STAThread]
    public static async Task Main(string[] args)
    {
        var builder = Host.CreateApplicationBuilder(args);
        
        // Configure services
        builder.Services.AddSingleton<App>();
        builder.Services.AddSingleton<MainWindow>();
        builder.Services.AddSingleton<MainViewModel>();
        
        // Add WPFUI services
        builder.Services.AddNavigationViewPageProvider();
        builder.Services.AddSingleton<INavigationService, NavigationService>();
        
        // Add application services
        builder.Services.AddSingleton<IFileService, FileService>();
        builder.Services.AddSingleton<IPluginService, PluginService>();
        builder.Services.AddSingleton<IImageService, ImageService>();
        
        var host = builder.Build();
        
        var app = host.Services.GetRequiredService<App>();
        app.InitializeComponent();
        app.Run();
    }
}
```

#### 2. Main Application Window
```csharp
// MainWindow.xaml.cs - Primary application window
public partial class MainWindow : FluentWindow
{
    public MainWindow(MainViewModel viewModel, INavigationService navigationService)
    {
        DataContext = viewModel;
        InitializeComponent();
        
        // Configure WPFUI navigation
        NavigationView.SetNavigationService(navigationService);
        
        // Apply theme management
        Loaded += OnWindowLoaded;
    }
    
    private void OnWindowLoaded(object sender, RoutedEventArgs e)
    {
        SystemThemeWatcher.Watch(this, WindowBackdropType.Mica, true);
    }
}
```

#### 3. Main ViewModel
```csharp
// MainViewModel.cs - Primary application view model
public partial class MainViewModel : ObservableObject
{
    private readonly IFileService _fileService;
    private readonly IPluginService _pluginService;
    
    [ObservableProperty]
    private ObservableCollection<ItemViewModel> _items = new();
    
    [ObservableProperty]
    private ItemViewModel? _selectedItem;
    
    [ObservableProperty]
    private bool _isLoading;
    
    public MainViewModel(IFileService fileService, IPluginService pluginService)
    {
        _fileService = fileService;
        _pluginService = pluginService;
        
        // Initialize commands
        OpenFileCommand = new AsyncRelayCommand<string>(OpenFileAsync);
        SaveFileCommand = new AsyncRelayCommand(SaveFileAsync);
    }
    
    public IAsyncRelayCommand<string> OpenFileCommand { get; }
    public IAsyncRelayCommand SaveFileCommand { get; }
    
    private async Task OpenFileAsync(string? filePath)
    {
        if (string.IsNullOrEmpty(filePath)) return;
        
        IsLoading = true;
        try
        {
            var items = await _fileService.LoadItemsAsync(filePath);
            Items.Clear();
            foreach (var item in items)
            {
                Items.Add(new ItemViewModel(item));
            }
        }
        finally
        {
            IsLoading = false;
        }
    }
}
```

### Service Layer Interfaces

#### 1. File Service Interface
```csharp
public interface IFileService
{
    Task<IEnumerable<Item>> LoadItemsAsync(string filePath, CancellationToken cancellationToken = default);
    Task SaveItemsAsync(string filePath, IEnumerable<Item> items, CancellationToken cancellationToken = default);
    Task<bool> ValidateFileAsync(string filePath, CancellationToken cancellationToken = default);
    Task<FileMetadata> GetFileMetadataAsync(string filePath, CancellationToken cancellationToken = default);
}
```

#### 2. Plugin Service Interface
```csharp
public interface IPluginService
{
    Task<IEnumerable<IPlugin>> LoadPluginsAsync(string pluginDirectory);
    Task<bool> ValidatePluginAsync(string pluginPath);
    IEnumerable<IPlugin> GetLoadedPlugins();
    Task UnloadPluginAsync(IPlugin plugin);
    event EventHandler<PluginEventArgs> PluginLoaded;
    event EventHandler<PluginEventArgs> PluginUnloaded;
}
```

#### 3. Image Service Interface
```csharp
public interface IImageService
{
    Task<BitmapSource> LoadSpriteAsync(string spritePath, int spriteId);
    Task<BitmapSource> RenderItemAsync(Item item, int width = 32, int height = 32);
    Task<byte[]> GenerateThumbnailAsync(BitmapSource source, int maxWidth, int maxHeight);
    Task<bool> CompareSpriteHashAsync(byte[] hash1, byte[] hash2);
}
```

### Modern Control Components

#### 1. Item List Control
```xml
<!-- ItemListView.xaml - Modern item list with virtualization -->
<UserControl x:Class="ItemEditor.Controls.ItemListView">
    <Grid>
        <ui:ListView 
            ItemsSource="{Binding Items}"
            SelectedItem="{Binding SelectedItem}"
            VirtualizingPanel.IsVirtualizing="True"
            VirtualizingPanel.VirtualizationMode="Recycling">
            <ListView.ItemTemplate>
                <DataTemplate>
                    <ui:Card Margin="4" Padding="8">
                        <Grid>
                            <Grid.ColumnDefinitions>
                                <ColumnDefinition Width="48"/>
                                <ColumnDefinition Width="*"/>
                            </Grid.ColumnDefinitions>
                            
                            <Image Grid.Column="0" 
                                   Source="{Binding Thumbnail}" 
                                   Width="32" Height="32"/>
                            
                            <StackPanel Grid.Column="1" Margin="8,0,0,0">
                                <ui:TextBlock Text="{Binding Name}" 
                                            FontTypography="BodyStrong"/>
                                <ui:TextBlock Text="{Binding Id, StringFormat='ID: {0}'}" 
                                            FontTypography="Caption"/>
                            </StackPanel>
                        </Grid>
                    </ui:Card>
                </DataTemplate>
            </ListView.ItemTemplate>
        </ui:ListView>
    </Grid>
</UserControl>
```

#### 2. Property Editor Control
```xml
<!-- PropertyEditor.xaml - Modern property editing interface -->
<UserControl x:Class="ItemEditor.Controls.PropertyEditor">
    <ui:Card>
        <ScrollViewer>
            <StackPanel Spacing="8">
                <ui:TextBox 
                    Header="Item Name"
                    Text="{Binding SelectedItem.Name, UpdateSourceTrigger=PropertyChanged}"/>
                
                <ui:NumberBox 
                    Header="Item ID"
                    Value="{Binding SelectedItem.Id}"
                    IsReadOnly="True"/>
                
                <ui:ComboBox 
                    Header="Item Type"
                    ItemsSource="{Binding ItemTypes}"
                    SelectedItem="{Binding SelectedItem.Type}"/>
                
                <ui:ToggleSwitch 
                    Header="Stackable"
                    IsOn="{Binding SelectedItem.IsStackable}"/>
                
                <!-- Additional properties... -->
            </StackPanel>
        </ScrollViewer>
    </ui:Card>
</UserControl>
```

## Data Models

### Core Data Models

#### 1. Enhanced Item Model
```csharp
public class Item : INotifyPropertyChanged
{
    private string _name = string.Empty;
    private ushort _id;
    private ServerItemType _type;
    private bool _isStackable;
    private BitmapSource? _thumbnail;
    
    public string Name
    {
        get => _name;
        set => SetProperty(ref _name, value);
    }
    
    public ushort Id
    {
        get => _id;
        set => SetProperty(ref _id, value);
    }
    
    public ServerItemType Type
    {
        get => _type;
        set => SetProperty(ref _type, value);
    }
    
    public bool IsStackable
    {
        get => _isStackable;
        set => SetProperty(ref _isStackable, value);
    }
    
    public BitmapSource? Thumbnail
    {
        get => _thumbnail;
        set => SetProperty(ref _thumbnail, value);
    }
    
    // Sprite and rendering properties
    public List<Sprite> Sprites { get; set; } = new();
    public byte[]? SpriteHash { get; set; }
    
    // Server item properties (maintaining compatibility)
    public TileStackOrder StackOrder { get; set; }
    public ushort Speed { get; set; }
    public ushort LightLevel { get; set; }
    public ushort LightColor { get; set; }
    
    public event PropertyChangedEventHandler? PropertyChanged;
    
    protected virtual void SetProperty<T>(ref T field, T value, [CallerMemberName] string? propertyName = null)
    {
        if (!EqualityComparer<T>.Default.Equals(field, value))
        {
            field = value;
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
```

#### 2. Plugin Metadata Model
```csharp
public class PluginMetadata
{
    public string Name { get; set; } = string.Empty;
    public Version Version { get; set; } = new();
    public string Description { get; set; } = string.Empty;
    public string Author { get; set; } = string.Empty;
    public List<SupportedClient> SupportedClients { get; set; } = new();
    public bool IsLoaded { get; set; }
    public string FilePath { get; set; } = string.Empty;
    public DateTime LoadedAt { get; set; }
}
```

#### 3. File Metadata Model
```csharp
public class FileMetadata
{
    public string FilePath { get; set; } = string.Empty;
    public FileType Type { get; set; }
    public long FileSize { get; set; }
    public DateTime LastModified { get; set; }
    public uint Signature { get; set; }
    public Version ClientVersion { get; set; } = new();
    public int ItemCount { get; set; }
    public bool IsValid { get; set; }
    public string? ErrorMessage { get; set; }
}

public enum FileType
{
    OTB,
    DAT,
    SPR
}
```

### ViewModel Models

#### 1. Item ViewModel
```csharp
public partial class ItemViewModel : ObservableObject
{
    private readonly Item _item;
    private readonly IImageService _imageService;
    
    public ItemViewModel(Item item, IImageService imageService)
    {
        _item = item;
        _imageService = imageService;
        
        // Subscribe to item property changes
        _item.PropertyChanged += OnItemPropertyChanged;
        
        // Initialize commands
        RefreshThumbnailCommand = new AsyncRelayCommand(RefreshThumbnailAsync);
    }
    
    public string Name
    {
        get => _item.Name;
        set => _item.Name = value;
    }
    
    public ushort Id => _item.Id;
    
    public ServerItemType Type
    {
        get => _item.Type;
        set => _item.Type = value;
    }
    
    public BitmapSource? Thumbnail => _item.Thumbnail;
    
    public IAsyncRelayCommand RefreshThumbnailCommand { get; }
    
    private async Task RefreshThumbnailAsync()
    {
        if (_item.Sprites.Count > 0)
        {
            _item.Thumbnail = await _imageService.RenderItemAsync(_item);
        }
    }
    
    private void OnItemPropertyChanged(object? sender, PropertyChangedEventArgs e)
    {
        OnPropertyChanged(e.PropertyName);
    }
}
```

## Error Handling

### Structured Error Handling Strategy

#### 1. Global Exception Handler
```csharp
public class GlobalExceptionHandler
{
    private readonly ILogger<GlobalExceptionHandler> _logger;
    
    public GlobalExceptionHandler(ILogger<GlobalExceptionHandler> logger)
    {
        _logger = logger;
        
        // Handle unhandled exceptions
        Application.Current.DispatcherUnhandledException += OnDispatcherUnhandledException;
        AppDomain.CurrentDomain.UnhandledException += OnUnhandledException;
        TaskScheduler.UnobservedTaskException += OnUnobservedTaskException;
    }
    
    private void OnDispatcherUnhandledException(object sender, DispatcherUnhandledExceptionEventArgs e)
    {
        _logger.LogError(e.Exception, "Unhandled dispatcher exception occurred");
        
        // Show user-friendly error dialog
        ShowErrorDialog("An unexpected error occurred", e.Exception);
        
        e.Handled = true;
    }
    
    private void ShowErrorDialog(string message, Exception exception)
    {
        var errorDialog = new ErrorDialog
        {
            Title = "Error",
            Message = message,
            Details = exception.ToString()
        };
        
        errorDialog.ShowDialog();
    }
}
```

#### 2. Service-Level Error Handling
```csharp
public class FileService : IFileService
{
    private readonly ILogger<FileService> _logger;
    
    public async Task<IEnumerable<Item>> LoadItemsAsync(string filePath, CancellationToken cancellationToken = default)
    {
        try
        {
            _logger.LogInformation("Loading items from {FilePath}", filePath);
            
            // Validate file exists and is accessible
            if (!File.Exists(filePath))
            {
                throw new FileNotFoundException($"File not found: {filePath}");
            }
            
            // Load and parse file
            var items = await LoadItemsInternalAsync(filePath, cancellationToken);
            
            _logger.LogInformation("Successfully loaded {ItemCount} items from {FilePath}", items.Count(), filePath);
            return items;
        }
        catch (OperationCanceledException)
        {
            _logger.LogInformation("File loading cancelled for {FilePath}", filePath);
            throw;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to load items from {FilePath}", filePath);
            throw new FileLoadException($"Failed to load items from {filePath}", ex);
        }
    }
}
```

#### 3. User-Friendly Error Messages
```csharp
public static class ErrorMessages
{
    public static class FileOperations
    {
        public const string FileNotFound = "The specified file could not be found. Please check the file path and try again.";
        public const string FileCorrupted = "The file appears to be corrupted or in an unsupported format.";
        public const string AccessDenied = "Access to the file was denied. Please check file permissions.";
        public const string DiskFull = "There is not enough disk space to complete this operation.";
    }
    
    public static class PluginOperations
    {
        public const string PluginLoadFailed = "Failed to load plugin. The plugin may be incompatible or corrupted.";
        public const string PluginNotFound = "The specified plugin could not be found.";
        public const string PluginVersionMismatch = "Plugin version is not compatible with this version of ItemEditor.";
    }
}
```

## Testing Strategy

### Unit Testing Framework

#### 1. Service Testing
```csharp
[TestClass]
public class FileServiceTests
{
    private Mock<ILogger<FileService>> _mockLogger;
    private FileService _fileService;
    
    [TestInitialize]
    public void Setup()
    {
        _mockLogger = new Mock<ILogger<FileService>>();
        _fileService = new FileService(_mockLogger.Object);
    }
    
    [TestMethod]
    public async Task LoadItemsAsync_ValidOtbFile_ReturnsItems()
    {
        // Arrange
        var testFilePath = "test.otb";
        CreateTestOtbFile(testFilePath);
        
        // Act
        var items = await _fileService.LoadItemsAsync(testFilePath);
        
        // Assert
        Assert.IsNotNull(items);
        Assert.IsTrue(items.Any());
        
        // Cleanup
        File.Delete(testFilePath);
    }
    
    [TestMethod]
    public async Task LoadItemsAsync_FileNotFound_ThrowsFileNotFoundException()
    {
        // Arrange
        var nonExistentFile = "nonexistent.otb";
        
        // Act & Assert
        await Assert.ThrowsExceptionAsync<FileNotFoundException>(
            () => _fileService.LoadItemsAsync(nonExistentFile));
    }
}
```

#### 2. ViewModel Testing
```csharp
[TestClass]
public class MainViewModelTests
{
    private Mock<IFileService> _mockFileService;
    private Mock<IPluginService> _mockPluginService;
    private MainViewModel _viewModel;
    
    [TestInitialize]
    public void Setup()
    {
        _mockFileService = new Mock<IFileService>();
        _mockPluginService = new Mock<IPluginService>();
        _viewModel = new MainViewModel(_mockFileService.Object, _mockPluginService.Object);
    }
    
    [TestMethod]
    public async Task OpenFileCommand_ValidFile_LoadsItems()
    {
        // Arrange
        var testItems = new List<Item> { new Item { Id = 1, Name = "Test Item" } };
        _mockFileService.Setup(x => x.LoadItemsAsync(It.IsAny<string>(), It.IsAny<CancellationToken>()))
                       .ReturnsAsync(testItems);
        
        // Act
        await _viewModel.OpenFileCommand.ExecuteAsync("test.otb");
        
        // Assert
        Assert.AreEqual(1, _viewModel.Items.Count);
        Assert.AreEqual("Test Item", _viewModel.Items[0].Name);
    }
}
```

#### 3. Integration Testing
```csharp
[TestClass]
public class PluginIntegrationTests
{
    private IServiceProvider _serviceProvider;
    
    [TestInitialize]
    public void Setup()
    {
        var services = new ServiceCollection();
        services.AddSingleton<IPluginService, PluginService>();
        services.AddLogging();
        
        _serviceProvider = services.BuildServiceProvider();
    }
    
    [TestMethod]
    public async Task LoadPlugin_LegacyPlugin_MaintainsCompatibility()
    {
        // Arrange
        var pluginService = _serviceProvider.GetRequiredService<IPluginService>();
        var legacyPluginPath = "LegacyPlugin.dll";
        
        // Act
        var plugins = await pluginService.LoadPluginsAsync(Path.GetDirectoryName(legacyPluginPath));
        
        // Assert
        Assert.IsTrue(plugins.Any());
        var plugin = plugins.First();
        Assert.IsNotNull(plugin.SupportedClients);
        Assert.IsTrue(plugin.SupportedClients.Any());
    }
}
```

### Performance Testing

#### 1. File Loading Performance
```csharp
[TestMethod]
public async Task LoadLargeOtbFile_Performance_CompletesWithinTimeout()
{
    // Arrange
    var largeOtbFile = "large_test.otb";
    var stopwatch = Stopwatch.StartNew();
    
    // Act
    var items = await _fileService.LoadItemsAsync(largeOtbFile);
    stopwatch.Stop();
    
    // Assert
    Assert.IsTrue(stopwatch.ElapsedMilliseconds < 5000, "File loading took too long");
    Assert.IsTrue(items.Count() > 1000, "Expected large number of items");
}
```

#### 2. Memory Usage Testing
```csharp
[TestMethod]
public async Task LoadMultipleFiles_MemoryUsage_DoesNotExceedLimit()
{
    // Arrange
    var initialMemory = GC.GetTotalMemory(true);
    var files = new[] { "test1.otb", "test2.otb", "test3.otb" };
    
    // Act
    foreach (var file in files)
    {
        await _fileService.LoadItemsAsync(file);
    }
    
    var finalMemory = GC.GetTotalMemory(true);
    var memoryIncrease = finalMemory - initialMemory;
    
    // Assert
    Assert.IsTrue(memoryIncrease < 100 * 1024 * 1024, "Memory usage exceeded 100MB limit");
}
```

This comprehensive design document provides the foundation for implementing a modern, maintainable, and high-performance ItemEditor application while maintaining complete compatibility with the legacy version. The architecture leverages modern .NET 9 features, WPFUI for contemporary UI design, and established patterns for scalability and maintainability.