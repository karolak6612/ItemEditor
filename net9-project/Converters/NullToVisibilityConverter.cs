using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace ItemEditor.Converters;

/// <summary>
/// Converts null values to Visibility
/// </summary>
public class NullToVisibilityConverter : IValueConverter
{
    /// <summary>
    /// Converts a value to Visibility based on whether it's null
    /// </summary>
    /// <param name="value">The value to check</param>
    /// <param name="targetType">The target type</param>
    /// <param name="parameter">Converter parameter (optional)</param>
    /// <param name="culture">The culture</param>
    /// <returns>Visibility.Visible if not null, Visibility.Collapsed if null</returns>
    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
    {
        bool isInverted = parameter?.ToString()?.ToLowerInvariant() == "invert";
        bool isNull = value == null;
        
        if (isInverted)
        {
            return isNull ? Visibility.Visible : Visibility.Collapsed;
        }
        
        return isNull ? Visibility.Collapsed : Visibility.Visible;
    }
    
    /// <summary>
    /// Not implemented for this converter
    /// </summary>
    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
    {
        throw new NotImplementedException();
    }
}