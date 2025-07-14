/**
 * Item Editor Qt6 - Find Item Dialog Implementation
 * Exact mirror of Legacy_App/csharp/Source/Dialogs/FindItemForm.cs
 *
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "FindItemDialog.h"
#include "ui_FindItemDialog.h"
#include "../MainForm.h"
#include <QVBoxLayout>
#include <QGridLayout>

namespace ItemEditor {

FindItemDialog::FindItemDialog(MainForm* mainForm, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindItemDialog),
    m_mainForm(mainForm),
    m_properties(OTLib::Server::Items::ServerItemFlag::None)
{
    ui->setupUi(this);
    setupUi();
    updateProperties();
}

FindItemDialog::~FindItemDialog()
{
    delete ui;
}

void FindItemDialog::setupUi()
{
    setWindowTitle("Find Item");
    setFixedSize(434, 561);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    m_searchModeGroupBox = new QGroupBox(this);
    QVBoxLayout* searchModeLayout = new QVBoxLayout(m_searchModeGroupBox);
    m_findBySidRadioButton = new QRadioButton("Find By Server ID", m_searchModeGroupBox);
    m_findByCidRadioButton = new QRadioButton("Find By Client ID", m_searchModeGroupBox);
    m_findByPropertiesRadioButton = new QRadioButton("Find By Properties", m_searchModeGroupBox);
    searchModeLayout->addWidget(m_findBySidRadioButton);
    searchModeLayout->addWidget(m_findByCidRadioButton);
    searchModeLayout->addWidget(m_findByPropertiesRadioButton);
    m_findBySidRadioButton->setChecked(true);
    mainLayout->addWidget(m_searchModeGroupBox);

    connect(m_findBySidRadioButton, &QRadioButton::toggled, this, &FindItemDialog::onSearchModeChanged);
    connect(m_findByCidRadioButton, &QRadioButton::toggled, this, &FindItemDialog::onSearchModeChanged);
    connect(m_findByPropertiesRadioButton, &QRadioButton::toggled, this, &FindItemDialog::onSearchModeChanged);

    m_itemIdGroupBox = new QGroupBox("ID", this);
    QVBoxLayout* itemIdLayout = new QVBoxLayout(m_itemIdGroupBox);
    m_findIdSpinBox = new QSpinBox(m_itemIdGroupBox);
    m_findIdSpinBox->setRange(100, 65535);
    itemIdLayout->addWidget(m_findIdSpinBox);
    mainLayout->addWidget(m_itemIdGroupBox);

    m_propertiesGroupBox = new QGroupBox("Attributes", this);
    QGridLayout* propertiesLayout = new QGridLayout(m_propertiesGroupBox);
    QStringList flags = {
        "Unpassable", "Block Missiles", "Block Pathfinder", "Movable", "Pickupable",
        "Stackable", "Force Use", "Multi Use", "Rotatable", "Hangable", "Hook South",
        "Hook East", "Has Elevation", "Ignore Look", "Readable", "Full Ground"
    };
    OTLib::Server::Items::ServerItemFlag flagValues[] = {
        OTLib::Server::Items::ServerItemFlag::Unpassable, OTLib::Server::Items::ServerItemFlag::BlockMissiles,
        OTLib::Server::Items::ServerItemFlag::BlockPathfinder, OTLib::Server::Items::ServerItemFlag::Movable,
        OTLib::Server::Items::ServerItemFlag::Pickupable, OTLib::Server::Items::ServerItemFlag::Stackable,
        OTLib::Server::Items::ServerItemFlag::ForceUse, OTLib::Server::Items::ServerItemFlag::MultiUse,
        OTLib::Server::Items::ServerItemFlag::Rotatable, OTLib::Server::Items::ServerItemFlag::Hangable,
        OTLib::Server::Items::ServerItemFlag::HookSouth, OTLib::Server::Items::ServerItemFlag::HookEast,
        OTLib::Server::Items::ServerItemFlag::HasElevation, OTLib::Server::Items::ServerItemFlag::IgnoreLook,
        OTLib::Server::Items::ServerItemFlag::Readable, OTLib::Server::Items::ServerItemFlag::FullGround
    };

    for (int i = 0; i < flags.size(); ++i) {
        FlagCheckBox* checkbox = new FlagCheckBox(flags[i], this);
        checkbox->setServerItemFlag(flagValues[i]);
        m_flagCheckBoxes.append(checkbox);
        propertiesLayout->addWidget(checkbox, i / 2, i % 2);
        connect(checkbox, &FlagCheckBox::toggled, this, &FindItemDialog::onPropertyCheckBoxChanged);
    }
    mainLayout->addWidget(m_propertiesGroupBox);

    m_resultGroupBox = new QGroupBox("Result", this);
    QVBoxLayout* resultLayout = new QVBoxLayout(m_resultGroupBox);
    m_serverItemList = new ServerItemListBox(m_resultGroupBox);
    resultLayout->addWidget(m_serverItemList);
    mainLayout->addWidget(m_resultGroupBox);
    connect(m_serverItemList, &ServerItemListBox::itemSelectionChanged, this, &FindItemDialog::onServerItemSelected);

    m_findButton = new QPushButton("Find", this);
    mainLayout->addWidget(m_findButton);
    connect(m_findButton, &QPushButton::clicked, this, &FindItemDialog::onFindClicked);
}

void FindItemDialog::onSearchModeChanged()
{
    updateProperties();
}

void FindItemDialog::updateProperties()
{
    m_itemIdGroupBox->setEnabled(m_findBySidRadioButton->isChecked() || m_findByCidRadioButton->isChecked());
    m_propertiesGroupBox->setEnabled(m_findByPropertiesRadioButton->isChecked());

    if (m_findBySidRadioButton->isChecked()) {
        m_itemIdGroupBox->setTitle("Server ID");
    } else if (m_findByCidRadioButton->isChecked()) {
        m_itemIdGroupBox->setTitle("Client ID");
    }
}

void FindItemDialog::onFindClicked()
{
    m_serverItemList->clearSelection();
    QVariantMap searchParameters;
    if (m_findBySidRadioButton->isChecked()) {
        searchParameters["searchMode"] = "sid";
        searchParameters["id"] = m_findIdSpinBox->value();
    } else if (m_findByCidRadioButton->isChecked()) {
        searchParameters["searchMode"] = "cid";
        searchParameters["id"] = m_findIdSpinBox->value();
    } else if (m_findByPropertiesRadioButton->isChecked()) {
        searchParameters["searchMode"] = "properties";
        searchParameters["properties"] = static_cast<int>(m_properties);
    }

    OTLib::Collections::ServerItemList* foundItems = m_mainForm->findItems(searchParameters);
    m_serverItemList->addItems(foundItems->items());
}

void FindItemDialog::onPropertyCheckBoxChanged()
{
    m_properties = OTLib::Server::Items::ServerItemFlag::None;
    for (FlagCheckBox* checkbox : m_flagCheckBoxes) {
        if (checkbox->isChecked()) {
            m_properties |= checkbox->getServerItemFlag();
        }
    }
}

void FindItemDialog::onServerItemSelected()
{
    // This will be implemented later
}

} // namespace ItemEditor
