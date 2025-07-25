namespace PluginInterface.OTLib.Exceptions;

/// <summary>
/// Base exception for OTLib operations
/// </summary>
public class OTLibException : Exception
{
    /// <summary>
    /// Initializes a new instance of the OTLibException class
    /// </summary>
    public OTLibException() : base() { }

    /// <summary>
    /// Initializes a new instance of the OTLibException class with a message
    /// </summary>
    /// <param name="message">Exception message</param>
    public OTLibException(string message) : base(message) { }

    /// <summary>
    /// Initializes a new instance of the OTLibException class with a message and inner exception
    /// </summary>
    /// <param name="message">Exception message</param>
    /// <param name="innerException">Inner exception</param>
    public OTLibException(string message, Exception innerException) : base(message, innerException) { }
}

/// <summary>
/// Exception thrown when file format is invalid or corrupted
/// </summary>
public class InvalidFileFormatException : OTLibException
{
    /// <summary>
    /// Gets the file path that caused the exception
    /// </summary>
    public string? FilePath { get; }

    /// <summary>
    /// Gets the expected file format
    /// </summary>
    public string? ExpectedFormat { get; }

    /// <summary>
    /// Initializes a new instance of the InvalidFileFormatException class
    /// </summary>
    /// <param name="message">Exception message</param>
    /// <param name="filePath">File path</param>
    /// <param name="expectedFormat">Expected format</param>
    public InvalidFileFormatException(string message, string? filePath = null, string? expectedFormat = null) 
        : base(message)
    {
        FilePath = filePath;
        ExpectedFormat = expectedFormat;
    }

    /// <summary>
    /// Initializes a new instance of the InvalidFileFormatException class with inner exception
    /// </summary>
    /// <param name="message">Exception message</param>
    /// <param name="innerException">Inner exception</param>
    /// <param name="filePath">File path</param>
    /// <param name="expectedFormat">Expected format</param>
    public InvalidFileFormatException(string message, Exception innerException, string? filePath = null, string? expectedFormat = null) 
        : base(message, innerException)
    {
        FilePath = filePath;
        ExpectedFormat = expectedFormat;
    }
}

/// <summary>
/// Exception thrown when file version is not supported
/// </summary>
public class UnsupportedFileVersionException : OTLibException
{
    /// <summary>
    /// Gets the file version that is not supported
    /// </summary>
    public uint FileVersion { get; }

    /// <summary>
    /// Gets the minimum supported version
    /// </summary>
    public uint? MinSupportedVersion { get; }

    /// <summary>
    /// Gets the maximum supported version
    /// </summary>
    public uint? MaxSupportedVersion { get; }

    /// <summary>
    /// Initializes a new instance of the UnsupportedFileVersionException class
    /// </summary>
    /// <param name="message">Exception message</param>
    /// <param name="fileVersion">File version</param>
    /// <param name="minSupportedVersion">Minimum supported version</param>
    /// <param name="maxSupportedVersion">Maximum supported version</param>
    public UnsupportedFileVersionException(
        string message, 
        uint fileVersion, 
        uint? minSupportedVersion = null, 
        uint? maxSupportedVersion = null) 
        : base(message)
    {
        FileVersion = fileVersion;
        MinSupportedVersion = minSupportedVersion;
        MaxSupportedVersion = maxSupportedVersion;
    }
}

/// <summary>
/// Exception thrown when sprite operations fail
/// </summary>
public class SpriteException : OTLibException
{
    /// <summary>
    /// Gets the sprite ID that caused the exception
    /// </summary>
    public ushort? SpriteId { get; }

    /// <summary>
    /// Initializes a new instance of the SpriteException class
    /// </summary>
    /// <param name="message">Exception message</param>
    /// <param name="spriteId">Sprite ID</param>
    public SpriteException(string message, ushort? spriteId = null) : base(message)
    {
        SpriteId = spriteId;
    }

    /// <summary>
    /// Initializes a new instance of the SpriteException class with inner exception
    /// </summary>
    /// <param name="message">Exception message</param>
    /// <param name="innerException">Inner exception</param>
    /// <param name="spriteId">Sprite ID</param>
    public SpriteException(string message, Exception innerException, ushort? spriteId = null) 
        : base(message, innerException)
    {
        SpriteId = spriteId;
    }
}

