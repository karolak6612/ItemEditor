namespace ItemEditor.Services;

/// <summary>
/// Interface for settings migration service
/// </summary>
public interface ISettingsMigrationService
{
    /// <summary>
    /// Checks if legacy settings exist and need migration
    /// </summary>
    /// <returns>True if legacy settings are found</returns>
    Task<bool> HasLegacySettingsAsync();
    
    /// <summary>
    /// Migrates legacy settings to modern format
    /// </summary>
    /// <param name="progress">Progress reporting callback</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Migration result</returns>
    Task<Models.MigrationResult> MigrateSettingsAsync(
        IProgress<Models.MigrationProgress>? progress = null,
        CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Creates a backup of current settings before migration
    /// </summary>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>Backup file path</returns>
    Task<string> CreateSettingsBackupAsync(CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Restores settings from a backup file
    /// </summary>
    /// <param name="backupFilePath">Path to backup file</param>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>True if restore was successful</returns>
    Task<bool> RestoreSettingsFromBackupAsync(string backupFilePath, CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Validates that migrated settings are correct
    /// </summary>
    /// <param name="cancellationToken">Cancellation token</param>
    /// <returns>True if validation passes</returns>
    Task<bool> ValidateMigratedSettingsAsync(CancellationToken cancellationToken = default);
    
    /// <summary>
    /// Gets the legacy settings file paths that will be checked for migration
    /// </summary>
    /// <returns>List of potential legacy settings file paths</returns>
    IEnumerable<string> GetLegacySettingsPaths();
}