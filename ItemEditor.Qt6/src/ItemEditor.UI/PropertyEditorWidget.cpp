#include "PropertyEditorWidget.h"
#include <QApplication>
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QPaintEvent>
#include <QToolTip>
#include <QDebug>
#include <QMessageBox>
#include <QColorDialog>
#include <QFontMetrics>
#include <QSplitter>

// PropertyEditorWidget Implementation
PropertyEditorWidget::PropertyEditorWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_contentLayout(nullptr)
    , m_basicGroup(nullptr)
    , m_appearanceGroup(nullptr)
    , m_behaviorGroup(nullptr)
    , m_combatGroup(nullptr)
    , m_tradeGroup(nullptr)
    , m_containerGroup(nullptr)
    , m_readWriteGroup(nullptr)
    , m_customGroup(nullptr)
    , m_buttonLayout(nullptr)
    , m_applyButton(nullptr)
    , m_revertButton(nullptr)
    , m_resetButton(nullptr)
    , m_copyFromClientButton(nullptr)
    , m_showMismatchesCheck(nullptr)
    , m_serverItem(nullptr)
    , m_clientItem(nullptr)
    , m_showMismatchesOnly(false)
    , m_showTooltips(true)
    , m_readOnly(false)
    , m_batchEditing(false)
    , m_validationTimer(nullptr)
    , m_validator(nullptr)
    , m_itemEditingManager(nullptr)
{
    setupUI();
    setupPropertyGroups();
    setupControls();
    connectSignals();
    applyDarkTheme();
    
    // Initialize validation timer
    m_validationTimer = new QTimer(this);
    m_validationTimer->setSingleShot(true);
    m_validationTimer->setInterval(500); // 500ms delay for validation
    connect(m_validationTimer, &QTimer::timeout, this, &PropertyEditorWidget::onValidationTimer);
    
    // Initialize validator (ItemValidator is static class, no need to instantiate)
    m_validator = nullptr;
    
    setMinimumWidth(300);
}

PropertyEditorWidget::~PropertyEditorWidget()
{
    // Qt's parent-child system will handle cleanup
}

// Integration with ItemEditingManager
void PropertyEditorWidget::setItemEditingManager(ItemEditingManager* manager)
{
    if (m_itemEditingManager == manager) {
        return;
    }
    
    // Disconnect from previous manager
    if (m_itemEditingManager) {
        disconnect(m_itemEditingManager, nullptr, this, nullptr);
    }
    
    m_itemEditingManager = manager;
    
    // Connect to new manager
    if (m_itemEditingManager) {
        connect(m_itemEditingManager, &ItemEditingManager::propertyChanged,
                this, &PropertyEditorWidget::onItemEditingManagerPropertyChanged);
        connect(m_itemEditingManager, &ItemEditingManager::itemSelected,
                this, &PropertyEditorWidget::onItemEditingManagerItemSelected);
        connect(m_itemEditingManager, &ItemEditingManager::itemDeselected,
                this, &PropertyEditorWidget::onItemEditingManagerItemDeselected);
        connect(m_itemEditingManager, &ItemEditingManager::validationStateChanged,
                this, &PropertyEditorWidget::onItemEditingManagerValidationStateChanged);
        
        // Update current state
        if (m_itemEditingManager->hasSelection()) {
            onItemEditingManagerItemSelected(m_itemEditingManager->getCurrentItemId());
        } else {
            onItemEditingManagerItemDeselected();
        }
    }
}

ItemEditingManager* PropertyEditorWidget::getItemEditingManager() const
{
    return m_itemEditingManager;
}

void PropertyEditorWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(4, 4, 4, 4);
    m_mainLayout->setSpacing(4);
    
    // Create scroll area
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Create content widget
    m_contentWidget = new QWidget();
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(4, 4, 4, 4);
    m_contentLayout->setSpacing(8);
    
    m_scrollArea->setWidget(m_contentWidget);
    m_mainLayout->addWidget(m_scrollArea, 1);
}

void PropertyEditorWidget::setupPropertyGroups()
{
    // Basic properties group
    m_basicGroup = new QGroupBox("Basic Properties", m_contentWidget);
    m_basicGroup->setLayout(new QFormLayout());
    
    // Appearance properties group
    m_appearanceGroup = new QGroupBox("Appearance", m_contentWidget);
    m_appearanceGroup->setLayout(new QFormLayout());
    
    // Behavior properties group
    m_behaviorGroup = new QGroupBox("Behavior", m_contentWidget);
    m_behaviorGroup->setLayout(new QFormLayout());
    
    // Combat properties group
    m_combatGroup = new QGroupBox("Combat", m_contentWidget);
    m_combatGroup->setLayout(new QFormLayout());
    
    // Trade properties group
    m_tradeGroup = new QGroupBox("Trade & Market", m_contentWidget);
    m_tradeGroup->setLayout(new QFormLayout());
    
    // Container properties group
    m_containerGroup = new QGroupBox("Container", m_contentWidget);
    m_containerGroup->setLayout(new QFormLayout());
    
    // Read/Write properties group
    m_readWriteGroup = new QGroupBox("Text & Reading", m_contentWidget);
    m_readWriteGroup->setLayout(new QFormLayout());
    
    // Custom properties group
    m_customGroup = new QGroupBox("Custom Properties", m_contentWidget);
    m_customGroup->setLayout(new QFormLayout());
    
    // Add groups to content layout
    m_contentLayout->addWidget(m_basicGroup);
    m_contentLayout->addWidget(m_appearanceGroup);
    m_contentLayout->addWidget(m_behaviorGroup);
    m_contentLayout->addWidget(m_combatGroup);
    m_contentLayout->addWidget(m_tradeGroup);
    m_contentLayout->addWidget(m_containerGroup);
    m_contentLayout->addWidget(m_readWriteGroup);
    m_contentLayout->addWidget(m_customGroup);
    m_contentLayout->addStretch();
}

