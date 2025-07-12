#include "ui/dialogs/enhancedfinditems.h"
#include <QSplitter>
#include <QHeaderView>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>
#include <QMetaEnum>

namespace UI {
namespace Dialogs {

EnhancedFindItems::EnhancedFindItems(const OTB::ServerItemList& items, QWidget *parent)
    : QDialog(parent)
    , m_items(items)
    , m_advancedMode(false)
{
    setWindowTitle("Find Items - Advanced Search");
    setModal(true);
    resize(800, 600);
    
    setupUI();
    setupConnections();
    
    // Focus on search field
    m_searchEdit->setFocus();
}

EnhancedFindItems::~EnhancedFindItems()
{
    // Qt handles cleanup automatically
}

void EnhancedFindItems::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    
    // Create splitter for resizable layout
    auto* splitter = new QSplitter(Qt::Vertical, this);
    
    // Search Group
    m_searchGroup = new QGroupBox("Search", this);
    auto* searchLayout = new QGridLayout(m_searchGroup);
    
    searchLayout->addWidget(new QLabel("Search for:", this), 0, 0);
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Enter item name, ID, or property...");
    searchLayout->addWidget(m_searchEdit, 0, 1, 1, 2);
    
    searchLayout->addWidget(new QLabel("Search in:", this), 1, 0);
    m_searchTypeCombo = new QComboBox(this);
    m_searchTypeCombo->addItems({"Name", "ID", "Client ID", "All Properties"});
    searchLayout->addWidget(m_searchTypeCombo, 1, 1);
    
    m_caseSensitiveCheck = new QCheckBox("Case sensitive", this);
    searchLayout->addWidget(m_caseSensitiveCheck, 1, 2);
    
    m_exactMatchCheck = new QCheckBox("Exact match", this);
    searchLayout->addWidget(m_exactMatchCheck, 2, 1);
    
    splitter->addWidget(m_searchGroup);
    
    // Filters Group
    m_filtersGroup = new QGroupBox("Filters", this);
    auto* filtersLayout = new QGridLayout(m_filtersGroup);
    
    // Basic filters
    filtersLayout->addWidget(new QLabel("Item Type:", this), 0, 0);
    m_itemTypeFilter = new QComboBox(this);
    m_itemTypeFilter->addItems({"All", "None", "Ground", "Container", "Splash", "Fluid", "Deprecated"});
    filtersLayout->addWidget(m_itemTypeFilter, 0, 1);
    
    filtersLayout->addWidget(new QLabel("Stack Order:", this), 0, 2);
    m_stackOrderFilter = new QComboBox(this);
    m_stackOrderFilter->addItems({"All", "None", "Border", "Ground", "Bottom", "Top", "Creature"});
    filtersLayout->addWidget(m_stackOrderFilter, 0, 3);
    
    // ID range
    filtersLayout->addWidget(new QLabel("ID Range:", this), 1, 0);
    m_minIdSpin = new QSpinBox(this);
    m_minIdSpin->setRange(0, 99999);
    m_minIdSpin->setValue(0);
    filtersLayout->addWidget(m_minIdSpin, 1, 1);
    
    filtersLayout->addWidget(new QLabel("to", this), 1, 2);
    m_maxIdSpin = new QSpinBox(this);
    m_maxIdSpin->setRange(0, 99999);
    m_maxIdSpin->setValue(99999);
    filtersLayout->addWidget(m_maxIdSpin, 1, 3);
    
    // Special filters
    m_onlyMismatchedCheck = new QCheckBox("Only mismatched items", this);
    filtersLayout->addWidget(m_onlyMismatchedCheck, 2, 0, 1, 2);
    
    m_onlyDeprecatedCheck = new QCheckBox("Only deprecated items", this);
    filtersLayout->addWidget(m_onlyDeprecatedCheck, 2, 2, 1, 2);
    
    splitter->addWidget(m_filtersGroup);
    
    // Advanced Group (initially hidden)
    m_advancedGroup = new QGroupBox("Advanced Filters", this);
    auto* advancedLayout = new QGridLayout(m_advancedGroup);
    
