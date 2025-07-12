#ifndef UPDATEOTBDIALOG_H
#define UPDATEOTBDIALOG_H

#include <QDialog>
#include "otb/item.h" // For OTB::SupportedClient

QT_BEGIN_NAMESPACE
class QComboBox;
class QCheckBox;
class QDialogButtonBox;
class QLabel;
class QGroupBox;
class QVBoxLayout;
QT_END_NAMESPACE

class PluginManager; // Forward declaration
class IPlugin;       // Forward declaration

// Structure to hold the selected update settings
struct UpdateOptions {
    bool reassignUnmatchedSprites = true;
    bool generateImageSignatures = false; // C# default is false, it's slow
    bool reloadItemAttributes = true;
    bool createNewItems = true;
    OTB::SupportedClient targetClient;
    IPlugin* targetPlugin = nullptr;
};

class UpdateOtbDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateOtbDialog(PluginManager* pluginManager, quint32 currentOtbVersion, QWidget *parent = nullptr);
    ~UpdateOtbDialog();

    UpdateOptions getSelectedUpdateOptions() const;

private slots:
    void onClientVersionSelected(int index);
    void accept() override;

private:
    void setupUi();
    void populateClientVersions();

    PluginManager* m_pluginManager;
    quint32 m_currentOtbVersion;

    // UI Elements
    QComboBox* targetClientComboBox;

    QGroupBox* optionsGroupBox;
    QCheckBox* reassignUnmatchedSpritesCheckBox;
    QCheckBox* generateSignaturesCheckBox;
    QCheckBox* reloadAttributesCheckBox;
    QCheckBox* createNewItemsCheckBox;

    QDialogButtonBox* buttonBox;

    // Selected options
    UpdateOptions m_options;
};

#endif // UPDATEOTBDIALOG_H
