/**
 * Item Editor Qt6 - Plugin Interface Definition
 * Exact mirror of Legacy_App/csharp/Source/PluginInterface/PluginInterface.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_IPLUGIN_H
#define ITEMEDITOR_IPLUGIN_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QtPlugin>
#include "SupportedClient.h"

namespace ItemEditor {

// Forward declarations
class ClientItems;
class SupportedClient;
class IPluginHost;
class ClientItem;

/**
 * ClientItems collection class
 * Exact mirror of C# ClientItems : Dictionary<ushort, ClientItem>
 */
class ClientItems : public QMap<quint16, ClientItem*>
{
public:
    ClientItems() : m_signatureCalculated(false) {}
    virtual ~ClientItems() = default;
    
    // Properties - exact mirror of C# properties
    bool signatureCalculated() const { return m_signatureCalculated; }
    void setSignatureCalculated(bool calculated) { m_signatureCalculated = calculated; }
    
private:
    bool m_signatureCalculated;
};

/**
 * Plugin Host Interface
 * Exact mirror of C# IPluginHost interface
 */
class IPluginHost
{
public:
    virtual ~IPluginHost() = default;
    
    // Host services will be added here as needed
    // Currently empty like the C# version
};

/**
 * Main Plugin Interface
 * Exact mirror of C# IPlugin : IDisposable interface
 */
class IPlugin
{
public:
    virtual ~IPlugin() = default;
    
    // Properties - exact mirror of C# IPlugin properties
    virtual QString name() const = 0;
    virtual IPluginHost* host() const = 0;
    virtual void setHost(IPluginHost* host) = 0;
    
    virtual ClientItems* items() const = 0;
    virtual quint16 minItemId() const = 0;
    virtual quint16 maxItemId() const = 0;
    virtual QList<SupportedClient> supportedClients() const = 0;
    virtual bool loaded() const = 0;
    
    // Methods - exact mirror of C# IPlugin methods
    virtual bool loadClient(const SupportedClient& client, 
                           bool extended, 
                           bool frameDurations, 
                           bool transparency,
                           const QString& datFullPath, 
                           const QString& sprFullPath) = 0;
    
    virtual void initialize() = 0;
    
    virtual SupportedClient getClientBySignatures(quint32 datSignature, quint32 sprSignature) = 0;
    
    virtual ItemEditor::ClientItem* getClientItem(quint16 id) = 0;
};

} // namespace ItemEditor

// Qt Plugin Interface Declaration
// This replaces the C# plugin loading mechanism
Q_DECLARE_INTERFACE(ItemEditor::IPlugin, "org.ottools.ItemEditor.IPlugin/1.0")

#endif // ITEMEDITOR_IPLUGIN_H