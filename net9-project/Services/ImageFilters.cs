using System.Windows.Media;
using System.Windows.Media.Effects;
using System.Windows.Media.Imaging;

namespace ItemEditor.Services;

/// <summary>
/// Brightness adjustment filter
/// </summary>
public class BrightnessFilter : ImageFilter
{
    /// <summary>
    /// Brightness adjustment value (-1.0 to 1.0)
    /// </summary>
    public double Brightness { get; set; }

    /// <summary>
    /// Initializes a new instance of the BrightnessFilter class
    /// </summary>
    /// <param name="brightness">Brightness adjustment (-1.0 to 1.0)</param>
    public BrightnessFilter(double brightness)
    {
        Brightness = Math.Clamp(brightness, -1.0, 1.0);
    }

    /// <inheritdoc />
    public override string Name => $"Brightness ({Brightness:F2})";

    /// <inheritdoc />
    public override BitmapSource Apply(BitmapSource source)
    {
        if (Math.Abs(Brightness) < 0.01)
            return source;

        var stride = source.PixelWidth * 4;
        var pixelData = new byte[stride * source.PixelHeight];
        source.CopyPixels(pixelData, stride, 0);

        var adjustment = (int)(Brightness * 255);

        for (int i = 0; i < pixelData.Length; i += 4)
        {
            // Adjust RGB channels (skip alpha)
            pixelData[i] = (byte)Math.Clamp(pixelData[i] + adjustment, 0, 255);     // B
            pixelData[i + 1] = (byte)Math.Clamp(pixelData[i + 1] + adjustment, 0, 255); // G
            pixelData[i + 2] = (byte)Math.Clamp(pixelData[i + 2] + adjustment, 0, 255); // R
        }

        var result = BitmapSource.Create(
            source.PixelWidth, source.PixelHeight,
            source.DpiX, source.DpiY,
            source.Format, source.Palette,
            pixelData, stride);

        result.Freeze();
        return result;
    }
}

/// <summary>
/// Contrast adjustment filter
/// </summary>
public class ContrastFilter : ImageFilter
{
    /// <summary>
    /// Contrast adjustment value (0.0 to 2.0, 1.0 = no change)
    /// </summary>
    public double Contrast { get; set; }

    /// <summary>
    /// Initializes a new instance of the ContrastFilter class
    /// </summary>
    /// <param name="contrast">Contrast adjustment (0.0 to 2.0)</param>
    public ContrastFilter(double contrast)
    {
        Contrast = Math.Clamp(contrast, 0.0, 2.0);
    }

    /// <inheritdoc />
    public override string Name => $"Contrast ({Contrast:F2})";

    /// <inheritdoc />
    public override BitmapSource Apply(BitmapSource source)
    {
        if (Math.Abs(Contrast - 1.0) < 0.01)
            return source;

        var stride = source.PixelWidth * 4;
        var pixelData = new byte[stride * source.PixelHeight];
        source.CopyPixels(pixelData, stride, 0);

        var factor = Contrast;
        var offset = 128 * (1 - factor);

        for (int i = 0; i < pixelData.Length; i += 4)
        {
            // Adjust RGB channels (skip alpha)
            pixelData[i] = (byte)Math.Clamp(pixelData[i] * factor + offset, 0, 255);     // B
            pixelData[i + 1] = (byte)Math.Clamp(pixelData[i + 1] * factor + offset, 0, 255); // G
            pixelData[i + 2] = (byte)Math.Clamp(pixelData[i + 2] * factor + offset, 0, 255); // R
        }

        var result = BitmapSource.Create(
            source.PixelWidth, source.PixelHeight,
            source.DpiX, source.DpiY,
            source.Format, source.Palette,
            pixelData, stride);

        result.Freeze();
        return result;
    }
}

/// <summary>
/// Grayscale conversion filter
/// </summary>
public class GrayscaleFilter : ImageFilter
{
    /// <inheritdoc />
    public override string Name => "Grayscale";

