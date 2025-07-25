namespace ItemEditor.Services;

/// <summary>
/// Interface for undo/redo functionality
/// </summary>
public interface IUndoRedoService
{
    /// <summary>
    /// Executes a command and adds it to the undo stack
    /// </summary>
    /// <param name="command">Command to execute</param>
    Task ExecuteCommandAsync(IUndoableCommand command);
    
    /// <summary>
    /// Undoes the last command
    /// </summary>
    /// <returns>True if undo was successful</returns>
    Task<bool> UndoAsync();
    
    /// <summary>
    /// Redoes the last undone command
    /// </summary>
    /// <returns>True if redo was successful</returns>
    Task<bool> RedoAsync();
    
    /// <summary>
    /// Clears the undo/redo history
    /// </summary>
    void ClearHistory();
    
    /// <summary>
    /// Gets whether undo is available
    /// </summary>
    bool CanUndo { get; }
    
    /// <summary>
    /// Gets whether redo is available
    /// </summary>
    bool CanRedo { get; }
    
    /// <summary>
    /// Gets the description of the next undo operation
    /// </summary>
    string? UndoDescription { get; }
    
    /// <summary>
    /// Gets the description of the next redo operation
    /// </summary>
    string? RedoDescription { get; }
    
    /// <summary>
    /// Gets the current undo stack size
    /// </summary>
    int UndoStackSize { get; }
    
    /// <summary>
    /// Gets the current redo stack size
    /// </summary>
    int RedoStackSize { get; }
    
    /// <summary>
    /// Gets or sets the maximum number of operations to keep in history
    /// </summary>
    int MaxHistorySize { get; set; }
    
    /// <summary>
    /// Creates a composite command that groups multiple operations
    /// </summary>
    /// <param name="description">Description of the composite operation</param>
    /// <returns>Composite command builder</returns>
    ICompositeCommandBuilder CreateCompositeCommand(string description);
    
    /// <summary>
    /// Event raised when the undo/redo state changes
    /// </summary>
    event EventHandler<UndoRedoStateChangedEventArgs>? StateChanged;
    
    /// <summary>
    /// Event raised when a command is executed
    /// </summary>
    event EventHandler<CommandExecutedEventArgs>? CommandExecuted;
    
    /// <summary>
    /// Event raised when a command is undone
    /// </summary>
    event EventHandler<CommandUndoneEventArgs>? CommandUndone;
    
    /// <summary>
    /// Event raised when a command is redone
    /// </summary>
    event EventHandler<CommandRedoneEventArgs>? CommandRedone;
}

/// <summary>
/// Interface for undoable commands
/// </summary>
public interface IUndoableCommand
{
    /// <summary>
    /// Description of the command for UI display
    /// </summary>
    string Description { get; }
    
    /// <summary>
    /// Executes the command
    /// </summary>
    Task ExecuteAsync();
    
    /// <summary>
    /// Undoes the command
    /// </summary>
    Task UndoAsync();
    
    /// <summary>
    /// Gets whether the command can be undone
    /// </summary>
    bool CanUndo { get; }
    
    /// <summary>
    /// Gets the timestamp when the command was executed
    /// </summary>
    DateTime ExecutedAt { get; }
    
    /// <summary>
    /// Gets any additional metadata about the command
    /// </summary>
    Dictionary<string, object> Metadata { get; }
}

/// <summary>
/// Interface for composite command builder
/// </summary>
public interface ICompositeCommandBuilder
{
    /// <summary>
    /// Adds a command to the composite
    /// </summary>
    /// <param name="command">Command to add</param>
    /// <returns>Builder for chaining</returns>
    ICompositeCommandBuilder AddCommand(IUndoableCommand command);
    
    /// <summary>
    /// Builds and returns the composite command
    /// </summary>
    /// <returns>Composite command</returns>
    IUndoableCommand Build();
}

/// <summary>
/// Base class for undoable commands
/// </summary>
public abstract class UndoableCommandBase : IUndoableCommand
{
    protected UndoableCommandBase(string description)
    {
        Description = description ?? throw new ArgumentNullException(nameof(description));
        ExecutedAt = DateTime.Now;
        Metadata = new Dictionary<string, object>();
    }
    
    /// <inheritdoc />
    public string Description { get; }
    
    /// <inheritdoc />
    public DateTime ExecutedAt { get; }
    
    /// <inheritdoc />
    public Dictionary<string, object> Metadata { get; }
    
    /// <inheritdoc />
    public virtual bool CanUndo => true;
    
    /// <inheritdoc />
    public abstract Task ExecuteAsync();
    
    /// <inheritdoc />
    public abstract Task UndoAsync();
}

/// <summary>
/// Event arguments for undo/redo state changes
/// </summary>
public class UndoRedoStateChangedEventArgs : EventArgs
{
    public bool CanUndo { get; }
    public bool CanRedo { get; }
    public string? UndoDescription { get; }
    public string? RedoDescription { get; }
    public int UndoStackSize { get; }
    public int RedoStackSize { get; }
    
    public UndoRedoStateChangedEventArgs(bool canUndo, bool canRedo, string? undoDescription, 
        string? redoDescription, int undoStackSize, int redoStackSize)
    {
        CanUndo = canUndo;
        CanRedo = canRedo;
        UndoDescription = undoDescription;
        RedoDescription = redoDescription;
        UndoStackSize = undoStackSize;
        RedoStackSize = redoStackSize;
    }
}

/// <summary>
/// Event arguments for command executed events
/// </summary>
public class CommandExecutedEventArgs : EventArgs
{
    public IUndoableCommand Command { get; }
    public bool Success { get; }
    public Exception? Exception { get; }
    
    public CommandExecutedEventArgs(IUndoableCommand command, bool success, Exception? exception = null)
    {
        Command = command ?? throw new ArgumentNullException(nameof(command));
        Success = success;
        Exception = exception;
    }
}

/// <summary>
/// Event arguments for command undone events
/// </summary>
public class CommandUndoneEventArgs : EventArgs
{
    public IUndoableCommand Command { get; }
    public bool Success { get; }
    public Exception? Exception { get; }
    
    public CommandUndoneEventArgs(IUndoableCommand command, bool success, Exception? exception = null)
    {
        Command = command ?? throw new ArgumentNullException(nameof(command));
        Success = success;
        Exception = exception;
    }
}

/// <summary>
/// Event arguments for command redone events
/// </summary>
public class CommandRedoneEventArgs : EventArgs
{
    public IUndoableCommand Command { get; }
    public bool Success { get; }
    public Exception? Exception { get; }
    
    public CommandRedoneEventArgs(IUndoableCommand command, bool success, Exception? exception = null)
    {
        Command = command ?? throw new ArgumentNullException(nameof(command));
        Success = success;
        Exception = exception;
    }
}