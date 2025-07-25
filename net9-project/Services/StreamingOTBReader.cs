using Microsoft.Extensions.Logging;
using System.Buffers;
using System.IO.MemoryMappedFiles;
using System.IO.Pipelines;
using PluginInterface.OTLib.FileFormats;

namespace ItemEditor.Services;

/// <summary>
/// High-performance streaming OTB reader with memory-mapped file support
/// </summary>
public class StreamingOTBReader : IDisposable
{
    private readonly ILogger? _logger;
    private readonly PipeReader? _pipeReader;
    private readonly MemoryMappedViewAccessor? _memoryAccessor;
    private readonly Stream? _baseStream;
    private readonly bool _isMemoryMapped;
    private long _position;
    private bool _disposed;

    /// <summary>
    /// Initializes a new instance with a stream for pipeline-based reading
    /// </summary>
    /// <param name="stream">Stream to read from</param>
    /// <param name="logger">Logger instance</param>
    public StreamingOTBReader(Stream stream, ILogger? logger = null)
    {
        _logger = logger;
        _baseStream = stream;
        _pipeReader = PipeReader.Create(stream, new StreamPipeReaderOptions(leaveOpen: false));
        _isMemoryMapped = false;
        _position = 0;
    }

    /// <summary>
    /// Initializes a new instance with memory-mapped file accessor
    /// </summary>
    /// <param name="accessor">Memory-mapped view accessor</param>
    /// <param name="logger">Logger instance</param>
    public StreamingOTBReader(MemoryMappedViewAccessor accessor, ILogger? logger = null)
    {
        _logger = logger;
        _memoryAccessor = accessor;
        _isMemoryMapped = true;
        _position = 0;
    }

    /// <summary>
    /// Gets the current position in the file
    /// </summary>
    public long Position => _position;

    /// <summary>
    /// Gets whether this reader uses memory mapping
    /// </summary>
    public bool IsMemoryMapped => _isMemoryMapped;

    /// <summary>
    /// Reads the OTB file header asynchronously
    /// </summary>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>OTB file header</returns>
    public async Task<OTBHeader> ReadHeaderAsync(CancellationToken cancellationToken = default)
    {
        try
        {
            _logger?.LogDebug("Reading OTB file header (MemoryMapped: {IsMemoryMapped})", _isMemoryMapped);

            if (_isMemoryMapped)
            {
                return ReadHeaderFromMemory();
            }
            else
            {
                return await ReadHeaderFromStreamAsync(cancellationToken);
            }
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Failed to read OTB header");
            throw;
        }
    }

    /// <summary>
    /// Reads all items from the OTB file with progress reporting
    /// </summary>
    /// <param name="progress">Progress reporter</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Collection of OTB items</returns>
    public async Task<List<OTBItem>> ReadItemsAsync(
        IProgress<FileReadProgress>? progress = null,
        CancellationToken cancellationToken = default)
    {
        try
        {
            _logger?.LogInformation("Starting to read OTB items (MemoryMapped: {IsMemoryMapped})", _isMemoryMapped);

            if (_isMemoryMapped)
            {
                return await ReadItemsFromMemoryAsync(progress, cancellationToken);
            }
            else
            {
                return await ReadItemsFromStreamAsync(progress, cancellationToken);
            }
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Failed to read OTB items");
            throw;
        }
    }

    /// <summary>
    /// Reads a specific range of items for efficient partial loading
    /// </summary>
    /// <param name="startIndex">Starting item index</param>
    /// <param name="count">Number of items to read</param>
    /// <param name="progress">Progress reporter</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Collection of OTB items</returns>
    public async Task<List<OTBItem>> ReadItemRangeAsync(
        int startIndex,
        int count,
        IProgress<FileReadProgress>? progress = null,
        CancellationToken cancellationToken = default)
    {
        try
        {
            _logger?.LogDebug("Reading OTB item range: {StartIndex}-{EndIndex}", startIndex, startIndex + count - 1);

            if (_isMemoryMapped)
            {
                return await ReadItemRangeFromMemoryAsync(startIndex, count, progress, cancellationToken);
            }
            else
            {
                // For streaming, we need to read sequentially up to the desired range
                return await ReadItemRangeFromStreamAsync(startIndex, count, progress, cancellationToken);
            }
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Failed to read OTB item range");
            throw;
        }
    }

