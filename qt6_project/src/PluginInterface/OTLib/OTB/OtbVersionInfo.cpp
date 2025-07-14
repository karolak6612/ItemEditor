/**
 * Item Editor Qt6 - OTB Version Info Implementation
 * Manages OTB file version information
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "OtbVersionInfo.h"

namespace OTLib {
namespace OTB {

OtbVersionInfo::OtbVersionInfo()
    : m_majorVersion(0)
    , m_minorVersion(0)
    , m_buildNumber(0)
{
}

OtbVersionInfo::OtbVersionInfo(int majorVersion, int minorVersion, int buildNumber, const QString& description)
    : m_majorVersion(majorVersion)
    , m_minorVersion(minorVersion)
    , m_buildNumber(buildNumber)
    , m_description(description)
{
}

QString OtbVersionInfo::getVersionString() const
{
    return QString("%1.%2.%3").arg(m_majorVersion).arg(m_minorVersion).arg(m_buildNumber);
}

bool OtbVersionInfo::isValid() const
{
    return m_majorVersion > 0 || m_minorVersion > 0 || m_buildNumber > 0;
}

OtbVersionInfo OtbVersionInfo::fromVersionNumber(int version)
{
    // Extract version components from packed version number
    int major = (version >> 16) & 0xFF;
    int minor = (version >> 8) & 0xFF;
    int build = version & 0xFF;
    
    QString description;
    
    // Map known versions to descriptions
    switch (version) {
        case 0x00000001:
            description = "OpenTibia 0.6.0";
            break;
        case 0x00000002:
            description = "OpenTibia 0.6.1";
            break;
        case 0x00000003:
            description = "OpenTibia 0.6.2";
            break;
        case 0x00000004:
            description = "OpenTibia 0.6.3";
            break;
        default:
            description = QString("Unknown version %1").arg(version, 0, 16);
            break;
    }
    
    return OtbVersionInfo(major, minor, build, description);
}

} // namespace OTB
} // namespace OTLib