    /// <inheritdoc />
    public override BitmapSource Apply(BitmapSource source)
    {
        var stride = source.PixelWidth * 4;
        var pixelData = new byte[stride * source.PixelHeight];
        source.CopyPixels(pixelData, stride, 0);

        for (int i = 0; i < pixelData.Length; i += 4)
        {
            // Calculate grayscale value using luminance formula
            var gray = (byte)(0.299 * pixelData[i + 2] + 0.587 * pixelData[i + 1] + 0.114 * pixelData[i]);
            
            pixelData[i] = gray;     // B
            pixelData[i + 1] = gray; // G
            pixelData[i + 2] = gray; // R
            // Keep alpha unchanged
        }

        var result = BitmapSource.Create(
            source.PixelWidth, source.PixelHeight,
            source.DpiX, source.DpiY,
            source.Format, source.Palette,
            pixelData, stride);

        result.Freeze();
        return result;
    }
}

/// <summary>
/// Color tint filter
/// </summary>
public class TintFilter : ImageFilter
{
    /// <summary>
    /// Tint color
    /// </summary>
    public Color TintColor { get; set; }

    /// <summary>
    /// Tint strength (0.0 to 1.0)
    /// </summary>
    public double Strength { get; set; }

    /// <summary>
    /// Initializes a new instance of the TintFilter class
    /// </summary>
    /// <param name="tintColor">Tint color</param>
    /// <param name="strength">Tint strength (0.0 to 1.0)</param>
    public TintFilter(Color tintColor, double strength)
    {
        TintColor = tintColor;
        Strength = Math.Clamp(strength, 0.0, 1.0);
    }

    /// <inheritdoc />
    public override string Name => $"Tint ({TintColor}, {Strength:F2})";

    /// <inheritdoc />
    public override BitmapSource Apply(BitmapSource source)
    {
        if (Strength < 0.01)
            return source;

        var stride = source.PixelWidth * 4;
        var pixelData = new byte[stride * source.PixelHeight];
        source.CopyPixels(pixelData, stride, 0);

        var tintB = TintColor.B;
        var tintG = TintColor.G;
        var tintR = TintColor.R;
        var invStrength = 1.0 - Strength;

        for (int i = 0; i < pixelData.Length; i += 4)
        {
            // Blend original color with tint color
            pixelData[i] = (byte)(pixelData[i] * invStrength + tintB * Strength);     // B
            pixelData[i + 1] = (byte)(pixelData[i + 1] * invStrength + tintG * Strength); // G
            pixelData[i + 2] = (byte)(pixelData[i + 2] * invStrength + tintR * Strength); // R
            // Keep alpha unchanged
        }

        var result = BitmapSource.Create(
            source.PixelWidth, source.PixelHeight,
            source.DpiX, source.DpiY,
            source.Format, source.Palette,
            pixelData, stride);

        result.Freeze();
        return result;
    }
}

/// <summary>
/// Blur filter using simple box blur
/// </summary>
public class BlurFilter : ImageFilter
{
    /// <summary>
    /// Blur radius in pixels
    /// </summary>
    public int Radius { get; set; }

    /// <summary>
    /// Initializes a new instance of the BlurFilter class
    /// </summary>
    /// <param name="radius">Blur radius in pixels</param>
    public BlurFilter(int radius)
    {
        Radius = Math.Max(1, radius);
    }

    /// <inheritdoc />
    public override string Name => $"Blur ({Radius}px)";

