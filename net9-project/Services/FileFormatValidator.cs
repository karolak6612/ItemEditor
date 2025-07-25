using Microsoft.Extensions.Logging;
using System.Text;
using ItemEditor.Models;
using PluginInterface.OTLib.FileFormats;

namespace ItemEditor.Services;

/// <summary>
/// Comprehensive file format validator with error recovery capabilities
/// </summary>
public class FileFormatValidator
{
    private readonly ILogger<FileFormatValidator> _logger;
    private readonly Dictionary<FileType, IFileFormatValidator> _validators;

    /// <summary>
    /// Initializes a new instance of the FileFormatValidator class
    /// </summary>
    /// <param name="logger">Logger instance</param>
    public FileFormatValidator(ILogger<FileFormatValidator> logger)
    {
        _logger = logger;
        _validators = new Dictionary<FileType, IFileFormatValidator>
        {
            { FileType.OTB, new OTBFormatValidator(logger) },
            { FileType.DAT, new DATFormatValidator(logger) },
            { FileType.SPR, new SPRFormatValidator(logger) }
        };
    }

    /// <summary>
    /// Validates a file with comprehensive error reporting and recovery suggestions
    /// </summary>
    /// <param name="filePath">Path to the file to validate</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Detailed validation result</returns>
    public async Task<FileValidationResult> ValidateAsync(string filePath, CancellationToken cancellationToken = default)
    {
        var result = new FileValidationResult
        {
            FileType = FileType.Unknown,
            IsValid = false,
            Warnings = new List<string>(),
            Errors = new List<string>(),
            ValidationTimestamp = DateTime.UtcNow
        };

        try
        {
            _logger.LogInformation("Starting comprehensive validation for {FilePath}", filePath);

            // Basic file checks
            if (!await PerformBasicFileChecksAsync(filePath, result, cancellationToken))
            {
                return result;
            }

            // Detect file type
            result.FileType = await DetectFileTypeAsync(filePath, cancellationToken);
            
            if (result.FileType == FileType.Unknown)
            {
                result.Errors.Add("Unable to determine file type");
                result.Message = "Unknown file format";
                return result;
            }

            // Perform format-specific validation
            if (_validators.TryGetValue(result.FileType, out var validator))
            {
                await validator.ValidateAsync(filePath, result, cancellationToken);
            }
            else
            {
                result.Errors.Add($"No validator available for {result.FileType} files");
                result.Message = "Unsupported file format";
            }

            // Generate final message
            if (result.IsValid)
            {
                var warningCount = result.Warnings.Count;
                result.Message = warningCount > 0 
                    ? $"Valid {result.FileType} file with {warningCount} warning(s)"
                    : $"Valid {result.FileType} file";
            }
            else
            {
                result.Message = $"Invalid {result.FileType} file with {result.Errors.Count} error(s)";
            }

            _logger.LogInformation("Validation completed for {FilePath}: {IsValid} ({ErrorCount} errors, {WarningCount} warnings)",
                filePath, result.IsValid, result.Errors.Count, result.Warnings.Count);

            return result;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Validation failed for {FilePath}", filePath);
            result.Errors.Add($"Validation exception: {ex.Message}");
            result.Message = "Validation failed due to error";
            return result;
        }
    }

    /// <summary>
    /// Performs basic file system checks
    /// </summary>
    /// <param name="filePath">File path</param>
    /// <param name="result">Validation result to populate</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>True if basic checks pass</returns>
    private async Task<bool> PerformBasicFileChecksAsync(
        string filePath, 
        FileValidationResult result, 
        CancellationToken cancellationToken)
    {
        // Check if file exists
        if (!File.Exists(filePath))
        {
            result.Errors.Add("File does not exist");
            result.Message = "File not found";
            return false;
        }

        // Check file size
        var fileInfo = new FileInfo(filePath);
        if (fileInfo.Length == 0)
        {
            result.Errors.Add("File is empty");
            result.Message = "Empty file";
            return false;
        }

        // Check if file is too large (>2GB)
        const long maxFileSize = 2L * 1024 * 1024 * 1024; // 2GB
        if (fileInfo.Length > maxFileSize)
        {
            result.Warnings.Add($"File is very large ({fileInfo.Length:N0} bytes). Processing may be slow.");
        }

        // Check file permissions
        try
        {
            using var stream = File.OpenRead(filePath);
            // File is readable
        }
        catch (UnauthorizedAccessException)
        {
            result.Errors.Add("Access denied. Check file permissions.");
            result.Message = "Access denied";
            return false;
        }
        catch (IOException ex)
        {
            result.Errors.Add($"I/O error: {ex.Message}");
            result.Message = "File I/O error";
            return false;
        }

        await Task.CompletedTask;
        return true;
    }

