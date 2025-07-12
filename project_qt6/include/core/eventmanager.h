#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <QObject>
#include <QEvent>
#include <QHash>
#include <QMutex>
#include <functional>

namespace ItemEditor {
namespace Core {

/**
 * @brief Centralized event management system
 * 
 * This class provides a centralized event handling system for the application,
 * allowing components to register for events and receive notifications.
 */
class EventManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Event handler function type
     */
    using EventHandler = std::function<bool(QEvent*)>;

    /**
     * @brief Event priority levels
     */
    enum class Priority {
        Low = 0,
        Normal = 50,
        High = 100,
        Critical = 200
    };

    /**
     * @brief Get singleton instance
     */
    static EventManager& instance();

    /**
     * @brief Register event handler
     * @param eventType Event type to handle
     * @param handler Handler function
     * @param priority Handler priority
     * @param receiver Optional receiver object for automatic cleanup
     * @return Handler ID for unregistration
     */
    int registerHandler(QEvent::Type eventType, EventHandler handler, 
                       Priority priority = Priority::Normal, QObject* receiver = nullptr);

    /**
     * @brief Unregister event handler
     * @param handlerId Handler ID returned by registerHandler
     */
    void unregisterHandler(int handlerId);

    /**
     * @brief Unregister all handlers for a receiver
     * @param receiver Receiver object
     */
    void unregisterReceiver(QObject* receiver);

    /**
     * @brief Post custom event
     * @param receiver Event receiver
     * @param event Event to post
     */
    void postEvent(QObject* receiver, QEvent* event);

    /**
     * @brief Send custom event immediately
     * @param receiver Event receiver
     * @param event Event to send
     * @return true if event was handled
     */
    bool sendEvent(QObject* receiver, QEvent* event);

    /**
     * @brief Process event through registered handlers
     * @param event Event to process
     * @return true if event was handled
     */
    bool processEvent(QEvent* event);

    /**
     * @brief Enable/disable event logging
     * @param enabled Logging enabled
     */
    void setEventLogging(bool enabled) { m_loggingEnabled = enabled; }

    /**
     * @brief Check if event logging is enabled
     * @return true if enabled
     */
    bool isEventLoggingEnabled() const { return m_loggingEnabled; }

signals:
    /**
     * @brief Emitted when an event is processed
     * @param eventType Event type
     * @param handled Whether event was handled
     */
    void eventProcessed(QEvent::Type eventType, bool handled);

private:
    explicit EventManager(QObject* parent = nullptr);
    ~EventManager() override = default;

    // Disable copy and assignment
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;

    /**
     * @brief Handler information
     */
    struct HandlerInfo {
        int id;
        EventHandler handler;
        Priority priority;
        QObject* receiver;
        
        bool operator<(const HandlerInfo& other) const {
            return priority > other.priority; // Higher priority first
        }
    };

    // Static instance
    static EventManager* s_instance;

    // Handler storage
    QHash<QEvent::Type, QList<HandlerInfo>> m_handlers;
    QHash<int, QEvent::Type> m_handlerTypes;
    QHash<QObject*, QList<int>> m_receiverHandlers;

    // Thread safety
    mutable QMutex m_mutex;

    // Configuration
    bool m_loggingEnabled;
    int m_nextHandlerId;
};

} // namespace Core
} // namespace ItemEditor

#endif // EVENTMANAGER_H