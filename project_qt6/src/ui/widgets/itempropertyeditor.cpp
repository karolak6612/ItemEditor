#include "ui/widgets/itempropertyeditor.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QToolTip>
#include <QDebug>

namespace UI {
namespace Widgets {

ItemPropertyEditor::ItemPropertyEditor(QWidget *parent)
    : QWidget(parent)
    , m_currentServerItem(nullptr)
    , m_currentClientItem(nullptr)
    , m_updatingControls(false)
    , m_normalTextColor(QApplication::palette().color(QPalette::Text))
    , m_mismatchTextColor(Qt::red)
{
    setupUI();
    setupConnections();
}

ItemPropertyEditor::~ItemPropertyEditor()
{
    // Qt handles cleanup automatically
}

void ItemPropertyEditor::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    
    // Appearance Group (matches C# appearanceGroupBox)
    m_appearanceGroup = new QGroupBox("Appearance", this);
    auto* appearanceLayout = new QGridLayout(m_appearanceGroup);
    
    // Server ID (read-only)
    appearanceLayout->addWidget(new QLabel("Server ID:", this), 0, 0);
    m_serverIdLabel = new QLabel("0", this);
    m_serverIdLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    appearanceLayout->addWidget(m_serverIdLabel, 0, 1);
    
    // Client ID
    appearanceLayout->addWidget(new QLabel("Client ID:", this), 1, 0);
    m_clientIdSpinBox = new QSpinBox(this);
    m_clientIdSpinBox->setRange(100, 99999);
    appearanceLayout->addWidget(m_clientIdSpinBox, 1, 1);
    
    // Type
    appearanceLayout->addWidget(new QLabel("Type:", this), 2, 0);
    m_typeComboBox = new QComboBox(this);
    m_typeComboBox->addItems({"None", "Ground", "Container", "Splash", "Fluid", "Deprecated"});
    appearanceLayout->addWidget(m_typeComboBox, 2, 1);
    
    // Stack Order
    appearanceLayout->addWidget(new QLabel("Stack Order:", this), 3, 0);
    m_stackOrderComboBox = new QComboBox(this);
    m_stackOrderComboBox->addItems({"None", "Border", "Ground", "Bottom", "Top", "Creature"});
    appearanceLayout->addWidget(m_stackOrderComboBox, 3, 1);
    
    // Name
    appearanceLayout->addWidget(new QLabel("Name:", this), 4, 0);
    m_nameLineEdit = new QLineEdit(this);
    appearanceLayout->addWidget(m_nameLineEdit, 4, 1);
    
    mainLayout->addWidget(m_appearanceGroup);
    
    // Attributes Group (matches C# optionsGroupBox)
    m_attributesGroup = new QGroupBox("Attributes", this);
    auto* attributesLayout = new QGridLayout(m_attributesGroup);
    
    // Flag checkboxes - Left column (matches C# layout)
    int row = 0;
    m_unpassableCheck = new QCheckBox("Unpassable", this);
    attributesLayout->addWidget(m_unpassableCheck, row++, 0);
    
    m_movableCheck = new QCheckBox("Movable", this);
    attributesLayout->addWidget(m_movableCheck, row++, 0);
    
    m_blockMissilesCheck = new QCheckBox("Block Missiles", this);
    attributesLayout->addWidget(m_blockMissilesCheck, row++, 0);
    
    m_blockPathfinderCheck = new QCheckBox("Block Pathfinder", this);
    attributesLayout->addWidget(m_blockPathfinderCheck, row++, 0);
    
    m_pickupableCheck = new QCheckBox("Pickupable", this);
    attributesLayout->addWidget(m_pickupableCheck, row++, 0);
    
    m_stackableCheck = new QCheckBox("Stackable", this);
    attributesLayout->addWidget(m_stackableCheck, row++, 0);
    
    m_multiUseCheck = new QCheckBox("Multi Use", this);
    attributesLayout->addWidget(m_multiUseCheck, row++, 0);
    
    m_rotatableCheck = new QCheckBox("Rotatable", this);
    attributesLayout->addWidget(m_rotatableCheck, row++, 0);
    
    m_hangableCheck = new QCheckBox("Hangable", this);
    attributesLayout->addWidget(m_hangableCheck, row++, 0);
    
    m_hookSouthCheck = new QCheckBox("Hook South", this);
    attributesLayout->addWidget(m_hookSouthCheck, row++, 0);
    
    m_hookEastCheck = new QCheckBox("Hook East", this);
    attributesLayout->addWidget(m_hookEastCheck, row++, 0);
    
