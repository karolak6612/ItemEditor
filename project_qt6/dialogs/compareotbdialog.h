#ifndef COMPAREOTBDIALOG_H
#define COMPAREOTBDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QTextEdit;
class QLabel;
class QGroupBox;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
QT_END_NAMESPACE

namespace OTB {
    struct ServerItemList; // Forward declaration
}

class CompareOtbDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CompareOtbDialog(QWidget *parent = nullptr);
    ~CompareOtbDialog();

private slots:
    void browseFile1();
    void browseFile2();
    void performComparison();

private:
    void setupUi();
    QString formatComparisonResults(const OTB::ServerItemList& list1, const QString& list1Name,
                                    const OTB::ServerItemList& list2, const QString& list2Name);

    // UI Elements
    QGroupBox* fileSelectionGroupBox;
    QLineEdit* file1LineEdit;
    QPushButton* browseFile1Button;
    QLineEdit* file2LineEdit;
    QPushButton* browseFile2Button;

    QPushButton* compareButton;

    QGroupBox* resultsGroupBox;
    QTextEdit* resultsTextEdit;

    QPushButton* closeButton;
};

#endif // COMPAREOTBDIALOG_H
