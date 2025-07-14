/**
 * Item Editor Qt6 - Server Item List Header
 * Exact mirror of Legacy_App/csharp/Source/PluginInterface/OTLib/Collections/ServerItemList.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef OTLIB_COLLECTIONS_SERVERITEMLIST_H
#define OTLIB_COLLECTIONS_SERVERITEMLIST_H

#include <QObject>
#include <QList>
#include <QMetaType>

// Forward declarations
namespace OTLib {
namespace Server {
namespace Items {
    class ServerItem;
    enum class ServerItemFlag : quint32;
    using ServerItemFlags = QFlags<ServerItemFlag>;
}
}
}

namespace OTLib {
namespace Collections {

/**
 * Server Item List Class
 * Exact mirror of C# ServerItemList class
 * 
 * Manages a collection of ServerItem objects with version tracking
 * and search capabilities. Uses Qt6 patterns with signals for change notifications.
 */
class ServerItemList : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor
     * Initializes empty list with default min/max IDs
     */
    explicit ServerItemList(QObject *parent = nullptr);

    /**
     * Destructor
     * Cleans up managed ServerItem objects
     */
    ~ServerItemList();

    // Properties (equivalent to C# properties)
    
    /**
     * Gets the list of items
     * @return Reference to internal QList<ServerItem*>
     */
    const QList<OTLib::Server::Items::ServerItem*>& items() const { return m_items; }
    
    /**
     * Gets the minimum item ID
     * @return Minimum ID value
     */
    quint16 minId() const { return m_minId; }
    
    /**
     * Gets the maximum item ID  
     * @return Maximum ID value
     */
    quint16 maxId() const { return m_maxId; }
    
    /**
     * Gets the count of items
     * @return Number of items in collection
     */
    int count() const { return m_items.count(); }
    
    /**
     * Gets/sets the major version
     */
    quint32 majorVersion() const { return m_majorVersion; }
    void setMajorVersion(quint32 version) { m_majorVersion = version; }
    
    /**
     * Gets/sets the minor version
     */
    quint32 minorVersion() const { return m_minorVersion; }
    void setMinorVersion(quint32 version) { m_minorVersion = version; }
    
    /**
     * Gets/sets the build number
     */
    quint32 buildNumber() const { return m_buildNumber; }
    void setBuildNumber(quint32 build) { m_buildNumber = build; }
    
    /**
     * Gets/sets the client version
     */
    quint32 clientVersion() const { return m_clientVersion; }
    void setClientVersion(quint32 version) { m_clientVersion = version; }

public slots:
    /**
     * Adds an item to the collection
     * Updates MaxId if necessary. Ignores duplicates.
     * @param item ServerItem to add (takes ownership)
     */
    void add(OTLib::Server::Items::ServerItem* item);
    
    /**
     * Clears all items from the collection
     * Resets MaxId to 100
     */
    void clear();

public:
    /**
     * Finds items by server ID
     * @param sid Server ID to search for
     * @return List of matching ServerItem pointers
     */
    QList<OTLib::Server::Items::ServerItem*> findByServerId(quint16 sid) const;
    
    /**
     * Finds items by client ID
     * @param cid Client ID to search for
     * @return List of matching ServerItem pointers
     */
    QList<OTLib::Server::Items::ServerItem*> findByClientId(quint16 cid) const;
    
    /**
     * Finds items by properties/flags
     * @param properties ServerItemFlags to search for
     * @return List of matching ServerItem pointers
     */
    QList<OTLib::Server::Items::ServerItem*> findByProperties(
        OTLib::Server::Items::ServerItemFlags properties) const;
    
    /**
     * Tries to get an item by server ID
     * @param sid Server ID to search for
     * @param item Output parameter for found item
     * @return true if item found, false otherwise
     */
    bool tryGetValue(quint16 sid, OTLib::Server::Items::ServerItem*& item) const;

    // Iterator support for range-based for loops
    QList<OTLib::Server::Items::ServerItem*>::const_iterator begin() const { return m_items.begin(); }
    QList<OTLib::Server::Items::ServerItem*>::const_iterator end() const { return m_items.end(); }

signals:
    /**
     * Emitted when an item is added to the collection
     * @param item The added ServerItem
     */
    void itemAdded(OTLib::Server::Items::ServerItem* item);
    
    /**
     * Emitted when the collection is cleared
     */
    void collectionCleared();
    
    /**
     * Emitted when the collection changes
     */
    void collectionChanged();

private:
    QList<OTLib::Server::Items::ServerItem*> m_items;
    quint16 m_minId;
    quint16 m_maxId;
    quint32 m_majorVersion;
    quint32 m_minorVersion;
    quint32 m_buildNumber;
    quint32 m_clientVersion;
};

} // namespace Collections
} // namespace OTLib

// Register with Qt meta-object system
Q_DECLARE_METATYPE(OTLib::Collections::ServerItemList*)

#endif // OTLIB_COLLECTIONS_SERVERITEMLIST_H