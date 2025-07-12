#include "core/eventmanager.h"
#include <QCoreApplication>
#include <QMutexLocker>
#include <QDebug>

namespace ItemEditor {
namespace Core {

// Static instance
EventManager* EventManager::s_instance = nullptr;

EventManager& EventManager::instance()
{
    if (!s_instance) {
        s_instance = new EventManager();
    }
    return *s_instance;
}

EventManager::EventManager(QObject* parent)
    : QObject(parent)
    , m_loggingEnabled(false)
    , m_nextHandlerId(1)
{
    qDebug() << "EventManager initialized";
}

int EventManager::registerHandler(QEvent::Type eventType, EventHandler handler, 
                                 Priority priority, QObject* receiver)
{
    QMutexLocker locker(&m_mutex);
    
    int handlerId = m_nextHandlerId++;
    
    HandlerInfo info;
    info.id = handlerId;
    info.handler = handler;
    info.priority = priority;
    info.receiver = receiver;
    
    // Add to handlers list and sort by priority
    m_handlers[eventType].append(info);
    std::sort(m_handlers[eventType].begin(), m_handlers[eventType].end());
    
    // Track handler type
    m_handlerTypes[handlerId] = eventType;
    
    // Track receiver handlers for cleanup
    if (receiver) {
        m_receiverHandlers[receiver].append(handlerId);
        
        // Connect to receiver destruction for automatic cleanup
        connect(receiver, &QObject::destroyed, this, [this, receiver]() {
            unregisterReceiver(receiver);
        });
    }
    
    if (m_loggingEnabled) {
        qDebug() << "Registered event handler" << handlerId 
                 << "for event type" << eventType
                 << "with priority" << static_cast<int>(priority);
    }
    
    return handlerId;
}void EventManager::unregisterHandler(int handlerId)
{
    QMutexLocker locker(&m_mutex);
    
    auto typeIt = m_handlerTypes.find(handlerId);
    if (typeIt == m_handlerTypes.end()) {
        return; // Handler not found
    }
    
    QEvent::Type eventType = typeIt.value();
    m_handlerTypes.erase(typeIt);
    
    // Remove from handlers list
    auto& handlers = m_handlers[eventType];
    handlers.erase(std::remove_if(handlers.begin(), handlers.end(),
                                 [handlerId](const HandlerInfo& info) {
                                     return info.id == handlerId;
                                 }), handlers.end());
    
    // Remove from receiver tracking
    for (auto it = m_receiverHandlers.begin(); it != m_receiverHandlers.end(); ++it) {
        it.value().removeAll(handlerId);
        if (it.value().isEmpty()) {
            m_receiverHandlers.erase(it);
            break;
        }
    }
    
    if (m_loggingEnabled) {
        qDebug() << "Unregistered event handler" << handlerId;
    }
}

void EventManager::unregisterReceiver(QObject* receiver)
{
    QMutexLocker locker(&m_mutex);
    
    auto it = m_receiverHandlers.find(receiver);
    if (it == m_receiverHandlers.end()) {
        return; // Receiver not found
    }
    
    // Unregister all handlers for this receiver
    const QList<int>& handlerIds = it.value();
    for (int handlerId : handlerIds) {
        auto typeIt = m_handlerTypes.find(handlerId);
        if (typeIt != m_handlerTypes.end()) {
            QEvent::Type eventType = typeIt.value();
            m_handlerTypes.erase(typeIt);
            
            // Remove from handlers list
            auto& handlers = m_handlers[eventType];
            handlers.erase(std::remove_if(handlers.begin(), handlers.end(),
                                         [handlerId](const HandlerInfo& info) {
                                             return info.id == handlerId;
                                         }), handlers.end());
        }
    }
    
    m_receiverHandlers.erase(it);
    
    if (m_loggingEnabled) {
        qDebug() << "Unregistered all handlers for receiver" << receiver;
    }
}void EventManager::postEvent(QObject* receiver, QEvent* event)
{
    if (!receiver || !event) {
        return;
    }
    
    QCoreApplication::postEvent(receiver, event);
    
    if (m_loggingEnabled) {
        qDebug() << "Posted event" << event->type() << "to" << receiver;
    }
}

bool EventManager::sendEvent(QObject* receiver, QEvent* event)
{
    if (!receiver || !event) {
        return false;
    }
    
    bool result = QCoreApplication::sendEvent(receiver, event);
    
    if (m_loggingEnabled) {
        qDebug() << "Sent event" << event->type() << "to" << receiver 
                 << "- handled:" << result;
    }
    
    return result;
}

bool EventManager::processEvent(QEvent* event)
{
    if (!event) {
        return false;
    }
    
    QMutexLocker locker(&m_mutex);
    
    QEvent::Type eventType = event->type();
    auto it = m_handlers.find(eventType);
    if (it == m_handlers.end()) {
        return false; // No handlers for this event type
    }
    
    bool handled = false;
    const QList<HandlerInfo>& handlers = it.value();
    
    for (const HandlerInfo& info : handlers) {
        try {
            if (info.handler(event)) {
                handled = true;
                break; // Event was handled, stop processing
            }
        } catch (const std::exception& e) {
            qWarning() << "Exception in event handler" << info.id 
                       << "for event" << eventType << ":" << e.what();
        } catch (...) {
            qWarning() << "Unknown exception in event handler" << info.id 
                       << "for event" << eventType;
        }
    }
    
    if (m_loggingEnabled) {
        qDebug() << "Processed event" << eventType << "- handled:" << handled;
    }
    
    emit eventProcessed(eventType, handled);
    return handled;
}

} // namespace Core
} // namespace ItemEditor

#include "moc_eventmanager.cpp"