    /// <summary>
    /// Detects file type from extension and content
    /// </summary>
    /// <param name="filePath">File path</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Detected file type</returns>
    private async Task<FileType> DetectFileTypeAsync(string filePath, CancellationToken cancellationToken)
    {
        // First try extension-based detection
        var extension = Path.GetExtension(filePath).ToLowerInvariant();
        var typeFromExtension = extension switch
        {
            ".otb" => FileType.OTB,
            ".dat" => FileType.DAT,
            ".spr" => FileType.SPR,
            _ => FileType.Unknown
        };

        if (typeFromExtension != FileType.Unknown)
        {
            // Verify with content-based detection
            var typeFromContent = await DetectFileTypeFromContentAsync(filePath, cancellationToken);
            
            if (typeFromContent == typeFromExtension)
            {
                return typeFromExtension;
            }
            else if (typeFromContent != FileType.Unknown)
            {
                _logger.LogWarning("File extension ({Extension}) doesn't match content type ({ContentType}) for {FilePath}",
                    extension, typeFromContent, filePath);
                return typeFromContent;
            }
        }

        // Fall back to content-based detection only
        return await DetectFileTypeFromContentAsync(filePath, cancellationToken);
    }

    /// <summary>
    /// Detects file type from file content (magic bytes)
    /// </summary>
    /// <param name="filePath">File path</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Detected file type</returns>
    private async Task<FileType> DetectFileTypeFromContentAsync(string filePath, CancellationToken cancellationToken)
    {
        try
        {
            using var stream = new FileStream(filePath, FileMode.Open, FileAccess.Read, FileShare.Read);
            var buffer = new byte[16]; // Read first 16 bytes for magic detection
            
            var bytesRead = await stream.ReadAsync(buffer, 0, buffer.Length, cancellationToken);
            if (bytesRead < 4)
            {
                return FileType.Unknown;
            }

            // Check for OTB signature (placeholder - update with actual signature)
            var signature = BitConverter.ToUInt32(buffer, 0);
            if (signature == OTBHeader.ValidSignature)
            {
                return FileType.OTB;
            }

            // Check for DAT format (version number in first 4 bytes, typically > 700)
            var version = BitConverter.ToUInt32(buffer, 0);
            if (version >= 700 && version <= 1500) // Reasonable version range for DAT files
            {
                return FileType.DAT;
            }

            // Check for SPR format (sprite count in bytes 4-6, version in bytes 6-8)
            if (bytesRead >= 8)
            {
                var spriteCount = BitConverter.ToUInt16(buffer, 4);
                var sprVersion = BitConverter.ToUInt16(buffer, 6);
                
                if (spriteCount > 0 && spriteCount < 65535 && sprVersion > 0 && sprVersion < 1000)
                {
                    return FileType.SPR;
                }
            }

            return FileType.Unknown;
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "Failed to detect file type from content for {FilePath}", filePath);
            return FileType.Unknown;
        }
    }
}

/// <summary>
/// Interface for format-specific validators
/// </summary>
public interface IFileFormatValidator
{
    /// <summary>
    /// Validates a file of specific format
    /// </summary>
    /// <param name="filePath">File path</param>
    /// <param name="result">Validation result to populate</param>
    /// <param name="cancellationToken">Cancellation token</param>
    Task ValidateAsync(string filePath, FileValidationResult result, CancellationToken cancellationToken);
}

