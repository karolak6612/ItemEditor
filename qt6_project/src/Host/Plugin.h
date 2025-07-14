/**
 * Item Editor Qt6 - Plugin Wrapper
 * Exact mirror of Legacy_App/csharp/Source/Host/Plugin.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_PLUGIN_H
#define ITEMEDITOR_PLUGIN_H

#include <QObject>
#include <QString>
#include <QPluginLoader>
#include "../PluginInterface/IPlugin.h"

namespace ItemEditor {

// Forward declaration
class SupportedClient;

/**
 * Plugin Wrapper Class
 * Exact mirror of C# Plugin class
 * Data class for plugins that holds an instance of the loaded plugin
 * and the plugin's assembly path, plus Qt6-specific QPluginLoader
 */
class Plugin : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Plugin(QObject* parent = nullptr);
    
    /**
     * Destructor - ensures proper cleanup
     */
    virtual ~Plugin();

    // Properties - exact mirror of C# properties plus Qt6-specific additions
    
    /**
     * Plugin instance
     * Exact mirror of C# Instance property
     */
    IPlugin* instance() const { return m_instance; }
    void setInstance(IPlugin* instance) { m_instance = instance; }
    
    /**
     * Assembly path
     * Exact mirror of C# AssemblyPath property
     */
    QString assemblyPath() const { return m_assemblyPath; }
    void setAssemblyPath(const QString& path) { m_assemblyPath = path; }
    
    /**
     * Qt6-specific: QPluginLoader for proper plugin lifecycle management
     * This replaces the C# Assembly reference
     */
    QPluginLoader* loader() const { return m_loader; }
    void setLoader(QPluginLoader* loader) { m_loader = loader; }
    
    /**
     * Check if plugin is loaded and valid
     */
    bool isLoaded() const;
    
    /**
     * Unload the plugin
     */
    void unload();
    
    /**
     * Get plugin name
     * Exact mirror of C# Name property
     */
    QString name() const;
    
    /**
     * Get client by signatures - convenience method
     */
    const SupportedClient* getClientBySignatures(quint32 datSignature, quint32 sprSignature) const;

private:
    IPlugin* m_instance;
    QString m_assemblyPath;
    QPluginLoader* m_loader;  // Qt6-specific addition for proper plugin management
};

} // namespace ItemEditor

#endif // ITEMEDITOR_PLUGIN_H