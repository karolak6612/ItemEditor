using Microsoft.Extensions.Logging;
using System.Buffers;
using System.IO.MemoryMappedFiles;
using System.IO.Pipelines;
using System.Security.Cryptography;
using ItemEditor.Models;
using PluginInterface.OTLib.FileFormats;

namespace ItemEditor.Services;

/// <summary>
/// High-performance streaming file service with memory-mapped file support
/// </summary>
public class StreamingFileService : IDisposable
{
    private readonly ILogger<StreamingFileService> _logger;
    private readonly FileFormatValidator _validator;
    private readonly Dictionary<string, MemoryMappedFile> _memoryMappedFiles = new();
    private readonly Dictionary<string, MemoryMappedViewAccessor> _memoryMappedAccessors = new();
    private readonly SemaphoreSlim _fileLock = new(1, 1);
    private bool _disposed;

    /// <summary>
    /// Initializes a new instance of the StreamingFileService class
    /// </summary>
    /// <param name="logger">Logger instance</param>
    /// <param name="validator">File format validator</param>
    public StreamingFileService(ILogger<StreamingFileService> logger, FileFormatValidator validator)
    {
        _logger = logger;
        _validator = validator;
    }

    /// <summary>
    /// Creates a streaming reader for OTB files with memory-mapped file support
    /// </summary>
    /// <param name="filePath">Path to the OTB file</param>
    /// <param name="useMemoryMapping">Whether to use memory-mapped files for large files</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Streaming OTB reader</returns>
    public async Task<StreamingOTBReader> CreateOTBReaderAsync(
        string filePath, 
        bool useMemoryMapping = true, 
        CancellationToken cancellationToken = default)
    {
        await _fileLock.WaitAsync(cancellationToken);
        try
        {
            _logger.LogInformation("Creating OTB reader for {FilePath} (MemoryMapping: {UseMemoryMapping})", 
                filePath, useMemoryMapping);

            var fileInfo = new FileInfo(filePath);
            if (!fileInfo.Exists)
            {
                throw new FileNotFoundException($"File not found: {filePath}");
            }

            // Use memory mapping for files larger than 50MB
            const long memoryMappingThreshold = 50 * 1024 * 1024;
            var shouldUseMemoryMapping = useMemoryMapping && fileInfo.Length > memoryMappingThreshold;

            if (shouldUseMemoryMapping)
            {
                return await CreateMemoryMappedOTBReaderAsync(filePath, cancellationToken);
            }
            else
            {
                return await CreateStreamingOTBReaderAsync(filePath, cancellationToken);
            }
        }
        finally
        {
            _fileLock.Release();
        }
    }

    /// <summary>
    /// Creates a memory-mapped OTB reader for very large files
    /// </summary>
    /// <param name="filePath">Path to the OTB file</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Memory-mapped OTB reader</returns>
    private async Task<StreamingOTBReader> CreateMemoryMappedOTBReaderAsync(
        string filePath, 
        CancellationToken cancellationToken)
    {
        try
        {
            var fileInfo = new FileInfo(filePath);
            var mapName = $"OTB_{Path.GetFileName(filePath)}_{fileInfo.Length}";

            // Create memory-mapped file
            var mmf = MemoryMappedFile.CreateFromFile(
                filePath, 
                FileMode.Open, 
                mapName, 
                fileInfo.Length, 
                MemoryMappedFileAccess.Read);

            var accessor = mmf.CreateViewAccessor(0, fileInfo.Length, MemoryMappedFileAccess.Read);

            // Store references for cleanup
            _memoryMappedFiles[filePath] = mmf;
            _memoryMappedAccessors[filePath] = accessor;

            _logger.LogDebug("Created memory-mapped file for {FilePath} ({FileSize} bytes)", 
                filePath, fileInfo.Length);

            return new StreamingOTBReader(accessor, _logger);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to create memory-mapped OTB reader for {FilePath}", filePath);
            throw;
        }
    }

