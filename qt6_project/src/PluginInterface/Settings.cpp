/**
 * Item Editor Qt6 - Settings Class Implementation
 * Exact mirror of Legacy_App/csharp/Source/PluginInterface/Settings.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "Settings.h"
#include "IPlugin.h"
#include <QDir>
#include <QFile>
#include <QDomDocument>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDebug>

namespace ItemEditor {

Settings::Settings()
{
    // Constructor - initialize XML document like C# version
}

bool Settings::load(const QString& filename)
{
    // Exact mirror of C# Load method
    QString path = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("Plugins");
    
    try {
        m_settingsFilename = QDir(path).absoluteFilePath(filename);
        
        QFile file(m_settingsFilename);
        if (!file.open(QIODevice::ReadOnly)) {
            // Create default XML like C# version
            m_xmlDocument.setContent("<settings></settings>");
            return false;
        }
        
        QString errorMsg;
        int errorLine, errorColumn;
        if (!m_xmlDocument.setContent(&file, &errorMsg, &errorLine, &errorColumn)) {
            qWarning() << "Failed to parse XML:" << errorMsg << "at line" << errorLine;
            m_xmlDocument.setContent("<settings></settings>");
            file.close();
            return false;
        }
        
        file.close();
        return true;
    }
    catch (...) {
        m_xmlDocument.setContent("<settings></settings>");
        return false;
    }
}

QList<SupportedClient> Settings::getSupportedClientList()
{
    // Exact mirror of C# GetSupportedClientList method
    QList<SupportedClient> list;
    
    // Find all client nodes - exact mirror of C# XPath query
    QDomNodeList nodes = m_xmlDocument.elementsByTagName("client");
    
    for (int i = 0; i < nodes.count(); ++i) {
        QDomNode node = nodes.at(i);
        QDomElement element = node.toElement();
        
        if (!element.isNull()) {
            try {
                // Parse attributes - exact mirror of C# parsing logic
                bool ok;
                quint32 version = element.attribute("version").toUInt(&ok);
                if (!ok) continue;
                
                QString description = element.attribute("description");
                
                quint32 otbVersion = element.attribute("otbversion").toUInt(&ok);
                if (!ok) continue;
                
                quint32 datSignature = element.attribute("datsignature").toUInt(&ok, 16);
                if (!ok) continue;
                
                quint32 sprSignature = element.attribute("sprsignature").toUInt(&ok, 16);
                if (!ok) continue;
                
                // Create SupportedClient - exact mirror of C# constructor call
                SupportedClient client(version, description, otbVersion, datSignature, sprSignature);
                list.append(client);
            }
            catch (...) {
                QMessageBox::warning(nullptr, 
                                   QCoreApplication::applicationName(),
                                   QString("Error loading file %1").arg(m_settingsFilename));
            }
        }
    }
    
    return list;
}

} // namespace ItemEditor