#pragma once

#include "BasePlugin.h"
#include <QMutex>
#include <QHash>
#include <QByteArray>

// Forward declarations
class DatParserV9;
class SprParserV9;
struct DatData;

/**
 * @brief Plugin Two - Client versions 8.60-9.86
 * 
 * Handles client data loading and processing for Tibia client versions 8.60-9.86.
 * Implements native C++ DAT/SPR file parsing with identical functionality to legacy system.
 */
class PluginTwo : public BasePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.itemeditor.IPlugin/1.0" FILE "plugintwo.json")
    Q_INTERFACES(IPlugin)

public:
    explicit PluginTwo(QObject* parent = nullptr);
    ~PluginTwo() override;

    // IPlugin interface
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
    // Helper methods
    void clearCaches();
    QByteArray calculateSpriteHash(const DatData& datData);
    QByteArray calculateSpriteSignature(const DatData& datData);
    QByteArray applyFFT2DRGB(const QByteArray& rgbData, int width, int height, bool reorder);
    QByteArray calculateEuclideanDistanceSignature(const QByteArray& fftData, int width, int height, int blockSize);
    double compareSpriteSignatures(const QByteArray& signature1, const QByteArray& signature2);

    // Member variables
    DatParserV9* m_datParser;
    SprParserV9* m_sprParser;
    mutable QMutex m_mutex;
    QString m_currentClientVersion;
    
    // Caches for performance
    QHash<quint16, QByteArray> m_clientDataCache;
    QHash<quint16, QByteArray> m_spriteHashCache;
    QHash<quint16, QByteArray> m_spriteSignatureCache;
};