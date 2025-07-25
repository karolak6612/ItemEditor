using Microsoft.Extensions.Logging;
using System.Buffers;
using System.IO.Pipelines;

namespace PluginInterface.OTLib.FileFormats;

/// <summary>
/// Modern OTB file writer with async/await patterns and System.IO.Pipelines
/// </summary>
public class OTBWriter : IDisposable
{
    private readonly ILogger<OTBWriter>? _logger;
    private readonly PipeWriter _pipeWriter;
    private bool _disposed;

    /// <summary>
    /// Initializes a new instance of the OTBWriter class
    /// </summary>
    /// <param name="stream">Stream to write to</param>
    /// <param name="logger">Logger instance</param>
    public OTBWriter(Stream stream, ILogger<OTBWriter>? logger = null)
    {
        _logger = logger;
        _pipeWriter = PipeWriter.Create(stream);
    }

    /// <summary>
    /// Write OTB file header asynchronously
    /// </summary>
    /// <param name="header">Header to write</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task representing the async operation</returns>
    public async Task WriteHeaderAsync(OTBHeader header, CancellationToken cancellationToken = default)
    {
        try
        {
            _logger?.LogDebug("Writing OTB file header");

            var memory = _pipeWriter.GetMemory(OTBHeader.HeaderSize);
            var span = memory.Span;

            // Write signature
            BitConverter.TryWriteBytes(span[0..4], header.Signature);
            
            // Write version
            BitConverter.TryWriteBytes(span[4..8], header.Version);
            
            // Write client version
            BitConverter.TryWriteBytes(span[8..12], header.ClientVersion);
            
            // Write build number
            BitConverter.TryWriteBytes(span[12..16], header.BuildNumber);

            _pipeWriter.Advance(OTBHeader.HeaderSize);
            await _pipeWriter.FlushAsync(cancellationToken);

            _logger?.LogDebug("Successfully wrote OTB header");
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Failed to write OTB header");
            throw;
        }
    }

    /// <summary>
    /// Write items to the OTB file with progress reporting
    /// </summary>
    /// <param name="items">Items to write</param>
    /// <param name="progress">Progress reporter</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Task representing the async operation</returns>
    public async Task WriteItemsAsync(
        IEnumerable<OTBItem> items, 
        IProgress<FileWriteProgress>? progress = null, 
        CancellationToken cancellationToken = default)
    {
        var itemList = items.ToList();
        var totalItems = itemList.Count;
        var itemsWritten = 0;
        var totalBytesWritten = 0L;

        try
        {
            _logger?.LogInformation("Starting to write {TotalItems} OTB items", totalItems);

            foreach (var item in itemList)
            {
                cancellationToken.ThrowIfCancellationRequested();

                var bytesWritten = await WriteItemAsync(item, cancellationToken);
                itemsWritten++;
                totalBytesWritten += bytesWritten;

                // Report progress
                progress?.Report(new FileWriteProgress
                {
                    ItemsWritten = itemsWritten,
                    TotalItems = totalItems,
                    BytesWritten = totalBytesWritten,
                    CurrentOperation = $"Writing item {item.Id}",
                    ProgressPercentage = (double)itemsWritten / totalItems * 100
                });

                _logger?.LogTrace("Wrote item {ItemId} ({BytesWritten} bytes)", item.Id, bytesWritten);

                // Flush periodically to avoid excessive memory usage
                if (itemsWritten % 100 == 0)
                {
                    await _pipeWriter.FlushAsync(cancellationToken);
                }
            }

            // Final flush
            await _pipeWriter.FlushAsync(cancellationToken);

            _logger?.LogInformation("Successfully wrote {ItemsWritten} items to OTB file ({TotalBytes} bytes)", 
                itemsWritten, totalBytesWritten);
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Failed to write OTB items");
            throw;
        }
    }

    /// <summary>
    /// Write a single item to the file
    /// </summary>
    /// <param name="item">Item to write</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Number of bytes written</returns>
    private async Task<long> WriteItemAsync(OTBItem item, CancellationToken cancellationToken)
    {
        // Calculate item size first
        var propertiesSize = CalculatePropertiesSize(item.Properties);
        var itemSize = (ushort)(2 + propertiesSize); // 2 bytes for item ID + properties

        // Get memory for the item
        var totalSize = 3 + itemSize; // 1 byte type + 2 bytes size + item data
        var memory = _pipeWriter.GetMemory(totalSize);
        var span = memory.Span;
        var offset = 0;

        // Write item type
        span[offset++] = (byte)item.Type;

        // Write item size
        BitConverter.TryWriteBytes(span[offset..(offset + 2)], itemSize);
        offset += 2;

        // Write item ID
        BitConverter.TryWriteBytes(span[offset..(offset + 2)], item.Id);
        offset += 2;

        // Write properties
        offset += WriteProperties(item.Properties, span[offset..]);

        _pipeWriter.Advance(totalSize);
        
        return await Task.FromResult(totalSize);
    }

