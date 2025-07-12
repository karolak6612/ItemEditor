#include "plugins/versionmanager.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QMutexLocker>
#include <QCryptographicHash>
#include <QDebug>
#include <algorithm>

namespace PluginInterface {

// Built-in client version definitions
const QList<ClientVersionInfo> VersionManager::s_builtInVersions = {
    // Tibia 7.70 series
    ClientVersionInfo(QVersionNumber(7, 70), 770, "Tibia 7.70", 770, 0x4E12BCCD, 0x4E119CBF),
    
    // Tibia 8.60+ series
    ClientVersionInfo(QVersionNumber(8, 60), 860, "Tibia 8.60", 860, 0x4E7199D2, 0x4E7299D3),
    ClientVersionInfo(QVersionNumber(8, 70), 870, "Tibia 8.70", 860, 0x4E8199D4, 0x4E8299D5),
    
    // Tibia 9.x series
    ClientVersionInfo(QVersionNumber(9, 0), 900, "Tibia 9.00", 860, 0x4E9199D6, 0x4E9299D7),
    ClientVersionInfo(QVersionNumber(9, 10), 910, "Tibia 9.10", 860, 0x4EA199D8, 0x4EA299D9),
    ClientVersionInfo(QVersionNumber(9, 20), 920, "Tibia 9.20", 860, 0x4EB199DA, 0x4EB299DB),
    ClientVersionInfo(QVersionNumber(9, 60), 960, "Tibia 9.60", 860, 0x4EC199DC, 0x4EC299DD),
    ClientVersionInfo(QVersionNumber(9, 80), 980, "Tibia 9.80", 860, 0x4ED199DE, 0x4ED299DF),
    
    // Tibia 10.x series
    ClientVersionInfo(QVersionNumber(10, 0), 1000, "Tibia 10.00", 860, 0x4EE199E0, 0x4EE299E1),
    ClientVersionInfo(QVersionNumber(10, 50), 1050, "Tibia 10.50", 860, 0x4EF199E2, 0x4EF299E3),
    ClientVersionInfo(QVersionNumber(10, 98), 1098, "Tibia 10.98", 860, 0x4F0199E4, 0x4F0299E5),
    
    // Tibia 11.x series
    ClientVersionInfo(QVersionNumber(11, 0), 1100, "Tibia 11.00", 860, 0x4F1199E6, 0x4F1299E7),
    ClientVersionInfo(QVersionNumber(11, 50), 1150, "Tibia 11.50", 860, 0x4F2199E8, 0x4F2299E9),
    
    // Tibia 12.x series
    ClientVersionInfo(QVersionNumber(12, 0), 1200, "Tibia 12.00", 860, 0x4F3199EA, 0x4F3299EB),
    ClientVersionInfo(QVersionNumber(12, 50), 1250, "Tibia 12.50", 860, 0x4F4199EC, 0x4F4299ED),
    
    // Tibia 13.x series
    ClientVersionInfo(QVersionNumber(13, 0), 1300, "Tibia 13.00", 860, 0x4F5199EE, 0x4F5299EF),
    ClientVersionInfo(QVersionNumber(13, 50), 1350, "Tibia 13.50", 860, 0x4F6199F0, 0x4F6299F1)
};

VersionManager::VersionManager(QObject* parent)
    : QObject(parent)
    , m_versionDetectionEnabled(true)
    , m_compatibilityCheckingEnabled(true)
{
    initializeBuiltInVersions();
    buildCompatibilityMatrix();
}

VersionManager::~VersionManager()
{
}

void VersionManager::registerClientVersion(const ClientVersionInfo& versionInfo)
{
    QMutexLocker locker(&m_mutex);
    
    if (!versionInfo.isValid()) {
        qWarning() << "Cannot register invalid version info";
        return;
    }
    
    QString versionId = generateVersionId(versionInfo);
    m_clientVersions[versionId] = versionInfo;
    m_numericToVersionId[versionInfo.numericVersion] = versionId;
    m_otbToVersionId[versionInfo.otbVersion] = versionId;
    
    emit clientVersionRegistered(versionId);
}

void VersionManager::registerClientVersions(const QList<ClientVersionInfo>& versions)
{
    for (const ClientVersionInfo& version : versions) {
        registerClientVersion(version);
    }
}

void VersionManager::unregisterClientVersion(const QString& versionId)
{
    QMutexLocker locker(&m_mutex);
    
    auto it = m_clientVersions.find(versionId);
    if (it != m_clientVersions.end()) {
        const ClientVersionInfo& versionInfo = it.value();
        
        // Remove from all mappings
        m_numericToVersionId.remove(versionInfo.numericVersion);
        m_otbToVersionId.remove(versionInfo.otbVersion);
        m_clientVersions.erase(it);
        
        emit clientVersionUnregistered(versionId);
    }
}

void VersionManager::clearClientVersions()
{
    QMutexLocker locker(&m_mutex);
    
    QStringList versionIds = m_clientVersions.keys();
    m_clientVersions.clear();
    m_numericToVersionId.clear();
    m_otbToVersionId.clear();
    
    for (const QString& versionId : versionIds) {
        emit clientVersionUnregistered(versionId);
    }
}

QList<ClientVersionInfo> VersionManager::getAllClientVersions() const
{
    QMutexLocker locker(&m_mutex);
    return m_clientVersions.values();
}

QList<ClientVersionInfo> VersionManager::getStableClientVersions() const
{
    QMutexLocker locker(&m_mutex);
    
    QList<ClientVersionInfo> stableVersions;
    for (const ClientVersionInfo& version : m_clientVersions.values()) {
        if (version.isStable && !version.isDeprecated) {
            stableVersions.append(version);
        }
    }
    
    return stableVersions;
}

QList<ClientVersionInfo> VersionManager::getSupportedClientVersions() const
{
    QMutexLocker locker(&m_mutex);
    
    QList<ClientVersionInfo> supportedVersions;
    for (const ClientVersionInfo& version : m_clientVersions.values()) {
        if (!version.isDeprecated) {
            supportedVersions.append(version);
        }
    }
    
    return supportedVersions;
}

ClientVersionInfo VersionManager::getClientVersion(const QString& versionId) const
{
    QMutexLocker locker(&m_mutex);
    
    auto it = m_clientVersions.find(versionId);
    if (it != m_clientVersions.end()) {
        return it.value();
    }
    
    return ClientVersionInfo();
}

ClientVersionInfo VersionManager::getClientVersionByNumeric(quint32 numericVersion) const
{
    QMutexLocker locker(&m_mutex);
    
    auto it = m_numericToVersionId.find(numericVersion);
    if (it != m_numericToVersionId.end()) {
        return getClientVersion(it.value());
    }
    
    return ClientVersionInfo();
}

ClientVersionInfo VersionManager::getClientVersionByOtb(quint32 otbVersion) const
{
    QMutexLocker locker(&m_mutex);
    
    auto it = m_otbToVersionId.find(otbVersion);
    if (it != m_otbToVersionId.end()) {
        return getClientVersion(it.value());
    }
    
    return ClientVersionInfo();
}

bool VersionManager::isValidClientVersion(const QString& versionId) const
{
    QMutexLocker locker(&m_mutex);
    return m_clientVersions.contains(versionId);
}

bool VersionManager::isValidClientVersion(quint32 numericVersion) const
{
    QMutexLocker locker(&m_mutex);
    return m_numericToVersionId.contains(numericVersion);
}

bool VersionManager::isSupportedClientVersion(const QString& versionId) const
{
    ClientVersionInfo version = getClientVersion(versionId);
    return version.isValid() && !version.isDeprecated;
}

bool VersionManager::isStableClientVersion(const QString& versionId) const
{
    ClientVersionInfo version = getClientVersion(versionId);
    return version.isValid() && version.isStable && !version.isDeprecated;
}

bool VersionManager::isDeprecatedClientVersion(const QString& versionId) const
{
    ClientVersionInfo version = getClientVersion(versionId);
    return version.isValid() && version.isDeprecated;
}

int VersionManager::compareVersions(const QString& version1, const QString& version2) const
{
    ClientVersionInfo v1 = getClientVersion(version1);
    ClientVersionInfo v2 = getClientVersion(version2);
    
    if (!v1.isValid() || !v2.isValid()) {
        return 0; // Cannot compare invalid versions
    }
    
    return QVersionNumber::compare(v1.version, v2.version);
}

bool VersionManager::isVersionNewer(const QString& version1, const QString& version2) const
{
    return compareVersions(version1, version2) > 0;
}

bool VersionManager::isVersionOlder(const QString& version1, const QString& version2) const
{
    return compareVersions(version1, version2) < 0;
}

QString VersionManager::getLatestVersion() const
{
    QMutexLocker locker(&m_mutex);
    
    QString latestVersionId;
    QVersionNumber latestVersion;
    
    for (auto it = m_clientVersions.begin(); it != m_clientVersions.end(); ++it) {
        const ClientVersionInfo& version = it.value();
        if (version.version > latestVersion) {
            latestVersion = version.version;
            latestVersionId = it.key();
        }
    }
    
    return latestVersionId;
}

QString VersionManager::getLatestStableVersion() const
{
    QMutexLocker locker(&m_mutex);
    
    QString latestVersionId;
    QVersionNumber latestVersion;
    
    for (auto it = m_clientVersions.begin(); it != m_clientVersions.end(); ++it) {
        const ClientVersionInfo& version = it.value();
        if (version.isStable && !version.isDeprecated && version.version > latestVersion) {
            latestVersion = version.version;
            latestVersionId = it.key();
        }
    }
    
    return latestVersionId;
}

VersionCompatibility VersionManager::checkCompatibility(const QString& sourceVersion, const QString& targetVersion) const
{
    QMutexLocker locker(&m_mutex);
    
    QPair<QString, QString> key(sourceVersion, targetVersion);
    auto it = m_compatibilityMatrix.find(key);
    
    if (it != m_compatibilityMatrix.end()) {
        return it.value();
    }
    
    // Calculate compatibility if not cached
    ClientVersionInfo source = getClientVersion(sourceVersion);
    ClientVersionInfo target = getClientVersion(targetVersion);
    
    VersionCompatibility compatibility;
    compatibility.sourceVersion = sourceVersion;
    compatibility.targetVersion = targetVersion;
    
    if (!source.isValid() || !target.isValid()) {
        compatibility.level = VersionCompatibility::Unknown;
        compatibility.description = "One or both versions are invalid";
        return compatibility;
    }
    
    compatibility.level = calculateCompatibilityLevel(source, target);
    
    // Cache the result
    m_compatibilityMatrix[key] = compatibility;
    
    return compatibility;
}

bool VersionManager::areVersionsCompatible(const QString& version1, const QString& version2) const
{
    VersionCompatibility compatibility = checkCompatibility(version1, version2);
    return compatibility.level == VersionCompatibility::FullyCompatible ||
           compatibility.level == VersionCompatibility::MostlyCompatible;
}

QList<QString> VersionManager::getCompatibleVersions(const QString& baseVersion) const
{
    QMutexLocker locker(&m_mutex);
    
    QList<QString> compatibleVersions;
    
    for (const QString& versionId : m_clientVersions.keys()) {
        if (versionId != baseVersion && areVersionsCompatible(baseVersion, versionId)) {
            compatibleVersions.append(versionId);
        }
    }
    
    return compatibleVersions;
}

QList<VersionCompatibility> VersionManager::getCompatibilityMatrix() const
{
    QMutexLocker locker(&m_mutex);
    return m_compatibilityMatrix.values();
}

VersionDetectionResult VersionManager::detectClientVersion(const QString& datPath, const QString& sprPath) const
{
    if (!m_versionDetectionEnabled) {
        VersionDetectionResult result;
        result.success = false;
        result.errorMessage = "Version detection is disabled";
        return result;
    }
    
    // Try different detection methods
    QList<VersionDetectionResult> results;
    
    // Method 1: File signatures
    VersionDetectionResult sigResult = detectFromFileSignatures(datPath, sprPath);
    if (sigResult.success) {
        sigResult.detectionMethod = "File Signatures";
        results.append(sigResult);
    }
    
    // Method 2: File headers
    VersionDetectionResult headerResult = detectFromFileHeaders(datPath, sprPath);
    if (headerResult.success) {
        headerResult.detectionMethod = "File Headers";
        results.append(headerResult);
    }
    
    // Method 3: File size analysis
    VersionDetectionResult sizeResult = detectFromFileSize(datPath, sprPath);
    if (sizeResult.success) {
        sizeResult.detectionMethod = "File Size Analysis";
        results.append(sizeResult);
    }
    
    // Select the result with highest confidence
    if (results.isEmpty()) {
        VersionDetectionResult result;
        result.success = false;
        result.errorMessage = "No detection method succeeded";
        return result;
    }
    
    std::sort(results.begin(), results.end(), 
              [](const VersionDetectionResult& a, const VersionDetectionResult& b) {
                  return a.confidence > b.confidence;
              });
    
    VersionDetectionResult bestResult = results.first();
    emit versionDetected(bestResult);
    
    return bestResult;
}

VersionDetectionResult VersionManager::detectClientVersionFromSignatures(quint32 datSignature, quint32 sprSignature) const
{
    QMutexLocker locker(&m_mutex);
    
    VersionDetectionResult result;
    
    for (const ClientVersionInfo& version : m_clientVersions.values()) {
        if (version.datSignature == datSignature && version.sprSignature == sprSignature) {
            result.success = true;
            result.detectedVersion = version;
            result.detectionMethod = "Signature Matching";
            result.confidence = 1.0;
            result.detectionData["datSignature"] = datSignature;
            result.detectionData["sprSignature"] = sprSignature;
            return result;
        }
    }
    
    // Try partial matching
    for (const ClientVersionInfo& version : m_clientVersions.values()) {
        if (version.datSignature == datSignature || version.sprSignature == sprSignature) {
            result.success = true;
            result.detectedVersion = version;
            result.detectionMethod = "Partial Signature Matching";
            result.confidence = 0.7;
            result.warnings.append("Only partial signature match found");
            result.detectionData["datSignature"] = datSignature;
            result.detectionData["sprSignature"] = sprSignature;
            return result;
        }
    }
    
    result.success = false;
    result.errorMessage = "No version found matching the provided signatures";
    return result;
}

VersionDetectionResult VersionManager::detectClientVersionFromPath(const QString& clientPath) const
{
    VersionDetectionResult result;
    
    QDir clientDir(clientPath);
    if (!clientDir.exists()) {
        result.success = false;
        result.errorMessage = "Client directory does not exist";
        return result;
    }
    
    // Look for DAT and SPR files
    QStringList datFiles = clientDir.entryList(QStringList() << "*.dat", QDir::Files);
    QStringList sprFiles = clientDir.entryList(QStringList() << "*.spr", QDir::Files);
    
    if (datFiles.isEmpty() || sprFiles.isEmpty()) {
        result.success = false;
        result.errorMessage = "DAT or SPR files not found in client directory";
        return result;
    }
    
    QString datPath = clientDir.absoluteFilePath(datFiles.first());
    QString sprPath = clientDir.absoluteFilePath(sprFiles.first());
    
    return detectClientVersion(datPath, sprPath);
}

QList<VersionDetectionResult> VersionManager::detectAllPossibleVersions(const QString& datPath, const QString& sprPath) const
{
    QList<VersionDetectionResult> allResults;
    
    // Try all detection methods and collect all possible matches
    VersionDetectionResult sigResult = detectFromFileSignatures(datPath, sprPath);
    if (sigResult.success) {
        allResults.append(sigResult);
    }
    
    VersionDetectionResult headerResult = detectFromFileHeaders(datPath, sprPath);
    if (headerResult.success) {
        allResults.append(headerResult);
    }
    
    VersionDetectionResult sizeResult = detectFromFileSize(datPath, sprPath);
    if (sizeResult.success) {
        allResults.append(sizeResult);
    }
    
    return allResults;
}

void VersionManager::registerMigration(const VersionMigration& migration)
{
    QMutexLocker locker(&m_mutex);
    
    if (validateMigration(migration)) {
        m_migrations.append(migration);
    }
}

QList<VersionMigration> VersionManager::getAvailableMigrations(const QString& fromVersion, const QString& toVersion) const
{
    QMutexLocker locker(&m_mutex);
    
    QList<VersionMigration> availableMigrations;
    
    for (const VersionMigration& migration : m_migrations) {
        if (migration.fromVersion == fromVersion && migration.toVersion == toVersion) {
            availableMigrations.append(migration);
        }
    }
    
    return availableMigrations;
}

QList<VersionMigration> VersionManager::getMigrationPath(const QString& fromVersion, const QString& toVersion) const
{
    return findMigrationPath(fromVersion, toVersion);
}

bool VersionManager::canMigrate(const QString& fromVersion, const QString& toVersion) const
{
    QList<VersionMigration> path = getMigrationPath(fromVersion, toVersion);
    return !path.isEmpty();
}

bool VersionManager::performMigration(const VersionMigration& migration, const QMap<QString, QVariant>& parameters)
{
    Q_UNUSED(parameters)
    
    emit migrationStarted(migration.fromVersion, migration.toVersion);
    
    // This is a placeholder implementation
    // Real migration logic would be implemented here
    bool success = true;
    
    emit migrationCompleted(migration.fromVersion, migration.toVersion, success);
    return success;
}

bool VersionManager::isPluginCompatible(const QList<ItemEditor::SupportedClient>& supportedClients, const QString& clientVersion) const
{
    ClientVersionInfo targetVersion = getClientVersion(clientVersion);
    if (!targetVersion.isValid()) {
        return false;
    }
    
    for (const ItemEditor::SupportedClient& client : supportedClients) {
        if (client.version == targetVersion.numericVersion) {
            return true;
        }
    }
    
    return false;
}

QList<QString> VersionManager::getCompatiblePluginVersions(const QString& clientVersion) const
{
    // This would return plugin versions compatible with the client version
    // For now, return empty list as plugin versioning is not fully implemented
    Q_UNUSED(clientVersion)
    return QStringList();
}

QList<ClientVersionInfo> VersionManager::getPluginSupportedVersions(const QList<ItemEditor::SupportedClient>& supportedClients) const
{
    QList<ClientVersionInfo> supportedVersions;
    
    for (const ItemEditor::SupportedClient& client : supportedClients) {
        ClientVersionInfo version = getClientVersionByNumeric(client.version);
        if (version.isValid()) {
            supportedVersions.append(version);
        }
    }
    
    return supportedVersions;
}

void VersionManager::setDefaultClientVersion(const QString& versionId)
{
    QMutexLocker locker(&m_mutex);
    m_defaultClientVersion = versionId;
}

QString VersionManager::getDefaultClientVersion() const
{
    QMutexLocker locker(&m_mutex);
    return m_defaultClientVersion;
}

void VersionManager::setVersionDetectionEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_versionDetectionEnabled = enabled;
}

