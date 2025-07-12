#include "plugins/pluginsecurity.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QProcess>
#include <QThread>
#include <QDateTime>

namespace PluginInterface {

// ============================================================================
// SecurityContext Implementation
// ============================================================================

SecurityContext::SecurityContext(const QString& pluginId)
    : m_pluginId(pluginId)
    , m_securityLevel(5) // Default medium security level
    , m_memoryLimit(100 * 1024 * 1024) // 100MB default
    , m_cpuTimeLimit(5000) // 5 seconds default
    , m_fileAccessLimit(100) // 100 files default
    , m_networkRequestLimit(50) // 50 requests default
    , m_sandboxEnabled(true) // Sandbox enabled by default
{
}

void SecurityContext::grantPermission(SecurityPermission permission)
{
    m_grantedPermissions.insert(permission);
}

void SecurityContext::revokePermission(SecurityPermission permission)
{
    m_grantedPermissions.remove(permission);
}

bool SecurityContext::hasPermission(SecurityPermission permission) const
{
    return m_grantedPermissions.contains(permission);
}

QSet<SecurityPermission> SecurityContext::getGrantedPermissions() const
{
    return m_grantedPermissions;
}

void SecurityContext::setSecurityLevel(int level)
{
    if (level >= 0 && level <= 10) {
        m_securityLevel = level;
    }
}

int SecurityContext::getSecurityLevel() const
{
    return m_securityLevel;
}

void SecurityContext::setMemoryLimit(qint64 bytes)
{
    if (bytes > 0) {
        m_memoryLimit = bytes;
    }
}

void SecurityContext::setCpuTimeLimit(int milliseconds)
{
    if (milliseconds > 0) {
        m_cpuTimeLimit = milliseconds;
    }
}

void SecurityContext::setFileAccessLimit(int maxFiles)
{
    if (maxFiles > 0) {
        m_fileAccessLimit = maxFiles;
    }
}

void SecurityContext::setNetworkRequestLimit(int maxRequests)
{
    if (maxRequests > 0) {
        m_networkRequestLimit = maxRequests;
    }
}

qint64 SecurityContext::getMemoryLimit() const
{
    return m_memoryLimit;
}

int SecurityContext::getCpuTimeLimit() const
{
    return m_cpuTimeLimit;
}

int SecurityContext::getFileAccessLimit() const
{
    return m_fileAccessLimit;
}

int SecurityContext::getNetworkRequestLimit() const
{
    return m_networkRequestLimit;
}

void SecurityContext::setSandboxEnabled(bool enabled)
{
    m_sandboxEnabled = enabled;
}

bool SecurityContext::isSandboxEnabled() const
{
    return m_sandboxEnabled;
}

void SecurityContext::setAllowedPaths(const QStringList& paths)
{
    m_allowedPaths = paths;
}

QStringList SecurityContext::getAllowedPaths() const
{
    return m_allowedPaths;
}

void SecurityContext::setAllowedDomains(const QStringList& domains)
{
    m_allowedDomains = domains;
}

QStringList SecurityContext::getAllowedDomains() const
{
    return m_allowedDomains;
}

QString SecurityContext::getPluginId() const
{
    return m_pluginId;
}

void SecurityContext::setPluginId(const QString& id)
{
    m_pluginId = id;
}

bool SecurityContext::isValid() const
{
    m_validationErrors.clear();
    
    if (m_pluginId.isEmpty()) {
        m_validationErrors.append("Plugin ID cannot be empty");
    }
    
    if (m_securityLevel < 0 || m_securityLevel > 10) {
        m_validationErrors.append("Security level must be between 0 and 10");
    }
    
    if (m_memoryLimit <= 0) {
        m_validationErrors.append("Memory limit must be positive");
    }
    
    if (m_cpuTimeLimit <= 0) {
        m_validationErrors.append("CPU time limit must be positive");
    }
    
    if (m_fileAccessLimit <= 0) {
        m_validationErrors.append("File access limit must be positive");
    }
    
    if (m_networkRequestLimit <= 0) {
        m_validationErrors.append("Network request limit must be positive");
    }
    
    return m_validationErrors.isEmpty();
}

QStringList SecurityContext::getValidationErrors() const
{
    return m_validationErrors;
}

// ============================================================================
// SecurityPolicy Implementation
// ============================================================================

SecurityPolicy::SecurityPolicy(const QString& name)
    : m_name(name)
    , m_defaultMemoryLimit(100 * 1024 * 1024) // 100MB
    , m_defaultCpuTimeLimit(5000) // 5 seconds
    , m_defaultFileAccessLimit(100) // 100 files
    , m_defaultNetworkRequestLimit(50) // 50 requests
    , m_sandboxRequired(true)
    , m_signatureRequired(false)
{
    if (m_name.isEmpty()) {
        m_name = "Default Security Policy";
    }
}

QString SecurityPolicy::getName() const
{
    return m_name;
}

void SecurityPolicy::setName(const QString& name)
{
    m_name = name;
}

QString SecurityPolicy::getDescription() const
{
    return m_description;
}

void SecurityPolicy::setDescription(const QString& description)
{
    m_description = description;
}

void SecurityPolicy::addPermissionRule(const QString& pluginPattern, SecurityPermission permission, bool allow)
{
    PermissionRule rule;
    rule.pluginPattern = pluginPattern;
    rule.permission = permission;
    rule.allow = allow;
    
    // Remove existing rule for same pattern and permission
    for (auto it = m_permissionRules.begin(); it != m_permissionRules.end(); ++it) {
        if (it->pluginPattern == pluginPattern && it->permission == permission) {
            m_permissionRules.erase(it);
            break;
        }
    }
    
    m_permissionRules.append(rule);
}

void SecurityPolicy::removePermissionRule(const QString& pluginPattern, SecurityPermission permission)
{
    for (auto it = m_permissionRules.begin(); it != m_permissionRules.end(); ++it) {
        if (it->pluginPattern == pluginPattern && it->permission == permission) {
            m_permissionRules.erase(it);
            break;
        }
    }
}

bool SecurityPolicy::isPermissionAllowed(const QString& pluginId, SecurityPermission permission) const
{
    // Check rules in reverse order (last rule wins)
    for (auto it = m_permissionRules.rbegin(); it != m_permissionRules.rend(); ++it) {
        if (it->permission == permission) {
            QRegularExpression regex(it->pluginPattern);
            if (regex.match(pluginId).hasMatch()) {
                return it->allow;
            }
        }
    }
    
    // Default deny for administrative permissions, allow for basic permissions
    switch (permission) {
        case SecurityPermission::ADMIN_PLUGIN_MANAGEMENT:
        case SecurityPermission::ADMIN_SECURITY_POLICY:
        case SecurityPermission::ADMIN_SYSTEM_ACCESS:
        case SecurityPermission::SYSTEM_REGISTRY:
        case SecurityPermission::SYSTEM_PROCESS:
        case SecurityPermission::FILE_WRITE_SYSTEM:
        case SecurityPermission::FILE_DELETE:
        case SecurityPermission::FILE_EXECUTE:
            return false;
        default:
            return true;
    }
}

void SecurityPolicy::setDefaultMemoryLimit(qint64 bytes)
{
    if (bytes > 0) {
        m_defaultMemoryLimit = bytes;
    }
}

void SecurityPolicy::setDefaultCpuTimeLimit(int milliseconds)
{
    if (milliseconds > 0) {
        m_defaultCpuTimeLimit = milliseconds;
    }
}

void SecurityPolicy::setDefaultFileAccessLimit(int maxFiles)
{
    if (maxFiles > 0) {
        m_defaultFileAccessLimit = maxFiles;
    }
}

void SecurityPolicy::setDefaultNetworkRequestLimit(int maxRequests)
{
    if (maxRequests > 0) {
        m_defaultNetworkRequestLimit = maxRequests;
    }
}

qint64 SecurityPolicy::getDefaultMemoryLimit() const
{
    return m_defaultMemoryLimit;
}

int SecurityPolicy::getDefaultCpuTimeLimit() const
{
    return m_defaultCpuTimeLimit;
}

int SecurityPolicy::getDefaultFileAccessLimit() const
{
    return m_defaultFileAccessLimit;
}

int SecurityPolicy::getDefaultNetworkRequestLimit() const
{
    return m_defaultNetworkRequestLimit;
}

void SecurityPolicy::setSandboxRequired(bool required)
{
    m_sandboxRequired = required;
}

bool SecurityPolicy::isSandboxRequired() const
{
    return m_sandboxRequired;
}

void SecurityPolicy::setAllowedBasePaths(const QStringList& paths)
{
    m_allowedBasePaths = paths;
}

QStringList SecurityPolicy::getAllowedBasePaths() const
{
    return m_allowedBasePaths;
}

void SecurityPolicy::setAllowedDomains(const QStringList& domains)
{
    m_allowedDomains = domains;
}

QStringList SecurityPolicy::getAllowedDomains() const
{
    return m_allowedDomains;
}

void SecurityPolicy::setAllowedPluginTypes(const QStringList& types)
{
    m_allowedPluginTypes = types;
}

QStringList SecurityPolicy::getAllowedPluginTypes() const
{
    return m_allowedPluginTypes;
}

void SecurityPolicy::setBlockedPluginPatterns(const QStringList& patterns)
{
    m_blockedPluginPatterns = patterns;
}

QStringList SecurityPolicy::getBlockedPluginPatterns() const
{
    return m_blockedPluginPatterns;
}

void SecurityPolicy::setSignatureRequired(bool required)
{
    m_signatureRequired = required;
}

bool SecurityPolicy::isSignatureRequired() const
{
    return m_signatureRequired;
}

void SecurityPolicy::setTrustedSigners(const QStringList& signers)
{
    m_trustedSigners = signers;
}

QStringList SecurityPolicy::getTrustedSigners() const
{
    return m_trustedSigners;
}

bool SecurityPolicy::isValid() const
{
    m_validationErrors.clear();
    
    if (m_name.isEmpty()) {
        m_validationErrors.append("Policy name cannot be empty");
    }
    
    if (m_defaultMemoryLimit <= 0) {
        m_validationErrors.append("Default memory limit must be positive");
    }
    
    if (m_defaultCpuTimeLimit <= 0) {
        m_validationErrors.append("Default CPU time limit must be positive");
    }
    
    if (m_defaultFileAccessLimit <= 0) {
        m_validationErrors.append("Default file access limit must be positive");
    }
    
    if (m_defaultNetworkRequestLimit <= 0) {
        m_validationErrors.append("Default network request limit must be positive");
    }
    
    return m_validationErrors.isEmpty();
}

QStringList SecurityPolicy::getValidationErrors() const
{
    return m_validationErrors;
}

SecurityContext SecurityPolicy::createSecurityContext(const QString& pluginId) const
{
    SecurityContext context(pluginId);
    
    // Apply default limits
    context.setMemoryLimit(m_defaultMemoryLimit);
    context.setCpuTimeLimit(m_defaultCpuTimeLimit);
    context.setFileAccessLimit(m_defaultFileAccessLimit);
    context.setNetworkRequestLimit(m_defaultNetworkRequestLimit);
    
    // Apply sandbox settings
    context.setSandboxEnabled(m_sandboxRequired);
    context.setAllowedPaths(m_allowedBasePaths);
    context.setAllowedDomains(m_allowedDomains);
    
    // Apply permissions based on rules
    QList<SecurityPermission> allPermissions = {
        SecurityPermission::FILE_READ_SYSTEM,
        SecurityPermission::FILE_READ_USER,
        SecurityPermission::FILE_READ_TEMP,
        SecurityPermission::FILE_WRITE_SYSTEM,
        SecurityPermission::FILE_WRITE_USER,
        SecurityPermission::FILE_WRITE_TEMP,
        SecurityPermission::FILE_DELETE,
        SecurityPermission::FILE_EXECUTE,
        SecurityPermission::NETWORK_HTTP_GET,
        SecurityPermission::NETWORK_HTTP_POST,
        SecurityPermission::NETWORK_SOCKET,
        SecurityPermission::NETWORK_DNS,
        SecurityPermission::SYSTEM_REGISTRY,
        SecurityPermission::SYSTEM_ENVIRONMENT,
        SecurityPermission::SYSTEM_PROCESS,
        SecurityPermission::SYSTEM_LIBRARY,
        SecurityPermission::APP_CONFIG_READ,
        SecurityPermission::APP_CONFIG_WRITE,
        SecurityPermission::APP_DATA_READ,
        SecurityPermission::APP_DATA_WRITE,
        SecurityPermission::APP_UI_ACCESS,
        SecurityPermission::PLUGIN_COMMUNICATION,
        SecurityPermission::PLUGIN_DISCOVERY,
        SecurityPermission::RESOURCE_MEMORY_HIGH,
        SecurityPermission::RESOURCE_CPU_HIGH,
        SecurityPermission::RESOURCE_DISK_HIGH,
        SecurityPermission::ADMIN_PLUGIN_MANAGEMENT,
        SecurityPermission::ADMIN_SECURITY_POLICY,
        SecurityPermission::ADMIN_SYSTEM_ACCESS
    };
    
    for (SecurityPermission permission : allPermissions) {
        if (isPermissionAllowed(pluginId, permission)) {
            context.grantPermission(permission);
        }
    }
    
    return context;
}

bool SecurityPolicy::isPluginAllowed(const QString& pluginId, const QString& pluginType) const
{
    // Check blocked patterns first
    for (const QString& pattern : m_blockedPluginPatterns) {
        QRegularExpression regex(pattern);
        if (regex.match(pluginId).hasMatch()) {
            return false;
        }
    }
    
    // Check allowed types
    if (!m_allowedPluginTypes.isEmpty() && !m_allowedPluginTypes.contains(pluginType)) {
        return false;
    }
    
    return true;
}// ============================================================================
// PluginSandbox Implementation
// ============================================================================

PluginSandbox::PluginSandbox(const SecurityContext& context, QObject* parent)
    : QObject(parent)
    , m_context(context)
    , m_initialized(false)
    , m_resourceTimer(nullptr)
    , m_currentMemoryUsage(0)
    , m_currentCpuTime(0)
    , m_currentFileAccessCount(0)
    , m_currentNetworkRequestCount(0)
    , m_violationCount(0)
{
}

PluginSandbox::~PluginSandbox()
{
    shutdown();
}

bool PluginSandbox::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    if (!m_context.isValid()) {
        qWarning() << "Cannot initialize sandbox with invalid security context";
        return false;
    }
    