    /// <summary>
    /// Seeks to a specific position in the file
    /// </summary>
    /// <param name="position">Position to seek to</param>
    /// <param name="cancellationToken">Cancellation token</param>
    public async Task SeekAsync(long position, CancellationToken cancellationToken = default)
    {
        if (_isMemoryMapped)
        {
            _position = Math.Max(0, Math.Min(position, _memoryAccessor!.Capacity));
        }
        else if (_baseStream != null)
        {
            _baseStream.Seek(position, SeekOrigin.Begin);
            _position = position;
        }

        await Task.CompletedTask;
    }

    #region Memory-Mapped File Operations

    /// <summary>
    /// Reads header from memory-mapped file
    /// </summary>
    /// <returns>OTB header</returns>
    private OTBHeader ReadHeaderFromMemory()
    {
        if (_memoryAccessor == null)
            throw new InvalidOperationException("Memory accessor is null");

        if (_memoryAccessor.Capacity < OTBHeader.HeaderSize)
            throw new InvalidDataException($"Insufficient data for OTB header. Expected {OTBHeader.HeaderSize} bytes");

        var signature = _memoryAccessor.ReadUInt32(_position);
        var version = _memoryAccessor.ReadUInt32(_position + 4);
        var clientVersion = _memoryAccessor.ReadUInt32(_position + 8);
        var buildNumber = _memoryAccessor.ReadUInt32(_position + 12);

        _position += OTBHeader.HeaderSize;

        var header = new OTBHeader
        {
            Signature = signature,
            Version = version,
            ClientVersion = clientVersion,
            BuildNumber = buildNumber
        };

        _logger?.LogDebug("Read OTB header from memory: Version {Version}, Signature 0x{Signature:X8}",
            header.Version, header.Signature);

        return header;
    }

    /// <summary>
    /// Reads items from memory-mapped file
    /// </summary>
    /// <param name="progress">Progress reporter</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>List of OTB items</returns>
    private async Task<List<OTBItem>> ReadItemsFromMemoryAsync(
        IProgress<FileReadProgress>? progress,
        CancellationToken cancellationToken)
    {
        var items = new List<OTBItem>();
        var itemCount = 0;
        var totalBytes = _memoryAccessor!.Capacity;

        while (_position < totalBytes)
        {
            cancellationToken.ThrowIfCancellationRequested();

            var item = ReadNextItemFromMemory();
            if (item == null)
                break;

            items.Add(item);
            itemCount++;

            // Report progress every 100 items
            if (itemCount % 100 == 0)
            {
                progress?.Report(new FileReadProgress
                {
                    ItemsRead = itemCount,
                    BytesRead = _position,
                    CurrentOperation = $"Reading item {item.Id}",
                    ProgressPercentage = (double)_position / totalBytes * 100
                });
            }

            // Yield control periodically for responsiveness
            if (itemCount % 1000 == 0)
            {
                await Task.Yield();
            }
        }

        _logger?.LogInformation("Read {ItemCount} items from memory-mapped OTB file", items.Count);
        return items;
    }

