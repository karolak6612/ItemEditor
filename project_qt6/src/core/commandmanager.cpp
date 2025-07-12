#include "core/commandmanager.h"
#include <QMutexLocker>
#include <QDebug>

namespace ItemEditor {
namespace Core {

// Static instance
CommandManager* CommandManager::s_instance = nullptr;

CommandManager& CommandManager::instance()
{
    if (!s_instance) {
        s_instance = new CommandManager();
    }
    return *s_instance;
}

CommandManager::CommandManager(QObject* parent)
    : QObject(parent)
    , m_defaultStack(nullptr)
    , m_activeStack(nullptr)
{
    // Create default stack
    createStack("default", 100);
    setActiveStack("default");
    
    qDebug() << "CommandManager initialized";
}

void CommandManager::executeCommand(std::unique_ptr<ICommand> command, 
                                   ExecutionMode mode, const QString& stackName)
{
    if (!command) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    QUndoStack* stack = getStackInternal(stackName);
    if (!stack) {
        qWarning() << "Command stack not found:" << stackName;
        return;
    }
    
    QString commandText = command->text();
    QString actualStackName = stackName.isEmpty() ? "default" : stackName;
    
    switch (mode) {
    case ExecutionMode::Immediate:
        stack->push(command.release());
        emit commandExecuted(commandText, actualStackName);
        break;
        
    case ExecutionMode::Deferred:
        // Add to stack without executing
        command->undo(); // Ensure it's in "undone" state
        stack->push(command.release());
        break;
        
    case ExecutionMode::Preview:
        // Execute but don't add to stack
        command->redo();
        command->undo(); // Revert after preview
        break;
    }
}void CommandManager::createStack(const QString& stackName, int cleanLimit)
{
    QMutexLocker locker(&m_mutex);
    
    QString actualName = stackName.isEmpty() ? "default" : stackName;
    
    if (m_stacks.contains(actualName)) {
        qWarning() << "Command stack already exists:" << actualName;
        return;
    }
    
    QUndoStack* stack = new QUndoStack(this);
    stack->setUndoLimit(cleanLimit);
    
    m_stacks[actualName] = stack;
    
    if (actualName == "default") {
        m_defaultStack = stack;
    }
    
    connectStackSignals(stack, actualName);
    
    qDebug() << "Created command stack:" << actualName << "with limit:" << cleanLimit;
}

void CommandManager::removeStack(const QString& stackName)
{
    QMutexLocker locker(&m_mutex);
    
    QString actualName = stackName.isEmpty() ? "default" : stackName;
    
    if (actualName == "default") {
        qWarning() << "Cannot remove default command stack";
        return;
    }
    
    auto it = m_stacks.find(actualName);
    if (it == m_stacks.end()) {
        return; // Stack doesn't exist
    }
    
    QUndoStack* stack = it.value();
    m_stacks.erase(it);
    
    if (m_activeStack == stack) {
        m_activeStack = m_defaultStack;
    }
    
    stack->deleteLater();
    
    qDebug() << "Removed command stack:" << actualName;
}

QUndoStack* CommandManager::getStack(const QString& stackName) const
{
    QMutexLocker locker(&m_mutex);
    return getStackInternal(stackName);
}

void CommandManager::setActiveStack(const QString& stackName)
{
    QMutexLocker locker(&m_mutex);
    
    QUndoStack* stack = getStackInternal(stackName);
    if (stack) {
        m_activeStack = stack;
        qDebug() << "Set active command stack:" << (stackName.isEmpty() ? "default" : stackName);
    }
}bool CommandManager::canUndo(const QString& stackName) const
{
    QMutexLocker locker(&m_mutex);
    QUndoStack* stack = getStackInternal(stackName);
    return stack ? stack->canUndo() : false;
}

bool CommandManager::canRedo(const QString& stackName) const
{
    QMutexLocker locker(&m_mutex);
    QUndoStack* stack = getStackInternal(stackName);
    return stack ? stack->canRedo() : false;
}

void CommandManager::undo(const QString& stackName)
{
    QMutexLocker locker(&m_mutex);
    QUndoStack* stack = getStackInternal(stackName);
    if (stack && stack->canUndo()) {
        QString commandText = stack->undoText();
        stack->undo();
        emit undoPerformed(commandText, stackName.isEmpty() ? "default" : stackName);
    }
}

void CommandManager::redo(const QString& stackName)
{
    QMutexLocker locker(&m_mutex);
    QUndoStack* stack = getStackInternal(stackName);
    if (stack && stack->canRedo()) {
        QString commandText = stack->redoText();
        stack->redo();
        emit redoPerformed(commandText, stackName.isEmpty() ? "default" : stackName);
    }
}

void CommandManager::clear(const QString& stackName)
{
    QMutexLocker locker(&m_mutex);
    
    if (stackName.isEmpty()) {
        // Clear all stacks
        for (auto it = m_stacks.begin(); it != m_stacks.end(); ++it) {
            it.value()->clear();
        }
        qDebug() << "Cleared all command stacks";
    } else {
        QUndoStack* stack = getStackInternal(stackName);
        if (stack) {
            stack->clear();
            qDebug() << "Cleared command stack:" << stackName;
        }
    }
}void CommandManager::beginMacro(const QString& text, const QString& stackName)
{
    QMutexLocker locker(&m_mutex);
    QUndoStack* stack = getStackInternal(stackName);
    if (stack) {
        stack->beginMacro(text);
    }
}

void CommandManager::endMacro(const QString& stackName)
{
    QMutexLocker locker(&m_mutex);
    QUndoStack* stack = getStackInternal(stackName);
    if (stack) {
        stack->endMacro();
    }
}

QString CommandManager::undoText(const QString& stackName) const
{
    QMutexLocker locker(&m_mutex);
    QUndoStack* stack = getStackInternal(stackName);
    return stack ? stack->undoText() : QString();
}

QString CommandManager::redoText(const QString& stackName) const
{
    QMutexLocker locker(&m_mutex);
    QUndoStack* stack = getStackInternal(stackName);
    return stack ? stack->redoText() : QString();
}

int CommandManager::commandCount(const QString& stackName) const
{
    QMutexLocker locker(&m_mutex);
    QUndoStack* stack = getStackInternal(stackName);
    return stack ? stack->count() : 0;
}

void CommandManager::setClean(const QString& stackName)
{
    QMutexLocker locker(&m_mutex);
    QUndoStack* stack = getStackInternal(stackName);
    if (stack) {
        stack->setClean();
    }
}

bool CommandManager::isClean(const QString& stackName) const
{
    QMutexLocker locker(&m_mutex);
    QUndoStack* stack = getStackInternal(stackName);
    return stack ? stack->isClean() : true;
}QUndoStack* CommandManager::getStackInternal(const QString& stackName) const
{
    QString actualName = stackName.isEmpty() ? "default" : stackName;
    auto it = m_stacks.find(actualName);
    return (it != m_stacks.end()) ? it.value() : nullptr;
}

void CommandManager::connectStackSignals(QUndoStack* stack, const QString& stackName)
{
    connect(stack, &QUndoStack::cleanChanged, this, [this, stackName](bool clean) {
        emit cleanChanged(clean, stackName);
    });
    
    connect(stack, &QUndoStack::canUndoChanged, this, [this, stackName](bool canUndo) {
        Q_UNUSED(canUndo);
        // Could emit additional signals here if needed
    });
    
    connect(stack, &QUndoStack::canRedoChanged, this, [this, stackName](bool canRedo) {
        Q_UNUSED(canRedo);
        // Could emit additional signals here if needed
    });
}

} // namespace Core
} // namespace ItemEditor

#include "moc_commandmanager.cpp"