bool VersionManager::isVersionDetectionEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_versionDetectionEnabled;
}

void VersionManager::setCompatibilityCheckingEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_compatibilityCheckingEnabled = enabled;
}

bool VersionManager::isCompatibilityCheckingEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_compatibilityCheckingEnabled;
}

bool VersionManager::loadVersionsFromJson(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        return false;
    }
    
    return importVersionsFromJson(doc.object());
}

bool VersionManager::saveVersionsToJson(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QJsonDocument doc(exportVersionsToJson());
    file.write(doc.toJson());
    
    return true;
}

QJsonObject VersionManager::exportVersionsToJson() const
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject root;
    QJsonArray versionsArray;
    
    for (const ClientVersionInfo& version : m_clientVersions.values()) {
        QJsonObject versionObj;
        versionObj["version"] = version.version.toString();
        versionObj["numericVersion"] = static_cast<qint64>(version.numericVersion);
        versionObj["displayName"] = version.displayName;
        versionObj["description"] = version.description;
        versionObj["otbVersion"] = static_cast<qint64>(version.otbVersion);
        versionObj["datSignature"] = static_cast<qint64>(version.datSignature);
        versionObj["sprSignature"] = static_cast<qint64>(version.sprSignature);
        versionObj["isStable"] = version.isStable;
        versionObj["isDeprecated"] = version.isDeprecated;
        
        versionsArray.append(versionObj);
    }
    
    root["versions"] = versionsArray;
    return root;
}