    /// <summary>
    /// Reads a range of items from memory-mapped file
    /// </summary>
    /// <param name="startIndex">Starting item index</param>
    /// <param name="count">Number of items to read</param>
    /// <param name="progress">Progress reporter</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>List of OTB items</returns>
    private async Task<List<OTBItem>> ReadItemRangeFromMemoryAsync(
        int startIndex,
        int count,
        IProgress<FileReadProgress>? progress,
        CancellationToken cancellationToken)
    {
        var items = new List<OTBItem>();
        var currentIndex = 0;
        var itemsRead = 0;

        // Skip to start index
        while (_position < _memoryAccessor!.Capacity && currentIndex < startIndex)
        {
            cancellationToken.ThrowIfCancellationRequested();
            
            var item = ReadNextItemFromMemory();
            if (item == null)
                break;
            
            currentIndex++;
        }

        // Read the requested range
        while (_position < _memoryAccessor.Capacity && itemsRead < count)
        {
            cancellationToken.ThrowIfCancellationRequested();

            var item = ReadNextItemFromMemory();
            if (item == null)
                break;

            items.Add(item);
            itemsRead++;

            progress?.Report(new FileReadProgress
            {
                ItemsRead = itemsRead,
                BytesRead = _position,
                CurrentOperation = $"Reading item {item.Id}",
                ProgressPercentage = (double)itemsRead / count * 100
            });

            if (itemsRead % 100 == 0)
            {
                await Task.Yield();
            }
        }

        return items;
    }

    /// <summary>
    /// Reads the next item from memory-mapped file
    /// </summary>
    /// <returns>OTB item or null if end of file</returns>
    private OTBItem? ReadNextItemFromMemory()
    {
        if (_position + 3 >= _memoryAccessor!.Capacity) // Minimum item size
            return null;

        var itemType = _memoryAccessor.ReadByte(_position);
        var itemSize = _memoryAccessor.ReadUInt16(_position + 1);
        var itemId = _memoryAccessor.ReadUInt16(_position + 3);

        if (_position + itemSize + 3 > _memoryAccessor.Capacity)
            return null;

        var item = new OTBItem
        {
            Type = (OTBItemType)itemType,
            Id = itemId,
            Properties = new Dictionary<OTBItemProperty, object>()
        };

        _position += 5; // Skip type, size, and ID

        // Read properties
        var propertiesEnd = _position + itemSize - 2; // Subtract ID size
        while (_position < propertiesEnd)
        {
            var propertyType = (OTBItemProperty)_memoryAccessor.ReadByte(_position);
            _position++;

            var propertyValue = ReadPropertyFromMemory(propertyType);
            if (propertyValue != null)
            {
                item.Properties[propertyType] = propertyValue;
            }
        }

        return item;
    }

    /// <summary>
    /// Reads property value from memory-mapped file
    /// </summary>
    /// <param name="property">Property type</param>
    /// <returns>Property value</returns>
    private object? ReadPropertyFromMemory(OTBItemProperty property)
    {
        return property switch
        {
            OTBItemProperty.ServerID => ReadUInt16FromMemory(),
            OTBItemProperty.ClientID => ReadUInt16FromMemory(),
            OTBItemProperty.Speed => ReadUInt16FromMemory(),
            OTBItemProperty.Light => ReadLightFromMemory(),
            OTBItemProperty.TopOrder => ReadByteFromMemory(),
            OTBItemProperty.Name => ReadStringFromMemory(),
            OTBItemProperty.SpriteHash => ReadSpriteHashFromMemory(),
            _ => ReadGenericPropertyFromMemory()
        };
    }

    private ushort? ReadUInt16FromMemory()
    {
        if (_position + 2 > _memoryAccessor!.Capacity)
            return null;
        
        var value = _memoryAccessor.ReadUInt16(_position);
        _position += 2;
        return value;
    }

    private byte? ReadByteFromMemory()
    {
        if (_position >= _memoryAccessor!.Capacity)
            return null;
        
        var value = _memoryAccessor.ReadByte(_position);
        _position++;
        return value;
    }

    private OTBLightInfo? ReadLightFromMemory()
    {
        if (_position + 4 > _memoryAccessor!.Capacity)
            return null;
        
        var level = _memoryAccessor.ReadUInt16(_position);
        var color = _memoryAccessor.ReadUInt16(_position + 2);
        _position += 4;
        
        return new OTBLightInfo { Level = level, Color = color };
    }