    // Property filters
    m_hasNameCheck = new QCheckBox("Has name", this);
    advancedLayout->addWidget(m_hasNameCheck, 0, 0);
    
    m_hasSpritesCheck = new QCheckBox("Has sprites", this);
    advancedLayout->addWidget(m_hasSpritesCheck, 0, 1);
    
    m_customCreatedCheck = new QCheckBox("Custom created", this);
    advancedLayout->addWidget(m_customCreatedCheck, 0, 2);
    
    // Flag filters
    advancedLayout->addWidget(new QLabel("Item Flags:", this), 1, 0, 1, 3);
    
    m_unpassableFilter = new QCheckBox("Unpassable", this);
    advancedLayout->addWidget(m_unpassableFilter, 2, 0);
    
    m_movableFilter = new QCheckBox("Movable", this);
    advancedLayout->addWidget(m_movableFilter, 2, 1);
    
    m_stackableFilter = new QCheckBox("Stackable", this);
    advancedLayout->addWidget(m_stackableFilter, 2, 2);
    
    m_pickupableFilter = new QCheckBox("Pickupable", this);
    advancedLayout->addWidget(m_pickupableFilter, 3, 0);
    
    m_readableFilter = new QCheckBox("Readable", this);
    advancedLayout->addWidget(m_readableFilter, 3, 1);
    
    m_advancedGroup->setVisible(false);
    splitter->addWidget(m_advancedGroup);
    
    // Results Group
    m_resultsGroup = new QGroupBox("Results", this);
    auto* resultsLayout = new QVBoxLayout(m_resultsGroup);
    
    m_resultsCountLabel = new QLabel("0 items found", this);
    resultsLayout->addWidget(m_resultsCountLabel);
    
    m_resultsList = new QListWidget(this);
    m_resultsList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    resultsLayout->addWidget(m_resultsList);
    
    splitter->addWidget(m_resultsGroup);
    
    // Set splitter proportions
    splitter->setStretchFactor(0, 0); // Search group - fixed
    splitter->setStretchFactor(1, 0); // Filters group - fixed  
    splitter->setStretchFactor(2, 0); // Advanced group - fixed
    splitter->setStretchFactor(3, 1); // Results group - expandable
    
    mainLayout->addWidget(splitter);
    
    // Button layout
    auto* buttonLayout = new QHBoxLayout();
    
    auto* advancedButton = new QPushButton("Advanced >>", this);
    advancedButton->setCheckable(true);
    connect(advancedButton, &QPushButton::toggled, this, &EnhancedFindItems::onAdvancedToggled);
    buttonLayout->addWidget(advancedButton);
    
    buttonLayout->addStretch();
    
    m_searchButton = new QPushButton("Search", this);
    m_searchButton->setDefault(true);
    buttonLayout->addWidget(m_searchButton);
    
    m_clearButton = new QPushButton("Clear", this);
    buttonLayout->addWidget(m_clearButton);
    
    buttonLayout->addSpacing(20);
    
    m_selectButton = new QPushButton("Select", this);
    m_selectButton->setEnabled(false);
    buttonLayout->addWidget(m_selectButton);
    
    m_cancelButton = new QPushButton("Cancel", this);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
}

void EnhancedFindItems::setupConnections()
{
    // Search controls
    connect(m_searchEdit, &QLineEdit::textChanged, this, &EnhancedFindItems::onSearchTextChanged);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &EnhancedFindItems::onSearchClicked);
    
    // Filter controls
    connect(m_searchTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EnhancedFindItems::onFilterChanged);
    connect(m_itemTypeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EnhancedFindItems::onFilterChanged);
    connect(m_stackOrderFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EnhancedFindItems::onFilterChanged);
    connect(m_caseSensitiveCheck, &QCheckBox::toggled, this, &EnhancedFindItems::onFilterChanged);
    connect(m_exactMatchCheck, &QCheckBox::toggled, this, &EnhancedFindItems::onFilterChanged);
    connect(m_onlyMismatchedCheck, &QCheckBox::toggled, this, &EnhancedFindItems::onFilterChanged);
    connect(m_onlyDeprecatedCheck, &QCheckBox::toggled, this, &EnhancedFindItems::onFilterChanged);
    