    // Initialize resource monitoring
    if (m_context.isSandboxEnabled()) {
        startResourceMonitoring();
    }
    
    m_initialized = true;
    qDebug() << "Plugin sandbox initialized for plugin:" << m_context.getPluginId();
    
    return true;
}

void PluginSandbox::shutdown()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        return;
    }
    
    stopResourceMonitoring();
    m_initialized = false;
    
    emit sandboxShutdown("Normal shutdown");
    qDebug() << "Plugin sandbox shutdown for plugin:" << m_context.getPluginId();
}

bool PluginSandbox::isInitialized() const
{
    QMutexLocker locker(&m_mutex);
    return m_initialized;
}

SecurityContext PluginSandbox::getSecurityContext() const
{
    QMutexLocker locker(&m_mutex);
    return m_context;
}

void PluginSandbox::updateSecurityContext(const SecurityContext& context)
{
    QMutexLocker locker(&m_mutex);
    m_context = context;
    
    if (m_initialized && m_context.isSandboxEnabled()) {
        updateResourceLimits();
    }
}

bool PluginSandbox::isFileAccessAllowed(const QString& filePath, SecurityPermission permission) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_context.hasPermission(permission)) {
        return false;
    }
    
    if (!m_context.isSandboxEnabled()) {
        return true;
    }
    
    return isPathAllowed(filePath);
}

bool PluginSandbox::isDirectoryAccessAllowed(const QString& dirPath, SecurityPermission permission) const
{
    return isFileAccessAllowed(dirPath, permission);
}

QStringList PluginSandbox::getAccessiblePaths() const
{
    QMutexLocker locker(&m_mutex);
    return m_context.getAllowedPaths();
}

bool PluginSandbox::isNetworkAccessAllowed(const QString& url, SecurityPermission permission) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_context.hasPermission(permission)) {
        return false;
    }
    
    if (!m_context.isSandboxEnabled()) {
        return true;
    }
    
    QUrl parsedUrl(url);
    return isDomainAllowed(parsedUrl.host());
}

