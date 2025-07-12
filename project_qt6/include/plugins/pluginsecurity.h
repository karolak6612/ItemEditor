#ifndef PLUGINSECURITY_H
#define PLUGINSECURITY_H

#include "plugins/iplugin.h"
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QSet>
#include <QObject>
#include <QMutex>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QVariant>
#include <QSharedPointer>
#include <functional>

namespace PluginInterface {

// Forward declarations
class PluginSecurityManager;
class SecurityPolicy;
class PluginSandbox;

/**
 * @brief Security permission enumeration for plugin operations
 * 
 * Defines granular permissions that can be granted or denied to plugins.
 * Based on principle of least privilege.
 */
enum class SecurityPermission {
    // File system permissions
    FILE_READ_SYSTEM,           // Read system files
    FILE_READ_USER,             // Read user files
    FILE_READ_TEMP,             // Read temporary files
    FILE_WRITE_SYSTEM,          // Write system files
    FILE_WRITE_USER,            // Write user files
    FILE_WRITE_TEMP,            // Write temporary files
    FILE_DELETE,                // Delete files
    FILE_EXECUTE,               // Execute files
    
    // Network permissions
    NETWORK_HTTP_GET,           // HTTP GET requests
    NETWORK_HTTP_POST,          // HTTP POST requests
    NETWORK_SOCKET,             // Socket operations
    NETWORK_DNS,                // DNS resolution
    
    // System permissions
    SYSTEM_REGISTRY,            // Registry access (Windows)
    SYSTEM_ENVIRONMENT,         // Environment variables
    SYSTEM_PROCESS,             // Process management
    SYSTEM_LIBRARY,             // Dynamic library loading
    
    // Application permissions
    APP_CONFIG_READ,            // Read application configuration
    APP_CONFIG_WRITE,           // Write application configuration
    APP_DATA_READ,              // Read application data
    APP_DATA_WRITE,             // Write application data
    APP_UI_ACCESS,              // Access UI components
    
    // Inter-plugin permissions
    PLUGIN_COMMUNICATION,       // Communicate with other plugins
    PLUGIN_DISCOVERY,           // Discover other plugins
    
    // Resource permissions
    RESOURCE_MEMORY_HIGH,       // High memory usage
    RESOURCE_CPU_HIGH,          // High CPU usage
    RESOURCE_DISK_HIGH,         // High disk usage
    
    // Administrative permissions
    ADMIN_PLUGIN_MANAGEMENT,    // Manage other plugins
    ADMIN_SECURITY_POLICY,      // Modify security policies
    ADMIN_SYSTEM_ACCESS         // Full system access
};

/**
 * @brief Security context for plugin operations
 * 
 * Contains the security information and permissions for a plugin.
 */
class SecurityContext
{
public:
    SecurityContext(const QString& pluginId = QString());
    virtual ~SecurityContext() = default;

    // Permission management
    void grantPermission(SecurityPermission permission);
    void revokePermission(SecurityPermission permission);
    bool hasPermission(SecurityPermission permission) const;
    QSet<SecurityPermission> getGrantedPermissions() const;
    
    // Security level
    void setSecurityLevel(int level); // 0 = no restrictions, 10 = maximum restrictions
    int getSecurityLevel() const;
    
    // Resource limits
    void setMemoryLimit(qint64 bytes);
    void setCpuTimeLimit(int milliseconds);
    void setFileAccessLimit(int maxFiles);
    void setNetworkRequestLimit(int maxRequests);
    
    qint64 getMemoryLimit() const;
    int getCpuTimeLimit() const;
    int getFileAccessLimit() const;
    int getNetworkRequestLimit() const;
    
    // Sandbox configuration
    void setSandboxEnabled(bool enabled);
    bool isSandboxEnabled() const;
    void setAllowedPaths(const QStringList& paths);
    QStringList getAllowedPaths() const;
    void setAllowedDomains(const QStringList& domains);
    QStringList getAllowedDomains() const;
    
    // Plugin identification
    QString getPluginId() const;
    void setPluginId(const QString& id);
    
    // Validation
    bool isValid() const;
    QStringList getValidationErrors() const;

private:
    QString m_pluginId;
    QSet<SecurityPermission> m_grantedPermissions;
    int m_securityLevel;
    qint64 m_memoryLimit;
    int m_cpuTimeLimit;
    int m_fileAccessLimit;
    int m_networkRequestLimit;
    bool m_sandboxEnabled;
    QStringList m_allowedPaths;
    QStringList m_allowedDomains;
    mutable QStringList m_validationErrors;
};

/**
 * @brief Security policy definition for plugins
 * 
 * Defines security rules and constraints that apply to plugins.
 */
class SecurityPolicy
{
public:
    SecurityPolicy(const QString& name = QString());
    virtual ~SecurityPolicy() = default;