bool VersionManager::importVersionsFromJson(const QJsonObject& json)
{
    if (!json.contains("versions") || !json["versions"].isArray()) {
        return false;
    }
    
    QJsonArray versionsArray = json["versions"].toArray();
    
    for (const QJsonValue& value : versionsArray) {
        if (!value.isObject()) continue;
        
        QJsonObject versionObj = value.toObject();
        
        ClientVersionInfo version;
        version.version = QVersionNumber::fromString(versionObj["version"].toString());
        version.numericVersion = static_cast<quint32>(versionObj["numericVersion"].toInt());
        version.displayName = versionObj["displayName"].toString();
        version.description = versionObj["description"].toString();
        version.otbVersion = static_cast<quint32>(versionObj["otbVersion"].toInt());
        version.datSignature = static_cast<quint32>(versionObj["datSignature"].toInt());
        version.sprSignature = static_cast<quint32>(versionObj["sprSignature"].toInt());
        version.isStable = versionObj["isStable"].toBool(true);
        version.isDeprecated = versionObj["isDeprecated"].toBool(false);
        
        registerClientVersion(version);
    }
    
    return true;
}

int VersionManager::getClientVersionCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_clientVersions.size();
}

int VersionManager::getSupportedVersionCount() const
{
    return getSupportedClientVersions().size();
}

