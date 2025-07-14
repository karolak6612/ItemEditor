/**
 * Item Editor Qt6 - Compare OTB Dialog Implementation
 *
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "CompareOtbDialog.h"
#include "ui_CompareOtbDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>

namespace ItemEditor {

CompareOtbDialog::CompareOtbDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CompareOtbDialog)
{
    ui->setupUi(this);
    setupUi();
}

CompareOtbDialog::~CompareOtbDialog()
{
    delete ui;
}

void CompareOtbDialog::setupUi()
{
    setWindowTitle("Compare OTB Files");
    setFixedSize(378, 344);

    QGridLayout* mainLayout = new QGridLayout(this);

    m_file1Text = new QLineEdit(this);
    m_browse1Button = new QPushButton("Browse...", this);
    mainLayout->addWidget(new QLabel("File 1:", this), 0, 0);
    mainLayout->addWidget(m_file1Text, 0, 1);
    mainLayout->addWidget(m_browse1Button, 0, 2);

    m_file2Text = new QLineEdit(this);
    m_browse2Button = new QPushButton("Browse...", this);
    mainLayout->addWidget(new QLabel("File 2:", this), 1, 0);
    mainLayout->addWidget(m_file2Text, 1, 1);
    mainLayout->addWidget(m_browse2Button, 1, 2);

    m_compareButton = new QPushButton("Compare", this);
    mainLayout->addWidget(m_compareButton, 2, 1);

    m_resultText = new QTextEdit(this);
    m_resultText->setReadOnly(true);
    mainLayout->addWidget(m_resultText, 3, 0, 1, 3);

    connect(m_browse1Button, &QPushButton::clicked, this, &CompareOtbDialog::onBrowse1);
    connect(m_browse2Button, &QPushButton::clicked, this, &CompareOtbDialog::onBrowse2);
    connect(m_compareButton, &QPushButton::clicked, this, &CompareOtbDialog::onCompare);
    connect(m_file1Text, &QLineEdit::textChanged, this, &CompareOtbDialog::onTextChanged);
    connect(m_file2Text, &QLineEdit::textChanged, this, &CompareOtbDialog::onTextChanged);

    onTextChanged();
}

void CompareOtbDialog::onBrowse1()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open OTB File", "", "OTB Files (*.otb)");
    if (!fileName.isEmpty()) {
        m_file1Text->setText(fileName);
    }
}

void CompareOtbDialog::onBrowse2()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open OTB File", "", "OTB Files (*.otb)");
    if (!fileName.isEmpty()) {
        m_file2Text->setText(fileName);
    }
}

void CompareOtbDialog::onCompare()
{
    m_resultText->clear();
    compareItems();
}

void CompareOtbDialog::onTextChanged()
{
    m_compareButton->setEnabled(!m_file1Text->text().isEmpty() && !m_file2Text->text().isEmpty());
}

bool CompareOtbDialog::compareItems()
{
    OTLib::OTB::OtbReader reader1;
    if (!reader1.read(m_file1Text->text())) {
        QMessageBox::critical(this, "Error", "Could not open file 1.");
        return false;
    }

    OTLib::OTB::OtbReader reader2;
    if (!reader2.read(m_file2Text->text())) {
        QMessageBox::critical(this, "Error", "Could not open file 2.");
        return false;
    }

    if (reader1.items()->count() != reader2.items()->count()) {
        m_resultText->append(QString("Item count mismatch: %1 vs %2").arg(reader1.items()->count()).arg(reader2.items()->count()));
    }

    auto it1 = reader1.items()->begin();
    auto it2 = reader2.items()->begin();

    while (it1 != reader1.items()->end() && it2 != reader2.items()->end()) {
        OTLib::Server::Items::ServerItem* item1 = *it1;
        OTLib::Server::Items::ServerItem* item2 = *it2;

        if (item1->id() != item2->id()) {
            m_resultText->append(QString("ID mismatch: %1 vs %2").arg(item1->id()).arg(item2->id()));
        } else {
            if (item1->clientId() != item2->clientId()) {
                m_resultText->append(QString("ID: %1 - Client ID mismatch: %2 vs %3").arg(item1->id()).arg(item1->clientId()).arg(item2->clientId()));
            }
            if (item1->name() != item2->name()) {
                m_resultText->append(QString("ID: %1 - Name mismatch: %2 vs %3").arg(item1->id()).arg(item1->name()).arg(item2->name()));
            }
            // Add more comparisons here...
        }

        ++it1;
        ++it2;
    }

    if (m_resultText->toPlainText().isEmpty()) {
        QMessageBox::information(this, "Compare OTB", "Files are identical.");
    }

    return true;
}

} // namespace ItemEditor
