#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

namespace Core {

/**
 * @brief Base interface for disposable objects
 * 
 * Equivalent to IDisposable in C#, provides cleanup functionality
 */
class IDisposable
{
public:
    virtual ~IDisposable() = default;
    virtual void dispose() = 0;
};

/**
 * @brief Base interface for initializable objects
 * 
 * Provides common initialization pattern
 */
class IInitializable
{
public:
    virtual ~IInitializable() = default;
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual bool isInitialized() const = 0;
};

/**
 * @brief Base interface for configurable objects
 * 
 * Provides configuration loading/saving capabilities
 */
class IConfigurable
{
public:
    virtual ~IConfigurable() = default;
    virtual void loadConfiguration() = 0;
    virtual void saveConfiguration() = 0;
    virtual QVariant getConfigValue(const QString &key, const QVariant &defaultValue = QVariant()) const = 0;
    virtual void setConfigValue(const QString &key, const QVariant &value) = 0;
};

/**
 * @brief Base interface for objects that can be validated
 * 
 * Provides validation functionality
 */
class IValidatable
{
public:
    virtual ~IValidatable() = default;
    virtual bool isValid() const = 0;
    virtual QStringList getValidationErrors() const = 0;
};

/**
 * @brief Base interface for objects that support cloning
 * 
 * Provides deep copy functionality
 */
template<typename T>
class ICloneable
{
public:
    virtual ~ICloneable() = default;
    virtual T* clone() const = 0;
};

} // namespace Core