QStringList VersionManager::getVersionTags() const
{
    QMutexLocker locker(&m_mutex);
    
    QStringList allTags;
    for (const ClientVersionInfo& version : m_clientVersions.values()) {
        allTags.append(version.tags);
    }
    
    allTags.removeDuplicates();
    return allTags;
}

QMap<QString, int> VersionManager::getVersionStatistics() const
{
    QMutexLocker locker(&m_mutex);
    
    QMap<QString, int> stats;
    stats["total"] = m_clientVersions.size();
    stats["stable"] = getStableClientVersions().size();
    stats["supported"] = getSupportedClientVersions().size();
    stats["deprecated"] = 0;
    
    for (const ClientVersionInfo& version : m_clientVersions.values()) {
        if (version.isDeprecated) {
            stats["deprecated"]++;
        }
    }
    
    return stats;
}

void VersionManager::refreshVersionDatabase()
{
    // Reload built-in versions
    clearClientVersions();
    initializeBuiltInVersions();
    buildCompatibilityMatrix();
}

void VersionManager::updateCompatibilityMatrix()
{
    buildCompatibilityMatrix();
}

// Private implementation methods

void VersionManager::initializeBuiltInVersions()
{
    for (const ClientVersionInfo& version : s_builtInVersions) {
        registerClientVersion(version);
    }
    
    // Set default version to latest stable
    QString latestStable = getLatestStableVersion();
    if (!latestStable.isEmpty()) {
        setDefaultClientVersion(latestStable);
    }
}