    // Policy identification
    QString getName() const;
    void setName(const QString& name);
    QString getDescription() const;
    void setDescription(const QString& description);
    
    // Permission rules
    void addPermissionRule(const QString& pluginPattern, SecurityPermission permission, bool allow);
    void removePermissionRule(const QString& pluginPattern, SecurityPermission permission);
    bool isPermissionAllowed(const QString& pluginId, SecurityPermission permission) const;
    
    // Resource limits
    void setDefaultMemoryLimit(qint64 bytes);
    void setDefaultCpuTimeLimit(int milliseconds);
    void setDefaultFileAccessLimit(int maxFiles);
    void setDefaultNetworkRequestLimit(int maxRequests);
    
    qint64 getDefaultMemoryLimit() const;
    int getDefaultCpuTimeLimit() const;
    int getDefaultFileAccessLimit() const;
    int getDefaultNetworkRequestLimit() const;
    
    // Sandbox rules
    void setSandboxRequired(bool required);
    bool isSandboxRequired() const;
    void setAllowedBasePaths(const QStringList& paths);
    QStringList getAllowedBasePaths() const;
    void setAllowedDomains(const QStringList& domains);
    QStringList getAllowedDomains() const;
    
    // Plugin restrictions
    void setAllowedPluginTypes(const QStringList& types);
    QStringList getAllowedPluginTypes() const;
    void setBlockedPluginPatterns(const QStringList& patterns);
    QStringList getBlockedPluginPatterns() const;
    
    // Signature verification
    void setSignatureRequired(bool required);
    bool isSignatureRequired() const;
    void setTrustedSigners(const QStringList& signers);
    QStringList getTrustedSigners() const;
    
    // Policy validation
    bool isValid() const;
    QStringList getValidationErrors() const;
    
    // Policy application
    SecurityContext createSecurityContext(const QString& pluginId) const;
    bool isPluginAllowed(const QString& pluginId, const QString& pluginType) const;

private:
    struct PermissionRule {
        QString pluginPattern;
        SecurityPermission permission;
        bool allow;
    };

    QString m_name;
    QString m_description;
    QList<PermissionRule> m_permissionRules;
    qint64 m_defaultMemoryLimit;
    int m_defaultCpuTimeLimit;
    int m_defaultFileAccessLimit;
    int m_defaultNetworkRequestLimit;
    bool m_sandboxRequired;
    QStringList m_allowedBasePaths;
    QStringList m_allowedDomains;
    QStringList m_allowedPluginTypes;
    QStringList m_blockedPluginPatterns;
    bool m_signatureRequired;
    QStringList m_trustedSigners;
    mutable QStringList m_validationErrors;
};

/**
 * @brief Plugin sandbox for isolating plugin execution
 * 
 * Provides runtime enforcement of security policies and resource limits.
 */
class PluginSandbox : public QObject
{
    Q_OBJECT

public:
    explicit PluginSandbox(const SecurityContext& context, QObject* parent = nullptr);
    virtual ~PluginSandbox();

    // Sandbox lifecycle
    bool initialize();
    void shutdown();
    bool isInitialized() const;
    
    // Security context
    SecurityContext getSecurityContext() const;
    void updateSecurityContext(const SecurityContext& context);
    
    // File system access control
    bool isFileAccessAllowed(const QString& filePath, SecurityPermission permission) const;
    bool isDirectoryAccessAllowed(const QString& dirPath, SecurityPermission permission) const;
    QStringList getAccessiblePaths() const;
    
    // Network access control
    bool isNetworkAccessAllowed(const QString& url, SecurityPermission permission) const;
    bool isDomainAllowed(const QString& domain) const;
    QStringList getAllowedDomains() const;
    
    // Resource monitoring
    qint64 getCurrentMemoryUsage() const;
    int getCurrentCpuTime() const;
    int getCurrentFileAccessCount() const;
    int getCurrentNetworkRequestCount() const;
    
    // Resource limit enforcement
    bool isMemoryLimitExceeded() const;
    bool isCpuTimeLimitExceeded() const;
    bool isFileAccessLimitExceeded() const;
    bool isNetworkRequestLimitExceeded() const;
    
    // Operation validation
    bool validateOperation(const QString& operation, const QVariantMap& parameters) const;
    bool requestPermission(SecurityPermission permission, const QString& reason = QString());
    