bool PluginSandbox::isDomainAllowed(const QString& domain) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_context.isSandboxEnabled()) {
        return true;
    }
    
    QStringList allowedDomains = m_context.getAllowedDomains();
    if (allowedDomains.isEmpty()) {
        return true; // No restrictions
    }
    
    for (const QString& allowedDomain : allowedDomains) {
        if (isPatternMatched(domain, allowedDomain)) {
            return true;
        }
    }
    
    return false;
}

QStringList PluginSandbox::getAllowedDomains() const
{
    QMutexLocker locker(&m_mutex);
    return m_context.getAllowedDomains();
}

qint64 PluginSandbox::getCurrentMemoryUsage() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentMemoryUsage;
}

int PluginSandbox::getCurrentCpuTime() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentCpuTime;
}

int PluginSandbox::getCurrentFileAccessCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentFileAccessCount;
}

int PluginSandbox::getCurrentNetworkRequestCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentNetworkRequestCount;
}

bool PluginSandbox::isMemoryLimitExceeded() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentMemoryUsage > m_context.getMemoryLimit();
}

bool PluginSandbox::isCpuTimeLimitExceeded() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentCpuTime > m_context.getCpuTimeLimit();
}

bool PluginSandbox::isFileAccessLimitExceeded() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentFileAccessCount > m_context.getFileAccessLimit();
}

bool PluginSandbox::isNetworkRequestLimitExceeded() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentNetworkRequestCount > m_context.getNetworkRequestLimit();
}

bool PluginSandbox::validateOperation(const QString& operation, const QVariantMap& parameters) const
{
    QMutexLocker locker(&m_mutex);
    
    // Basic operation validation
    if (operation.isEmpty()) {
        return false;
    }
    
    // Check if sandbox is enabled
    if (!m_context.isSandboxEnabled()) {
        return true;
    }
    
    // Validate specific operations
    if (operation == "file_read" || operation == "file_write") {
        QString filePath = parameters.value("path").toString();
        SecurityPermission permission = operation == "file_read" ? 
            SecurityPermission::FILE_READ_USER : SecurityPermission::FILE_WRITE_USER;
        return isFileAccessAllowed(filePath, permission);
    }
    
    if (operation == "network_request") {
        QString url = parameters.value("url").toString();
        SecurityPermission permission = SecurityPermission::NETWORK_HTTP_GET;
        return isNetworkAccessAllowed(url, permission);
    }
    
    return true;
}

bool PluginSandbox::requestPermission(SecurityPermission permission, const QString& reason)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_context.hasPermission(permission)) {
        return true;
    }
    
    emit permissionRequested(permission, reason);
    
    // For now, return false. In a real implementation, this would
    // wait for user response or policy decision
    return false;
}

void PluginSandbox::reportViolation(const QString& violation, const QVariantMap& details)
{
    QMutexLocker locker(&m_mutex);
    recordViolation(violation, details);
    emit violationDetected(violation, details);
}

QStringList PluginSandbox::getViolationHistory() const
{
    QMutexLocker locker(&m_mutex);
    return m_violationHistory;
}

int PluginSandbox::getViolationCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_violationCount;
}

void PluginSandbox::resetResourceCounters()
{
    QMutexLocker locker(&m_mutex);
    m_currentMemoryUsage = 0;
    m_currentCpuTime = 0;
    m_currentFileAccessCount = 0;
    m_currentNetworkRequestCount = 0;
}

void PluginSandbox::updateResourceLimits()
{
    // Resource limits are updated through security context
    // This method can be used to apply new limits immediately
    if (m_resourceTimer && m_resourceTimer->isActive()) {
        onResourceMonitorTimer(); // Force immediate check
    }
}

void PluginSandbox::onResourceMonitorTimer()
{
    updateResourceUsage();
    
    // Check for limit violations
    if (isMemoryLimitExceeded()) {
        emit resourceLimitExceeded("memory", m_currentMemoryUsage, m_context.getMemoryLimit());
        reportViolation("Memory limit exceeded", {
            {"current", m_currentMemoryUsage},
            {"limit", m_context.getMemoryLimit()}
        });
    }
    
    if (isCpuTimeLimitExceeded()) {
        emit resourceLimitExceeded("cpu_time", m_currentCpuTime, m_context.getCpuTimeLimit());
        reportViolation("CPU time limit exceeded", {
            {"current", m_currentCpuTime},
            {"limit", m_context.getCpuTimeLimit()}
        });
    }
    
    if (isFileAccessLimitExceeded()) {
        emit resourceLimitExceeded("file_access", m_currentFileAccessCount, m_context.getFileAccessLimit());
        reportViolation("File access limit exceeded", {
            {"current", m_currentFileAccessCount},
            {"limit", m_context.getFileAccessLimit()}
        });
    }
    
    if (isNetworkRequestLimitExceeded()) {
        emit resourceLimitExceeded("network_requests", m_currentNetworkRequestCount, m_context.getNetworkRequestLimit());
        reportViolation("Network request limit exceeded", {
            {"current", m_currentNetworkRequestCount},
            {"limit", m_context.getNetworkRequestLimit()}
        });
    }
}

void PluginSandbox::startResourceMonitoring()
{
    if (!m_resourceTimer) {
        m_resourceTimer = new QTimer(this);
        connect(m_resourceTimer, &QTimer::timeout, this, &PluginSandbox::onResourceMonitorTimer);
    }
    
    m_resourceTimer->start(1000); // Check every second
}

void PluginSandbox::stopResourceMonitoring()
{
    if (m_resourceTimer) {
        m_resourceTimer->stop();
    }
}

void PluginSandbox::updateResourceUsage()
{
    // This is a simplified implementation
    // In a real implementation, you would use platform-specific APIs
    // to get actual resource usage
    
    // For demonstration, we'll simulate resource usage
    static qint64 baseMemory = 10 * 1024 * 1024; // 10MB base
    m_currentMemoryUsage = baseMemory + (qrand() % (5 * 1024 * 1024)); // +0-5MB random
    
    static int baseCpuTime = 100;
    m_currentCpuTime = baseCpuTime + (qrand() % 200); // +0-200ms random
}

bool PluginSandbox::isPathAllowed(const QString& path) const
{
    QStringList allowedPaths = m_context.getAllowedPaths();
    if (allowedPaths.isEmpty()) {
        return true; // No restrictions
    }
    
    QString normalizedPath = QDir::cleanPath(path);
    
    for (const QString& allowedPath : allowedPaths) {
        QString normalizedAllowed = QDir::cleanPath(allowedPath);
        if (normalizedPath.startsWith(normalizedAllowed)) {
            return true;
        }
    }
    
    return false;
}

bool PluginSandbox::isPatternMatched(const QString& text, const QString& pattern) const
{
    QRegularExpression regex(pattern);
    return regex.match(text).hasMatch();
}

void PluginSandbox::recordViolation(const QString& violation, const QVariantMap& details)
{
    QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString violationRecord = QString("[%1] %2").arg(timestamp, violation);
    
    if (!details.isEmpty()) {
        QJsonObject jsonDetails = QJsonObject::fromVariantMap(details);
        QJsonDocument doc(jsonDetails);
        violationRecord += QString(" - Details: %1").arg(doc.toJson(QJsonDocument::Compact));
    }
    
    m_violationHistory.append(violationRecord);
    m_violationCount++;
    
    // Keep only last 100 violations to prevent memory issues
    if (m_violationHistory.size() > 100) {
        m_violationHistory.removeFirst();
    }
}// ============================================================================
// PluginSignatureVerifier Implementation
// ============================================================================

PluginSignatureVerifier::PluginSignatureVerifier(QObject* parent)
    : QObject(parent)
    , m_requireSignature(false)
    , m_allowSelfSigned(true)
{
}

PluginSignatureVerifier::~PluginSignatureVerifier()
{
}

