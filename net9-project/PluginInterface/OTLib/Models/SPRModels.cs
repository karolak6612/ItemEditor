namespace PluginInterface.OTLib.FileFormats;

/// <summary>
/// SPR file header structure
/// </summary>
public class SPRHeader
{
    /// <summary>
    /// Header size in bytes
    /// </summary>
    public const int HeaderSize = 8;

    /// <summary>
    /// File signature
    /// </summary>
    public uint Signature { get; set; }

    /// <summary>
    /// Number of sprites in the file
    /// </summary>
    public ushort SpriteCount { get; set; }

    /// <summary>
    /// SPR format version
    /// </summary>
    public ushort Version { get; set; }
}

/// <summary>
/// SPR sprite representation
/// </summary>
public class SPRSprite
{
    /// <summary>
    /// Sprite ID
    /// </summary>
    public ushort Id { get; set; }

    /// <summary>
    /// Sprite width in pixels
    /// </summary>
    public int Width { get; set; }

    /// <summary>
    /// Sprite height in pixels
    /// </summary>
    public int Height { get; set; }

    /// <summary>
    /// Raw pixel data in RGBA format
    /// </summary>
    public byte[] PixelData { get; set; } = Array.Empty<byte>();

    /// <summary>
    /// Sprite hash for comparison
    /// </summary>
    public byte[]? Hash { get; set; }

    /// <summary>
    /// Whether the sprite is empty (transparent)
    /// </summary>
    public bool IsEmpty => PixelData.All(b => b == 0);

    /// <summary>
    /// Get pixel color at specific coordinates
    /// </summary>
    /// <param name="x">X coordinate</param>
    /// <param name="y">Y coordinate</param>
    /// <returns>RGBA color values</returns>
    public (byte R, byte G, byte B, byte A) GetPixel(int x, int y)
    {
        if (x < 0 || x >= Width || y < 0 || y >= Height)
            return (0, 0, 0, 0);

        var offset = (y * Width + x) * 4;
        return (
            PixelData[offset + 2], // R
            PixelData[offset + 1], // G
            PixelData[offset],     // B
            PixelData[offset + 3]  // A
        );
    }

    /// <summary>
    /// Set pixel color at specific coordinates
    /// </summary>
    /// <param name="x">X coordinate</param>
    /// <param name="y">Y coordinate</param>
    /// <param name="r">Red component</param>
    /// <param name="g">Green component</param>
    /// <param name="b">Blue component</param>
    /// <param name="a">Alpha component</param>
    public void SetPixel(int x, int y, byte r, byte g, byte b, byte a = 255)
    {
        if (x < 0 || x >= Width || y < 0 || y >= Height)
            return;

        var offset = (y * Width + x) * 4;
        PixelData[offset] = b;     // B
        PixelData[offset + 1] = g; // G
        PixelData[offset + 2] = r; // R
        PixelData[offset + 3] = a; // A
    }

    /// <summary>
    /// Calculate MD5 hash of the sprite data
    /// </summary>
    /// <returns>MD5 hash bytes</returns>
    public byte[] CalculateHash()
    {
        using var md5 = System.Security.Cryptography.MD5.Create();
        Hash = md5.ComputeHash(PixelData);
        return Hash;
    }

    /// <summary>
    /// Compare this sprite with another sprite
    /// </summary>
    /// <param name="other">Other sprite to compare</param>
    /// <returns>True if sprites are identical</returns>
    public bool IsIdenticalTo(SPRSprite other)
    {
        if (other == null || Width != other.Width || Height != other.Height)
            return false;

        if (Hash == null) CalculateHash();
        if (other.Hash == null) other.CalculateHash();

        return Hash != null && other.Hash != null && Hash.SequenceEqual(other.Hash);
    }

    /// <summary>
    /// Create a copy of this sprite
    /// </summary>
    /// <returns>Sprite copy</returns>
    public SPRSprite Clone()
    {
        return new SPRSprite
        {
            Id = Id,
            Width = Width,
            Height = Height,
            PixelData = (byte[])PixelData.Clone(),
            Hash = Hash != null ? (byte[])Hash.Clone() : null
        };
    }

    /// <summary>
    /// Resize the sprite to new dimensions
    /// </summary>
    /// <param name="newWidth">New width</param>
    /// <param name="newHeight">New height</param>
    /// <returns>Resized sprite</returns>
    public SPRSprite Resize(int newWidth, int newHeight)
    {
        if (newWidth <= 0 || newHeight <= 0)
            throw new ArgumentException("Width and height must be positive");

        var resized = new SPRSprite
        {
            Id = Id,
            Width = newWidth,
            Height = newHeight,
            PixelData = new byte[newWidth * newHeight * 4]
        };

        // Simple nearest-neighbor scaling
        for (int y = 0; y < newHeight; y++)
        {
            for (int x = 0; x < newWidth; x++)
            {
                var srcX = (int)((double)x / newWidth * Width);
                var srcY = (int)((double)y / newHeight * Height);
                
                var (r, g, b, a) = GetPixel(srcX, srcY);
                resized.SetPixel(x, y, r, g, b, a);
            }
        }

        return resized;
    }

    /// <summary>
    /// Apply alpha blending with another sprite
    /// </summary>
    /// <param name="overlay">Sprite to blend on top</param>
    /// <param name="offsetX">X offset for overlay</param>
    /// <param name="offsetY">Y offset for overlay</param>
    public void BlendWith(SPRSprite overlay, int offsetX = 0, int offsetY = 0)
    {
        if (overlay == null) return;

        for (int y = 0; y < overlay.Height; y++)
        {
            for (int x = 0; x < overlay.Width; x++)
            {
                var targetX = x + offsetX;
                var targetY = y + offsetY;

                if (targetX < 0 || targetX >= Width || targetY < 0 || targetY >= Height)
                    continue;

                var (overlayR, overlayG, overlayB, overlayA) = overlay.GetPixel(x, y);
                
                if (overlayA == 0) continue; // Fully transparent

                var (baseR, baseG, baseB, baseA) = GetPixel(targetX, targetY);

                // Alpha blending
                var alpha = overlayA / 255.0;
                var invAlpha = 1.0 - alpha;

                var blendedR = (byte)(overlayR * alpha + baseR * invAlpha);
                var blendedG = (byte)(overlayG * alpha + baseG * invAlpha);
                var blendedB = (byte)(overlayB * alpha + baseB * invAlpha);
                var blendedA = (byte)Math.Max(overlayA, baseA);

                SetPixel(targetX, targetY, blendedR, blendedG, blendedB, blendedA);
            }
        }

        // Invalidate hash after modification
        Hash = null;
    }
}