/// <summary>
/// Exception thrown when item operations fail
/// </summary>
public class ItemException : OTLibException
{
    /// <summary>
    /// Gets the item ID that caused the exception
    /// </summary>
    public ushort? ItemId { get; }

    /// <summary>
    /// Initializes a new instance of the ItemException class
    /// </summary>
    /// <param name="message">Exception message</param>
    /// <param name="itemId">Item ID</param>
    public ItemException(string message, ushort? itemId = null) : base(message)
    {
        ItemId = itemId;
    }

    /// <summary>
    /// Initializes a new instance of the ItemException class with inner exception
    /// </summary>
    /// <param name="message">Exception message</param>
    /// <param name="innerException">Inner exception</param>
    /// <param name="itemId">Item ID</param>
    public ItemException(string message, Exception innerException, ushort? itemId = null) 
        : base(message, innerException)
    {
        ItemId = itemId;
    }
}

/// <summary>
/// Exception thrown when file I/O operations fail
/// </summary>
public class FileIOException : OTLibException
{
    /// <summary>
    /// Gets the file path that caused the exception
    /// </summary>
    public string? FilePath { get; }

    /// <summary>
    /// Gets the I/O operation that failed
    /// </summary>
    public string? Operation { get; }

    /// <summary>
    /// Initializes a new instance of the FileIOException class
    /// </summary>
    /// <param name="message">Exception message</param>
    /// <param name="filePath">File path</param>
    /// <param name="operation">I/O operation</param>
    public FileIOException(string message, string? filePath = null, string? operation = null) 
        : base(message)
    {
        FilePath = filePath;
        Operation = operation;
    }

    /// <summary>
    /// Initializes a new instance of the FileIOException class with inner exception
    /// </summary>
    /// <param name="message">Exception message</param>
    /// <param name="innerException">Inner exception</param>
    /// <param name="filePath">File path</param>
    /// <param name="operation">I/O operation</param>
    public FileIOException(string message, Exception innerException, string? filePath = null, string? operation = null) 
        : base(message, innerException)
    {
        FilePath = filePath;
        Operation = operation;
    }
}

/// <summary>
/// Exception thrown when data corruption is detected
/// </summary>
public class DataCorruptionException : OTLibException
{
    /// <summary>
    /// Gets the file offset where corruption was detected
    /// </summary>
    public long? FileOffset { get; }

    /// <summary>
    /// Gets the expected data description
    /// </summary>
    public string? ExpectedData { get; }

    /// <summary>
    /// Gets the actual data description
    /// </summary>
    public string? ActualData { get; }

    /// <summary>
    /// Initializes a new instance of the DataCorruptionException class
    /// </summary>
    /// <param name="message">Exception message</param>
    /// <param name="fileOffset">File offset</param>
    /// <param name="expectedData">Expected data description</param>
    /// <param name="actualData">Actual data description</param>
    public DataCorruptionException(
        string message, 
        long? fileOffset = null, 
        string? expectedData = null, 
        string? actualData = null) 
        : base(message)
    {
        FileOffset = fileOffset;
        ExpectedData = expectedData;
        ActualData = actualData;
    }
}

/// <summary>
/// Exception thrown when memory allocation fails
/// </summary>
public class MemoryAllocationException : OTLibException
{
    /// <summary>
    /// Gets the requested memory size in bytes
    /// </summary>
    public long RequestedSize { get; }

    /// <summary>
    /// Initializes a new instance of the MemoryAllocationException class
    /// </summary>
    /// <param name="message">Exception message</param>
    /// <param name="requestedSize">Requested memory size</param>
    public MemoryAllocationException(string message, long requestedSize) : base(message)
    {
        RequestedSize = requestedSize;
    }

    /// <summary>
    /// Initializes a new instance of the MemoryAllocationException class with inner exception
    /// </summary>
    /// <param name="message">Exception message</param>
    /// <param name="innerException">Inner exception</param>
    /// <param name="requestedSize">Requested memory size</param>
    public MemoryAllocationException(string message, Exception innerException, long requestedSize) 
        : base(message, innerException)
    {
        RequestedSize = requestedSize;
    }
}