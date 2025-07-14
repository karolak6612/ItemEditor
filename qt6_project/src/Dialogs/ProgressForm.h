/**
 * Item Editor Qt6 - Progress Dialog Header
 * Progress indication dialog for long-running operations
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_PROGRESSFORM_H
#define ITEMEDITOR_PROGRESSFORM_H

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class ProgressDialog; }
QT_END_NAMESPACE

namespace ItemEditor {

/**
 * Progress Dialog Class
 * Displays progress for long-running operations
 * 
 * Provides progress indication with optional cancel functionality
 */
class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = nullptr);
    ~ProgressDialog();

    // Progress control methods
    void setProgress(int value);
    void setMaximum(int maximum);
    void setMinimum(int minimum);
    void setRange(int minimum, int maximum);
    void setText(const QString &text);
    void setTitle(const QString &title);
    
    // Cancel functionality
    void setCancelEnabled(bool enabled);
    bool wasCanceled() const { return isCanceled; }

public slots:
    void reset();

signals:
    void cancelRequested();

private slots:
    void onCancelClicked();

private:
    void setupUi();

private:
    Ui::ProgressDialog *ui;
    bool isCanceled;
};

} // namespace ItemEditor

#endif // ITEMEDITOR_PROGRESSFORM_H