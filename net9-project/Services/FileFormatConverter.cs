using Microsoft.Extensions.Logging;
using System.Security.Cryptography;
using System.Text.Json;
using ItemEditor.Models;
using PluginInterface.OTLib.FileFormats;

namespace ItemEditor.Services;

/// <summary>
/// Service for converting between different file formats with progress indication and data integrity verification
/// </summary>
public class FileFormatConverter : IDisposable
{
    private readonly ILogger<FileFormatConverter> _logger;
    private readonly StreamingFileService _streamingService;
    private readonly SemaphoreSlim _conversionLock = new(1, 1);
    private bool _disposed;

    /// <summary>
    /// Initializes a new instance of the FileFormatConverter class
    /// </summary>
    /// <param name="logger">Logger instance</param>
    /// <param name="streamingService">Streaming file service</param>
    public FileFormatConverter(ILogger<FileFormatConverter> logger, StreamingFileService streamingService)
    {
        _logger = logger;
        _streamingService = streamingService;
    }

    /// <summary>
    /// Event raised when conversion progress changes
    /// </summary>
    public event EventHandler<ConversionProgressEventArgs>? ProgressChanged;

    /// <summary>
    /// Event raised when conversion completes
    /// </summary>
    public event EventHandler<ConversionCompletedEventArgs>? ConversionCompleted;

    /// <summary>
    /// Converts a file from one format to another
    /// </summary>
    /// <param name="inputPath">Input file path</param>
    /// <param name="outputPath">Output file path</param>
    /// <param name="targetFormat">Target file format</param>
    /// <param name="options">Conversion options</param>
    /// <param name="progress">Progress reporter</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Conversion result</returns>
    public async Task<ConversionResult> ConvertFileAsync(
        string inputPath,
        string outputPath,
        FileType targetFormat,
        ConversionOptions? options = null,
        IProgress<ConversionProgress>? progress = null,
        CancellationToken cancellationToken = default)
    {
        options ??= new ConversionOptions();
        var startTime = DateTime.UtcNow;
        
        await _conversionLock.WaitAsync(cancellationToken);
        try
        {
            _logger.LogInformation("Starting conversion from {InputPath} to {OutputPath} (format: {TargetFormat})",
                inputPath, outputPath, targetFormat);

            // Validate input file
            var validationResult = await _streamingService.ValidateFileFormatAsync(inputPath, cancellationToken);
            if (!validationResult.IsValid)
            {
                return new ConversionResult
                {
                    Success = false,
                    ErrorMessage = $"Input file validation failed: {validationResult.Message}",
                    ValidationErrors = validationResult.Errors
                };
            }

            var sourceFormat = validationResult.FileType;
            
            // Check if conversion is needed
            if (sourceFormat == targetFormat)
            {
                _logger.LogInformation("Source and target formats are the same, copying file");
                File.Copy(inputPath, outputPath, true);
                
                return new ConversionResult
                {
                    Success = true,
                    SourceFormat = sourceFormat,
                    TargetFormat = targetFormat,
                    ItemsProcessed = validationResult.ItemCount,
                    Duration = DateTime.UtcNow - startTime,
                    OutputChecksum = await CalculateFileChecksumAsync(outputPath, cancellationToken)
                };
            }

            // Perform conversion based on source and target formats
            var result = await PerformConversionAsync(
                inputPath, outputPath, sourceFormat, targetFormat, 
                options, progress, cancellationToken);

            result.Duration = DateTime.UtcNow - startTime;
            
            // Verify output file integrity
            if (result.Success && options.VerifyIntegrity)
            {
                result.OutputChecksum = await CalculateFileChecksumAsync(outputPath, cancellationToken);
                
                var outputValidation = await _streamingService.ValidateFileFormatAsync(outputPath, cancellationToken);
                if (!outputValidation.IsValid)
                {
                    result.Success = false;
                    result.ErrorMessage = $"Output file validation failed: {outputValidation.Message}";
                    result.ValidationErrors = outputValidation.Errors;
                }
            }

            OnConversionCompleted(new ConversionCompletedEventArgs
            {
                InputPath = inputPath,
                OutputPath = outputPath,
                Result = result
            });

            _logger.LogInformation("Conversion completed: {Success} ({Duration}ms)", 
                result.Success, result.Duration.TotalMilliseconds);

            return result;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Conversion failed for {InputPath} to {OutputPath}", inputPath, outputPath);
            
            return new ConversionResult
            {
                Success = false,
                ErrorMessage = ex.Message,
                Duration = DateTime.UtcNow - startTime
            };
        }
        finally
        {
            _conversionLock.Release();
        }
    }