bool PluginSignatureVerifier::verifyPluginSignature(const QString& pluginPath, const QString& signaturePath) const
{
    QMutexLocker locker(&m_mutex);
    
    m_lastErrors.clear();
    m_lastError.clear();
    
    if (!QFileInfo::exists(pluginPath)) {
        m_lastError = "Plugin file does not exist: " + pluginPath;
        m_lastErrors.append(m_lastError);
        return false;
    }
    
    if (!m_requireSignature) {
        return true; // Signature not required
    }
    
    QString sigPath = signaturePath;
    if (sigPath.isEmpty()) {
        sigPath = pluginPath + ".sig";
    }
    
    if (!QFileInfo::exists(sigPath)) {
        m_lastError = "Signature file does not exist: " + sigPath;
        m_lastErrors.append(m_lastError);
        return false;
    }
    
    bool result = verifySignatureInternal(pluginPath, sigPath);
    emit signatureVerified(pluginPath, result);
    
    return result;
}

bool PluginSignatureVerifier::verifyPluginIntegrity(const QString& pluginPath) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!QFileInfo::exists(pluginPath)) {
        m_lastError = "Plugin file does not exist: " + pluginPath;
        return false;
    }
    
    // Calculate current hash
    QString currentHash = calculatePluginHash(pluginPath);
    if (currentHash.isEmpty()) {
        m_lastError = "Failed to calculate plugin hash";
        return false;
    }
    
    // Check if integrity file exists
    QString integrityPath = pluginPath + ".integrity";
    if (!QFileInfo::exists(integrityPath)) {
        // No integrity file, create one for future verification
        QFile integrityFile(integrityPath);
        if (integrityFile.open(QIODevice::WriteOnly)) {
            integrityFile.write(currentHash.toUtf8());
            integrityFile.close();
        }
        return true; // First time, assume valid
    }
    
    // Read stored hash
    QFile integrityFile(integrityPath);
    if (!integrityFile.open(QIODevice::ReadOnly)) {
        m_lastError = "Failed to read integrity file";
        return false;
    }
    
    QString storedHash = QString::fromUtf8(integrityFile.readAll()).trimmed();
    integrityFile.close();
    
    bool valid = (currentHash == storedHash);
    if (!valid) {
        m_lastError = "Plugin integrity check failed - hash mismatch";
    }
    
    return valid;
}

QString PluginSignatureVerifier::calculatePluginHash(const QString& pluginPath) const
{
    QFile file(pluginPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(&file);
    
    return hash.result().toHex();
}

bool PluginSignatureVerifier::addTrustedCertificate(const QString& certificatePath)
{
    QMutexLocker locker(&m_mutex);
    
    if (!QFileInfo::exists(certificatePath)) {
        m_lastError = "Certificate file does not exist: " + certificatePath;
        return false;
    }
    
    QString certificateId = extractCertificateId(certificatePath);
    if (certificateId.isEmpty()) {
        m_lastError = "Failed to extract certificate ID";
        return false;
    }
    
    if (!m_trustedCertificates.contains(certificateId)) {
        m_trustedCertificates.append(certificateId);
        emit certificateAdded(certificateId);
    }
    
    return true;
}

bool PluginSignatureVerifier::removeTrustedCertificate(const QString& certificateId)
{
    QMutexLocker locker(&m_mutex);
    
    bool removed = m_trustedCertificates.removeOne(certificateId);
    if (removed) {
        emit certificateRemoved(certificateId);
    }
    
    return removed;
}

QStringList PluginSignatureVerifier::getTrustedCertificates() const
{
    QMutexLocker locker(&m_mutex);
    return m_trustedCertificates;
}

bool PluginSignatureVerifier::isCertificateTrusted(const QString& certificateId) const
{
    QMutexLocker locker(&m_mutex);
    return m_trustedCertificates.contains(certificateId);
}

bool PluginSignatureVerifier::signPlugin(const QString& pluginPath, const QString& privateKeyPath, const QString& certificatePath)
{
    QMutexLocker locker(&m_mutex);
    
    // This is a simplified implementation
    // In a real implementation, you would use proper cryptographic libraries
    
    if (!QFileInfo::exists(pluginPath)) {
        m_lastError = "Plugin file does not exist: " + pluginPath;
        return false;
    }
    
    if (!QFileInfo::exists(privateKeyPath)) {
        m_lastError = "Private key file does not exist: " + privateKeyPath;
        return false;
    }
    
    if (!QFileInfo::exists(certificatePath)) {
        m_lastError = "Certificate file does not exist: " + certificatePath;
        return false;
    }
    
    QString signaturePath = pluginPath + ".sig";
    return createSignatureFile(pluginPath, signaturePath, privateKeyPath);
}

bool PluginSignatureVerifier::createSignatureFile(const QString& pluginPath, const QString& signaturePath, const QString& privateKeyPath)
{
    // Simplified signature creation
    // In a real implementation, you would use proper digital signatures
    
    QString pluginHash = calculatePluginHash(pluginPath);
    if (pluginHash.isEmpty()) {
        m_lastError = "Failed to calculate plugin hash for signing";
        return false;
    }
    
    // Create a simple signature file (this is not cryptographically secure)
    QFile signatureFile(signaturePath);
    if (!signatureFile.open(QIODevice::WriteOnly)) {
        m_lastError = "Failed to create signature file: " + signaturePath;
        return false;
    }
    
    QJsonObject signature;
    signature["plugin_hash"] = pluginHash;
    signature["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    signature["signer"] = "ItemEditor Plugin System";
    signature["version"] = "1.0";
    
    QJsonDocument doc(signature);
    signatureFile.write(doc.toJson());
    signatureFile.close();
    
    return true;
}

void PluginSignatureVerifier::setRequireSignature(bool required)
{
    QMutexLocker locker(&m_mutex);
    m_requireSignature = required;
}

bool PluginSignatureVerifier::isSignatureRequired() const
{
    QMutexLocker locker(&m_mutex);
    return m_requireSignature;
}

void PluginSignatureVerifier::setAllowSelfSigned(bool allow)
{
    QMutexLocker locker(&m_mutex);
    m_allowSelfSigned = allow;
}

bool PluginSignatureVerifier::isSelfSignedAllowed() const
{
    QMutexLocker locker(&m_mutex);
    return m_allowSelfSigned;
}

QStringList PluginSignatureVerifier::getLastVerificationErrors() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastErrors;
}

QString PluginSignatureVerifier::getLastError() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastError;
}

bool PluginSignatureVerifier::verifySignatureInternal(const QString& pluginPath, const QString& signaturePath) const
{
    // Simplified signature verification
    // In a real implementation, you would use proper cryptographic verification
    
    QFile signatureFile(signaturePath);
    if (!signatureFile.open(QIODevice::ReadOnly)) {
        m_lastError = "Failed to read signature file";
        return false;
    }
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(signatureFile.readAll(), &parseError);
    signatureFile.close();
    
    if (parseError.error != QJsonParseError::NoError) {
        m_lastError = "Invalid signature file format: " + parseError.errorString();
        return false;
    }
    
    QJsonObject signature = doc.object();
    QString storedHash = signature["plugin_hash"].toString();
    
    if (storedHash.isEmpty()) {
        m_lastError = "No plugin hash found in signature";
        return false;
    }
    
    QString currentHash = calculatePluginHash(pluginPath);
    if (currentHash != storedHash) {
        m_lastError = "Plugin hash does not match signature";
        return false;
    }
    
    // Additional verification could be done here
    // For now, we consider it valid if the hash matches
    return true;
}

bool PluginSignatureVerifier::validateCertificateChain(const QString& certificatePath) const
{
    // Simplified certificate validation
    // In a real implementation, you would validate the full certificate chain
    
    if (!QFileInfo::exists(certificatePath)) {
        return false;
    }
    
    QString certificateId = extractCertificateId(certificatePath);
    return isCertificateTrusted(certificateId);
}

QString PluginSignatureVerifier::extractCertificateId(const QString& certificatePath) const
{
    // Simplified certificate ID extraction
    // In a real implementation, you would parse the actual certificate
    
    QFileInfo fileInfo(certificatePath);
    return fileInfo.baseName(); // Use filename as ID for simplicity
}// ============================================================================
// PluginSecurityManager Implementation
// ============================================================================

