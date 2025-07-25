using System.Windows.Media.Imaging;
using ItemEditor.Models;
using PluginInterface.OTLib.FileFormats;

namespace ItemEditor.Services;

/// <summary>
/// Modern image service interface for sprite rendering and thumbnail generation
/// </summary>
public interface IImageService
{
    /// <summary>
    /// Loads a sprite from a sprite file with caching support
    /// </summary>
    /// <param name="spritePath">Path to the sprite file</param>
    /// <param name="spriteId">ID of the sprite to load</param>
    /// <param name="useCache">Whether to use sprite cache</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Bitmap source of the sprite</returns>
    Task<BitmapSource> LoadSpriteAsync(string spritePath, ushort spriteId, bool useCache = true, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Loads multiple sprites efficiently
    /// </summary>
    /// <param name="spritePath">Path to the sprite file</param>
    /// <param name="spriteIds">IDs of sprites to load</param>
    /// <param name="progress">Progress reporter</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Dictionary of sprite IDs to bitmap sources</returns>
    Task<Dictionary<ushort, BitmapSource>> LoadSpritesAsync(string spritePath, IEnumerable<ushort> spriteIds, IProgress<ImageLoadProgress>? progress = null, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Renders an item as a bitmap with advanced rendering options
    /// </summary>
    /// <param name="item">Item to render</param>
    /// <param name="renderOptions">Rendering options</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Bitmap source of the rendered item</returns>
    Task<BitmapSource> RenderItemAsync(Item item, ItemRenderOptions? renderOptions = null, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Renders a DAT item with sprite composition
    /// </summary>
    /// <param name="datItem">DAT item to render</param>
    /// <param name="spriteData">Sprite data dictionary</param>
    /// <param name="renderOptions">Rendering options</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Rendered bitmap source</returns>
    Task<BitmapSource> RenderDATItemAsync(DATItem datItem, Dictionary<ushort, SPRSprite> spriteData, ItemRenderOptions? renderOptions = null, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Generates a thumbnail for an image with quality options
    /// </summary>
    /// <param name="source">Source bitmap</param>
    /// <param name="thumbnailOptions">Thumbnail generation options</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Thumbnail as byte array</returns>
    Task<byte[]> GenerateThumbnailAsync(BitmapSource source, ThumbnailOptions? thumbnailOptions = null, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Generates thumbnails for multiple images
    /// </summary>
    /// <param name="sources">Source bitmaps with identifiers</param>
    /// <param name="thumbnailOptions">Thumbnail generation options</param>
    /// <param name="progress">Progress reporter</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Dictionary of identifiers to thumbnail data</returns>
    Task<Dictionary<string, byte[]>> GenerateThumbnailsAsync(Dictionary<string, BitmapSource> sources, ThumbnailOptions? thumbnailOptions = null, IProgress<ImageProcessingProgress>? progress = null, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Compares two sprite hashes with similarity threshold
    /// </summary>
    /// <param name="hash1">First hash</param>
    /// <param name="hash2">Second hash</param>
    /// <param name="similarityThreshold">Similarity threshold (0.0 to 1.0)</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Similarity score (0.0 to 1.0)</returns>
    Task<double> CompareSpriteHashAsync(byte[] hash1, byte[] hash2, double similarityThreshold = 1.0, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Finds similar sprites in a collection
    /// </summary>
    /// <param name="sprites">Sprite collection to search</param>
    /// <param name="targetSprite">Target sprite to find similarities for</param>
    /// <param name="similarityThreshold">Minimum similarity threshold</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>List of similar sprites with similarity scores</returns>
    Task<List<SpriteMatch>> FindSimilarSpritesAsync(IEnumerable<SPRSprite> sprites, SPRSprite targetSprite, double similarityThreshold = 0.9, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Converts SPR sprite to WPF BitmapSource
    /// </summary>
    /// <param name="sprite">SPR sprite to convert</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>WPF BitmapSource</returns>
    Task<BitmapSource> ConvertSpriteToImageAsync(SPRSprite sprite, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Converts WPF BitmapSource to SPR sprite
    /// </summary>
    /// <param name="bitmapSource">WPF BitmapSource to convert</param>
    /// <param name="spriteId">Sprite ID to assign</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>SPR sprite</returns>
    Task<SPRSprite> ConvertImageToSpriteAsync(BitmapSource bitmapSource, ushort spriteId, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Applies image filters to a bitmap
    /// </summary>
    /// <param name="source">Source bitmap</param>
    /// <param name="filters">Image filters to apply</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Filtered bitmap</returns>
    Task<BitmapSource> ApplyFiltersAsync(BitmapSource source, IEnumerable<ImageFilter> filters, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Clears the sprite cache
    /// </summary>
    /// <param name="filePath">Specific file path to clear (null for all)</param>
    void ClearSpriteCache(string? filePath = null);
    
    /// <summary>
    /// Gets cache statistics
    /// </summary>
    /// <returns>Cache statistics</returns>
    ImageCacheStatistics GetCacheStatistics();
    
    /// <summary>
    /// Event raised when image processing progress changes
    /// </summary>
    event EventHandler<ImageProcessingProgressEventArgs>? ProgressChanged;
    
    /// <summary>
    /// Event raised when cache is updated
    /// </summary>
    event EventHandler<CacheUpdatedEventArgs>? CacheUpdated;
}

/// <summary>
/// Item rendering options
/// </summary>
public class ItemRenderOptions
{
    /// <summary>
    /// Render width (default: 32)
    /// </summary>
    public int Width { get; set; } = 32;
    
    /// <summary>
    /// Render height (default: 32)
    /// </summary>
    public int Height { get; set; } = 32;
    
    /// <summary>
    /// Background color (default: transparent)
    /// </summary>
    public System.Windows.Media.Color BackgroundColor { get; set; } = System.Windows.Media.Colors.Transparent;
    
    /// <summary>
    /// Whether to apply anti-aliasing
    /// </summary>
    public bool AntiAliasing { get; set; } = true;
    
    /// <summary>
    /// Render quality (0.0 to 1.0)
    /// </summary>
    public double Quality { get; set; } = 1.0;
    
    /// <summary>
    /// Animation frame to render (0 for first frame)
    /// </summary>
    public int AnimationFrame { get; set; } = 0;
    
    /// <summary>
    /// Whether to render with lighting effects
    /// </summary>
    public bool ApplyLighting { get; set; } = false;
}

/// <summary>
/// Thumbnail generation options
/// </summary>
public class ThumbnailOptions
{
    /// <summary>
    /// Maximum width (default: 64)
    /// </summary>
    public int MaxWidth { get; set; } = 64;
    
    /// <summary>
    /// Maximum height (default: 64)
    /// </summary>
    public int MaxHeight { get; set; } = 64;
    
    /// <summary>
    /// JPEG quality (1-100, default: 85)
    /// </summary>
    public int Quality { get; set; } = 85;
    
    /// <summary>
    /// Whether to maintain aspect ratio
    /// </summary>
    public bool MaintainAspectRatio { get; set; } = true;
    
    /// <summary>
    /// Background color for transparent images
    /// </summary>
    public System.Windows.Media.Color BackgroundColor { get; set; } = System.Windows.Media.Colors.White;
}

/// <summary>
/// Sprite similarity match
/// </summary>
public class SpriteMatch
{
    /// <summary>
    /// Matching sprite
    /// </summary>
    public SPRSprite Sprite { get; set; } = null!;
    
    /// <summary>
    /// Similarity score (0.0 to 1.0)
    /// </summary>
    public double SimilarityScore { get; set; }
    
    /// <summary>
    /// Match confidence level
    /// </summary>
    public MatchConfidence Confidence { get; set; }
}

/// <summary>
/// Match confidence levels
/// </summary>
public enum MatchConfidence
{
    /// <summary>
    /// Low confidence match
    /// </summary>
    Low,
    
    /// <summary>
    /// Medium confidence match
    /// </summary>
    Medium,
    
    /// <summary>
    /// High confidence match
    /// </summary>
    High,
    
    /// <summary>
    /// Exact match
    /// </summary>
    Exact
}

/// <summary>
/// Image filter types
/// </summary>
public abstract class ImageFilter
{
    /// <summary>
    /// Filter name
    /// </summary>
    public abstract string Name { get; }
    
    /// <summary>
    /// Apply the filter to a bitmap
    /// </summary>
    /// <param name="source">Source bitmap</param>
    /// <returns>Filtered bitmap</returns>
    public abstract BitmapSource Apply(BitmapSource source);
}

/// <summary>
/// Image load progress information
/// </summary>
public class ImageLoadProgress
{
    /// <summary>
    /// Number of images loaded
    /// </summary>
    public int ImagesLoaded { get; set; }
    
    /// <summary>
    /// Total images to load
    /// </summary>
    public int TotalImages { get; set; }
    
    /// <summary>
    /// Current operation
    /// </summary>
    public string CurrentOperation { get; set; } = string.Empty;
    
    /// <summary>
    /// Progress percentage (0-100)
    /// </summary>
    public double ProgressPercentage => TotalImages > 0 ? (double)ImagesLoaded / TotalImages * 100 : 0;
}

/// <summary>
/// Image processing progress information
/// </summary>
public class ImageProcessingProgress
{
    /// <summary>
    /// Number of images processed
    /// </summary>
    public int ImagesProcessed { get; set; }
    
    /// <summary>
    /// Total images to process
    /// </summary>
    public int TotalImages { get; set; }
    
    /// <summary>
    /// Current operation
    /// </summary>
    public string CurrentOperation { get; set; } = string.Empty;
    
    /// <summary>
    /// Progress percentage (0-100)
    /// </summary>
    public double ProgressPercentage => TotalImages > 0 ? (double)ImagesProcessed / TotalImages * 100 : 0;
}

/// <summary>
/// Image cache statistics
/// </summary>
public class ImageCacheStatistics
{
    /// <summary>
    /// Number of cached sprites
    /// </summary>
    public int CachedSprites { get; set; }
    
    /// <summary>
    /// Cache memory usage in bytes
    /// </summary>
    public long MemoryUsage { get; set; }
    
    /// <summary>
    /// Cache hit ratio (0.0 to 1.0)
    /// </summary>
    public double HitRatio { get; set; }
    
    /// <summary>
    /// Number of cache hits
    /// </summary>
    public long CacheHits { get; set; }
    
    /// <summary>
    /// Number of cache misses
    /// </summary>
    public long CacheMisses { get; set; }
}

/// <summary>
/// Image processing progress event arguments
/// </summary>
public class ImageProcessingProgressEventArgs : EventArgs
{
    /// <summary>
    /// Progress information
    /// </summary>
    public ImageProcessingProgress Progress { get; set; } = null!;
}

/// <summary>
/// Cache updated event arguments
/// </summary>
public class CacheUpdatedEventArgs : EventArgs
{
    /// <summary>
    /// File path that was cached
    /// </summary>
    public string FilePath { get; set; } = string.Empty;
    
    /// <summary>
    /// Number of sprites added to cache
    /// </summary>
    public int SpritesAdded { get; set; }
    
    /// <summary>
    /// Cache operation type
    /// </summary>
    public CacheOperation Operation { get; set; }
}

/// <summary>
/// Cache operation types
/// </summary>
public enum CacheOperation
{
    /// <summary>
    /// Sprites added to cache
    /// </summary>
    Add,
    
    /// <summary>
    /// Sprites removed from cache
    /// </summary>
    Remove,
    
    /// <summary>
    /// Cache cleared
    /// </summary>
    Clear
}