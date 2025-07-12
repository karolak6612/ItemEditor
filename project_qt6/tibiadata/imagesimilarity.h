#ifndef IMAGESIMILARITY_H
#define IMAGESIMILARITY_H

#include <QtGlobal>
#include <QVector>
#include <QImage> // For processing images
#include <QVariantMap> // For returning signature (double[,] equivalent)

namespace TibiaData {
namespace ImageSimilarity {

// Based on C# PluginInterface/ImageSimilarity/Complex.cs
struct Complex
{
    double real;
    double imag;

    Complex(double r = 0.0, double i = 0.0) : real(r), imag(i) {}

    Complex operator+(const Complex& other) const {
        return Complex(real + other.real, imag + other.imag);
    }
    Complex operator-(const Complex& other) const {
        return Complex(real - other.real, imag - other.imag);
    }
    Complex operator*(const Complex& other) const {
        return Complex(real * other.real - imag * other.imag,
                       real * other.imag + imag * other.real);
    }
    // No division needed for current C# FFT implementation it seems
    // Complex operator/(const Complex& other) const;

    double magnitude() const {
        return qSqrt(real * real + imag * imag);
    }
    // Other complex operations if needed by FFT algorithm
};

// Based on C# PluginInterface/ImageSimilarity/Fourier.cs
namespace Fourier {
    // 1D FFT and IFFT (Inverse FFT) - not directly used by ItemEditor's SpriteSignature but good to have if porting full Fourier.cs
    // QVector<Complex> FFT(const QVector<Complex>& x);
    // QVector<Complex> IFFT(const QVector<Complex>& x);

    // 2D FFT for grayscale image (or one channel)
    // Returns a 2D array of Complex numbers (represented as QVector<QVector<Complex>>)
    QVector<QVector<Complex>> FFT2D(const QVector<QVector<double>>& input, bool shift);

    // Helper to convert QImage to a 2D QVector of doubles (grayscale intensity)
    QVector<QVector<double>> ImageToGrayscaleDoubles(const QImage& image);

    // Helper to get magnitude spectrum from complex FFT result (for visualization or further processing)
    QVector<QVector<double>> GetMagnitudeSpectrum(const QVector<QVector<Complex>>& fftResult);

    // C# ItemEditor uses fft2dRGB which processes each RGB channel separately
    // This function is a direct conceptual port.
    // Returns 3 (R,G,B) 2D arrays of Complex numbers.
    // For simplicity, we might just need a grayscale FFT for signature.
    // The C# ImageUtils.CalculateEuclideanDistance takes a Bitmap that seems to be fft2dRGB(canvas, false)
    // And that bitmap is then processed. Let's clarify what CalculateEuclideanDistance expects.
    // The C# fft2dRGB returns a Bitmap where R,G,B channels are magnitudes of FFT of original R,G,B.
    // So, we need a version that returns QImage.
    QImage FFT2D_RGB(const QImage& inputImage, bool shift);

} // namespace Fourier


// Based on C# PluginInterface/ImageSimilarity/ImageUtils.cs
namespace Utils {

    // Calculates Euclidean distance signature from an image (typically an FFT magnitude image)
    // blocksize: C# uses 1. This means it divides the image into 4x4 blocks (16 total for a 32x32 fft image if width/height are 4*blocksize)
    // and calculates average intensity per block.
    // The C# `ImageUtils.CalculateEuclideanDistance` actually takes a `Bitmap` (which is an FFT magnitude image)
    // and `blocksize` (default 1). It divides the image into `(width/blocksize_pixels) x (height/blocksize_pixels)` regions.
    // For a 32x32 image and blocksize=1 (meaning 4x4 regions resulting in 16 values), each region is 8x8 pixels.
    // It calculates the average intensity of each of these 8x8 pixel regions.
    // Returns a map representing a 2D array of doubles (the signature).
    // Map keys could be "row_X_col_Y" or similar, or a nested map.
    // C# ClientItem.SpriteSignature is double[,]
    // Let's return QVariantMap for flexibility, e.g. QMap<int, QMap<int, double>> -> QVariantMap
    QVariantMap CalculateEuclideanDistanceSignature(const QImage& fftMagnitudeImage, int regions = 4); // regions = 4 means 4x4 grid

    // Compares two signatures (QVariantMap representing double[,])
    double CompareSignatures(const QVariantMap& sig1, const QVariantMap& sig2);

} // namespace Utils

} // namespace ImageSimilarity
} // namespace TibiaData

#endif // IMAGESIMILARITY_H
