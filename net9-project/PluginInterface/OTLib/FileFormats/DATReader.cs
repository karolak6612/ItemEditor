using Microsoft.Extensions.Logging;
using System.Buffers;
using System.IO.Pipelines;

namespace PluginInterface.OTLib.FileFormats;

/// <summary>
/// Modern DAT file reader with streaming support for large files
/// </summary>
public class DATReader : IDisposable
{
    private readonly ILogger<DATReader>? _logger;
    private readonly PipeReader _pipeReader;
    private bool _disposed;

    /// <summary>
    /// Initializes a new instance of the DATReader class
    /// </summary>
    /// <param name="stream">Stream to read from</param>
    /// <param name="logger">Logger instance</param>
    public DATReader(Stream stream, ILogger<DATReader>? logger = null)
    {
        _logger = logger;
        _pipeReader = PipeReader.Create(stream);
    }

    /// <summary>
    /// Read DAT file header asynchronously
    /// </summary>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>DAT file header</returns>
    public async Task<DATHeader> ReadHeaderAsync(CancellationToken cancellationToken = default)
    {
        try
        {
            _logger?.LogDebug("Reading DAT file header");

            var result = await _pipeReader.ReadAsync(cancellationToken);
            var buffer = result.Buffer;

            if (buffer.Length < DATHeader.HeaderSize)
            {
                throw new InvalidDataException($"Insufficient data for DAT header. Expected {DATHeader.HeaderSize} bytes, got {buffer.Length}");
            }

            var header = ReadHeaderFromBuffer(buffer);
            
            _pipeReader.AdvanceTo(buffer.GetPosition(DATHeader.HeaderSize));
            
            _logger?.LogDebug("Successfully read DAT header: Version {Version}, Items {ItemCount}, Outfits {OutfitCount}, Effects {EffectCount}, Missiles {MissileCount}", 
                header.Version, header.ItemCount, header.OutfitCount, header.EffectCount, header.MissileCount);

            return header;
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Failed to read DAT header");
            throw;
        }
    }

    /// <summary>
    /// Read all items from the DAT file with progress reporting
    /// </summary>
    /// <param name="header">DAT file header</param>
    /// <param name="progress">Progress reporter</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Collection of DAT items</returns>
    public async Task<List<DATItem>> ReadItemsAsync(
        DATHeader header,
        IProgress<FileReadProgress>? progress = null, 
        CancellationToken cancellationToken = default)
    {
        var items = new List<DATItem>();
        var totalItems = header.ItemCount;
        var itemsRead = 0;
        var totalBytesRead = 0L;

        try
        {
            _logger?.LogInformation("Starting to read {TotalItems} DAT items", totalItems);

            for (int i = 0; i < totalItems; i++)
            {
                cancellationToken.ThrowIfCancellationRequested();

                var result = await _pipeReader.ReadAsync(cancellationToken);
                var buffer = result.Buffer;

                if (buffer.IsEmpty && result.IsCompleted)
                    break;

                var (item, bytesConsumed) = await ReadNextItemAsync(buffer, (ushort)(i + 100), cancellationToken);
                
                if (item != null)
                {
                    items.Add(item);
                    itemsRead++;
                    totalBytesRead += bytesConsumed;

                    // Report progress
                    progress?.Report(new FileReadProgress
                    {
                        ItemsRead = itemsRead,
                        BytesRead = totalBytesRead,
                        CurrentOperation = $"Reading item {item.Id}",
                        ProgressPercentage = (double)itemsRead / totalItems * 100
                    });

                    _logger?.LogTrace("Read DAT item {ItemId} ({BytesConsumed} bytes)", item.Id, bytesConsumed);
                }

                _pipeReader.AdvanceTo(buffer.GetPosition(bytesConsumed));

                if (result.IsCompleted && bytesConsumed == 0)
                    break;
            }

            _logger?.LogInformation("Successfully read {ItemCount} items from DAT file", items.Count);
            return items;
        }
        catch (Exception ex)
        {
            _logger?.LogError(ex, "Failed to read DAT items");
            throw;
        }
    }