PluginSecurityManager::PluginSecurityManager(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
    , m_signatureVerifier(nullptr)
    , m_strictMode(false)
    , m_autoSandbox(true)
    , m_securityMonitoringActive(false)
    , m_securedPluginCount(0)
{
    m_signatureVerifier = new PluginSignatureVerifier(this);
    
    // Connect signature verifier signals
    connect(m_signatureVerifier, &PluginSignatureVerifier::signatureVerified,
            this, &PluginSecurityManager::onSignatureVerified);
}

PluginSecurityManager::~PluginSecurityManager()
{
    shutdown();
}

bool PluginSecurityManager::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    // Initialize default security policy
    initializeDefaultPolicy();
    
    // Load default trusted certificates
    loadDefaultTrustedCertificates();
    
    // Validate configuration
    if (!validateSecurityConfiguration()) {
        qWarning() << "Security configuration validation failed";
        return false;
    }
    
    m_initialized = true;
    qDebug() << "Plugin security manager initialized";
    
    return true;
}

void PluginSecurityManager::shutdown()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_initialized) {
        return;
    }
    
    // Stop security monitoring
    stopSecurityMonitoring();
    
    // Destroy all sandboxes
    for (auto it = m_sandboxes.begin(); it != m_sandboxes.end(); ++it) {
        delete it.value();
    }
    m_sandboxes.clear();
    
    m_initialized = false;
    qDebug() << "Plugin security manager shutdown";
}

bool PluginSecurityManager::isInitialized() const
{
    QMutexLocker locker(&m_mutex);
    return m_initialized;
}

void PluginSecurityManager::setSecurityPolicy(const SecurityPolicy& policy)
{
    QMutexLocker locker(&m_mutex);
    m_securityPolicy = policy;
    emit securityPolicyChanged();
}

SecurityPolicy PluginSecurityManager::getSecurityPolicy() const
{
    QMutexLocker locker(&m_mutex);
    return m_securityPolicy;
}

bool PluginSecurityManager::loadSecurityPolicy(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open security policy file:" << filePath;
        return false;
    }
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();
    
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse security policy:" << parseError.errorString();
        return false;
    }
    
    // Convert JSON to SecurityPolicy
    SecurityPolicy policy = SecurityUtils::securityPolicyFromMap(doc.object().toVariantMap());
    
    if (!policy.isValid()) {
        qWarning() << "Invalid security policy:" << policy.getValidationErrors();
        return false;
    }
    
    setSecurityPolicy(policy);
    return true;
}

bool PluginSecurityManager::saveSecurityPolicy(const QString& filePath) const
{
    QMutexLocker locker(&m_mutex);
    
    QVariantMap policyMap = SecurityUtils::securityPolicyToMap(m_securityPolicy);
    QJsonDocument doc = QJsonDocument::fromVariant(policyMap);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to create security policy file:" << filePath;
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    return true;
}

bool PluginSecurityManager::validatePlugin(const QString& pluginPath) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!QFileInfo::exists(pluginPath)) {
        return false;
    }
    
    // Check signature if required
    if (m_securityPolicy.isSignatureRequired()) {
        if (!m_signatureVerifier->verifyPluginSignature(pluginPath)) {
            emit signatureVerificationFailed(pluginPath, m_signatureVerifier->getLastError());
            return false;
        }
    }
    
    // Verify integrity
    if (!m_signatureVerifier->verifyPluginIntegrity(pluginPath)) {
        return false;
    }
    
    // Additional validation can be added here
    
    emit pluginValidated(pluginPath, true);
    return true;
}

bool PluginSecurityManager::validatePluginSecurity(IPlugin* plugin) const
{
    if (!plugin) {
        return false;
    }
    
    QString pluginName = plugin->pluginName();
    QString pluginVersion = plugin->pluginVersion();
    
    // Check if plugin is allowed by policy
    if (!m_securityPolicy.isPluginAllowed(pluginName, "IPlugin")) {
        qWarning() << "Plugin not allowed by security policy:" << pluginName;
        return false;
    }
    
    return true;
}

SecurityContext PluginSecurityManager::createSecurityContext(const QString& pluginId) const
{
    QMutexLocker locker(&m_mutex);
    return m_securityPolicy.createSecurityContext(pluginId);
}

PluginSandbox* PluginSecurityManager::createSandbox(const QString& pluginId)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_sandboxes.contains(pluginId)) {
        return m_sandboxes[pluginId]; // Already exists
    }
    
    SecurityContext context = createSecurityContext(pluginId);
    PluginSandbox* sandbox = new PluginSandbox(context, this);
    
    // Connect sandbox signals
    connect(sandbox, &PluginSandbox::violationDetected,
            this, &PluginSecurityManager::onSandboxViolation);
    connect(sandbox, &PluginSandbox::permissionRequested,
            this, &PluginSecurityManager::onPermissionRequested);
    
    if (!sandbox->initialize()) {
        delete sandbox;
        return nullptr;
    }
    
    m_sandboxes[pluginId] = sandbox;
    emit sandboxCreated(pluginId);
    
    return sandbox;
}

PluginSandbox* PluginSecurityManager::getSandbox(const QString& pluginId) const
{
    QMutexLocker locker(&m_mutex);
    return m_sandboxes.value(pluginId, nullptr);
}

void PluginSecurityManager::destroySandbox(const QString& pluginId)
{
    QMutexLocker locker(&m_mutex);
    
    PluginSandbox* sandbox = m_sandboxes.value(pluginId, nullptr);
    if (sandbox) {
        sandbox->shutdown();
        m_sandboxes.remove(pluginId);
        delete sandbox;
        emit sandboxDestroyed(pluginId);
    }
}

QStringList PluginSecurityManager::getActiveSandboxes() const
{
    QMutexLocker locker(&m_mutex);
    return m_sandboxes.keys();
}

bool PluginSecurityManager::checkPermission(const QString& pluginId, SecurityPermission permission) const
{
    QMutexLocker locker(&m_mutex);
    return m_securityPolicy.isPermissionAllowed(pluginId, permission);
}

bool PluginSecurityManager::requestPermission(const QString& pluginId, SecurityPermission permission, const QString& reason)
{
    QMutexLocker locker(&m_mutex);
    
    if (checkPermission(pluginId, permission)) {
        return true;
    }
    
    emit permissionRequested(pluginId, permission, reason);
    
    // In a real implementation, this would wait for user response
    // For now, return false (permission denied)
    return false;
}

void PluginSecurityManager::grantPermission(const QString& pluginId, SecurityPermission permission)
{
    QMutexLocker locker(&m_mutex);
    
    // Add permission rule to policy
    m_securityPolicy.addPermissionRule(pluginId, permission, true);
    
    // Update sandbox if exists
    PluginSandbox* sandbox = m_sandboxes.value(pluginId, nullptr);
    if (sandbox) {
        SecurityContext context = sandbox->getSecurityContext();
        context.grantPermission(permission);
        sandbox->updateSecurityContext(context);
    }
    
    emit permissionGranted(pluginId, permission);
}

void PluginSecurityManager::revokePermission(const QString& pluginId, SecurityPermission permission)
{
    QMutexLocker locker(&m_mutex);
    
    // Add deny rule to policy
    m_securityPolicy.addPermissionRule(pluginId, permission, false);
    
    // Update sandbox if exists
    PluginSandbox* sandbox = m_sandboxes.value(pluginId, nullptr);
    if (sandbox) {
        SecurityContext context = sandbox->getSecurityContext();
        context.revokePermission(permission);
        sandbox->updateSecurityContext(context);
    }
    
    emit permissionRevoked(pluginId, permission);
}

PluginSignatureVerifier* PluginSecurityManager::getSignatureVerifier() const
{
    return m_signatureVerifier;
}

bool PluginSecurityManager::verifyPluginSignature(const QString& pluginPath) const
{
    return m_signatureVerifier->verifyPluginSignature(pluginPath);
}

void PluginSecurityManager::startSecurityMonitoring()
{
    QMutexLocker locker(&m_mutex);
    m_securityMonitoringActive = true;
    
    // Start monitoring all active sandboxes
    for (PluginSandbox* sandbox : m_sandboxes) {
        // Monitoring is already active in individual sandboxes
    }
}

void PluginSecurityManager::stopSecurityMonitoring()
{
    QMutexLocker locker(&m_mutex);
    m_securityMonitoringActive = false;
}

bool PluginSecurityManager::isSecurityMonitoringActive() const
{
    QMutexLocker locker(&m_mutex);
    return m_securityMonitoringActive;
}

