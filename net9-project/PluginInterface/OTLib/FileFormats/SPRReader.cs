using Microsoft.Extensions.Logging;
using System.Buffers;
using System.IO.Pipelines;

namespace PluginInterface.OTLib.FileFormats;

/// <summary>
/// Modern SPR file reader with streaming support for large sprite files
/// </summary>
public class SPRReader : IDisposable
{
    private readonly ILogger<SPRReader>? _logger;
    private readonly PipeReader _pipeReader;
    private readonly Stream _baseStream;
    private bool _disposed;

    /// <summary>
    /// Initializes a new instance of the SPRReader class
    /// </summary>
    /// <param name="stream">Stream to read from</param>
    /// <param name="logger">Logger instance</param>
    public SPRReader(Stream stream, ILogger<SPRReader>? logger = null)
    {
        _logger = logger;
        _baseStream = stream;
        _pipeReader = PipeReader.Create(stream);
    }

    /// <summary>
    /// Read SPR file header asynchronously
    /// </summary>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>SPR file header</returns>
    public async Task<SPRHeader> ReadHeaderAsync(CancellationToken cancellationToken = default)
    {
        try
        {
            _logger?.LogDebug("Reading SPR file header");

            var result = await _pipeReader.ReadAsync(cancellationToken);
            var buffer = result.Buffer;

            if (buffer.Length < SPRHeader.HeaderSize)
            {
                throw new InvalidDataException($"Insufficient data for SPR header. Expected {SPRHeader.HeaderSize} bytes, got {buffer.Length}");
            }

            var header = ReadHeaderFromBuffer(buffer);
            
            _pipeReader.AdvanceTo(buffer.GetPosition(SPRHeader.HeaderSize));
            
            _logger?.LogDebug("Successfully read SPR header: Version {Version}, Sprites {SpriteCount}", 
                header.Version, header.SpriteCount);

            return header;
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Failed to read SPR header");
            throw;
        }
    }

    /// <summary>
    /// Read sprite addresses from the SPR file
    /// </summary>
    /// <param name="header">SPR file header</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Array of sprite addresses</returns>
    public async Task<uint[]> ReadSpriteAddressesAsync(SPRHeader header, CancellationToken cancellationToken = default)
    {
        try
        {
            _logger?.LogDebug("Reading {SpriteCount} sprite addresses", header.SpriteCount);

            var addressCount = header.SpriteCount;
            var addresses = new uint[addressCount];
            var bytesNeeded = addressCount * 4; // 4 bytes per address

            var result = await _pipeReader.ReadAsync(cancellationToken);
            var buffer = result.Buffer;

            if (buffer.Length < bytesNeeded)
            {
                throw new InvalidDataException($"Insufficient data for sprite addresses. Expected {bytesNeeded} bytes, got {buffer.Length}");
            }

            var reader = new SequenceReader<byte>(buffer);
            
            for (int i = 0; i < addressCount; i++)
            {
                if (!reader.TryReadLittleEndian(out uint address))
                {
                    throw new InvalidDataException($"Failed to read sprite address at index {i}");
                }
                addresses[i] = address;
            }

            _pipeReader.AdvanceTo(buffer.GetPosition(bytesNeeded));

            _logger?.LogDebug("Successfully read {AddressCount} sprite addresses", addresses.Length);
            return addresses;
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Failed to read sprite addresses");
            throw;
        }
    }

    /// <summary>
    /// Read a specific sprite by ID
    /// </summary>
    /// <param name="spriteId">Sprite ID to read</param>
    /// <param name="addresses">Sprite addresses array</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Sprite data</returns>
    public async Task<SPRSprite?> ReadSpriteAsync(ushort spriteId, uint[] addresses, CancellationToken cancellationToken = default)
    {
        if (spriteId == 0 || spriteId > addresses.Length)
        {
            _logger?.LogWarning("Invalid sprite ID: {SpriteId}", spriteId);
            return null;
        }

        try
        {
            var address = addresses[spriteId - 1]; // Sprite IDs are 1-based
            
            if (address == 0)
            {
                // Empty sprite
                return new SPRSprite
                {
                    Id = spriteId,
                    Width = 32,
                    Height = 32,
                    PixelData = new byte[32 * 32 * 4] // RGBA
                };
            }

            // Seek to sprite position
            _baseStream.Seek(address, SeekOrigin.Begin);
            
            // Create new pipe reader for this position
            using var spriteReader = PipeReader.Create(_baseStream, new StreamPipeReaderOptions(leaveOpen: true));
            
            var result = await spriteReader.ReadAsync(cancellationToken);
            var buffer = result.Buffer;

            if (buffer.Length < 4) // Minimum size for sprite header
            {
                _logger?.LogWarning("Insufficient data for sprite {SpriteId} at address {Address}", spriteId, address);
                return null;
            }

            var sprite = await ReadSpriteFromBufferAsync(buffer, spriteId, cancellationToken);
            
            _logger?.LogTrace("Read sprite {SpriteId} ({Width}x{Height})", spriteId, sprite?.Width, sprite?.Height);
            
            return sprite;
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Failed to read sprite {SpriteId}", spriteId);
            return null;
        }
    }

