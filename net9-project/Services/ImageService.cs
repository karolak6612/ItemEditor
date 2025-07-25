using Microsoft.Extensions.Logging;
using System.Collections.Concurrent;
using System.Security.Cryptography;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using ItemEditor.Models;
using PluginInterface.OTLib.FileFormats;

namespace ItemEditor.Services;

/// <summary>
/// High-performance image service with efficient sprite caching and processing
/// </summary>
public class ImageService : IImageService, IDisposable
{
    private readonly ILogger<ImageService> _logger;
    private readonly SpriteCache _spriteCache;
    private readonly ThumbnailCache _thumbnailCache;
    private readonly SemaphoreSlim _processingLock = new(Environment.ProcessorCount, Environment.ProcessorCount);
    private readonly Timer _cacheCleanupTimer;
    private bool _disposed;

    // Cache statistics
    private long _cacheHits;
    private long _cacheMisses;

    /// <summary>
    /// Initializes a new instance of the ImageService class
    /// </summary>
    /// <param name="logger">Logger instance</param>
    public ImageService(ILogger<ImageService> logger)
    {
        _logger = logger;
        _spriteCache = new SpriteCache(logger);
        _thumbnailCache = new ThumbnailCache(logger);
        
        // Setup cache cleanup timer (runs every 5 minutes)
        _cacheCleanupTimer = new Timer(CleanupCaches, null, TimeSpan.FromMinutes(5), TimeSpan.FromMinutes(5));
        
        _logger.LogInformation("ImageService initialized with caching enabled");
    }

    /// <inheritdoc />
    public event EventHandler<ImageProcessingProgressEventArgs>? ProgressChanged;
    
    /// <inheritdoc />
    public event EventHandler<CacheUpdatedEventArgs>? CacheUpdated;

