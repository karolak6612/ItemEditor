#include "compareotbdialog.h"
#include "otb/otbreader.h"   // For OtbReader
#include "otb/otbtypes.h"  // For ServerItemList and ServerItem

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QFileDialog>
#include <QFileInfo> // To get file names for display
#include <QMessageBox>
#include <QMap>

CompareOtbDialog::CompareOtbDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Compare OTB Files"));
    setMinimumSize(600, 400);
    setupUi();

    compareButton->setEnabled(false); // Enabled when both files are selected
    connect(file1LineEdit, &QLineEdit::textChanged, this, [this](){ compareButton->setEnabled(!file1LineEdit->text().isEmpty() && !file2LineEdit->text().isEmpty()); });
    connect(file2LineEdit, &QLineEdit::textChanged, this, [this](){ compareButton->setEnabled(!file1LineEdit->text().isEmpty() && !file2LineEdit->text().isEmpty()); });
}

CompareOtbDialog::~CompareOtbDialog()
{
}

void CompareOtbDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // File Selection Group
    fileSelectionGroupBox = new QGroupBox(tr("Select OTB Files to Compare"), this);
    QGridLayout* fileSelectionLayout = new QGridLayout(fileSelectionGroupBox);

    file1LineEdit = new QLineEdit(this);
    file1LineEdit->setPlaceholderText(tr("Path to first OTB file"));
    browseFile1Button = new QPushButton(tr("Browse..."), this);
    connect(browseFile1Button, &QPushButton::clicked, this, &CompareOtbDialog::browseFile1);
    fileSelectionLayout->addWidget(new QLabel(tr("OTB File 1:"), this), 0, 0);
    fileSelectionLayout->addWidget(file1LineEdit, 0, 1);
    fileSelectionLayout->addWidget(browseFile1Button, 0, 2);

    file2LineEdit = new QLineEdit(this);
    file2LineEdit->setPlaceholderText(tr("Path to second OTB file"));
    browseFile2Button = new QPushButton(tr("Browse..."), this);
    connect(browseFile2Button, &QPushButton::clicked, this, &CompareOtbDialog::browseFile2);
    fileSelectionLayout->addWidget(new QLabel(tr("OTB File 2:"), this), 1, 0);
    fileSelectionLayout->addWidget(file2LineEdit, 1, 1);
    fileSelectionLayout->addWidget(browseFile2Button, 1, 2);

    mainLayout->addWidget(fileSelectionGroupBox);

    // Compare Button
    compareButton = new QPushButton(tr("Compare"), this);
    connect(compareButton, &QPushButton::clicked, this, &CompareOtbDialog::performComparison);
    QHBoxLayout* compareButtonLayout = new QHBoxLayout();
    compareButtonLayout->addStretch();
    compareButtonLayout->addWidget(compareButton);
    compareButtonLayout->addStretch();
    mainLayout->addLayout(compareButtonLayout);


    // Results Group
    resultsGroupBox = new QGroupBox(tr("Comparison Results"), this);
    QVBoxLayout* resultsLayout = new QVBoxLayout(resultsGroupBox);
    resultsTextEdit = new QTextEdit(this);
    resultsTextEdit->setReadOnly(true);
    resultsTextEdit->setLineWrapMode(QTextEdit::NoWrap); // Good for structured text
    resultsLayout->addWidget(resultsTextEdit);
    mainLayout->addWidget(resultsGroupBox);
    resultsGroupBox->setMinimumHeight(200);


    // Close Button
    closeButton = new QPushButton(tr("Close"), this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    QHBoxLayout* bottomButtonLayout = new QHBoxLayout();
    bottomButtonLayout->addStretch();
    bottomButtonLayout->addWidget(closeButton);
    mainLayout->addLayout(bottomButtonLayout);

    setLayout(mainLayout);
}

void CompareOtbDialog::browseFile1()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select OTB File 1"),
                                                    file1LineEdit->text().isEmpty() ? QDir::homePath() : QFileInfo(file1LineEdit->text()).path(),
                                                    tr("OTB Files (*.otb);;All Files (*)"));
    if (!fileName.isEmpty()) {
        file1LineEdit->setText(fileName);
    }
}

void CompareOtbDialog::browseFile2()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select OTB File 2"),
                                                    file2LineEdit->text().isEmpty() ? QDir::homePath() : QFileInfo(file2LineEdit->text()).path(),
                                                    tr("OTB Files (*.otb);;All Files (*)"));
    if (!fileName.isEmpty()) {
        file2LineEdit->setText(fileName);
    }
}

