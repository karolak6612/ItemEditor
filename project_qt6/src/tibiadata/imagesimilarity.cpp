#include "imagesimilarity.h"
#include <QDebug>
#include <cmath>
#include <algorithm>
#include <QtMath> // For qCos, qSin, M_PI

namespace TibiaData {
namespace ImageSimilarity {

// --- Complex struct methods if any non-inline ---
// (All are inline in header)


// --- Fourier namespace ---
namespace Fourier {

// Recursive 1D FFT implementation based on Cooley-Tukey algorithm, ported from C# Fourier.cs
QVector<Complex> FFT(const QVector<Complex>& x)
{
    int N = x.size();
    if (N <= 1) {
        return x;
    }

    // Recurse on even and odd parts
    QVector<Complex> even(N / 2);
    QVector<Complex> odd(N / 2);
    for (int i = 0; i < N / 2; ++i) {
        even[i] = x[i * 2];
        odd[i] = x[i * 2 + 1];
    }
    QVector<Complex> q = FFT(even);
    QVector<Complex> r = FFT(odd);

    // Combine
    QVector<Complex> y(N);
    for (int k = 0; k < N / 2; ++k) {
        double kth = -2 * k * M_PI / N;
        Complex wk(qCos(kth), qSin(kth));
        y[k] = q[k] + wk * r[k];
        y[k + N / 2] = q[k] - wk * r[k];
    }
    return y;
}

// Helper to convert QImage to a 2D QVector of doubles (grayscale intensity)
QVector<QVector<double>> ImageToGrayscaleDoubles(const QImage& image) {
    QVector<QVector<double>> result(image.height(), QVector<double>(image.width()));
    if (image.isNull()) return result;

    QImage grayImage = image.convertToFormat(QImage::Format_Grayscale8);
    for (int y = 0; y < grayImage.height(); ++y) {
        const uchar* line = grayImage.constScanLine(y);
        for (int x = 0; x < grayImage.width(); ++x) {
            result[y][x] = static_cast<double>(line[x]);
        }
    }
    return result;
}

// 2D FFT on a single channel (grayscale)
QVector<QVector<Complex>> FFT2D(const QVector<QVector<double>>& input, bool shift) {
    int height = input.size();
    if (height == 0) return {};
    int width = input[0].size();
    if (width == 0) return {};

    QVector<QVector<Complex>> output(height, QVector<Complex>(width));

    // FFT on rows
    for (int y = 0; y < height; ++y) {
        QVector<Complex> row(width);
        for (int x = 0; x < width; ++x) {
            row[x] = Complex(input[y][x], 0);
        }
        output[y] = FFT(row);
    }

    // FFT on columns
    for (int x = 0; x < width; ++x) {
        QVector<Complex> col(height);
        for (int y = 0; y < height; ++y) {
            col[y] = output[y][x];
        }
        QVector<Complex> fftCol = FFT(col);
        for (int y = 0; y < height; ++y) {
            output[y][x] = fftCol[y];
        }
    }

    if (shift) {
        // Perform FFT shift (swap quadrants)
        QVector<QVector<Complex>> shiftedOutput(height, QVector<Complex>(width));
        int h2 = height / 2;
        int w2 = width / 2;
        for(int y=0; y<h2; ++y) {
            for(int x=0; x<w2; ++x) {
                shiftedOutput[y][x] = output[y+h2][x+w2];
                shiftedOutput[y+h2][x+w2] = output[y][x];
                shiftedOutput[y][x+w2] = output[y+h2][x];
                shiftedOutput[y+h2][x] = output[y][x+w2];
            }
        }
        return shiftedOutput;
    }

    return output;
}

QVector<QVector<double>> GetMagnitudeSpectrum(const QVector<QVector<Complex>>& fftResult) {
    QVector<QVector<double>> magnitude;
    if (fftResult.isEmpty() || fftResult[0].isEmpty()) return magnitude;

    int height = fftResult.size();
    int width = fftResult[0].size();
    magnitude.resize(height);
    for (int y = 0; y < height; ++y) {
        magnitude[y].resize(width);
        for (int x = 0; x < width; ++x) {
            magnitude[y][x] = std::log10(1.0 + fftResult[y][x].magnitude());
        }
    }
    return magnitude;
}


QImage FFT2D_RGB(const QImage& inputImage, bool shift) {
    if (inputImage.isNull()) return QImage();

    int width = inputImage.width();
    int height = inputImage.height();
    QImage image = inputImage.convertToFormat(QImage::Format_RGB32);

    QVector<QVector<double>> r(height, QVector<double>(width));
    QVector<QVector<double>> g(height, QVector<double>(width));
    QVector<QVector<double>> b(height, QVector<double>(width));

    for (int y = 0; y < height; ++y) {
        const QRgb* line = reinterpret_cast<const QRgb*>(image.constScanLine(y));
        for (int x = 0; x < width; ++x) {
            r[y][x] = qRed(line[x]);
            g[y][x] = qGreen(line[x]);
            b[y][x] = qBlue(line[x]);
        }
    }

    QVector<QVector<Complex>> fft_r = FFT2D(r, shift);
    QVector<QVector<Complex>> fft_g = FFT2D(g, shift);
    QVector<QVector<Complex>> fft_b = FFT2D(b, shift);

    QImage resultImage(width, height, QImage::Format_RGB32);
    double max_r = 0, max_g = 0, max_b = 0;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double mag_r = fft_r[y][x].magnitude();
            double mag_g = fft_g[y][x].magnitude();
            double mag_b = fft_b[y][x].magnitude();
            if (mag_r > max_r) max_r = mag_r;
            if (mag_g > max_g) max_g = mag_g;
            if (mag_b > max_b) max_b = mag_b;
        }
    }

