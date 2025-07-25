using CommunityToolkit.Mvvm.ComponentModel;
using ItemEditor.Models;

namespace ItemEditor.ViewModels;

/// <summary>
/// ViewModel for the settings migration dialog
/// </summary>
public partial class SettingsMigrationViewModel : ObservableObject
{
    [ObservableProperty]
    private string _currentStep = "Initializing...";
    
    [ObservableProperty]
    private int _progressPercentage = 0;
    
    [ObservableProperty]
    private int _totalSteps = 1;
    
    [ObservableProperty]
    private int _currentStepNumber = 0;
    
    [ObservableProperty]
    private bool _isComplete = false;
    
    [ObservableProperty]
    private string? _details;
    
    [ObservableProperty]
    private bool _migrationSuccessful = false;
    
    [ObservableProperty]
    private bool _migrationFailed = false;
    
    [ObservableProperty]
    private string _resultMessage = string.Empty;
    
    [ObservableProperty]
    private int _settingsMigrated = 0;
    
    [ObservableProperty]
    private string? _legacySettingsPath;
    
    [ObservableProperty]
    private string? _newSettingsPath;
    
    [ObservableProperty]
    private List<string> _errors = new();
    
    [ObservableProperty]
    private List<string> _warnings = new();
    
    [ObservableProperty]
    private bool _showRetryButton = false;
    
    /// <summary>
    /// Gets whether there are any errors
    /// </summary>
    public bool HasErrors => Errors.Count > 0;
    
    /// <summary>
    /// Gets whether there are any warnings
    /// </summary>
    public bool HasWarnings => Warnings.Count > 0;
    
    /// <summary>
    /// Updates the progress from a MigrationProgress object
    /// </summary>
    /// <param name="progress">Migration progress information</param>
    public void UpdateProgress(MigrationProgress progress)
    {
        CurrentStep = progress.CurrentStep;
        ProgressPercentage = progress.ProgressPercentage;
        TotalSteps = progress.TotalSteps;
        CurrentStepNumber = progress.CurrentStepNumber;
        IsComplete = progress.IsComplete;
        Details = progress.Details;
    }
    
    /// <summary>
    /// Updates the result from a MigrationResult object
    /// </summary>
    /// <param name="result">Migration result information</param>
    public void UpdateResult(MigrationResult result)
    {
        MigrationSuccessful = result.Success;
        MigrationFailed = !result.Success;
        ResultMessage = result.Message;
        SettingsMigrated = result.SettingsMigrated;
        LegacySettingsPath = result.LegacySettingsPath;
        NewSettingsPath = result.NewSettingsPath;
        Errors = result.Errors;
        Warnings = result.Warnings;
        ShowRetryButton = !result.Success;
        
        // Trigger property change notifications for computed properties
        OnPropertyChanged(nameof(HasErrors));
        OnPropertyChanged(nameof(HasWarnings));
    }
    
    /// <summary>
    /// Resets the migration state for retry
    /// </summary>
    public void Reset()
    {
        CurrentStep = "Initializing...";
        ProgressPercentage = 0;
        CurrentStepNumber = 0;
        IsComplete = false;
        Details = null;
        MigrationSuccessful = false;
        MigrationFailed = false;
        ResultMessage = string.Empty;
        SettingsMigrated = 0;
        LegacySettingsPath = null;
        NewSettingsPath = null;
        Errors = new List<string>();
        Warnings = new List<string>();
        ShowRetryButton = false;
        
        OnPropertyChanged(nameof(HasErrors));
        OnPropertyChanged(nameof(HasWarnings));
    }
}