    /// <summary>
    /// Read a single DAT item from the buffer
    /// </summary>
    /// <param name="buffer">Buffer to read from</param>
    /// <param name="itemId">Item ID</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Item and bytes consumed</returns>
    private async Task<(DATItem? item, long bytesConsumed)> ReadNextItemAsync(
        ReadOnlySequence<byte> buffer, 
        ushort itemId,
        CancellationToken cancellationToken)
    {
        if (buffer.Length < 1) // Minimum size for flags
            return (null, 0);

        var reader = new SequenceReader<byte>(buffer);
        var startPosition = reader.Consumed;

        var item = new DATItem
        {
            Id = itemId,
            Flags = new List<DATItemFlag>(),
            SpriteIds = new List<ushort>()
        };

        // Read item flags
        while (reader.Remaining > 0)
        {
            cancellationToken.ThrowIfCancellationRequested();

            if (!reader.TryRead(out byte flagByte))
                break;

            var flag = (DATItemFlag)flagByte;
            
            if (flag == DATItemFlag.LastFlag)
                break;

            item.Flags.Add(flag);

            // Read flag-specific data
            var flagDataSize = ReadFlagData(flag, ref reader, item, cancellationToken);
            if (flagDataSize == 0 && reader.Remaining == 0)
                break; // End of data
        }

        // Read sprite information
        if (reader.TryRead(out byte width) && 
            reader.TryRead(out byte height) && 
            reader.TryRead(out byte layers) && 
            reader.TryRead(out byte patternX) && 
            reader.TryRead(out byte patternY) && 
            reader.TryRead(out byte patternZ) && 
            reader.TryRead(out byte animations))
        {
            item.Width = width;
            item.Height = height;
            item.Layers = layers;
            item.PatternX = patternX;
            item.PatternY = patternY;
            item.PatternZ = patternZ;
            item.Animations = animations;

            // Calculate total sprites
            var totalSprites = width * height * layers * patternX * patternY * patternZ * Math.Max(animations, 1);

            // Read sprite IDs
            for (int i = 0; i < totalSprites; i++)
            {
                if (reader.TryReadLittleEndian(out ushort spriteId))
                {
                    item.SpriteIds.Add(spriteId);
                }
                else
                {
                    break; // Not enough data
                }
            }
        }

        var bytesConsumed = reader.Consumed - startPosition;
        return (item, bytesConsumed);
    }

    /// <summary>
    /// Read flag-specific data
    /// </summary>
    /// <param name="flag">Item flag</param>
    /// <param name="reader">Sequence reader</param>
    /// <param name="item">Item being read</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Bytes consumed for flag data</returns>
    private int ReadFlagData(
        DATItemFlag flag, 
        ref SequenceReader<byte> reader, 
        DATItem item, 
        CancellationToken cancellationToken)
    {
        cancellationToken.ThrowIfCancellationRequested();

        return flag switch
            {
                DATItemFlag.Ground => ReadGroundSpeed(ref reader, item),
                DATItemFlag.GroundBorder => 0, // No additional data
                DATItemFlag.OnBottom => 0, // No additional data
                DATItemFlag.OnTop => 0, // No additional data
                DATItemFlag.Container => 0, // No additional data
                DATItemFlag.Stackable => 0, // No additional data
                DATItemFlag.ForceUse => 0, // No additional data
                DATItemFlag.MultiUse => 0, // No additional data
                DATItemFlag.Writeable => ReadWriteableData(ref reader, item),
                DATItemFlag.WriteableOnce => ReadWriteableData(ref reader, item),
                DATItemFlag.FluidContainer => 0, // No additional data
                DATItemFlag.Splash => 0, // No additional data
                DATItemFlag.NotWalkable => 0, // No additional data
                DATItemFlag.NotMoveable => 0, // No additional data
                DATItemFlag.BlockProjectile => 0, // No additional data
                DATItemFlag.NotPathable => 0, // No additional data
                DATItemFlag.Pickupable => 0, // No additional data
                DATItemFlag.Hangable => 0, // No additional data
                DATItemFlag.HookSouth => 0, // No additional data
                DATItemFlag.HookEast => 0, // No additional data
                DATItemFlag.Rotatable => 0, // No additional data
                DATItemFlag.Light => ReadLightData(ref reader, item),
                DATItemFlag.DontHide => 0, // No additional data
                DATItemFlag.Translucent => 0, // No additional data
                DATItemFlag.Displacement => ReadDisplacementData(ref reader, item),
                DATItemFlag.Elevation => ReadElevationData(ref reader, item),
                DATItemFlag.LyingCorpse => 0, // No additional data
                DATItemFlag.AnimateAlways => 0, // No additional data
                DATItemFlag.MinimapColor => ReadMinimapColor(ref reader, item),
            DATItemFlag.LensHelp => ReadLensHelp(ref reader, item),
            DATItemFlag.FullGround => 0, // No additional data
            DATItemFlag.IgnoreLook => 0, // No additional data
            DATItemFlag.Cloth => ReadClothSlot(ref reader, item),
            DATItemFlag.Market => ReadMarketData(ref reader, item),
            _ => 0 // Unknown flag, no data
        };
    }

