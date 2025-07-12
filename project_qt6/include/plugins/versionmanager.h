#ifndef VERSIONMANAGER_H
#define VERSIONMANAGER_H

#include "otb/item.h"
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QObject>
#include <QMutex>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QVersionNumber>

namespace PluginInterface {

/**
 * @brief Version information structure
 * 
 * Represents version information for different components
 * including client versions, OTB versions, and plugin API versions.
 */
struct VersionInfo {
    QVersionNumber version;              // Semantic version (major.minor.patch)
    quint32 numericVersion;              // Numeric representation (e.g., 1098 for 10.98)
    QString displayName;                 // Human-readable name
    QString description;                 // Detailed description
    QDateTime releaseDate;               // Release date
    bool isStable;                       // Whether this is a stable release
    bool isDeprecated;                   // Whether this version is deprecated
    QStringList tags;                    // Additional tags (e.g., "beta", "experimental")
    QMap<QString, QVariant> metadata;    // Additional metadata

    VersionInfo() 
        : numericVersion(0)
        , isStable(true)
        , isDeprecated(false)
    {}
    
    VersionInfo(const QVersionNumber& ver, quint32 numeric, const QString& name)
        : version(ver)
        , numericVersion(numeric)
        , displayName(name)
        , isStable(true)
        , isDeprecated(false)
    {}
    
    bool isValid() const { return !version.isNull() && numericVersion > 0; }
    QString toString() const { return displayName.isEmpty() ? version.toString() : displayName; }
};

/**
 * @brief Client version information
 * 
 * Specific version information for Tibia client versions
 * with associated OTB versions and file signatures.
 */
struct ClientVersionInfo : public VersionInfo {
    quint32 otbVersion;                  // Associated OTB version
    quint32 datSignature;                // Expected DAT file signature
    quint32 sprSignature;                // Expected SPR file signature
    QString clientPath;                  // Path to client files (if known)
    QStringList supportedFeatures;      // List of supported features
    QMap<QString, quint32> fileVersions; // Version info for different file types

    ClientVersionInfo() 
        : VersionInfo()
        , otbVersion(0)
        , datSignature(0)
        , sprSignature(0)
    {}
    
    ClientVersionInfo(const QVersionNumber& ver, quint32 numeric, const QString& name, 
                     quint32 otb, quint32 datSig, quint32 sprSig)
        : VersionInfo(ver, numeric, name)
        , otbVersion(otb)
        , datSignature(datSig)
        , sprSignature(sprSig)
    {}
    
    ItemEditor::SupportedClient toSupportedClient() const {
        return ItemEditor::SupportedClient(numericVersion, displayName, otbVersion, datSignature, sprSignature);
    }
};

/**
 * @brief Version compatibility information
 * 
 * Describes compatibility relationships between different versions.
 */
struct VersionCompatibility {
    QString sourceVersion;               // Source version identifier
    QString targetVersion;               // Target version identifier
    enum CompatibilityLevel {
        FullyCompatible,                 // Fully compatible
        MostlyCompatible,                // Compatible with minor issues
        PartiallyCompatible,             // Partially compatible, may have issues
        Incompatible,                    // Not compatible
        Unknown                          // Compatibility unknown
    } level;
    
    QString description;                 // Compatibility description
    QStringList knownIssues;            // List of known compatibility issues
    QStringList workarounds;            // Suggested workarounds
    bool requiresMigration;              // Whether migration is required
    QString migrationPath;               // Path for migration if needed

    VersionCompatibility() 
        : level(Unknown)
        , requiresMigration(false)
    {}
};

/**
 * @brief Version detection result
 * 
 * Result of automatic version detection from client files.
 */
struct VersionDetectionResult {
    bool success;                        // Whether detection was successful
    ClientVersionInfo detectedVersion;   // Detected version information
    QString detectionMethod;             // Method used for detection
    qreal confidence;                    // Confidence level (0.0 - 1.0)
    QStringList warnings;                // Detection warnings
    QString errorMessage;                // Error message if detection failed
    QMap<QString, QVariant> detectionData; // Additional detection data

    VersionDetectionResult() 
        : success(false)
        , confidence(0.0)
    {}
};

/**
 * @brief Version migration information
 * 
 * Information about migrating between versions.
 */
struct VersionMigration {
    QString fromVersion;                 // Source version
    QString toVersion;                   // Target version
    QString migrationName;               // Migration name/identifier
    QString description;                 // Migration description
    QStringList steps;                   // Migration steps
    bool isReversible;                   // Whether migration can be reversed
    bool requiresBackup;                 // Whether backup is recommended
    QString backupPath;                  // Suggested backup path
    QMap<QString, QVariant> parameters;  // Migration parameters

