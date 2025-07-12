// ... (All existing includes and content of mainwindow.cpp from before this specific change) ...

// Add the new compareItems helper method
bool MainWindow::compareItems(const OTB::ServerItem* serverItem, const OTB::ClientItem* clientItem, bool compareHash)
{
    if (!serverItem || !clientItem) return false;
    if (serverItem->type == OTB::ServerItemType::Deprecated) return true; // Deprecated items are considered "matching" to not show up as mismatched

    if (compareHash) {
        // The C# code uses a mutable copy of the client item to calculate hash on demand.
        // Our ClientItem::getSpriteHash() is const but the dummy calculates on demand.
        // A real implementation might need a mutable copy if calculation modifies state.
        OTB::ClientItem mutableClientItem = *clientItem; // Make a copy to be safe
        if (serverItem->spriteHash != mutableClientItem.getSpriteHash()) {
            return false;
        }
    }
    // ServerItem::equals takes an ItemBase, which ClientItem inherits from
    return serverItem->equals(*clientItem);
}


// Replace the existing buildFilteredItemsList method
void MainWindow::buildFilteredItemsList()
{
    serverItemListBox->clear();
    listItemToServerItemMap.clear();

    // Disconnect selection changed signal to avoid updates while building list
    disconnect(serverItemListBox, &QListWidget::currentItemChanged, this, &MainWindow::onServerItemSelectionChanged);
    clearItemDetailsView();

    if (m_showOnlyMismatched && (!currentPlugin || !currentPlugin->isClientLoaded())) {
        QMessageBox::information(this, tr("Filter Warning"), tr("Cannot filter by mismatched items because no client data is loaded."));
        // Reset the flag and action if we can't honor the filter.
        m_showOnlyMismatched = false;
        showMismatchedAct->setChecked(false);
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    for (int i = 0; i < currentOtbItems.items.size(); ++i) {
        OTB::ServerItem* serverItem = &currentOtbItems.items[i];

        // Apply Deprecated Filter
        if (m_showOnlyDeprecated && serverItem->type != OTB::ServerItemType::Deprecated) {
            continue;
        }
        if (!m_showOnlyDeprecated && serverItem->type == OTB::ServerItemType::Deprecated) {
            continue;
        }

        // Apply Mismatch Filter
        if (m_showOnlyMismatched && currentPlugin && currentPlugin->isClientLoaded()) {
            OTB::ClientItem clientItem;
            bool hasClientItem = currentPlugin->getClientItem(serverItem->clientId, clientItem);

            bool isMatch = false;
            if (hasClientItem) {
                if (compareItems(serverItem, &clientItem, true)) {
                    isMatch = true;
                }
            }
            if (isMatch) {
                continue;
            }
        }

        QListWidgetItem *listItemWidget = new QListWidgetItem(QString("[%1] %2").arg(serverItem->id).arg(serverItem->name), serverItemListBox);
        listItemToServerItemMap.insert(listItemWidget, serverItem);
    }

    QApplication::restoreOverrideCursor();
    itemsCountLabel->setText(tr("%1 Items").arg(serverItemListBox->count()));

    // Reconnect signal
    connect(serverItemListBox, &QListWidget::currentItemChanged, this, &MainWindow::onServerItemSelectionChanged);

    if (serverItemListBox->count() > 0) {
        serverItemListBox->setCurrentRow(0); // This will trigger onServerItemSelectionChanged
    }
}

// ... (Rest of mainwindow.cpp remains the same) ...
// (This is a simplified representation of overwriting the file. The full file content must be provided to the tool)I will overwrite `mainwindow.cpp` with the necessary changes, which includes adding the `compareItems` helper method and replacing the existing `buildFilteredItemsList` method with the corrected version. The rest of the file will be preserved.
