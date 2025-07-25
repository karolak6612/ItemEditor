using ItemEditor.Models;
// using PluginInterface.OTLib.FileFormats;

namespace ItemEditor.Services;

// Temporary stub types for compilation
public class OTBItem { }
public class OTBHeader { }
public class DATItem { }
public class SPRSprite { }

/// <summary>
/// Modern file service interface with async file operations and cancellation support
/// </summary>
public interface IFileService
{
    /// <summary>
    /// Loads items from a file asynchronously with progress reporting
    /// </summary>
    /// <param name="filePath">Path to the file to load</param>
    /// <param name="progress">Progress reporter for long-running operations</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Collection of loaded items</returns>
    Task<IEnumerable<Item>> LoadItemsAsync(string filePath, IProgress<FileReadProgress>? progress = null, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Saves items to a file asynchronously with progress reporting
    /// </summary>
    /// <param name="filePath">Path to the file to save</param>
    /// <param name="items">Items to save</param>
    /// <param name="progress">Progress reporter for long-running operations</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task representing the asynchronous operation</returns>
    Task SaveItemsAsync(string filePath, IEnumerable<Item> items, IProgress<FileWriteProgress>? progress = null, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Validates a file format with detailed validation results
    /// </summary>
    /// <param name="filePath">Path to the file to validate</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Validation result with detailed information</returns>
    Task<FileValidationResult> ValidateFileAsync(string filePath, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Gets comprehensive file metadata
    /// </summary>
    /// <param name="filePath">Path to the file</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>File metadata</returns>
    Task<FileMetadata> GetFileMetadataAsync(string filePath, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Loads OTB file with modern streaming support
    /// </summary>
    /// <param name="filePath">Path to the OTB file</param>
    /// <param name="progress">Progress reporter</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>OTB items</returns>
    Task<IEnumerable<OTBItem>> LoadOTBAsync(string filePath, IProgress<FileReadProgress>? progress = null, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Saves OTB file with modern streaming support
    /// </summary>
    /// <param name="filePath">Path to save the OTB file</param>
    /// <param name="items">OTB items to save</param>
    /// <param name="header">OTB header</param>
    /// <param name="progress">Progress reporter</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task representing the async operation</returns>
    Task SaveOTBAsync(string filePath, IEnumerable<OTBItem> items, OTBHeader header, IProgress<FileWriteProgress>? progress = null, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Loads DAT file with streaming support
    /// </summary>
    /// <param name="filePath">Path to the DAT file</param>
    /// <param name="progress">Progress reporter</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>DAT items</returns>
    Task<IEnumerable<DATItem>> LoadDATAsync(string filePath, IProgress<FileReadProgress>? progress = null, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Loads SPR file with streaming support
    /// </summary>
    /// <param name="filePath">Path to the SPR file</param>
    /// <param name="spriteIds">Specific sprite IDs to load (null for all)</param>
    /// <param name="progress">Progress reporter</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>SPR sprites</returns>
    Task<Dictionary<ushort, SPRSprite>> LoadSPRAsync(string filePath, IEnumerable<ushort>? spriteIds = null, IProgress<FileReadProgress>? progress = null, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Gets supported file formats
    /// </summary>
    /// <returns>Array of supported file extensions</returns>
    string[] GetSupportedFormats();
    
    /// <summary>
    /// Determines file type from file path or content
    /// </summary>
    /// <param name="filePath">Path to the file</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Detected file type</returns>
    Task<FileType> DetectFileTypeAsync(string filePath, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Closes all open files and releases resources
    /// </summary>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task representing the asynchronous operation</returns>
    Task CloseAllFilesAsync(CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Event raised when file operation progress changes
    /// </summary>
    event EventHandler<FileOperationProgressEventArgs>? ProgressChanged;
    
    /// <summary>
    /// Event raised when a file operation completes
    /// </summary>
    event EventHandler<FileOperationCompletedEventArgs>? OperationCompleted;
    
    /// <summary>
    /// Event raised when a file operation encounters an error
    /// </summary>
    event EventHandler<FileOperationErrorEventArgs>? OperationError;
}

/// <summary>
/// File validation result with detailed information
/// </summary>
public class FileValidationResult
{
    /// <summary>
    /// Whether the file is valid
    /// </summary>
    public bool IsValid { get; set; }
    
    /// <summary>
    /// Validation message
    /// </summary>
    public string Message { get; set; } = string.Empty;
    
    /// <summary>
    /// Validation warnings
    /// </summary>
    public List<string> Warnings { get; set; } = new();
    
    /// <summary>
    /// Validation errors
    /// </summary>
    public List<string> Errors { get; set; } = new();
    
    /// <summary>
    /// Detected file type
    /// </summary>
    public FileType FileType { get; set; }
    
    /// <summary>
    /// File version if applicable
    /// </summary>
    public uint? FileVersion { get; set; }
    
    /// <summary>
    /// Client version if applicable
    /// </summary>
    public Version? ClientVersion { get; set; }
    
    /// <summary>
    /// Number of items in the file
    /// </summary>
    public int ItemCount { get; set; }
    
    /// <summary>
    /// Number of sprites in the file (for SPR files)
    /// </summary>
    public int SpriteCount { get; set; }
    
    /// <summary>
    /// File checksum for integrity verification
    /// </summary>
    public string FileChecksum { get; set; } = string.Empty;
    
    /// <summary>
    /// Validation timestamp
    /// </summary>
    public DateTime ValidationTimestamp { get; set; } = DateTime.UtcNow;
    
    /// <summary>
    /// Time taken for validation
    /// </summary>
    public TimeSpan ValidationDuration { get; set; }
}

/// <summary>
/// File operation progress event arguments
/// </summary>
public class FileOperationProgressEventArgs : EventArgs
{
    /// <summary>
    /// File path being processed
    /// </summary>
    public string FilePath { get; set; } = string.Empty;
    
    /// <summary>
    /// Operation type
    /// </summary>
    public FileOperationType OperationType { get; set; }
    
    /// <summary>
    /// Progress percentage (0-100)
    /// </summary>
    public double ProgressPercentage { get; set; }
    
    /// <summary>
    /// Current operation description
    /// </summary>
    public string CurrentOperation { get; set; } = string.Empty;
}

/// <summary>
/// File operation completed event arguments
/// </summary>
public class FileOperationCompletedEventArgs : EventArgs
{
    /// <summary>
    /// File path that was processed
    /// </summary>
    public string FilePath { get; set; } = string.Empty;
    
    /// <summary>
    /// Operation type
    /// </summary>
    public FileOperationType OperationType { get; set; }
    
    /// <summary>
    /// Operation duration
    /// </summary>
    public TimeSpan Duration { get; set; }
    
    /// <summary>
    /// Number of items processed
    /// </summary>
    public int ItemsProcessed { get; set; }
}

/// <summary>
/// File operation error event arguments
/// </summary>
public class FileOperationErrorEventArgs : EventArgs
{
    /// <summary>
    /// File path that caused the error
    /// </summary>
    public string FilePath { get; set; } = string.Empty;
    
    /// <summary>
    /// Operation type
    /// </summary>
    public FileOperationType OperationType { get; set; }
    
    /// <summary>
    /// Exception that occurred
    /// </summary>
    public Exception Exception { get; set; } = new();
    
    /// <summary>
    /// Whether the error was handled
    /// </summary>
    public bool Handled { get; set; }
}

/// <summary>
/// File operation types
/// </summary>
public enum FileOperationType
{
    /// <summary>
    /// Loading file
    /// </summary>
    Load,
    
    /// <summary>
    /// Saving file
    /// </summary>
    Save,
    
    /// <summary>
    /// Validating file
    /// </summary>
    Validate,
    
    /// <summary>
    /// Getting metadata
    /// </summary>
    GetMetadata
}

/// <summary>
/// Supported file types
/// </summary>
public enum FileType
{
    /// <summary>
    /// Unknown file type
    /// </summary>
    Unknown,
    
    /// <summary>
    /// OpenTibia Binary file
    /// </summary>
    OTB,
    
    /// <summary>
    /// Data file
    /// </summary>
    DAT,
    
    /// <summary>
    /// Sprite file
    /// </summary>
    SPR
}

/// <summary>
/// File read progress information
/// </summary>
public class FileReadProgress
{
    /// <summary>
    /// Current progress percentage (0-100)
    /// </summary>
    public double ProgressPercentage { get; set; }
    
    /// <summary>
    /// Current operation description
    /// </summary>
    public string CurrentOperation { get; set; } = string.Empty;
    
    /// <summary>
    /// Number of items processed so far
    /// </summary>
    public int ItemsProcessed { get; set; }
    
    /// <summary>
    /// Total number of items to process
    /// </summary>
    public int TotalItems { get; set; }
    
    /// <summary>
    /// Bytes processed so far
    /// </summary>
    public long BytesProcessed { get; set; }
    
    /// <summary>
    /// Total bytes to process
    /// </summary>
    public long TotalBytes { get; set; }
}

/// <summary>
/// File write progress information
/// </summary>
public class FileWriteProgress
{
    /// <summary>
    /// Current progress percentage (0-100)
    /// </summary>
    public double ProgressPercentage { get; set; }
    
    /// <summary>
    /// Current operation description
    /// </summary>
    public string CurrentOperation { get; set; } = string.Empty;
    
    /// <summary>
    /// Number of items written so far
    /// </summary>
    public int ItemsWritten { get; set; }
    
    /// <summary>
    /// Total number of items to write
    /// </summary>
    public int TotalItems { get; set; }
    
    /// <summary>
    /// Bytes written so far
    /// </summary>
    public long BytesWritten { get; set; }
    
    /// <summary>
    /// Total bytes to write
    /// </summary>
    public long TotalBytes { get; set; }
}