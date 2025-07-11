#include "finditemdialog.h"
#include "otb/otbtypes.h" // For OTB::ServerItemFlag namespace

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QListWidget>
#include <QPushButton>
#include <QMessageBox> // For later use if needed

FindItemDialog::FindItemDialog(const OTB::ServerItemList& allItems, QWidget *parent)
    : QDialog(parent), m_allItems(allItems), m_selectedServerIdInDialog(0)
{
    setWindowTitle(tr("Find Item"));
    setMinimumWidth(500); // Adjusted width
    setupUi();

    // Initial state
    searchCriteriaModeChanged(); // Set initial visibility of input fields
    goToItemButton->setEnabled(false);
}

FindItemDialog::~FindItemDialog()
{
}

void FindItemDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // --- Search Mode Group ---
    searchModeGroupBox = new QGroupBox(tr("Search By"), this);
    QHBoxLayout* searchModeLayout = new QHBoxLayout(searchModeGroupBox);
    findBySidRadioButton = new QRadioButton(tr("Server ID"), this);
    findByCidRadioButton = new QRadioButton(tr("Client ID"), this);
    findByNameRadioButton = new QRadioButton(tr("Name"), this);
    findByFlagsRadioButton = new QRadioButton(tr("Flags"), this);
    searchModeLayout->addWidget(findBySidRadioButton);
    searchModeLayout->addWidget(findByCidRadioButton);
    searchModeLayout->addWidget(findByNameRadioButton);
    searchModeLayout->addWidget(findByFlagsRadioButton);
    findBySidRadioButton->setChecked(true); // Default search mode
    mainLayout->addWidget(searchModeGroupBox);

    connect(findBySidRadioButton, &QRadioButton::toggled, this, &FindItemDialog::searchCriteriaModeChanged);
    connect(findByCidRadioButton, &QRadioButton::toggled, this, &FindItemDialog::searchCriteriaModeChanged);
    connect(findByNameRadioButton, &QRadioButton::toggled, this, &FindItemDialog::searchCriteriaModeChanged);
    connect(findByFlagsRadioButton, &QRadioButton::toggled, this, &FindItemDialog::searchCriteriaModeChanged);

    // --- Value Input Group (for ID or Name) ---
    valueInputGroupBox = new QGroupBox(tr("Search Value"), this);
    QGridLayout* valueInputLayout = new QGridLayout(valueInputGroupBox);
    valueLabel = new QLabel(tr("Server ID:"), this); // Text will change
    idSpinBox = new QSpinBox(this);
    idSpinBox->setRange(0, 65535);
    nameLineEdit = new QLineEdit(this);
    nameLineEdit->setVisible(false); // Initially hidden
    valueInputLayout->addWidget(valueLabel, 0, 0);
    valueInputLayout->addWidget(idSpinBox, 0, 1);
    valueInputLayout->addWidget(nameLineEdit, 0, 1); // Will toggle visibility with idSpinBox
    mainLayout->addWidget(valueInputGroupBox);

    // --- Flags Group ---
    flagsGroupBox = new QGroupBox(tr("Item Flags (must have all checked)"), this);
    flagsLayout = new QGridLayout(flagsGroupBox);
    flagsGroupBox->setVisible(false); // Initially hidden

    // Helper for adding checkboxes to the grid
    int flag_row = 0, flag_col = 0;
    auto addFlagSearchCheckBox = [&](QCheckBox*& checkBox, const QString& label) {
        checkBox = new QCheckBox(label, this);
        flagsLayout->addWidget(checkBox, flag_row, flag_col);
        flag_col++;
        if (flag_col >= 3) { // 3 columns for flags
            flag_col = 0;
            flag_row++;
        }
    };

    addFlagSearchCheckBox(unpassableCheckBox, tr("Unpassable"));
    addFlagSearchCheckBox(blockMissilesCheckBox, tr("Block Missiles"));
    addFlagSearchCheckBox(blockPathfinderCheckBox, tr("Block Pathfinder"));
    addFlagSearchCheckBox(hasElevationCheckBox, tr("Has Elevation"));
    addFlagSearchCheckBox(forceUseCheckBox, tr("Force Use"));
    addFlagSearchCheckBox(multiUseCheckBox, tr("Multi Use"));
    addFlagSearchCheckBox(pickupableCheckBox, tr("Pickupable"));
    addFlagSearchCheckBox(movableCheckBox, tr("Movable"));
    addFlagSearchCheckBox(stackableCheckBox, tr("Stackable"));
    addFlagSearchCheckBox(readableCheckBox, tr("Readable"));
    addFlagSearchCheckBox(rotatableCheckBox, tr("Rotatable"));
    addFlagSearchCheckBox(hangableCheckBox, tr("Hangable"));
    addFlagSearchCheckBox(hookSouthCheckBox, tr("Hook South"));
    addFlagSearchCheckBox(hookEastCheckBox, tr("Hook East"));
    addFlagSearchCheckBox(ignoreLookCheckBox, tr("Ignore Look"));
    addFlagSearchCheckBox(fullGroundCheckBox, tr("Full Ground"));
    addFlagSearchCheckBox(allowDistReadCheckBox, tr("Allow Dist. Read"));
    addFlagSearchCheckBox(hasChargesCheckBox, tr("Has Charges"));
    addFlagSearchCheckBox(isAnimationCheckBox, tr("Is Animation"));

    if (flag_col != 0) { // Ensure next row starts clear if last row wasn't full
        flag_row++; flag_col = 0;
    }
    flagsLayout->setRowStretch(flag_row, 1); // Add stretch at the bottom

    mainLayout->addWidget(flagsGroupBox);

    // --- Results Group ---
    resultsGroupBox = new QGroupBox(tr("Results"), this);
    QVBoxLayout* resultsLayout = new QVBoxLayout(resultsGroupBox);
    resultsListWidget = new QListWidget(this);
    resultsLayout->addWidget(resultsListWidget);
    mainLayout->addWidget(resultsGroupBox);
    resultsGroupBox->setMinimumHeight(150);

    connect(resultsListWidget, &QListWidget::itemSelectionChanged, this, &FindItemDialog::resultItemSelected);
    connect(resultsListWidget, &QListWidget::itemDoubleClicked, this, &FindItemDialog::acceptFoundItem);


    // --- Action Buttons ---
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    findButton = new QPushButton(tr("Find"), this);
    goToItemButton = new QPushButton(tr("Go to Item"), this);
    cancelButton = new QPushButton(tr("Cancel"), this);
    buttonsLayout->addWidget(findButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(goToItemButton);
    buttonsLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonsLayout);

    connect(findButton, &QPushButton::clicked, this, &FindItemDialog::findItems);
    connect(goToItemButton, &QPushButton::clicked, this, &FindItemDialog::acceptFoundItem);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    setLayout(mainLayout);
}


