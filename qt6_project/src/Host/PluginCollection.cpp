/**
 * Item Editor Qt6 - Plugin Collection Implementation
 * Exact mirror of Legacy_App/csharp/Source/Host/PluginCollection.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "PluginCollection.h"
#include <QDebug>

namespace ItemEditor {

PluginCollection::PluginCollection(QObject* parent)
    : QObject(parent)
{
    // Constructor - initialize empty collection
}

PluginCollection::~PluginCollection()
{
    // Clean up all plugins
    clear();
}

void PluginCollection::add(Plugin* plugin)
{
    // Exact mirror of C# Add() method
    if (plugin && !m_plugins.contains(plugin)) {
        m_plugins.append(plugin);
        emit pluginAdded(plugin);
    }
}

void PluginCollection::remove(Plugin* plugin)
{
    // Exact mirror of C# Remove() method
    if (plugin && m_plugins.contains(plugin)) {
        m_plugins.removeOne(plugin);
        emit pluginRemoved(plugin);
    }
}

void PluginCollection::clear()
{
    // Clear all plugins - exact mirror of C# Clear() method
    m_plugins.clear();
    emit collectionCleared();
}

Plugin* PluginCollection::find(const QString& pluginNameOrPath) const
{
    // Exact mirror of C# Find(string pluginNameOrPath) method
    foreach (Plugin* plugin, m_plugins) {
        if (!plugin || !plugin->instance()) {
            continue;
        }

        // Check supported clients descriptions
        QList<SupportedClient> supportedClients = plugin->instance()->supportedClients();
        foreach (const SupportedClient& client, supportedClients) {
            if (client.description() == pluginNameOrPath) {
                return plugin;
            }
        }

        // Check assembly path
        if (plugin->assemblyPath() == pluginNameOrPath) {
            return plugin;
        }
    }

    return nullptr;
}Plugin* PluginCollection::find(quint32 otbVersion) const
{
    // Exact mirror of C# Find(uint otbVersion) method
    foreach (Plugin* plugin, m_plugins) {
        if (!plugin || !plugin->instance()) {
            continue;
        }

        QList<SupportedClient> supportedClients = plugin->instance()->supportedClients();
        foreach (const SupportedClient& client, supportedClients) {
            if (client.otbVersion() == otbVersion) {
                return plugin;
            }
        }
    }

    return nullptr;
}

Plugin* PluginCollection::find(quint32 datSignature, quint32 sprSignature) const
{
    // Exact mirror of C# Find(uint datSignature, uint sprSignature) method
    foreach (Plugin* plugin, m_plugins) {
        if (!plugin || !plugin->instance()) {
            continue;
        }

        QList<SupportedClient> supportedClients = plugin->instance()->supportedClients();
        foreach (const SupportedClient& client, supportedClients) {
            if (client.datSignature() == datSignature && 
                client.sprSignature() == sprSignature) {
                return plugin;
            }
        }
    }

    return nullptr;
}

} // namespace ItemEditor