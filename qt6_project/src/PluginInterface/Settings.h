/**
 * Item Editor Qt6 - Settings Class Header
 * Exact mirror of Legacy_App/csharp/Source/PluginInterface/Settings.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_SETTINGS_H
#define ITEMEDITOR_SETTINGS_H

#include <QString>
#include <QList>
#include <QDomDocument>

namespace ItemEditor {

// Forward declaration
class SupportedClient;

/**
 * Settings Class
 * Exact mirror of C# Settings class
 * Handles XML-based plugin configuration
 */
class Settings
{
public:
    /**
     * Constructor
     */
    Settings();
    
    /**
     * Destructor
     */
    virtual ~Settings() = default;

    // Properties - exact mirror of C# properties
    QString settingsFilename() const { return m_settingsFilename; }

    // Methods - exact mirror of C# methods
    bool load(const QString& filename);
    QList<SupportedClient> getSupportedClientList();

private:
    QDomDocument m_xmlDocument;
    QString m_settingsFilename;
};

} // namespace ItemEditor

#endif // ITEMEDITOR_SETTINGS_H