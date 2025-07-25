using Microsoft.Extensions.Logging;
using PluginInterface;
using ItemEditor.Models;
using System.Reflection;
using System.Diagnostics;

namespace ItemEditor.Services;

/// <summary>
/// Comprehensive testing suite for plugin compatibility validation
/// </summary>
public class PluginCompatibilityTester : IPluginCompatibilityTester
{
    private readonly ILogger<PluginCompatibilityTester> _logger;
    private readonly ILegacyPluginCompatibilityLayer _compatibilityLayer;

    /// <summary>
    /// Initializes a new instance of the PluginCompatibilityTester class
    /// </summary>
    /// <param name="logger">Logger instance</param>
    /// <param name="compatibilityLayer">Legacy plugin compatibility layer</param>
    public PluginCompatibilityTester(
        ILogger<PluginCompatibilityTester> logger,
        ILegacyPluginCompatibilityLayer compatibilityLayer)
    {
        _logger = logger ?? throw new ArgumentNullException(nameof(logger));
        _compatibilityLayer = compatibilityLayer ?? throw new ArgumentNullException(nameof(compatibilityLayer));
    }

    /// <summary>
    /// Runs comprehensive compatibility tests on a plugin
    /// </summary>
    /// <param name="pluginPath">Path to the plugin file</param>
    /// <returns>Compatibility test results</returns>
    public async Task<PluginCompatibilityTestResult> RunCompatibilityTestsAsync(string pluginPath)
    {
        if (string.IsNullOrEmpty(pluginPath))
            throw new ArgumentException("Plugin path cannot be null or empty", nameof(pluginPath));

        var result = new PluginCompatibilityTestResult
        {
            PluginPath = pluginPath,
            TestStartTime = DateTime.UtcNow
        };

        try
        {
            _logger.LogInformation("Starting compatibility tests for plugin: {PluginPath}", pluginPath);

            // Test 1: File existence and basic validation
            await RunFileValidationTest(result);

            if (!result.FileValidationPassed)
            {
                result.OverallResult = CompatibilityTestResult.Failed;
                return result;
            }

            // Test 2: Assembly loading test
            await RunAssemblyLoadingTest(result);

            if (!result.AssemblyLoadingPassed)
            {
                result.OverallResult = CompatibilityTestResult.Failed;
                return result;
            }

            // Test 3: Plugin type detection test
            await RunPluginTypeDetectionTest(result);

            if (!result.PluginTypesFound)
            {
                result.OverallResult = CompatibilityTestResult.Failed;
                return result;
            }

            // Test 4: Legacy compatibility test
            await RunLegacyCompatibilityTest(result);

            // Test 5: Modern plugin interface test
            await RunModernPluginInterfaceTest(result);

            // Test 6: Plugin instantiation test
            await RunPluginInstantiationTest(result);

            // Test 7: Plugin lifecycle test
            await RunPluginLifecycleTest(result);

            // Test 8: Dependency validation test
            await RunDependencyValidationTest(result);

            // Determine overall result
            DetermineOverallResult(result);

            result.TestEndTime = DateTime.UtcNow;
            result.TestDuration = result.TestEndTime - result.TestStartTime;

            _logger.LogInformation("Compatibility tests completed for plugin: {PluginPath}. Result: {Result}", 
                pluginPath, result.OverallResult);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error running compatibility tests for plugin: {PluginPath}", pluginPath);
            result.OverallResult = CompatibilityTestResult.Error;
            result.ErrorMessage = ex.Message;
            result.TestEndTime = DateTime.UtcNow;
        }

        return result;
    }

    /// <summary>
    /// Runs compatibility tests on multiple plugins
    /// </summary>
    /// <param name="pluginPaths">Paths to plugin files</param>
    /// <returns>Collection of compatibility test results</returns>
    public async Task<IEnumerable<PluginCompatibilityTestResult>> RunBatchCompatibilityTestsAsync(IEnumerable<string> pluginPaths)
    {
        if (pluginPaths == null)
            throw new ArgumentNullException(nameof(pluginPaths));

        var results = new List<PluginCompatibilityTestResult>();
        var paths = pluginPaths.ToList();

        _logger.LogInformation("Starting batch compatibility tests for {Count} plugins", paths.Count);

        var tasks = paths.Select(RunCompatibilityTestsAsync);
        var testResults = await Task.WhenAll(tasks);

        results.AddRange(testResults);

        _logger.LogInformation("Batch compatibility tests completed. {Passed} passed, {Failed} failed, {Errors} errors",
            results.Count(r => r.OverallResult == CompatibilityTestResult.Passed),
            results.Count(r => r.OverallResult == CompatibilityTestResult.Failed),
            results.Count(r => r.OverallResult == CompatibilityTestResult.Error));

        return results;
    }

