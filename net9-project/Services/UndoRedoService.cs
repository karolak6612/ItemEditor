using System.Collections.Concurrent;
using Microsoft.Extensions.Logging;

namespace ItemEditor.Services;

/// <summary>
/// Service for managing undo/redo operations
/// </summary>
public class UndoRedoService : IUndoRedoService
{
    private readonly ILogger<UndoRedoService> _logger;
    private readonly Stack<IUndoableCommand> _undoStack = new();
    private readonly Stack<IUndoableCommand> _redoStack = new();
    private readonly object _lockObject = new();
    private int _maxHistorySize = 100;
    
    public UndoRedoService(ILogger<UndoRedoService> logger)
    {
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
        _logger.LogDebug("UndoRedoService initialized");
    }
    
    /// <inheritdoc />
    public async Task ExecuteCommandAsync(IUndoableCommand command)
    {
        if (command == null)
            throw new ArgumentNullException(nameof(command));
        
        try
        {
            _logger.LogDebug("Executing command: {Description}", command.Description);
            
            // Execute the command
            await command.ExecuteAsync();
            
            lock (_lockObject)
            {
                // Add to undo stack
                _undoStack.Push(command);
                
                // Clear redo stack since we're executing a new command
                _redoStack.Clear();
                
                // Maintain history size limit
                while (_undoStack.Count > _maxHistorySize)
                {
                    var oldestCommand = _undoStack.ToArray().Last();
                    var tempStack = new Stack<IUndoableCommand>();
                    
                    // Remove the oldest command
                    while (_undoStack.Count > 0)
                    {
                        var cmd = _undoStack.Pop();
                        if (cmd != oldestCommand)
                        {
                            tempStack.Push(cmd);
                        }
                    }
                    
                    // Restore the stack without the oldest command
                    while (tempStack.Count > 0)
                    {
                        _undoStack.Push(tempStack.Pop());
                    }
                }
            }
            
            // Raise events
            CommandExecuted?.Invoke(this, new CommandExecutedEventArgs(command, true));
            RaiseStateChanged();
            
            _logger.LogDebug("Command executed successfully: {Description}", command.Description);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error executing command: {Description}", command.Description);
            CommandExecuted?.Invoke(this, new CommandExecutedEventArgs(command, false, ex));
            throw;
        }
    }
    
    /// <inheritdoc />
    public async Task<bool> UndoAsync()
    {
        IUndoableCommand? command = null;
        
        lock (_lockObject)
        {
            if (_undoStack.Count == 0)
            {
                _logger.LogDebug("No commands to undo");
                return false;
            }
            
            command = _undoStack.Pop();
        }
        
        try
        {
            _logger.LogDebug("Undoing command: {Description}", command.Description);
            
            if (!command.CanUndo)
            {
                _logger.LogWarning("Command cannot be undone: {Description}", command.Description);
                
                // Put the command back on the stack
                lock (_lockObject)
                {
                    _undoStack.Push(command);
                }
                return false;
            }
            
            // Undo the command
            await command.UndoAsync();
            
            lock (_lockObject)
            {
                // Add to redo stack
                _redoStack.Push(command);
            }
            
            // Raise events
            CommandUndone?.Invoke(this, new CommandUndoneEventArgs(command, true));
            RaiseStateChanged();
            
            _logger.LogDebug("Command undone successfully: {Description}", command.Description);
            return true;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error undoing command: {Description}", command.Description);
            
            // Put the command back on the undo stack
            lock (_lockObject)
            {
                _undoStack.Push(command);
            }
            
            CommandUndone?.Invoke(this, new CommandUndoneEventArgs(command, false, ex));
            RaiseStateChanged();
            return false;
        }
    }
    
    /// <inheritdoc />
    public async Task<bool> RedoAsync()
    {
        IUndoableCommand? command = null;
        
        lock (_lockObject)
        {
            if (_redoStack.Count == 0)
            {
                _logger.LogDebug("No commands to redo");
                return false;
            }
            
            command = _redoStack.Pop();
        }
        
        try
        {
            _logger.LogDebug("Redoing command: {Description}", command.Description);
            
            // Re-execute the command
            await command.ExecuteAsync();
            
            lock (_lockObject)
            {
                // Add back to undo stack
                _undoStack.Push(command);
            }
            
            // Raise events
            CommandRedone?.Invoke(this, new CommandRedoneEventArgs(command, true));
            RaiseStateChanged();
            
            _logger.LogDebug("Command redone successfully: {Description}", command.Description);
            return true;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error redoing command: {Description}", command.Description);
            
            // Put the command back on the redo stack
            lock (_lockObject)
            {
                _redoStack.Push(command);
            }
            
            CommandRedone?.Invoke(this, new CommandRedoneEventArgs(command, false, ex));
            RaiseStateChanged();
            return false;
        }
    }
    
    /// <inheritdoc />
    public void ClearHistory()
    {
        lock (_lockObject)
        {
            var undoCount = _undoStack.Count;
            var redoCount = _redoStack.Count;
            
            _undoStack.Clear();
            _redoStack.Clear();
            
            _logger.LogDebug("Cleared undo/redo history: {UndoCount} undo, {RedoCount} redo", undoCount, redoCount);
        }
        
        RaiseStateChanged();
    }
    
    /// <inheritdoc />
    public bool CanUndo
    {
        get
        {
            lock (_lockObject)
            {
                return _undoStack.Count > 0;
            }
        }
    }
    
    /// <inheritdoc />
    public bool CanRedo
    {
        get
        {
            lock (_lockObject)
            {
                return _redoStack.Count > 0;
            }
        }
    }
    
