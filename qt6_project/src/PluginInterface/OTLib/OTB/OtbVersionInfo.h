/**
 * Item Editor Qt6 - OTB Version Info Header
 * Manages OTB file version information
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_OTBVERSIONINFO_H
#define ITEMEDITOR_OTBVERSIONINFO_H

#include <QString>

namespace OTLib {
namespace OTB {

/**
 * OTB Version Information Class
 * Manages version information for OTB files
 */
class OtbVersionInfo
{
public:
    OtbVersionInfo();
    OtbVersionInfo(int majorVersion, int minorVersion, int buildNumber, const QString& description);
    OtbVersionInfo(const OtbVersionInfo& other) = default;
    OtbVersionInfo& operator=(const OtbVersionInfo& other) = default;
    virtual ~OtbVersionInfo() = default;

    // Getters
    int getMajorVersion() const { return m_majorVersion; }
    int getMinorVersion() const { return m_minorVersion; }
    int getBuildNumber() const { return m_buildNumber; }
    QString getDescription() const { return m_description; }
    QString getVersionString() const;

    // Setters
    void setMajorVersion(int version) { m_majorVersion = version; }
    void setMinorVersion(int version) { m_minorVersion = version; }
    void setBuildNumber(int build) { m_buildNumber = build; }
    void setDescription(const QString& description) { m_description = description; }

    // Utility methods
    bool isValid() const;
    static OtbVersionInfo fromVersionNumber(int version);

private:
    int m_majorVersion;
    int m_minorVersion;
    int m_buildNumber;
    QString m_description;
};

} // namespace OTB
} // namespace OTLib

#endif // ITEMEDITOR_OTBVERSIONINFO_H