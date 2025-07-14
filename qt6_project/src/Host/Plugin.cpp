/**
 * Item Editor Qt6 - Plugin Wrapper Implementation
 * Exact mirror of Legacy_App/csharp/Source/Host/Plugin.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "Plugin.h"
#include <QDebug>
#include <QFileInfo>

namespace ItemEditor {

Plugin::Plugin(QObject* parent)
    : QObject(parent)
    , m_instance(nullptr)
    , m_loader(nullptr)
{
    // Constructor - initialize members to null like C# version
}

Plugin::~Plugin()
{
    // Ensure proper cleanup
    unload();
}

bool Plugin::isLoaded() const
{
    return m_instance != nullptr && 
           m_loader != nullptr && 
           m_loader->isLoaded();
}

QString Plugin::name() const
{
    if (m_instance) {
        return m_instance->name();
    }
    
    // Fallback: extract name from assembly path
    if (!m_assemblyPath.isEmpty()) {
        QFileInfo fileInfo(m_assemblyPath);
        return fileInfo.baseName();
    }
    
    return QString("Unknown Plugin");
}

void Plugin::unload()
{
    // Clean up plugin instance
    if (m_instance) {
        delete m_instance;
        m_instance = nullptr;
    }
    
    // Unload and clean up the plugin loader
    if (m_loader) {
        if (m_loader->isLoaded()) {
            m_loader->unload();
        }
        m_loader->deleteLater();
        m_loader = nullptr;
    }
    
    qDebug() << "Plugin unloaded:" << m_assemblyPath;
}

const SupportedClient* Plugin::getClientBySignatures(quint32 datSignature, quint32 sprSignature) const
{
    if (!m_instance) {
        return nullptr;
    }
    
    // Get the supported client from the plugin instance
    SupportedClient client = m_instance->getClientBySignatures(datSignature, sprSignature);
    
    // Return a pointer to the client if valid (version != 0 indicates valid client)
    if (client.version() != 0) {
        // Note: This returns a temporary object pointer, which is not ideal
        // In a real implementation, you might want to store this in the plugin
        // or return by value instead
        static SupportedClient s_tempClient;
        s_tempClient = client;
        return &s_tempClient;
    }
    
    return nullptr;
}

} // namespace ItemEditor