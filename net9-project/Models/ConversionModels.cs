namespace ItemEditor.Models;

/// <summary>
/// Conversion options for file format conversion
/// </summary>
public class ConversionOptions
{
    /// <summary>
    /// Whether to verify output file integrity after conversion
    /// </summary>
    public bool VerifyIntegrity { get; set; } = true;

    /// <summary>
    /// Whether to preserve original file timestamps
    /// </summary>
    public bool PreserveTimestamps { get; set; } = true;

    /// <summary>
    /// Whether to create backup of original file
    /// </summary>
    public bool CreateBackup { get; set; } = false;

    /// <summary>
    /// Maximum number of concurrent conversions for batch operations
    /// </summary>
    public int MaxConcurrency { get; set; } = Environment.ProcessorCount;

    /// <summary>
    /// Compression level for output files (if applicable)
    /// </summary>
    public CompressionLevel CompressionLevel { get; set; } = CompressionLevel.Default;

    /// <summary>
    /// Custom conversion parameters
    /// </summary>
    public Dictionary<string, object> CustomParameters { get; set; } = new();
}

/// <summary>
/// Export options for data export operations
/// </summary>
public class ExportOptions
{
    /// <summary>
    /// Whether to format output for readability
    /// </summary>
    public bool PrettyFormat { get; set; } = true;

    /// <summary>
    /// Whether to include metadata in export
    /// </summary>
    public bool IncludeMetadata { get; set; } = true;

    /// <summary>
    /// Whether to verify output file integrity
    /// </summary>
    public bool VerifyIntegrity { get; set; } = true;

    /// <summary>
    /// Fields to include in export (null for all)
    /// </summary>
    public List<string>? IncludeFields { get; set; }

    /// <summary>
    /// Fields to exclude from export
    /// </summary>
    public List<string> ExcludeFields { get; set; } = new();

    /// <summary>
    /// Custom export parameters
    /// </summary>
    public Dictionary<string, object> CustomParameters { get; set; } = new();
}

/// <summary>
/// Migration options for version migration
/// </summary>
public class MigrationOptions
{
    /// <summary>
    /// Whether to create backup before migration
    /// </summary>
    public bool CreateBackup { get; set; } = true;

    /// <summary>
    /// Whether to validate migrated file
    /// </summary>
    public bool ValidateOutput { get; set; } = true;

    /// <summary>
    /// Whether to preserve original data when possible
    /// </summary>
    public bool PreserveOriginalData { get; set; } = true;

    /// <summary>
    /// Migration strategy for incompatible data
    /// </summary>
    public MigrationStrategy Strategy { get; set; } = MigrationStrategy.BestEffort;

    /// <summary>
    /// Custom migration parameters
    /// </summary>
    public Dictionary<string, object> CustomParameters { get; set; } = new();
}

/// <summary>
/// Conversion request for batch operations
/// </summary>
public class ConversionRequest
{
    /// <summary>
    /// Input file path
    /// </summary>
    public string InputPath { get; set; } = string.Empty;

    /// <summary>
    /// Output file path
    /// </summary>
    public string OutputPath { get; set; } = string.Empty;

    /// <summary>
    /// Target file format
    /// </summary>
    public FileType TargetFormat { get; set; }

    /// <summary>
    /// Request priority
    /// </summary>
    public ConversionPriority Priority { get; set; } = ConversionPriority.Normal;

    /// <summary>
    /// Custom request parameters
    /// </summary>
    public Dictionary<string, object> Parameters { get; set; } = new();
}

/// <summary>
/// Result of a file format conversion
/// </summary>
public class ConversionResult
{
    /// <summary>
    /// Whether the conversion was successful
    /// </summary>
    public bool Success { get; set; }

    /// <summary>
    /// Source file format
    /// </summary>
    public FileType SourceFormat { get; set; }

    /// <summary>
    /// Target file format
    /// </summary>
    public FileType TargetFormat { get; set; }

    /// <summary>
    /// Number of items processed
    /// </summary>
    public int ItemsProcessed { get; set; }

