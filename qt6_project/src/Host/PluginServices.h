/**
 * Item Editor Qt6 - Plugin Services
 * Exact mirror of Legacy_App/csharp/Source/Host/PluginServices.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_PLUGINSERVICES_H
#define ITEMEDITOR_PLUGINSERVICES_H

#include <QObject>
#include <QString>
#include <QDir>
#include <QPluginLoader>
#include <QMessageBox>
#include "PluginCollection.h"
#include "../PluginInterface/IPlugin.h"

namespace ItemEditor {

/**
 * Plugin Services Class
 * Exact mirror of C# PluginServices : IPluginHost
 * Manages plugin discovery, loading, and lifecycle
 */
class PluginServices : public QObject, public IPluginHost
{
    Q_OBJECT

public:
    /**
     * Constructor of PluginServices
     * Exact mirror of C# constructor
     */
    explicit PluginServices(QObject* parent = nullptr);
    
    /**
     * Destructor - ensures proper cleanup
     */
    virtual ~PluginServices();

    // Properties - exact mirror of C# properties
    /**
     * A collection of all plugins found by findPlugins()
     * Exact mirror of C# AvailablePlugins property
     */
    PluginCollection* availablePlugins() const { return m_availablePlugins; }
    
    /**
     * Singleton instance access
     */
    static PluginServices* getInstance();
    
    /**
     * Find plugin by signatures
     * Convenience method for plugin discovery
     */
    Plugin* findPlugin(quint32 datSignature, quint32 sprSignature) const;

public slots:
    /**
     * Searches the Path for plugins
     * Exact mirror of C# FindPlugins() method
     * Uses QDir instead of Directory.GetFiles()
     */
    void findPlugins();
    
    /**
     * Unloads all plugins
     * Exact mirror of C# ClosePlugins() method
     */
    void closePlugins();

signals:
    // Qt-specific signals for plugin events
    void pluginFound(const QString& pluginPath);
    void pluginLoaded(Plugin* plugin);
    void pluginLoadFailed(const QString& pluginPath, const QString& error);
    void allPluginsClosed();

private slots:
    /**
     * Memory optimization callback
     * Called when system requests memory optimization
     */
    void onMemoryOptimized();

private:
    /**
     * Adds a plugin from the specified path
     * Exact mirror of C# AddPlugin() method
     * Uses QPluginLoader instead of Assembly.LoadFrom()
     */
    void addPlugin(const QString& path);

private:
    PluginCollection* m_availablePlugins;
    static PluginServices* s_instance;
};

} // namespace ItemEditor

#endif // ITEMEDITOR_PLUGINSERVICES_H