#include "ClientItem.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QtMath>
#include <QIODevice>
#include <QDataStream>

ClientItem::ClientItem()
    : ServerItem()
    , animationPhases(1)
    , xDiv(1)
    , yDiv(1)
    , zDiv(1)
    , animationSpeed(0)
    , animationType(AnimationType::None)
{
    initializeClientDefaults();
}

ClientItem::ClientItem(const ClientItem& other)
    : ServerItem(other)
{
    copyClientData(other);
}

ClientItem& ClientItem::operator=(const ClientItem& other)
{
    if (this != &other) {
        ServerItem::operator=(other);
        copyClientData(other);
    }
    return *this;
}

ClientItem::~ClientItem()
{
}

bool ClientItem::hasSprites() const
{
    return !spriteList.isEmpty();
}

int ClientItem::getSpriteCount() const
{
    return spriteList.size();
}

QPixmap ClientItem::getSprite(int index) const
{
    if (index >= 0 && index < spritePixmaps.size()) {
        return spritePixmaps[index];
    }
    return QPixmap();
}

QByteArray ClientItem::getSpriteData(int index) const
{
    if (index >= 0 && index < spriteList.size()) {
        return spriteList[index];
    }
    return QByteArray();
}

void ClientItem::calculateSpriteSignature()
{
    spriteSignature.clear();
    
    for (const QByteArray& spriteData : spriteList) {
        QVector<double> signature;
        
        // Simple signature calculation using byte values
        // In a real implementation, this would use Fourier transform
        if (!spriteData.isEmpty()) {
            const int signatureSize = 64; // 8x8 signature grid
            signature.reserve(signatureSize);
            
            // Calculate average value for each signature block
            for (int i = 0; i < signatureSize; ++i) {
                int blockStart = (i * spriteData.size()) / signatureSize;
                int blockEnd = ((i + 1) * spriteData.size()) / signatureSize;
                
                double sum = 0.0;
                int count = 0;
                
                for (int j = blockStart; j < blockEnd && j < spriteData.size(); ++j) {
                    sum += static_cast<double>(static_cast<quint8>(spriteData[j]));
                    count++;
                }
                
                if (count > 0) {
                    signature.append(sum / (count * 255.0)); // Normalize to 0-1 range
                } else {
                    signature.append(0.0);
                }
            }
        }
        
        spriteSignature.append(signature);
    }
}

bool ClientItem::compareSignature(const ClientItem& other, double threshold) const
{
    if (spriteSignature.size() != other.spriteSignature.size()) {
        return false;
    }
    
    double similarity = getSignatureSimilarity(other);
    return similarity >= threshold;
}

double ClientItem::getSignatureSimilarity(const ClientItem& other) const
{
    if (spriteSignature.isEmpty() || other.spriteSignature.isEmpty()) {
        return 0.0;
    }
    
    if (spriteSignature.size() != other.spriteSignature.size()) {
        return 0.0;
    }
    
    double totalSimilarity = 0.0;
    int validComparisons = 0;
    
    for (int i = 0; i < spriteSignature.size(); ++i) {
        const QVector<double>& sig1 = spriteSignature[i];
        const QVector<double>& sig2 = other.spriteSignature[i];
        
        if (sig1.size() == sig2.size() && !sig1.isEmpty()) {
            double dotProduct = 0.0;
            double norm1 = 0.0;
            double norm2 = 0.0;
            
            for (int j = 0; j < sig1.size(); ++j) {
                dotProduct += sig1[j] * sig2[j];
                norm1 += sig1[j] * sig1[j];
                norm2 += sig2[j] * sig2[j];
            }
            
            if (norm1 > 0.0 && norm2 > 0.0) {
                double similarity = dotProduct / (qSqrt(norm1) * qSqrt(norm2));
                totalSimilarity += similarity;
                validComparisons++;
            }
        }
    }
    
    return validComparisons > 0 ? totalSimilarity / validComparisons : 0.0;
}

bool ClientItem::validateClientData() const
{
    return getClientValidationErrors().isEmpty();
}

QStringList ClientItem::getClientValidationErrors() const
{
    QStringList errors;
    
    // Inherit base validation errors
    errors.append(getValidationErrors());
    
    if (animationPhases < 1) {
        errors << "Animation phases must be at least 1";
    }
    
    if (xDiv < 1 || yDiv < 1 || zDiv < 1) {
        errors << "Division values must be at least 1";
    }
    
    if (hasSprites()) {
        int expectedSpriteCount = width * height * layers * patternX * patternY * patternZ * frames * animationPhases;
        if (spriteList.size() != expectedSpriteCount) {
            errors << QString("Sprite count mismatch: expected %1, got %2")
                        .arg(expectedSpriteCount).arg(spriteList.size());
        }
        
        // Validate sprite data integrity
        for (int i = 0; i < spriteList.size(); ++i) {
            if (spriteList[i].isEmpty()) {
                errors << QString("Sprite %1 has no data").arg(i);
            }
        }
    }
    
    return errors;
}

void ClientItem::calculateSpriteHash()
{
    if (spriteList.isEmpty()) {
        spriteHash.clear();
        return;
    }
    
    QCryptographicHash hash(QCryptographicHash::Md5);
    
    // Hash all sprite data in order
    for (const QByteArray& spriteData : spriteList) {
        hash.addData(spriteData);
    }
    
    // Include sprite metadata in hash
    QByteArray metadata;
    QDataStream stream(&metadata, QIODevice::WriteOnly);
    stream << width << height << layers << patternX << patternY << patternZ 
           << frames << animationPhases << xDiv << yDiv << zDiv;
    hash.addData(metadata);
    
    spriteHash = hash.result();
}

bool ClientItem::verifySpriteHash() const
{
    if (spriteList.isEmpty()) {
        return spriteHash.isEmpty();
    }
    
    ClientItem temp = *this;
    temp.calculateSpriteHash();
    return temp.spriteHash == spriteHash;
}

void ClientItem::copyClientData(const ClientItem& other)
{
    spriteList = other.spriteList;
    spriteSignature = other.spriteSignature;
    spritePixmaps = other.spritePixmaps;
    animationPhases = other.animationPhases;
    xDiv = other.xDiv;
    yDiv = other.yDiv;
    zDiv = other.zDiv;
    animationSpeed = other.animationSpeed;
    animationType = other.animationType;
}

void ClientItem::initializeClientDefaults()
{
    // Set default client-specific values
    hasClientData = true;
}