    // Flag checkboxes - Right column
    row = 0;
    m_hasElevationCheck = new QCheckBox("Has Elevation", this);
    attributesLayout->addWidget(m_hasElevationCheck, row++, 1);
    
    m_ignoreLookCheck = new QCheckBox("Ignore Look", this);
    attributesLayout->addWidget(m_ignoreLookCheck, row++, 1);
    
    m_readableCheck = new QCheckBox("Readable", this);
    attributesLayout->addWidget(m_readableCheck, row++, 1);
    
    m_fullGroundCheck = new QCheckBox("Full Ground", this);
    attributesLayout->addWidget(m_fullGroundCheck, row++, 1);
    
    m_forceUseCheck = new QCheckBox("Force Use", this);
    attributesLayout->addWidget(m_forceUseCheck, row++, 1);
    
    // Numeric attributes - Right side (matches C# layout)
    row = 0;
    m_groundSpeedLabel = new QLabel("Ground Speed:", this);
    attributesLayout->addWidget(m_groundSpeedLabel, row, 2);
    m_groundSpeedEdit = new QLineEdit(this);
    m_groundSpeedEdit->setMaximumWidth(80);
    attributesLayout->addWidget(m_groundSpeedEdit, row++, 3);
    
    m_minimapColorLabel = new QLabel("Minimap Color:", this);
    attributesLayout->addWidget(m_minimapColorLabel, row, 2);
    m_minimapColorEdit = new QLineEdit(this);
    m_minimapColorEdit->setMaximumWidth(80);
    attributesLayout->addWidget(m_minimapColorEdit, row++, 3);
    
    m_lightLevelLabel = new QLabel("Light Level:", this);
    attributesLayout->addWidget(m_lightLevelLabel, row, 2);
    m_lightLevelEdit = new QLineEdit(this);
    m_lightLevelEdit->setMaximumWidth(80);
    attributesLayout->addWidget(m_lightLevelEdit, row++, 3);
    
    m_lightColorLabel = new QLabel("Light Color:", this);
    attributesLayout->addWidget(m_lightColorLabel, row, 2);
    m_lightColorEdit = new QLineEdit(this);
    m_lightColorEdit->setMaximumWidth(80);
    attributesLayout->addWidget(m_lightColorEdit, row++, 3);
    
    m_tradeAsLabel = new QLabel("Trade As:", this);
    attributesLayout->addWidget(m_tradeAsLabel, row, 2);
    m_tradeAsEdit = new QLineEdit(this);
    m_tradeAsEdit->setMaximumWidth(80);
    attributesLayout->addWidget(m_tradeAsEdit, row++, 3);
    
    m_maxReadCharsLabel = new QLabel("Read Length:", this);
    attributesLayout->addWidget(m_maxReadCharsLabel, row, 2);
    m_maxReadCharsEdit = new QLineEdit(this);
    m_maxReadCharsEdit->setMaximumWidth(80);
    attributesLayout->addWidget(m_maxReadCharsEdit, row++, 3);
    
    m_maxReadWriteCharsLabel = new QLabel("Read/Write Length:", this);
    attributesLayout->addWidget(m_maxReadWriteCharsLabel, row, 2);
    m_maxReadWriteCharsEdit = new QLineEdit(this);
    m_maxReadWriteCharsEdit->setMaximumWidth(80);
    attributesLayout->addWidget(m_maxReadWriteCharsEdit, row++, 3);
    
    mainLayout->addWidget(m_attributesGroup);
    
