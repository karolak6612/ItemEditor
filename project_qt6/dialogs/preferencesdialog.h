#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QList> // For QList<OTB::SupportedClient>

QT_BEGIN_NAMESPACE
class QComboBox;
class QLineEdit;
class QPushButton;
class QCheckBox;
class QGroupBox;
class QLabel;
class QVBoxLayout;
class QHBoxLayout;
class QDialogButtonBox;
QT_END_NAMESPACE

namespace OTB {
    struct SupportedClient; // Forward declaration
}

class IPlugin; // Forward declaration
class PluginManager; // Forward declaration

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(PluginManager* pluginManager, QWidget *parent = nullptr);
    ~PreferencesDialog();

    // Methods to get selected preferences (if needed by caller immediately)
    QString getSelectedClientDirectory() const;
    OTB::SupportedClient getSelectedClient() const; // Returns the chosen client
    IPlugin* getSelectedPlugin() const; // Returns the plugin that provides the chosen client
    bool isExtendedChecked() const;
    bool isFrameDurationsChecked() const;
    bool isTransparencyChecked() const;


private slots:
    void browseClientDirectory();
    void onPluginSelected(int index); // When a plugin is chosen from a (future) plugin list
    void onClientVersionSelected(int index); // When a client version of a plugin is chosen
    void accept() override; // To save settings before closing

private:
    void setupUi();
    void loadSettings();
    void saveSettings();
    void populateClientVersions(); // Populates client versions for the selected plugin

    PluginManager* m_pluginManager; // To get list of plugins and their supported clients

    // UI Elements
    QGroupBox* clientSelectionGroupBox;
    QLabel* clientVersionLabel;
    QComboBox* clientVersionComboBox; // Lists descriptions of OTB::SupportedClient

    QGroupBox* clientPathGroupBox;
    QLabel* clientDirectoryLabel;
    QLineEdit* clientDirectoryLineEdit;
    QPushButton* browseDirectoryButton;

    QGroupBox* clientOptionsGroupBox;
    QCheckBox* extendedCheckBox;
    QCheckBox* frameDurationsCheckBox;
    QCheckBox* transparencyCheckBox;
    QLabel* clientLoadingInfoLabel; // To show messages like "Restart required" or "Client will load on next OTB open"

    QDialogButtonBox* buttonBox;

    // Stored selections
    IPlugin* m_selectedPlugin; // Plugin that provides the m_selectedClient
    OTB::SupportedClient m_selectedClient; // The actual client version chosen
};

#endif // PREFERENCESDIALOG_H
