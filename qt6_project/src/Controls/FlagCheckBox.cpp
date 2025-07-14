/**
 * Item Editor Qt6 - Flag CheckBox Implementation
 * Exact mirror of Legacy_App/csharp/Source/Controls/FlagCheckBox.cs
 * 
 * Copyright © 2014-2019 OTTools <https://github.com/ottools/ItemEditor/>
 * Licensed under MIT License
 */

#include "FlagCheckBox.h"
#include <QMap>

namespace ItemEditor {

// Static flag name mapping for display purposes
static const QMap<OTLib::Server::Items::ServerItemFlag, QString> flagNames = {
    {OTLib::Server::Items::ServerItemFlag::None, "None"},
    {OTLib::Server::Items::ServerItemFlag::Unpassable, "Unpassable"},
    {OTLib::Server::Items::ServerItemFlag::BlockMissiles, "Block Missiles"},
    {OTLib::Server::Items::ServerItemFlag::BlockPathfinder, "Block Pathfinder"},
    {OTLib::Server::Items::ServerItemFlag::HasElevation, "Has Elevation"},
    {OTLib::Server::Items::ServerItemFlag::MultiUse, "Multi Use"},
    {OTLib::Server::Items::ServerItemFlag::Pickupable, "Pickupable"},
    {OTLib::Server::Items::ServerItemFlag::Movable, "Movable"},
    {OTLib::Server::Items::ServerItemFlag::Stackable, "Stackable"},
    {OTLib::Server::Items::ServerItemFlag::FloorChangeDown, "Floor Change Down"},
    {OTLib::Server::Items::ServerItemFlag::FloorChangeNorth, "Floor Change North"},
    {OTLib::Server::Items::ServerItemFlag::FloorChangeEast, "Floor Change East"},
    {OTLib::Server::Items::ServerItemFlag::FloorChangeSouth, "Floor Change South"},
    {OTLib::Server::Items::ServerItemFlag::FloorChangeWest, "Floor Change West"},
    {OTLib::Server::Items::ServerItemFlag::StackOrder, "Stack Order"},
    {OTLib::Server::Items::ServerItemFlag::Readable, "Readable"},
    {OTLib::Server::Items::ServerItemFlag::Rotatable, "Rotatable"},
    {OTLib::Server::Items::ServerItemFlag::Hangable, "Hangable"},
    {OTLib::Server::Items::ServerItemFlag::HookSouth, "Hook South"},
    {OTLib::Server::Items::ServerItemFlag::HookEast, "Hook East"},
    {OTLib::Server::Items::ServerItemFlag::CanNotDecay, "Cannot Decay"},
    {OTLib::Server::Items::ServerItemFlag::AllowDistanceRead, "Allow Distance Read"},
    {OTLib::Server::Items::ServerItemFlag::Unused, "Unused"},
    {OTLib::Server::Items::ServerItemFlag::ClientCharges, "Client Charges"},
    {OTLib::Server::Items::ServerItemFlag::IgnoreLook, "Ignore Look"},
    {OTLib::Server::Items::ServerItemFlag::IsAnimation, "Is Animation"},
    {OTLib::Server::Items::ServerItemFlag::FullGround, "Full Ground"},
    {OTLib::Server::Items::ServerItemFlag::ForceUse, "Force Use"}
};

FlagCheckBox::FlagCheckBox(QWidget *parent)
    : QCheckBox(parent)
    , m_serverItemFlag(OTLib::Server::Items::ServerItemFlag::None)
    , m_fontMetrics(font())
{
    initializeControl();
}

FlagCheckBox::FlagCheckBox(const QString &text, QWidget *parent)
    : QCheckBox(text, parent)
    , m_serverItemFlag(OTLib::Server::Items::ServerItemFlag::None)
    , m_fontMetrics(font())
{
    initializeControl();
}

FlagCheckBox::FlagCheckBox(OTLib::Server::Items::ServerItemFlag flag, QWidget *parent)
    : QCheckBox(parent)
    , m_serverItemFlag(flag)
    , m_fontMetrics(font())
{
    initializeControl();
    updateText();
}

FlagCheckBox::FlagCheckBox(OTLib::Server::Items::ServerItemFlag flag, const QString &text, QWidget *parent)
    : QCheckBox(text, parent)
    , m_serverItemFlag(flag)
    , m_fontMetrics(font())
{
    initializeControl();
}

void FlagCheckBox::setServerItemFlag(OTLib::Server::Items::ServerItemFlag flag)
{
    if (m_serverItemFlag != flag) {
        m_serverItemFlag = flag;
        updateText();
        emit serverItemFlagChanged(flag);
    }
}

QString FlagCheckBox::flagName() const
{
    return flagToString(m_serverItemFlag);
}

QString FlagCheckBox::flagToString(OTLib::Server::Items::ServerItemFlag flag)
{
    return flagNames.value(flag, "Unknown");
}

void FlagCheckBox::nextCheckState()
{
    QCheckBox::nextCheckState();
    emit serverItemFlagChanged(m_serverItemFlag);
}

void FlagCheckBox::onStateChanged(int state)
{
    Q_UNUSED(state)
    emit serverItemFlagChanged(m_serverItemFlag);
}

void FlagCheckBox::initializeControl()
{
    // Connect state change signal to our custom handler
    connect(this, QOverload<int>::of(&QCheckBox::stateChanged),
            this, &FlagCheckBox::onStateChanged);
    
    // Set default properties similar to DarkCheckBox with performance optimizations
    setTristate(false);
    
    // Performance optimizations
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_StaticContents, true);
    
    // Set up efficient update handling
    setUpdatesEnabled(true);
}

void FlagCheckBox::updateText()
{
    if (text().isEmpty()) {
        QString newText = flagName();
        if (newText != m_cachedText) {
            m_cachedText = newText;
            setText(newText);
        }
    }
}

void FlagCheckBox::paintEvent(QPaintEvent *event)
{
    // Use optimized painting for better performance
    QCheckBox::paintEvent(event);
}

QSize FlagCheckBox::sizeHint() const
{
    // Cache size hint calculation for performance
    if (m_cachedSizeHint.isValid()) {
        return m_cachedSizeHint;
    }
    
    QSize hint = QCheckBox::sizeHint();
    
    // Adjust for flag text if needed
    if (!text().isEmpty()) {
        QRect textRect = m_fontMetrics.boundingRect(text());
        hint.setWidth(qMax(hint.width(), textRect.width() + 25)); // 25px for checkbox
    }
    
    const_cast<FlagCheckBox*>(this)->m_cachedSizeHint = hint;
    return hint;
}

} // namespace ItemEditor