    /// <summary>
    /// Generates a compatibility report
    /// </summary>
    /// <param name="testResults">Test results</param>
    /// <returns>Compatibility report</returns>
    public PluginCompatibilityReport GenerateCompatibilityReport(IEnumerable<PluginCompatibilityTestResult> testResults)
    {
        if (testResults == null)
            throw new ArgumentNullException(nameof(testResults));

        var results = testResults.ToList();
        
        return new PluginCompatibilityReport
        {
            GeneratedAt = DateTime.UtcNow,
            TotalPluginsTested = results.Count,
            PassedTests = results.Count(r => r.OverallResult == CompatibilityTestResult.Passed),
            FailedTests = results.Count(r => r.OverallResult == CompatibilityTestResult.Failed),
            ErrorTests = results.Count(r => r.OverallResult == CompatibilityTestResult.Error),
            LegacyPluginsDetected = results.Count(r => r.IsLegacyPlugin),
            ModernPluginsDetected = results.Count(r => r.IsModernPlugin),
            TestResults = results,
            CommonIssues = AnalyzeCommonIssues(results),
            Recommendations = GenerateRecommendations(results)
        };
    }

    /// <summary>
    /// Runs file validation test
    /// </summary>
    private async Task RunFileValidationTest(PluginCompatibilityTestResult result)
    {
        try
        {
            result.FileValidationPassed = File.Exists(result.PluginPath);
            
            if (result.FileValidationPassed)
            {
                var fileInfo = new FileInfo(result.PluginPath);
                result.FileSize = fileInfo.Length;
                result.FileLastModified = fileInfo.LastWriteTime;
                
                // Check file extension
                if (!string.Equals(fileInfo.Extension, ".dll", StringComparison.OrdinalIgnoreCase))
                {
                    result.TestWarnings.Add("Plugin file does not have .dll extension");
                }
            }
            else
            {
                result.TestErrors.Add("Plugin file does not exist");
            }
        }
        catch (Exception ex)
        {
            result.FileValidationPassed = false;
            result.TestErrors.Add($"File validation error: {ex.Message}");
        }
    }

    /// <summary>
    /// Runs assembly loading test
    /// </summary>
    private async Task RunAssemblyLoadingTest(PluginCompatibilityTestResult result)
    {
        try
        {
            var assemblyName = AssemblyName.GetAssemblyName(result.PluginPath);
            result.AssemblyName = assemblyName.FullName;
            result.AssemblyVersion = assemblyName.Version?.ToString() ?? "Unknown";
            result.AssemblyLoadingPassed = true;
        }
        catch (Exception ex)
        {
            result.AssemblyLoadingPassed = false;
            result.TestErrors.Add($"Assembly loading error: {ex.Message}");
        }
    }

    /// <summary>
    /// Runs plugin type detection test
    /// </summary>
    private async Task RunPluginTypeDetectionTest(PluginCompatibilityTestResult result)
    {
        try
        {
            var assembly = Assembly.LoadFrom(result.PluginPath);
            var types = assembly.GetTypes();
            
            var pluginTypes = new List<Type>();
            
            foreach (var type in types)
            {
                if (!type.IsAbstract && !type.IsInterface)
                {
                    // Check for modern plugin interface
                    if (typeof(IPlugin).IsAssignableFrom(type))
                    {
                        pluginTypes.Add(type);
                        result.IsModernPlugin = true;
                    }
                    // Check for legacy plugin interface
                    else if (typeof(ILegacyPlugin).IsAssignableFrom(type))
                    {
                        pluginTypes.Add(type);
                        result.IsLegacyPlugin = true;
                    }
                    // Check for other legacy plugin patterns
                    else if (_compatibilityLayer.IsLegacyPlugin(type))
                    {
                        pluginTypes.Add(type);
                        result.IsLegacyPlugin = true;
                    }
                }
            }
            
            result.PluginTypes = pluginTypes.Select(t => t.FullName ?? t.Name).ToList();
            result.PluginTypesFound = pluginTypes.Any();
            
            if (!result.PluginTypesFound)
            {
                result.TestErrors.Add("No plugin types found in assembly");
            }
        }
        catch (Exception ex)
        {
            result.PluginTypesFound = false;
            result.TestErrors.Add($"Plugin type detection error: {ex.Message}");
        }
    }