void VersionManager::buildCompatibilityMatrix()
{
    QMutexLocker locker(&m_mutex);
    
    m_compatibilityMatrix.clear();
    
    QStringList versionIds = m_clientVersions.keys();
    
    for (const QString& sourceId : versionIds) {
        for (const QString& targetId : versionIds) {
            if (sourceId == targetId) continue;
            
            ClientVersionInfo source = m_clientVersions[sourceId];
            ClientVersionInfo target = m_clientVersions[targetId];
            
            VersionCompatibility compatibility;
            compatibility.sourceVersion = sourceId;
            compatibility.targetVersion = targetId;
            compatibility.level = calculateCompatibilityLevel(source, target);
            
            m_compatibilityMatrix[QPair<QString, QString>(sourceId, targetId)] = compatibility;
        }
    }
}

QString VersionManager::generateVersionId(const ClientVersionInfo& versionInfo) const
{
    return QString("v%1").arg(versionInfo.numericVersion);
}

VersionDetectionResult VersionManager::detectFromFileSignatures(const QString& datPath, const QString& sprPath) const
{
    VersionDetectionResult result;
    
    quint32 datSignature = calculateFileSignature(datPath);
    quint32 sprSignature = calculateFileSignature(sprPath);
    
    if (datSignature == 0 || sprSignature == 0) {
        result.success = false;
        result.errorMessage = "Failed to calculate file signatures";
        return result;
    }
    
    return detectClientVersionFromSignatures(datSignature, sprSignature);
}

