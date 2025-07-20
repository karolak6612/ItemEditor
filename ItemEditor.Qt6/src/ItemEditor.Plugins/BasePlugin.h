#pragma once

#include "IPlugin.h"

/**
 * @brief Base plugin implementation
 * 
 * Placeholder implementation - will be fully implemented in later tasks
 */
class BasePlugin : public IPlugin
{
    Q_OBJECT

public:
    explicit BasePlugin(QObject* parent = nullptr);
    ~BasePlugin() override;

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

protected:
    QString m_name;
    QString m_version;
    QStringList m_supportedVersions;
    bool m_isLoaded;
};