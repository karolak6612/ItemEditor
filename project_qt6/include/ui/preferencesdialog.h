#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QList> // For QList<OTB::SupportedClient>
#include "otb/item.h" // For OTB::SupportedClient

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

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();

    // Methods to get selected preferences (if needed by caller immediately)
    QString getSelectedClientDirectory() const;
    OTB::SupportedClient getSelectedClient() const; // Returns the chosen client
    bool isExtendedChecked() const;
    bool isFrameDurationsChecked() const;
    bool isTransparencyChecked() const;


private slots:
    void browseClientDirectory();
    void onClientVersionSelected(int index); // When a client version is chosen
    void accept() override; // To save settings before closing

private:
    void setupUi();
    void loadSettings();
    void saveSettings();

    // UI Elements
    QGroupBox* clientSelectionGroupBox;
    QLabel* clientVersionLabel;
    QComboBox* clientVersionComboBox; // Lists client versions

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
};

#endif // PREFERENCESDIALOG_H
