using Microsoft.Extensions.Logging;
using System.Collections.Concurrent;
using System.Windows.Media.Imaging;
using ItemEditor.Models;

namespace ItemEditor.Services;

/// <summary>
/// Background service for generating thumbnails with lazy loading and priority queuing
/// </summary>
public class BackgroundThumbnailGenerator : IDisposable
{
    private readonly ILogger<BackgroundThumbnailGenerator> _logger;
    private readonly IImageService _imageService;
    private readonly ConcurrentQueue<ThumbnailRequest> _requestQueue = new();
    private readonly ConcurrentDictionary<string, TaskCompletionSource<byte[]>> _pendingRequests = new();
    private readonly SemaphoreSlim _workerSemaphore;
    private readonly CancellationTokenSource _cancellationTokenSource = new();
    private readonly Task[] _workerTasks;
    private bool _disposed;

    /// <summary>
    /// Thumbnail generation request
    /// </summary>
    private class ThumbnailRequest
    {
        public string Key { get; set; } = string.Empty;
        public BitmapSource Source { get; set; } = null!;
        public ThumbnailOptions Options { get; set; } = null!;
        public ThumbnailPriority Priority { get; set; }
        public DateTime RequestTime { get; set; }
        public TaskCompletionSource<byte[]> CompletionSource { get; set; } = null!;
    }

    /// <summary>
    /// Thumbnail generation priority levels
    /// </summary>
    public enum ThumbnailPriority
    {
        /// <summary>
        /// Low priority - background generation
        /// </summary>
        Low = 0,
        
        /// <summary>
        /// Normal priority - user requested
        /// </summary>
        Normal = 1,
        
        /// <summary>
        /// High priority - immediately visible
        /// </summary>
        High = 2,
        
        /// <summary>
        /// Critical priority - user waiting
        /// </summary>
        Critical = 3
    }

    /// <summary>
    /// Initializes a new instance of the BackgroundThumbnailGenerator class
    /// </summary>
    /// <param name="logger">Logger instance</param>
    /// <param name="imageService">Image service for thumbnail generation</param>
    /// <param name="workerCount">Number of background worker threads (default: CPU count / 2)</param>
    public BackgroundThumbnailGenerator(
        ILogger<BackgroundThumbnailGenerator> logger, 
        IImageService imageService,
        int workerCount = 0)
    {
        _logger = logger;
        _imageService = imageService;
        
        if (workerCount <= 0)
            workerCount = Math.Max(1, Environment.ProcessorCount / 2);
        
        _workerSemaphore = new SemaphoreSlim(workerCount, workerCount);
        _workerTasks = new Task[workerCount];
        
        // Start worker tasks
        for (int i = 0; i < workerCount; i++)
        {
            _workerTasks[i] = Task.Run(WorkerLoop, _cancellationTokenSource.Token);
        }
        
        _logger.LogInformation("BackgroundThumbnailGenerator started with {WorkerCount} workers", workerCount);
    }

    /// <summary>
    /// Event raised when a thumbnail is generated
    /// </summary>
    public event EventHandler<ThumbnailGeneratedEventArgs>? ThumbnailGenerated;

    /// <summary>
    /// Requests thumbnail generation with specified priority
    /// </summary>
    /// <param name="key">Unique key for the thumbnail</param>
    /// <param name="source">Source bitmap</param>
    /// <param name="options">Thumbnail options</param>
    /// <param name="priority">Generation priority</param>
    /// <returns>Task that completes when thumbnail is generated</returns>
    public Task<byte[]> RequestThumbnailAsync(
        string key, 
        BitmapSource source, 
        ThumbnailOptions? options = null, 
        ThumbnailPriority priority = ThumbnailPriority.Normal)
    {
        if (_disposed)
            throw new ObjectDisposedException(nameof(BackgroundThumbnailGenerator));

        options ??= new ThumbnailOptions();

        // Check if request is already pending
        if (_pendingRequests.TryGetValue(key, out var existingTcs))
        {
            _logger.LogTrace("Thumbnail request already pending for key: {Key}", key);
            return existingTcs.Task;
        }

        var tcs = new TaskCompletionSource<byte[]>();
        var request = new ThumbnailRequest
        {
            Key = key,
            Source = source,
            Options = options,
            Priority = priority,
            RequestTime = DateTime.UtcNow,
            CompletionSource = tcs
        };

        // Add to pending requests
        _pendingRequests[key] = tcs;

        // Queue the request
        _requestQueue.Enqueue(request);
        
        _logger.LogTrace("Queued thumbnail request for key: {Key} with priority: {Priority}", key, priority);
        
        return tcs.Task;
    }

