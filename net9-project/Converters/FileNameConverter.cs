using System.Globalization;
using System.IO;
using System.Windows.Data;

namespace ItemEditor.Converters;

/// <summary>
/// Converter to extract file name from full path
/// </summary>
public class FileNameConverter : IValueConverter
{
    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
    {
        if (value is string filePath && !string.IsNullOrEmpty(filePath))
        {
            return Path.GetFileName(filePath);
        }
        
        return value;
    }

    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
    {
        throw new NotImplementedException();
    }
}