void PropertyEditorWidget::setupControls()
{
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->setSpacing(4);
    
    // Control buttons
    m_applyButton = new QPushButton("Apply", this);
    m_applyButton->setEnabled(false);
    
    m_revertButton = new QPushButton("Revert", this);
    m_revertButton->setEnabled(false);
    
    m_resetButton = new QPushButton("Reset", this);
    m_resetButton->setToolTip("Reset all properties to default values");
    
    m_copyFromClientButton = new QPushButton("Copy from Client", this);
    m_copyFromClientButton->setToolTip("Copy matching properties from client item");
    m_copyFromClientButton->setEnabled(false);
    
    m_showMismatchesCheck = new QCheckBox("Show Mismatches Only", this);
    m_showMismatchesCheck->setChecked(m_showMismatchesOnly);
    
    // Add buttons to layout
    m_buttonLayout->addWidget(m_applyButton);
    m_buttonLayout->addWidget(m_revertButton);
    m_buttonLayout->addWidget(m_resetButton);
    m_buttonLayout->addWidget(m_copyFromClientButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_showMismatchesCheck);
    
    m_mainLayout->addLayout(m_buttonLayout);
}

void PropertyEditorWidget::connectSignals()
{
    // Button connections
    connect(m_applyButton, &QPushButton::clicked, this, &PropertyEditorWidget::applyChanges);
    connect(m_revertButton, &QPushButton::clicked, this, &PropertyEditorWidget::revertChanges);
    connect(m_resetButton, &QPushButton::clicked, this, &PropertyEditorWidget::resetToDefaults);
    connect(m_copyFromClientButton, &QPushButton::clicked, this, &PropertyEditorWidget::copyFromClient);
    connect(m_showMismatchesCheck, &QCheckBox::toggled, this, &PropertyEditorWidget::showMismatchesOnly);
}

void PropertyEditorWidget::applyDarkTheme()
{
    setStyleSheet(R"(
        QWidget {
            background-color: #3C3F41;
            color: #DCDCDC;
        }
        QGroupBox {
            font-weight: bold;
            border: 1px solid #555555;
            border-radius: 4px;
            margin-top: 8px;
            padding-top: 4px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 8px;
            padding: 0 4px 0 4px;
        }
        QPushButton {
            background-color: #45494A;
            border: 1px solid #555555;
            padding: 6px 12px;
            border-radius: 2px;
        }
        QPushButton:hover {
            background-color: #4C5052;
        }
        QPushButton:pressed {
            background-color: #3A3D3F;
        }
        QPushButton:disabled {
            background-color: #3A3A3A;
            color: #808080;
        }
        QCheckBox {
            spacing: 4px;
        }
        QCheckBox::indicator {
            width: 14px;
            height: 14px;
        }
        QCheckBox::indicator:unchecked {
            border: 1px solid #555555;
            background-color: #45494A;
        }
        QCheckBox::indicator:checked {
            border: 1px solid #6897BB;
            background-color: #6897BB;
        }
        QScrollArea {
            border: 1px solid #555555;
            background-color: #2B2B2B;
        }
        QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QTextEdit {
            background-color: #45494A;
            border: 1px solid #555555;
            padding: 4px;
            border-radius: 2px;
        }
        QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus, QTextEdit:focus {
            border-color: #6897BB;
        }
        QSlider::groove:horizontal {
            border: 1px solid #555555;
            height: 6px;
            background: #45494A;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            background: #6897BB;
            border: 1px solid #555555;
            width: 14px;
            margin: -4px 0;
            border-radius: 7px;
        }
    )");
}

// Data management methods
void PropertyEditorWidget::setServerItem(ServerItem* item)
{
    if (m_serverItem == item) {
        return;
    }
    
    m_serverItem = item;
    storeOriginalValues();
    populateProperties();
    updateControlStates();
}

void PropertyEditorWidget::setClientItem(const ClientItem* item)
{
    if (m_clientItem == item) {
        return;
    }
    
    m_clientItem = item;
    m_copyFromClientButton->setEnabled(item != nullptr && m_serverItem != nullptr);
    detectMismatches();
    updateMismatchIndicators();
}

void PropertyEditorWidget::setItems(ServerItem* serverItem, const ClientItem* clientItem)
{
    m_serverItem = serverItem;
    m_clientItem = clientItem;
    
    storeOriginalValues();
    populateProperties();
    detectMismatches();
    updateMismatchIndicators();
    updateControlStates();
}

void PropertyEditorWidget::clearItems()
{
    m_serverItem = nullptr;
    m_clientItem = nullptr;
    m_propertyFields.clear();
    m_originalValues.clear();
    m_currentValues.clear();
    
    // Clear all property groups
    QFormLayout* layouts[] = {
        qobject_cast<QFormLayout*>(m_basicGroup->layout()),
        qobject_cast<QFormLayout*>(m_appearanceGroup->layout()),
        qobject_cast<QFormLayout*>(m_behaviorGroup->layout()),
        qobject_cast<QFormLayout*>(m_combatGroup->layout()),
        qobject_cast<QFormLayout*>(m_tradeGroup->layout()),
        qobject_cast<QFormLayout*>(m_containerGroup->layout()),
        qobject_cast<QFormLayout*>(m_readWriteGroup->layout()),
        qobject_cast<QFormLayout*>(m_customGroup->layout())
    };
    
    for (QFormLayout* layout : layouts) {
        if (layout) {
            QLayoutItem* item;
            while ((item = layout->takeAt(0)) != nullptr) {
                delete item->widget();
                delete item;
            }
        }
    }
    
    updateControlStates();
}

ServerItem* PropertyEditorWidget::getServerItem() const
{
    return m_serverItem;
}

const ClientItem* PropertyEditorWidget::getClientItem() const
{
    return m_clientItem;
}

// Property editing methods
void PropertyEditorWidget::refreshProperties()
{
    if (!m_serverItem) {
        return;
    }
    
    populateProperties();
    detectMismatches();
    updateMismatchIndicators();
    updateControlStates();
}

void PropertyEditorWidget::applyChanges()
{
    if (m_itemEditingManager) {
        // ItemEditingManager handles changes automatically
        // This method is mainly for UI state updates
        updateControlStates();
        return;
    }
    
    // Fallback for direct mode
    if (!m_serverItem || !hasChanges()) {
        return;
    }
    
    beginBatchEdit();
    
    // Apply all current values to server item
    for (auto it = m_currentValues.begin(); it != m_currentValues.end(); ++it) {
        const QString& propertyName = it.key();
        const QVariant& value = it.value();
        
        if (m_originalValues.value(propertyName) != value) {
            setPropertyValue(propertyName, value);
        }
    }
    
    // Update original values to current values
    m_originalValues = m_currentValues;
    
    // Mark server item as modified
    if (m_serverItem) {
        m_serverItem->markAsModified();
    }
    
    endBatchEdit();
    
    updateControlStates();
    emit itemModified();
}

