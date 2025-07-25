using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using ItemEditor.Models;
using ItemEditor.Services;
using System.Collections.ObjectModel;
using System.IO;

namespace ItemEditor.ViewModels;

/// <summary>
/// ViewModel for the OTB comparison dialog
/// </summary>
public partial class CompareOtbViewModel : ObservableObject
{
    private readonly IFileService _fileService;
    
    [ObservableProperty]
    private string _leftFilePath = string.Empty;
    
    [ObservableProperty]
    private string _rightFilePath = string.Empty;
    
    [ObservableProperty]
    private string _leftFileName = string.Empty;
    
    [ObservableProperty]
    private string _rightFileName = string.Empty;
    
    [ObservableProperty]
    private bool _isComparing;
    
    [ObservableProperty]
    private bool _hasComparison;
    
    [ObservableProperty]
    private string _comparisonStatus = string.Empty;
    
    [ObservableProperty]
    private ObservableCollection<ComparisonResult> _differences = new();
    
    [ObservableProperty]
    private ComparisonResult? _selectedDifference;
    
    [ObservableProperty]
    private ComparisonSummary? _summary;
    
    [ObservableProperty]
    private double _progressValue;
    
    [ObservableProperty]
    private string _progressText = string.Empty;
    
    public CompareOtbViewModel(IFileService fileService)
    {
        _fileService = fileService;
        
        // Initialize commands
        SelectLeftFileCommand = new AsyncRelayCommand(SelectLeftFileAsync);
        SelectRightFileCommand = new AsyncRelayCommand(SelectRightFileAsync);
        CompareFilesCommand = new AsyncRelayCommand(CompareFilesAsync, CanCompareFiles);
        ExportResultsCommand = new AsyncRelayCommand(ExportResultsAsync, () => HasComparison);
        ClearComparisonCommand = new RelayCommand(ClearComparison);
    }
    
    public IAsyncRelayCommand SelectLeftFileCommand { get; }
    public IAsyncRelayCommand SelectRightFileCommand { get; }
    public IAsyncRelayCommand CompareFilesCommand { get; }
    public IAsyncRelayCommand ExportResultsCommand { get; }
    public IRelayCommand ClearComparisonCommand { get; }
    
    private bool CanCompareFiles()
    {
        return !string.IsNullOrEmpty(LeftFilePath) && 
               !string.IsNullOrEmpty(RightFilePath) && 
               !IsComparing;
    }
    
    private async Task SelectLeftFileAsync()
    {
        var dialog = new Microsoft.Win32.OpenFileDialog
        {
            Title = "Select Left OTB File",
            Filter = "OTB Files (*.otb)|*.otb|All Files (*.*)|*.*",
            CheckFileExists = true
        };
        
        if (dialog.ShowDialog() == true)
        {
            LeftFilePath = dialog.FileName;
            LeftFileName = Path.GetFileName(dialog.FileName);
            CompareFilesCommand.NotifyCanExecuteChanged();
        }
    }
    
    private async Task SelectRightFileAsync()
    {
        var dialog = new Microsoft.Win32.OpenFileDialog
        {
            Title = "Select Right OTB File",
            Filter = "OTB Files (*.otb)|*.otb|All Files (*.*)|*.*",
            CheckFileExists = true
        };
        
        if (dialog.ShowDialog() == true)
        {
            RightFilePath = dialog.FileName;
            RightFileName = Path.GetFileName(dialog.FileName);
            CompareFilesCommand.NotifyCanExecuteChanged();
        }
    }
    
    private async Task CompareFilesAsync()
    {
        try
        {
            IsComparing = true;
            HasComparison = false;
            Differences.Clear();
            ProgressValue = 0;
            ProgressText = "Loading files...";
            
            // Load both files
            var leftItems = await _fileService.LoadItemsAsync(LeftFilePath);
            ProgressValue = 25;
            ProgressText = "Loading right file...";
            
            var rightItems = await _fileService.LoadItemsAsync(RightFilePath);
            ProgressValue = 50;
            ProgressText = "Comparing items...";
            
            // Perform comparison
            var results = await CompareItemsAsync(leftItems.ToList(), rightItems.ToList());
            
            ProgressValue = 100;
            ProgressText = "Comparison complete";
            
            // Update results
            foreach (var result in results)
            {
                Differences.Add(result);
            }
            
            // Create summary
            Summary = new ComparisonSummary
            {
                LeftFileItemCount = leftItems.Count(),
                RightFileItemCount = rightItems.Count(),
                AddedItems = results.Count(r => r.Type == ComparisonType.Added),
                RemovedItems = results.Count(r => r.Type == ComparisonType.Removed),
                ModifiedItems = results.Count(r => r.Type == ComparisonType.Modified),
                UnchangedItems = results.Count(r => r.Type == ComparisonType.Unchanged)
            };
            
            HasComparison = true;
            ComparisonStatus = $"Found {Differences.Count} differences";
            ExportResultsCommand.NotifyCanExecuteChanged();
        }
        catch (Exception ex)
        {
            ComparisonStatus = $"Error: {ex.Message}";
        }
        finally
        {
            IsComparing = false;
        }
    }
    