    private string? ReadStringFromMemory()
    {
        if (_position + 2 > _memoryAccessor!.Capacity)
            return null;
        
        var length = _memoryAccessor.ReadUInt16(_position);
        _position += 2;
        
        if (_position + length > _memoryAccessor.Capacity)
            return null;
        
        var bytes = new byte[length];
        _memoryAccessor.ReadArray(_position, bytes, 0, length);
        _position += length;
        
        return System.Text.Encoding.UTF8.GetString(bytes);
    }

    private byte[]? ReadSpriteHashFromMemory()
    {
        const int hashSize = 16; // MD5 hash size
        if (_position + hashSize > _memoryAccessor!.Capacity)
            return null;
        
        var hash = new byte[hashSize];
        _memoryAccessor.ReadArray(_position, hash, 0, hashSize);
        _position += hashSize;
        
        return hash;
    }

    private byte[]? ReadGenericPropertyFromMemory()
    {
        if (_position + 2 > _memoryAccessor!.Capacity)
            return null;
        
        var length = _memoryAccessor.ReadUInt16(_position);
        _position += 2;
        
        if (_position + length > _memoryAccessor.Capacity)
            return null;
        
        var data = new byte[length];
        _memoryAccessor.ReadArray(_position, data, 0, length);
        _position += length;
        
        return data;
    }

    #endregion

    #region Stream-Based Operations

    /// <summary>
    /// Reads header from stream using pipelines
    /// </summary>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>OTB header</returns>
    private async Task<OTBHeader> ReadHeaderFromStreamAsync(CancellationToken cancellationToken)
    {
        var result = await _pipeReader!.ReadAsync(cancellationToken);
        var buffer = result.Buffer;

        if (buffer.Length < OTBHeader.HeaderSize)
        {
            throw new InvalidDataException($"Insufficient data for OTB header. Expected {OTBHeader.HeaderSize} bytes, got {buffer.Length}");
        }

        var header = ReadHeaderFromBuffer(buffer);
        _pipeReader.AdvanceTo(buffer.GetPosition(OTBHeader.HeaderSize));
        _position += OTBHeader.HeaderSize;

        return header;
    }

    /// <summary>
    /// Reads items from stream using pipelines
    /// </summary>
    /// <param name="progress">Progress reporter</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>List of OTB items</returns>
    private async Task<List<OTBItem>> ReadItemsFromStreamAsync(
        IProgress<FileReadProgress>? progress,
        CancellationToken cancellationToken)
    {
        var items = new List<OTBItem>();
        var itemCount = 0;
        var totalBytesRead = 0L;

        while (true)
        {
            cancellationToken.ThrowIfCancellationRequested();

            var result = await _pipeReader!.ReadAsync(cancellationToken);
            var buffer = result.Buffer;

            if (buffer.IsEmpty && result.IsCompleted)
                break;

            var (item, bytesConsumed) = ReadNextItemFromBuffer(buffer);

            if (item != null)
            {
                items.Add(item);
                itemCount++;
                totalBytesRead += bytesConsumed;
                _position += bytesConsumed;

                // Report progress
                if (itemCount % 100 == 0)
                {
                    progress?.Report(new FileReadProgress
                    {
                        ItemsRead = itemCount,
                        BytesRead = totalBytesRead,
                        CurrentOperation = $"Reading item {item.Id}"
                    });
                }
            }

            _pipeReader.AdvanceTo(buffer.GetPosition(bytesConsumed));

            if (result.IsCompleted && bytesConsumed == 0)
                break;

            // Yield control periodically
            if (itemCount % 1000 == 0)
            {
                await Task.Yield();
            }
        }

        return items;
    }

