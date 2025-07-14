/**
 * Item Editor Qt6 - PluginThree Header
 * Exact mirror of Legacy_App/csharp/Source/PluginThree/Plugin.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef PLUGINTHREE_PLUGIN_H
#define PLUGINTHREE_PLUGIN_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QString>
#include <QDebug>
#include "IPlugin.h"
#include "Item.h"
#include "Settings.h"
#include "Sprite.h"

namespace PluginThree {

/**
 * ItemFlag enumeration
 * Exact mirror of C# ItemFlag enum in PluginThree
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
    NoMoveAnimation = 0x10,
    Pickupable = 0x11,
    Hangable = 0x12,
    IsHorizontal = 0x13,
    IsVertical = 0x14,
    Rotatable = 0x15,
    HasLight = 0x16,
    DontHide = 0x17,
    Translucent = 0x18,
    HasOffset = 0x19,
    HasElevation = 0x1A,
    Lying = 0x1B,
    AnimateAlways = 0x1C,
    Minimap = 0x1D,
    LensHelp = 0x1E,
    FullGround = 0x1F,
    IgnoreLook = 0x20,
    Cloth = 0x21,
    Market = 0x22,
    DefaultAction = 0x23,
    Wrappable = 0x24,
    Unwrappable = 0x25,
    TopEffect = 0x26,
    Usable = 0xFE,
    LastFlag = 0xFF
};

/**
 * PluginThree Class
 * Exact mirror of C# PluginThree.Plugin class
 * Handles OTB/DAT/SPR file format for newer Tibia clients
 */
class Plugin : public QObject, public ItemEditor::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ottools.ItemEditor.IPlugin/1.0" FILE "PluginThree.json")
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

} // namespace PluginThree

#endif // PLUGINTHREE_PLUGIN_H