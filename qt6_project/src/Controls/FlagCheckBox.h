/**
 * Item Editor Qt6 - Flag CheckBox Header
 * Exact mirror of Legacy_App/csharp/Source/Controls/FlagCheckBox.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#ifndef ITEMEDITOR_FLAGCHECKBOX_H
#define ITEMEDITOR_FLAGCHECKBOX_H

#include <QCheckBox>
#include <QString>
#include <QFontMetrics>
#include <QSize>
#include <QPaintEvent>
#include "OTLib/Server/Items/ServerItemFlag.h"

namespace ItemEditor {

/**
 * Flag CheckBox Control
 * Exact mirror of C# FlagCheckBox : DarkCheckBox
 * 
 * Custom checkbox control for editing server item flags
 * Inherits from QCheckBox and adds ServerItemFlag binding
 */
class FlagCheckBox : public QCheckBox
{
    Q_OBJECT
    Q_PROPERTY(OTLib::Server::Items::ServerItemFlag serverItemFlag READ serverItemFlag WRITE setServerItemFlag NOTIFY serverItemFlagChanged)

public:
    explicit FlagCheckBox(QWidget *parent = nullptr);
    explicit FlagCheckBox(const QString &text, QWidget *parent = nullptr);
    explicit FlagCheckBox(OTLib::Server::Items::ServerItemFlag flag, QWidget *parent = nullptr);
    explicit FlagCheckBox(OTLib::Server::Items::ServerItemFlag flag, const QString &text, QWidget *parent = nullptr);
    virtual ~FlagCheckBox() = default;

    // Properties - exact mirror of C# ServerItemFlag property
    OTLib::Server::Items::ServerItemFlag serverItemFlag() const { return m_serverItemFlag; }
    void setServerItemFlag(OTLib::Server::Items::ServerItemFlag flag);

    // Convenience methods
    QString flagName() const;
    static QString flagToString(OTLib::Server::Items::ServerItemFlag flag);
    
    // Performance optimizations
    QSize sizeHint() const override;

signals:
    void serverItemFlagChanged(OTLib::Server::Items::ServerItemFlag flag);

protected:
    // Override state change to emit flag change signal
    void nextCheckState() override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onStateChanged(int state);

private:
    // Private properties - exact mirror of C# private fields
    OTLib::Server::Items::ServerItemFlag m_serverItemFlag;
    
    // Performance optimization members
    mutable QFontMetrics m_fontMetrics;
    mutable QSize m_cachedSizeHint;
    QString m_cachedText;

    // Helper methods
    void initializeControl();
    void updateText();
};

} // namespace ItemEditor

#endif // ITEMEDITOR_FLAGCHECKBOX_H