void PropertyEditorWidget::revertChanges()
{
    if (m_itemEditingManager) {
        // Use undo functionality from ItemEditingManager
        m_itemEditingManager->undo();
        return;
    }
    
    // Fallback for direct mode
    if (!hasChanges()) {
        return;
    }
    
    beginBatchEdit();
    
    // Revert all values to original
    m_currentValues = m_originalValues;
    
    // Update all property fields
    for (auto it = m_propertyFields.begin(); it != m_propertyFields.end(); ++it) {
        const QString& propertyName = it.key();
        PropertyEditorField* field = it.value();
        
        if (m_originalValues.contains(propertyName)) {
            field->setValue(m_originalValues[propertyName]);
        }
    }
    
    endBatchEdit();
    
    updateControlStates();
}

bool PropertyEditorWidget::hasChanges() const
{
    for (auto it = m_currentValues.begin(); it != m_currentValues.end(); ++it) {
        const QString& propertyName = it.key();
        const QVariant& currentValue = it.value();
        const QVariant& originalValue = m_originalValues.value(propertyName);
        
        if (currentValue != originalValue) {
            return true;
        }
    }
    
    return false;
}

bool PropertyEditorWidget::hasValidationErrors() const
{
    return !m_validationErrors.isEmpty();
}

// Validation methods
void PropertyEditorWidget::validateAllProperties()
{
    if (!m_serverItem) {
        return;
    }
    
    m_validationErrors.clear();
    
    for (auto it = m_propertyFields.begin(); it != m_propertyFields.end(); ++it) {
        const QString& propertyName = it.key();
        validateProperty(propertyName);
    }
    
    updateValidationStates();
    emit validationStateChanged(hasValidationErrors());
}

void PropertyEditorWidget::validateProperty(const QString& propertyName)
{
    if (!m_serverItem || !m_propertyFields.contains(propertyName)) {
        return;
    }
    
    PropertyEditorField* field = m_propertyFields[propertyName];
    QVariant value = field->getValue();
    
    // Create a temporary server item with the new value for validation
    ServerItem tempItem = *m_serverItem;
    tempItem.setProperty(propertyName, value);
    
    // Validate the property based on its type
    bool isValid = true;
    QString errorMessage;
    
    if (propertyName == "id") {
        isValid = ItemValidator::validateItemId(value.toUInt());
        if (!isValid) errorMessage = "Invalid item ID";
    } else if (propertyName == "name") {
        isValid = ItemValidator::validateItemName(value.toString());
        if (!isValid) errorMessage = "Invalid item name";
    } else if (propertyName == "width" || propertyName == "height") {
        quint8 width = (propertyName == "width") ? value.toUInt() : tempItem.width;
        quint8 height = (propertyName == "height") ? value.toUInt() : tempItem.height;
        isValid = ItemValidator::validateDimensions(width, height);
        if (!isValid) errorMessage = "Invalid dimensions";
    } else if (propertyName == "speed") {
        isValid = ItemValidator::validateSpeed(value.toUInt());
        if (!isValid) errorMessage = "Invalid speed value";
    } else if (propertyName == "lightLevel") {
        isValid = ItemValidator::validateLight(value.toUInt(), tempItem.lightColor);
        if (!isValid) errorMessage = "Invalid light level";
    } else if (propertyName == "flags") {
        isValid = ItemValidator::validateFlags(value.toUInt());
        if (!isValid) errorMessage = "Invalid flag combination";
    } else {
        // For other properties, do basic validation
        isValid = !value.toString().isEmpty() || value.type() != QVariant::String;
    }
    
    if (isValid) {
        field->setValidationState(PropertyEditorField::Valid);
        field->setValidationMessage("");
        m_validationErrors.removeAll(propertyName);
    } else {
        field->setValidationState(PropertyEditorField::Error);
        field->setValidationMessage(errorMessage);
        
        if (!m_validationErrors.contains(propertyName)) {
            m_validationErrors.append(propertyName);
        }
    }
}

QStringList PropertyEditorWidget::getValidationErrors() const
{
    return m_validationErrors;
}

QStringList PropertyEditorWidget::getModifiedProperties() const
{
    QStringList modified;
    
    for (auto it = m_currentValues.begin(); it != m_currentValues.end(); ++it) {
        const QString& propertyName = it.key();
        const QVariant& currentValue = it.value();
        const QVariant& originalValue = m_originalValues.value(propertyName);
        
        if (currentValue != originalValue) {
            modified.append(propertyName);
        }
    }
    
    return modified;
}

// Display options
void PropertyEditorWidget::setShowMismatchesOnly(bool show)
{
    if (m_showMismatchesOnly != show) {
        m_showMismatchesOnly = show;
        m_showMismatchesCheck->setChecked(show);
        
        // Update visibility of property fields
        for (auto it = m_propertyFields.begin(); it != m_propertyFields.end(); ++it) {
            const QString& propertyName = it.key();
            PropertyEditorField* field = it.value();
            field->setVisible(isPropertyVisible(propertyName));
        }
    }
}

bool PropertyEditorWidget::getShowMismatchesOnly() const
{
    return m_showMismatchesOnly;
}

void PropertyEditorWidget::setShowTooltips(bool show)
{
    m_showTooltips = show;
}

bool PropertyEditorWidget::getShowTooltips() const
{
    return m_showTooltips;
}

void PropertyEditorWidget::setReadOnly(bool readOnly)
{
    if (m_readOnly != readOnly) {
        m_readOnly = readOnly;
        
        // Update all property fields
        for (auto it = m_propertyFields.begin(); it != m_propertyFields.end(); ++it) {
            it.value()->setReadOnly(readOnly);
        }
        
        // Update control buttons
        m_applyButton->setEnabled(!readOnly && hasChanges());
        m_revertButton->setEnabled(!readOnly && hasChanges());
        m_resetButton->setEnabled(!readOnly);
        m_copyFromClientButton->setEnabled(!readOnly && m_clientItem != nullptr);
    }
}

bool PropertyEditorWidget::isReadOnly() const
{
    return m_readOnly;
}