void CompareOtbDialog::performComparison()
{
    resultsTextEdit->clear();
    QString path1 = file1LineEdit->text();
    QString path2 = file2LineEdit->text();

    if (path1.isEmpty() || path2.isEmpty()) {
        QMessageBox::warning(this, tr("Files Not Selected"), tr("Please select two OTB files to compare."));
        return;
    }
    if (path1 == path2) {
        QMessageBox::information(this, tr("Same File"), tr("The selected files are identical."));
        resultsTextEdit->setText(tr("Selected files are the same."));
        return;
    }

    OTB::OtbReader reader1, reader2;
    OTB::ServerItemList list1, list2;
    QString errorStr1, errorStr2;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool success1 = reader1.read(path1, list1, errorStr1);
    bool success2 = reader2.read(path2, list2, errorStr2);
    QApplication::restoreOverrideCursor();

    if (!success1) {
        QMessageBox::critical(this, tr("Error Loading File 1"), tr("Could not load OTB file:\n%1\n\nError: %2").arg(path1, errorStr1));
        return;
    }
    if (!success2) {
        QMessageBox::critical(this, tr("Error Loading File 2"), tr("Could not load OTB file:\n%1\n\nError: %2").arg(path2, errorStr2));
        return;
    }

    QString results = formatComparisonResults(list1, QFileInfo(path1).fileName(), list2, QFileInfo(path2).fileName());
    resultsTextEdit->setHtml(results); // Use setHtml for rich text formatting
}

// Stub for formatting results, will be complex.
QString CompareOtbDialog::formatComparisonResults(
    const OTB::ServerItemList& list1, const QString& list1Name,
    const OTB::ServerItemList& list2, const QString& list2Name)
{
    // This is a placeholder. A detailed comparison is complex.
    // It needs to:
    // 1. Map items by Server ID from both lists.
    // 2. Iterate and find items only in list1, only in list2.
    // 3. For common items, compare each property using ServerItem::equals or field-by-field.
    //    The ServerItem::equals method needs to be robust.
    // For now, a very basic comparison:
    QString htmlResult = "<h1>OTB Comparison Results</h1>";
    htmlResult += QString("<h2>Comparing: <font color='blue'>%1</font> vs <font color='green'>%2</font></h2>").arg(list1Name, list2Name);

    htmlResult += QString("<p><b>%1 items in %3:</b> %4</p>").arg(list1Name).arg(list1.items.count()).arg(list1Name).arg(list1.items.count());
    htmlResult += QString("<p><b>%1 items in %3:</b> %4</p>").arg(list2Name).arg(list2.items.count()).arg(list2Name).arg(list2.items.count());

    if (list1.items.count() != list2.items.count()) {
         htmlResult += "<p><font color='red'>Item counts differ.</font></p>";
    } else {
         htmlResult += "<p>Item counts are the same.</p>";
    }

    // Simple version info comparison
    htmlResult += "<h3>Version Info:</h3>";
    htmlResult += "<table border='1' cellspacing='0' cellpadding='3'>";
    htmlResult += "<tr><th>Property</th><th>" + list1Name + "</th><th>" + list2Name + "</th></tr>";
    auto addRow = [&](const QString& prop, const QString& val1, const QString& val2) {
        htmlResult += "<tr><td>" + prop + "</td>";
        if (val1 == val2) {
            htmlResult += "<td>" + val1 + "</td><td>" + val2 + "</td>";
        } else {
            htmlResult += "<td><font color='red'>" + val1 + "</font></td><td><font color='red'>" + val2 + "</font></td>";
        }
        htmlResult += "</tr>";
    };
    addRow("OTB Major Version", QString::number(list1.majorVersion), QString::number(list2.majorVersion));
    addRow("OTB Minor Version (Client Target)", QString::number(list1.minorVersion), QString::number(list2.minorVersion));
    addRow("OTB Build Number", QString::number(list1.buildNumber), QString::number(list2.buildNumber));
    addRow("OTB Description", list1.description, list2.description);
    addRow("Internal Client Version", QString::number(list1.clientVersion), QString::number(list2.clientVersion));
    htmlResult += "</table>";

    htmlResult += "<p><i>Detailed item-by-item comparison not yet implemented.</i></p>";

    return htmlResult;
}
