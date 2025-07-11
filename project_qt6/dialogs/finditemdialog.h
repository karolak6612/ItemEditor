#ifndef FINDITEMDIALOG_H
#define FINDITEMDIALOG_H

#include <QDialog>
#include <QList>
#include <QMap> // For mapping list items to server item IDs

QT_BEGIN_NAMESPACE
class QRadioButton;
class QLineEdit;
class QSpinBox;
class QCheckBox;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QGroupBox;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QLabel;
QT_END_NAMESPACE

namespace OTB {
    struct ServerItem; // Forward declaration
    struct ServerItemList; // Forward declaration for passing all items
}

class FindItemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindItemDialog(const OTB::ServerItemList& allItems, QWidget *parent = nullptr);
    ~FindItemDialog();

    quint16 getSelectedServerId() const; // Returns the Server ID of the selected item

private slots:
    void searchCriteriaModeChanged(); // Slot to enable/disable input fields based on radio button
    void findItems();
    void resultItemSelected(QListWidgetItem* item); // Updates goToItemButton enabled state
    void acceptFoundItem(); // Slot for "Go to Item" or double-click on list

private:
    void setupUi();

    // Search criteria UI
    QGroupBox* searchModeGroupBox; // Renamed from searchByGroupBox for clarity
    QRadioButton* findBySidRadioButton;
    QRadioButton* findByCidRadioButton;
    QRadioButton* findByNameRadioButton;
    QRadioButton* findByFlagsRadioButton;

    QGroupBox* valueInputGroupBox; // Unified group for ID or Name input
    QLabel* valueLabel; // Changes text based on mode (ID: or Name:)
    QSpinBox* idSpinBox;    // Used for SID/CID
    QLineEdit* nameLineEdit; // Used for Name

    QGroupBox* flagsGroupBox; // For flag checkboxes
    QGridLayout* flagsLayout; // Layout for flags
    // Checkboxes for flags
    QCheckBox* unpassableCheckBox;
    QCheckBox* blockMissilesCheckBox;
    QCheckBox* blockPathfinderCheckBox;
    QCheckBox* hasElevationCheckBox;
    QCheckBox* forceUseCheckBox;
    QCheckBox* multiUseCheckBox;
    QCheckBox* pickupableCheckBox;
    QCheckBox* movableCheckBox;
    QCheckBox* stackableCheckBox;
    QCheckBox* readableCheckBox;
    QCheckBox* rotatableCheckBox;
    QCheckBox* hangableCheckBox;
    QCheckBox* hookSouthCheckBox;
    QCheckBox* hookEastCheckBox;
    QCheckBox* ignoreLookCheckBox;
    QCheckBox* fullGroundCheckBox;
    QCheckBox* allowDistReadCheckBox; // Added
    QCheckBox* hasChargesCheckBox;    // Added
    QCheckBox* isAnimationCheckBox;   // Added


    // Results UI
    QGroupBox* resultsGroupBox;
    QListWidget* resultsListWidget;

    // Action buttons
    QPushButton* findButton;
    QPushButton* goToItemButton;
    QPushButton* cancelButton;

    // Data
    const OTB::ServerItemList& m_allItems;
    quint16 m_selectedServerIdInDialog; // Store SID of item selected in dialog's list
    QMap<QListWidgetItem*, quint16> m_resultItemToServerIdMap;
};

#endif // FINDITEMDIALOG_H