// Property filtering
void PropertyEditorWidget::setPropertyFilter(const QString& filter)
{
    if (m_propertyFilter != filter) {
        m_propertyFilter = filter;
        
        // Update visibility of property fields
        for (auto it = m_propertyFields.begin(); it != m_propertyFields.end(); ++it) {
            const QString& propertyName = it.key();
            PropertyEditorField* field = it.value();
            field->setVisible(isPropertyVisible(propertyName));
        }
    }
}

void PropertyEditorWidget::setPropertyCategory(const QString& category)
{
    if (m_propertyCategory != category) {
        m_propertyCategory = category;
        
        // Update visibility of property fields
        for (auto it = m_propertyFields.begin(); it != m_propertyFields.end(); ++it) {
            const QString& propertyName = it.key();
            PropertyEditorField* field = it.value();
            field->setVisible(isPropertyVisible(propertyName));
        }
    }
}

void PropertyEditorWidget::clearFilters()
{
    m_propertyFilter.clear();
    m_propertyCategory.clear();
    
    // Show all property fields
    for (auto it = m_propertyFields.begin(); it != m_propertyFields.end(); ++it) {
        PropertyEditorField* field = it.value();
        field->setVisible(isPropertyVisible(it.key()));
    }
}

// Batch operations
void PropertyEditorWidget::beginBatchEdit()
{
    m_batchEditing = true;
}

void PropertyEditorWidget::endBatchEdit()
{
    m_batchEditing = false;
    updateControlStates();
}

bool PropertyEditorWidget::isBatchEditing() const
{
    return m_batchEditing;
}

// Public slots
void PropertyEditorWidget::onServerItemChanged()
{
    refreshProperties();
}

void PropertyEditorWidget::onClientItemChanged()
{
    detectMismatches();
    updateMismatchIndicators();
}

