namespace ItemEditor.Services;

/// <summary>
/// Interface for global exception handling service
/// </summary>
public interface IGlobalExceptionHandler
{
    /// <summary>
    /// Initializes the global exception handler
    /// </summary>
    void Initialize();
    
    /// <summary>
    /// Handles an exception with optional context information
    /// </summary>
    /// <param name="exception">The exception to handle</param>
    /// <param name="context">Optional context information</param>
    void HandleException(Exception exception, string? context = null);
}