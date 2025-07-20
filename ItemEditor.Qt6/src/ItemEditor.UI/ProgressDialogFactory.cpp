#include "ProgressDialogFactory.h"

std::unique_ptr<ProgressDialog> ProgressDialogFactory::createFileLoadingDialog(QWidget* parent)
{
    auto dialog = std::make_unique<ProgressDialog>("Loading file...", "Cancel", 0, 100, parent);
    configureFileOperationDialog(dialog.get());
    dialog->setOperation("Loading OTB file...");
    dialog->enableTimeEstimation(true);
    return dialog;
}

std::unique_ptr<ProgressDialog> ProgressDialogFactory::createFileSavingDialog(QWidget* parent)
{
    auto dialog = std::make_unique<ProgressDialog>("Saving file...", "Cancel", 0, 100, parent);
    configureFileOperationDialog(dialog.get());
    dialog->setOperation("Saving OTB file...");
    dialog->enableTimeEstimation(true);
    return dialog;
}

std::unique_ptr<ProgressDialog> ProgressDialogFactory::createFileValidationDialog(QWidget* parent)
{
    auto dialog = std::make_unique<ProgressDialog>("Validating file...", "Cancel", 0, 100, parent);
    configureFileOperationDialog(dialog.get());
    dialog->setOperation("Validating OTB file structure...");
    dialog->enableTimeEstimation(false); // Validation is usually quick
    return dialog;
}

std::unique_ptr<ProgressDialog> ProgressDialogFactory::createPluginLoadingDialog(QWidget* parent)
{
    auto dialog = std::make_unique<ProgressDialog>("Loading plugins...", "Cancel", 0, 100, parent);
    configurePluginOperationDialog(dialog.get());
    dialog->setOperation("Discovering and loading plugins...");
    dialog->enableTimeEstimation(false);
    return dialog;
}

std::unique_ptr<ProgressDialog> ProgressDialogFactory::createClientDataLoadingDialog(QWidget* parent)
{
    auto dialog = std::make_unique<ProgressDialog>("Loading client data...", "Cancel", 0, 100, parent);
    configurePluginOperationDialog(dialog.get());
    dialog->setOperation("Loading DAT and SPR files...");
    dialog->enableTimeEstimation(true);
    return dialog;
}

std::unique_ptr<ProgressDialog> ProgressDialogFactory::createSpriteProcessingDialog(QWidget* parent)
{
    auto dialog = std::make_unique<ProgressDialog>("Processing sprites...", "Cancel", 0, 100, parent);
    configurePluginOperationDialog(dialog.get());
    dialog->setOperation("Calculating sprite hashes and signatures...");
    dialog->enableTimeEstimation(true);
    return dialog;
}

std::unique_ptr<ProgressDialog> ProgressDialogFactory::createItemComparisonDialog(int totalItems, QWidget* parent)
{
    auto dialog = std::make_unique<ProgressDialog>("Comparing items...", "Cancel", 0, totalItems, parent);
    configureItemOperationDialog(dialog.get());
    configureBatchOperationDialog(dialog.get(), totalItems);
    dialog->setBatchOperation(totalItems, "items");
    dialog->setOperation("Comparing server and client items...");
    dialog->enableTimeEstimation(true);
    return dialog;
}

std::unique_ptr<ProgressDialog> ProgressDialogFactory::createItemValidationDialog(int totalItems, QWidget* parent)
{
    auto dialog = std::make_unique<ProgressDialog>("Validating items...", "Cancel", 0, totalItems, parent);
    configureItemOperationDialog(dialog.get());
    configureBatchOperationDialog(dialog.get(), totalItems);
    dialog->setBatchOperation(totalItems, "items");
    dialog->setOperation("Validating item properties...");
    dialog->enableTimeEstimation(true);
    return dialog;
}

std::unique_ptr<ProgressDialog> ProgressDialogFactory::createItemReloadDialog(int totalItems, QWidget* parent)
{
    auto dialog = std::make_unique<ProgressDialog>("Reloading items...", "Cancel", 0, totalItems, parent);
    configureItemOperationDialog(dialog.get());
    configureBatchOperationDialog(dialog.get(), totalItems);
    dialog->setBatchOperation(totalItems, "items");
    dialog->setOperation("Reloading item data from client...");
    dialog->enableTimeEstimation(true);
    return dialog;
}

std::unique_ptr<ProgressDialog> ProgressDialogFactory::createBatchOperationDialog(
    const QString& operation, int totalItems, const QString& itemName, QWidget* parent)
{
    auto dialog = std::make_unique<ProgressDialog>(operation, "Cancel", 0, totalItems, parent);
    configureItemOperationDialog(dialog.get());
    configureBatchOperationDialog(dialog.get(), totalItems);
    dialog->setBatchOperation(totalItems, itemName);
    dialog->setOperation(operation);
    dialog->enableTimeEstimation(true);
    return dialog;
}

std::unique_ptr<ProgressDialog> ProgressDialogFactory::createGenericDialog(
    const QString& title, const QString& operation, QWidget* parent)
{
    auto dialog = std::make_unique<ProgressDialog>(operation, "Cancel", 0, 100, parent);
    dialog->setWindowTitle(title);
    dialog->setOperation(operation);
    dialog->enableTimeEstimation(true);
    dialog->setCancellationEnabled(true);
    return dialog;
}

std::unique_ptr<ProgressDialog> ProgressDialogFactory::createIndeterminateDialog(
    const QString& operation, QWidget* parent)
{
    auto dialog = std::make_unique<ProgressDialog>(operation, "Cancel", 0, 0, parent);
    dialog->setOperation(operation);
    dialog->enableTimeEstimation(false);
    dialog->setCancellationEnabled(true);
    return dialog;
}

void ProgressDialogFactory::configureFileOperationDialog(ProgressDialog* dialog)
{
    if (!dialog) return;
    
    dialog->setWindowTitle("File Operation");
    dialog->setCancellationEnabled(true);
    dialog->setAutoClose(false); // Let user see completion
    
    // File operations can be cancelled but should ask for confirmation
    dialog->setCancellationCallback([]() {
        // Cleanup callback would be set by the calling code
    });
}

void ProgressDialogFactory::configurePluginOperationDialog(ProgressDialog* dialog)
{
    if (!dialog) return;
    
    dialog->setWindowTitle("Plugin Operation");
    dialog->setCancellationEnabled(true);
    dialog->setAutoClose(true, 500); // Auto-close quickly for plugin operations
}

void ProgressDialogFactory::configureItemOperationDialog(ProgressDialog* dialog)
{
    if (!dialog) return;
    
    dialog->setWindowTitle("Item Operation");
    dialog->setCancellationEnabled(true);
    dialog->setAutoClose(false); // Let user see results
}

void ProgressDialogFactory::configureBatchOperationDialog(ProgressDialog* dialog, int totalItems)
{
    if (!dialog) return;
    
    // For large batch operations, enable all features
    if (totalItems > 100) {
        dialog->enableTimeEstimation(true);
        dialog->setCancellationEnabled(true);
    } else {
        dialog->enableTimeEstimation(false);
        dialog->setCancellationEnabled(totalItems > 10);
    }
}