void PropertyEditorWidget::resetToDefaults()
{
    if (m_itemEditingManager) {
        int result = QMessageBox::question(this, "Reset Properties", 
            "Are you sure you want to reset all properties to their default values?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            
        if (result == QMessageBox::Yes) {
            m_itemEditingManager->resetToDefaults();
        }
        return;
    }
    
    // Fallback for direct mode
    if (!m_serverItem) {
        return;
    }
    
    int result = QMessageBox::question(this, "Reset Properties", 
        "Are you sure you want to reset all properties to their default values?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        
    if (result == QMessageBox::Yes) {
        beginBatchEdit();
        
        // Create a new server item with default values
        ServerItem defaultItem;
        
        // Copy default values to current values
        QStringList propertyNames = m_serverItem->getPropertyNames();
        for (const QString& propertyName : propertyNames) {
            QVariant defaultValue = defaultItem.getProperty(propertyName);
            m_currentValues[propertyName] = defaultValue;
            
            if (m_propertyFields.contains(propertyName)) {
                m_propertyFields[propertyName]->setValue(defaultValue);
            }
        }
        
        endBatchEdit();
    }
}

void PropertyEditorWidget::copyFromClient()
{
    if (m_itemEditingManager) {
        m_itemEditingManager->copyFromClient();
        return;
    }
    
    // Fallback for direct mode
    if (!m_serverItem || !m_clientItem) {
        return;
    }
    
    beginBatchEdit();
    
    // Copy matching properties from client item
    // This would require a mapping between server and client properties
    // For now, we'll copy basic properties that have the same names
    
    QStringList clientProperties = m_clientItem->getPropertyNames();
    QStringList serverProperties = m_serverItem->getPropertyNames();
    
    for (const QString& propertyName : clientProperties) {
        if (serverProperties.contains(propertyName)) {
            QVariant clientValue = m_clientItem->getProperty(propertyName);
            m_currentValues[propertyName] = clientValue;
            
            if (m_propertyFields.contains(propertyName)) {
                m_propertyFields[propertyName]->setValue(clientValue);
            }
        }
    }
    
    endBatchEdit();
}

void PropertyEditorWidget::showMismatchesOnly(bool show)
{
    setShowMismatchesOnly(show);
}

// Private slots
void PropertyEditorWidget::onPropertyValueChanged()
{
    PropertyEditorField* field = qobject_cast<PropertyEditorField*>(sender());
    if (!field) {
        return;
    }
    
    QString propertyName = field->getPropertyName();
    QVariant oldValue = m_currentValues.value(propertyName);
    QVariant newValue = field->getValue();
    
    if (oldValue != newValue) {
        // Use ItemEditingManager if available, otherwise handle directly
        if (m_itemEditingManager) {
            m_itemEditingManager->setProperty(propertyName, newValue);
        } else {
            // Fallback to direct handling
            m_currentValues[propertyName] = newValue;
            
            // Start validation timer
            m_validationTimer->start();
            
            // Update control states
            if (!m_batchEditing) {
                updateControlStates();
            }
            
            emit propertyChanged(propertyName, oldValue, newValue);
        }
    }
}

void PropertyEditorWidget::onValidationTimer()
{
    validateAllProperties();
}

void PropertyEditorWidget::onFieldFocused(const QString& propertyName)
{
    emit propertyFocused(propertyName);
}

void PropertyEditorWidget::onFieldLostFocus(const QString& propertyName)
{
    // Validate the specific property when focus is lost
    validateProperty(propertyName);
}

void PropertyEditorWidget::onItemEditingManagerPropertyChanged(const QString& propertyName, const QVariant& oldValue, const QVariant& newValue)
{
    // Update the property field to reflect the change
    if (m_propertyFields.contains(propertyName)) {
        PropertyEditorField* field = m_propertyFields[propertyName];
        field->setValue(newValue);
        m_currentValues[propertyName] = newValue;
    }
    
    updateControlStates();
}

void PropertyEditorWidget::onItemEditingManagerItemSelected(ItemId id)
{
    if (m_itemEditingManager) {
        ServerItem* serverItem = m_itemEditingManager->getCurrentItem();
        const ClientItem* clientItem = m_itemEditingManager->getClientItem();
        setItems(serverItem, clientItem);
    }
}

void PropertyEditorWidget::onItemEditingManagerItemDeselected()
{
    clearItems();
}

void PropertyEditorWidget::onItemEditingManagerValidationStateChanged(bool hasErrors)
{
    // Update UI to reflect validation state
    updateValidationStates();
    emit validationStateChanged(hasErrors);
}

// Private methods
void PropertyEditorWidget::populateProperties()
{
    if (!m_serverItem) {
        return;
    }
    
    // Clear existing fields
    for (auto it = m_propertyFields.begin(); it != m_propertyFields.end(); ++it) {
        delete it.value();
    }
    m_propertyFields.clear();
    
    // Get all property names from server item
    QStringList propertyNames = m_serverItem->getPropertyNames();
    
    for (const QString& propertyName : propertyNames) {
        QVariant value = getPropertyValue(propertyName);
        QString displayName = propertyName; // Could be enhanced with proper display names
        QString category = getPropertyCategory(propertyName);
        
        createPropertyField(propertyName, displayName, value, category);
    }
    
    layoutPropertyGroups();
}

void PropertyEditorWidget::createPropertyField(const QString& propertyName, const QString& displayName, 
                                             const QVariant& value, const QString& category)
{
    PropertyEditorField* field = new PropertyEditorField(propertyName, displayName, value, this);
    field->setReadOnly(m_readOnly);
    
    // Connect field signals
    connect(field, &PropertyEditorField::valueChanged, this, &PropertyEditorWidget::onPropertyValueChanged);
    connect(field, &PropertyEditorField::focused, this, &PropertyEditorWidget::onFieldFocused);
    connect(field, &PropertyEditorField::lostFocus, this, &PropertyEditorWidget::onFieldLostFocus);
    
    // Set tooltip if enabled
    if (m_showTooltips) {
        QString tooltip = getPropertyTooltip(propertyName);
        if (!tooltip.isEmpty()) {
            field->setToolTip(tooltip);
        }
    }
    
    // Add to appropriate group
    QGroupBox* group = nullptr;
    if (category == "Basic") {
        group = m_basicGroup;
    } else if (category == "Appearance") {
        group = m_appearanceGroup;
    } else if (category == "Behavior") {
        group = m_behaviorGroup;
    } else if (category == "Combat") {
        group = m_combatGroup;
    } else if (category == "Trade") {
        group = m_tradeGroup;
    } else if (category == "Container") {
        group = m_containerGroup;
    } else if (category == "ReadWrite") {
        group = m_readWriteGroup;
    } else {
        group = m_customGroup;
    }
    
    if (group) {
        QFormLayout* layout = qobject_cast<QFormLayout*>(group->layout());
        if (layout) {
            layout->addRow(displayName + ":", field);
        }
    }
    
    m_propertyFields[propertyName] = field;
    m_currentValues[propertyName] = value;
}

void PropertyEditorWidget::updatePropertyField(const QString& propertyName, const QVariant& value)
{
    if (m_propertyFields.contains(propertyName)) {
        PropertyEditorField* field = m_propertyFields[propertyName];
        field->setValue(value);
        m_currentValues[propertyName] = value;
    }
}

void PropertyEditorWidget::updateMismatchIndicators()
{
    if (!m_clientItem) {
        return;
    }
    
    for (auto it = m_propertyFields.begin(); it != m_propertyFields.end(); ++it) {
        const QString& propertyName = it.key();
        PropertyEditorField* field = it.value();
        
        // Check if there's a mismatch with client data
        QVariant serverValue = field->getValue();
        QVariant clientValue = m_clientItem->getProperty(propertyName);
        
        bool hasMismatch = clientValue.isValid() && serverValue != clientValue;
        field->setMismatchHighlight(hasMismatch);
        
        if (hasMismatch) {
            field->setExpectedValue(clientValue);
            field->setValidationState(PropertyEditorField::Mismatch);
            field->setValidationMessage(QString("Expected: %1").arg(clientValue.toString()));
            emit mismatchDetected(propertyName);
        }
    }
}

void PropertyEditorWidget::updateValidationStates()
{
    for (auto it = m_propertyFields.begin(); it != m_propertyFields.end(); ++it) {
        const QString& propertyName = it.key();
        PropertyEditorField* field = it.value();
        
        if (m_validationErrors.contains(propertyName)) {
            field->setValidationState(PropertyEditorField::Error);
        } else if (field->hasMismatchHighlight()) {
            field->setValidationState(PropertyEditorField::Mismatch);
        } else {
            field->setValidationState(PropertyEditorField::Valid);
        }
    }
}

void PropertyEditorWidget::updateControlStates()
{
    bool hasChangesFlag = hasChanges();
    bool hasErrors = hasValidationErrors();
    
    m_applyButton->setEnabled(!m_readOnly && hasChangesFlag && !hasErrors);
    m_revertButton->setEnabled(!m_readOnly && hasChangesFlag);
    m_resetButton->setEnabled(!m_readOnly);
    m_copyFromClientButton->setEnabled(!m_readOnly && m_clientItem != nullptr);
}

bool PropertyEditorWidget::isPropertyVisible(const QString& propertyName) const
{
    // Apply property filter
    if (!m_propertyFilter.isEmpty()) {
        if (!propertyName.contains(m_propertyFilter, Qt::CaseInsensitive)) {
            return false;
        }
    }
    
    // Apply category filter
    if (!m_propertyCategory.isEmpty()) {
        QString category = getPropertyCategory(propertyName);
        if (category != m_propertyCategory) {
            return false;
        }
    }
    
    // Apply mismatch filter
    if (m_showMismatchesOnly) {
        if (m_propertyFields.contains(propertyName)) {
            PropertyEditorField* field = m_propertyFields[propertyName];
            if (!field->hasMismatchHighlight()) {
                return false;
            }
        }
    }
    
    return true;
}

QVariant PropertyEditorWidget::getPropertyValue(const QString& propertyName) const
{
    if (m_serverItem) {
        return m_serverItem->getProperty(propertyName);
    }
    return QVariant();
}

void PropertyEditorWidget::setPropertyValue(const QString& propertyName, const QVariant& value)
{
    if (m_serverItem) {
        m_serverItem->setProperty(propertyName, value);
    }
}

void PropertyEditorWidget::storeOriginalValues()
{
    m_originalValues.clear();
    m_currentValues.clear();
    
    if (m_serverItem) {
        QStringList propertyNames = m_serverItem->getPropertyNames();
        for (const QString& propertyName : propertyNames) {
            QVariant value = getPropertyValue(propertyName);
            m_originalValues[propertyName] = value;
            m_currentValues[propertyName] = value;
        }
    }
}

void PropertyEditorWidget::detectMismatches()
{
    updateMismatchIndicators();
}

QString PropertyEditorWidget::getPropertyCategory(const QString& propertyName) const
{
    // Categorize properties based on their names
    if (propertyName == "id" || propertyName == "name" || propertyName == "type" || 
        propertyName == "clientId" || propertyName == "description") {
        return "Basic";
    } else if (propertyName == "width" || propertyName == "height" || propertyName == "layers" ||
               propertyName == "patternX" || propertyName == "patternY" || propertyName == "patternZ" ||
               propertyName == "frames" || propertyName == "spriteHash") {
        return "Appearance";
    } else if (propertyName == "flags" || propertyName == "stackOrder" || propertyName == "speed" ||
               propertyName == "elevation") {
        return "Behavior";
    } else if (propertyName == "weaponType" || propertyName == "ammoType" || propertyName == "shootType" ||
               propertyName == "attack" || propertyName == "defense" || propertyName == "armor") {
        return "Combat";
    } else if (propertyName == "tradeAs" || propertyName == "showAs") {
        return "Trade";
    } else if (propertyName == "containerSize") {
        return "Container";
    } else if (propertyName == "maxReadChars" || propertyName == "maxWriteChars" || 
               propertyName == "maxReadWriteChars") {
        return "ReadWrite";
    } else {
        return "Custom";
    }
}

QString PropertyEditorWidget::getPropertyTooltip(const QString& propertyName) const
{
    // Provide detailed tooltips for each property
    static QHash<QString, QString> tooltips = {
        {"id", "Unique identifier for the server item"},
        {"clientId", "Client item ID for sprite data reference"},
        {"name", "Display name of the item"},
        {"type", "Item type classification"},
        {"description", "Detailed description of the item"},
        {"width", "Item width in tiles"},
        {"height", "Item height in tiles"},
        {"layers", "Number of sprite layers"},
        {"patternX", "Horizontal pattern variations"},
        {"patternY", "Vertical pattern variations"},
        {"patternZ", "Depth pattern variations"},
        {"frames", "Number of animation frames"},
        {"spriteHash", "MD5 hash of sprite data"},
        {"flags", "Item behavior flags"},
        {"stackOrder", "Stacking order on tiles"},
        {"speed", "Movement speed modifier"},
        {"elevation", "Height above ground level"},
        {"lightLevel", "Light intensity (0-255)"},
        {"lightColor", "Light color value"},
        {"minimapColor", "Color on minimap"},
        {"tradeAs", "Trade classification ID"},
        {"showAs", "Display as different item"},
        {"weaponType", "Weapon classification"},
        {"ammoType", "Ammunition type"},
        {"shootType", "Projectile type"},
        {"attack", "Attack power"},
        {"defense", "Defense value"},
        {"armor", "Armor rating"},
        {"containerSize", "Container capacity"},
        {"maxReadChars", "Maximum readable characters"},
        {"maxWriteChars", "Maximum writable characters"},
        {"maxReadWriteChars", "Maximum read/write characters"}
    };
    
    return tooltips.value(propertyName, QString("Property: %1").arg(propertyName));
}

QWidget* PropertyEditorWidget::createEditorForProperty(const QString& propertyName, const QVariant& value)
{
    // Create appropriate editor widget based on property type and name
    QWidget* editor = nullptr;
    
    switch (value.type()) {
        case QVariant::Bool:
            editor = new QCheckBox();
            break;
        case QVariant::Int:
        case QVariant::UInt:
            {
                QSpinBox* spinBox = new QSpinBox();
                spinBox->setRange(0, 65535);
                editor = spinBox;
            }
            break;
        case QVariant::Double:
            {
                QDoubleSpinBox* doubleSpinBox = new QDoubleSpinBox();
                doubleSpinBox->setRange(0.0, 999999.0);
                editor = doubleSpinBox;
            }
            break;
        case QVariant::String:
            {
                if (propertyName == "description") {
                    QTextEdit* textEdit = new QTextEdit();
                    textEdit->setMaximumHeight(80);
                    editor = textEdit;
                } else {
                    editor = new QLineEdit();
                }
            }
            break;
        case QVariant::ByteArray:
            {
                QLineEdit* lineEdit = new QLineEdit();
                lineEdit->setReadOnly(true);
                lineEdit->setPlaceholderText("Binary data");
                editor = lineEdit;
            }
            break;
        default:
            editor = new QLineEdit();
            break;
    }
    
    return editor;
}

void PropertyEditorWidget::layoutPropertyGroups()
{
    // Ensure all groups are properly laid out
    QGroupBox* groups[] = {
        m_basicGroup, m_appearanceGroup, m_behaviorGroup, m_combatGroup,
        m_tradeGroup, m_containerGroup, m_readWriteGroup, m_customGroup
    };
    
    for (QGroupBox* group : groups) {
        if (group && group->layout()) {
            QFormLayout* layout = qobject_cast<QFormLayout*>(group->layout());
            if (layout && layout->rowCount() == 0) {
                group->hide();
            } else {
                group->show();
            }
        }
    }
}

// PropertyEditorField Implementation
PropertyEditorField::PropertyEditorField(const QString& propertyName, const QString& displayName, 
                                       const QVariant& value, QWidget* parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_nameLabel(nullptr)
    , m_editorWidget(nullptr)
    , m_validationIndicator(nullptr)
    , m_propertyName(propertyName)
    , m_displayName(displayName)
    , m_value(value)
    , m_originalValue(value)
    , m_validationState(Valid)
    , m_mismatchHighlight(false)
    , m_readOnly(false)
    , m_highlightAnimation(nullptr)
    , m_colorizeEffect(nullptr)
{
    setupUI();
    setValue(value);
}

PropertyEditorField::~PropertyEditorField()
{
    // Qt's parent-child system handles cleanup
}

void PropertyEditorField::setupUI()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(2, 2, 2, 2);
    m_layout->setSpacing(4);
    
    // Create name label
    m_nameLabel = new QLabel(m_displayName, this);
    m_nameLabel->setMinimumWidth(120);
    m_nameLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    
    // Create editor widget
    m_editorWidget = createEditorForValue(m_value);
    if (m_editorWidget) {
        connectEditorSignals();
    }
    
    // Create validation indicator
    m_validationIndicator = new ValidationIndicator(this);
    m_validationIndicator->setFixedSize(16, 16);
    m_validationIndicator->setState(m_validationState);
    
    // Add widgets to layout
    m_layout->addWidget(m_nameLabel);
    m_layout->addWidget(m_editorWidget, 1);
    m_layout->addWidget(m_validationIndicator);
    
    // Setup colorize effect for mismatch highlighting
    m_colorizeEffect = new QGraphicsColorizeEffect(this);
    m_colorizeEffect->setColor(QColor(255, 165, 0)); // Orange
    m_colorizeEffect->setStrength(0.0);
    
    // Setup highlight animation
    m_highlightAnimation = new QPropertyAnimation(m_colorizeEffect, "strength", this);
    m_highlightAnimation->setDuration(300);
    connect(m_highlightAnimation, &QPropertyAnimation::finished, 
            this, &PropertyEditorField::onValidationAnimationFinished);
    
    setGraphicsEffect(m_colorizeEffect);
}

QString PropertyEditorField::getPropertyName() const
{
    return m_propertyName;
}

QString PropertyEditorField::getDisplayName() const
{
    return m_displayName;
}

QVariant PropertyEditorField::getValue() const
{
    if (!m_editorWidget) {
        return m_value;
    }
    
    // Extract value from editor widget
    if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(m_editorWidget)) {
        return checkBox->isChecked();
    } else if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(m_editorWidget)) {
        return spinBox->value();
    } else if (QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(m_editorWidget)) {
        return doubleSpinBox->value();
    } else if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(m_editorWidget)) {
        return lineEdit->text();
    } else if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(m_editorWidget)) {
        return textEdit->toPlainText();
    } else if (QComboBox* comboBox = qobject_cast<QComboBox*>(m_editorWidget)) {
        return comboBox->currentData();
    }
    
    return m_value;
}