    /// <summary>
    /// Converts multiple files in batch with progress reporting
    /// </summary>
    /// <param name="conversions">List of conversion requests</param>
    /// <param name="options">Conversion options</param>
    /// <param name="progress">Progress reporter</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Batch conversion results</returns>
    public async Task<BatchConversionResult> ConvertFilesAsync(
        IEnumerable<ConversionRequest> conversions,
        ConversionOptions? options = null,
        IProgress<BatchConversionProgress>? progress = null,
        CancellationToken cancellationToken = default)
    {
        var conversionList = conversions.ToList();
        var results = new List<ConversionResult>();
        var startTime = DateTime.UtcNow;
        var completedCount = 0;
        var totalCount = conversionList.Count;

        _logger.LogInformation("Starting batch conversion of {TotalCount} files", totalCount);

        try
        {
            var semaphore = new SemaphoreSlim(options?.MaxConcurrency ?? Environment.ProcessorCount, 
                                            options?.MaxConcurrency ?? Environment.ProcessorCount);

            var tasks = conversionList.Select(async (conversion, index) =>
            {
                await semaphore.WaitAsync(cancellationToken);
                try
                {
                    var result = await ConvertFileAsync(
                        conversion.InputPath,
                        conversion.OutputPath,
                        conversion.TargetFormat,
                        options,
                        new Progress<ConversionProgress>(p =>
                        {
                            progress?.Report(new BatchConversionProgress
                            {
                                CompletedFiles = completedCount,
                                TotalFiles = totalCount,
                                CurrentFile = conversion.InputPath,
                                CurrentFileProgress = p.ProgressPercentage,
                                OverallProgress = (double)completedCount / totalCount * 100
                            });
                        }),
                        cancellationToken);

                    Interlocked.Increment(ref completedCount);
                    
                    progress?.Report(new BatchConversionProgress
                    {
                        CompletedFiles = completedCount,
                        TotalFiles = totalCount,
                        CurrentFile = string.Empty,
                        CurrentFileProgress = 100,
                        OverallProgress = (double)completedCount / totalCount * 100
                    });

                    return result;
                }
                finally
                {
                    semaphore.Release();
                }
            });

            results.AddRange(await Task.WhenAll(tasks));
            semaphore.Dispose();
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Batch conversion failed");
        }

        var batchResult = new BatchConversionResult
        {
            TotalFiles = totalCount,
            SuccessfulConversions = results.Count(r => r.Success),
            FailedConversions = results.Count(r => !r.Success),
            Results = results,
            Duration = DateTime.UtcNow - startTime
        };

        _logger.LogInformation("Batch conversion completed: {Successful}/{Total} successful ({Duration}ms)",
            batchResult.SuccessfulConversions, batchResult.TotalFiles, batchResult.Duration.TotalMilliseconds);

        return batchResult;
    }

