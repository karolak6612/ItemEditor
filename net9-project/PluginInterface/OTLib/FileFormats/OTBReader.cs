using Microsoft.Extensions.Logging;
using System.Buffers;
using System.IO.Pipelines;

namespace PluginInterface.OTLib.FileFormats;

/// <summary>
/// Modern OTB file reader with async/await patterns and System.IO.Pipelines
/// </summary>
public class OTBReader : IDisposable
{
    private readonly ILogger<OTBReader>? _logger;
    private readonly PipeReader _pipeReader;
    private bool _disposed;

    /// <summary>
    /// Initializes a new instance of the OTBReader class
    /// </summary>
    /// <param name="stream">Stream to read from</param>
    /// <param name="logger">Logger instance</param>
    public OTBReader(Stream stream, ILogger<OTBReader>? logger = null)
    {
        _logger = logger;
        _pipeReader = PipeReader.Create(stream);
    }

    /// <summary>
    /// Read OTB file header asynchronously
    /// </summary>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>OTB file header</returns>
    public async Task<OTBHeader> ReadHeaderAsync(CancellationToken cancellationToken = default)
    {
        try
        {
            _logger?.LogDebug("Reading OTB file header");

            var result = await _pipeReader.ReadAsync(cancellationToken);
            var buffer = result.Buffer;

            if (buffer.Length < OTBHeader.HeaderSize)
            {
                throw new InvalidDataException($"Insufficient data for OTB header. Expected {OTBHeader.HeaderSize} bytes, got {buffer.Length}");
            }

            var header = ReadHeaderFromBuffer(buffer);
            
            _pipeReader.AdvanceTo(buffer.GetPosition(OTBHeader.HeaderSize));
            
            _logger?.LogDebug("Successfully read OTB header: Version {Version}, Signature 0x{Signature:X8}", 
                header.Version, header.Signature);

            return header;
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Failed to read OTB header");
            throw;
        }
    }

    /// <summary>
    /// Read all items from the OTB file with progress reporting
    /// </summary>
    /// <param name="progress">Progress reporter</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Collection of items</returns>
    public async Task<List<OTBItem>> ReadItemsAsync(
        IProgress<FileReadProgress>? progress = null, 
        CancellationToken cancellationToken = default)
    {
        var items = new List<OTBItem>();
        var totalBytesRead = 0L;
        var itemCount = 0;

        try
        {
            _logger?.LogInformation("Starting to read OTB items");

            while (true)
            {
                cancellationToken.ThrowIfCancellationRequested();

                var result = await _pipeReader.ReadAsync(cancellationToken);
                var buffer = result.Buffer;

                if (buffer.IsEmpty && result.IsCompleted)
                    break;

                var (item, bytesConsumed) = await ReadNextItemAsync(buffer, cancellationToken);
                
                if (item != null)
                {
                    items.Add(item);
                    itemCount++;
                    totalBytesRead += bytesConsumed;

                    // Report progress
                    progress?.Report(new FileReadProgress
                    {
                        ItemsRead = itemCount,
                        BytesRead = totalBytesRead,
                        CurrentOperation = $"Reading item {item.Id}"
                    });

                    _logger?.LogTrace("Read item {ItemId} ({BytesConsumed} bytes)", item.Id, bytesConsumed);
                }

                _pipeReader.AdvanceTo(buffer.GetPosition(bytesConsumed));

                if (result.IsCompleted && bytesConsumed == 0)
                    break;
            }

            _logger?.LogInformation("Successfully read {ItemCount} items from OTB file", items.Count);
            return items;
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Failed to read OTB items");
            throw;
        }
    }

    /// <summary>
    /// Read a single item from the buffer
    /// </summary>
    /// <param name="buffer">Buffer to read from</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Item and bytes consumed</returns>
    private async Task<(OTBItem? item, long bytesConsumed)> ReadNextItemAsync(
        ReadOnlySequence<byte> buffer, 
        CancellationToken cancellationToken)
    {
        if (buffer.Length < 4) // Minimum size for item header
            return (null, 0);

        var reader = new SequenceReader<byte>(buffer);
        
        // Read item type
        if (!reader.TryRead(out byte itemType))
            return (null, 0);

        // Read item size
        if (!reader.TryReadLittleEndian(out ushort itemSize))
            return (null, 0);

        // Check if we have enough data for the complete item
        if (buffer.Length < itemSize + 3) // +3 for type and size fields
            return (null, 0);

        // Read item ID
        if (!reader.TryReadLittleEndian(out ushort itemId))
            return (null, 0);

        var item = new OTBItem
        {
            Type = (OTBItemType)itemType,
            Id = itemId,
            Properties = new Dictionary<OTBItemProperty, object>()
        };

        // Read item properties
        var propertiesSize = itemSize - 2; // Subtract item ID size
        var propertiesRead = 0;

        while (propertiesRead < propertiesSize && reader.Remaining > 0)
        {
            cancellationToken.ThrowIfCancellationRequested();

            if (!reader.TryRead(out byte propertyType))
                break;

            var property = (OTBItemProperty)propertyType;
            var propertyValue = ReadPropertyValue(property, ref reader, cancellationToken);
            
            if (propertyValue != null)
            {
                item.Properties[property] = propertyValue;
            }

            propertiesRead = (int)(reader.Consumed - 3); // Subtract header size
        }

        return (item, reader.Consumed);
    }