    /// <summary>
    /// Reads a range of items from stream
    /// </summary>
    /// <param name="startIndex">Starting item index</param>
    /// <param name="count">Number of items to read</param>
    /// <param name="progress">Progress reporter</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>List of OTB items</returns>
    private async Task<List<OTBItem>> ReadItemRangeFromStreamAsync(
        int startIndex,
        int count,
        IProgress<FileReadProgress>? progress,
        CancellationToken cancellationToken)
    {
        var items = new List<OTBItem>();
        var currentIndex = 0;
        var itemsRead = 0;

        while (true)
        {
            cancellationToken.ThrowIfCancellationRequested();

            var result = await _pipeReader!.ReadAsync(cancellationToken);
            var buffer = result.Buffer;

            if (buffer.IsEmpty && result.IsCompleted)
                break;

            var (item, bytesConsumed) = ReadNextItemFromBuffer(buffer);

            if (item != null)
            {
                if (currentIndex >= startIndex && itemsRead < count)
                {
                    items.Add(item);
                    itemsRead++;

                    progress?.Report(new FileReadProgress
                    {
                        ItemsRead = itemsRead,
                        CurrentOperation = $"Reading item {item.Id}",
                        ProgressPercentage = (double)itemsRead / count * 100
                    });

                    if (itemsRead >= count)
                        break;
                }

                currentIndex++;
                _position += bytesConsumed;
            }

            _pipeReader.AdvanceTo(buffer.GetPosition(bytesConsumed));

            if (result.IsCompleted && bytesConsumed == 0)
                break;
        }

        return items;
    }

    /// <summary>
    /// Reads the next item from buffer
    /// </summary>
    /// <param name="buffer">Buffer to read from</param>
    /// <returns>Item and bytes consumed</returns>
    private (OTBItem? item, long bytesConsumed) ReadNextItemFromBuffer(ReadOnlySequence<byte> buffer)
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
            if (!reader.TryRead(out byte propertyType))
                break;

            var property = (OTBItemProperty)propertyType;
            var propertyValue = ReadPropertyFromBuffer(property, ref reader);

            if (propertyValue != null)
            {
                item.Properties[property] = propertyValue;
            }

            propertiesRead = (int)(reader.Consumed - 3); // Subtract header size
        }

        return (item, reader.Consumed);
    }

    /// <summary>
    /// Reads property value from buffer
    /// </summary>
    /// <param name="property">Property type</param>
    /// <param name="reader">Sequence reader</param>
    /// <returns>Property value</returns>
    private object? ReadPropertyFromBuffer(OTBItemProperty property, ref SequenceReader<byte> reader)
    {
        return property switch
        {
            OTBItemProperty.ServerID => reader.TryReadLittleEndian(out ushort serverId) ? serverId : null,
            OTBItemProperty.ClientID => reader.TryReadLittleEndian(out ushort clientId) ? clientId : null,
            OTBItemProperty.Speed => reader.TryReadLittleEndian(out ushort speed) ? speed : null,
            OTBItemProperty.Light => ReadLightFromBuffer(ref reader),
            OTBItemProperty.TopOrder => reader.TryRead(out byte topOrder) ? topOrder : null,
            OTBItemProperty.Name => ReadStringFromBuffer(ref reader),
            OTBItemProperty.SpriteHash => ReadSpriteHashFromBuffer(ref reader),
            _ => ReadGenericPropertyFromBuffer(ref reader)
        };
    }

    private OTBLightInfo? ReadLightFromBuffer(ref SequenceReader<byte> reader)
    {
        if (reader.TryReadLittleEndian(out ushort lightLevel) &&
            reader.TryReadLittleEndian(out ushort lightColor))
        {
            return new OTBLightInfo { Level = lightLevel, Color = lightColor };
        }
        return null;
    }

    private string? ReadStringFromBuffer(ref SequenceReader<byte> reader)
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

    private byte[]? ReadSpriteHashFromBuffer(ref SequenceReader<byte> reader)
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

    private byte[]? ReadGenericPropertyFromBuffer(ref SequenceReader<byte> reader)
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

    #endregion

    /// <inheritdoc />
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    /// <summary>
    /// Disposes the streaming OTB reader
    /// </summary>
    /// <param name="disposing">True if disposing managed resources</param>
    protected virtual void Dispose(bool disposing)
    {
        if (!_disposed && disposing)
        {
            _pipeReader?.Complete();
            _baseStream?.Dispose();
            _memoryAccessor?.Dispose();
            _disposed = true;
        }
    }
}