/// <summary>
/// OTB format validator
/// </summary>
public class OTBFormatValidator : IFileFormatValidator
{
    private readonly ILogger _logger;

    public OTBFormatValidator(ILogger logger)
    {
        _logger = logger;
    }

    public async Task ValidateAsync(string filePath, FileValidationResult result, CancellationToken cancellationToken)
    {
        try
        {
            using var stream = new FileStream(filePath, FileMode.Open, FileAccess.Read, FileShare.Read);
            using var reader = new OTBReader(stream, _logger);

            // Validate header
            var header = await reader.ReadHeaderAsync(cancellationToken);
            
            result.FileVersion = header.Version;
            result.ClientVersion = new Version((int)(header.ClientVersion >> 16), (int)(header.ClientVersion & 0xFFFF));

            if (header.Signature != OTBHeader.ValidSignature)
            {
                result.Errors.Add($"Invalid OTB signature: 0x{header.Signature:X8}");
                return;
            }

            // Validate version range
            if (header.Version < 1 || header.Version > 100)
            {
                result.Warnings.Add($"Unusual OTB version: {header.Version}");
            }

            // Attempt to read items to validate structure
            var itemCount = 0;
            var duplicateIds = new HashSet<ushort>();
            var seenIds = new HashSet<ushort>();

            try
            {
                var items = await reader.ReadItemsAsync(cancellationToken: cancellationToken);
                
                foreach (var item in items)
                {
                    itemCount++;
                    
                    // Check for duplicate IDs
                    if (!seenIds.Add(item.Id))
                    {
                        duplicateIds.Add(item.Id);
                    }

                    // Validate item properties
                    if (item.Id == 0)
                    {
                        result.Warnings.Add($"Item with ID 0 found at position {itemCount}");
                    }

                    if (item.Properties.Count == 0)
                    {
                        result.Warnings.Add($"Item {item.Id} has no properties");
                    }
                }

                result.ItemCount = itemCount;

                if (duplicateIds.Count > 0)
                {
                    result.Warnings.Add($"Found {duplicateIds.Count} duplicate item IDs: {string.Join(", ", duplicateIds.Take(10))}");
                }

                if (itemCount == 0)
                {
                    result.Warnings.Add("OTB file contains no items");
                }

                result.IsValid = true;
            }
            catch (Exception ex)
            {
                result.Errors.Add($"Error reading items: {ex.Message}");
            }
        }
        catch (Exception ex)
        {
            result.Errors.Add($"OTB validation error: {ex.Message}");
        }
    }
}

/// <summary>
/// DAT format validator
/// </summary>
public class DATFormatValidator : IFileFormatValidator
{
    private readonly ILogger _logger;

    public DATFormatValidator(ILogger logger)
    {
        _logger = logger;
    }