    // Violation handling
    void reportViolation(const QString& violation, const QVariantMap& details = QVariantMap());
    QStringList getViolationHistory() const;
    int getViolationCount() const;

signals:
    void permissionRequested(SecurityPermission permission, const QString& reason);
    void violationDetected(const QString& violation, const QVariantMap& details);
    void resourceLimitExceeded(const QString& resource, qint64 current, qint64 limit);
    void sandboxShutdown(const QString& reason);

public slots:
    void resetResourceCounters();
    void updateResourceLimits();

private slots:
    void onResourceMonitorTimer();

private:
    // Resource monitoring
    void startResourceMonitoring();
    void stopResourceMonitoring();
    void updateResourceUsage();
    
    // Access validation
    bool isPathAllowed(const QString& path) const;
    bool isPatternMatched(const QString& text, const QString& pattern) const;
    
    // Violation tracking
    void recordViolation(const QString& violation, const QVariantMap& details);
    
    SecurityContext m_context;
    bool m_initialized;
    
    // Resource monitoring
    QTimer* m_resourceTimer;
    qint64 m_currentMemoryUsage;
    int m_currentCpuTime;
    int m_currentFileAccessCount;
    int m_currentNetworkRequestCount;
    
    // Violation tracking
    QStringList m_violationHistory;
    int m_violationCount;
    
    // Thread safety
    mutable QMutex m_mutex;
};

/**
 * @brief Plugin signature verification system
 * 
 * Handles verification of plugin authenticity and integrity.
 */
class PluginSignatureVerifier : public QObject
{
    Q_OBJECT

public:
    explicit PluginSignatureVerifier(QObject* parent = nullptr);
    virtual ~PluginSignatureVerifier();

    // Signature verification
    bool verifyPluginSignature(const QString& pluginPath, const QString& signaturePath = QString()) const;
    bool verifyPluginIntegrity(const QString& pluginPath) const;
    QString calculatePluginHash(const QString& pluginPath) const;
    
    // Certificate management
    bool addTrustedCertificate(const QString& certificatePath);
    bool removeTrustedCertificate(const QString& certificateId);
    QStringList getTrustedCertificates() const;
    bool isCertificateTrusted(const QString& certificateId) const;
    
    // Signature creation (for development)
    bool signPlugin(const QString& pluginPath, const QString& privateKeyPath, const QString& certificatePath);
    bool createSignatureFile(const QString& pluginPath, const QString& signaturePath, const QString& privateKeyPath);
    
    // Configuration
    void setRequireSignature(bool required);
    bool isSignatureRequired() const;
    void setAllowSelfSigned(bool allow);
    bool isSelfSignedAllowed() const;
    
    // Validation
    QStringList getLastVerificationErrors() const;
    QString getLastError() const;

signals:
    void signatureVerified(const QString& pluginPath, bool valid);
    void certificateAdded(const QString& certificateId);
    void certificateRemoved(const QString& certificateId);

private:
    // Internal verification methods
    bool verifySignatureInternal(const QString& pluginPath, const QString& signaturePath) const;
    bool validateCertificateChain(const QString& certificatePath) const;
    QString extractCertificateId(const QString& certificatePath) const;
    
    bool m_requireSignature;
    bool m_allowSelfSigned;
    QStringList m_trustedCertificates;
    mutable QStringList m_lastErrors;
    mutable QString m_lastError;
    
    // Thread safety
    mutable QMutex m_mutex;
};

/**
 * @brief Main plugin security manager
 * 
 * Central coordinator for all plugin security operations.
 */
class PluginSecurityManager : public QObject
{
    Q_OBJECT

public:
    explicit PluginSecurityManager(QObject* parent = nullptr);
    virtual ~PluginSecurityManager();

    // Lifecycle management
    bool initialize();
    void shutdown();
    bool isInitialized() const;
    
    // Security policy management
    void setSecurityPolicy(const SecurityPolicy& policy);
    SecurityPolicy getSecurityPolicy() const;
    bool loadSecurityPolicy(const QString& filePath);
    bool saveSecurityPolicy(const QString& filePath) const;
    
    // Plugin security validation
    bool validatePlugin(const QString& pluginPath) const;
    bool validatePluginSecurity(IPlugin* plugin) const;
    SecurityContext createSecurityContext(const QString& pluginId) const;
    
    // Sandbox management
    PluginSandbox* createSandbox(const QString& pluginId);
    PluginSandbox* getSandbox(const QString& pluginId) const;
    void destroySandbox(const QString& pluginId);
    QStringList getActiveSandboxes() const;
    
