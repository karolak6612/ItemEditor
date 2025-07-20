#pragma once

#include "ProgressDialog.h"
#include <QWidget>
#include <memory>

/**
 * @brief Factory class for creating standardized progress dialogs
 * 
 * Provides pre-configured progress dialogs for common operations
 * matching the legacy system's progress dialog behavior.
 */
class ProgressDialogFactory
{
public:
    // File operation progress dialogs
    static std::unique_ptr<ProgressDialog> createFileLoadingDialog(QWidget* parent = nullptr);
    static std::unique_ptr<ProgressDialog> createFileSavingDialog(QWidget* parent = nullptr);
    static std::unique_ptr<ProgressDialog> createFileValidationDialog(QWidget* parent = nullptr);
    
    // Plugin operation progress dialogs
    static std::unique_ptr<ProgressDialog> createPluginLoadingDialog(QWidget* parent = nullptr);
    static std::unique_ptr<ProgressDialog> createClientDataLoadingDialog(QWidget* parent = nullptr);
    static std::unique_ptr<ProgressDialog> createSpriteProcessingDialog(QWidget* parent = nullptr);
    
    // Item operation progress dialogs
    static std::unique_ptr<ProgressDialog> createItemComparisonDialog(int totalItems, QWidget* parent = nullptr);
    static std::unique_ptr<ProgressDialog> createItemValidationDialog(int totalItems, QWidget* parent = nullptr);
    static std::unique_ptr<ProgressDialog> createItemReloadDialog(int totalItems, QWidget* parent = nullptr);
    
    // Batch operation progress dialogs
    static std::unique_ptr<ProgressDialog> createBatchOperationDialog(
        const QString& operation, int totalItems, const QString& itemName = "items", QWidget* parent = nullptr);
    
    // Generic progress dialogs
    static std::unique_ptr<ProgressDialog> createGenericDialog(
        const QString& title, const QString& operation, QWidget* parent = nullptr);
    static std::unique_ptr<ProgressDialog> createIndeterminateDialog(
        const QString& operation, QWidget* parent = nullptr);

private:
    // Helper methods for common configurations
    static void configureFileOperationDialog(ProgressDialog* dialog);
    static void configurePluginOperationDialog(ProgressDialog* dialog);
    static void configureItemOperationDialog(ProgressDialog* dialog);
    static void configureBatchOperationDialog(ProgressDialog* dialog, int totalItems);
};