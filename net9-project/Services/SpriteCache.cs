using Microsoft.Extensions.Logging;
using System.Collections.Concurrent;
using System.Windows.Media.Imaging;

namespace ItemEditor.Services;

/// <summary>
/// LRU cache for sprite images with memory management
/// </summary>
public class SpriteCache : IDisposable
{
    private readonly ILogger _logger;
    private readonly ConcurrentDictionary<string, CacheItem> _cache = new();
    private readonly LinkedList<string> _accessOrder = new();
    private readonly ReaderWriterLockSlim _accessLock = new();
    private readonly int _maxItems;
    private readonly long _maxMemoryBytes;
    private long _currentMemoryUsage;
    private bool _disposed;

    /// <summary>
    /// Cache item with metadata
    /// </summary>
    private class CacheItem
    {
        public BitmapSource Bitmap { get; set; } = null!;
        public DateTime LastAccessed { get; set; }
        public long MemorySize { get; set; }
        public LinkedListNode<string>? AccessNode { get; set; }
    }

    /// <summary>
    /// Initializes a new instance of the SpriteCache class
    /// </summary>
    /// <param name="logger">Logger instance</param>
    /// <param name="maxItems">Maximum number of items to cache (default: 1000)</param>
    /// <param name="maxMemoryMB">Maximum memory usage in MB (default: 100)</param>
    public SpriteCache(ILogger logger, int maxItems = 1000, int maxMemoryMB = 100)
    {
        _logger = logger;
        _maxItems = maxItems;
        _maxMemoryBytes = maxMemoryMB * 1024L * 1024L;
        _currentMemoryUsage = 0;
    }

    /// <summary>
    /// Gets the number of cached items
    /// </summary>
    public int Count => _cache.Count;

    /// <summary>
    /// Gets the current memory usage in bytes
    /// </summary>
    public long MemoryUsage => _currentMemoryUsage;

    /// <summary>
    /// Tries to get a cached sprite
    /// </summary>
    /// <param name="key">Cache key</param>
    /// <param name="bitmap">Retrieved bitmap</param>
    /// <returns>True if found in cache</returns>
    public bool TryGet(string key, out BitmapSource bitmap)
    {
        bitmap = null!;
        
        if (_cache.TryGetValue(key, out var item))
        {
            // Update access time and order
            item.LastAccessed = DateTime.UtcNow;
            
            _accessLock.EnterWriteLock();
            try
            {
                if (item.AccessNode != null)
                {
                    _accessOrder.Remove(item.AccessNode);
                    item.AccessNode = _accessOrder.AddFirst(key);
                }
            }
            finally
            {
                _accessLock.ExitWriteLock();
            }
            
            bitmap = item.Bitmap;
            return true;
        }
        
        return false;
    }

    /// <summary>
    /// Sets a sprite in the cache
    /// </summary>
    /// <param name="key">Cache key</param>
    /// <param name="bitmap">Bitmap to cache</param>
    public void Set(string key, BitmapSource bitmap)
    {
        var memorySize = EstimateMemorySize(bitmap);
        var now = DateTime.UtcNow;
        
        _accessLock.EnterWriteLock();
        try
        {
            // Remove existing item if present
            if (_cache.TryRemove(key, out var existingItem))
            {
                Interlocked.Add(ref _currentMemoryUsage, -existingItem.MemorySize);
                if (existingItem.AccessNode != null)
                {
                    _accessOrder.Remove(existingItem.AccessNode);
                }
            }

            // Ensure we have space
            EnsureCapacity(memorySize);

            // Add new item
            var accessNode = _accessOrder.AddFirst(key);
            var newItem = new CacheItem
            {
                Bitmap = bitmap,
                LastAccessed = now,
                MemorySize = memorySize,
                AccessNode = accessNode
            };

            _cache[key] = newItem;
            Interlocked.Add(ref _currentMemoryUsage, memorySize);
        }
        finally
        {
            _accessLock.ExitWriteLock();
        }
    }