    VersionMigration() 
        : isReversible(false)
        , requiresBackup(true)
    {}
};

/**
 * @brief Version management system
 * 
 * Central system for managing plugin and client version compatibility,
 * automatic version detection, and version migration support.
 */
class VersionManager : public QObject
{
    Q_OBJECT

public:
    explicit VersionManager(QObject* parent = nullptr);
    virtual ~VersionManager();

    // Version registration and management
    void registerClientVersion(const ClientVersionInfo& versionInfo);
    void registerClientVersions(const QList<ClientVersionInfo>& versions);
    void unregisterClientVersion(const QString& versionId);
    void clearClientVersions();

    // Version lookup and query
    QList<ClientVersionInfo> getAllClientVersions() const;
    QList<ClientVersionInfo> getStableClientVersions() const;
    QList<ClientVersionInfo> getSupportedClientVersions() const;
    ClientVersionInfo getClientVersion(const QString& versionId) const;
    ClientVersionInfo getClientVersionByNumeric(quint32 numericVersion) const;
    ClientVersionInfo getClientVersionByOtb(quint32 otbVersion) const;
    
    // Version validation
    bool isValidClientVersion(const QString& versionId) const;
    bool isValidClientVersion(quint32 numericVersion) const;
    bool isSupportedClientVersion(const QString& versionId) const;
    bool isStableClientVersion(const QString& versionId) const;
    bool isDeprecatedClientVersion(const QString& versionId) const;

    // Version comparison
    int compareVersions(const QString& version1, const QString& version2) const;
    bool isVersionNewer(const QString& version1, const QString& version2) const;
    bool isVersionOlder(const QString& version1, const QString& version2) const;
    QString getLatestVersion() const;
    QString getLatestStableVersion() const;

    // Compatibility checking
    VersionCompatibility checkCompatibility(const QString& sourceVersion, const QString& targetVersion) const;
    bool areVersionsCompatible(const QString& version1, const QString& version2) const;
    QList<QString> getCompatibleVersions(const QString& baseVersion) const;
    QList<VersionCompatibility> getCompatibilityMatrix() const;

    // Version detection
    VersionDetectionResult detectClientVersion(const QString& datPath, const QString& sprPath) const;
    VersionDetectionResult detectClientVersionFromSignatures(quint32 datSignature, quint32 sprSignature) const;
    VersionDetectionResult detectClientVersionFromPath(const QString& clientPath) const;
    QList<VersionDetectionResult> detectAllPossibleVersions(const QString& datPath, const QString& sprPath) const;

    // Version migration
    void registerMigration(const VersionMigration& migration);
    QList<VersionMigration> getAvailableMigrations(const QString& fromVersion, const QString& toVersion) const;
    QList<VersionMigration> getMigrationPath(const QString& fromVersion, const QString& toVersion) const;
    bool canMigrate(const QString& fromVersion, const QString& toVersion) const;
    bool performMigration(const VersionMigration& migration, const QMap<QString, QVariant>& parameters = {});

    // Plugin compatibility
    bool isPluginCompatible(const QList<ItemEditor::SupportedClient>& supportedClients, const QString& clientVersion) const;
    QList<QString> getCompatiblePluginVersions(const QString& clientVersion) const;
    QList<ClientVersionInfo> getPluginSupportedVersions(const QList<ItemEditor::SupportedClient>& supportedClients) const;

    // Configuration and settings
    void setDefaultClientVersion(const QString& versionId);
    QString getDefaultClientVersion() const;
    void setVersionDetectionEnabled(bool enabled);
    bool isVersionDetectionEnabled() const;
    void setCompatibilityCheckingEnabled(bool enabled);
    bool isCompatibilityCheckingEnabled() const;

    // Import/Export
    bool loadVersionsFromJson(const QString& filePath);
    bool saveVersionsToJson(const QString& filePath) const;
    QJsonObject exportVersionsToJson() const;
    bool importVersionsFromJson(const QJsonObject& json);

