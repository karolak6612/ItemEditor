using System.Globalization;
using System.Windows.Data;

namespace ItemEditor.Converters;

/// <summary>
/// Converts boolean values to string representations
/// </summary>
public class BooleanToStringConverter : IValueConverter
{
    /// <summary>
    /// Converts a boolean value to a string
    /// </summary>
    /// <param name="value">The boolean value</param>
    /// <param name="targetType">The target type</param>
    /// <param name="parameter">Converter parameter in format "TrueValue|FalseValue"</param>
    /// <param name="culture">The culture</param>
    /// <returns>String representation of the boolean value</returns>
    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
    {
        if (value is not bool boolValue)
            return "Unknown";
        
        if (parameter is string paramString && paramString.Contains('|'))
        {
            var parts = paramString.Split('|');
            if (parts.Length >= 2)
            {
                return boolValue ? parts[0] : parts[1];
            }
        }
        
        return boolValue ? "True" : "False";
    }
    
    /// <summary>
    /// Converts a string back to a boolean value
    /// </summary>
    /// <param name="value">The string value</param>
    /// <param name="targetType">The target type</param>
    /// <param name="parameter">Converter parameter</param>
    /// <param name="culture">The culture</param>
    /// <returns>Boolean value</returns>
    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
    {
        if (value is not string stringValue)
            return false;
        
        if (parameter is string paramString && paramString.Contains('|'))
        {
            var parts = paramString.Split('|');
            if (parts.Length >= 2)
            {
                return stringValue.Equals(parts[0], StringComparison.OrdinalIgnoreCase);
            }
        }
        
        return stringValue.Equals("True", StringComparison.OrdinalIgnoreCase);
    }
}