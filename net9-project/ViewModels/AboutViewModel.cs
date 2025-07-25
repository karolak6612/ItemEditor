using CommunityToolkit.Mvvm.ComponentModel;
using System.Reflection;
using System.Runtime.InteropServices;

namespace ItemEditor.ViewModels;

/// <summary>
/// ViewModel for the About dialog
/// </summary>
public partial class AboutViewModel : ObservableObject
{
    [ObservableProperty]
    private string _applicationName = "ItemEditor";
    
    [ObservableProperty]
    private string _version = string.Empty;
    
    [ObservableProperty]
    private string _buildDate = string.Empty;
    
    [ObservableProperty]
    private string _copyright = string.Empty;
    
    [ObservableProperty]
    private string _description = string.Empty;
    
    [ObservableProperty]
    private string _frameworkVersion = string.Empty;
    
    [ObservableProperty]
    private string _operatingSystem = string.Empty;
    
    [ObservableProperty]
    private string _architecture = string.Empty;
    
    [ObservableProperty]
    private string _licenseText = string.Empty;
    
    [ObservableProperty]
    private List<ContributorInfo> _contributors = new();
    
    public AboutViewModel()
    {
        LoadApplicationInfo();
        LoadSystemInfo();
        LoadLicenseInfo();
        LoadContributors();
    }
    
    private void LoadApplicationInfo()
    {
        var assembly = Assembly.GetExecutingAssembly();
        var assemblyName = assembly.GetName();
        
        Version = assemblyName.Version?.ToString() ?? "1.0.0.0";
        
        // Get build date from assembly
        var buildDateAttribute = assembly.GetCustomAttribute<AssemblyMetadataAttribute>();
        BuildDate = buildDateAttribute?.Value ?? DateTime.Now.ToString("yyyy-MM-dd");
        
        // Get copyright information
        var copyrightAttribute = assembly.GetCustomAttribute<AssemblyCopyrightAttribute>();
        Copyright = copyrightAttribute?.Copyright ?? "© 2024 ItemEditor Contributors";
        
        // Get description
        var descriptionAttribute = assembly.GetCustomAttribute<AssemblyDescriptionAttribute>();
        Description = descriptionAttribute?.Description ?? 
            "A modern WPF application for editing OpenTibia Binary (OTB) files, sprites, and data files used in OpenTibia server development.";
    }
    
    private void LoadSystemInfo()
    {
        FrameworkVersion = Environment.Version.ToString();
        OperatingSystem = Environment.OSVersion.ToString();
        Architecture = RuntimeInformation.ProcessArchitecture.ToString();
    }
    
    private void LoadLicenseInfo()
    {
        LicenseText = """
            MIT License
            
            Copyright (c) 2024 ItemEditor Contributors
            
            Permission is hereby granted, free of charge, to any person obtaining a copy
            of this software and associated documentation files (the "Software"), to deal
            in the Software without restriction, including without limitation the rights
            to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
            copies of the Software, and to permit persons to whom the Software is
            furnished to do so, subject to the following conditions:
            
            The above copyright notice and this permission notice shall be included in all
            copies or substantial portions of the Software.
            
            THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
            IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
            FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
            AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
            LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
            OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
            SOFTWARE.
            """;
    }
    
    private void LoadContributors()
    {
        Contributors = new List<ContributorInfo>
        {
            new ContributorInfo
            {
                Name = "ItemEditor Team",
                Role = "Core Development",
                Email = "team@itemeditor.dev",
                Website = "https://github.com/itemeditor/itemeditor"
            },
            new ContributorInfo
            {
                Name = "OpenTibia Community",
                Role = "Testing & Feedback",
                Website = "https://otland.net"
            },
            new ContributorInfo
            {
                Name = "WPFUI Contributors",
                Role = "UI Framework",
                Website = "https://github.com/lepoco/wpfui"
            }
        };
    }
}

/// <summary>
/// Information about a contributor
/// </summary>
public class ContributorInfo
{
    public string Name { get; set; } = string.Empty;
    public string Role { get; set; } = string.Empty;
    public string Email { get; set; } = string.Empty;
    public string Website { get; set; } = string.Empty;
}