    // Statistics and reporting
    int getClientVersionCount() const;
    int getSupportedVersionCount() const;
    QStringList getVersionTags() const;
    QMap<QString, int> getVersionStatistics() const;

signals:
    void clientVersionRegistered(const QString& versionId);
    void clientVersionUnregistered(const QString& versionId);
    void versionDetected(const VersionDetectionResult& result);
    void migrationStarted(const QString& fromVersion, const QString& toVersion);
    void migrationCompleted(const QString& fromVersion, const QString& toVersion, bool success);
    void compatibilityChanged(const QString& version1, const QString& version2);

public slots:
    void refreshVersionDatabase();
    void updateCompatibilityMatrix();

private:
    // Internal version management
    void initializeBuiltInVersions();
    void buildCompatibilityMatrix();
    QString generateVersionId(const ClientVersionInfo& versionInfo) const;
    
    // Version detection helpers
    VersionDetectionResult detectFromFileSignatures(const QString& datPath, const QString& sprPath) const;
    VersionDetectionResult detectFromFileHeaders(const QString& datPath, const QString& sprPath) const;
    VersionDetectionResult detectFromFileSize(const QString& datPath, const QString& sprPath) const;
    quint32 calculateFileSignature(const QString& filePath) const;
    
    // Compatibility calculation
    VersionCompatibility::CompatibilityLevel calculateCompatibilityLevel(
        const ClientVersionInfo& source, const ClientVersionInfo& target) const;
    void updateCompatibilityEntry(const QString& source, const QString& target, 
                                 VersionCompatibility::CompatibilityLevel level);
    
    // Migration helpers
    bool validateMigration(const VersionMigration& migration) const;
    QList<VersionMigration> findMigrationPath(const QString& fromVersion, const QString& toVersion) const;
    
    // Thread safety
    mutable QMutex m_mutex;
    
    // Version storage
    QMap<QString, ClientVersionInfo> m_clientVersions;     // versionId -> version info
    QMap<quint32, QString> m_numericToVersionId;           // numeric version -> version ID
    QMap<quint32, QString> m_otbToVersionId;               // OTB version -> version ID
    
    // Compatibility matrix
    QMap<QPair<QString, QString>, VersionCompatibility> m_compatibilityMatrix;
    
    // Migration registry
    QList<VersionMigration> m_migrations;
    
    // Configuration
    QString m_defaultClientVersion;
    bool m_versionDetectionEnabled;
    bool m_compatibilityCheckingEnabled;
    
    // Built-in version definitions
    static const QList<ClientVersionInfo> s_builtInVersions;
};

/**
 * @brief Version utility functions
 * 
 * Standalone utility functions for version operations.
 */
namespace VersionUtils {
    // Version parsing and formatting
    QVersionNumber parseVersionString(const QString& versionString);
    quint32 parseNumericVersion(const QString& versionString);
    QString formatVersion(const QVersionNumber& version);
    QString formatNumericVersion(quint32 numericVersion);
    
    // Version comparison utilities
    bool isVersionInRange(const QString& version, const QString& minVersion, const QString& maxVersion);
    QString getVersionRange(const QStringList& versions);
    QStringList sortVersions(const QStringList& versions, bool ascending = true);
    
    // Client version utilities
    quint32 clientVersionToNumeric(quint8 major, quint8 minor);
    QPair<quint8, quint8> numericToClientVersion(quint32 numericVersion);
    QString clientVersionToString(quint32 numericVersion);
    
    // OTB version utilities
    bool isValidOtbVersion(quint32 otbVersion);
    QString otbVersionToString(quint32 otbVersion);
    QList<quint32> getKnownOtbVersions();
    
    // File signature utilities
    quint32 calculateDatSignature(const QString& datPath);
    quint32 calculateSprSignature(const QString& sprPath);
    bool validateFileSignature(const QString& filePath, quint32 expectedSignature);
    
    // Version detection utilities
    QStringList getVersionDetectionMethods();
    qreal calculateDetectionConfidence(const QMap<QString, QVariant>& detectionData);
    
    // Migration utilities
    bool isValidMigrationPath(const QStringList& versionPath);
    QStringList optimizeMigrationPath(const QStringList& versionPath);
    
    // Error handling
    QString formatVersionError(const QString& operation, const QString& version, const QString& error);
}

} // namespace PluginInterface

// Meta-type declarations for Qt's type system
Q_DECLARE_METATYPE(PluginInterface::VersionInfo)
Q_DECLARE_METATYPE(PluginInterface::ClientVersionInfo)
Q_DECLARE_METATYPE(PluginInterface::VersionCompatibility)
Q_DECLARE_METATYPE(PluginInterface::VersionDetectionResult)
Q_DECLARE_METATYPE(PluginInterface::VersionMigration)

#endif // VERSIONMANAGER_H