    /// <summary>
    /// Runs legacy compatibility test
    /// </summary>
    private async Task RunLegacyCompatibilityTest(PluginCompatibilityTestResult result)
    {
        if (!result.IsLegacyPlugin)
        {
            result.LegacyCompatibilityPassed = true; // Not applicable
            return;
        }

        try
        {
            var assembly = Assembly.LoadFrom(result.PluginPath);
            var types = assembly.GetTypes()
                .Where(t => !t.IsAbstract && !t.IsInterface && _compatibilityLayer.IsLegacyPlugin(t));

            foreach (var type in types)
            {
                var validationResult = _compatibilityLayer.ValidateLegacyPluginCompatibility(type);
                
                if (!validationResult.IsValid)
                {
                    result.TestErrors.AddRange(validationResult.Errors);
                }
                
                result.TestWarnings.AddRange(validationResult.Warnings);
            }
            
            result.LegacyCompatibilityPassed = !result.TestErrors.Any();
        }
        catch (Exception ex)
        {
            result.LegacyCompatibilityPassed = false;
            result.TestErrors.Add($"Legacy compatibility test error: {ex.Message}");
        }
    }

    /// <summary>
    /// Runs modern plugin interface test
    /// </summary>
    private async Task RunModernPluginInterfaceTest(PluginCompatibilityTestResult result)
    {
        if (!result.IsModernPlugin)
        {
            result.ModernInterfacePassed = true; // Not applicable
            return;
        }

        try
        {
            var assembly = Assembly.LoadFrom(result.PluginPath);
            var pluginTypes = assembly.GetTypes()
                .Where(t => !t.IsAbstract && !t.IsInterface && typeof(IPlugin).IsAssignableFrom(t));

            foreach (var type in pluginTypes)
            {
                // Check for required interface methods
                var requiredMethods = new[]
                {
                    nameof(IPlugin.InitializeAsync),
                    nameof(IPlugin.ShutdownAsync),
                    nameof(IPlugin.ValidateCompatibilityAsync)
                };

                foreach (var methodName in requiredMethods)
                {
                    var method = type.GetMethod(methodName);
                    if (method == null)
                    {
                        result.TestErrors.Add($"Missing required method: {methodName} in type {type.FullName}");
                    }
                }

                // Check for required properties
                var requiredProperties = new[]
                {
                    nameof(IPlugin.Name),
                    nameof(IPlugin.Version),
                    nameof(IPlugin.Description),
                    nameof(IPlugin.Author),
                    nameof(IPlugin.Metadata)
                };

                foreach (var propertyName in requiredProperties)
                {
                    var property = type.GetProperty(propertyName);
                    if (property == null)
                    {
                        result.TestErrors.Add($"Missing required property: {propertyName} in type {type.FullName}");
                    }
                }
            }
            
            result.ModernInterfacePassed = !result.TestErrors.Any();
        }
        catch (Exception ex)
        {
            result.ModernInterfacePassed = false;
            result.TestErrors.Add($"Modern interface test error: {ex.Message}");
        }
    }

    /// <summary>
    /// Runs plugin instantiation test
    /// </summary>
    private async Task RunPluginInstantiationTest(PluginCompatibilityTestResult result)
    {
        try
        {
            var assembly = Assembly.LoadFrom(result.PluginPath);
            var pluginTypes = assembly.GetTypes()
                .Where(t => !t.IsAbstract && !t.IsInterface && 
                           (typeof(IPlugin).IsAssignableFrom(t) || 
                            typeof(ILegacyPlugin).IsAssignableFrom(t) ||
                            _compatibilityLayer.IsLegacyPlugin(t)));

            var instantiatedCount = 0;
            
            foreach (var type in pluginTypes)
            {
                try
                {
                    var instance = Activator.CreateInstance(type);
                    if (instance != null)
                    {
                        instantiatedCount++;
                        
                        // Dispose if disposable
                        if (instance is IDisposable disposable)
                        {
                            disposable.Dispose();
                        }
                    }
                }
                catch (Exception ex)
                {
                    result.TestErrors.Add($"Failed to instantiate plugin type {type.FullName}: {ex.Message}");
                }
            }
            
            result.InstantiationPassed = instantiatedCount > 0 && !result.TestErrors.Any();
        }
        catch (Exception ex)
        {
            result.InstantiationPassed = false;
            result.TestErrors.Add($"Plugin instantiation test error: {ex.Message}");
        }
    }