    /// <inheritdoc />
    public string? UndoDescription
    {
        get
        {
            lock (_lockObject)
            {
                return _undoStack.Count > 0 ? _undoStack.Peek().Description : null;
            }
        }
    }
    
    /// <inheritdoc />
    public string? RedoDescription
    {
        get
        {
            lock (_lockObject)
            {
                return _redoStack.Count > 0 ? _redoStack.Peek().Description : null;
            }
        }
    }
    
    /// <inheritdoc />
    public int UndoStackSize
    {
        get
        {
            lock (_lockObject)
            {
                return _undoStack.Count;
            }
        }
    }
    
    /// <inheritdoc />
    public int RedoStackSize
    {
        get
        {
            lock (_lockObject)
            {
                return _redoStack.Count;
            }
        }
    }
    
    /// <inheritdoc />
    public int MaxHistorySize
    {
        get => _maxHistorySize;
        set
        {
            if (value <= 0)
                throw new ArgumentOutOfRangeException(nameof(value), "Max history size must be greater than 0");
            
            _maxHistorySize = value;
            
            // Trim existing history if needed
            lock (_lockObject)
            {
                while (_undoStack.Count > _maxHistorySize)
                {
                    var commands = _undoStack.ToArray();
                    _undoStack.Clear();
                    
                    // Keep the most recent commands
                    for (int i = 0; i < _maxHistorySize; i++)
                    {
                        _undoStack.Push(commands[i]);
                    }
                }
            }
            
            _logger.LogDebug("Max history size set to: {MaxHistorySize}", _maxHistorySize);
        }
    }
    
    /// <inheritdoc />
    public ICompositeCommandBuilder CreateCompositeCommand(string description)
    {
        return new CompositeCommandBuilder(description, _logger);
    }
    
    /// <inheritdoc />
    public event EventHandler<UndoRedoStateChangedEventArgs>? StateChanged;
    
    /// <inheritdoc />
    public event EventHandler<CommandExecutedEventArgs>? CommandExecuted;
    
    /// <inheritdoc />
    public event EventHandler<CommandUndoneEventArgs>? CommandUndone;
    
    /// <inheritdoc />
    public event EventHandler<CommandRedoneEventArgs>? CommandRedone;
    
    #region Private Methods
    
    private void RaiseStateChanged()
    {
        try
        {
            var args = new UndoRedoStateChangedEventArgs(
                CanUndo, CanRedo, UndoDescription, RedoDescription, UndoStackSize, RedoStackSize);
            
            StateChanged?.Invoke(this, args);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error raising state changed event");
        }
    }
    
    #endregion
}

/// <summary>
/// Builder for composite commands
/// </summary>
internal class CompositeCommandBuilder : ICompositeCommandBuilder
{
    private readonly string _description;
    private readonly ILogger _logger;
    private readonly List<IUndoableCommand> _commands = new();
    
    public CompositeCommandBuilder(string description, ILogger logger)
    {
        _description = description ?? throw new ArgumentNullException(nameof(description));
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
    }
    
    /// <inheritdoc />
    public ICompositeCommandBuilder AddCommand(IUndoableCommand command)
    {
        if (command == null)
            throw new ArgumentNullException(nameof(command));
        
        _commands.Add(command);
        return this;
    }
    
    /// <inheritdoc />
    public IUndoableCommand Build()
    {
        return new CompositeCommand(_description, _commands.ToList(), _logger);
    }
}

/// <summary>
/// Composite command that groups multiple operations
/// </summary>
internal class CompositeCommand : UndoableCommandBase
{
    private readonly List<IUndoableCommand> _commands;
    private readonly ILogger _logger;
    private readonly List<IUndoableCommand> _executedCommands = new();
    
    public CompositeCommand(string description, List<IUndoableCommand> commands, ILogger logger)
        : base(description)
    {
        _commands = commands ?? throw new ArgumentNullException(nameof(commands));
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
        
        if (_commands.Count == 0)
            throw new ArgumentException("Composite command must contain at least one command", nameof(commands));
    }
    
    /// <inheritdoc />
    public override async Task ExecuteAsync()
    {
        _executedCommands.Clear();
        
        try
        {
            foreach (var command in _commands)
            {
                await command.ExecuteAsync();
                _executedCommands.Add(command);
            }
            
            _logger.LogDebug("Composite command executed: {Description} ({CommandCount} commands)", 
                Description, _commands.Count);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error executing composite command: {Description}", Description);
            
            // Undo any commands that were successfully executed
            await UndoExecutedCommandsAsync();
            throw;
        }
    }
    
    /// <inheritdoc />
    public override async Task UndoAsync()
    {
        try
        {
            // Undo in reverse order
            for (int i = _executedCommands.Count - 1; i >= 0; i--)
            {
                var command = _executedCommands[i];
                if (command.CanUndo)
                {
                    await command.UndoAsync();
                }
            }
            
            _logger.LogDebug("Composite command undone: {Description} ({CommandCount} commands)", 
                Description, _executedCommands.Count);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error undoing composite command: {Description}", Description);
            throw;
        }
    }
    
    /// <inheritdoc />
    public override bool CanUndo => _executedCommands.All(c => c.CanUndo);
    
    private async Task UndoExecutedCommandsAsync()
    {
        try
        {
            // Undo in reverse order
            for (int i = _executedCommands.Count - 1; i >= 0; i--)
            {
                var command = _executedCommands[i];
                if (command.CanUndo)
                {
                    await command.UndoAsync();
                }
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error undoing executed commands during composite command failure");
        }
    }
}