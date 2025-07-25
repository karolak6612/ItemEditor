using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Windows.Input;
using System.Windows.Threading;
using ItemEditor.Services;
using Microsoft.Extensions.Logging;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;

namespace ItemEditor.ViewModels;

/// <summary>
/// View model for undo/redo status control
/// </summary>
public partial class UndoRedoStatusViewModel : ObservableObject
{
    private readonly IUndoRedoService _undoRedoService;
    private readonly ILogger<UndoRedoStatusViewModel> _logger;
    private readonly DispatcherTimer _hideTimer;
    
    [ObservableProperty]
    private bool _isVisible;
    
    [ObservableProperty]
    private string _statusMessage = string.Empty;
    
    [ObservableProperty]
    private bool _showUndoRedoInfo = true;
    
    [ObservableProperty]
    private bool _showFloatingButtons;
    
    [ObservableProperty]
    private bool _showHistoryPanel;
    
    [ObservableProperty]
    private bool _canUndo;
    
    [ObservableProperty]
    private bool _canRedo;
    
    [ObservableProperty]
    private string _undoTooltip = "Undo";
    
    [ObservableProperty]
    private string _redoTooltip = "Redo";
    
    [ObservableProperty]
    private int _undoStackSize;
    
    [ObservableProperty]
    private int _redoStackSize;
    
    [ObservableProperty]
    private int _totalCommands;
    
    [ObservableProperty]
    private ObservableCollection<CommandHistoryItem> _commandHistory = new();
    
    public UndoRedoStatusViewModel(IUndoRedoService undoRedoService, ILogger<UndoRedoStatusViewModel> logger)
    {
        _undoRedoService = undoRedoService ?? throw new ArgumentNullException(nameof(undoRedoService));
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
        
        // Initialize timer for auto-hide
        _hideTimer = new DispatcherTimer
        {
            Interval = TimeSpan.FromSeconds(3)
        };
        _hideTimer.Tick += OnHideTimerTick;
        
        // Initialize commands
        UndoCommand = new AsyncRelayCommand(UndoAsync, () => CanUndo);
        RedoCommand = new AsyncRelayCommand(RedoAsync, () => CanRedo);
        CloseHistoryCommand = new RelayCommand(CloseHistory);
        ClearHistoryCommand = new RelayCommand(ClearHistory);
        JumpToCommand = new RelayCommand<CommandHistoryItem>(JumpToCommand);
        
        // Subscribe to undo/redo service events
        _undoRedoService.StateChanged += OnUndoRedoStateChanged;
        _undoRedoService.CommandExecuted += OnCommandExecuted;
        _undoRedoService.CommandUndone += OnCommandUndone;
        _undoRedoService.CommandRedone += OnCommandRedone;
        
        // Initialize state
        UpdateState();
        
        _logger.LogDebug("UndoRedoStatusViewModel initialized");
    }
    
    #region Commands
    
    public IAsyncRelayCommand UndoCommand { get; }
    public IAsyncRelayCommand RedoCommand { get; }
    public IRelayCommand CloseHistoryCommand { get; }
    public IRelayCommand ClearHistoryCommand { get; }
    public IRelayCommand<CommandHistoryItem> JumpToCommand { get; }
    
    #endregion
    
    #region Events
    
    /// <summary>
    /// Event raised when status changes
    /// </summary>
    public event EventHandler<StatusChangedEventArgs>? StatusChanged;
    
    #endregion
    
    #region Public Methods
    
    /// <summary>
    /// Shows status with a message
    /// </summary>
    /// <param name="message">Status message</param>
    /// <param name="operationType">Operation type</param>
    /// <param name="success">Whether operation was successful</param>
    public void ShowStatus(string message, UndoRedoOperationType operationType, bool success = true)
    {
        try
        {
            StatusMessage = message;
            IsVisible = true;
            
            // Restart hide timer
            _hideTimer.Stop();
            _hideTimer.Start();
            
            // Raise status changed event
            StatusChanged?.Invoke(this, new StatusChangedEventArgs(operationType, success, message));
            
            _logger.LogDebug("Showing status: {Message} ({OperationType}, Success: {Success})", 
                message, operationType, success);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error showing status");
        }
    }
    
    /// <summary>
    /// Hides the status display
    /// </summary>
    public void HideStatus()
    {
        try
        {
            IsVisible = false;
            _hideTimer.Stop();
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error hiding status");
        }
    }
    
    /// <summary>
    /// Updates the command history display
    /// </summary>
    public void UpdateCommandHistory()
    {
        try
        {
            CommandHistory.Clear();
            
            // This would need to be implemented based on the actual undo/redo service
            // For now, create placeholder items
            for (int i = 0; i < UndoStackSize; i++)
            {
                CommandHistory.Add(new CommandHistoryItem
                {
                    Description = $"Command {i + 1}",
                    Timestamp = DateTime.Now.AddMinutes(-i),
                    IsExecuted = true,
                    IsCurrentPosition = i == 0,
                    CommandType = "Execute"
                });
            }
            
            TotalCommands = CommandHistory.Count;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error updating command history");
        }
    }
    