    for (int y = 0; y < height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(resultImage.scanLine(y));
        for (int x = 0; x < width; ++x) {
            int r_val = (max_r > 0) ? static_cast<int>(255.0 * (fft_r[y][x].magnitude() / max_r)) : 0;
            int g_val = (max_g > 0) ? static_cast<int>(255.0 * (fft_g[y][x].magnitude() / max_g)) : 0;
            int b_val = (max_b > 0) ? static_cast<int>(255.0 * (fft_b[y][x].magnitude() / max_b)) : 0;
            line[x] = qRgb(r_val, g_val, b_val);
        }
    }

    return resultImage;
}


} // namespace Fourier


// --- ImageUtils namespace ---
namespace Utils {

QVariantMap CalculateEuclideanDistanceSignature(const QImage& fftMagnitudeImage, int regions) {
    QVariantMap signature;
    if (fftMagnitudeImage.isNull() || regions <= 0) return signature;

    int blockWidth = fftMagnitudeImage.width() / regions;
    int blockHeight = fftMagnitudeImage.height() / regions;

    if (blockWidth == 0 || blockHeight == 0) {
        qWarning() << "ImageUtils::CalculateEuclideanDistanceSignature: Image too small for specified regions.";
        return signature;
    }

    QImage grayImage = fftMagnitudeImage.convertToFormat(QImage::Format_Grayscale8);

    for (int r = 0; r < regions; ++r) {
        QVariantMap rowMap;
        for (int c = 0; c < regions; ++c) {
            double sum = 0;
            int count = 0;
            int startX = c * blockWidth;
            int startY = r * blockHeight;

            for (int y = startY; y < startY + blockHeight; ++y) {
                if(y >= grayImage.height()) continue;
                const uchar* line = grayImage.constScanLine(y);
                for (int x = startX; x < startX + blockWidth; ++x) {
                     if(x >= grayImage.width()) continue;
                    sum += line[x];
                    count++;
                }
            }
            double avgIntensity = (count > 0) ? (sum / count) : 0.0;
            rowMap.insert(QString("col_%1").arg(c), avgIntensity);
        }
        signature.insert(QString("row_%1").arg(r), rowMap);
    }
    return signature;
}

double CompareSignatures(const QVariantMap& sig1, const QVariantMap& sig2) {
    double sumOfSquares = 0.0;
    int N = 0;

    if (sig1.keys().size() != sig2.keys().size() || sig1.keys().isEmpty()) {
        return 1.0e6;
    }

    for (const QString& rowKey : sig1.keys()) {
        if (!sig2.contains(rowKey)) return 1.0e6;

        QVariantMap row1 = sig1.value(rowKey).toMap();
        QVariantMap row2 = sig2.value(rowKey).toMap();

        if (row1.keys().size() != row2.keys().size() || row1.keys().isEmpty()) return 1.0e6;

        for (const QString& colKey : row1.keys()) {
            if (!row2.contains(colKey)) return 1.0e6;

            double val1 = row1.value(colKey).toDouble();
            double val2 = row2.value(colKey).toDouble();
            sumOfSquares += std::pow(val1 - val2, 2.0);
            N++;
        }
    }

    if (N == 0) return 1.0e6;

    return std::sqrt(sumOfSquares);
}


} // namespace Utils
} // namespace ImageSimilarity
} // namespace TibiaData