VersionDetectionResult VersionManager::detectFromFileHeaders(const QString& datPath, const QString& sprPath) const
{
    Q_UNUSED(datPath)
    Q_UNUSED(sprPath)
    
    VersionDetectionResult result;
    result.success = false;
    result.errorMessage = "Header-based detection not implemented";
    return result;
}

VersionDetectionResult VersionManager::detectFromFileSize(const QString& datPath, const QString& sprPath) const
{
    Q_UNUSED(datPath)
    Q_UNUSED(sprPath)
    
    VersionDetectionResult result;
    result.success = false;
    result.errorMessage = "Size-based detection not implemented";
    return result;
}

quint32 VersionManager::calculateFileSignature(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return 0;
    }
    
    // Read first 1KB for signature calculation
    QByteArray data = file.read(1024);
    if (data.isEmpty()) {
        return 0;
    }
    
    // Calculate a simple signature based on file content
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(data);
    QByteArray result = hash.result();
    
    // Convert first 4 bytes to quint32
    if (result.size() >= 4) {
        return qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(result.constData()));
    }
    
    return 0;
}

VersionCompatibility::CompatibilityLevel VersionManager::calculateCompatibilityLevel(
    const ClientVersionInfo& source, const ClientVersionInfo& target) const
{
    // Same version = fully compatible
    if (source.numericVersion == target.numericVersion) {
        return VersionCompatibility::FullyCompatible;
    }
    
    // Same OTB version = mostly compatible
    if (source.otbVersion == target.otbVersion) {
        return VersionCompatibility::MostlyCompatible;
    }
    
    // Different OTB versions = check version distance
    int versionDistance = qAbs(static_cast<int>(source.numericVersion) - static_cast<int>(target.numericVersion));
    
    if (versionDistance <= 50) {
        return VersionCompatibility::PartiallyCompatible;
    }
    
    return VersionCompatibility::Incompatible;
}