    /// <summary>
    /// Exports file data to various formats (JSON, XML, CSV)
    /// </summary>
    /// <param name="inputPath">Input file path</param>
    /// <param name="outputPath">Output file path</param>
    /// <param name="exportFormat">Export format</param>
    /// <param name="options">Export options</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Export result</returns>
    public async Task<ExportResult> ExportDataAsync(
        string inputPath,
        string outputPath,
        ExportFormat exportFormat,
        ExportOptions? options = null,
        CancellationToken cancellationToken = default)
    {
        options ??= new ExportOptions();
        var startTime = DateTime.UtcNow;

        try
        {
            _logger.LogInformation("Exporting data from {InputPath} to {OutputPath} (format: {ExportFormat})",
                inputPath, outputPath, exportFormat);

            // Validate and load input file
            var validationResult = await _streamingService.ValidateFileFormatAsync(inputPath, cancellationToken);
            if (!validationResult.IsValid)
            {
                return new ExportResult
                {
                    Success = false,
                    ErrorMessage = $"Input file validation failed: {validationResult.Message}"
                };
            }

            var exportData = await LoadDataForExportAsync(inputPath, validationResult.FileType, cancellationToken);
            
            // Export based on format
            switch (exportFormat)
            {
                case ExportFormat.Json:
                    await ExportToJsonAsync(exportData, outputPath, options, cancellationToken);
                    break;
                case ExportFormat.Xml:
                    await ExportToXmlAsync(exportData, outputPath, options, cancellationToken);
                    break;
                case ExportFormat.Csv:
                    await ExportToCsvAsync(exportData, outputPath, options, cancellationToken);
                    break;
                default:
                    throw new NotSupportedException($"Export format {exportFormat} is not supported");
            }

            var result = new ExportResult
            {
                Success = true,
                ExportFormat = exportFormat,
                ItemsExported = exportData.Items?.Count ?? 0,
                Duration = DateTime.UtcNow - startTime
            };

            if (options.VerifyIntegrity)
            {
                result.OutputChecksum = await CalculateFileChecksumAsync(outputPath, cancellationToken);
            }

            _logger.LogInformation("Export completed successfully: {ItemsExported} items exported", result.ItemsExported);
            return result;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Export failed for {InputPath}", inputPath);
            
            return new ExportResult
            {
                Success = false,
                ErrorMessage = ex.Message,
                Duration = DateTime.UtcNow - startTime
            };
        }
    }

    /// <summary>
    /// Creates file format migration tools for version compatibility
    /// </summary>
    /// <param name="inputPath">Input file path</param>
    /// <param name="outputPath">Output file path</param>
    /// <param name="targetVersion">Target version</param>
    /// <param name="migrationOptions">Migration options</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Migration result</returns>
    public async Task<MigrationResult> MigrateFileVersionAsync(
        string inputPath,
        string outputPath,
        uint targetVersion,
        MigrationOptions? migrationOptions = null,
        CancellationToken cancellationToken = default)
    {
        migrationOptions ??= new MigrationOptions();
        var startTime = DateTime.UtcNow;

        try
        {
            _logger.LogInformation("Migrating file {InputPath} to version {TargetVersion}", inputPath, targetVersion);

            var validationResult = await _streamingService.ValidateFileFormatAsync(inputPath, cancellationToken);
            if (!validationResult.IsValid)
            {
                return new MigrationResult
                {
                    Success = false,
                    ErrorMessage = $"Input file validation failed: {validationResult.Message}"
                };
            }

            var sourceVersion = validationResult.FileVersion ?? 0;
            
            if (sourceVersion == targetVersion)
            {
                _logger.LogInformation("File is already at target version {TargetVersion}", targetVersion);
                File.Copy(inputPath, outputPath, true);
                
                return new MigrationResult
                {
                    Success = true,
                    SourceVersion = sourceVersion,
                    TargetVersion = targetVersion,
                    Duration = DateTime.UtcNow - startTime,
                    MigrationSteps = new List<string> { "No migration needed - versions match" }
                };
            }

            // Perform version-specific migration
            var result = await PerformVersionMigrationAsync(
                inputPath, outputPath, validationResult.FileType, 
                sourceVersion, targetVersion, migrationOptions, cancellationToken);

            result.Duration = DateTime.UtcNow - startTime;

            _logger.LogInformation("Migration completed: {Success} (v{SourceVersion} -> v{TargetVersion})",
                result.Success, sourceVersion, targetVersion);

            return result;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Migration failed for {InputPath}", inputPath);
            
            return new MigrationResult
            {
                Success = false,
                ErrorMessage = ex.Message,
                Duration = DateTime.UtcNow - startTime
            };
        }
    }

