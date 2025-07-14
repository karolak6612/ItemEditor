/**
 * Item Editor Qt6 - Update OTB Settings Dialog Implementation
 *
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "UpdateOtbSettingsDialog.h"
#include "ui_UpdateOtbSettingsDialog.h"
#include <QVBoxLayout>

namespace ItemEditor {

UpdateOtbSettingsDialog::UpdateOtbSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateOtbSettingsDialog)
{
    ui->setupUi(this);
    setupUi();
}

UpdateOtbSettingsDialog::~UpdateOtbSettingsDialog()
{
    delete ui;
}

void UpdateOtbSettingsDialog::setupUi()
{
    setWindowTitle("Update Settings");
    setFixedSize(322, 184);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    m_updateSettingsGroupBox = new QGroupBox("Settings", this);
    QVBoxLayout* groupBoxLayout = new QVBoxLayout(m_updateSettingsGroupBox);
    m_reassignUnmatchedSpritesCheck = new QCheckBox("Reassign Items With Unmatched Sprites", m_updateSettingsGroupBox);
    m_reassignUnmatchedSpritesCheck->setChecked(true);
    m_reloadItemAttributesCheck = new QCheckBox("Reload Item Attributes", m_updateSettingsGroupBox);
    m_reloadItemAttributesCheck->setChecked(true);
    m_createNewItemsCheck = new QCheckBox("Create New Item(s) For Unassigned Sprites", m_updateSettingsGroupBox);
    m_createNewItemsCheck->setChecked(true);
    m_generateSignatureCheck = new QCheckBox("Generate Image Signatures (Slow)", m_updateSettingsGroupBox);
    groupBoxLayout->addWidget(m_reassignUnmatchedSpritesCheck);
    groupBoxLayout->addWidget(m_reloadItemAttributesCheck);
    groupBoxLayout->addWidget(m_createNewItemsCheck);
    groupBoxLayout->addWidget(m_generateSignatureCheck);
    mainLayout->addWidget(m_updateSettingsGroupBox);

    m_okButton = new QPushButton("OK", this);
    mainLayout->addWidget(m_okButton);

    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
}

bool UpdateOtbSettingsDialog::reassignUnmatchedSprites() const
{
    return m_reassignUnmatchedSpritesCheck->isChecked();
}

bool UpdateOtbSettingsDialog::reloadItemAttributes() const
{
    return m_reloadItemAttributesCheck->isChecked();
}

bool UpdateOtbSettingsDialog::createNewItems() const
{
    return m_createNewItemsCheck->isChecked();
}

bool UpdateOtbSettingsDialog::generateSignatures() const
{
    return m_generateSignatureCheck->isChecked();
}

} // namespace ItemEditor
