/**
 * Item Editor Qt6 - Supported Client Interface
 * Defines client version support for plugins
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_SUPPORTEDCLIENT_H
#define ITEMEDITOR_SUPPORTEDCLIENT_H

#include <QString>
#include <QObject>
#include <QMetaType>

namespace ItemEditor {

/**
 * Supported Client Class
 * Represents a client version supported by a plugin
 * Note: Not inheriting from QObject to allow storage in Qt containers
 */
class SupportedClient
{
public:
    SupportedClient();
    SupportedClient(const QString& name, const QString& version, int otbVersion);
    SupportedClient(quint32 version, const QString& description, quint32 otbVersion, quint32 datSignature, quint32 sprSignature);
    virtual ~SupportedClient() = default;

    // Copy constructor and assignment operator
    SupportedClient(const SupportedClient& other) = default;
    SupportedClient& operator=(const SupportedClient& other) = default;

    // Getters - match IPlugin.h interface
    QString getName() const { return m_name; }
    QString getVersion() const { return m_version; }
    int getOtbVersion() const { return m_otbVersion; }
    QString getDisplayName() const;
    
    // Additional getters for compatibility
    quint32 version() const { return static_cast<quint32>(m_otbVersion); }
    QString description() const { return getDisplayName(); }
    quint32 otbVersion() const { return static_cast<quint32>(m_otbVersion); }
    quint32 datSignature() const { return m_datSignature; }
    quint32 sprSignature() const { return m_sprSignature; }
    
    // Method-style getters for compatibility with NewOtbFileForm
    quint32 getDatSignature() const { return m_datSignature; }
    quint32 getSprSignature() const { return m_sprSignature; }

    // Setters
    void setName(const QString& name) { m_name = name; }
    void setVersion(const QString& version) { m_version = version; }
    void setOtbVersion(int version) { m_otbVersion = version; }

private:
    QString m_name;
    QString m_version;
    int m_otbVersion;
    quint32 m_datSignature;
    quint32 m_sprSignature;
};

} // namespace ItemEditor

// Register the type with Qt's meta-object system
Q_DECLARE_METATYPE(ItemEditor::SupportedClient)

#endif // ITEMEDITOR_SUPPORTEDCLIENT_H