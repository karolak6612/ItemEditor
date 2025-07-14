/**
 * Item Editor Qt6 - Version Information Header
 * Exact mirror of Legacy_App/csharp/Source/Properties/AssemblyInfo.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_VERSION_H
#define ITEMEDITOR_VERSION_H

#include <QString>
#include <QVersionNumber>

namespace ItemEditor {

/**
 * Version class
 * Exact mirror of C# AssemblyInfo version information
 */
class Version
{
public:
    // Version constants - exact mirror of C# AssemblyInfo
    static constexpr int MAJOR_VERSION = 1;
    static constexpr int MINOR_VERSION = 0;
    static constexpr int PATCH_VERSION = 0;
    static constexpr int BUILD_VERSION = 0;
    
    // Version strings
    static QString getVersionString();
    static QString getFullVersionString();
    static QVersionNumber getVersionNumber();
    
    // Application information - exact mirror of C# AssemblyInfo
    static QString getApplicationName();
    static QString getCompanyName();
    static QString getCopyright();
    static QString getDescription();
    
private:
    Version() = delete; // Static class
};

} // namespace ItemEditor

#endif // ITEMEDITOR_VERSION_H