QStringList PluginSecurityManager::getSecurityViolations() const
{
    QMutexLocker locker(&m_mutex);
    return m_securityViolations;
}

int PluginSecurityManager::getViolationCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_securityViolations.size();
}

void PluginSecurityManager::clearViolationHistory()
{
    QMutexLocker locker(&m_mutex);
    m_securityViolations.clear();
}

void PluginSecurityManager::setStrictMode(bool strict)
{
    QMutexLocker locker(&m_mutex);
    m_strictMode = strict;
}

bool PluginSecurityManager::isStrictMode() const
{
    QMutexLocker locker(&m_mutex);
    return m_strictMode;
}

void PluginSecurityManager::setAutoSandbox(bool autoSandbox)
{
    QMutexLocker locker(&m_mutex);
    m_autoSandbox = autoSandbox;
}

bool PluginSecurityManager::isAutoSandbox() const
{
    QMutexLocker locker(&m_mutex);
    return m_autoSandbox;
}

int PluginSecurityManager::getSecuredPluginCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_securedPluginCount;
}

int PluginSecurityManager::getActiveSandboxCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_sandboxes.size();
}

QMap<QString, QVariant> PluginSecurityManager::getSecurityStatistics() const
{
    QMutexLocker locker(&m_mutex);
    
    QMap<QString, QVariant> stats;
    stats["secured_plugins"] = m_securedPluginCount;
    stats["active_sandboxes"] = m_sandboxes.size();
    stats["security_violations"] = m_securityViolations.size();
    stats["monitoring_active"] = m_securityMonitoringActive;
    stats["strict_mode"] = m_strictMode;
    stats["auto_sandbox"] = m_autoSandbox;
    
    return stats;
}

void PluginSecurityManager::refreshSecurityPolicy()
{
    // Reload policy from default location or emit signal for external refresh
    emit securityPolicyChanged();
}

void PluginSecurityManager::validateAllPlugins()
{
    // This would iterate through all known plugins and validate them
    // Implementation depends on how plugins are tracked in the system
}

void PluginSecurityManager::emergencyShutdown()
{
    qWarning() << "Emergency security shutdown initiated";
    
    // Stop all sandboxes immediately
    for (PluginSandbox* sandbox : m_sandboxes) {
        sandbox->shutdown();
    }
    
    // Clear all permissions
    m_securityPolicy = SecurityPolicy("Emergency Policy");
    
    emit securityPolicyChanged();
}

void PluginSecurityManager::onSandboxViolation(const QString& violation, const QVariantMap& details)
{
    PluginSandbox* sandbox = qobject_cast<PluginSandbox*>(sender());
    if (sandbox) {
        QString pluginId = sandbox->getSecurityContext().getPluginId();
        
        QString violationRecord = QString("[%1] %2: %3")
            .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
            .arg(pluginId)
            .arg(violation);
        
        m_securityViolations.append(violationRecord);
        emit securityViolation(pluginId, violation, details);
        
        // In strict mode, shutdown the plugin on any violation
        if (m_strictMode) {
            destroySandbox(pluginId);
        }
    }
}

void PluginSecurityManager::onPermissionRequested(SecurityPermission permission, const QString& reason)
{
    PluginSandbox* sandbox = qobject_cast<PluginSandbox*>(sender());
    if (sandbox) {
        QString pluginId = sandbox->getSecurityContext().getPluginId();
        emit permissionRequested(pluginId, permission, reason);
    }
}

void PluginSecurityManager::onSignatureVerified(const QString& pluginPath, bool valid)
{
    emit pluginValidated(pluginPath, valid);
    
    if (valid) {
        m_securedPluginCount++;
    }
}

void PluginSecurityManager::initializeDefaultPolicy()
{
    m_securityPolicy = SecurityPolicy("Default Security Policy");
    m_securityPolicy.setDescription("Default security policy for ItemEditor plugins");
    
    // Set default resource limits
    m_securityPolicy.setDefaultMemoryLimit(100 * 1024 * 1024); // 100MB
    m_securityPolicy.setDefaultCpuTimeLimit(5000); // 5 seconds
    m_securityPolicy.setDefaultFileAccessLimit(100); // 100 files
    m_securityPolicy.setDefaultNetworkRequestLimit(50); // 50 requests
    
    // Enable sandbox by default
    m_securityPolicy.setSandboxRequired(true);
    
    // Set allowed base paths
    QStringList allowedPaths;
    allowedPaths << QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    allowedPaths << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    allowedPaths << QCoreApplication::applicationDirPath();
    m_securityPolicy.setAllowedBasePaths(allowedPaths);
    
    // Allow basic permissions for all plugins
    m_securityPolicy.addPermissionRule(".*", SecurityPermission::FILE_READ_USER, true);
    m_securityPolicy.addPermissionRule(".*", SecurityPermission::FILE_READ_TEMP, true);
    m_securityPolicy.addPermissionRule(".*", SecurityPermission::FILE_WRITE_TEMP, true);
    m_securityPolicy.addPermissionRule(".*", SecurityPermission::APP_CONFIG_READ, true);
    m_securityPolicy.addPermissionRule(".*", SecurityPermission::APP_DATA_READ, true);
    m_securityPolicy.addPermissionRule(".*", SecurityPermission::PLUGIN_COMMUNICATION, true);
    
    // Deny dangerous permissions by default
    m_securityPolicy.addPermissionRule(".*", SecurityPermission::ADMIN_PLUGIN_MANAGEMENT, false);
    m_securityPolicy.addPermissionRule(".*", SecurityPermission::ADMIN_SECURITY_POLICY, false);
    m_securityPolicy.addPermissionRule(".*", SecurityPermission::ADMIN_SYSTEM_ACCESS, false);
    m_securityPolicy.addPermissionRule(".*", SecurityPermission::SYSTEM_REGISTRY, false);
    m_securityPolicy.addPermissionRule(".*", SecurityPermission::SYSTEM_PROCESS, false);
    m_securityPolicy.addPermissionRule(".*", SecurityPermission::FILE_DELETE, false);
    m_securityPolicy.addPermissionRule(".*", SecurityPermission::FILE_EXECUTE, false);
}

void PluginSecurityManager::loadDefaultTrustedCertificates()
{
    // Load any default trusted certificates
    // This would typically load certificates from a trusted store
    
    QString certDir = QCoreApplication::applicationDirPath() + "/certificates";
    QDir dir(certDir);
    
    if (dir.exists()) {
        QStringList certFiles = dir.entryList(QStringList() << "*.crt" << "*.pem", QDir::Files);
        for (const QString& certFile : certFiles) {
            QString certPath = dir.absoluteFilePath(certFile);
            m_signatureVerifier->addTrustedCertificate(certPath);
        }
    }
}

bool PluginSecurityManager::validateSecurityConfiguration() const
{
    if (!m_securityPolicy.isValid()) {
        qWarning() << "Invalid security policy:" << m_securityPolicy.getValidationErrors();
        return false;
    }
    
    if (!m_signatureVerifier) {
        qWarning() << "Signature verifier not initialized";
        return false;
    }
    
    return true;
}

void PluginSecurityManager::cleanupInactiveSandboxes()
{
    // Remove sandboxes that are no longer active
    // This would be called periodically to clean up resources
    
    for (auto it = m_sandboxes.begin(); it != m_sandboxes.end();) {
        PluginSandbox* sandbox = it.value();
        if (!sandbox->isInitialized()) {
            delete sandbox;
            it = m_sandboxes.erase(it);
        } else {
            ++it;
        }
    }
}// ============================================================================
// SecurityUtils Implementation
// ============================================================================