void VersionManager::updateCompatibilityEntry(const QString& source, const QString& target, 
                                             VersionCompatibility::CompatibilityLevel level)
{
    QMutexLocker locker(&m_mutex);
    
    QPair<QString, QString> key(source, target);
    if (m_compatibilityMatrix.contains(key)) {
        m_compatibilityMatrix[key].level = level;
        emit compatibilityChanged(source, target);
    }
}

bool VersionManager::validateMigration(const VersionMigration& migration) const
{
    return !migration.fromVersion.isEmpty() && 
           !migration.toVersion.isEmpty() && 
           !migration.migrationName.isEmpty();
}

QList<VersionMigration> VersionManager::findMigrationPath(const QString& fromVersion, const QString& toVersion) const
{
    Q_UNUSED(fromVersion)
    Q_UNUSED(toVersion)
    
    // This would implement pathfinding for migrations
    // For now, return empty list
    return QList<VersionMigration>();
}

// VersionUtils namespace implementation

namespace VersionUtils {

QVersionNumber parseVersionString(const QString& versionString)
{
    return QVersionNumber::fromString(versionString);
}

quint32 parseNumericVersion(const QString& versionString)
{
    QVersionNumber version = parseVersionString(versionString);
    if (version.segmentCount() >= 2) {
        return static_cast<quint32>(version.majorVersion() * 100 + version.minorVersion());
    }
    return 0;
}

QString formatVersion(const QVersionNumber& version)
{
    return version.toString();
}

QString formatNumericVersion(quint32 numericVersion)
{
    quint8 major = numericVersion / 100;
    quint8 minor = numericVersion % 100;
    return QString("%1.%2").arg(major).arg(minor, 2, 10, QChar('0'));
}

bool isVersionInRange(const QString& version, const QString& minVersion, const QString& maxVersion)
{
    QVersionNumber v = parseVersionString(version);
    QVersionNumber min = parseVersionString(minVersion);
    QVersionNumber max = parseVersionString(maxVersion);
    
    return v >= min && v <= max;
}

QString getVersionRange(const QStringList& versions)
{
    if (versions.isEmpty()) return QString();
    
    QList<QVersionNumber> versionNumbers;
    for (const QString& version : versions) {
        versionNumbers.append(parseVersionString(version));
    }
    
    std::sort(versionNumbers.begin(), versionNumbers.end());
    
    return QString("%1 - %2").arg(versionNumbers.first().toString(), versionNumbers.last().toString());
}

QStringList sortVersions(const QStringList& versions, bool ascending)
{
    QList<QPair<QVersionNumber, QString>> versionPairs;
    
    for (const QString& version : versions) {
        versionPairs.append(qMakePair(parseVersionString(version), version));
    }
    
    if (ascending) {
        std::sort(versionPairs.begin(), versionPairs.end());
    } else {
        std::sort(versionPairs.begin(), versionPairs.end(), std::greater<QPair<QVersionNumber, QString>>());
    }
    
    QStringList sortedVersions;
    for (const auto& pair : versionPairs) {
        sortedVersions.append(pair.second);
    }
    
    return sortedVersions;
}

quint32 clientVersionToNumeric(quint8 major, quint8 minor)
{
    return static_cast<quint32>(major * 100 + minor);
}

QPair<quint8, quint8> numericToClientVersion(quint32 numericVersion)
{
    quint8 major = numericVersion / 100;
    quint8 minor = numericVersion % 100;
    return qMakePair(major, minor);
}

QString clientVersionToString(quint32 numericVersion)
{
    return formatNumericVersion(numericVersion);
}

bool isValidOtbVersion(quint32 otbVersion)
{
    QList<quint32> knownVersions = getKnownOtbVersions();
    return knownVersions.contains(otbVersion);
}

QString otbVersionToString(quint32 otbVersion)
{
    return QString("OTB %1").arg(otbVersion);
}

QList<quint32> getKnownOtbVersions()
{
    return QList<quint32>() << 770 << 860;
}

quint32 calculateDatSignature(const QString& datPath)
{
    QFile file(datPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return 0;
    }
    
    // Read DAT header for signature calculation
    QByteArray header = file.read(32);
    if (header.size() < 4) {
        return 0;
    }
    
    return qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(header.constData()));
}