    private async Task<List<ComparisonResult>> CompareItemsAsync(List<Item> leftItems, List<Item> rightItems)
    {
        var results = new List<ComparisonResult>();
        
        // Create dictionaries for faster lookup
        var leftDict = leftItems.ToDictionary(i => i.Id, i => i);
        var rightDict = rightItems.ToDictionary(i => i.Id, i => i);
        
        var allIds = leftDict.Keys.Union(rightDict.Keys).OrderBy(id => id);
        
        foreach (var id in allIds)
        {
            var hasLeft = leftDict.TryGetValue(id, out var leftItem);
            var hasRight = rightDict.TryGetValue(id, out var rightItem);
            
            if (hasLeft && hasRight)
            {
                // Compare items
                var differences = CompareItems(leftItem!, rightItem!);
                if (differences.Any())
                {
                    results.Add(new ComparisonResult
                    {
                        Type = ComparisonType.Modified,
                        ItemId = id,
                        LeftItem = leftItem,
                        RightItem = rightItem,
                        PropertyDifferences = differences
                    });
                }
                else
                {
                    results.Add(new ComparisonResult
                    {
                        Type = ComparisonType.Unchanged,
                        ItemId = id,
                        LeftItem = leftItem,
                        RightItem = rightItem
                    });
                }
            }
            else if (hasLeft && !hasRight)
            {
                results.Add(new ComparisonResult
                {
                    Type = ComparisonType.Removed,
                    ItemId = id,
                    LeftItem = leftItem,
                    RightItem = null
                });
            }
            else if (!hasLeft && hasRight)
            {
                results.Add(new ComparisonResult
                {
                    Type = ComparisonType.Added,
                    ItemId = id,
                    LeftItem = null,
                    RightItem = rightItem
                });
            }
        }
        
        return results;
    }
    
    private List<PropertyDifference> CompareItems(Item left, Item right)
    {
        var differences = new List<PropertyDifference>();
        
        if (left.Name != right.Name)
        {
            differences.Add(new PropertyDifference
            {
                PropertyName = "Name",
                LeftValue = left.Name,
                RightValue = right.Name
            });
        }
        
        if (left.Type != right.Type)
        {
            differences.Add(new PropertyDifference
            {
                PropertyName = "Type",
                LeftValue = left.Type.ToString(),
                RightValue = right.Type.ToString()
            });
        }
        
        if (left.IsStackable != right.IsStackable)
        {
            differences.Add(new PropertyDifference
            {
                PropertyName = "IsStackable",
                LeftValue = left.IsStackable.ToString(),
                RightValue = right.IsStackable.ToString()
            });
        }
        
        // Add more property comparisons as needed
        
        return differences;
    }
    
    private async Task ExportResultsAsync()
    {
        var dialog = new Microsoft.Win32.SaveFileDialog
        {
            Title = "Export Comparison Results",
            Filter = "CSV Files (*.csv)|*.csv|Text Files (*.txt)|*.txt|All Files (*.*)|*.*",
            DefaultExt = "csv"
        };
        
        if (dialog.ShowDialog() == true)
        {
            try
            {
                await ExportToFileAsync(dialog.FileName);
                ComparisonStatus = $"Results exported to {Path.GetFileName(dialog.FileName)}";
            }
            catch (Exception ex)
            {
                ComparisonStatus = $"Export failed: {ex.Message}";
            }
        }
    }
    
    private async Task ExportToFileAsync(string filePath)
    {
        var lines = new List<string>
        {
            "Item ID,Change Type,Property,Left Value,Right Value"
        };
        
        foreach (var diff in Differences.Where(d => d.Type != ComparisonType.Unchanged))
        {
            if (diff.PropertyDifferences?.Any() == true)
            {
                foreach (var propDiff in diff.PropertyDifferences)
                {
                    lines.Add($"{diff.ItemId},{diff.Type},{propDiff.PropertyName},\"{propDiff.LeftValue}\",\"{propDiff.RightValue}\"");
                }
            }
            else
            {
                lines.Add($"{diff.ItemId},{diff.Type},,,");
            }
        }
        
        await File.WriteAllLinesAsync(filePath, lines);
    }
    
    private void ClearComparison()
    {
        LeftFilePath = string.Empty;
        RightFilePath = string.Empty;
        LeftFileName = string.Empty;
        RightFileName = string.Empty;
        Differences.Clear();
        Summary = null;
        HasComparison = false;
        ComparisonStatus = string.Empty;
        CompareFilesCommand.NotifyCanExecuteChanged();
        ExportResultsCommand.NotifyCanExecuteChanged();
    }
}

/// <summary>
/// Represents a comparison result between two items
/// </summary>
public class ComparisonResult
{
    public ComparisonType Type { get; set; }
    public ushort ItemId { get; set; }
    public Item? LeftItem { get; set; }
    public Item? RightItem { get; set; }
    public List<PropertyDifference> PropertyDifferences { get; set; } = new();
}

/// <summary>
/// Represents a difference in a property between two items
/// </summary>
public class PropertyDifference
{
    public string PropertyName { get; set; } = string.Empty;
    public string LeftValue { get; set; } = string.Empty;
    public string RightValue { get; set; } = string.Empty;
}

/// <summary>
/// Summary of comparison results
/// </summary>
public class ComparisonSummary
{
    public int LeftFileItemCount { get; set; }
    public int RightFileItemCount { get; set; }
    public int AddedItems { get; set; }
    public int RemovedItems { get; set; }
    public int ModifiedItems { get; set; }
    public int UnchangedItems { get; set; }
}

/// <summary>
/// Types of comparison results
/// </summary>
public enum ComparisonType
{
    Added,
    Removed,
    Modified,
    Unchanged
}