    /// <summary>
    /// Runs plugin lifecycle test
    /// </summary>
    private async Task RunPluginLifecycleTest(PluginCompatibilityTestResult result)
    {
        try
        {
            var assembly = Assembly.LoadFrom(result.PluginPath);
            var pluginTypes = assembly.GetTypes()
                .Where(t => !t.IsAbstract && !t.IsInterface && 
                           (typeof(IPlugin).IsAssignableFrom(t) || 
                            typeof(ILegacyPlugin).IsAssignableFrom(t) ||
                            _compatibilityLayer.IsLegacyPlugin(t)))
                .Take(1); // Test only first plugin type to avoid side effects

            foreach (var type in pluginTypes)
            {
                try
                {
                    var instance = Activator.CreateInstance(type);
                    
                    if (instance is IPlugin modernPlugin)
                    {
                        // Test modern plugin lifecycle
                        await modernPlugin.InitializeAsync();
                        await modernPlugin.ShutdownAsync();
                        modernPlugin.Dispose();
                    }
                    else if (instance is ILegacyPlugin legacyPlugin)
                    {
                        // Test legacy plugin lifecycle
                        legacyPlugin.Initialize();
                        legacyPlugin.Shutdown();
                        
                        if (legacyPlugin is IDisposable disposable)
                        {
                            disposable.Dispose();
                        }
                    }
                    else if (_compatibilityLayer.IsLegacyPlugin(type))
                    {
                        // Test generic legacy plugin through adapter
                        var adapter = _compatibilityLayer.CreateLegacyPluginAdapter(instance, type);
                        adapter.Initialize();
                        adapter.Shutdown();
                        
                        if (adapter is IDisposable disposable)
                        {
                            disposable.Dispose();
                        }
                    }
                    
                    result.LifecyclePassed = true;
                    break; // Only test one plugin to avoid conflicts
                }
                catch (Exception ex)
                {
                    result.TestErrors.Add($"Plugin lifecycle test failed for {type.FullName}: {ex.Message}");
                }
            }
            
            if (!result.LifecyclePassed && !result.TestErrors.Any())
            {
                result.TestWarnings.Add("No plugin lifecycle test performed");
                result.LifecyclePassed = true; // Don't fail if no testable plugins
            }
        }
        catch (Exception ex)
        {
            result.LifecyclePassed = false;
            result.TestErrors.Add($"Plugin lifecycle test error: {ex.Message}");
        }
    }

    /// <summary>
    /// Runs dependency validation test
    /// </summary>
    private async Task RunDependencyValidationTest(PluginCompatibilityTestResult result)
    {
        try
        {
            var assembly = Assembly.LoadFrom(result.PluginPath);
            var referencedAssemblies = assembly.GetReferencedAssemblies();
            
            result.Dependencies = referencedAssemblies.Select(a => a.FullName).ToList();
            
            // Check for problematic dependencies
            var problematicDependencies = new[]
            {
                "System.Windows.Forms", // WinForms in WPF app
                "Microsoft.VisualBasic", // VB.NET runtime
            };
            
            foreach (var dependency in referencedAssemblies)
            {
                if (problematicDependencies.Any(pd => dependency.Name?.StartsWith(pd) == true))
                {
                    result.TestWarnings.Add($"Potentially problematic dependency: {dependency.Name}");
                }
            }
            
            result.DependencyValidationPassed = true;
        }
        catch (Exception ex)
        {
            result.DependencyValidationPassed = false;
            result.TestErrors.Add($"Dependency validation error: {ex.Message}");
        }
    }