    /// <summary>
    /// Requests multiple thumbnails in batch
    /// </summary>
    /// <param name="requests">Dictionary of keys to bitmap sources</param>
    /// <param name="options">Thumbnail options</param>
    /// <param name="priority">Generation priority</param>
    /// <returns>Task that completes when all thumbnails are generated</returns>
    public async Task<Dictionary<string, byte[]>> RequestThumbnailsAsync(
        Dictionary<string, BitmapSource> requests,
        ThumbnailOptions? options = null,
        ThumbnailPriority priority = ThumbnailPriority.Normal)
    {
        var tasks = requests.Select(kvp => 
            RequestThumbnailAsync(kvp.Key, kvp.Value, options, priority)
                .ContinueWith(t => new { Key = kvp.Key, Result = t.Result }, TaskContinuationOptions.OnlyOnRanToCompletion))
            .ToArray();

        var results = await Task.WhenAll(tasks);
        return results.ToDictionary(r => r.Key, r => r.Result);
    }

    /// <summary>
    /// Cancels a pending thumbnail request
    /// </summary>
    /// <param name="key">Thumbnail key to cancel</param>
    /// <returns>True if request was cancelled</returns>
    public bool CancelRequest(string key)
    {
        if (_pendingRequests.TryRemove(key, out var tcs))
        {
            tcs.SetCanceled();
            _logger.LogTrace("Cancelled thumbnail request for key: {Key}", key);
            return true;
        }
        
        return false;
    }

    /// <summary>
    /// Gets the current queue statistics
    /// </summary>
    /// <returns>Queue statistics</returns>
    public ThumbnailQueueStatistics GetQueueStatistics()
    {
        var queueCount = _requestQueue.Count;
        var pendingCount = _pendingRequests.Count;
        
        // Count by priority
        var priorityCounts = new Dictionary<ThumbnailPriority, int>();
        foreach (ThumbnailPriority priority in Enum.GetValues<ThumbnailPriority>())
        {
            priorityCounts[priority] = 0;
        }

        // This is approximate since we can't efficiently iterate the concurrent queue
        var tempQueue = new List<ThumbnailRequest>();
        while (_requestQueue.TryDequeue(out var request))
        {
            tempQueue.Add(request);
            priorityCounts[request.Priority]++;
        }

        // Re-queue the requests
        foreach (var request in tempQueue)
        {
            _requestQueue.Enqueue(request);
        }

        return new ThumbnailQueueStatistics
        {
            QueuedRequests = queueCount,
            PendingRequests = pendingCount,
            PriorityCounts = priorityCounts,
            WorkerCount = _workerTasks.Length
        };
    }

    /// <summary>
    /// Main worker loop for processing thumbnail requests
    /// </summary>
    private async Task WorkerLoop()
    {
        _logger.LogDebug("Thumbnail worker started");
        
        while (!_cancellationTokenSource.Token.IsCancellationRequested)
        {
            try
            {
                await _workerSemaphore.WaitAsync(_cancellationTokenSource.Token);
                
                try
                {
                    var request = await GetNextRequestAsync();
                    if (request != null)
                    {
                        await ProcessRequestAsync(request);
                    }
                    else
                    {
                        // No requests available, wait a bit
                        await Task.Delay(100, _cancellationTokenSource.Token);
                    }
                }
                finally
                {
                    _workerSemaphore.Release();
                }
            }
            catch (OperationCanceledException)
            {
                break;
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error in thumbnail worker loop");
                await Task.Delay(1000, _cancellationTokenSource.Token); // Wait before retrying
            }
        }
        
        _logger.LogDebug("Thumbnail worker stopped");
    }

