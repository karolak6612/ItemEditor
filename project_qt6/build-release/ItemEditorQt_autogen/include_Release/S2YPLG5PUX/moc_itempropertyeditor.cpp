/****************************************************************************
** Meta object code from reading C++ file 'itempropertyeditor.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../include/ui/widgets/itempropertyeditor.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'itempropertyeditor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN2UI7Widgets18ItemPropertyEditorE_t {};
} // unnamed namespace

template <> constexpr inline auto UI::Widgets::ItemPropertyEditor::qt_create_metaobjectdata<qt_meta_tag_ZN2UI7Widgets18ItemPropertyEditorE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "UI::Widgets::ItemPropertyEditor",
        "itemPropertyChanged",
        "",
        "clientIdChanged",
        "newId",
        "itemNameChanged",
        "newName",
        "itemTypeChanged",
        "OTB::ServerItemType",
        "newType",
        "stackOrderChanged",
        "OTB::TileStackOrder",
        "newOrder",
        "unpassableChanged",
        "value",
        "blockMissilesChanged",
        "blockPathfinderChanged",
        "hasElevationChanged",
        "forceUseChanged",
        "multiUseChanged",
        "pickupableChanged",
        "movableChanged",
        "stackableChanged",
        "readableChanged",
        "rotatableChanged",
        "hangableChanged",
        "hookSouthChanged",
        "hookEastChanged",
        "ignoreLookChanged",
        "fullGroundChanged",
        "groundSpeedChanged",
        "lightLevelChanged",
        "lightColorChanged",
        "minimapColorChanged",
        "maxReadCharsChanged",
        "maxReadWriteCharsChanged",
        "tradeAsChanged",
        "onClientIdChanged",
        "onNameChanged",
        "text",
        "onTypeChanged",
        "index",
        "onStackOrderChanged",
        "onFlagChanged",
        "onAttributeChanged"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'itemPropertyChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'clientIdChanged'
        QtMocHelpers::SignalData<void(quint16)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::UShort, 4 },
        }}),
        // Signal 'itemNameChanged'
        QtMocHelpers::SignalData<void(const QString &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 },
        }}),
        // Signal 'itemTypeChanged'
        QtMocHelpers::SignalData<void(OTB::ServerItemType)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 9 },
        }}),
        // Signal 'stackOrderChanged'
        QtMocHelpers::SignalData<void(OTB::TileStackOrder)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Signal 'unpassableChanged'
        QtMocHelpers::SignalData<void(bool)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 14 },
        }}),
        // Signal 'blockMissilesChanged'
        QtMocHelpers::SignalData<void(bool)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 14 },
        }}),
        // Signal 'blockPathfinderChanged'
        QtMocHelpers::SignalData<void(bool)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 14 },
        }}),
        // Signal 'hasElevationChanged'
        QtMocHelpers::SignalData<void(bool)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 14 },
        }}),
        // Signal 'forceUseChanged'
        QtMocHelpers::SignalData<void(bool)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 14 },
        }}),
        // Signal 'multiUseChanged'
        QtMocHelpers::SignalData<void(bool)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 14 },
        }}),
        // Signal 'pickupableChanged'
        QtMocHelpers::SignalData<void(bool)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 14 },
        }}),
        // Signal 'movableChanged'
        QtMocHelpers::SignalData<void(bool)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 14 },
        }}),
        // Signal 'stackableChanged'
        QtMocHelpers::SignalData<void(bool)>(22, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 14 },
        }}),
        // Signal 'readableChanged'
        QtMocHelpers::SignalData<void(bool)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 14 },
        }}),
        // Signal 'rotatableChanged'
        QtMocHelpers::SignalData<void(bool)>(24, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 14 },
        }}),
        // Signal 'hangableChanged'
        QtMocHelpers::SignalData<void(bool)>(25, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 14 },
        }}),
        // Signal 'hookSouthChanged'
        QtMocHelpers::SignalData<void(bool)>(26, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 14 },
        }}),
        // Signal 'hookEastChanged'
        QtMocHelpers::SignalData<void(bool)>(27, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 14 },
        }}),
        // Signal 'ignoreLookChanged'
        QtMocHelpers::SignalData<void(bool)>(28, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 14 },
        }}),
        // Signal 'fullGroundChanged'
        QtMocHelpers::SignalData<void(bool)>(29, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 14 },
        }}),
        // Signal 'groundSpeedChanged'
        QtMocHelpers::SignalData<void(quint16)>(30, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::UShort, 14 },
        }}),
        // Signal 'lightLevelChanged'
        QtMocHelpers::SignalData<void(quint16)>(31, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::UShort, 14 },
        }}),
        // Signal 'lightColorChanged'
        QtMocHelpers::SignalData<void(quint16)>(32, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::UShort, 14 },
        }}),
        // Signal 'minimapColorChanged'
        QtMocHelpers::SignalData<void(quint16)>(33, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::UShort, 14 },
        }}),
        // Signal 'maxReadCharsChanged'
        QtMocHelpers::SignalData<void(quint16)>(34, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::UShort, 14 },
        }}),
        // Signal 'maxReadWriteCharsChanged'
        QtMocHelpers::SignalData<void(quint16)>(35, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::UShort, 14 },
        }}),
        // Signal 'tradeAsChanged'
        QtMocHelpers::SignalData<void(quint16)>(36, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::UShort, 14 },
        }}),
        // Slot 'onClientIdChanged'
        QtMocHelpers::SlotData<void(int)>(37, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 14 },
        }}),
        // Slot 'onNameChanged'
        QtMocHelpers::SlotData<void(const QString &)>(38, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 39 },
        }}),
        // Slot 'onTypeChanged'
        QtMocHelpers::SlotData<void(int)>(40, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 41 },
        }}),
        // Slot 'onStackOrderChanged'
        QtMocHelpers::SlotData<void(int)>(42, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 41 },
        }}),
        // Slot 'onFlagChanged'
        QtMocHelpers::SlotData<void()>(43, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAttributeChanged'
        QtMocHelpers::SlotData<void()>(44, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ItemPropertyEditor, qt_meta_tag_ZN2UI7Widgets18ItemPropertyEditorE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject UI::Widgets::ItemPropertyEditor::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN2UI7Widgets18ItemPropertyEditorE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN2UI7Widgets18ItemPropertyEditorE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN2UI7Widgets18ItemPropertyEditorE_t>.metaTypes,
    nullptr
} };

void UI::Widgets::ItemPropertyEditor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ItemPropertyEditor *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->itemPropertyChanged(); break;
        case 1: _t->clientIdChanged((*reinterpret_cast< std::add_pointer_t<quint16>>(_a[1]))); break;
        case 2: _t->itemNameChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->itemTypeChanged((*reinterpret_cast< std::add_pointer_t<OTB::ServerItemType>>(_a[1]))); break;
        case 4: _t->stackOrderChanged((*reinterpret_cast< std::add_pointer_t<OTB::TileStackOrder>>(_a[1]))); break;
        case 5: _t->unpassableChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 6: _t->blockMissilesChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 7: _t->blockPathfinderChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 8: _t->hasElevationChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 9: _t->forceUseChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 10: _t->multiUseChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 11: _t->pickupableChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 12: _t->movableChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 13: _t->stackableChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 14: _t->readableChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 15: _t->rotatableChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 16: _t->hangableChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 17: _t->hookSouthChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 18: _t->hookEastChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 19: _t->ignoreLookChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 20: _t->fullGroundChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 21: _t->groundSpeedChanged((*reinterpret_cast< std::add_pointer_t<quint16>>(_a[1]))); break;
        case 22: _t->lightLevelChanged((*reinterpret_cast< std::add_pointer_t<quint16>>(_a[1]))); break;
        case 23: _t->lightColorChanged((*reinterpret_cast< std::add_pointer_t<quint16>>(_a[1]))); break;
        case 24: _t->minimapColorChanged((*reinterpret_cast< std::add_pointer_t<quint16>>(_a[1]))); break;
        case 25: _t->maxReadCharsChanged((*reinterpret_cast< std::add_pointer_t<quint16>>(_a[1]))); break;
        case 26: _t->maxReadWriteCharsChanged((*reinterpret_cast< std::add_pointer_t<quint16>>(_a[1]))); break;
        case 27: _t->tradeAsChanged((*reinterpret_cast< std::add_pointer_t<quint16>>(_a[1]))); break;
        case 28: _t->onClientIdChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 29: _t->onNameChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 30: _t->onTypeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 31: _t->onStackOrderChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 32: _t->onFlagChanged(); break;
        case 33: _t->onAttributeChanged(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)()>(_a, &ItemPropertyEditor::itemPropertyChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(quint16 )>(_a, &ItemPropertyEditor::clientIdChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(const QString & )>(_a, &ItemPropertyEditor::itemNameChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(OTB::ServerItemType )>(_a, &ItemPropertyEditor::itemTypeChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(OTB::TileStackOrder )>(_a, &ItemPropertyEditor::stackOrderChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(bool )>(_a, &ItemPropertyEditor::unpassableChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(bool )>(_a, &ItemPropertyEditor::blockMissilesChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(bool )>(_a, &ItemPropertyEditor::blockPathfinderChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(bool )>(_a, &ItemPropertyEditor::hasElevationChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(bool )>(_a, &ItemPropertyEditor::forceUseChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(bool )>(_a, &ItemPropertyEditor::multiUseChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(bool )>(_a, &ItemPropertyEditor::pickupableChanged, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(bool )>(_a, &ItemPropertyEditor::movableChanged, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(bool )>(_a, &ItemPropertyEditor::stackableChanged, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(bool )>(_a, &ItemPropertyEditor::readableChanged, 14))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(bool )>(_a, &ItemPropertyEditor::rotatableChanged, 15))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(bool )>(_a, &ItemPropertyEditor::hangableChanged, 16))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(bool )>(_a, &ItemPropertyEditor::hookSouthChanged, 17))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(bool )>(_a, &ItemPropertyEditor::hookEastChanged, 18))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(bool )>(_a, &ItemPropertyEditor::ignoreLookChanged, 19))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(bool )>(_a, &ItemPropertyEditor::fullGroundChanged, 20))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(quint16 )>(_a, &ItemPropertyEditor::groundSpeedChanged, 21))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(quint16 )>(_a, &ItemPropertyEditor::lightLevelChanged, 22))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(quint16 )>(_a, &ItemPropertyEditor::lightColorChanged, 23))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(quint16 )>(_a, &ItemPropertyEditor::minimapColorChanged, 24))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(quint16 )>(_a, &ItemPropertyEditor::maxReadCharsChanged, 25))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(quint16 )>(_a, &ItemPropertyEditor::maxReadWriteCharsChanged, 26))
            return;
        if (QtMocHelpers::indexOfMethod<void (ItemPropertyEditor::*)(quint16 )>(_a, &ItemPropertyEditor::tradeAsChanged, 27))
            return;
    }
}

const QMetaObject *UI::Widgets::ItemPropertyEditor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *UI::Widgets::ItemPropertyEditor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN2UI7Widgets18ItemPropertyEditorE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int UI::Widgets::ItemPropertyEditor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 34)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 34;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 34)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 34;
    }
    return _id;
}

// SIGNAL 0
void UI::Widgets::ItemPropertyEditor::itemPropertyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void UI::Widgets::ItemPropertyEditor::clientIdChanged(quint16 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void UI::Widgets::ItemPropertyEditor::itemNameChanged(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void UI::Widgets::ItemPropertyEditor::itemTypeChanged(OTB::ServerItemType _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void UI::Widgets::ItemPropertyEditor::stackOrderChanged(OTB::TileStackOrder _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void UI::Widgets::ItemPropertyEditor::unpassableChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void UI::Widgets::ItemPropertyEditor::blockMissilesChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void UI::Widgets::ItemPropertyEditor::blockPathfinderChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}

// SIGNAL 8
void UI::Widgets::ItemPropertyEditor::hasElevationChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1);
}

// SIGNAL 9
void UI::Widgets::ItemPropertyEditor::forceUseChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1);
}

// SIGNAL 10
void UI::Widgets::ItemPropertyEditor::multiUseChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 10, nullptr, _t1);
}

// SIGNAL 11
void UI::Widgets::ItemPropertyEditor::pickupableChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 11, nullptr, _t1);
}

// SIGNAL 12
void UI::Widgets::ItemPropertyEditor::movableChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 12, nullptr, _t1);
}

// SIGNAL 13
void UI::Widgets::ItemPropertyEditor::stackableChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 13, nullptr, _t1);
}

// SIGNAL 14
void UI::Widgets::ItemPropertyEditor::readableChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 14, nullptr, _t1);
}

// SIGNAL 15
void UI::Widgets::ItemPropertyEditor::rotatableChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 15, nullptr, _t1);
}

// SIGNAL 16
void UI::Widgets::ItemPropertyEditor::hangableChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 16, nullptr, _t1);
}

// SIGNAL 17
void UI::Widgets::ItemPropertyEditor::hookSouthChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 17, nullptr, _t1);
}

// SIGNAL 18
void UI::Widgets::ItemPropertyEditor::hookEastChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 18, nullptr, _t1);
}

// SIGNAL 19
void UI::Widgets::ItemPropertyEditor::ignoreLookChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 19, nullptr, _t1);
}

// SIGNAL 20
void UI::Widgets::ItemPropertyEditor::fullGroundChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 20, nullptr, _t1);
}

// SIGNAL 21
void UI::Widgets::ItemPropertyEditor::groundSpeedChanged(quint16 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 21, nullptr, _t1);
}

// SIGNAL 22
void UI::Widgets::ItemPropertyEditor::lightLevelChanged(quint16 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 22, nullptr, _t1);
}

// SIGNAL 23
void UI::Widgets::ItemPropertyEditor::lightColorChanged(quint16 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 23, nullptr, _t1);
}

// SIGNAL 24
void UI::Widgets::ItemPropertyEditor::minimapColorChanged(quint16 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 24, nullptr, _t1);
}

// SIGNAL 25
void UI::Widgets::ItemPropertyEditor::maxReadCharsChanged(quint16 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 25, nullptr, _t1);
}

// SIGNAL 26
void UI::Widgets::ItemPropertyEditor::maxReadWriteCharsChanged(quint16 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 26, nullptr, _t1);
}

// SIGNAL 27
void UI::Widgets::ItemPropertyEditor::tradeAsChanged(quint16 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 27, nullptr, _t1);
}
QT_WARNING_POP