    /// <summary>
    /// Determines the overall test result
    /// </summary>
    private static void DetermineOverallResult(PluginCompatibilityTestResult result)
    {
        if (result.TestErrors.Any())
        {
            result.OverallResult = CompatibilityTestResult.Failed;
        }
        else if (result.FileValidationPassed && 
                 result.AssemblyLoadingPassed && 
                 result.PluginTypesFound &&
                 result.InstantiationPassed &&
                 result.LifecyclePassed)
        {
            result.OverallResult = result.TestWarnings.Any() 
                ? CompatibilityTestResult.PassedWithWarnings 
                : CompatibilityTestResult.Passed;
        }
        else
        {
            result.OverallResult = CompatibilityTestResult.Failed;
        }
    }

    /// <summary>
    /// Analyzes common issues across test results
    /// </summary>
    private static List<string> AnalyzeCommonIssues(List<PluginCompatibilityTestResult> results)
    {
        var issues = new List<string>();
        
        var failedCount = results.Count(r => r.OverallResult == CompatibilityTestResult.Failed);
        if (failedCount > 0)
        {
            issues.Add($"{failedCount} plugins failed compatibility tests");
        }
        
        var legacyCount = results.Count(r => r.IsLegacyPlugin);
        if (legacyCount > 0)
        {
            issues.Add($"{legacyCount} legacy plugins detected - consider updating to modern plugin interface");
        }
        
        var commonErrors = results
            .SelectMany(r => r.TestErrors)
            .GroupBy(e => e)
            .Where(g => g.Count() > 1)
            .OrderByDescending(g => g.Count())
            .Take(5);
        
        foreach (var error in commonErrors)
        {
            issues.Add($"Common error ({error.Count()} plugins): {error.Key}");
        }
        
        return issues;
    }

    /// <summary>
    /// Generates recommendations based on test results
    /// </summary>
    private static List<string> GenerateRecommendations(List<PluginCompatibilityTestResult> results)
    {
        var recommendations = new List<string>();
        
        var legacyCount = results.Count(r => r.IsLegacyPlugin);
        if (legacyCount > 0)
        {
            recommendations.Add("Consider migrating legacy plugins to the modern IPlugin interface for better performance and features");
        }
        
        var failedCount = results.Count(r => r.OverallResult == CompatibilityTestResult.Failed);
        if (failedCount > 0)
        {
            recommendations.Add("Review failed plugins and update them to meet compatibility requirements");
        }
        
        var warningCount = results.Count(r => r.TestWarnings.Any());
        if (warningCount > 0)
        {
            recommendations.Add("Address warnings in plugins to ensure optimal compatibility and performance");
        }
        
        if (results.Any(r => r.Dependencies.Any(d => d.Contains("System.Windows.Forms"))))
        {
            recommendations.Add("Consider updating plugins that use Windows Forms to WPF for better integration");
        }
        
        return recommendations;
    }
}

/// <summary>
/// Interface for plugin compatibility tester
/// </summary>
public interface IPluginCompatibilityTester
{
    /// <summary>
    /// Runs comprehensive compatibility tests on a plugin
    /// </summary>
    /// <param name="pluginPath">Path to the plugin file</param>
    /// <returns>Compatibility test results</returns>
    Task<PluginCompatibilityTestResult> RunCompatibilityTestsAsync(string pluginPath);

    /// <summary>
    /// Runs compatibility tests on multiple plugins
    /// </summary>
    /// <param name="pluginPaths">Paths to plugin files</param>
    /// <returns>Collection of compatibility test results</returns>
    Task<IEnumerable<PluginCompatibilityTestResult>> RunBatchCompatibilityTestsAsync(IEnumerable<string> pluginPaths);

    /// <summary>
    /// Generates a compatibility report
    /// </summary>
    /// <param name="testResults">Test results</param>
    /// <returns>Compatibility report</returns>
    PluginCompatibilityReport GenerateCompatibilityReport(IEnumerable<PluginCompatibilityTestResult> testResults);
}

/// <summary>
/// Plugin compatibility test result
/// </summary>
public class PluginCompatibilityTestResult
{
    /// <summary>
    /// Plugin file path
    /// </summary>
    public string PluginPath { get; set; } = string.Empty;

    /// <summary>
    /// Test start time
    /// </summary>
    public DateTime TestStartTime { get; set; }

