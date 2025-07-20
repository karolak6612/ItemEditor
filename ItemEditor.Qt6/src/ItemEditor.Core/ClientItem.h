#pragma once

#include "ServerItem.h"
#include <QList>
#include <QVector>
#include <QPixmap>

/**
 * @brief Client item data structure
 * 
 * Represents a client item with sprite data and additional client-specific properties.
 * Inherits from ServerItem and adds client data loading capabilities.
 */
class ClientItem : public ServerItem
{
public:
    ClientItem();
    ClientItem(const ClientItem& other);
    ClientItem& operator=(const ClientItem& other);
    ~ClientItem() override;

    // Sprite data
    QList<QByteArray> spriteList;
    QVector<QVector<double>> spriteSignature;
    QList<QPixmap> spritePixmaps;
    
    // Client-specific properties
    quint8 animationPhases;
    quint8 xDiv;
    quint8 yDiv;
    quint8 zDiv;
    quint16 animationSpeed;
    AnimationType animationType;
    
    // Sprite processing
    bool hasSprites() const;
    int getSpriteCount() const;
    QPixmap getSprite(int index) const;
    QByteArray getSpriteData(int index) const;
    
    // Signature calculation
    void calculateSpriteSignature();
    bool compareSignature(const ClientItem& other, double threshold = 0.95) const;
    double getSignatureSimilarity(const ClientItem& other) const;
    
    // Client data validation
    bool validateClientData() const;
    QStringList getClientValidationErrors() const;
    
    // Sprite hash calculation
    void calculateSpriteHash();
    bool verifySpriteHash() const;

private:
    void copyClientData(const ClientItem& other);
    void initializeClientDefaults();
};