    #region Private Methods

    /// <summary>
    /// Performs the actual conversion between formats
    /// </summary>
    private async Task<ConversionResult> PerformConversionAsync(
        string inputPath, string outputPath, FileType sourceFormat, FileType targetFormat,
        ConversionOptions options, IProgress<ConversionProgress>? progress, CancellationToken cancellationToken)
    {
        var result = new ConversionResult
        {
            SourceFormat = sourceFormat,
            TargetFormat = targetFormat
        };

        try
        {
            switch (sourceFormat)
            {
                case FileType.OTB when targetFormat == FileType.DAT:
                    await ConvertOTBToDATAsync(inputPath, outputPath, options, progress, cancellationToken);
                    break;
                case FileType.DAT when targetFormat == FileType.OTB:
                    await ConvertDATToOTBAsync(inputPath, outputPath, options, progress, cancellationToken);
                    break;
                default:
                    throw new NotSupportedException($"Conversion from {sourceFormat} to {targetFormat} is not supported");
            }

            result.Success = true;
        }
        catch (Exception ex)
        {
            result.Success = false;
            result.ErrorMessage = ex.Message;
        }

        return result;
    }

    /// <summary>
    /// Converts OTB file to DAT format
    /// </summary>
    private async Task ConvertOTBToDATAsync(
        string inputPath, string outputPath, ConversionOptions options,
        IProgress<ConversionProgress>? progress, CancellationToken cancellationToken)
    {
        using var reader = await _streamingService.CreateOTBReaderAsync(inputPath, cancellationToken: cancellationToken);
        var header = await reader.ReadHeaderAsync(cancellationToken);
        var items = await reader.ReadItemsAsync(
            new Progress<FileReadProgress>(p => progress?.Report(new ConversionProgress
            {
                ProgressPercentage = p.ProgressPercentage * 0.8, // 80% for reading
                CurrentOperation = p.CurrentOperation
            })), cancellationToken);

        // Convert OTB items to DAT items
        var datItems = new List<DATItem>();
        var processedCount = 0;

        foreach (var otbItem in items)
        {
            var datItem = ConvertOTBItemToDAT(otbItem);
            datItems.Add(datItem);
            
            processedCount++;
            progress?.Report(new ConversionProgress
            {
                ProgressPercentage = 80 + (double)processedCount / items.Count * 20, // 20% for conversion
                CurrentOperation = $"Converting item {otbItem.Id}"
            });
        }

        // Write DAT file
        await WriteDATFileAsync(outputPath, datItems, header.ClientVersion, cancellationToken);
    }

    /// <summary>
    /// Converts DAT file to OTB format
    /// </summary>
    private async Task ConvertDATToOTBAsync(
        string inputPath, string outputPath, ConversionOptions options,
        IProgress<ConversionProgress>? progress, CancellationToken cancellationToken)
    {
        using var fileStream = new FileStream(inputPath, FileMode.Open, FileAccess.Read, FileShare.Read);
        using var reader = new DATReader(fileStream, _logger);
        
        var header = await reader.ReadHeaderAsync(cancellationToken);
        var items = await reader.ReadItemsAsync(header,
            new Progress<FileReadProgress>(p => progress?.Report(new ConversionProgress
            {
                ProgressPercentage = p.ProgressPercentage * 0.8,
                CurrentOperation = p.CurrentOperation
            })), cancellationToken);

        // Convert DAT items to OTB items
        var otbItems = new List<OTBItem>();
        var processedCount = 0;

        foreach (var datItem in items)
        {
            var otbItem = ConvertDATItemToOTB(datItem);
            otbItems.Add(otbItem);
            
            processedCount++;
            progress?.Report(new ConversionProgress
            {
                ProgressPercentage = 80 + (double)processedCount / items.Count * 20,
                CurrentOperation = $"Converting item {datItem.Id}"
            });
        }

        // Write OTB file
        await WriteOTBFileAsync(outputPath, otbItems, header.Version, cancellationToken);
    }

