#pragma once

#include <QWidget>
#include <QGroupBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolTip>
#include "otb/otbtypes.h"
#include "otb/item.h"

namespace UI {
namespace Widgets {

/**
 * @brief Comprehensive item property editor widget
 * 
 * Port of the property editing functionality from C# MainForm.Designer.cs
 * Provides complete editing interface for all ServerItem properties with
 * real-time validation and visual feedback.
 */
class ItemPropertyEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ItemPropertyEditor(QWidget *parent = nullptr);
    virtual ~ItemPropertyEditor();

    // Main interface
    void setServerItem(OTB::ServerItem* item);
    void setClientItem(const ItemEditor::ClientItem* clientItem);
    void clear();
    
    // Enable/disable editing
    void setEnabled(bool enabled);

signals:
    // Property change signals
    void itemPropertyChanged();
    void clientIdChanged(quint16 newId);
    void itemNameChanged(const QString& newName);
    void itemTypeChanged(OTB::ServerItemType newType);
    void stackOrderChanged(OTB::TileStackOrder newOrder);
    
    // Flag change signals
    void unpassableChanged(bool value);
    void blockMissilesChanged(bool value);
    void blockPathfinderChanged(bool value);
    void hasElevationChanged(bool value);
    void forceUseChanged(bool value);
    void multiUseChanged(bool value);
    void pickupableChanged(bool value);
    void movableChanged(bool value);
    void stackableChanged(bool value);
    void readableChanged(bool value);
    void rotatableChanged(bool value);
    void hangableChanged(bool value);
    void hookSouthChanged(bool value);
    void hookEastChanged(bool value);
    void ignoreLookChanged(bool value);
    void fullGroundChanged(bool value);
    
    // Attribute change signals
    void groundSpeedChanged(quint16 value);
    void lightLevelChanged(quint16 value);
    void lightColorChanged(quint16 value);
    void minimapColorChanged(quint16 value);
    void maxReadCharsChanged(quint16 value);
    void maxReadWriteCharsChanged(quint16 value);
    void tradeAsChanged(quint16 value);

private slots:
    // Internal slots for handling UI changes
    void onClientIdChanged(int value);
    void onNameChanged(const QString& text);
    void onTypeChanged(int index);
    void onStackOrderChanged(int index);
    void onFlagChanged();
    void onAttributeChanged();

private:
    void setupUI();
    void setupConnections();
    void updateControls();
    void updateValidationStyling();
    void updatePropertyStyle(QWidget* control, bool matches);
    void updatePropertyStyleWithTooltip(QWidget* control, bool matches, const QString& expectedValue);
    void resetStyling();
    
    // UI Groups
    QGroupBox* m_appearanceGroup;
    QGroupBox* m_attributesGroup;
    
    // Appearance controls
    QLabel* m_serverIdLabel;
    QSpinBox* m_clientIdSpinBox;
    QComboBox* m_typeComboBox;
    QComboBox* m_stackOrderComboBox;
    QLineEdit* m_nameLineEdit;
    
    // Flag checkboxes (matching C# form layout)
    QCheckBox* m_unpassableCheck;
    QCheckBox* m_blockMissilesCheck;
    QCheckBox* m_blockPathfinderCheck;
    QCheckBox* m_hasElevationCheck;
    QCheckBox* m_forceUseCheck;
    QCheckBox* m_multiUseCheck;
    QCheckBox* m_pickupableCheck;
    QCheckBox* m_movableCheck;
    QCheckBox* m_stackableCheck;
    QCheckBox* m_readableCheck;
    QCheckBox* m_rotatableCheck;
    QCheckBox* m_hangableCheck;
    QCheckBox* m_hookSouthCheck;
    QCheckBox* m_hookEastCheck;
    QCheckBox* m_ignoreLookCheck;
    QCheckBox* m_fullGroundCheck;
    
    // Attribute line edits
    QLineEdit* m_groundSpeedEdit;
    QLineEdit* m_lightLevelEdit;
    QLineEdit* m_lightColorEdit;
    QLineEdit* m_minimapColorEdit;
    QLineEdit* m_maxReadCharsEdit;
    QLineEdit* m_maxReadWriteCharsEdit;
    QLineEdit* m_tradeAsEdit;
    
    // Labels for attributes
    QLabel* m_groundSpeedLabel;
    QLabel* m_lightLevelLabel;
    QLabel* m_lightColorLabel;
    QLabel* m_minimapColorLabel;
    QLabel* m_maxReadCharsLabel;
    QLabel* m_maxReadWriteCharsLabel;
    QLabel* m_tradeAsLabel;
    
    // Data
    OTB::ServerItem* m_currentServerItem;
    const ItemEditor::ClientItem* m_currentClientItem;
    bool m_updatingControls;
    
    // Style colors for validation feedback
    QColor m_normalTextColor;
    QColor m_mismatchTextColor;
};

} // namespace Widgets
} // namespace UI