#pragma once

#include <QtGlobal>

/**
 * @brief Core type definitions for ItemEditor
 * 
 * Defines fundamental types used throughout the application
 * to maintain compatibility with the legacy system.
 */

// Basic type aliases for compatibility
using ItemId = quint16;
using ClientId = quint16;
using ServerId = quint16;

// Version information structure
struct VersionInfo
{
    quint32 majorVersion;
    quint32 minorVersion;
    quint32 buildNumber;
    quint32 clientVersion;
};

// Item range structure
struct ItemRange
{
    ItemId minId;
    ItemId maxId;
    
    bool contains(ItemId id) const {
        return id >= minId && id <= maxId;
    }
    
    quint32 count() const {
        return (maxId >= minId) ? (maxId - minId + 1) : 0;
    }
};