    /// <summary>
    /// Converts OTB item to DAT item
    /// </summary>
    private DATItem ConvertOTBItemToDAT(OTBItem otbItem)
    {
        var datItem = new DATItem
        {
            Id = otbItem.Id,
            Width = 1,
            Height = 1,
            Layers = 1,
            PatternX = 1,
            PatternY = 1,
            PatternZ = 1,
            Animations = 1
        };

        // Convert properties
        if (otbItem.HasProperty(OTBItemProperty.Speed))
        {
            datItem.GroundSpeed = otbItem.GetProperty<ushort>(OTBItemProperty.Speed);
            datItem.AddFlag(DATItemFlag.Ground);
        }

        if (otbItem.HasProperty(OTBItemProperty.Light))
        {
            var light = otbItem.GetProperty<OTBLightInfo>(OTBItemProperty.Light);
            datItem.LightLevel = light.Level;
            datItem.LightColor = light.Color;
            datItem.AddFlag(DATItemFlag.Light);
        }

        // Add other property conversions as needed
        return datItem;
    }

    /// <summary>
    /// Converts DAT item to OTB item
    /// </summary>
    private OTBItem ConvertDATItemToOTB(DATItem datItem)
    {
        var otbItem = new OTBItem
        {
            Id = datItem.Id,
            Type = DetermineOTBItemType(datItem)
        };

        // Convert properties
        if (datItem.GroundSpeed.HasValue)
        {
            otbItem.SetProperty(OTBItemProperty.Speed, datItem.GroundSpeed.Value);
        }

        if (datItem.LightLevel.HasValue && datItem.LightColor.HasValue)
        {
            otbItem.SetProperty(OTBItemProperty.Light, new OTBLightInfo
            {
                Level = datItem.LightLevel.Value,
                Color = datItem.LightColor.Value
            });
        }

        // Add other property conversions as needed
        return otbItem;
    }

    /// <summary>
    /// Determines OTB item type from DAT item flags
    /// </summary>
    private OTBItemType DetermineOTBItemType(DATItem datItem)
    {
        if (datItem.HasFlag(DATItemFlag.Ground))
            return OTBItemType.Ground;
        if (datItem.HasFlag(DATItemFlag.Container))
            return OTBItemType.Container;
        if (datItem.HasFlag(DATItemFlag.Writeable))
            return OTBItemType.Writeable;
        
        return OTBItemType.None;
    }

    /// <summary>
    /// Loads data for export based on file type
    /// </summary>
    private async Task<ExportData> LoadDataForExportAsync(string filePath, FileType fileType, CancellationToken cancellationToken)
    {
        var exportData = new ExportData { FileType = fileType };

        switch (fileType)
        {
            case FileType.OTB:
                using (var reader = await _streamingService.CreateOTBReaderAsync(filePath, cancellationToken: cancellationToken))
                {
                    var header = await reader.ReadHeaderAsync(cancellationToken);
                    var items = await reader.ReadItemsAsync(cancellationToken: cancellationToken);
                    
                    exportData.Header = new
                    {
                        header.Version,
                        header.ClientVersion,
                        header.BuildNumber,
                        header.Signature
                    };
                    exportData.Items = items.Cast<object>().ToList();
                }
                break;

            case FileType.DAT:
                using (var fileStream = new FileStream(filePath, FileMode.Open, FileAccess.Read, FileShare.Read))
                using (var reader = new DATReader(fileStream, _logger))
                {
                    var header = await reader.ReadHeaderAsync(cancellationToken);
                    var items = await reader.ReadItemsAsync(header, cancellationToken: cancellationToken);
                    
                    exportData.Header = new
                    {
                        header.Version,
                        header.ItemCount,
                        header.OutfitCount,
                        header.EffectCount,
                        header.MissileCount
                    };
                    exportData.Items = items.Cast<object>().ToList();
                }
                break;

            default:
                throw new NotSupportedException($"Export not supported for file type {fileType}");
        }

        return exportData;
    }