    /// <summary>
    /// Read property value based on property type
    /// </summary>
    /// <param name="property">Property type</param>
    /// <param name="reader">Sequence reader</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Property value</returns>
    private object? ReadPropertyValue(
        OTBItemProperty property, 
        ref SequenceReader<byte> reader, 
        CancellationToken cancellationToken)
    {
        cancellationToken.ThrowIfCancellationRequested();

        return property switch
        {
            OTBItemProperty.ServerID => reader.TryReadLittleEndian(out ushort serverId) ? serverId : null,
            OTBItemProperty.ClientID => reader.TryReadLittleEndian(out ushort clientId) ? clientId : null,
            OTBItemProperty.Speed => reader.TryReadLittleEndian(out ushort speed) ? speed : null,
            OTBItemProperty.Light => ReadLightProperty(ref reader),
            OTBItemProperty.TopOrder => reader.TryRead(out byte topOrder) ? topOrder : null,
            OTBItemProperty.Name => ReadStringProperty(ref reader),
            OTBItemProperty.SpriteHash => ReadSpriteHashProperty(ref reader),
            _ => ReadGenericProperty(ref reader)
        };
    }

    private object? ReadLightProperty(ref SequenceReader<byte> reader)
    {
        if (reader.TryReadLittleEndian(out ushort lightLevel) && 
            reader.TryReadLittleEndian(out ushort lightColor))
        {
            return new OTBLightInfo { Level = lightLevel, Color = lightColor };
        }
        return null;
    }

    private string? ReadStringProperty(ref SequenceReader<byte> reader)
    {
        if (!reader.TryReadLittleEndian(out ushort length))
            return null;

        if (reader.Remaining < length)
            return null;

        var stringBytes = new byte[length];
        if (!reader.TryCopyTo(stringBytes))
            return null;

        reader.Advance(length);
        return System.Text.Encoding.UTF8.GetString(stringBytes);
    }

    private byte[]? ReadSpriteHashProperty(ref SequenceReader<byte> reader)
    {
        const int hashSize = 16; // MD5 hash size
        if (reader.Remaining < hashSize)
            return null;

        var hash = new byte[hashSize];
        if (!reader.TryCopyTo(hash))
            return null;

        reader.Advance(hashSize);
        return hash;
    }

    private byte[]? ReadGenericProperty(ref SequenceReader<byte> reader)
    {
        if (!reader.TryReadLittleEndian(out ushort length))
            return null;

        if (reader.Remaining < length)
            return null;

        var data = new byte[length];
        if (!reader.TryCopyTo(data))
            return null;

        reader.Advance(length);
        return data;
    }

    private static OTBHeader ReadHeaderFromBuffer(ReadOnlySequence<byte> buffer)
    {
        var reader = new SequenceReader<byte>(buffer);
        
        // Read signature (4 bytes)
        if (!reader.TryReadLittleEndian(out uint signature))
            throw new InvalidDataException("Failed to read OTB signature");

        // Validate signature
        if (signature != OTBHeader.ValidSignature)
            throw new InvalidDataException($"Invalid OTB signature: 0x{signature:X8}");

        // Read version info
        if (!reader.TryReadLittleEndian(out uint version))
            throw new InvalidDataException("Failed to read OTB version");

        if (!reader.TryReadLittleEndian(out uint clientVersion))
            throw new InvalidDataException("Failed to read client version");

        if (!reader.TryReadLittleEndian(out uint buildNumber))
            throw new InvalidDataException("Failed to read build number");

        return new OTBHeader
        {
            Signature = signature,
            Version = version,
            ClientVersion = clientVersion,
            BuildNumber = buildNumber
        };
    }

    /// <inheritdoc />
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    /// <summary>
    /// Disposes the OTB reader
    /// </summary>
    /// <param name="disposing">True if disposing managed resources</param>
    protected virtual void Dispose(bool disposing)
    {
        if (!_disposed && disposing)
        {
            _pipeReader?.Complete();
            _disposed = true;
        }
    }
}