    // Permission management
    bool checkPermission(const QString& pluginId, SecurityPermission permission) const;
    bool requestPermission(const QString& pluginId, SecurityPermission permission, const QString& reason = QString());
    void grantPermission(const QString& pluginId, SecurityPermission permission);
    void revokePermission(const QString& pluginId, SecurityPermission permission);
    
    // Signature verification
    PluginSignatureVerifier* getSignatureVerifier() const;
    bool verifyPluginSignature(const QString& pluginPath) const;
    
    // Security monitoring
    void startSecurityMonitoring();
    void stopSecurityMonitoring();
    bool isSecurityMonitoringActive() const;
    
    // Violation handling
    QStringList getSecurityViolations() const;
    int getViolationCount() const;
    void clearViolationHistory();
    
    // Configuration
    void setStrictMode(bool strict);
    bool isStrictMode() const;
    void setAutoSandbox(bool autoSandbox);
    bool isAutoSandbox() const;
    
    // Statistics
    int getSecuredPluginCount() const;
    int getActiveSandboxCount() const;
    QMap<QString, QVariant> getSecurityStatistics() const;

signals:
    void securityPolicyChanged();
    void pluginValidated(const QString& pluginPath, bool valid);
    void sandboxCreated(const QString& pluginId);
    void sandboxDestroyed(const QString& pluginId);
    void permissionRequested(const QString& pluginId, SecurityPermission permission, const QString& reason);
    void permissionGranted(const QString& pluginId, SecurityPermission permission);
    void permissionRevoked(const QString& pluginId, SecurityPermission permission);
    void securityViolation(const QString& pluginId, const QString& violation, const QVariantMap& details);
    void signatureVerificationFailed(const QString& pluginPath, const QString& reason);

public slots:
    void refreshSecurityPolicy();
    void validateAllPlugins();
    void emergencyShutdown();

private slots:
    void onSandboxViolation(const QString& violation, const QVariantMap& details);
    void onPermissionRequested(SecurityPermission permission, const QString& reason);
    void onSignatureVerified(const QString& pluginPath, bool valid);

private:
    // Internal methods
    void initializeDefaultPolicy();
    void loadDefaultTrustedCertificates();
    bool validateSecurityConfiguration() const;
    void cleanupInactiveSandboxes();
    
    bool m_initialized;
    SecurityPolicy m_securityPolicy;
    PluginSignatureVerifier* m_signatureVerifier;
    QMap<QString, PluginSandbox*> m_sandboxes;
    QStringList m_securityViolations;
    bool m_strictMode;
    bool m_autoSandbox;
    bool m_securityMonitoringActive;
    
    // Statistics
    int m_securedPluginCount;
    QMap<QString, QVariant> m_securityStatistics;
    
    // Thread safety
    mutable QMutex m_mutex;
};

/**
 * @brief Security utility functions
 * 
 * Helper functions for security operations.
 */
namespace SecurityUtils {
    // Permission utilities
    QString permissionToString(SecurityPermission permission);
    SecurityPermission stringToPermission(const QString& permissionStr);
    QStringList getAllPermissionNames();
    bool isFileSystemPermission(SecurityPermission permission);
    bool isNetworkPermission(SecurityPermission permission);
    bool isSystemPermission(SecurityPermission permission);
    bool isAdministrativePermission(SecurityPermission permission);
    
    // Path utilities
    bool isPathSafe(const QString& path);
    QString normalizePath(const QString& path);
    bool isPathWithinAllowed(const QString& path, const QStringList& allowedPaths);
    QStringList expandPathPatterns(const QStringList& patterns);
    
    // Security validation
    bool isPluginIdValid(const QString& pluginId);
    bool isSecurityLevelValid(int level);
    bool areResourceLimitsValid(qint64 memory, int cpuTime, int fileAccess, int networkRequests);
    
    // Hash and signature utilities
    QString calculateFileHash(const QString& filePath, QCryptographicHash::Algorithm algorithm = QCryptographicHash::Sha256);
    bool compareHashes(const QString& hash1, const QString& hash2);
    QString generateSecureToken(int length = 32);
    
    // Configuration utilities
    QVariantMap securityContextToMap(const SecurityContext& context);
    SecurityContext securityContextFromMap(const QVariantMap& map);
    QVariantMap securityPolicyToMap(const SecurityPolicy& policy);
    SecurityPolicy securityPolicyFromMap(const QVariantMap& map);
}

} // namespace PluginInterface

// Meta-type declarations for Qt's type system
Q_DECLARE_METATYPE(PluginInterface::SecurityPermission)
Q_DECLARE_METATYPE(PluginInterface::SecurityContext)
Q_DECLARE_METATYPE(PluginInterface::SecurityPolicy)

#endif // PLUGINSECURITY_H