    public async Task ValidateAsync(string filePath, FileValidationResult result, CancellationToken cancellationToken)
    {
        try
        {
            using var stream = new FileStream(filePath, FileMode.Open, FileAccess.Read, FileShare.Read);
            using var reader = new DATReader(stream, _logger);

            // Validate header
            var header = await reader.ReadHeaderAsync(cancellationToken);
            
            result.FileVersion = header.Version;
            result.ItemCount = header.ItemCount;

            // Validate version range
            if (header.Version < 700 || header.Version > 1500)
            {
                result.Warnings.Add($"Unusual DAT version: {header.Version}");
            }

            if (header.ItemCount == 0)
            {
                result.Warnings.Add("DAT file contains no items");
            }

            // Validate counts are reasonable
            if (header.ItemCount > 50000)
            {
                result.Warnings.Add($"Very high item count: {header.ItemCount}");
            }

            // Attempt to read items to validate structure
            try
            {
                var items = await reader.ReadItemsAsync(header, cancellationToken: cancellationToken);
                
                var actualItemCount = items.Count;
                if (actualItemCount != header.ItemCount)
                {
                    result.Errors.Add($"Item count mismatch: header says {header.ItemCount}, found {actualItemCount}");
                }
                else
                {
                    result.IsValid = true;
                }

                // Validate item structure
                var itemsWithoutSprites = 0;
                var itemsWithManySprites = 0;

                foreach (var item in items.Take(100)) // Sample first 100 items
                {
                    if (item.SpriteIds.Count == 0)
                    {
                        itemsWithoutSprites++;
                    }
                    else if (item.SpriteIds.Count > 100)
                    {
                        itemsWithManySprites++;
                    }

                    // Validate sprite dimensions
                    if (item.Width == 0 || item.Height == 0)
                    {
                        result.Warnings.Add($"Item {item.Id} has zero dimensions");
                    }
                }

                if (itemsWithoutSprites > 10)
                {
                    result.Warnings.Add($"{itemsWithoutSprites} items have no sprites");
                }

                if (itemsWithManySprites > 0)
                {
                    result.Warnings.Add($"{itemsWithManySprites} items have unusually many sprites");
                }
            }
            catch (Exception ex)
            {
                result.Errors.Add($"Error reading DAT items: {ex.Message}");
            }
        }
        catch (Exception ex)
        {
            result.Errors.Add($"DAT validation error: {ex.Message}");
        }
    }
}

/// <summary>
/// SPR format validator
/// </summary>
public class SPRFormatValidator : IFileFormatValidator
{
    private readonly ILogger _logger;

    public SPRFormatValidator(ILogger logger)
    {
        _logger = logger;
    }

    public async Task ValidateAsync(string filePath, FileValidationResult result, CancellationToken cancellationToken)
    {
        try
        {
            using var stream = new FileStream(filePath, FileMode.Open, FileAccess.Read, FileShare.Read);
            using var reader = new SPRReader(stream, _logger);

            // Validate header
            var header = await reader.ReadHeaderAsync(cancellationToken);
            
            result.FileVersion = header.Version;
            result.SpriteCount = header.SpriteCount;

            if (header.SpriteCount == 0)
            {
                result.Warnings.Add("SPR file contains no sprites");
            }

            // Validate sprite count is reasonable
            if (header.SpriteCount > 100000)
            {
                result.Warnings.Add($"Very high sprite count: {header.SpriteCount}");
            }

            // Validate sprite addresses
            try
            {
                var addresses = await reader.ReadSpriteAddressesAsync(header, cancellationToken);
                
                if (addresses.Length != header.SpriteCount)
                {
                    result.Errors.Add($"Sprite address count mismatch: expected {header.SpriteCount}, got {addresses.Length}");
                    return;
                }

                // Validate addresses are within file bounds
                var fileSize = stream.Length;
                var invalidAddresses = 0;

                for (int i = 0; i < Math.Min(addresses.Length, 1000); i++) // Sample first 1000 addresses
                {
                    if (addresses[i] > 0 && addresses[i] >= fileSize)
                    {
                        invalidAddresses++;
                    }
                }

                if (invalidAddresses > 0)
                {
                    result.Errors.Add($"{invalidAddresses} sprite addresses point beyond file end");
                }
                else
                {
                    result.IsValid = true;
                }

                // Test reading a few sprites
                var testSpriteIds = new List<ushort>();
                for (ushort i = 1; i <= Math.Min(header.SpriteCount, 10); i++)
                {
                    testSpriteIds.Add(i);
                }

                var testSprites = await reader.ReadSpritesAsync(testSpriteIds, addresses, cancellationToken: cancellationToken);
                
                if (testSprites.Count < testSpriteIds.Count)
                {
                    result.Warnings.Add($"Could only read {testSprites.Count} of {testSpriteIds.Count} test sprites");
                }
            }
            catch (Exception ex)
            {
                result.Errors.Add($"Error reading sprite addresses: {ex.Message}");
            }
        }
        catch (Exception ex)
        {
            result.Errors.Add($"SPR validation error: {ex.Message}");
        }
    }
}