    /// <summary>
    /// Conversion duration
    /// </summary>
    public TimeSpan Duration { get; set; }

    /// <summary>
    /// Error message if conversion failed
    /// </summary>
    public string? ErrorMessage { get; set; }

    /// <summary>
    /// Validation errors
    /// </summary>
    public List<string> ValidationErrors { get; set; } = new();

    /// <summary>
    /// Output file checksum
    /// </summary>
    public string? OutputChecksum { get; set; }

    /// <summary>
    /// Conversion warnings
    /// </summary>
    public List<string> Warnings { get; set; } = new();

    /// <summary>
    /// Additional result metadata
    /// </summary>
    public Dictionary<string, object> Metadata { get; set; } = new();
}

/// <summary>
/// Result of a batch conversion operation
/// </summary>
public class BatchConversionResult
{
    /// <summary>
    /// Total number of files processed
    /// </summary>
    public int TotalFiles { get; set; }

    /// <summary>
    /// Number of successful conversions
    /// </summary>
    public int SuccessfulConversions { get; set; }

    /// <summary>
    /// Number of failed conversions
    /// </summary>
    public int FailedConversions { get; set; }

    /// <summary>
    /// Individual conversion results
    /// </summary>
    public List<ConversionResult> Results { get; set; } = new();

    /// <summary>
    /// Total batch processing duration
    /// </summary>
    public TimeSpan Duration { get; set; }

    /// <summary>
    /// Success rate (0.0 to 1.0)
    /// </summary>
    public double SuccessRate => TotalFiles > 0 ? (double)SuccessfulConversions / TotalFiles : 0.0;
}

/// <summary>
/// Result of a data export operation
/// </summary>
public class ExportResult
{
    /// <summary>
    /// Whether the export was successful
    /// </summary>
    public bool Success { get; set; }

    /// <summary>
    /// Export format used
    /// </summary>
    public ExportFormat ExportFormat { get; set; }

    /// <summary>
    /// Number of items exported
    /// </summary>
    public int ItemsExported { get; set; }

    /// <summary>
    /// Export duration
    /// </summary>
    public TimeSpan Duration { get; set; }

    /// <summary>
    /// Error message if export failed
    /// </summary>
    public string? ErrorMessage { get; set; }

    /// <summary>
    /// Output file checksum
    /// </summary>
    public string? OutputChecksum { get; set; }

    /// <summary>
    /// Export warnings
    /// </summary>
    public List<string> Warnings { get; set; } = new();
}

/// <summary>
/// Result of a version migration operation
/// </summary>
public class MigrationResult
{
    /// <summary>
    /// Whether the migration was successful
    /// </summary>
    public bool Success { get; set; }

    /// <summary>
    /// Source version
    /// </summary>
    public uint SourceVersion { get; set; }

    /// <summary>
    /// Target version
    /// </summary>
    public uint TargetVersion { get; set; }

    /// <summary>
    /// Migration duration
    /// </summary>
    public TimeSpan Duration { get; set; }

    /// <summary>
    /// Error message if migration failed
    /// </summary>
    public string? ErrorMessage { get; set; }

    /// <summary>
    /// Migration steps performed
    /// </summary>
    public List<string> MigrationSteps { get; set; } = new();

    /// <summary>
    /// Migration warnings
    /// </summary>
    public List<string> Warnings { get; set; } = new();

    /// <summary>
    /// Data that couldn't be migrated
    /// </summary>
    public List<string> UnmigratedData { get; set; } = new();
}

/// <summary>
/// Progress information for conversion operations
/// </summary>
public class ConversionProgress
{
    /// <summary>
    /// Progress percentage (0-100)
    /// </summary>
    public double ProgressPercentage { get; set; }

    /// <summary>
    /// Current operation description
    /// </summary>
    public string CurrentOperation { get; set; } = string.Empty;

    /// <summary>
    /// Number of items processed
    /// </summary>
    public int ItemsProcessed { get; set; }

