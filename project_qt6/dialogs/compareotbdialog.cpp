#include "compareotbdialog.h"
#include "otb/otbreader.h"
#include "otb/otbtypes.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QMap>
#include <QSet>
#include <algorithm>

CompareOtbDialog::CompareOtbDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Compare OTB Files"));
    setMinimumSize(700, 500); // Increased size for better results display
    setupUi();

    compareButton->setEnabled(false);
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
    resultsTextEdit->setLineWrapMode(QTextEdit::NoWrap);
    resultsLayout->addWidget(resultsTextEdit);
    mainLayout->addWidget(resultsGroupBox);

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
    resultsTextEdit->setHtml(results);
}

QString CompareOtbDialog::formatComparisonResults(
    const OTB::ServerItemList& list1, const QString& list1Name,
    const OTB::ServerItemList& list2, const QString& list2Name)
{
    QString htmlResult = "<h1>OTB Comparison Results</h1>";
    htmlResult += QString("<h2>Comparing: <font color='blue'>%1</font> vs <font color='green'>%2</font></h2>").arg(list1Name, list2Name);

    htmlResult += "<h3>Version Info:</h3>";
    htmlResult += "<table border='1' cellspacing='0' cellpadding='3' width='100%'>";
    htmlResult += "<tr><th>Property</th><th>" + list1Name + "</th><th>" + list2Name + "</th></tr>";
    auto addRow = [&](const QString& prop, const QString& val1, const QString& val2) {
        htmlResult += "<tr><td><b>" + prop + "</b></td>";
        if (val1 == val2) {
            htmlResult += "<td>" + val1 + "</td><td>" + val2 + "</td>";
        } else {
            htmlResult += "<td><font color='red'>" + val1 + "</font></td><td><font color='red'>" + val2 + "</font></td>";
        }
        htmlResult += "</tr>";
    };
    addRow("OTB Major Version", QString::number(list1.majorVersion), QString::number(list2.majorVersion));
    addRow("OTB Minor Version", QString::number(list1.minorVersion), QString::number(list2.minorVersion));
    addRow("OTB Build Number", QString::number(list1.buildNumber), QString::number(list2.buildNumber));
    addRow("Internal Client Version", QString::number(list1.clientVersion), QString::number(list2.clientVersion));
    addRow("Description", list1.description, list2.description);
    htmlResult += "</table>";

    htmlResult += "<h3>Item Differences:</h3>";

    QMap<quint16, const OTB::ServerItem*> map1, map2;
    for (const auto& item : list1.items) map1.insert(item.id, &item);
    for (const auto& item : list2.items) map2.insert(item.id, &item);

    QList<quint16> allKeys = QSet<quint16>::fromList(map1.keys()).unite(QSet<quint16>::fromList(map2.keys())).values();
    std::sort(allKeys.begin(), allKeys.end());

    QString diffs;
    for (quint16 id : allKeys) {
        const OTB::ServerItem* item1 = map1.value(id, nullptr);
        const OTB::ServerItem* item2 = map2.value(id, nullptr);
        QString itemDiff;

        if (item1 && !item2) {
            itemDiff = QString("<tr><td>%1</td><td><font color='blue'>Present</font></td><td><font color='red'>Missing</font></td><td>-</td></tr>")
                       .arg(id);
        } else if (!item1 && item2) {
            itemDiff = QString("<tr><td>%1</td><td><font color='red'>Missing</font></td><td><font color='green'>Present</font></td><td>-</td></tr>")
                       .arg(id);
        } else if (item1 && item2) {
            // Compare items field by field
            OTB::ServerItem mutableItem1 = *item1;
            OTB::ServerItem mutableItem2 = *item2;
            mutableItem1.updatePropertiesFromFlags(); // Ensure booleans are synced
            mutableItem2.updatePropertiesFromFlags();

            QString differences;
            auto check = [&](const QString& prop, const auto& val1, const auto& val2) {
                if (val1 != val2) {
                    differences += QString("<li><b>%1:</b> '%2' vs '%3'</li>").arg(prop, QString::number(val1), QString::number(val2));
                }
            };
             auto checkStr = [&](const QString& prop, const QString& val1, const QString& val2) {
                if (val1 != val2) {
                    differences += QString("<li><b>%1:</b> \"%2\" vs \"%3\"</li>").arg(prop, val1, val2);
                }
            };
            auto checkHex = [&](const QString& prop, const QByteArray& val1, const QByteArray& val2) {
                if (val1 != val2) {
                    differences += QString("<li><b>%1:</b> %2 vs %3</li>").arg(prop, QString(val1.toHex()), QString(val2.toHex()));
                }
            };

            checkStr("Name", mutableItem1.name, mutableItem2.name);
            check("Client ID", mutableItem1.clientId, mutableItem2.clientId);
            check("Type", static_cast<int>(mutableItem1.type), static_cast<int>(mutableItem2.type));
            check("Ground Speed", mutableItem1.groundSpeed, mutableItem2.groundSpeed);
            check("Minimap Color", mutableItem1.minimapColor, mutableItem2.minimapColor);
            check("Light Level", mutableItem1.lightLevel, mutableItem2.lightLevel);
            check("Light Color", mutableItem1.lightColor, mutableItem2.lightColor);
            check("Trade As", mutableItem1.tradeAs, mutableItem2.tradeAs);
            check("Flags", mutableItem1.flags, mutableItem2.flags);
            checkHex("Sprite Hash", mutableItem1.spriteHash, mutableItem2.spriteHash);

            if (!differences.isEmpty()) {
                itemDiff = QString("<tr><td>%1</td><td>Same</td><td>Same</td><td><ul>%2</ul></td></tr>")
                           .arg(id, differences);
            }
        }
        diffs += itemDiff;
    }

    if (diffs.isEmpty()) {
        htmlResult += "<p>No differences found in items present in both files.</p>";
    } else {
        htmlResult += "<table border='1' cellspacing='0' cellpadding='3' width='100%'>";
        htmlResult += "<tr><th>Server ID</th><th>In " + list1Name + "</th><th>In " + list2Name + "</th><th>Details</th></tr>";
        htmlResult += diffs;
        htmlResult += "</table>";
    }

    return htmlResult;
}