namespace SecurityUtils {

QString permissionToString(SecurityPermission permission)
{
    switch (permission) {
        case SecurityPermission::FILE_READ_SYSTEM: return "FILE_READ_SYSTEM";
        case SecurityPermission::FILE_READ_USER: return "FILE_READ_USER";
        case SecurityPermission::FILE_READ_TEMP: return "FILE_READ_TEMP";
        case SecurityPermission::FILE_WRITE_SYSTEM: return "FILE_WRITE_SYSTEM";
        case SecurityPermission::FILE_WRITE_USER: return "FILE_WRITE_USER";
        case SecurityPermission::FILE_WRITE_TEMP: return "FILE_WRITE_TEMP";
        case SecurityPermission::FILE_DELETE: return "FILE_DELETE";
        case SecurityPermission::FILE_EXECUTE: return "FILE_EXECUTE";
        case SecurityPermission::NETWORK_HTTP_GET: return "NETWORK_HTTP_GET";
        case SecurityPermission::NETWORK_HTTP_POST: return "NETWORK_HTTP_POST";
        case SecurityPermission::NETWORK_SOCKET: return "NETWORK_SOCKET";
        case SecurityPermission::NETWORK_DNS: return "NETWORK_DNS";
        case SecurityPermission::SYSTEM_REGISTRY: return "SYSTEM_REGISTRY";
        case SecurityPermission::SYSTEM_ENVIRONMENT: return "SYSTEM_ENVIRONMENT";
        case SecurityPermission::SYSTEM_PROCESS: return "SYSTEM_PROCESS";
        case SecurityPermission::SYSTEM_LIBRARY: return "SYSTEM_LIBRARY";
        case SecurityPermission::APP_CONFIG_READ: return "APP_CONFIG_READ";
        case SecurityPermission::APP_CONFIG_WRITE: return "APP_CONFIG_WRITE";
        case SecurityPermission::APP_DATA_READ: return "APP_DATA_READ";
        case SecurityPermission::APP_DATA_WRITE: return "APP_DATA_WRITE";
        case SecurityPermission::APP_UI_ACCESS: return "APP_UI_ACCESS";
        case SecurityPermission::PLUGIN_COMMUNICATION: return "PLUGIN_COMMUNICATION";
        case SecurityPermission::PLUGIN_DISCOVERY: return "PLUGIN_DISCOVERY";
        case SecurityPermission::RESOURCE_MEMORY_HIGH: return "RESOURCE_MEMORY_HIGH";
        case SecurityPermission::RESOURCE_CPU_HIGH: return "RESOURCE_CPU_HIGH";
        case SecurityPermission::RESOURCE_DISK_HIGH: return "RESOURCE_DISK_HIGH";
        case SecurityPermission::ADMIN_PLUGIN_MANAGEMENT: return "ADMIN_PLUGIN_MANAGEMENT";
        case SecurityPermission::ADMIN_SECURITY_POLICY: return "ADMIN_SECURITY_POLICY";
        case SecurityPermission::ADMIN_SYSTEM_ACCESS: return "ADMIN_SYSTEM_ACCESS";
        default: return "UNKNOWN";
    }
}

SecurityPermission stringToPermission(const QString& permissionStr)
{
    if (permissionStr == "FILE_READ_SYSTEM") return SecurityPermission::FILE_READ_SYSTEM;
    if (permissionStr == "FILE_READ_USER") return SecurityPermission::FILE_READ_USER;
    if (permissionStr == "FILE_READ_TEMP") return SecurityPermission::FILE_READ_TEMP;
    if (permissionStr == "FILE_WRITE_SYSTEM") return SecurityPermission::FILE_WRITE_SYSTEM;
    if (permissionStr == "FILE_WRITE_USER") return SecurityPermission::FILE_WRITE_USER;
    if (permissionStr == "FILE_WRITE_TEMP") return SecurityPermission::FILE_WRITE_TEMP;
    if (permissionStr == "FILE_DELETE") return SecurityPermission::FILE_DELETE;
    if (permissionStr == "FILE_EXECUTE") return SecurityPermission::FILE_EXECUTE;
    if (permissionStr == "NETWORK_HTTP_GET") return SecurityPermission::NETWORK_HTTP_GET;
    if (permissionStr == "NETWORK_HTTP_POST") return SecurityPermission::NETWORK_HTTP_POST;
    if (permissionStr == "NETWORK_SOCKET") return SecurityPermission::NETWORK_SOCKET;
    if (permissionStr == "NETWORK_DNS") return SecurityPermission::NETWORK_DNS;
    if (permissionStr == "SYSTEM_REGISTRY") return SecurityPermission::SYSTEM_REGISTRY;
    if (permissionStr == "SYSTEM_ENVIRONMENT") return SecurityPermission::SYSTEM_ENVIRONMENT;
    if (permissionStr == "SYSTEM_PROCESS") return SecurityPermission::SYSTEM_PROCESS;
    if (permissionStr == "SYSTEM_LIBRARY") return SecurityPermission::SYSTEM_LIBRARY;
    if (permissionStr == "APP_CONFIG_READ") return SecurityPermission::APP_CONFIG_READ;
    if (permissionStr == "APP_CONFIG_WRITE") return SecurityPermission::APP_CONFIG_WRITE;
    if (permissionStr == "APP_DATA_READ") return SecurityPermission::APP_DATA_READ;
    if (permissionStr == "APP_DATA_WRITE") return SecurityPermission::APP_DATA_WRITE;
    if (permissionStr == "APP_UI_ACCESS") return SecurityPermission::APP_UI_ACCESS;
    if (permissionStr == "PLUGIN_COMMUNICATION") return SecurityPermission::PLUGIN_COMMUNICATION;
    if (permissionStr == "PLUGIN_DISCOVERY") return SecurityPermission::PLUGIN_DISCOVERY;
    if (permissionStr == "RESOURCE_MEMORY_HIGH") return SecurityPermission::RESOURCE_MEMORY_HIGH;
    if (permissionStr == "RESOURCE_CPU_HIGH") return SecurityPermission::RESOURCE_CPU_HIGH;
    if (permissionStr == "RESOURCE_DISK_HIGH") return SecurityPermission::RESOURCE_DISK_HIGH;
    if (permissionStr == "ADMIN_PLUGIN_MANAGEMENT") return SecurityPermission::ADMIN_PLUGIN_MANAGEMENT;
    if (permissionStr == "ADMIN_SECURITY_POLICY") return SecurityPermission::ADMIN_SECURITY_POLICY;
    if (permissionStr == "ADMIN_SYSTEM_ACCESS") return SecurityPermission::ADMIN_SYSTEM_ACCESS;
    
    // Default to a safe permission
    return SecurityPermission::APP_CONFIG_READ;
}

QStringList getAllPermissionNames()
{
    QStringList permissions;
    permissions << "FILE_READ_SYSTEM" << "FILE_READ_USER" << "FILE_READ_TEMP";
    permissions << "FILE_WRITE_SYSTEM" << "FILE_WRITE_USER" << "FILE_WRITE_TEMP";
    permissions << "FILE_DELETE" << "FILE_EXECUTE";
    permissions << "NETWORK_HTTP_GET" << "NETWORK_HTTP_POST" << "NETWORK_SOCKET" << "NETWORK_DNS";
    permissions << "SYSTEM_REGISTRY" << "SYSTEM_ENVIRONMENT" << "SYSTEM_PROCESS" << "SYSTEM_LIBRARY";
    permissions << "APP_CONFIG_READ" << "APP_CONFIG_WRITE" << "APP_DATA_READ" << "APP_DATA_WRITE" << "APP_UI_ACCESS";
    permissions << "PLUGIN_COMMUNICATION" << "PLUGIN_DISCOVERY";
    permissions << "RESOURCE_MEMORY_HIGH" << "RESOURCE_CPU_HIGH" << "RESOURCE_DISK_HIGH";
    permissions << "ADMIN_PLUGIN_MANAGEMENT" << "ADMIN_SECURITY_POLICY" << "ADMIN_SYSTEM_ACCESS";
    return permissions;
}

bool isFileSystemPermission(SecurityPermission permission)
{
    switch (permission) {
        case SecurityPermission::FILE_READ_SYSTEM:
        case SecurityPermission::FILE_READ_USER:
        case SecurityPermission::FILE_READ_TEMP:
        case SecurityPermission::FILE_WRITE_SYSTEM:
        case SecurityPermission::FILE_WRITE_USER:
        case SecurityPermission::FILE_WRITE_TEMP:
        case SecurityPermission::FILE_DELETE:
        case SecurityPermission::FILE_EXECUTE:
            return true;
        default:
            return false;
    }
}

bool isNetworkPermission(SecurityPermission permission)
{
    switch (permission) {
        case SecurityPermission::NETWORK_HTTP_GET:
        case SecurityPermission::NETWORK_HTTP_POST:
        case SecurityPermission::NETWORK_SOCKET:
        case SecurityPermission::NETWORK_DNS:
            return true;
        default:
            return false;
    }
}

bool isSystemPermission(SecurityPermission permission)
{
    switch (permission) {
        case SecurityPermission::SYSTEM_REGISTRY:
        case SecurityPermission::SYSTEM_ENVIRONMENT:
        case SecurityPermission::SYSTEM_PROCESS:
        case SecurityPermission::SYSTEM_LIBRARY:
            return true;
        default:
            return false;
    }
}

bool isAdministrativePermission(SecurityPermission permission)
{
    switch (permission) {
        case SecurityPermission::ADMIN_PLUGIN_MANAGEMENT:
        case SecurityPermission::ADMIN_SECURITY_POLICY:
        case SecurityPermission::ADMIN_SYSTEM_ACCESS:
            return true;
        default:
            return false;
    }
}

bool isPathSafe(const QString& path)
{
    // Check for dangerous path patterns
    if (path.contains("..") || path.contains("~")) {
        return false;
    }
    
    // Check for absolute paths to system directories
    QStringList dangerousPaths = {
        "/bin", "/sbin", "/usr/bin", "/usr/sbin",
        "C:\\Windows", "C:\\Program Files", "C:\\Program Files (x86)",
        "/etc", "/var", "/root"
    };
    
    QString normalizedPath = QDir::cleanPath(path);
    for (const QString& dangerousPath : dangerousPaths) {
        if (normalizedPath.startsWith(dangerousPath, Qt::CaseInsensitive)) {
            return false;
        }
    }
    
    return true;
}

QString normalizePath(const QString& path)
{
    return QDir::cleanPath(path);
}

bool isPathWithinAllowed(const QString& path, const QStringList& allowedPaths)
{
    QString normalizedPath = normalizePath(path);
    
    for (const QString& allowedPath : allowedPaths) {
        QString normalizedAllowed = normalizePath(allowedPath);
        if (normalizedPath.startsWith(normalizedAllowed)) {
            return true;
        }
    }
    
    return false;
}

QStringList expandPathPatterns(const QStringList& patterns)
{
    QStringList expandedPaths;
    
    for (const QString& pattern : patterns) {
        if (pattern.contains('*') || pattern.contains('?')) {
            // Handle wildcard patterns
            QDir dir(QFileInfo(pattern).path());
            QString fileName = QFileInfo(pattern).fileName();
            QStringList matches = dir.entryList(QStringList() << fileName, QDir::Dirs | QDir::Files);
            
            for (const QString& match : matches) {
                expandedPaths.append(dir.absoluteFilePath(match));
            }
        } else {
            expandedPaths.append(pattern);
        }
    }
    
    return expandedPaths;
}

bool isPluginIdValid(const QString& pluginId)
{
    if (pluginId.isEmpty() || pluginId.length() > 255) {
        return false;
    }
    
    // Check for valid characters (alphanumeric, underscore, dash, dot)
    QRegularExpression regex("^[a-zA-Z0-9_.-]+$");
    return regex.match(pluginId).hasMatch();
}

bool isSecurityLevelValid(int level)
{
    return level >= 0 && level <= 10;
}

bool areResourceLimitsValid(qint64 memory, int cpuTime, int fileAccess, int networkRequests)
{
    return memory > 0 && cpuTime > 0 && fileAccess > 0 && networkRequests > 0;
}

QString calculateFileHash(const QString& filePath, QCryptographicHash::Algorithm algorithm)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    
    QCryptographicHash hash(algorithm);
    hash.addData(&file);
    