    /// <summary>
    /// Exports data to JSON format
    /// </summary>
    private async Task ExportToJsonAsync(ExportData data, string outputPath, ExportOptions options, CancellationToken cancellationToken)
    {
        var jsonOptions = new JsonSerializerOptions
        {
            WriteIndented = options.PrettyFormat,
            PropertyNamingPolicy = JsonNamingPolicy.CamelCase
        };

        var exportObject = new
        {
            FileType = data.FileType.ToString(),
            Header = data.Header,
            Items = data.Items,
            ExportedAt = DateTime.UtcNow,
            ExportedBy = "ItemEditor"
        };

        using var fileStream = new FileStream(outputPath, FileMode.Create, FileAccess.Write);
        await JsonSerializer.SerializeAsync(fileStream, exportObject, jsonOptions, cancellationToken);
    }

    /// <summary>
    /// Exports data to XML format
    /// </summary>
    private async Task ExportToXmlAsync(ExportData data, string outputPath, ExportOptions options, CancellationToken cancellationToken)
    {
        // XML export implementation would go here
        throw new NotImplementedException("XML export is not yet implemented");
    }

    /// <summary>
    /// Exports data to CSV format
    /// </summary>
    private async Task ExportToCsvAsync(ExportData data, string outputPath, ExportOptions options, CancellationToken cancellationToken)
    {
        // CSV export implementation would go here
        throw new NotImplementedException("CSV export is not yet implemented");
    }

    /// <summary>
    /// Writes DAT file
    /// </summary>
    private async Task WriteDATFileAsync(string outputPath, List<DATItem> items, uint clientVersion, CancellationToken cancellationToken)
    {
        // DAT file writing implementation would go here
        throw new NotImplementedException("DAT file writing is not yet implemented");
    }

    /// <summary>
    /// Writes OTB file
    /// </summary>
    private async Task WriteOTBFileAsync(string outputPath, List<OTBItem> items, uint version, CancellationToken cancellationToken)
    {
        // OTB file writing implementation would go here
        throw new NotImplementedException("OTB file writing is not yet implemented");
    }

    /// <summary>
    /// Performs version-specific migration
    /// </summary>
    private async Task<MigrationResult> PerformVersionMigrationAsync(
        string inputPath, string outputPath, FileType fileType,
        uint sourceVersion, uint targetVersion, MigrationOptions options, CancellationToken cancellationToken)
    {
        // Version migration implementation would go here
        throw new NotImplementedException("Version migration is not yet implemented");
    }

    /// <summary>
    /// Calculates SHA-256 checksum of a file
    /// </summary>
    private async Task<string> CalculateFileChecksumAsync(string filePath, CancellationToken cancellationToken)
    {
        using var sha256 = SHA256.Create();
        using var fileStream = new FileStream(filePath, FileMode.Open, FileAccess.Read, FileShare.Read);
        var hashBytes = await sha256.ComputeHashAsync(fileStream, cancellationToken);
        return Convert.ToHexString(hashBytes);
    }

    /// <summary>
    /// Raises the ProgressChanged event
    /// </summary>
    private void OnProgressChanged(ConversionProgressEventArgs args)
    {
        ProgressChanged?.Invoke(this, args);
    }

    /// <summary>
    /// Raises the ConversionCompleted event
    /// </summary>
    private void OnConversionCompleted(ConversionCompletedEventArgs args)
    {
        ConversionCompleted?.Invoke(this, args);
    }

    #endregion

    /// <inheritdoc />
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    /// <summary>
    /// Disposes the file format converter
    /// </summary>
    /// <param name="disposing">True if disposing managed resources</param>
    protected virtual void Dispose(bool disposing)
    {
        if (!_disposed && disposing)
        {
            _conversionLock?.Dispose();
            _disposed = true;
        }
    }
}