    /// <summary>
    /// Calculate the total size of properties in bytes
    /// </summary>
    /// <param name="properties">Properties to calculate size for</param>
    /// <returns>Total size in bytes</returns>
    private int CalculatePropertiesSize(Dictionary<OTBItemProperty, object> properties)
    {
        var size = 0;
        
        foreach (var (property, value) in properties)
        {
            size += 1; // Property type byte
            size += property switch
            {
                OTBItemProperty.ServerID => 2,
                OTBItemProperty.ClientID => 2,
                OTBItemProperty.Speed => 2,
                OTBItemProperty.Light => 4, // 2 bytes level + 2 bytes color
                OTBItemProperty.TopOrder => 1,
                OTBItemProperty.Name => 2 + (value as string)?.Length ?? 0, // 2 bytes length + string
                OTBItemProperty.SpriteHash => 16, // MD5 hash size
                _ => value switch
                {
                    byte[] bytes => 2 + bytes.Length, // 2 bytes length + data
                    string str => 2 + str.Length, // 2 bytes length + string
                    _ => 4 // Default to 4 bytes for unknown types
                }
            };
        }
        
        return size;
    }

    /// <summary>
    /// Write properties to the span
    /// </summary>
    /// <param name="properties">Properties to write</param>
    /// <param name="span">Span to write to</param>
    /// <returns>Number of bytes written</returns>
    private int WriteProperties(Dictionary<OTBItemProperty, object> properties, Span<byte> span)
    {
        var offset = 0;

        foreach (var (property, value) in properties)
        {
            // Write property type
            span[offset++] = (byte)property;

            // Write property value
            offset += property switch
            {
                OTBItemProperty.ServerID => WriteUInt16(span[offset..], (ushort)value),
                OTBItemProperty.ClientID => WriteUInt16(span[offset..], (ushort)value),
                OTBItemProperty.Speed => WriteUInt16(span[offset..], (ushort)value),
                OTBItemProperty.Light => WriteLightInfo(span[offset..], (OTBLightInfo)value),
                OTBItemProperty.TopOrder => WriteByte(span[offset..], (byte)value),
                OTBItemProperty.Name => WriteString(span[offset..], (string)value),
                OTBItemProperty.SpriteHash => WriteByteArray(span[offset..], (byte[])value, false),
                _ => WriteGenericProperty(span[offset..], value)
            };
        }

        return offset;
    }

    private int WriteUInt16(Span<byte> span, ushort value)
    {
        BitConverter.TryWriteBytes(span, value);
        return 2;
    }

    private int WriteByte(Span<byte> span, byte value)
    {
        span[0] = value;
        return 1;
    }

    private int WriteLightInfo(Span<byte> span, OTBLightInfo lightInfo)
    {
        BitConverter.TryWriteBytes(span[0..2], lightInfo.Level);
        BitConverter.TryWriteBytes(span[2..4], lightInfo.Color);
        return 4;
    }

    private int WriteString(Span<byte> span, string value)
    {
        var bytes = System.Text.Encoding.UTF8.GetBytes(value);
        var length = (ushort)bytes.Length;
        
        BitConverter.TryWriteBytes(span[0..2], length);
        bytes.CopyTo(span[2..]);
        
        return 2 + bytes.Length;
    }

    private int WriteByteArray(Span<byte> span, byte[] value, bool includeLength = true)
    {
        if (includeLength)
        {
            var length = (ushort)value.Length;
            BitConverter.TryWriteBytes(span[0..2], length);
            value.CopyTo(span[2..]);
            return 2 + value.Length;
        }
        else
        {
            value.CopyTo(span);
            return value.Length;
        }
    }

    private int WriteGenericProperty(Span<byte> span, object value)
    {
        return value switch
        {
            byte[] bytes => WriteByteArray(span, bytes),
            string str => WriteString(span, str),
            ushort us => WriteUInt16(span, us),
            byte b => WriteByte(span, b),
            _ => 0 // Unknown type, skip
        };
    }

    /// <inheritdoc />
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    /// <summary>
    /// Disposes the OTB writer
    /// </summary>
    /// <param name="disposing">True if disposing managed resources</param>
    protected virtual void Dispose(bool disposing)
    {
        if (!_disposed && disposing)
        {
            _pipeWriter?.Complete();
            _disposed = true;
        }
    }
}

/// <summary>
/// File write progress information
/// </summary>
public class FileWriteProgress
{
    /// <summary>
    /// Number of items written so far
    /// </summary>
    public int ItemsWritten { get; set; }

    /// <summary>
    /// Total number of items to write
    /// </summary>
    public int TotalItems { get; set; }

    /// <summary>
    /// Total bytes written
    /// </summary>
    public long BytesWritten { get; set; }

    /// <summary>
    /// Current operation description
    /// </summary>
    public string CurrentOperation { get; set; } = string.Empty;

    /// <summary>
    /// Progress percentage (0-100)
    /// </summary>
    public double ProgressPercentage { get; set; }

    /// <summary>
    /// Estimated time remaining
    /// </summary>
    public TimeSpan? EstimatedTimeRemaining { get; set; }
}