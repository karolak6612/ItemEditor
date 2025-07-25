using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace ItemEditor.Converters;

/// <summary>
/// Converts a string to Visibility, returning Visible for non-empty strings and Collapsed for empty/null strings
/// </summary>
public class StringToVisibilityConverter : IValueConverter
{
    /// <summary>
    /// Converts a string value to Visibility
    /// </summary>
    /// <param name="value">The string value to convert</param>
    /// <param name="targetType">The target type (Visibility)</param>
    /// <param name="parameter">Optional parameter (not used)</param>
    /// <param name="culture">The culture info</param>
    /// <returns>Visible if string is not empty, Collapsed otherwise</returns>
    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
    {
        if (value is string str && !string.IsNullOrWhiteSpace(str))
        {
            return Visibility.Visible;
        }
        
        return Visibility.Collapsed;
    }
    
    /// <summary>
    /// Converts back from Visibility to string (not implemented)
    /// </summary>
    /// <param name="value">The Visibility value</param>
    /// <param name="targetType">The target type (string)</param>
    /// <param name="parameter">Optional parameter (not used)</param>
    /// <param name="culture">The culture info</param>
    /// <returns>Not implemented</returns>
    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
    {
        throw new NotImplementedException();
    }
}