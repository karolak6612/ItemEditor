/**
 * Item Editor Qt6 - Update OTB Settings Dialog Header
 *
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_UPDATEOTBSETTINGSDIALOG_H
#define ITEMEDITOR_UPDATEOTBSETTINGSDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QGroupBox>

QT_BEGIN_NAMESPACE
namespace Ui { class UpdateOtbSettingsDialog; }
QT_END_NAMESPACE

namespace ItemEditor {

class UpdateOtbSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateOtbSettingsDialog(QWidget *parent = nullptr);
    ~UpdateOtbSettingsDialog();

    bool reassignUnmatchedSprites() const;
    bool reloadItemAttributes() const;
    bool createNewItems() const;
    bool generateSignatures() const;

private:
    void setupUi();

    Ui::UpdateOtbSettingsDialog *ui;

    QGroupBox* m_updateSettingsGroupBox;
    QCheckBox* m_reassignUnmatchedSpritesCheck;
    QCheckBox* m_reloadItemAttributesCheck;
    QCheckBox* m_createNewItemsCheck;
    QCheckBox* m_generateSignatureCheck;
    QPushButton* m_okButton;
};

} // namespace ItemEditor

#endif // ITEMEDITOR_UPDATEOTBSETTINGSDIALOG_H