    /// <summary>
    /// Test end time
    /// </summary>
    public DateTime TestEndTime { get; set; }

    /// <summary>
    /// Test duration
    /// </summary>
    public TimeSpan TestDuration { get; set; }

    /// <summary>
    /// Overall test result
    /// </summary>
    public CompatibilityTestResult OverallResult { get; set; }

    /// <summary>
    /// Error message if test failed
    /// </summary>
    public string? ErrorMessage { get; set; }

    /// <summary>
    /// File validation passed
    /// </summary>
    public bool FileValidationPassed { get; set; }

    /// <summary>
    /// Assembly loading passed
    /// </summary>
    public bool AssemblyLoadingPassed { get; set; }

    /// <summary>
    /// Plugin types found
    /// </summary>
    public bool PluginTypesFound { get; set; }

    /// <summary>
    /// Legacy compatibility passed
    /// </summary>
    public bool LegacyCompatibilityPassed { get; set; }

    /// <summary>
    /// Modern interface passed
    /// </summary>
    public bool ModernInterfacePassed { get; set; }

    /// <summary>
    /// Instantiation passed
    /// </summary>
    public bool InstantiationPassed { get; set; }

    /// <summary>
    /// Lifecycle test passed
    /// </summary>
    public bool LifecyclePassed { get; set; }

    /// <summary>
    /// Dependency validation passed
    /// </summary>
    public bool DependencyValidationPassed { get; set; }

    /// <summary>
    /// Is legacy plugin
    /// </summary>
    public bool IsLegacyPlugin { get; set; }

    /// <summary>
    /// Is modern plugin
    /// </summary>
    public bool IsModernPlugin { get; set; }

    /// <summary>
    /// Assembly name
    /// </summary>
    public string? AssemblyName { get; set; }

    /// <summary>
    /// Assembly version
    /// </summary>
    public string? AssemblyVersion { get; set; }

    /// <summary>
    /// File size
    /// </summary>
    public long FileSize { get; set; }

    /// <summary>
    /// File last modified
    /// </summary>
    public DateTime FileLastModified { get; set; }

    /// <summary>
    /// Plugin types found
    /// </summary>
    public List<string> PluginTypes { get; set; } = new();

    /// <summary>
    /// Dependencies
    /// </summary>
    public List<string> Dependencies { get; set; } = new();

    /// <summary>
    /// Test errors
    /// </summary>
    public List<string> TestErrors { get; set; } = new();

    /// <summary>
    /// Test warnings
    /// </summary>
    public List<string> TestWarnings { get; set; } = new();
}

/// <summary>
/// Plugin compatibility report
/// </summary>
public class PluginCompatibilityReport
{
    /// <summary>
    /// Report generated at
    /// </summary>
    public DateTime GeneratedAt { get; set; }

    /// <summary>
    /// Total plugins tested
    /// </summary>
    public int TotalPluginsTested { get; set; }

    /// <summary>
    /// Passed tests
    /// </summary>
    public int PassedTests { get; set; }

    /// <summary>
    /// Failed tests
    /// </summary>
    public int FailedTests { get; set; }

    /// <summary>
    /// Error tests
    /// </summary>
    public int ErrorTests { get; set; }

    /// <summary>
    /// Legacy plugins detected
    /// </summary>
    public int LegacyPluginsDetected { get; set; }

    /// <summary>
    /// Modern plugins detected
    /// </summary>
    public int ModernPluginsDetected { get; set; }

    /// <summary>
    /// Test results
    /// </summary>
    public List<PluginCompatibilityTestResult> TestResults { get; set; } = new();

    /// <summary>
    /// Common issues
    /// </summary>
    public List<string> CommonIssues { get; set; } = new();

    /// <summary>
    /// Recommendations
    /// </summary>
    public List<string> Recommendations { get; set; } = new();
}

/// <summary>
/// Compatibility test result enumeration
/// </summary>
public enum CompatibilityTestResult
{
    /// <summary>
    /// Test passed
    /// </summary>
    Passed,

    /// <summary>
    /// Test passed with warnings
    /// </summary>
    PassedWithWarnings,

    /// <summary>
    /// Test failed
    /// </summary>
    Failed,

    /// <summary>
    /// Test error
    /// </summary>
    Error
}