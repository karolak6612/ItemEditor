#pragma once

#include "BasePlugin.h"
#include "DatParserV10.h"
#include "SprParserV10.h"
#include <QHash>
#include <QMutex>

/**
 * @brief Plugin Three - Client versions 10.00-10.77
 * 
 * Handles client data loading and processing for Tibia client versions 10.00-10.77.
 * Implements the IPlugin interface with full DAT/SPR parsing capabilities,
 * sprite hash calculation, and signature generation for item comparison.
 */
class PluginThree : public BasePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.itemeditor.IPlugin/1.0" FILE "pluginthree.json")
    Q_INTERFACES(IPlugin)

public:
    explicit PluginThree(QObject* parent = nullptr);
    ~PluginThree() override;

    // IPlugin interface implementation
    bool initialize() override;
    QString name() const override;
    QString version() const override;
    QStringList supportedVersions() const override;
    bool loadClient(const QString& datPath, const QString& sprPath) override;
    QByteArray getClientData(quint16 clientId) override;
    QByteArray getSpriteHash(quint16 clientId) override;
    QByteArray getSpriteSignature(quint16 clientId) override;
    bool isClientLoaded() const override;
    QString getClientVersion() const override;
    void cleanup() override;

private:
    // Parser instances
    DatParserV10* m_datParser;
    SprParserV10* m_sprParser;
    
    // Client data cache
    QHash<quint16, QByteArray> m_clientDataCache;
    QHash<quint16, QByteArray> m_spriteHashCache;
    QHash<quint16, QByteArray> m_spriteSignatureCache;
    
    // Current client version loaded
    QString m_currentClientVersion;
    
    // Thread safety
    mutable QMutex m_mutex;
    
    // Helper methods
    void clearCaches();
    QByteArray calculateSpriteHash(const DatData& datData);
    QByteArray calculateSpriteSignature(const DatData& datData);
    
    // Fourier transform and signature calculation methods
    QByteArray applyFFT2DRGB(const QByteArray& rgbData, int width, int height, bool reorder);
    QByteArray calculateEuclideanDistanceSignature(const QByteArray& fftData, int width, int height, int blockSize);
    double compareSpriteSignatures(const QByteArray& signature1, const QByteArray& signature2);
};