/**
 * Item Editor Qt6 - Plugin Collection
 * Exact mirror of Legacy_App/csharp/Source/Host/PluginCollection.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_PLUGINCOLLECTION_H
#define ITEMEDITOR_PLUGINCOLLECTION_H

#include <QObject>
#include <QList>
#include <QString>
#include "Plugin.h"

namespace ItemEditor {

/**
 * Plugin Collection Class
 * Exact mirror of C# PluginCollection : CollectionBase
 * Collection of Plugin objects with search functionality
 */
class PluginCollection : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit PluginCollection(QObject* parent = nullptr);
    
    /**
     * Destructor
     */
    virtual ~PluginCollection();

    // Collection interface - exact mirror of C# CollectionBase functionality
    
    /**
     * Add a plugin
     * Exact mirror of C# Add() method
     */
    void add(Plugin* plugin);
    
    /**
     * Remove a plugin
     * Exact mirror of C# Remove() method
     */
    void remove(Plugin* plugin);
    
    /**
     * Clear all plugins
     * Exact mirror of C# Clear() method
     */
    void clear();
    
    /**
     * Get count of plugins
     */
    int count() const { return m_plugins.count(); }
    
    /**
     * Get size of plugins (alias for count for compatibility)
     */
    int size() const { return m_plugins.size(); }
    
    /**
     * Get plugin at index
     */
    Plugin* at(int index) const { return m_plugins.at(index); }
    
    /**
     * Check if collection is empty
     */
    bool isEmpty() const { return m_plugins.isEmpty(); }

    // Search methods - exact mirror of C# Find() overloads
    
    /**
     * Search for a plugin by name or path
     * Exact mirror of C# Find(string pluginNameOrPath) method
     */
    Plugin* find(const QString& pluginNameOrPath) const;
    
    /**
     * Search for a plugin by OTB version compatibility
     * Exact mirror of C# Find(uint otbVersion) method
     */
    Plugin* find(quint32 otbVersion) const;
    
    /**
     * Search for a plugin by file signatures
     * Exact mirror of C# Find(uint datSignature, uint sprSignature) method
     */
    Plugin* find(quint32 datSignature, quint32 sprSignature) const;

    // Qt-style iteration support
    typedef QList<Plugin*>::iterator iterator;
    typedef QList<Plugin*>::const_iterator const_iterator;
    
    iterator begin() { return m_plugins.begin(); }
    iterator end() { return m_plugins.end(); }
    const_iterator begin() const { return m_plugins.begin(); }
    const_iterator end() const { return m_plugins.end(); }
    const_iterator constBegin() const { return m_plugins.constBegin(); }
    const_iterator constEnd() const { return m_plugins.constEnd(); }

signals:
    void pluginAdded(Plugin* plugin);
    void pluginRemoved(Plugin* plugin);
    void collectionCleared();

private:
    QList<Plugin*> m_plugins;  // Qt6 equivalent of C# CollectionBase.List
};

} // namespace ItemEditor

#endif // ITEMEDITOR_PLUGINCOLLECTION_H