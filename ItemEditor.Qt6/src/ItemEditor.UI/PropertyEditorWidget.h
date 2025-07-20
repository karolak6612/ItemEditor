#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QSlider>
#include <QTextEdit>
#include <QColorDialog>
#include <QTimer>
#include <QValidator>
#include <QRegularExpressionValidator>
#include <QToolTip>
#include <QPropertyAnimation>
#include <QGraphicsEffect>
#include <QGraphicsColorizeEffect>
#include <QHash>
#include <QVariant>
#include <functional>

#include "../ItemEditor.Core/ServerItem.h"
#include "../ItemEditor.Core/ClientItem.h"
#include "../ItemEditor.Core/ItemValidator.h"
#include "../ItemEditor.Core/ItemEnums.h"
#include "../ItemEditor.Core/ItemEditingManager.h"

class PropertyEditorField;
class ValidationIndicator;

/**
 * @brief Comprehensive property editor widget for item attributes
 * 
 * Provides advanced property editing with:
 * - Real-time validation and feedback
 * - Color-coded mismatch indicators
 * - Tooltip display for expected values
 * - Undo/redo support for changes
 * - Batch editing capabilities
 * - Custom property editors for different data types
 * - Visual feedback for validation states
 */
class PropertyEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PropertyEditorWidget(QWidget *parent = nullptr);
    ~PropertyEditorWidget();

    // Integration with ItemEditingManager
    void setItemEditingManager(ItemEditingManager* manager);
    ItemEditingManager* getItemEditingManager() const;

    // Data management
    void setServerItem(ServerItem* item);
    void setClientItem(const ClientItem* item);
    void setItems(ServerItem* serverItem, const ClientItem* clientItem);
    void clearItems();
    
    ServerItem* getServerItem() const;
    const ClientItem* getClientItem() const;

    // Property editing
    void refreshProperties();
    void applyChanges();
    void revertChanges();
    bool hasChanges() const;
    bool hasValidationErrors() const;
    
    // Validation
    void validateAllProperties();
    void validateProperty(const QString& propertyName);
    QStringList getValidationErrors() const;
    QStringList getModifiedProperties() const;

    // Display options
    void setShowMismatchesOnly(bool show);
    bool getShowMismatchesOnly() const;
    void setShowTooltips(bool show);
    bool getShowTooltips() const;
    void setReadOnly(bool readOnly);
    bool isReadOnly() const;

    // Property filtering
    void setPropertyFilter(const QString& filter);
    void setPropertyCategory(const QString& category);
    void clearFilters();

    // Batch operations
    void beginBatchEdit();
    void endBatchEdit();
    bool isBatchEditing() const;

signals:
    void propertyChanged(const QString& propertyName, const QVariant& oldValue, const QVariant& newValue);
    void validationStateChanged(bool hasErrors);
    void itemModified();
    void mismatchDetected(const QString& propertyName);
    void propertyFocused(const QString& propertyName);

public slots:
    void onServerItemChanged();
    void onClientItemChanged();
    void resetToDefaults();
    void copyFromClient();
    void showMismatchesOnly(bool show);

private slots:
    void onPropertyValueChanged();
    void onValidationTimer();
    void onFieldFocused(const QString& propertyName);
    void onFieldLostFocus(const QString& propertyName);
    void onItemEditingManagerPropertyChanged(const QString& propertyName, const QVariant& oldValue, const QVariant& newValue);
    void onItemEditingManagerItemSelected(ItemId id);
    void onItemEditingManagerItemDeselected();
    void onItemEditingManagerValidationStateChanged(bool hasErrors);

