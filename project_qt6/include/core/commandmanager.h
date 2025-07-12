#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <QObject>
#include <QUndoStack>
#include <QUndoCommand>
#include <QHash>
#include <QMutex>
#include <memory>

namespace ItemEditor {
namespace Core {

/**
 * @brief Base command interface
 */
class ICommand : public QUndoCommand
{
public:
    explicit ICommand(const QString& text, QUndoCommand* parent = nullptr)
        : QUndoCommand(text, parent) {}
    
    virtual ~ICommand() = default;
    
    /**
     * @brief Get command category
     */
    virtual QString category() const { return "General"; }
    
    /**
     * @brief Check if command can be merged with another
     */
    virtual bool canMergeWith(const ICommand* other) const { Q_UNUSED(other); return false; }
    
    /**
     * @brief Merge with another command
     */
    virtual void mergeWith(const ICommand* other) { Q_UNUSED(other); }
};

/**
 * @brief Centralized command management system
 * 
 * This class provides undo/redo functionality and command pattern implementation
 * for the application, allowing for complex operation management and history tracking.
 */
class CommandManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Command execution mode
     */
    enum class ExecutionMode {
        Immediate,      // Execute immediately and add to stack
        Deferred,       // Add to stack but don't execute
        Preview         // Execute but don't add to stack
    };

    /**
     * @brief Get singleton instance
     */
    static CommandManager& instance();

    /**
     * @brief Execute command
     * @param command Command to execute
     * @param mode Execution mode
     * @param stackName Stack name (default stack if empty)
     */
    void executeCommand(std::unique_ptr<ICommand> command, 
                       ExecutionMode mode = ExecutionMode::Immediate,
                       const QString& stackName = QString());

    /**
     * @brief Create named command stack
     * @param stackName Stack name
     * @param cleanLimit Clean limit for the stack
     */
    void createStack(const QString& stackName, int cleanLimit = 100);

    /**
     * @brief Remove command stack
     * @param stackName Stack name
     */
    void removeStack(const QString& stackName);

    /**
     * @brief Get command stack
     * @param stackName Stack name (default stack if empty)
     * @return Command stack or nullptr if not found
     */
    QUndoStack* getStack(const QString& stackName = QString()) const;

    /**
     * @brief Set active command stack
     * @param stackName Stack name
     */
    void setActiveStack(const QString& stackName);

    /**
     * @brief Get active command stack
     * @return Active command stack
     */
    QUndoStack* activeStack() const { return m_activeStack; }

    /**
     * @brief Check if undo is available
     * @param stackName Stack name (active stack if empty)
     */
    bool canUndo(const QString& stackName = QString()) const;

    /**
     * @brief Check if redo is available
     * @param stackName Stack name (active stack if empty)
     */
    bool canRedo(const QString& stackName = QString()) const;

    /**
     * @brief Undo last command
     * @param stackName Stack name (active stack if empty)
     */
    void undo(const QString& stackName = QString());

    /**
     * @brief Redo last undone command
     * @param stackName Stack name (active stack if empty)
     */
    void redo(const QString& stackName = QString());

    /**
     * @brief Clear command history
     * @param stackName Stack name (all stacks if empty)
     */
    void clear(const QString& stackName = QString());

    /**
     * @brief Begin macro command
     * @param text Macro command text
     * @param stackName Stack name (active stack if empty)
     */
    void beginMacro(const QString& text, const QString& stackName = QString());

    /**
     * @brief End macro command
     * @param stackName Stack name (active stack if empty)
     */
    void endMacro(const QString& stackName = QString());

    /**
     * @brief Get undo text
     * @param stackName Stack name (active stack if empty)
     */
    QString undoText(const QString& stackName = QString()) const;

    /**
     * @brief Get redo text
     * @param stackName Stack name (active stack if empty)
     */
    QString redoText(const QString& stackName = QString()) const;

    /**
     * @brief Get command count
     * @param stackName Stack name (active stack if empty)
     */
    int commandCount(const QString& stackName = QString()) const;

    /**
     * @brief Set clean index
     * @param stackName Stack name (active stack if empty)
     */
    void setClean(const QString& stackName = QString());

    /**
     * @brief Check if stack is clean
     * @param stackName Stack name (active stack if empty)
     */
    bool isClean(const QString& stackName = QString()) const;

signals:
    /**
     * @brief Emitted when command is executed
     */
    void commandExecuted(const QString& commandText, const QString& stackName);

    /**
     * @brief Emitted when undo is performed
     */
    void undoPerformed(const QString& commandText, const QString& stackName);

    /**
     * @brief Emitted when redo is performed
     */
    void redoPerformed(const QString& commandText, const QString& stackName);

    /**
     * @brief Emitted when stack clean state changes
     */
    void cleanChanged(bool clean, const QString& stackName);

private:
    explicit CommandManager(QObject* parent = nullptr);
    ~CommandManager() override = default;

    // Disable copy and assignment
    CommandManager(const CommandManager&) = delete;
    CommandManager& operator=(const CommandManager&) = delete;

    // Static instance
    static CommandManager* s_instance;

    // Command stacks
    QHash<QString, QUndoStack*> m_stacks;
    QUndoStack* m_defaultStack;
    QUndoStack* m_activeStack;

    // Thread safety
    mutable QMutex m_mutex;

    // Helper methods
    QUndoStack* getStackInternal(const QString& stackName) const;
    void connectStackSignals(QUndoStack* stack, const QString& stackName);
};

} // namespace Core
} // namespace ItemEditor

#endif // COMMANDMANAGER_H