void PropertyEditorField::setValue(const QVariant& value)
{
    if (m_value == value) {
        return;
    }
    
    QVariant oldValue = m_value;
    m_value = value;
    
    // Update editor widget
    if (m_editorWidget) {
        disconnectEditorSignals();
        
        if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(m_editorWidget)) {
            checkBox->setChecked(value.toBool());
        } else if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(m_editorWidget)) {
            spinBox->setValue(value.toInt());
        } else if (QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(m_editorWidget)) {
            doubleSpinBox->setValue(value.toDouble());
        } else if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(m_editorWidget)) {
            lineEdit->setText(value.toString());
        } else if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(m_editorWidget)) {
            textEdit->setPlainText(value.toString());
        } else if (QComboBox* comboBox = qobject_cast<QComboBox*>(m_editorWidget)) {
            int index = comboBox->findData(value);
            if (index >= 0) {
                comboBox->setCurrentIndex(index);
            }
        }
        
        connectEditorSignals();
    }
    
    emit valueChanged(oldValue, value);
}

QVariant PropertyEditorField::getOriginalValue() const
{
    return m_originalValue;
}

void PropertyEditorField::setOriginalValue(const QVariant& value)
{
    m_originalValue = value;
}

void PropertyEditorField::setValidationState(ValidationState state)
{
    if (m_validationState != state) {
        m_validationState = state;
        m_validationIndicator->setState(state);
        updateValidationIndicator();
        animateValidationChange();
    }
}