    /// <inheritdoc />
    public async Task<BitmapSource> LoadSpriteAsync(string spritePath, ushort spriteId, bool useCache = true, CancellationToken cancellationToken = default)
    {
        try
        {
            var cacheKey = $"{spritePath}:{spriteId}";
            
            // Check cache first
            if (useCache && _spriteCache.TryGet(cacheKey, out var cachedSprite))
            {
                Interlocked.Increment(ref _cacheHits);
                _logger.LogTrace("Cache hit for sprite {SpriteId} from {SpritePath}", spriteId, spritePath);
                return cachedSprite;
            }

            Interlocked.Increment(ref _cacheMisses);
            
            await _processingLock.WaitAsync(cancellationToken);
            try
            {
                // Load sprite from file
                using var fileStream = new FileStream(spritePath, FileMode.Open, FileAccess.Read, FileShare.Read);
                using var sprReader = new SPRReader(fileStream, _logger);
                
                var header = await sprReader.ReadHeaderAsync(cancellationToken);
                var addresses = await sprReader.ReadSpriteAddressesAsync(header, cancellationToken);
                var sprSprite = await sprReader.ReadSpriteAsync(spriteId, addresses, cancellationToken);
                
                if (sprSprite == null)
                {
                    _logger.LogWarning("Failed to load sprite {SpriteId} from {SpritePath}", spriteId, spritePath);
                    return CreateEmptySprite();
                }

                var bitmapSource = await ConvertSpriteToImageAsync(sprSprite, cancellationToken);
                
                // Cache the result
                if (useCache)
                {
                    _spriteCache.Set(cacheKey, bitmapSource);
                    OnCacheUpdated(spritePath, 1, CacheOperation.Add);
                }
                
                _logger.LogTrace("Loaded sprite {SpriteId} from {SpritePath}", spriteId, spritePath);
                return bitmapSource;
            }
            finally
            {
                _processingLock.Release();
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to load sprite {SpriteId} from {SpritePath}", spriteId, spritePath);
            return CreateEmptySprite();
        }
    }

    /// <inheritdoc />
    public async Task<Dictionary<ushort, BitmapSource>> LoadSpritesAsync(
        string spritePath, 
        IEnumerable<ushort> spriteIds, 
        IProgress<ImageLoadProgress>? progress = null, 
        CancellationToken cancellationToken = default)
    {
        var sprites = new Dictionary<ushort, BitmapSource>();
        var spriteList = spriteIds.ToList();
        var totalSprites = spriteList.Count;
        var loadedCount = 0;

        try
        {
            _logger.LogInformation("Loading {TotalSprites} sprites from {SpritePath}", totalSprites, spritePath);

            // Check cache for all sprites first
            var uncachedSprites = new List<ushort>();
            foreach (var spriteId in spriteList)
            {
                var cacheKey = $"{spritePath}:{spriteId}";
                if (_spriteCache.TryGet(cacheKey, out var cachedSprite))
                {
                    sprites[spriteId] = cachedSprite;
                    Interlocked.Increment(ref _cacheHits);
                    loadedCount++;
                }
                else
                {
                    uncachedSprites.Add(spriteId);
                    Interlocked.Increment(ref _cacheMisses);
                }
            }

            // Report initial progress
            progress?.Report(new ImageLoadProgress
            {
                ImagesLoaded = loadedCount,
                TotalImages = totalSprites,
                CurrentOperation = $"Loaded {loadedCount} sprites from cache"
            });

            if (uncachedSprites.Count == 0)
            {
                _logger.LogInformation("All {TotalSprites} sprites loaded from cache", totalSprites);
                return sprites;
            }

            // Load uncached sprites
            using var fileStream = new FileStream(spritePath, FileMode.Open, FileAccess.Read, FileShare.Read);
            using var sprReader = new SPRReader(fileStream, _logger);
            
            var header = await sprReader.ReadHeaderAsync(cancellationToken);
            var addresses = await sprReader.ReadSpriteAddressesAsync(header, cancellationToken);
            
            var loadedSprites = await sprReader.ReadSpritesAsync(uncachedSprites, addresses, 
                new Progress<FileReadProgress>(p => 
                {
                    progress?.Report(new ImageLoadProgress
                    {
                        ImagesLoaded = loadedCount + p.ItemsRead,
                        TotalImages = totalSprites,
                        CurrentOperation = p.CurrentOperation
                    });
                }), cancellationToken);

            // Convert and cache loaded sprites
            var newlyCachedCount = 0;
            foreach (var kvp in loadedSprites)
            {
                var bitmapSource = await ConvertSpriteToImageAsync(kvp.Value, cancellationToken);
                sprites[kvp.Key] = bitmapSource;
                
                var cacheKey = $"{spritePath}:{kvp.Key}";
                _spriteCache.Set(cacheKey, bitmapSource);
                newlyCachedCount++;
                loadedCount++;
            }

            OnCacheUpdated(spritePath, newlyCachedCount, CacheOperation.Add);
            
            _logger.LogInformation("Successfully loaded {LoadedCount}/{TotalSprites} sprites from {SpritePath}", 
                loadedCount, totalSprites, spritePath);
            
            return sprites;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to load sprites from {SpritePath}", spritePath);
            throw;
        }
    }

    /// <inheritdoc />
    public async Task<BitmapSource> RenderItemAsync(Item item, ItemRenderOptions? renderOptions = null, CancellationToken cancellationToken = default)
    {
        renderOptions ??= new ItemRenderOptions();
        
        try
        {
            _logger.LogTrace("Rendering item {ItemId} ({ItemName})", item.Id, item.Name);

            // Create render target
            var renderTarget = new RenderTargetBitmap(
                renderOptions.Width, 
                renderOptions.Height, 
                96, 96, 
                PixelFormats.Pbgra32);

            var visual = new DrawingVisual();
            using (var context = visual.RenderOpen())
            {
                // Fill background
                if (renderOptions.BackgroundColor != Colors.Transparent)
                {
                    context.DrawRectangle(
                        new SolidColorBrush(renderOptions.BackgroundColor),
                        null,
                        new System.Windows.Rect(0, 0, renderOptions.Width, renderOptions.Height));
                }

                // Render item thumbnail if available
                if (item.Thumbnail != null)
                {
                    var rect = new System.Windows.Rect(0, 0, renderOptions.Width, renderOptions.Height);
                    context.DrawImage(item.Thumbnail, rect);
                }
                else
                {
                    // Render placeholder
                    var brush = new SolidColorBrush(Color.FromArgb(128, 128, 128, 128));
                    var pen = new Pen(Brushes.Gray, 1);
                    var rect = new System.Windows.Rect(1, 1, renderOptions.Width - 2, renderOptions.Height - 2);
                    context.DrawRectangle(brush, pen, rect);
                    
                    // Draw item ID text
                    var text = new FormattedText(
                        item.Id.ToString(),
                        System.Globalization.CultureInfo.CurrentCulture,
                        FlowDirection.LeftToRight,
                        new Typeface("Arial"),
                        10,
                        Brushes.White,
                        VisualTreeHelper.GetDpi(visual).PixelsPerDip);
                    
                    var textRect = new System.Windows.Rect(
                        (renderOptions.Width - text.Width) / 2,
                        (renderOptions.Height - text.Height) / 2,
                        text.Width,
                        text.Height);
                    
                    context.DrawText(text, textRect.TopLeft);
                }
            }

            renderTarget.Render(visual);
            renderTarget.Freeze();
            
            return renderTarget;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to render item {ItemId}", item.Id);
            return CreateEmptySprite(renderOptions.Width, renderOptions.Height);
        }
    }

    /// <inheritdoc />
    public async Task<BitmapSource> RenderDATItemAsync(
        DATItem datItem, 
        Dictionary<ushort, SPRSprite> spriteData, 
        ItemRenderOptions? renderOptions = null, 
        CancellationToken cancellationToken = default)
    {
        renderOptions ??= new ItemRenderOptions();
        
        try
        {
            _logger.LogTrace("Rendering DAT item {ItemId}", datItem.Id);

            var renderTarget = new RenderTargetBitmap(
                renderOptions.Width, 
                renderOptions.Height, 
                96, 96, 
                PixelFormats.Pbgra32);

            var visual = new DrawingVisual();
            using (var context = visual.RenderOpen())
            {
                // Fill background
                if (renderOptions.BackgroundColor != Colors.Transparent)
                {
                    context.DrawRectangle(
                        new SolidColorBrush(renderOptions.BackgroundColor),
                        null,
                        new System.Windows.Rect(0, 0, renderOptions.Width, renderOptions.Height));
                }

                // Render sprites in layers
                var spriteWidth = renderOptions.Width / Math.Max(datItem.Width, 1);
                var spriteHeight = renderOptions.Height / Math.Max(datItem.Height, 1);

                for (int layer = 0; layer < datItem.Layers; layer++)
                {
                    for (int y = 0; y < datItem.Height; y++)
                    {
                        for (int x = 0; x < datItem.Width; x++)
                        {
                            var spriteIndex = (layer * datItem.Height * datItem.Width) + (y * datItem.Width) + x;
                            
                            if (spriteIndex < datItem.SpriteIds.Count)
                            {
                                var spriteId = datItem.SpriteIds[spriteIndex];
                                
                                if (spriteData.TryGetValue(spriteId, out var sprite))
                                {
                                    var spriteBitmap = await ConvertSpriteToImageAsync(sprite, cancellationToken);
                                    var rect = new System.Windows.Rect(
                                        x * spriteWidth, 
                                        y * spriteHeight, 
                                        spriteWidth, 
                                        spriteHeight);
                                    
                                    context.DrawImage(spriteBitmap, rect);
                                }
                            }
                        }
                    }
                }
            }

            renderTarget.Render(visual);
            renderTarget.Freeze();
            
            return renderTarget;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to render DAT item {ItemId}", datItem.Id);
            return CreateEmptySprite(renderOptions.Width, renderOptions.Height);
        }
    }

    /// <inheritdoc />
    public async Task<byte[]> GenerateThumbnailAsync(
        BitmapSource source, 
        ThumbnailOptions? thumbnailOptions = null, 
        CancellationToken cancellationToken = default)
    {
        thumbnailOptions ??= new ThumbnailOptions();
        
        try
        {
            // Check thumbnail cache
            var sourceHash = CalculateImageHash(source);
            var cacheKey = $"{sourceHash}:{thumbnailOptions.MaxWidth}x{thumbnailOptions.MaxHeight}:{thumbnailOptions.Quality}";
            
            if (_thumbnailCache.TryGet(cacheKey, out var cachedThumbnail))
            {
                return cachedThumbnail;
            }

            await _processingLock.WaitAsync(cancellationToken);
            try
            {
                // Calculate thumbnail dimensions
                var (thumbnailWidth, thumbnailHeight) = CalculateThumbnailDimensions(
                    source.PixelWidth, source.PixelHeight, 
                    thumbnailOptions.MaxWidth, thumbnailOptions.MaxHeight, 
                    thumbnailOptions.MaintainAspectRatio);

                // Create thumbnail
                var thumbnail = new TransformedBitmap(source, new ScaleTransform(
                    (double)thumbnailWidth / source.PixelWidth,
                    (double)thumbnailHeight / source.PixelHeight));

                // Convert to byte array
                var encoder = new JpegBitmapEncoder
                {
                    QualityLevel = thumbnailOptions.Quality
                };
                
                encoder.Frames.Add(BitmapFrame.Create(thumbnail));
                
                using var stream = new MemoryStream();
                encoder.Save(stream);
                var thumbnailData = stream.ToArray();
                
                // Cache the result
                _thumbnailCache.Set(cacheKey, thumbnailData);
                
                return thumbnailData;
            }
            finally
            {
                _processingLock.Release();
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to generate thumbnail");
            throw;
        }
    }

    /// <inheritdoc />
    public async Task<Dictionary<string, byte[]>> GenerateThumbnailsAsync(
        Dictionary<string, BitmapSource> sources, 
        ThumbnailOptions? thumbnailOptions = null, 
        IProgress<ImageProcessingProgress>? progress = null, 
        CancellationToken cancellationToken = default)
    {
        var thumbnails = new Dictionary<string, byte[]>();
        var totalImages = sources.Count;
        var processedCount = 0;

        try
        {
            _logger.LogInformation("Generating {TotalImages} thumbnails", totalImages);

            var tasks = sources.Select(async kvp =>
            {
                var thumbnail = await GenerateThumbnailAsync(kvp.Value, thumbnailOptions, cancellationToken);
                
                lock (thumbnails)
                {
                    thumbnails[kvp.Key] = thumbnail;
                    processedCount++;
                    
                    progress?.Report(new ImageProcessingProgress
                    {
                        ImagesProcessed = processedCount,
                        TotalImages = totalImages,
                        CurrentOperation = $"Generated thumbnail for {kvp.Key}"
                    });
                }
                
                return thumbnail;
            });

            await Task.WhenAll(tasks);
            
            _logger.LogInformation("Successfully generated {ThumbnailCount} thumbnails", thumbnails.Count);
            return thumbnails;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to generate thumbnails");
            throw;
        }
    }

    /// <inheritdoc />
    public async Task<double> CompareSpriteHashAsync(
        byte[] hash1, 
        byte[] hash2, 
        double similarityThreshold = 1.0, 
        CancellationToken cancellationToken = default)
    {
        return await Task.Run(() =>
        {
            if (hash1 == null || hash2 == null || hash1.Length != hash2.Length)
                return 0.0;

            if (hash1.SequenceEqual(hash2))
                return 1.0;

            // Calculate Hamming distance for similarity
            var differences = 0;
            for (int i = 0; i < hash1.Length; i++)
            {
                var xor = hash1[i] ^ hash2[i];
                differences += CountBits(xor);
            }

            var maxDifferences = hash1.Length * 8; // 8 bits per byte
            var similarity = 1.0 - (double)differences / maxDifferences;
            
            return similarity >= similarityThreshold ? similarity : 0.0;
        }, cancellationToken);
    }

    /// <inheritdoc />
    public async Task<List<SpriteMatch>> FindSimilarSpritesAsync(
        IEnumerable<SPRSprite> sprites, 
        SPRSprite targetSprite, 
        double similarityThreshold = 0.9, 
        CancellationToken cancellationToken = default)
    {
        var matches = new List<SpriteMatch>();
        
        try
        {
            // Ensure target sprite has hash
            if (targetSprite.Hash == null)
                targetSprite.CalculateHash();

            var spriteList = sprites.ToList();
            _logger.LogDebug("Searching for similar sprites among {SpriteCount} sprites", spriteList.Count);

            var tasks = spriteList.Select(async sprite =>
            {
                if (sprite.Hash == null)
                    sprite.CalculateHash();

                var similarity = await CompareSpriteHashAsync(targetSprite.Hash!, sprite.Hash!, similarityThreshold, cancellationToken);
                
                if (similarity >= similarityThreshold)
                {
                    var confidence = similarity switch
                    {
                        1.0 => MatchConfidence.Exact,
                        >= 0.95 => MatchConfidence.High,
                        >= 0.85 => MatchConfidence.Medium,
                        _ => MatchConfidence.Low
                    };

                    return new SpriteMatch
                    {
                        Sprite = sprite,
                        SimilarityScore = similarity,
                        Confidence = confidence
                    };
                }
                
                return null;
            });

            var results = await Task.WhenAll(tasks);
            matches.AddRange(results.Where(m => m != null)!);
            
            // Sort by similarity score descending
            matches.Sort((a, b) => b.SimilarityScore.CompareTo(a.SimilarityScore));
            
            _logger.LogDebug("Found {MatchCount} similar sprites", matches.Count);
            return matches;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to find similar sprites");
            throw;
        }
    }

    /// <inheritdoc />
    public async Task<BitmapSource> ConvertSpriteToImageAsync(SPRSprite sprite, CancellationToken cancellationToken = default)
    {
        return await Task.Run(() =>
        {
            if (sprite.PixelData == null || sprite.PixelData.Length == 0)
                return CreateEmptySprite(sprite.Width, sprite.Height);

            var bitmap = BitmapSource.Create(
                sprite.Width,
                sprite.Height,
                96, 96,
                PixelFormats.Bgra32,
                null,
                sprite.PixelData,
                sprite.Width * 4);

            bitmap.Freeze();
            return bitmap;
        }, cancellationToken);
    }

    /// <inheritdoc />
    public async Task<SPRSprite> ConvertImageToSpriteAsync(BitmapSource bitmapSource, ushort spriteId, CancellationToken cancellationToken = default)
    {
        return await Task.Run(() =>
        {
            var sprite = new SPRSprite
            {
                Id = spriteId,
                Width = bitmapSource.PixelWidth,
                Height = bitmapSource.PixelHeight
            };

            // Convert to BGRA32 format if needed
            var convertedBitmap = bitmapSource.Format == PixelFormats.Bgra32 
                ? bitmapSource 
                : new FormatConvertedBitmap(bitmapSource, PixelFormats.Bgra32, null, 0);

            var stride = convertedBitmap.PixelWidth * 4;
            sprite.PixelData = new byte[stride * convertedBitmap.PixelHeight];
            
            convertedBitmap.CopyPixels(sprite.PixelData, stride, 0);
            sprite.CalculateHash();
            
            return sprite;
        }, cancellationToken);
    }

    /// <inheritdoc />
    public async Task<BitmapSource> ApplyFiltersAsync(BitmapSource source, IEnumerable<ImageFilter> filters, CancellationToken cancellationToken = default)
    {
        return await Task.Run(() =>
        {
            var result = source;
            
            foreach (var filter in filters)
            {
                cancellationToken.ThrowIfCancellationRequested();
                result = filter.Apply(result);
            }
            
            return result;
        }, cancellationToken);
    }

    /// <inheritdoc />
    public void ClearSpriteCache(string? filePath = null)
    {
        if (filePath == null)
        {
            _spriteCache.Clear();
            _thumbnailCache.Clear();
            OnCacheUpdated(string.Empty, 0, CacheOperation.Clear);
            _logger.LogInformation("Cleared all sprite and thumbnail caches");
        }
        else
        {
            var removedCount = _spriteCache.RemoveByPrefix(filePath);
            OnCacheUpdated(filePath, removedCount, CacheOperation.Remove);
            _logger.LogInformation("Cleared {RemovedCount} sprites from cache for {FilePath}", removedCount, filePath);
        }
    }

    /// <inheritdoc />
    public ImageCacheStatistics GetCacheStatistics()
    {
        return new ImageCacheStatistics
        {
            CachedSprites = _spriteCache.Count,
            MemoryUsage = _spriteCache.MemoryUsage + _thumbnailCache.MemoryUsage,
            HitRatio = _cacheHits + _cacheMisses > 0 ? (double)_cacheHits / (_cacheHits + _cacheMisses) : 0.0,
            CacheHits = _cacheHits,
            CacheMisses = _cacheMisses
        };
    }

    #region Private Methods

    private BitmapSource CreateEmptySprite(int width = 32, int height = 32)
    {
        var pixelData = new byte[width * height * 4]; // BGRA
        
        // Fill with transparent pixels
        for (int i = 0; i < pixelData.Length; i += 4)
        {
            pixelData[i] = 0;     // B
            pixelData[i + 1] = 0; // G
            pixelData[i + 2] = 0; // R
            pixelData[i + 3] = 0; // A (transparent)
        }

        var bitmap = BitmapSource.Create(width, height, 96, 96, PixelFormats.Bgra32, null, pixelData, width * 4);
        bitmap.Freeze();
        return bitmap;
    }

    private static (int width, int height) CalculateThumbnailDimensions(
        int sourceWidth, int sourceHeight, 
        int maxWidth, int maxHeight, 
        bool maintainAspectRatio)
    {
        if (!maintainAspectRatio)
            return (maxWidth, maxHeight);

        var aspectRatio = (double)sourceWidth / sourceHeight;
        
        if (aspectRatio > 1) // Wider than tall
        {
            var width = Math.Min(maxWidth, sourceWidth);
            var height = (int)(width / aspectRatio);
            return (width, Math.Min(height, maxHeight));
        }
        else // Taller than wide
        {
            var height = Math.Min(maxHeight, sourceHeight);
            var width = (int)(height * aspectRatio);
            return (Math.Min(width, maxWidth), height);
        }
    }

    private static string CalculateImageHash(BitmapSource bitmap)
    {
        using var md5 = MD5.Create();
        
        var stride = bitmap.PixelWidth * 4;
        var pixelData = new byte[stride * bitmap.PixelHeight];
        bitmap.CopyPixels(pixelData, stride, 0);
        
        var hash = md5.ComputeHash(pixelData);
        return Convert.ToHexString(hash);
    }

    private static int CountBits(byte value)
    {
        int count = 0;
        while (value != 0)
        {
            count++;
            value &= (byte)(value - 1);
        }
        return count;
    }

    private void CleanupCaches(object? state)
    {
        try
        {
            var beforeSprites = _spriteCache.Count;
            var beforeThumbnails = _thumbnailCache.Count;
            
            _spriteCache.Cleanup();
            _thumbnailCache.Cleanup();
            
            var afterSprites = _spriteCache.Count;
            var afterThumbnails = _thumbnailCache.Count;
            
            if (beforeSprites != afterSprites || beforeThumbnails != afterThumbnails)
            {
                _logger.LogDebug("Cache cleanup: Sprites {BeforeSprites} -> {AfterSprites}, Thumbnails {BeforeThumbnails} -> {AfterThumbnails}",
                    beforeSprites, afterSprites, beforeThumbnails, afterThumbnails);
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error during cache cleanup");
        }
    }

    private void OnCacheUpdated(string filePath, int spritesAdded, CacheOperation operation)
    {
        CacheUpdated?.Invoke(this, new CacheUpdatedEventArgs
        {
            FilePath = filePath,
            SpritesAdded = spritesAdded,
            Operation = operation
        });
    }

    #endregion

    /// <inheritdoc />
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    /// <summary>
    /// Disposes the image service
    /// </summary>
    /// <param name="disposing">True if disposing managed resources</param>
    protected virtual void Dispose(bool disposing)
    {
        if (!_disposed && disposing)
        {
            _cacheCleanupTimer?.Dispose();
            _processingLock?.Dispose();
            _spriteCache?.Dispose();
            _thumbnailCache?.Dispose();
            _disposed = true;
        }
    }
}