    // Initially disabled
    setEnabled(false);
}

void ItemPropertyEditor::setupConnections()
{
    // Connect all controls to their respective slots
    connect(m_clientIdSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ItemPropertyEditor::onClientIdChanged);
    connect(m_nameLineEdit, &QLineEdit::textChanged,
            this, &ItemPropertyEditor::onNameChanged);
    connect(m_typeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ItemPropertyEditor::onTypeChanged);
    connect(m_stackOrderComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ItemPropertyEditor::onStackOrderChanged);
    
    // Connect all checkboxes
    connect(m_unpassableCheck, &QCheckBox::toggled, this, &ItemPropertyEditor::onFlagChanged);
    connect(m_movableCheck, &QCheckBox::toggled, this, &ItemPropertyEditor::onFlagChanged);
    connect(m_blockMissilesCheck, &QCheckBox::toggled, this, &ItemPropertyEditor::onFlagChanged);
    connect(m_blockPathfinderCheck, &QCheckBox::toggled, this, &ItemPropertyEditor::onFlagChanged);
    connect(m_hasElevationCheck, &QCheckBox::toggled, this, &ItemPropertyEditor::onFlagChanged);
    connect(m_forceUseCheck, &QCheckBox::toggled, this, &ItemPropertyEditor::onFlagChanged);
    connect(m_multiUseCheck, &QCheckBox::toggled, this, &ItemPropertyEditor::onFlagChanged);
    connect(m_pickupableCheck, &QCheckBox::toggled, this, &ItemPropertyEditor::onFlagChanged);
    connect(m_stackableCheck, &QCheckBox::toggled, this, &ItemPropertyEditor::onFlagChanged);
    connect(m_readableCheck, &QCheckBox::toggled, this, &ItemPropertyEditor::onFlagChanged);
    connect(m_rotatableCheck, &QCheckBox::toggled, this, &ItemPropertyEditor::onFlagChanged);
    connect(m_hangableCheck, &QCheckBox::toggled, this, &ItemPropertyEditor::onFlagChanged);
    connect(m_hookSouthCheck, &QCheckBox::toggled, this, &ItemPropertyEditor::onFlagChanged);
    connect(m_hookEastCheck, &QCheckBox::toggled, this, &ItemPropertyEditor::onFlagChanged);
    connect(m_ignoreLookCheck, &QCheckBox::toggled, this, &ItemPropertyEditor::onFlagChanged);
    connect(m_fullGroundCheck, &QCheckBox::toggled, this, &ItemPropertyEditor::onFlagChanged);
    
    // Connect attribute line edits
    connect(m_groundSpeedEdit, &QLineEdit::textChanged, this, &ItemPropertyEditor::onAttributeChanged);
    connect(m_lightLevelEdit, &QLineEdit::textChanged, this, &ItemPropertyEditor::onAttributeChanged);
    connect(m_lightColorEdit, &QLineEdit::textChanged, this, &ItemPropertyEditor::onAttributeChanged);
    connect(m_minimapColorEdit, &QLineEdit::textChanged, this, &ItemPropertyEditor::onAttributeChanged);
    connect(m_maxReadCharsEdit, &QLineEdit::textChanged, this, &ItemPropertyEditor::onAttributeChanged);
    connect(m_maxReadWriteCharsEdit, &QLineEdit::textChanged, this, &ItemPropertyEditor::onAttributeChanged);
    connect(m_tradeAsEdit, &QLineEdit::textChanged, this, &ItemPropertyEditor::onAttributeChanged);
}

void ItemPropertyEditor::setServerItem(OTB::ServerItem* item)
{
    m_currentServerItem = item;
    updateControls();
}

void ItemPropertyEditor::setClientItem(const ItemEditor::ClientItem* clientItem)
{
    m_currentClientItem = clientItem;
    updateControls();
}

void ItemPropertyEditor::updateControls()
{
    if (!m_currentServerItem) {
        clear();
        return;
    }
    
    m_updatingControls = true;
    
    // Update appearance controls
    m_serverIdLabel->setText(QString::number(m_currentServerItem->id));
    m_clientIdSpinBox->setValue(m_currentServerItem->clientId);
    m_typeComboBox->setCurrentIndex(static_cast<int>(m_currentServerItem->type));
    m_stackOrderComboBox->setCurrentIndex(static_cast<int>(m_currentServerItem->stackOrder));
    m_nameLineEdit->setText(m_currentServerItem->name);
    
    // Update flag checkboxes
    m_unpassableCheck->setChecked(m_currentServerItem->unpassable);
    m_movableCheck->setChecked(m_currentServerItem->movable);
    m_blockMissilesCheck->setChecked(m_currentServerItem->blockMissiles);
    m_blockPathfinderCheck->setChecked(m_currentServerItem->blockPathfinder);
    m_hasElevationCheck->setChecked(m_currentServerItem->hasElevation);
    m_forceUseCheck->setChecked(m_currentServerItem->forceUse);
    m_multiUseCheck->setChecked(m_currentServerItem->multiUse);
    m_pickupableCheck->setChecked(m_currentServerItem->pickupable);
    m_stackableCheck->setChecked(m_currentServerItem->stackable);
    m_readableCheck->setChecked(m_currentServerItem->readable);
    m_rotatableCheck->setChecked(m_currentServerItem->rotatable);
    m_hangableCheck->setChecked(m_currentServerItem->hangable);
    m_hookSouthCheck->setChecked(m_currentServerItem->hookSouth);
    m_hookEastCheck->setChecked(m_currentServerItem->hookEast);
    m_ignoreLookCheck->setChecked(m_currentServerItem->ignoreLook);
    m_fullGroundCheck->setChecked(m_currentServerItem->fullGround);
    
    // Update attribute line edits
    m_groundSpeedEdit->setText(QString::number(m_currentServerItem->groundSpeed));
    m_lightLevelEdit->setText(QString::number(m_currentServerItem->lightLevel));
    m_lightColorEdit->setText(QString::number(m_currentServerItem->lightColor));
    m_minimapColorEdit->setText(QString::number(m_currentServerItem->minimapColor));
    m_maxReadCharsEdit->setText(QString::number(m_currentServerItem->maxReadChars));
    m_maxReadWriteCharsEdit->setText(QString::number(m_currentServerItem->maxReadWriteChars));
    m_tradeAsEdit->setText(QString::number(m_currentServerItem->tradeAs));
    
    // Apply validation styling if client item is available
    if (m_currentClientItem) {
        updateValidationStyling();
    }
    
    m_updatingControls = false;
}

void ItemPropertyEditor::updateValidationStyling()
{
    if (!m_currentServerItem || !m_currentClientItem) {
        return;
    }
    
    // Update styling based on comparison with client item
    updatePropertyStyle(m_typeComboBox, m_currentServerItem->type == m_currentClientItem->Type);
    updatePropertyStyle(m_stackOrderComboBox, m_currentServerItem->stackOrder == m_currentClientItem->StackOrder);
    updatePropertyStyle(m_nameLineEdit, m_currentServerItem->name == m_currentClientItem->Name);
    
    // Update flag styling
    updatePropertyStyle(m_unpassableCheck, m_currentServerItem->unpassable == m_currentClientItem->Unpassable);
    updatePropertyStyle(m_movableCheck, m_currentServerItem->movable == m_currentClientItem->Movable);
    updatePropertyStyle(m_blockMissilesCheck, m_currentServerItem->blockMissiles == m_currentClientItem->BlockMissiles);
    updatePropertyStyle(m_blockPathfinderCheck, m_currentServerItem->blockPathfinder == m_currentClientItem->BlockPathfinder);
    updatePropertyStyle(m_hasElevationCheck, m_currentServerItem->hasElevation == m_currentClientItem->HasElevation);
    updatePropertyStyle(m_forceUseCheck, m_currentServerItem->forceUse == m_currentClientItem->ForceUse);
    updatePropertyStyle(m_multiUseCheck, m_currentServerItem->multiUse == m_currentClientItem->MultiUse);
    updatePropertyStyle(m_pickupableCheck, m_currentServerItem->pickupable == m_currentClientItem->Pickupable);
    updatePropertyStyle(m_stackableCheck, m_currentServerItem->stackable == m_currentClientItem->Stackable);
    updatePropertyStyle(m_readableCheck, m_currentServerItem->readable == m_currentClientItem->Readable);
    updatePropertyStyle(m_rotatableCheck, m_currentServerItem->rotatable == m_currentClientItem->Rotatable);
    updatePropertyStyle(m_hangableCheck, m_currentServerItem->hangable == m_currentClientItem->Hangable);
    updatePropertyStyle(m_hookSouthCheck, m_currentServerItem->hookSouth == m_currentClientItem->HookSouth);
    updatePropertyStyle(m_hookEastCheck, m_currentServerItem->hookEast == m_currentClientItem->HookEast);
    updatePropertyStyle(m_ignoreLookCheck, m_currentServerItem->ignoreLook == m_currentClientItem->IgnoreLook);
    updatePropertyStyle(m_fullGroundCheck, m_currentServerItem->fullGround == m_currentClientItem->FullGround);
}

void ItemPropertyEditor::updatePropertyStyle(QWidget* control, bool matches)
{
    if (!control) return;
    
    QPalette palette = control->palette();
    if (matches) {
        palette.setColor(QPalette::Text, m_normalTextColor);
    } else {
        palette.setColor(QPalette::Text, m_mismatchTextColor);
    }
    control->setPalette(palette);
}

void ItemPropertyEditor::clear()
{
    m_updatingControls = true;
    
    m_currentServerItem = nullptr;
    m_currentClientItem = nullptr;
    
    // Clear all controls
    m_serverIdLabel->setText("0");
    m_clientIdSpinBox->setValue(100);
    m_typeComboBox->setCurrentIndex(0);
    m_stackOrderComboBox->setCurrentIndex(0);
    m_nameLineEdit->clear();
    
    // Clear checkboxes
    m_unpassableCheck->setChecked(false);
    m_movableCheck->setChecked(true); // Default true like C#
    m_blockMissilesCheck->setChecked(false);
    m_blockPathfinderCheck->setChecked(false);
    m_hasElevationCheck->setChecked(false);
    m_forceUseCheck->setChecked(false);
    m_multiUseCheck->setChecked(false);
    m_pickupableCheck->setChecked(false);
    m_stackableCheck->setChecked(false);
    m_readableCheck->setChecked(false);
    m_rotatableCheck->setChecked(false);
    m_hangableCheck->setChecked(false);
    m_hookSouthCheck->setChecked(false);
    m_hookEastCheck->setChecked(false);
    m_ignoreLookCheck->setChecked(false);
    m_fullGroundCheck->setChecked(false);
    
    // Clear line edits
    m_groundSpeedEdit->clear();
    m_lightLevelEdit->clear();
    m_lightColorEdit->clear();
    m_minimapColorEdit->clear();
    m_maxReadCharsEdit->clear();
    m_maxReadWriteCharsEdit->clear();
    m_tradeAsEdit->clear();
    
    m_updatingControls = false;
}

// Slot implementations
void ItemPropertyEditor::onClientIdChanged(int value)
{
    if (m_updatingControls || !m_currentServerItem) return;
    
    m_currentServerItem->clientId = static_cast<quint16>(value);
    emit clientIdChanged(static_cast<quint16>(value));
    emit itemPropertyChanged();
}

void ItemPropertyEditor::onNameChanged(const QString& text)
{
    if (m_updatingControls || !m_currentServerItem) return;
    
    m_currentServerItem->name = text;
    emit itemNameChanged(text);
    emit itemPropertyChanged();
}

void ItemPropertyEditor::onTypeChanged(int index)
{
    if (m_updatingControls || !m_currentServerItem) return;
    
    auto newType = static_cast<OTB::ServerItemType>(index);
    m_currentServerItem->type = newType;
    emit itemTypeChanged(newType);
    emit itemPropertyChanged();
}

void ItemPropertyEditor::onStackOrderChanged(int index)
{
    if (m_updatingControls || !m_currentServerItem) return;
    
    auto newOrder = static_cast<OTB::TileStackOrder>(index);
    m_currentServerItem->stackOrder = newOrder;
    emit stackOrderChanged(newOrder);
    emit itemPropertyChanged();
}

void ItemPropertyEditor::onFlagChanged()
{
    if (m_updatingControls || !m_currentServerItem) return;
    
    // Update all flags from checkboxes
    m_currentServerItem->unpassable = m_unpassableCheck->isChecked();
    m_currentServerItem->movable = m_movableCheck->isChecked();
    m_currentServerItem->blockMissiles = m_blockMissilesCheck->isChecked();
    m_currentServerItem->blockPathfinder = m_blockPathfinderCheck->isChecked();
    m_currentServerItem->hasElevation = m_hasElevationCheck->isChecked();
    m_currentServerItem->forceUse = m_forceUseCheck->isChecked();
    m_currentServerItem->multiUse = m_multiUseCheck->isChecked();
    m_currentServerItem->pickupable = m_pickupableCheck->isChecked();
    m_currentServerItem->stackable = m_stackableCheck->isChecked();
    m_currentServerItem->readable = m_readableCheck->isChecked();
    m_currentServerItem->rotatable = m_rotatableCheck->isChecked();
    m_currentServerItem->hangable = m_hangableCheck->isChecked();
    m_currentServerItem->hookSouth = m_hookSouthCheck->isChecked();
    m_currentServerItem->hookEast = m_hookEastCheck->isChecked();
    m_currentServerItem->ignoreLook = m_ignoreLookCheck->isChecked();
    m_currentServerItem->fullGround = m_fullGroundCheck->isChecked();
    
    emit itemPropertyChanged();
}

void ItemPropertyEditor::onAttributeChanged()
{
    if (m_updatingControls || !m_currentServerItem) return;
    
    // Update all attributes from line edits
    m_currentServerItem->groundSpeed = m_groundSpeedEdit->text().toUShort();
    m_currentServerItem->lightLevel = m_lightLevelEdit->text().toUShort();
    m_currentServerItem->lightColor = m_lightColorEdit->text().toUShort();
    m_currentServerItem->minimapColor = m_minimapColorEdit->text().toUShort();
    m_currentServerItem->maxReadChars = m_maxReadCharsEdit->text().toUShort();
    m_currentServerItem->maxReadWriteChars = m_maxReadWriteCharsEdit->text().toUShort();
    m_currentServerItem->tradeAs = m_tradeAsEdit->text().toUShort();
    
    emit itemPropertyChanged();
}

} // namespace Widgets
} // namespace UI