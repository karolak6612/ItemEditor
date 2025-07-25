using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace ItemEditor.Converters;

/// <summary>
/// Converts boolean values to Visibility values
/// </summary>
public class BooleanToVisibilityConverter : IValueConverter
{
    /// <summary>
    /// Gets or sets whether to invert the conversion (true = Collapsed, false = Visible)
    /// </summary>
    public bool Invert { get; set; }

    /// <summary>
    /// Gets or sets whether to use Hidden instead of Collapsed for false values
    /// </summary>
    public bool UseHidden { get; set; }

    /// <summary>
    /// Converts a boolean value to a Visibility value
    /// </summary>
    /// <param name="value">The boolean value</param>
    /// <param name="targetType">The target type (Visibility)</param>
    /// <param name="parameter">Optional parameter</param>
    /// <param name="culture">The culture info</param>
    /// <returns>Visibility value</returns>
    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
    {
        bool boolValue = false;

        if (value is bool b)
        {
            boolValue = b;
        }
        else if (value is bool?)
        {
            boolValue = ((bool?)value) ?? false;
        }
        else if (value != null)
        {
            // Try to parse string values
            if (bool.TryParse(value.ToString(), out bool parsed))
            {
                boolValue = parsed;
            }
        }

        // Apply inversion if requested
        if (Invert)
        {
            boolValue = !boolValue;
        }

        // Return appropriate visibility
        if (boolValue)
        {
            return Visibility.Visible;
        }
        else
        {
            return UseHidden ? Visibility.Hidden : Visibility.Collapsed;
        }
    }

    /// <summary>
    /// Converts a Visibility value back to a boolean value
    /// </summary>
    /// <param name="value">The Visibility value</param>
    /// <param name="targetType">The target type (bool)</param>
    /// <param name="parameter">Optional parameter</param>
    /// <param name="culture">The culture info</param>
    /// <returns>Boolean value</returns>
    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
    {
        if (value is Visibility visibility)
        {
            bool result = visibility == Visibility.Visible;
            
            // Apply inversion if requested
            if (Invert)
            {
                result = !result;
            }
            
            return result;
        }

        return false;
    }
}

/// <summary>
/// Inverted boolean to visibility converter (true = Collapsed, false = Visible)
/// </summary>
public class InvertedBooleanToVisibilityConverter : BooleanToVisibilityConverter
{
    /// <summary>
    /// Initializes a new instance of the InvertedBooleanToVisibilityConverter class
    /// </summary>
    public InvertedBooleanToVisibilityConverter()
    {
        Invert = true;
    }
}

/// <summary>
/// Boolean to visibility converter that uses Hidden instead of Collapsed
/// </summary>
public class BooleanToVisibilityHiddenConverter : BooleanToVisibilityConverter
{
    /// <summary>
    /// Initializes a new instance of the BooleanToVisibilityHiddenConverter class
    /// </summary>
    public BooleanToVisibilityHiddenConverter()
    {
        UseHidden = true;
    }
}