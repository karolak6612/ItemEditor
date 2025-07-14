/**
 * Item Editor Qt6 - Compare OTB Form Implementation
 * Exact mirror of Legacy_App/csharp/Source/Dialogs/CompareOtbForm.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "CompareOtbForm.h"
#include "ui_CompareOtbForm.h"
#include "../PluginInterface/OTLib/OTB/OtbReader.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

using namespace OTLib::OTB;

namespace ItemEditor {
namespace Dialogs {

CompareOtbForm::CompareOtbForm(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CompareOtbForm)
    , m_resultTextBox(nullptr)
    , m_file1Text(nullptr)
    , m_file2Text(nullptr)
    , m_browseButton1(nullptr)
    , m_browseButton2(nullptr)
    , m_compareButton(nullptr)
    , m_label1(nullptr)
    , m_label2(nullptr)
{
    ui->setupUi(this);
    setupUI();
    connectSignals();
    
    // Set window properties - exact mirror of C# form properties
    setWindowTitle(tr("Compare OTB Files"));
    setModal(true);
    setFixedSize(378, 344);
    
    // Center on parent
    if (parent) {
        move(parent->geometry().center() - rect().center());
    }
}

CompareOtbForm::~CompareOtbForm()
{
    delete ui;
}

void CompareOtbForm::setupUI()
{
    // Get references to UI elements created by the .ui file
    m_resultTextBox = ui->resultTextBox;
    m_file1Text = ui->file1Text;
    m_file2Text = ui->file2Text;
    m_browseButton1 = ui->browseButton1;
    m_browseButton2 = ui->browseButton2;
    m_compareButton = ui->compareButton;
    m_label1 = ui->label1;
    m_label2 = ui->label2;
    
    // Set initial state
    m_compareButton->setEnabled(false);
    m_resultTextBox->setReadOnly(true);
    
    // Set placeholder text
    m_file1Text->setPlaceholderText(tr("Select first OTB file..."));
    m_file2Text->setPlaceholderText(tr("Select second OTB file..."));
    m_resultTextBox->setPlaceholderText(tr("Comparison results will appear here..."));
}

void CompareOtbForm::connectSignals()
{
    // Connect signals - exact mirror of C# event handlers
    connect(m_browseButton1, &QPushButton::clicked, this, &CompareOtbForm::onBrowseFile1);
    connect(m_browseButton2, &QPushButton::clicked, this, &CompareOtbForm::onBrowseFile2);
    connect(m_compareButton, &QPushButton::clicked, this, &CompareOtbForm::onCompare);
    
    // Text change events to enable/disable compare button
    connect(m_file1Text, &QLineEdit::textChanged, this, &CompareOtbForm::onFileTextChanged);
    connect(m_file2Text, &QLineEdit::textChanged, this, &CompareOtbForm::onFileTextChanged);
}

void CompareOtbForm::onBrowseFile1()
{
    // Exact mirror of C# BrowseButton1_Click
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open OTB File"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("OTB files (*.otb);;All files (*.*)")
    );
    
    if (!fileName.isEmpty()) {
        m_file1Text->setText(fileName);
        m_file1Path = fileName;
    }
}

void CompareOtbForm::onBrowseFile2()
{
    // Exact mirror of C# BrowseButton2_Click
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open OTB File"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("OTB files (*.otb);;All files (*.*)")
    );
    
    if (!fileName.isEmpty()) {
        m_file2Text->setText(fileName);
        m_file2Path = fileName;
    }
}

void CompareOtbForm::onCompare()
{
    // Exact mirror of C# CompareButton_Click
    m_resultTextBox->clear();
    
    if (!compareItems()) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to compare OTB files."));
    }
}

void CompareOtbForm::onFileTextChanged()
{
    // Exact mirror of C# FileText_TextChanged
    bool bothFilesSelected = !m_file1Text->text().isEmpty() && !m_file2Text->text().isEmpty();
    m_compareButton->setEnabled(bothFilesSelected);
    
    // Update file paths
    m_file1Path = m_file1Text->text();
    m_file2Path = m_file2Text->text();
}

bool CompareOtbForm::compareItems()
{
    // Exact mirror of C# CompareItems method
    
    // Create readers for both files
    OtbReader reader1;
    if (!reader1.read(m_file1Path)) {
        QMessageBox::warning(this, tr("Error"), tr("Could not open %1.").arg(m_file1Path));
        return false;
    }
    
    OtbReader reader2;
    if (!reader2.read(m_file2Path)) {
        QMessageBox::warning(this, tr("Error"), tr("Could not open %1.").arg(m_file2Path));
        return false;
    }
    
    // Compare item counts
    if (reader1.count() != reader2.count()) {
        m_resultTextBox->append(tr("Item count: [ %1 / %2 ]")
                               .arg(reader1.count())
                               .arg(reader2.count()));
    }
    
    // Get item lists
    auto items1 = reader1.items();
    auto items2 = reader2.items();
    
    // Compare items
    int maxItems = qMin(items1->count(), items2->count());
    
    for (int i = 0; i < maxItems; ++i) {
        auto item1 = items1->items().value(i);
        auto item2 = items2->items().value(i);
        
        if (!item1 || !item2) {
            continue;
        }
        
        // Compare client IDs (sprite changes)
        if (item1->clientId() != item2->clientId()) {
            m_resultTextBox->append(tr("ID: %1 - Sprite changed - [ %2 / %3 ]")
                                   .arg(item1->id())
                                   .arg(item1->clientId())
                                   .arg(item2->clientId()));
            continue;
        }
        
        // Compare sprite hashes
        if (!item1->spriteHash().isEmpty() && !item2->spriteHash().isEmpty() &&
            item1->spriteHash() != item2->spriteHash()) {
            m_resultTextBox->append(tr("ID: %1 - Sprite updated.")
                                   .arg(item1->id()));
        }
        
        // Compare properties using ServerItem comparison
        if (!item1->equals(item2)) {
            m_resultTextBox->append(tr("ID: %1 - Properties differ")
                                   .arg(item1->id()));
        }
    }
    
    // Check if no differences were found
    if (m_resultTextBox->toPlainText().isEmpty()) {
        QMessageBox::information(this, tr("Comparison Complete"), tr("No differences found!"));
    }
    
    return true;
}

} // namespace Dialogs
} // namespace ItemEditor