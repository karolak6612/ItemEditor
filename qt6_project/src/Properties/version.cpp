/**
 * Item Editor Qt6 - Version Information Implementation
 * Exact mirror of Legacy_App/csharp/Source/Properties/AssemblyInfo.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "version.h"
#include <QCoreApplication>

namespace ItemEditor {

QString Version::getVersionString()
{
    return QString("%1.%2.%3")
        .arg(MAJOR_VERSION)
        .arg(MINOR_VERSION)
        .arg(PATCH_VERSION);
}

QString Version::getFullVersionString()
{
    return QString("%1.%2.%3.%4")
        .arg(MAJOR_VERSION)
        .arg(MINOR_VERSION)
        .arg(PATCH_VERSION)
        .arg(BUILD_VERSION);
}

QVersionNumber Version::getVersionNumber()
{
    return QVersionNumber(MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION);
}

QString Version::getApplicationName()
{
    return QStringLiteral("Item Editor");
}

QString Version::getCompanyName()
{
    return QStringLiteral("OTTools");
}

QString Version::getCopyright()
{
    return QStringLiteral("Copyright © 2014-2019 OTTools");
}

QString Version::getDescription()
{
    return QStringLiteral("A tool for editing OTB (Open Tibia Binary) item databases");
}

} // namespace ItemEditor