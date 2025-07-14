/**
 * Item Editor Qt6 - PluginTwo Header
 * Exact mirror of Legacy_App/csharp/Source/PluginTwo/Plugin.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef PLUGINTWO_PLUGIN_H
#define PLUGINTWO_PLUGIN_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QString>
#include <QDebug>
#include "IPlugin.h"
#include "Item.h"
#include "Settings.h"
#include "Sprite.h"

namespace PluginTwo {

/**
 * ItemFlag enumeration
 * Exact mirror of C# ItemFlag enum in PluginTwo
 */
enum class ItemFlag : quint8
{
    Ground = 0x00,
    GroundBorder = 0x01,
    OnBottom = 0x02,
    OnTop = 0x03,
    Container = 0x04,
    Stackable = 0x05,
    ForceUse = 0x06,
    MultiUse = 0x07,
    Writable = 0x08,
    WritableOnce = 0x09,
    FluidContainer = 0x0A,
    Fluid = 0x0B,
    IsUnpassable = 0x0C,
    IsUnmoveable = 0x0D,
    BlockMissiles = 0x0E,
    BlockPathfinder = 0x0F,
    Pickupable = 0x10,
    Hangable = 0x11,
    IsHorizontal = 0x12,
    IsVertical = 0x13,
    Rotatable = 0x14,
    HasLight = 0x15,
    DontHide = 0x16,
    Translucent = 0x17,
    HasOffset = 0x18,
    HasElevation = 0x19,
    Lying = 0x1A,
    AnimateAlways = 0x1B,
    Minimap = 0x1C,
    LensHelp = 0x1D,
    FullGround = 0x1E,
    IgnoreLook = 0x1F,
    Cloth = 0x20,
    Market = 0x21,
    LastFlag = 0xFF
};

/**
 * PluginTwo Class
 * Exact mirror of C# PluginTwo.Plugin class
 * Handles OTB/DAT/SPR file format for mid-range Tibia clients
 */
class Plugin : public QObject, public ItemEditor::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ottools.ItemEditor.IPlugin/1.0" FILE "PluginTwo.json")
    Q_INTERFACES(ItemEditor::IPlugin)

public:
    /**
     * Constructor
     */
    explicit Plugin(QObject *parent = nullptr);
    
    /**
     * Destructor
     */
    virtual ~Plugin();

    // IPlugin interface implementation
    QString name() const override;
    ItemEditor::IPluginHost* host() const override;
    void setHost(ItemEditor::IPluginHost* host) override;
    
    ItemEditor::ClientItems* items() const override;
    quint16 minItemId() const override;
    quint16 maxItemId() const override;
    QList<ItemEditor::SupportedClient> supportedClients() const override;
    bool loaded() const override;
    
    bool loadClient(const ItemEditor::SupportedClient& client, 
                   bool extended, 
                   bool frameDurations, 
                   bool transparency,
                   const QString& datFullPath, 
                   const QString& sprFullPath) override;
    
    void initialize() override;
    
    ItemEditor::SupportedClient getClientBySignatures(quint32 datSignature, quint32 sprSignature) override;
    
    ItemEditor::ClientItem* getClientItem(quint16 id) override;

    // Plugin-specific methods
    bool loadSprites(const QString& filename, const ItemEditor::SupportedClient& client, bool extended, bool transparency);
    bool loadDat(const QString& filename, const ItemEditor::SupportedClient& client, bool extended, bool frameDurations);
    void dispose();

signals:
    // Qt-specific signals for plugin events
    void pluginLoaded();
    void pluginUnloaded();
    void clientLoaded(const ItemEditor::SupportedClient& client);
    void loadingProgress(int percentage);
    void errorOccurred(const QString& error);

private:
    ItemEditor::IPluginHost* m_host;
    ItemEditor::Settings* m_settings;
    QMap<quint32, ItemEditor::Sprite*> m_sprites;
    ItemEditor::ClientItems* m_items;
    QList<ItemEditor::SupportedClient> m_supportedClients;
    quint16 m_itemCount;
    bool m_loaded;
};

} // namespace PluginTwo

#endif // PLUGINTWO_PLUGIN_H