    private int ReadGroundSpeed(ref SequenceReader<byte> reader, DATItem item)
    {
        if (reader.TryReadLittleEndian(out ushort speed))
        {
            item.GroundSpeed = speed;
            return 2;
        }
        return 0;
    }

    private int ReadWriteableData(ref SequenceReader<byte> reader, DATItem item)
    {
        if (reader.TryReadLittleEndian(out ushort maxLength))
        {
            item.MaxTextLength = maxLength;
            return 2;
        }
        return 0;
    }

    private int ReadLightData(ref SequenceReader<byte> reader, DATItem item)
    {
        if (reader.TryReadLittleEndian(out ushort lightLevel) && 
            reader.TryReadLittleEndian(out ushort lightColor))
        {
            item.LightLevel = lightLevel;
            item.LightColor = lightColor;
            return 4;
        }
        return 0;
    }

    private int ReadDisplacementData(ref SequenceReader<byte> reader, DATItem item)
    {
        if (reader.TryReadLittleEndian(out ushort displacementX) && 
            reader.TryReadLittleEndian(out ushort displacementY))
        {
            item.DisplacementX = displacementX;
            item.DisplacementY = displacementY;
            return 4;
        }
        return 0;
    }

    private int ReadElevationData(ref SequenceReader<byte> reader, DATItem item)
    {
        if (reader.TryReadLittleEndian(out ushort elevation))
        {
            item.Elevation = elevation;
            return 2;
        }
        return 0;
    }

    private int ReadMinimapColor(ref SequenceReader<byte> reader, DATItem item)
    {
        if (reader.TryReadLittleEndian(out ushort color))
        {
            item.MinimapColor = color;
            return 2;
        }
        return 0;
    }

    private int ReadLensHelp(ref SequenceReader<byte> reader, DATItem item)
    {
        if (reader.TryReadLittleEndian(out ushort lensHelp))
        {
            item.LensHelp = lensHelp;
            return 2;
        }
        return 0;
    }

    private int ReadClothSlot(ref SequenceReader<byte> reader, DATItem item)
    {
        if (reader.TryReadLittleEndian(out ushort clothSlot))
        {
            item.ClothSlot = clothSlot;
            return 2;
        }
        return 0;
    }

    private int ReadMarketData(ref SequenceReader<byte> reader, DATItem item)
    {
        var bytesRead = 0;
        
        if (reader.TryReadLittleEndian(out ushort category))
        {
            item.MarketCategory = category;
            bytesRead += 2;
        }
        
        if (reader.TryReadLittleEndian(out ushort tradeAs))
        {
            item.MarketTradeAs = tradeAs;
            bytesRead += 2;
        }
        
        if (reader.TryReadLittleEndian(out ushort showAs))
        {
            item.MarketShowAs = showAs;
            bytesRead += 2;
        }
        
        if (reader.TryReadLittleEndian(out ushort nameLength))
        {
            bytesRead += 2;
            
            if (reader.Remaining >= nameLength)
            {
                var nameBytes = new byte[nameLength];
                if (reader.TryCopyTo(nameBytes))
                {
                    reader.Advance(nameLength);
                    item.MarketName = System.Text.Encoding.UTF8.GetString(nameBytes);
                    bytesRead += nameLength;
                }
            }
        }
        
        if (reader.TryReadLittleEndian(out ushort restrictProfession))
        {
            item.MarketRestrictProfession = restrictProfession;
            bytesRead += 2;
        }
        
        if (reader.TryReadLittleEndian(out ushort restrictLevel))
        {
            item.MarketRestrictLevel = restrictLevel;
            bytesRead += 2;
        }
        
        return bytesRead;
    }

    private static DATHeader ReadHeaderFromBuffer(ReadOnlySequence<byte> buffer)
    {
        var reader = new SequenceReader<byte>(buffer);
        
        // Read version (4 bytes)
        if (!reader.TryReadLittleEndian(out uint version))
            throw new InvalidDataException("Failed to read DAT version");

        // Read counts
        if (!reader.TryReadLittleEndian(out ushort itemCount))
            throw new InvalidDataException("Failed to read item count");

        if (!reader.TryReadLittleEndian(out ushort outfitCount))
            throw new InvalidDataException("Failed to read outfit count");

        if (!reader.TryReadLittleEndian(out ushort effectCount))
            throw new InvalidDataException("Failed to read effect count");

        if (!reader.TryReadLittleEndian(out ushort missileCount))
            throw new InvalidDataException("Failed to read missile count");

        return new DATHeader
        {
            Version = version,
            ItemCount = itemCount,
            OutfitCount = outfitCount,
            EffectCount = effectCount,
            MissileCount = missileCount
        };
    }

    /// <inheritdoc />
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    /// <summary>
    /// Disposes the DAT reader
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