quint32 calculateSprSignature(const QString& sprPath)
{
    QFile file(sprPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return 0;
    }
    
    // Read SPR header for signature calculation
    QByteArray header = file.read(32);
    if (header.size() < 4) {
        return 0;
    }
    
    return qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(header.constData()));
}

bool validateFileSignature(const QString& filePath, quint32 expectedSignature)
{
    QString extension = QFileInfo(filePath).suffix().toLower();
    
    quint32 actualSignature = 0;
    if (extension == "dat") {
        actualSignature = calculateDatSignature(filePath);
    } else if (extension == "spr") {
        actualSignature = calculateSprSignature(filePath);
    } else {
        return false;
    }
    
    return actualSignature == expectedSignature;
}

QStringList getVersionDetectionMethods()
{
    return QStringList() << "File Signatures" << "File Headers" << "File Size Analysis" << "Path Analysis";
}

qreal calculateDetectionConfidence(const QMap<QString, QVariant>& detectionData)
{
    Q_UNUSED(detectionData)
    
    // This would calculate confidence based on detection data
    // For now, return a default value
    return 0.8;
}

bool isValidMigrationPath(const QStringList& versionPath)
{
    return versionPath.size() >= 2;
}

QStringList optimizeMigrationPath(const QStringList& versionPath)
{
    // This would optimize the migration path
    // For now, return the original path
    return versionPath;
}

QString formatVersionError(const QString& operation, const QString& version, const QString& error)
{
    return QString("%1 failed for version '%2': %3").arg(operation, version, error);
}

} // namespace VersionUtils

} // namespace PluginInterface