    /// <summary>
    /// Cleans up resources
    /// </summary>
    public void Cleanup()
    {
        try
        {
            _hideTimer.Stop();
            _hideTimer.Tick -= OnHideTimerTick;
            
            _undoRedoService.StateChanged -= OnUndoRedoStateChanged;
            _undoRedoService.CommandExecuted -= OnCommandExecuted;
            _undoRedoService.CommandUndone -= OnCommandUndone;
            _undoRedoService.CommandRedone -= OnCommandRedone;
            
            _logger.LogDebug("UndoRedoStatusViewModel cleaned up");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error cleaning up UndoRedoStatusViewModel");
        }
    }
    
    #endregion
    
    #region Private Methods
    
    private void OnHideTimerTick(object? sender, EventArgs e)
    {
        HideStatus();
    }
    
    private void OnUndoRedoStateChanged(object? sender, UndoRedoStateChangedEventArgs e)
    {
        try
        {
            UpdateState();
            UpdateCommandHistory();
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error handling undo/redo state change");
        }
    }
    
    private void OnCommandExecuted(object? sender, CommandExecutedEventArgs e)
    {
        try
        {
            var message = e.Success 
                ? $"Executed: {e.Command.Description}"
                : $"Failed to execute: {e.Command.Description}";
            
            ShowStatus(message, UndoRedoOperationType.Execute, e.Success);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error handling command executed event");
        }
    }
    
    private void OnCommandUndone(object? sender, CommandUndoneEventArgs e)
    {
        try
        {
            var message = e.Success 
                ? $"Undone: {e.Command.Description}"
                : $"Failed to undo: {e.Command.Description}";
            
            ShowStatus(message, UndoRedoOperationType.Undo, e.Success);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error handling command undone event");
        }
    }
    
    private void OnCommandRedone(object? sender, CommandRedoneEventArgs e)
    {
        try
        {
            var message = e.Success 
                ? $"Redone: {e.Command.Description}"
                : $"Failed to redo: {e.Command.Description}";
            
            ShowStatus(message, UndoRedoOperationType.Redo, e.Success);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error handling command redone event");
        }
    }
    
    private void UpdateState()
    {
        try
        {
            CanUndo = _undoRedoService.CanUndo;
            CanRedo = _undoRedoService.CanRedo;
            UndoStackSize = _undoRedoService.UndoStackSize;
            RedoStackSize = _undoRedoService.RedoStackSize;
            
            // Update tooltips
            UndoTooltip = _undoRedoService.UndoDescription != null 
                ? $"Undo: {_undoRedoService.UndoDescription}"
                : "Undo";
            
            RedoTooltip = _undoRedoService.RedoDescription != null 
                ? $"Redo: {_undoRedoService.RedoDescription}"
                : "Redo";
            
            // Update command can execute
            ((AsyncRelayCommand)UndoCommand).NotifyCanExecuteChanged();
            ((AsyncRelayCommand)RedoCommand).NotifyCanExecuteChanged();
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error updating state");
        }
    }
    
    #endregion
    
    #region Command Implementations
    
    private async Task UndoAsync()
    {
        try
        {
            await _undoRedoService.UndoAsync();
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error executing undo command");
        }
    }
    
    private async Task RedoAsync()
    {
        try
        {
            await _undoRedoService.RedoAsync();
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error executing redo command");
        }
    }
    
    private void CloseHistory()
    {
        ShowHistoryPanel = false;
    }
    
    private void ClearHistory()
    {
        try
        {
            _undoRedoService.ClearHistory();
            ShowStatus("Command history cleared", UndoRedoOperationType.Execute);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error clearing history");
        }
    }
    
    private void JumpToCommand(CommandHistoryItem? item)
    {
        try
        {
            if (item == null) return;
            
            // This would implement jumping to a specific command in history
            _logger.LogDebug("Jump to command requested: {Description}", item.Description);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error jumping to command");
        }
    }
    
    #endregion
}

/// <summary>
/// Command history item for display
/// </summary>
public class CommandHistoryItem
{
    public string Description { get; set; } = string.Empty;
    public DateTime Timestamp { get; set; }
    public bool IsExecuted { get; set; }
    public bool IsCurrentPosition { get; set; }
    public string CommandType { get; set; } = string.Empty;
}

/// <summary>
/// Undo/redo operation types
/// </summary>
public enum UndoRedoOperationType
{
    Execute,
    Undo,
    Redo
}

/// <summary>
/// Event arguments for status changes
/// </summary>
public class StatusChangedEventArgs : EventArgs
{
    public UndoRedoOperationType OperationType { get; }
    public bool Success { get; }
    public string Message { get; }
    
    public StatusChangedEventArgs(UndoRedoOperationType operationType, bool success, string message)
    {
        OperationType = operationType;
        Success = success;
        Message = message ?? throw new ArgumentNullException(nameof(message));
    }
}