    /// <summary>
    /// Gets the next request from the queue with priority ordering
    /// </summary>
    /// <returns>Next request to process or null if queue is empty</returns>
    private async Task<ThumbnailRequest?> GetNextRequestAsync()
    {
        var requests = new List<ThumbnailRequest>();
        
        // Collect available requests
        while (_requestQueue.TryDequeue(out var request) && requests.Count < 10)
        {
            // Check if request is still valid
            if (_pendingRequests.ContainsKey(request.Key))
            {
                requests.Add(request);
            }
        }

        if (requests.Count == 0)
            return null;

        // Sort by priority (highest first) and then by request time (oldest first)
        var sortedRequests = requests
            .OrderByDescending(r => (int)r.Priority)
            .ThenBy(r => r.RequestTime)
            .ToList();

        // Re-queue the requests we're not processing
        for (int i = 1; i < sortedRequests.Count; i++)
        {
            _requestQueue.Enqueue(sortedRequests[i]);
        }

        return sortedRequests[0];
    }

    /// <summary>
    /// Processes a thumbnail generation request
    /// </summary>
    /// <param name="request">Request to process</param>
    private async Task ProcessRequestAsync(ThumbnailRequest request)
    {
        try
        {
            _logger.LogTrace("Processing thumbnail request for key: {Key}", request.Key);
            
            var thumbnail = await _imageService.GenerateThumbnailAsync(
                request.Source, 
                request.Options, 
                _cancellationTokenSource.Token);

            // Complete the request
            if (_pendingRequests.TryRemove(request.Key, out var tcs))
            {
                tcs.SetResult(thumbnail);
                
                // Raise event
                ThumbnailGenerated?.Invoke(this, new ThumbnailGeneratedEventArgs
                {
                    Key = request.Key,
                    ThumbnailData = thumbnail,
                    Priority = request.Priority,
                    ProcessingTime = DateTime.UtcNow - request.RequestTime
                });
                
                _logger.LogTrace("Completed thumbnail request for key: {Key}", request.Key);
            }
        }
        catch (OperationCanceledException)
        {
            // Handle cancellation
            if (_pendingRequests.TryRemove(request.Key, out var tcs))
            {
                tcs.SetCanceled();
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to generate thumbnail for key: {Key}", request.Key);
            
            if (_pendingRequests.TryRemove(request.Key, out var tcs))
            {
                tcs.SetException(ex);
            }
        }
    }

    /// <inheritdoc />
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    /// <summary>
    /// Disposes the background thumbnail generator
    /// </summary>
    /// <param name="disposing">True if disposing managed resources</param>
    protected virtual void Dispose(bool disposing)
    {
        if (!_disposed && disposing)
        {
            _logger.LogInformation("Shutting down BackgroundThumbnailGenerator");
            
            _cancellationTokenSource.Cancel();
            
            try
            {
                Task.WaitAll(_workerTasks, TimeSpan.FromSeconds(5));
            }
            catch (Exception ex)
            {
                _logger.LogWarning(ex, "Error waiting for worker tasks to complete");
            }
            
            // Cancel all pending requests
            foreach (var kvp in _pendingRequests)
            {
                kvp.Value.SetCanceled();
            }
            _pendingRequests.Clear();
            
            _cancellationTokenSource.Dispose();
            _workerSemaphore.Dispose();
            _disposed = true;
        }
    }
}

/// <summary>
/// Thumbnail generated event arguments
/// </summary>
public class ThumbnailGeneratedEventArgs : EventArgs
{
    /// <summary>
    /// Thumbnail key
    /// </summary>
    public string Key { get; set; } = string.Empty;
    
    /// <summary>
    /// Generated thumbnail data
    /// </summary>
    public byte[] ThumbnailData { get; set; } = null!;
    
    /// <summary>
    /// Request priority
    /// </summary>
    public BackgroundThumbnailGenerator.ThumbnailPriority Priority { get; set; }
    
    /// <summary>
    /// Time taken to process the request
    /// </summary>
    public TimeSpan ProcessingTime { get; set; }
}

/// <summary>
/// Thumbnail queue statistics
/// </summary>
public class ThumbnailQueueStatistics
{
    /// <summary>
    /// Number of queued requests
    /// </summary>
    public int QueuedRequests { get; set; }
    
    /// <summary>
    /// Number of pending requests
    /// </summary>
    public int PendingRequests { get; set; }
    
    /// <summary>
    /// Count of requests by priority
    /// </summary>
    public Dictionary<BackgroundThumbnailGenerator.ThumbnailPriority, int> PriorityCounts { get; set; } = new();
    
    /// <summary>
    /// Number of worker threads
    /// </summary>
    public int WorkerCount { get; set; }
}