void FindItemDialog::searchCriteriaModeChanged()
{
    if (findBySidRadioButton->isChecked() || findByCidRadioButton->isChecked()) {
        valueLabel->setText(findBySidRadioButton->isChecked() ? tr("Server ID:") : tr("Client ID:"));
        idSpinBox->setVisible(true);
        nameLineEdit->setVisible(false);
        flagsGroupBox->setVisible(false);
        valueInputGroupBox->setTitle(tr("ID Value"));
    } else if (findByNameRadioButton->isChecked()) {
        valueLabel->setText(tr("Name (contains):"));
        idSpinBox->setVisible(false);
        nameLineEdit->setVisible(true);
        flagsGroupBox->setVisible(false);
        valueInputGroupBox->setTitle(tr("Name Value"));
    } else if (findByFlagsRadioButton->isChecked()) {
        idSpinBox->setVisible(false);
        nameLineEdit->setVisible(false);
        flagsGroupBox->setVisible(true);
        valueInputGroupBox->setTitle(tr("Value (N/A for Flags search)"));
        valueInputGroupBox->setEnabled(false); // No direct value for flags only search
    }
    valueInputGroupBox->setEnabled(!(findByFlagsRadioButton->isChecked()));
}

void FindItemDialog::findItems()
{
    // Implementation later
    resultsListWidget->clear();
    m_resultItemToServerIdMap.clear();
    m_selectedServerIdInDialog = 0;
    goToItemButton->setEnabled(false);

    QString searchText;
    int searchId = 0;
    bool searchById = false;

    if (findBySidRadioButton->isChecked() || findByCidRadioButton->isChecked()) {
        searchId = idSpinBox->value();
        searchById = true;
    } else if (findByNameRadioButton->isChecked()) {
        searchText = nameLineEdit->text().trimmed();
        if (searchText.isEmpty() && !findByFlagsRadioButton->isChecked()) { // Don't search on empty name unless flags also used
            return;
        }
    }

    for (const OTB::ServerItem& item : m_allItems.items) {
        bool match = false;
        if (findBySidRadioButton->isChecked()) {
            if (item.id == searchId) match = true;
        } else if (findByCidRadioButton->isChecked()) {
            if (item.clientId == searchId) match = true;
        } else if (findByNameRadioButton->isChecked()) {
            if (item.name.contains(searchText, Qt::CaseInsensitive)) match = true;
        } else if (findByFlagsRadioButton->isChecked()) {
            match = true; // Start with true, then AND with flag checks
        }

        if (match && findByFlagsRadioButton->isChecked()) { // If flag search is active, further refine
            // Ensure item's boolean properties are up-to-date with its flags field
            // This is important if the ServerItem instances in m_allItems might not have had updatePropertiesFromFlags() called recently.
            // However, m_allItems is const& so we can't modify. Assume they are correct.
            // Or, create a temporary copy to call updatePropertiesFromFlags if needed for search.
            // For now, assume direct flag checks on item.flags or boolean members are okay.
            // Let's use the boolean members as they are more direct if updatePropertiesFromFlags() was called after load/edit.
            // To be perfectly safe, one might check item.hasFlag(OTB::ServerItemFlag::Unpassable) instead of item.unpassable directly.
            // For this implementation, I'll use the boolean members.

            if (unpassableCheckBox->isChecked() && !item.unpassable) match = false;
            if (match && blockMissilesCheckBox->isChecked() && !item.blockMissiles) match = false;
            if (match && blockPathfinderCheckBox->isChecked() && !item.blockPathfinder) match = false;
            if (match && hasElevationCheckBox->isChecked() && !item.hasElevation) match = false;
            if (match && forceUseCheckBox->isChecked() && !item.forceUse) match = false;
            if (match && multiUseCheckBox->isChecked() && !item.multiUse) match = false;
            if (match && pickupableCheckBox->isChecked() && !item.pickupable) match = false;
            if (match && movableCheckBox->isChecked() && !item.movable) match = false;
            if (match && stackableCheckBox->isChecked() && !item.stackable) match = false;
            if (match && readableCheckBox->isChecked() && !item.readable) match = false;
            if (match && rotatableCheckBox->isChecked() && !item.rotatable) match = false;
            if (match && hangableCheckBox->isChecked() && !item.hangable) match = false;
            if (match && hookSouthCheckBox->isChecked() && !item.hookSouth) match = false;
            if (match && hookEastCheckBox->isChecked() && !item.hookEast) match = false;
            if (match && ignoreLookCheckBox->isChecked() && !item.ignoreLook) match = false;
            if (match && fullGroundCheckBox->isChecked() && !item.fullGround) match = false;
            if (match && allowDistReadCheckBox->isChecked() && !item.allowDistanceRead) match = false;
            if (match && hasChargesCheckBox->isChecked() && !item.hasCharges) match = false;
            if (match && isAnimationCheckBox->isChecked() && !item.isAnimation) match = false;
        }

        // If only searching by name and name is empty, but flags are NOT being searched, don't add all items.
        if (findByNameRadioButton->isChecked() && searchText.isEmpty() && !findByFlagsRadioButton->isChecked()){
             match = false;
        }


        if (match) {
            QListWidgetItem* listItem = new QListWidgetItem(
                QString("[%1] %2 (CID: %3)").arg(item.id).arg(item.name).arg(item.clientId),
                resultsListWidget
            );
            m_resultItemToServerIdMap.insert(listItem, item.id);
        }
    }

    if (resultsListWidget->count() > 0) {
        resultsListWidget->setCurrentRow(0); // Select first result
    }
}

void FindItemDialog::resultItemSelected()
{
    QListWidgetItem* current = resultsListWidget->currentItem();
    if (current) {
        m_selectedServerIdInDialog = m_resultItemToServerIdMap.value(current, 0);
        goToItemButton->setEnabled(m_selectedServerIdInDialog != 0);
    } else {
        m_selectedServerIdInDialog = 0;
        goToItemButton->setEnabled(false);
    }
}

void FindItemDialog::acceptFoundItem()
{
    if (m_selectedServerIdInDialog != 0) {
        accept(); // QDialog::accept() will close the dialog with Accepted state
    }
}

quint16 FindItemDialog::getSelectedServerId() const
{
    return m_selectedServerIdInDialog;
}