    /// <summary>
    /// Total items to process
    /// </summary>
    public int TotalItems { get; set; }

    /// <summary>
    /// Estimated time remaining
    /// </summary>
    public TimeSpan? EstimatedTimeRemaining { get; set; }
}

/// <summary>
/// Progress information for batch conversion operations
/// </summary>
public class BatchConversionProgress
{
    /// <summary>
    /// Number of completed files
    /// </summary>
    public int CompletedFiles { get; set; }

    /// <summary>
    /// Total number of files
    /// </summary>
    public int TotalFiles { get; set; }

    /// <summary>
    /// Currently processing file
    /// </summary>
    public string CurrentFile { get; set; } = string.Empty;

    /// <summary>
    /// Progress of current file (0-100)
    /// </summary>
    public double CurrentFileProgress { get; set; }

    /// <summary>
    /// Overall progress (0-100)
    /// </summary>
    public double OverallProgress { get; set; }
}

/// <summary>
/// Data structure for export operations
/// </summary>
public class ExportData
{
    /// <summary>
    /// Source file type
    /// </summary>
    public FileType FileType { get; set; }

    /// <summary>
    /// File header information
    /// </summary>
    public object? Header { get; set; }

    /// <summary>
    /// Items to export
    /// </summary>
    public List<object>? Items { get; set; }

    /// <summary>
    /// Additional metadata
    /// </summary>
    public Dictionary<string, object> Metadata { get; set; } = new();
}

/// <summary>
/// Compression levels for output files
/// </summary>
public enum CompressionLevel
{
    /// <summary>
    /// No compression
    /// </summary>
    None,

    /// <summary>
    /// Fast compression
    /// </summary>
    Fast,

    /// <summary>
    /// Default compression
    /// </summary>
    Default,

    /// <summary>
    /// Maximum compression
    /// </summary>
    Maximum
}

/// <summary>
/// Export formats supported
/// </summary>
public enum ExportFormat
{
    /// <summary>
    /// JSON format
    /// </summary>
    Json,

    /// <summary>
    /// XML format
    /// </summary>
    Xml,

    /// <summary>
    /// CSV format
    /// </summary>
    Csv,

    /// <summary>
    /// Binary format
    /// </summary>
    Binary
}

/// <summary>
/// Migration strategies for handling incompatible data
/// </summary>
public enum MigrationStrategy
{
    /// <summary>
    /// Attempt to migrate as much as possible, skip incompatible data
    /// </summary>
    BestEffort,

    /// <summary>
    /// Fail if any data cannot be migrated
    /// </summary>
    Strict,

    /// <summary>
    /// Use default values for incompatible data
    /// </summary>
    UseDefaults,

    /// <summary>
    /// Prompt user for decisions on incompatible data
    /// </summary>
    Interactive
}

/// <summary>
/// Conversion priority levels
/// </summary>
public enum ConversionPriority
{
    /// <summary>
    /// Low priority
    /// </summary>
    Low,

    /// <summary>
    /// Normal priority
    /// </summary>
    Normal,

    /// <summary>
    /// High priority
    /// </summary>
    High,

    /// <summary>
    /// Critical priority
    /// </summary>
    Critical
}

/// <summary>
/// Event arguments for conversion progress
/// </summary>
public class ConversionProgressEventArgs : EventArgs
{
    /// <summary>
    /// Progress information
    /// </summary>
    public ConversionProgress Progress { get; set; } = null!;

    /// <summary>
    /// File being processed
    /// </summary>
    public string FilePath { get; set; } = string.Empty;
}

/// <summary>
/// Event arguments for conversion completion
/// </summary>
public class ConversionCompletedEventArgs : EventArgs
{
    /// <summary>
    /// Input file path
    /// </summary>
    public string InputPath { get; set; } = string.Empty;

    /// <summary>
    /// Output file path
    /// </summary>
    public string OutputPath { get; set; } = string.Empty;

    /// <summary>
    /// Conversion result
    /// </summary>
    public ConversionResult Result { get; set; } = null!;
}