    return hash.result().toHex();
}

bool compareHashes(const QString& hash1, const QString& hash2)
{
    return hash1.compare(hash2, Qt::CaseInsensitive) == 0;
}

QString generateSecureToken(int length)
{
    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    QString token;
    
    for (int i = 0; i < length; ++i) {
        token.append(chars.at(qrand() % chars.length()));
    }
    
    return token;
}

QVariantMap securityContextToMap(const SecurityContext& context)
{
    QVariantMap map;
    
    map["plugin_id"] = context.getPluginId();
    map["security_level"] = context.getSecurityLevel();
    map["memory_limit"] = static_cast<qint64>(context.getMemoryLimit());
    map["cpu_time_limit"] = context.getCpuTimeLimit();
    map["file_access_limit"] = context.getFileAccessLimit();
    map["network_request_limit"] = context.getNetworkRequestLimit();
    map["sandbox_enabled"] = context.isSandboxEnabled();
    map["allowed_paths"] = context.getAllowedPaths();
    map["allowed_domains"] = context.getAllowedDomains();
    
    // Convert permissions to string list
    QStringList permissionStrings;
    QSet<SecurityPermission> permissions = context.getGrantedPermissions();
    for (SecurityPermission permission : permissions) {
        permissionStrings.append(permissionToString(permission));
    }
    map["granted_permissions"] = permissionStrings;
    
    return map;
}

SecurityContext securityContextFromMap(const QVariantMap& map)
{
    SecurityContext context(map.value("plugin_id").toString());
    
    context.setSecurityLevel(map.value("security_level", 5).toInt());
    context.setMemoryLimit(map.value("memory_limit", 100 * 1024 * 1024).toLongLong());
    context.setCpuTimeLimit(map.value("cpu_time_limit", 5000).toInt());
    context.setFileAccessLimit(map.value("file_access_limit", 100).toInt());
    context.setNetworkRequestLimit(map.value("network_request_limit", 50).toInt());
    context.setSandboxEnabled(map.value("sandbox_enabled", true).toBool());
    context.setAllowedPaths(map.value("allowed_paths").toStringList());
    context.setAllowedDomains(map.value("allowed_domains").toStringList());
    
    // Convert permission strings back to permissions
    QStringList permissionStrings = map.value("granted_permissions").toStringList();
    for (const QString& permissionStr : permissionStrings) {
        context.grantPermission(stringToPermission(permissionStr));
    }
    
    return context;
}

QVariantMap securityPolicyToMap(const SecurityPolicy& policy)
{
    QVariantMap map;
    
    map["name"] = policy.getName();
    map["description"] = policy.getDescription();
    map["default_memory_limit"] = static_cast<qint64>(policy.getDefaultMemoryLimit());
    map["default_cpu_time_limit"] = policy.getDefaultCpuTimeLimit();
    map["default_file_access_limit"] = policy.getDefaultFileAccessLimit();
    map["default_network_request_limit"] = policy.getDefaultNetworkRequestLimit();
    map["sandbox_required"] = policy.isSandboxRequired();
    map["allowed_base_paths"] = policy.getAllowedBasePaths();
    map["allowed_domains"] = policy.getAllowedDomains();
    map["allowed_plugin_types"] = policy.getAllowedPluginTypes();
    map["blocked_plugin_patterns"] = policy.getBlockedPluginPatterns();
    map["signature_required"] = policy.isSignatureRequired();
    map["trusted_signers"] = policy.getTrustedSigners();
    
    return map;
}

SecurityPolicy securityPolicyFromMap(const QVariantMap& map)
{
    SecurityPolicy policy(map.value("name").toString());
    
    policy.setDescription(map.value("description").toString());
    policy.setDefaultMemoryLimit(map.value("default_memory_limit", 100 * 1024 * 1024).toLongLong());
    policy.setDefaultCpuTimeLimit(map.value("default_cpu_time_limit", 5000).toInt());
    policy.setDefaultFileAccessLimit(map.value("default_file_access_limit", 100).toInt());
    policy.setDefaultNetworkRequestLimit(map.value("default_network_request_limit", 50).toInt());
    policy.setSandboxRequired(map.value("sandbox_required", true).toBool());
    policy.setAllowedBasePaths(map.value("allowed_base_paths").toStringList());
    policy.setAllowedDomains(map.value("allowed_domains").toStringList());
    policy.setAllowedPluginTypes(map.value("allowed_plugin_types").toStringList());
    policy.setBlockedPluginPatterns(map.value("blocked_plugin_patterns").toStringList());
    policy.setSignatureRequired(map.value("signature_required", false).toBool());
    policy.setTrustedSigners(map.value("trusted_signers").toStringList());
    
    return policy;
}

} // namespace SecurityUtils

} // namespace PluginInterface

#include "pluginsecurity.moc"