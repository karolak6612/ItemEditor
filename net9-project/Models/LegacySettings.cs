using System.ComponentModel;

namespace ItemEditor.Models;

/// <summary>
/// Legacy settings model for migration from .NET Framework version
/// </summary>
public class LegacySettings
{
    /// <summary>
    /// Gets or sets the client directory path
    /// </summary>
    public string ClientDirectory { get; set; } = string.Empty;
    
    /// <summary>
    /// Gets or sets whether extended features are enabled
    /// </summary>
    public bool Extended { get; set; } = false;
    
    /// <summary>
    /// Gets or sets whether transparency is supported
    /// </summary>
    public bool Transparency { get; set; } = false;
    
    /// <summary>
    /// Gets or sets the DAT file signature
    /// </summary>
    public uint DatSignature { get; set; } = 0;
    
    /// <summary>
    /// Gets or sets the SPR file signature
    /// </summary>
    public uint SprSignature { get; set; } = 0;
    
    /// <summary>
    /// Gets or sets whether frame durations are supported
    /// </summary>
    public bool FrameDurations { get; set; } = false;
}

/// <summary>
/// Migration result information
/// </summary>
public class MigrationResult
{
    /// <summary>
    /// Gets or sets whether the migration was successful
    /// </summary>
    public bool Success { get; set; }
    
    /// <summary>
    /// Gets or sets the migration message
    /// </summary>
    public string Message { get; set; } = string.Empty;
    
    /// <summary>
    /// Gets or sets the number of settings migrated
    /// </summary>
    public int SettingsMigrated { get; set; }
    
    /// <summary>
    /// Gets or sets any errors that occurred during migration
    /// </summary>
    public List<string> Errors { get; set; } = new();
    
    /// <summary>
    /// Gets or sets any warnings that occurred during migration
    /// </summary>
    public List<string> Warnings { get; set; } = new();
    
    /// <summary>
    /// Gets or sets the legacy settings file path that was migrated
    /// </summary>
    public string? LegacySettingsPath { get; set; }
    
    /// <summary>
    /// Gets or sets the new settings file path
    /// </summary>
    public string? NewSettingsPath { get; set; }
}

/// <summary>
/// Migration progress information
/// </summary>
public class MigrationProgress
{
    /// <summary>
    /// Gets or sets the current step being performed
    /// </summary>
    public string CurrentStep { get; set; } = string.Empty;
    
    /// <summary>
    /// Gets or sets the progress percentage (0-100)
    /// </summary>
    public int ProgressPercentage { get; set; }
    
    /// <summary>
    /// Gets or sets the total number of steps
    /// </summary>
    public int TotalSteps { get; set; }
    
    /// <summary>
    /// Gets or sets the current step number
    /// </summary>
    public int CurrentStepNumber { get; set; }
    
    /// <summary>
    /// Gets or sets whether the migration is complete
    /// </summary>
    public bool IsComplete { get; set; }
    
    /// <summary>
    /// Gets or sets any additional details about the current step
    /// </summary>
    public string? Details { get; set; }
}