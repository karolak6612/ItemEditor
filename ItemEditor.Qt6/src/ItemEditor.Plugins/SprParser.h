#pragma once

#include <QString>
#include "ClientDataTypes.h"

/**
 * @brief Base SPR file parser interface
 * 
 * Placeholder implementation - will be fully implemented in later tasks
 */
class SprParser
{
public:
    virtual ~SprParser() = default;
    
    virtual bool parseFile(const QString& filePath) = 0;
    virtual ::SpriteData getSpriteData(quint16 id) const = 0;
    virtual bool isLoaded() const = 0;
    virtual void cleanup() = 0;
};