    // Advanced filters
    connect(m_hasNameCheck, &QCheckBox::toggled, this, &EnhancedFindItems::onFilterChanged);
    connect(m_hasSpritesCheck, &QCheckBox::toggled, this, &EnhancedFindItems::onFilterChanged);
    connect(m_customCreatedCheck, &QCheckBox::toggled, this, &EnhancedFindItems::onFilterChanged);
    connect(m_unpassableFilter, &QCheckBox::toggled, this, &EnhancedFindItems::onFilterChanged);
    connect(m_movableFilter, &QCheckBox::toggled, this, &EnhancedFindItems::onFilterChanged);
    connect(m_stackableFilter, &QCheckBox::toggled, this, &EnhancedFindItems::onFilterChanged);
    connect(m_pickupableFilter, &QCheckBox::toggled, this, &EnhancedFindItems::onFilterChanged);
    connect(m_readableFilter, &QCheckBox::toggled, this, &EnhancedFindItems::onFilterChanged);
    
    // Results
    connect(m_resultsList, &QListWidget::itemSelectionChanged, this, &EnhancedFindItems::onItemSelectionChanged);
    connect(m_resultsList, &QListWidget::itemDoubleClicked, this, &EnhancedFindItems::onItemDoubleClicked);
    
    // Buttons
    connect(m_searchButton, &QPushButton::clicked, this, &EnhancedFindItems::onSearchClicked);
    connect(m_clearButton, &QPushButton::clicked, this, &EnhancedFindItems::onClearClicked);
    connect(m_selectButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void EnhancedFindItems::performSearch()
{
    QString searchText = m_searchEdit->text().trimmed();
    if (searchText.isEmpty()) {
        clearResults();
        return;
    }
    
    QList<OTB::ServerItem*> results;
    Qt::CaseSensitivity caseSensitivity = m_caseSensitiveCheck->isChecked() ? 
        Qt::CaseSensitive : Qt::CaseInsensitive;
    
    for (auto& item : m_items.items) {
        if (!matchesFilters(&item)) {
            continue;
        }
        
        bool matches = false;
        int searchType = m_searchTypeCombo->currentIndex();
        
        switch (searchType) {
            case 0: // Name
                if (m_exactMatchCheck->isChecked()) {
                    matches = item.name.compare(searchText, caseSensitivity) == 0;
                } else {
                    matches = item.name.contains(searchText, caseSensitivity);
                }
                break;
                
            case 1: // ID
                matches = QString::number(item.id).contains(searchText);
                break;
                
            case 2: // Client ID
                matches = QString::number(item.clientId).contains(searchText);
                break;
                
            case 3: // All Properties
                matches = item.name.contains(searchText, caseSensitivity) ||
                         QString::number(item.id).contains(searchText) ||
                         QString::number(item.clientId).contains(searchText);
                break;
        }
        
        if (matches) {
            results.append(const_cast<OTB::ServerItem*>(&item));
        }
    }
    
    populateResults(results);
}

bool EnhancedFindItems::matchesFilters(const OTB::ServerItem* item) const
{
    // Item type filter
    int typeFilter = m_itemTypeFilter->currentIndex();
    if (typeFilter > 0 && static_cast<int>(item->type) != (typeFilter - 1)) {
        return false;
    }
    
    // Stack order filter
    int stackFilter = m_stackOrderFilter->currentIndex();
    if (stackFilter > 0 && static_cast<int>(item->stackOrder) != (stackFilter - 1)) {
        return false;
    }
    
    // ID range filter
    if (item->id < m_minIdSpin->value() || item->id > m_maxIdSpin->value()) {
        return false;
    }
    
    // Special filters
    if (m_onlyDeprecatedCheck->isChecked() && item->type != OTB::ServerItemType::Deprecated) {
        return false;
    }
    
    // Advanced filters (only if advanced mode is enabled)
    if (m_advancedMode) {
        if (m_hasNameCheck->isChecked() && item->name.isEmpty()) {
            return false;
        }
        
        if (m_customCreatedCheck->isChecked() && !item->isCustomCreated) {
            return false;
        }
        
        // Flag filters
        if (m_unpassableFilter->isChecked() && !item->unpassable) {
            return false;
        }
        if (m_movableFilter->isChecked() && !item->movable) {
            return false;
        }
        if (m_stackableFilter->isChecked() && !item->stackable) {
            return false;
        }
        if (m_pickupableFilter->isChecked() && !item->pickupable) {
            return false;
        }
        if (m_readableFilter->isChecked() && !item->readable) {
            return false;
        }
    }
    
    return true;
}

void EnhancedFindItems::populateResults(const QList<OTB::ServerItem*>& results)
{
    m_filteredResults = results;
    m_resultsList->clear();
    
    for (const auto* item : results) {
        QString displayText = QString("ID: %1 | Client: %2 | %3 | %4")
            .arg(item->id)
            .arg(item->clientId)
            .arg(item->name.isEmpty() ? "Unnamed" : item->name)
            .arg([](OTB::ServerItemType type) -> QString {
                switch(type) {
                    case OTB::ServerItemType::None: return "None";
                    case OTB::ServerItemType::Ground: return "Ground";
                    case OTB::ServerItemType::Container: return "Container";
                    case OTB::ServerItemType::Splash: return "Splash";
                    case OTB::ServerItemType::Fluid: return "Fluid";
                    case OTB::ServerItemType::Deprecated: return "Deprecated";
                    default: return "Unknown";
                }
            }(item->type));
            
        auto* listItem = new QListWidgetItem(displayText);
        listItem->setData(Qt::UserRole, QVariant::fromValue(const_cast<OTB::ServerItem*>(item)));
        m_resultsList->addItem(listItem);
    }
    
    m_resultsCountLabel->setText(QString("%1 items found").arg(results.size()));
}

void EnhancedFindItems::clearResults()
{
    m_resultsList->clear();
    m_filteredResults.clear();
    m_resultsCountLabel->setText("0 items found");
    m_selectButton->setEnabled(false);
}

QList<OTB::ServerItem*> EnhancedFindItems::getSelectedItems() const
{
    QList<OTB::ServerItem*> selected;
    for (auto* item : m_resultsList->selectedItems()) {
        auto* serverItem = item->data(Qt::UserRole).value<OTB::ServerItem*>();
        if (serverItem) {
            selected.append(serverItem);
        }
    }
    return selected;
}

OTB::ServerItem* EnhancedFindItems::getFirstSelectedItem() const
{
    auto selected = getSelectedItems();
    return selected.isEmpty() ? nullptr : selected.first();
}

// Slot implementations
void EnhancedFindItems::onSearchTextChanged()
{
    // Auto-search as user types (with small delay)
    if (!m_searchEdit->text().trimmed().isEmpty()) {
        performSearch();
    } else {
        clearResults();
    }
}

void EnhancedFindItems::onFilterChanged()
{
    // Re-run search when filters change
    if (!m_searchEdit->text().trimmed().isEmpty()) {
        performSearch();
    }
}

void EnhancedFindItems::onAdvancedToggled(bool enabled)
{
    m_advancedMode = enabled;
    m_advancedGroup->setVisible(enabled);
    
    auto* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        button->setText(enabled ? "<< Advanced" : "Advanced >>");
    }
    
    // Re-run search if text is present
    if (!m_searchEdit->text().trimmed().isEmpty()) {
        performSearch();
    }
}

void EnhancedFindItems::onSearchClicked()
{
    performSearch();
}

void EnhancedFindItems::onClearClicked()
{
    m_searchEdit->clear();
    clearResults();
}

void EnhancedFindItems::onItemSelectionChanged()
{
    bool hasSelection = !m_resultsList->selectedItems().isEmpty();
    m_selectButton->setEnabled(hasSelection);
    
    if (hasSelection) {
        auto* item = getFirstSelectedItem();
        if (item) {
            emit itemSelected(item);
        }
    }
}

void EnhancedFindItems::onItemDoubleClicked()
{
    auto* item = getFirstSelectedItem();
    if (item) {
        emit itemDoubleClicked(item);
        accept();
    }
}

} // namespace Dialogs
} // namespace UI