    /// <inheritdoc />
    public override BitmapSource Apply(BitmapSource source)
    {
        if (Radius <= 1)
            return source;

        var width = source.PixelWidth;
        var height = source.PixelHeight;
        var stride = width * 4;
        var pixelData = new byte[stride * height];
        source.CopyPixels(pixelData, stride, 0);

        var result = new byte[pixelData.Length];
        var kernelSize = Radius * 2 + 1;
        var kernelWeight = 1.0 / (kernelSize * kernelSize);

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                double sumB = 0, sumG = 0, sumR = 0, sumA = 0;

                // Apply box blur kernel
                for (int ky = -Radius; ky <= Radius; ky++)
                {
                    for (int kx = -Radius; kx <= Radius; kx++)
                    {
                        var px = Math.Clamp(x + kx, 0, width - 1);
                        var py = Math.Clamp(y + ky, 0, height - 1);
                        var offset = py * stride + px * 4;

                        sumB += pixelData[offset];
                        sumG += pixelData[offset + 1];
                        sumR += pixelData[offset + 2];
                        sumA += pixelData[offset + 3];
                    }
                }

                var resultOffset = y * stride + x * 4;
                result[resultOffset] = (byte)(sumB * kernelWeight);
                result[resultOffset + 1] = (byte)(sumG * kernelWeight);
                result[resultOffset + 2] = (byte)(sumR * kernelWeight);
                result[resultOffset + 3] = (byte)(sumA * kernelWeight);
            }
        }

        var bitmap = BitmapSource.Create(
            width, height,
            source.DpiX, source.DpiY,
            source.Format, source.Palette,
            result, stride);

        bitmap.Freeze();
        return bitmap;
    }
}

/// <summary>
/// Sharpen filter using unsharp mask
/// </summary>
public class SharpenFilter : ImageFilter
{
    /// <summary>
    /// Sharpen strength (0.0 to 2.0)
    /// </summary>
    public double Strength { get; set; }

    /// <summary>
    /// Initializes a new instance of the SharpenFilter class
    /// </summary>
    /// <param name="strength">Sharpen strength (0.0 to 2.0)</param>
    public SharpenFilter(double strength)
    {
        Strength = Math.Clamp(strength, 0.0, 2.0);
    }

    /// <inheritdoc />
    public override string Name => $"Sharpen ({Strength:F2})";

    /// <inheritdoc />
    public override BitmapSource Apply(BitmapSource source)
    {
        if (Strength < 0.01)
            return source;

        var width = source.PixelWidth;
        var height = source.PixelHeight;
        var stride = width * 4;
        var pixelData = new byte[stride * height];
        source.CopyPixels(pixelData, stride, 0);

        var result = new byte[pixelData.Length];
        
        // Sharpen kernel
        var kernel = new double[,]
        {
            { 0, -Strength, 0 },
            { -Strength, 1 + 4 * Strength, -Strength },
            { 0, -Strength, 0 }
        };

        for (int y = 1; y < height - 1; y++)
        {
            for (int x = 1; x < width - 1; x++)
            {
                double sumB = 0, sumG = 0, sumR = 0;

                // Apply sharpen kernel
                for (int ky = -1; ky <= 1; ky++)
                {
                    for (int kx = -1; kx <= 1; kx++)
                    {
                        var px = x + kx;
                        var py = y + ky;
                        var offset = py * stride + px * 4;
                        var weight = kernel[ky + 1, kx + 1];

                        sumB += pixelData[offset] * weight;
                        sumG += pixelData[offset + 1] * weight;
                        sumR += pixelData[offset + 2] * weight;
                    }
                }

                var resultOffset = y * stride + x * 4;
                result[resultOffset] = (byte)Math.Clamp(sumB, 0, 255);
                result[resultOffset + 1] = (byte)Math.Clamp(sumG, 0, 255);
                result[resultOffset + 2] = (byte)Math.Clamp(sumR, 0, 255);
                result[resultOffset + 3] = pixelData[resultOffset + 3]; // Keep alpha
            }
        }

        // Copy edges unchanged
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                if (x == 0 || x == width - 1 || y == 0 || y == height - 1)
                {
                    var offset = y * stride + x * 4;
                    result[offset] = pixelData[offset];
                    result[offset + 1] = pixelData[offset + 1];
                    result[offset + 2] = pixelData[offset + 2];
                    result[offset + 3] = pixelData[offset + 3];
                }
            }
        }

        var bitmap = BitmapSource.Create(
            width, height,
            source.DpiX, source.DpiY,
            source.Format, source.Palette,
            result, stride);

        bitmap.Freeze();
        return bitmap;
    }
}