PropertyEditorField::ValidationState PropertyEditorField::getValidationState() const
{
    return m_validationState;
}

void PropertyEditorField::setValidationMessage(const QString& message)
{
    m_validationMessage = message;
    m_validationIndicator->setMessage(message);
    setToolTip(message);
}

QString PropertyEditorField::getValidationMessage() const
{
    return m_validationMessage;
}

void PropertyEditorField::setExpectedValue(const QVariant& value)
{
    m_expectedValue = value;
    
    // Update tooltip to show expected value
    QString tooltip = m_validationMessage;
    if (!tooltip.isEmpty() && value.isValid()) {
        tooltip += QString("\nExpected: %1").arg(value.toString());
    }
    setToolTip(tooltip);
}

QVariant PropertyEditorField::getExpectedValue() const
{
    return m_expectedValue;
}

void PropertyEditorField::setMismatchHighlight(bool highlight)
{
    if (m_mismatchHighlight != highlight) {
        m_mismatchHighlight = highlight;
        updateHighlight();
    }
}

bool PropertyEditorField::hasMismatchHighlight() const
{
    return m_mismatchHighlight;
}

void PropertyEditorField::setReadOnly(bool readOnly)
{
    if (m_readOnly != readOnly) {
        m_readOnly = readOnly;
        
        if (m_editorWidget) {
            m_editorWidget->setEnabled(!readOnly);
        }
    }
}

bool PropertyEditorField::isReadOnly() const
{
    return m_readOnly;
}

QWidget* PropertyEditorField::getEditorWidget() const
{
    return m_editorWidget;
}

void PropertyEditorField::setEditorWidget(QWidget* widget)
{
    if (m_editorWidget == widget) {
        return;
    }
    
    if (m_editorWidget) {
        disconnectEditorSignals();
        m_layout->removeWidget(m_editorWidget);
        m_editorWidget->deleteLater();
    }
    
    m_editorWidget = widget;
    
    if (m_editorWidget) {
        m_layout->insertWidget(1, m_editorWidget, 1);
        connectEditorSignals();
        m_editorWidget->setEnabled(!m_readOnly);
    }
}

void PropertyEditorField::focusInEvent(QFocusEvent* event)
{
    QWidget::focusInEvent(event);
    emit focused(m_propertyName);
}

void PropertyEditorField::focusOutEvent(QFocusEvent* event)
{
    QWidget::focusOutEvent(event);
    emit lostFocus(m_propertyName);
}

void PropertyEditorField::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    
    // Draw mismatch border if highlighted
    if (m_mismatchHighlight) {
        QPainter painter(this);
        painter.setPen(QPen(QColor(255, 165, 0), 2)); // Orange border
        painter.drawRect(rect().adjusted(1, 1, -1, -1));
    }
}

void PropertyEditorField::onEditorValueChanged()
{
    QVariant newValue = getValue();
    if (newValue != m_value) {
        QVariant oldValue = m_value;
        m_value = newValue;
        emit valueChanged(oldValue, newValue);
        emit validationRequested();
    }
}

void PropertyEditorField::onValidationAnimationFinished()
{
    // Animation finished, keep the final state
}

void PropertyEditorField::updateValidationIndicator()
{
    if (m_validationIndicator) {
        m_validationIndicator->setState(m_validationState);
        m_validationIndicator->setMessage(m_validationMessage);
    }
}