private:
    // UI components
    QVBoxLayout* m_mainLayout;
    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    QVBoxLayout* m_contentLayout;
    
    // Property groups
    QGroupBox* m_basicGroup;
    QGroupBox* m_appearanceGroup;
    QGroupBox* m_behaviorGroup;
    QGroupBox* m_combatGroup;
    QGroupBox* m_tradeGroup;
    QGroupBox* m_containerGroup;
    QGroupBox* m_readWriteGroup;
    QGroupBox* m_customGroup;
    
    // Control buttons
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_applyButton;
    QPushButton* m_revertButton;
    QPushButton* m_resetButton;
    QPushButton* m_copyFromClientButton;
    QCheckBox* m_showMismatchesCheck;
    
    // Data
    ServerItem* m_serverItem;
    const ClientItem* m_clientItem;
    QHash<QString, PropertyEditorField*> m_propertyFields;
    QHash<QString, QVariant> m_originalValues;
    QHash<QString, QVariant> m_currentValues;
    
    // Settings
    bool m_showMismatchesOnly;
    bool m_showTooltips;
    bool m_readOnly;
    bool m_batchEditing;
    QString m_propertyFilter;
    QString m_propertyCategory;
    
    // Validation
    QTimer* m_validationTimer;
    ItemValidator* m_validator;
    QStringList m_validationErrors;
    
    // Item editing integration
    ItemEditingManager* m_itemEditingManager;
    
    // Private methods
    void setupUI();
    void setupPropertyGroups();
    void setupControls();
    void connectSignals();
    void applyDarkTheme();
    void populateProperties();
    void createPropertyField(const QString& propertyName, const QString& displayName, 
                           const QVariant& value, const QString& category);
    void updatePropertyField(const QString& propertyName, const QVariant& value);
    void updateMismatchIndicators();
    void updateValidationStates();
    void updateControlStates();
    bool isPropertyVisible(const QString& propertyName) const;
    QVariant getPropertyValue(const QString& propertyName) const;
    void setPropertyValue(const QString& propertyName, const QVariant& value);
    void storeOriginalValues();
    void detectMismatches();
    QString getPropertyCategory(const QString& propertyName) const;
    QString getPropertyTooltip(const QString& propertyName) const;
    QWidget* createEditorForProperty(const QString& propertyName, const QVariant& value);
    void layoutPropertyGroups();
};

/**
 * @brief Custom property editor field with validation and mismatch detection
 */
class PropertyEditorField : public QWidget
{
    Q_OBJECT

public:
    enum ValidationState {
        Valid,
        Warning,
        Error,
        Mismatch
    };

    explicit PropertyEditorField(const QString& propertyName, const QString& displayName, 
                               const QVariant& value, QWidget* parent = nullptr);
    ~PropertyEditorField();

    // Property management
    QString getPropertyName() const;
    QString getDisplayName() const;
    QVariant getValue() const;
    void setValue(const QVariant& value);
    QVariant getOriginalValue() const;
    void setOriginalValue(const QVariant& value);

    // Validation
    void setValidationState(ValidationState state);
    ValidationState getValidationState() const;
    void setValidationMessage(const QString& message);
    QString getValidationMessage() const;
    void setExpectedValue(const QVariant& value);
    QVariant getExpectedValue() const;

    // Appearance
    void setMismatchHighlight(bool highlight);
    bool hasMismatchHighlight() const;
    void setReadOnly(bool readOnly);
    bool isReadOnly() const;

    // Editor widget
    QWidget* getEditorWidget() const;
    void setEditorWidget(QWidget* widget);

signals:
    void valueChanged(const QVariant& oldValue, const QVariant& newValue);
    void validationRequested();
    void focused(const QString& propertyName);
    void lostFocus(const QString& propertyName);

protected:
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onEditorValueChanged();
    void onValidationAnimationFinished();

private:
    // UI components
    QHBoxLayout* m_layout;
    QLabel* m_nameLabel;
    QWidget* m_editorWidget;
    ValidationIndicator* m_validationIndicator;
    
    // Data
    QString m_propertyName;
    QString m_displayName;
    QVariant m_value;
    QVariant m_originalValue;
    QVariant m_expectedValue;
    
    // State
    ValidationState m_validationState;
    QString m_validationMessage;
    bool m_mismatchHighlight;
    bool m_readOnly;
    
    // Animation
    QPropertyAnimation* m_highlightAnimation;
    QGraphicsColorizeEffect* m_colorizeEffect;
    
    // Private methods
    void setupUI();
    void updateValidationIndicator();
    void updateHighlight();
    void animateValidationChange();
    QWidget* createEditorForValue(const QVariant& value);
    void connectEditorSignals();
    void disconnectEditorSignals();
};

/**
 * @brief Visual indicator for property validation state
 */
class ValidationIndicator : public QWidget
{
    Q_OBJECT

public:
    explicit ValidationIndicator(QWidget* parent = nullptr);
    
    void setState(PropertyEditorField::ValidationState state);
    PropertyEditorField::ValidationState getState() const;
    void setMessage(const QString& message);
    QString getMessage() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    PropertyEditorField::ValidationState m_state;
    QString m_message;
    bool m_hovered;
    
    QColor getStateColor() const;
    QIcon getStateIcon() const;
    void showTooltip();
};