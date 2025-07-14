/**
 * Item Editor Qt6 - Supported Client Implementation
 * Defines client version support for plugins
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "SupportedClient.h"

namespace ItemEditor {

SupportedClient::SupportedClient()
    : m_name("Unknown")
    , m_version("0.0.0")
    , m_otbVersion(0)
    , m_datSignature(0)
    , m_sprSignature(0)
{
}

SupportedClient::SupportedClient(const QString& name, const QString& version, int otbVersion)
    : m_name(name)
    , m_version(version)
    , m_otbVersion(otbVersion)
    , m_datSignature(0)
    , m_sprSignature(0)
{
}

SupportedClient::SupportedClient(quint32 version, const QString& description, quint32 otbVersion, quint32 datSignature, quint32 sprSignature)
    : m_name("Client")
    , m_version(QString::number(version))
    , m_otbVersion(static_cast<int>(otbVersion))
    , m_datSignature(datSignature)
    , m_sprSignature(sprSignature)
{
    if (!description.isEmpty()) {
        m_name = description;
    }
}

QString SupportedClient::getDisplayName() const
{
    if (m_name.isEmpty() && m_version.isEmpty()) {
        return QString("Unknown Client (OTB %1)").arg(m_otbVersion);
    } else if (m_version.isEmpty()) {
        return QString("%1 (OTB %2)").arg(m_name).arg(m_otbVersion);
    } else {
        return QString("%1 %2 (OTB %3)").arg(m_name).arg(m_version).arg(m_otbVersion);
    }
}

} // namespace ItemEditor