void PropertyEditorField::updateHighlight()
{
    if (m_mismatchHighlight) {
        m_highlightAnimation->setStartValue(0.0);
        m_highlightAnimation->setEndValue(0.3);
        m_highlightAnimation->start();
    } else {
        m_highlightAnimation->setStartValue(0.3);
        m_highlightAnimation->setEndValue(0.0);
        m_highlightAnimation->start();
    }
    
    update(); // Trigger repaint for border
}

void PropertyEditorField::animateValidationChange()
{
    // Animate validation state change
    if (m_validationState == Error || m_validationState == Mismatch) {
        // Flash red/orange for errors
        m_highlightAnimation->setStartValue(0.0);
        m_highlightAnimation->setEndValue(0.5);
        m_highlightAnimation->start();
    }
}

QWidget* PropertyEditorField::createEditorForValue(const QVariant& value)
{
    QWidget* editor = nullptr;
    
    switch (value.type()) {
        case QVariant::Bool:
            {
                QCheckBox* checkBox = new QCheckBox(this);
                checkBox->setChecked(value.toBool());
                editor = checkBox;
            }
            break;
        case QVariant::Int:
        case QVariant::UInt:
            {
                QSpinBox* spinBox = new QSpinBox(this);
                spinBox->setRange(0, 65535);
                spinBox->setValue(value.toInt());
                editor = spinBox;
            }
            break;
        case QVariant::Double:
            {
                QDoubleSpinBox* doubleSpinBox = new QDoubleSpinBox(this);
                doubleSpinBox->setRange(0.0, 999999.0);
                doubleSpinBox->setValue(value.toDouble());
                editor = doubleSpinBox;
            }
            break;
        case QVariant::String:
            {
                if (m_propertyName == "description") {
                    QTextEdit* textEdit = new QTextEdit(this);
                    textEdit->setMaximumHeight(80);
                    textEdit->setPlainText(value.toString());
                    editor = textEdit;
                } else {
                    QLineEdit* lineEdit = new QLineEdit(this);
                    lineEdit->setText(value.toString());
                    editor = lineEdit;
                }
            }
            break;
        case QVariant::ByteArray:
            {
                QLineEdit* lineEdit = new QLineEdit(this);
                lineEdit->setReadOnly(true);
                lineEdit->setText(value.toByteArray().toHex());
                lineEdit->setPlaceholderText("Binary data (hex)");
                editor = lineEdit;
            }
            break;
        default:
            {
                QLineEdit* lineEdit = new QLineEdit(this);
                lineEdit->setText(value.toString());
                editor = lineEdit;
            }
            break;
    }
    
    return editor;
}

void PropertyEditorField::connectEditorSignals()
{
    if (!m_editorWidget) {
        return;
    }
    
    if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(m_editorWidget)) {
        connect(checkBox, &QCheckBox::toggled, this, &PropertyEditorField::onEditorValueChanged);
    } else if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(m_editorWidget)) {
        connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
                this, &PropertyEditorField::onEditorValueChanged);
    } else if (QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(m_editorWidget)) {
        connect(doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
                this, &PropertyEditorField::onEditorValueChanged);
    } else if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(m_editorWidget)) {
        connect(lineEdit, &QLineEdit::textChanged, this, &PropertyEditorField::onEditorValueChanged);
    } else if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(m_editorWidget)) {
        connect(textEdit, &QTextEdit::textChanged, this, &PropertyEditorField::onEditorValueChanged);
    } else if (QComboBox* comboBox = qobject_cast<QComboBox*>(m_editorWidget)) {
        connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                this, &PropertyEditorField::onEditorValueChanged);
    }
}

void PropertyEditorField::disconnectEditorSignals()
{
    if (!m_editorWidget) {
        return;
    }
    
    // Disconnect all signals from the editor widget
    disconnect(m_editorWidget, nullptr, this, nullptr);
}

// ValidationIndicator Implementation
ValidationIndicator::ValidationIndicator(QWidget* parent)
    : QWidget(parent)
    , m_state(PropertyEditorField::Valid)
    , m_hovered(false)
{
    setFixedSize(16, 16);
    setMouseTracking(true);
}

void ValidationIndicator::setState(PropertyEditorField::ValidationState state)
{
    if (m_state != state) {
        m_state = state;
        update();
    }
}

PropertyEditorField::ValidationState ValidationIndicator::getState() const
{
    return m_state;
}

void ValidationIndicator::setMessage(const QString& message)
{
    m_message = message;
    setToolTip(message);
}

QString ValidationIndicator::getMessage() const
{
    return m_message;
}

void ValidationIndicator::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QColor color = getStateColor();
    QIcon icon = getStateIcon();
    
    if (!icon.isNull()) {
        icon.paint(&painter, rect());
    } else {
        // Draw simple colored circle
        painter.setBrush(color);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(rect().adjusted(2, 2, -2, -2));
    }
    
    if (m_hovered && !m_message.isEmpty()) {
        showTooltip();
    }
}

void ValidationIndicator::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event)
    if (!m_message.isEmpty()) {
        showTooltip();
    }
}

void ValidationIndicator::enterEvent(QEnterEvent* event)
{
    Q_UNUSED(event)
    m_hovered = true;
    update();
}

void ValidationIndicator::leaveEvent(QEvent* event)
{
    Q_UNUSED(event)
    m_hovered = false;
    update();
}

QColor ValidationIndicator::getStateColor() const
{
    switch (m_state) {
        case PropertyEditorField::Valid:
            return QColor(0, 255, 0, 128); // Green
        case PropertyEditorField::Warning:
            return QColor(255, 255, 0, 128); // Yellow
        case PropertyEditorField::Error:
            return QColor(255, 0, 0, 128); // Red
        case PropertyEditorField::Mismatch:
            return QColor(255, 165, 0, 128); // Orange
        default:
            return QColor(128, 128, 128, 128); // Gray
    }
}

QIcon ValidationIndicator::getStateIcon() const
{
    // Return appropriate icons for each state
    // For now, return empty icon and use colored circles
    return QIcon();
}

void ValidationIndicator::showTooltip()
{
    if (!m_message.isEmpty()) {
        QToolTip::showText(mapToGlobal(rect().bottomLeft()), m_message, this);
    }
}

#include "PropertyEditorWidget.moc"