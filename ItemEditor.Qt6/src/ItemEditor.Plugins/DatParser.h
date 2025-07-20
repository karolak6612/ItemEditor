#pragma once

#include <QString>
#include "ClientDataTypes.h"

/**
 * @brief Base DAT file parser interface
 * 
 * Placeholder implementation - will be fully implemented in later tasks
 */
class DatParser
{
public:
    virtual ~DatParser() = default;
    
    virtual bool parseFile(const QString& filePath) = 0;
    virtual DatData getDatData(quint16 id) const = 0;
    virtual bool isLoaded() const = 0;
    virtual void cleanup() = 0;
};