    /// <summary>
    /// Creates a streaming OTB reader using System.IO.Pipelines
    /// </summary>
    /// <param name="filePath">Path to the OTB file</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Streaming OTB reader</returns>
    private async Task<StreamingOTBReader> CreateStreamingOTBReaderAsync(
        string filePath, 
        CancellationToken cancellationToken)
    {
        try
        {
            var fileStream = new FileStream(filePath, FileMode.Open, FileAccess.Read, FileShare.Read, 
                bufferSize: 64 * 1024, useAsync: true);

            _logger.LogDebug("Created streaming reader for {FilePath}", filePath);

            return new StreamingOTBReader(fileStream, _logger);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to create streaming OTB reader for {FilePath}", filePath);
            throw;
        }
    }

    /// <summary>
    /// Validates file format with comprehensive error reporting and recovery
    /// </summary>
    /// <param name="filePath">Path to the file to validate</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Comprehensive validation result</returns>
    public async Task<FileValidationResult> ValidateFileFormatAsync(
        string filePath, 
        CancellationToken cancellationToken = default)
    {
        try
        {
            _logger.LogInformation("Validating file format for {FilePath}", filePath);
            
            var result = await _validator.ValidateAsync(filePath, cancellationToken);
            
            // Calculate file checksum for integrity verification
            result.FileChecksum = await CalculateFileChecksumAsync(filePath, cancellationToken);

            _logger.LogInformation("File validation completed for {FilePath}: {IsValid}", 
                filePath, result.IsValid);

            return result;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error during file validation for {FilePath}", filePath);
            return new FileValidationResult
            {
                FileType = FileType.Unknown,
                IsValid = false,
                Errors = new List<string> { $"Validation error: {ex.Message}" },
                Message = "Validation failed due to error",
                FileChecksum = string.Empty
            };
        }
    }



    /// <summary>
    /// Calculates SHA-256 checksum of a file
    /// </summary>
    /// <param name="filePath">Path to the file</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>File checksum as hex string</returns>
    private async Task<string> CalculateFileChecksumAsync(string filePath, CancellationToken cancellationToken)
    {
        try
        {
            using var sha256 = SHA256.Create();
            using var fileStream = new FileStream(filePath, FileMode.Open, FileAccess.Read, FileShare.Read, 
                bufferSize: 64 * 1024, useAsync: true);

            var hashBytes = await sha256.ComputeHashAsync(fileStream, cancellationToken);
            return Convert.ToHexString(hashBytes);
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "Failed to calculate checksum for {FilePath}", filePath);
            return string.Empty;
        }
    }

    /// <summary>
    /// Closes memory-mapped file for the specified path
    /// </summary>
    /// <param name="filePath">File path</param>
    public async Task CloseMemoryMappedFileAsync(string filePath)
    {
        await _fileLock.WaitAsync();
        try
        {
            if (_memoryMappedAccessors.TryGetValue(filePath, out var accessor))
            {
                accessor.Dispose();
                _memoryMappedAccessors.Remove(filePath);
            }

            if (_memoryMappedFiles.TryGetValue(filePath, out var mmf))
            {
                mmf.Dispose();
                _memoryMappedFiles.Remove(filePath);
            }

            _logger.LogDebug("Closed memory-mapped file for {FilePath}", filePath);
        }
        finally
        {
            _fileLock.Release();
        }
    }

    /// <inheritdoc />
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    /// <summary>
    /// Disposes the streaming file service
    /// </summary>
    /// <param name="disposing">True if disposing managed resources</param>
    protected virtual void Dispose(bool disposing)
    {
        if (!_disposed && disposing)
        {
            _fileLock.Wait();
            try
            {
                // Dispose all memory-mapped accessors
                foreach (var accessor in _memoryMappedAccessors.Values)
                {
                    accessor.Dispose();
                }
                _memoryMappedAccessors.Clear();

                // Dispose all memory-mapped files
                foreach (var mmf in _memoryMappedFiles.Values)
                {
                    mmf.Dispose();
                }
                _memoryMappedFiles.Clear();
            }
            finally
            {
                _fileLock.Release();
                _fileLock.Dispose();
            }

            _disposed = true;
        }
    }
}