    /// <summary>
    /// Read multiple sprites with progress reporting
    /// </summary>
    /// <param name="spriteIds">Sprite IDs to read</param>
    /// <param name="addresses">Sprite addresses array</param>
    /// <param name="progress">Progress reporter</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Dictionary of sprites by ID</returns>
    public async Task<Dictionary<ushort, SPRSprite>> ReadSpritesAsync(
        IEnumerable<ushort> spriteIds,
        uint[] addresses,
        IProgress<FileReadProgress>? progress = null,
        CancellationToken cancellationToken = default)
    {
        var sprites = new Dictionary<ushort, SPRSprite>();
        var spriteList = spriteIds.ToList();
        var totalSprites = spriteList.Count;
        var spritesRead = 0;

        try
        {
            _logger?.LogInformation("Reading {TotalSprites} sprites", totalSprites);

            foreach (var spriteId in spriteList)
            {
                cancellationToken.ThrowIfCancellationRequested();

                var sprite = await ReadSpriteAsync(spriteId, addresses, cancellationToken);
                if (sprite != null)
                {
                    sprites[spriteId] = sprite;
                }

                spritesRead++;

                // Report progress
                progress?.Report(new FileReadProgress
                {
                    ItemsRead = spritesRead,
                    CurrentOperation = $"Reading sprite {spriteId}",
                    ProgressPercentage = (double)spritesRead / totalSprites * 100
                });
            }

            _logger?.LogInformation("Successfully read {SpriteCount} sprites", sprites.Count);
            return sprites;
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Failed to read sprites");
            throw;
        }
    }

    /// <summary>
    /// Read sprite data from buffer
    /// </summary>
    /// <param name="buffer">Buffer containing sprite data</param>
    /// <param name="spriteId">Sprite ID</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Sprite data</returns>
    private async Task<SPRSprite?> ReadSpriteFromBufferAsync(
        ReadOnlySequence<byte> buffer, 
        ushort spriteId, 
        CancellationToken cancellationToken)
    {
        return await Task.Run(() =>
        {
            cancellationToken.ThrowIfCancellationRequested();

            var reader = new SequenceReader<byte>(buffer);

            // Read sprite dimensions (assuming 32x32 for now, could be dynamic)
            const int spriteWidth = 32;
            const int spriteHeight = 32;
            const int pixelsPerSprite = spriteWidth * spriteHeight;

            // Read color count
            if (!reader.TryReadLittleEndian(out ushort colorCount))
                return null;

            var sprite = new SPRSprite
            {
                Id = spriteId,
                Width = spriteWidth,
                Height = spriteHeight,
                PixelData = new byte[pixelsPerSprite * 4] // RGBA format
            };

            if (colorCount == 0)
            {
                // Transparent sprite
                return sprite;
            }

            // Read pixel data based on format
            if (colorCount <= 256)
            {
                // Indexed color format
                return ReadIndexedSprite(ref reader, sprite, colorCount);
            }
            else
            {
                // True color format
                return ReadTrueColorSprite(ref reader, sprite);
            }
        }, cancellationToken);
    }

    private SPRSprite? ReadIndexedSprite(ref SequenceReader<byte> reader, SPRSprite sprite, ushort colorCount)
    {
        // Read color palette
        var palette = new uint[colorCount];
        for (int i = 0; i < colorCount; i++)
        {
            if (!reader.TryRead(out byte r) || 
                !reader.TryRead(out byte g) || 
                !reader.TryRead(out byte b))
            {
                return null;
            }
            palette[i] = (uint)((255 << 24) | (r << 16) | (g << 8) | b); // ARGB
        }

        // Read pixel indices
        var pixelCount = sprite.Width * sprite.Height;
        for (int i = 0; i < pixelCount; i++)
        {
            if (!reader.TryRead(out byte colorIndex))
                break;

            if (colorIndex < palette.Length)
            {
                var color = palette[colorIndex];
                var pixelOffset = i * 4;
                
                sprite.PixelData[pixelOffset] = (byte)(color & 0xFF);     // B
                sprite.PixelData[pixelOffset + 1] = (byte)((color >> 8) & 0xFF);  // G
                sprite.PixelData[pixelOffset + 2] = (byte)((color >> 16) & 0xFF); // R
                sprite.PixelData[pixelOffset + 3] = (byte)((color >> 24) & 0xFF); // A
            }
        }

        return sprite;
    }

    private SPRSprite? ReadTrueColorSprite(ref SequenceReader<byte> reader, SPRSprite sprite)
    {
        var pixelCount = sprite.Width * sprite.Height;
        
        for (int i = 0; i < pixelCount; i++)
        {
            if (!reader.TryRead(out byte r) || 
                !reader.TryRead(out byte g) || 
                !reader.TryRead(out byte b) ||
                !reader.TryRead(out byte a))
            {
                break;
            }

            var pixelOffset = i * 4;
            sprite.PixelData[pixelOffset] = b;     // B
            sprite.PixelData[pixelOffset + 1] = g; // G
            sprite.PixelData[pixelOffset + 2] = r; // R
            sprite.PixelData[pixelOffset + 3] = a; // A
        }

        return sprite;
    }

    private static SPRHeader ReadHeaderFromBuffer(ReadOnlySequence<byte> buffer)
    {
        var reader = new SequenceReader<byte>(buffer);
        
        // Read signature (4 bytes)
        if (!reader.TryReadLittleEndian(out uint signature))
            throw new InvalidDataException("Failed to read SPR signature");

        // Read sprite count
        if (!reader.TryReadLittleEndian(out ushort spriteCount))
            throw new InvalidDataException("Failed to read sprite count");

        // Read version (2 bytes)
        if (!reader.TryReadLittleEndian(out ushort version))
            throw new InvalidDataException("Failed to read SPR version");

        return new SPRHeader
        {
            Signature = signature,
            SpriteCount = spriteCount,
            Version = version
        };
    }

    /// <inheritdoc />
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    /// <summary>
    /// Disposes the SPR reader
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