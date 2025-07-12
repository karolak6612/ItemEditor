#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include "otb/otbtypes.h"

namespace UI {
namespace Dialogs {

/**
 * @brief Enhanced Find Items dialog with advanced filtering
 * 
 * Port of C# FindItemForm with comprehensive search capabilities
 * including text search, property filters, and advanced options.
 */
class EnhancedFindItems : public QDialog
{
    Q_OBJECT

public:
    explicit EnhancedFindItems(const OTB::ServerItemList& items, QWidget *parent = nullptr);
    virtual ~EnhancedFindItems();

    // Results
    QList<OTB::ServerItem*> getSelectedItems() const;
    OTB::ServerItem* getFirstSelectedItem() const;

signals:
    void itemSelected(OTB::ServerItem* item);
    void itemDoubleClicked(OTB::ServerItem* item);

private slots:
    void onSearchTextChanged();
    void onFilterChanged();
    void onAdvancedToggled(bool enabled);
    void onSearchClicked();
    void onClearClicked();
    void onItemSelectionChanged();
    void onItemDoubleClicked();

private:
    void setupUI();
    void setupConnections();
    void performSearch();
    void clearResults();
    void populateResults(const QList<OTB::ServerItem*>& results);
    bool matchesFilters(const OTB::ServerItem* item) const;
    
    // UI Components
    QGroupBox* m_searchGroup;
    QGroupBox* m_filtersGroup;
    QGroupBox* m_advancedGroup;
    QGroupBox* m_resultsGroup;
    
    // Search controls
    QLineEdit* m_searchEdit;
    QComboBox* m_searchTypeCombo;
    QCheckBox* m_caseSensitiveCheck;
    QCheckBox* m_exactMatchCheck;
    
    // Filter controls
    QComboBox* m_itemTypeFilter;
    QComboBox* m_stackOrderFilter;
    QSpinBox* m_minIdSpin;
    QSpinBox* m_maxIdSpin;
    QCheckBox* m_onlyMismatchedCheck;
    QCheckBox* m_onlyDeprecatedCheck;
    
    // Advanced filters
    QCheckBox* m_hasNameCheck;
    QCheckBox* m_hasSpritesCheck;
    QCheckBox* m_customCreatedCheck;
    
    // Flag filters
    QCheckBox* m_unpassableFilter;
    QCheckBox* m_movableFilter;
    QCheckBox* m_stackableFilter;
    QCheckBox* m_pickupableFilter;
    QCheckBox* m_readableFilter;
    
    // Results
    QListWidget* m_resultsList;
    QLabel* m_resultsCountLabel;
    
    // Buttons
    QPushButton* m_searchButton;
    QPushButton* m_clearButton;
    QPushButton* m_selectButton;
    QPushButton* m_cancelButton;
    
    // Data
    const OTB::ServerItemList& m_items;
    QList<OTB::ServerItem*> m_filteredResults;
    bool m_advancedMode;
};

} // namespace Dialogs
} // namespace UI