    /// <summary>
    /// Removes items from cache by key prefix
    /// </summary>
    /// <param name="prefix">Key prefix to match</param>
    /// <returns>Number of items removed</returns>
    public int RemoveByPrefix(string prefix)
    {
        var removedCount = 0;
        var keysToRemove = new List<string>();

        // Find keys to remove
        foreach (var kvp in _cache)
        {
            if (kvp.Key.StartsWith(prefix, StringComparison.OrdinalIgnoreCase))
            {
                keysToRemove.Add(kvp.Key);
            }
        }

        // Remove items
        _accessLock.EnterWriteLock();
        try
        {
            foreach (var key in keysToRemove)
            {
                if (_cache.TryRemove(key, out var item))
                {
                    Interlocked.Add(ref _currentMemoryUsage, -item.MemorySize);
                    if (item.AccessNode != null)
                    {
                        _accessOrder.Remove(item.AccessNode);
                    }
                    removedCount++;
                }
            }
        }
        finally
        {
            _accessLock.ExitWriteLock();
        }

        return removedCount;
    }

    /// <summary>
    /// Clears all cached items
    /// </summary>
    public void Clear()
    {
        _accessLock.EnterWriteLock();
        try
        {
            _cache.Clear();
            _accessOrder.Clear();
            _currentMemoryUsage = 0;
        }
        finally
        {
            _accessLock.ExitWriteLock();
        }
    }

    /// <summary>
    /// Performs cache cleanup by removing old items
    /// </summary>
    public void Cleanup()
    {
        var cutoffTime = DateTime.UtcNow.AddMinutes(-30); // Remove items older than 30 minutes
        var keysToRemove = new List<string>();

        // Find old items
        foreach (var kvp in _cache)
        {
            if (kvp.Value.LastAccessed < cutoffTime)
            {
                keysToRemove.Add(kvp.Key);
            }
        }

        // Remove old items
        if (keysToRemove.Count > 0)
        {
            _accessLock.EnterWriteLock();
            try
            {
                foreach (var key in keysToRemove)
                {
                    if (_cache.TryRemove(key, out var item))
                    {
                        Interlocked.Add(ref _currentMemoryUsage, -item.MemorySize);
                        if (item.AccessNode != null)
                        {
                            _accessOrder.Remove(item.AccessNode);
                        }
                    }
                }
            }
            finally
            {
                _accessLock.ExitWriteLock();
            }

            _logger.LogDebug("Cleaned up {RemovedCount} old sprites from cache", keysToRemove.Count);
        }
    }

    /// <summary>
    /// Ensures cache capacity by removing LRU items if necessary
    /// </summary>
    /// <param name="requiredMemory">Required memory for new item</param>
    private void EnsureCapacity(long requiredMemory)
    {
        // Check if we need to free up space
        while ((_cache.Count >= _maxItems || _currentMemoryUsage + requiredMemory > _maxMemoryBytes) 
               && _accessOrder.Count > 0)
        {
            // Remove least recently used item
            var lruKey = _accessOrder.Last?.Value;
            if (lruKey != null && _cache.TryRemove(lruKey, out var lruItem))
            {
                Interlocked.Add(ref _currentMemoryUsage, -lruItem.MemorySize);
                _accessOrder.RemoveLast();
                
                _logger.LogTrace("Evicted LRU sprite from cache: {Key}", lruKey);
            }
            else
            {
                break; // Safety break
            }
        }
    }

    /// <summary>
    /// Estimates memory size of a bitmap
    /// </summary>
    /// <param name="bitmap">Bitmap to estimate</param>
    /// <returns>Estimated memory size in bytes</returns>
    private static long EstimateMemorySize(BitmapSource bitmap)
    {
        // Estimate: width * height * bytes per pixel + overhead
        var bytesPerPixel = bitmap.Format.BitsPerPixel / 8;
        return bitmap.PixelWidth * bitmap.PixelHeight * bytesPerPixel + 1024; // 1KB overhead
    }

    /// <inheritdoc />
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    /// <summary>
    /// Disposes the sprite cache
    /// </summary>
    /// <param name="disposing">True if disposing managed resources</param>
    protected virtual void Dispose(bool disposing)
    {
        if (!_disposed && disposing)
        {
            Clear();
            _accessLock?.Dispose();
            _disposed = true;
        }
    }
}