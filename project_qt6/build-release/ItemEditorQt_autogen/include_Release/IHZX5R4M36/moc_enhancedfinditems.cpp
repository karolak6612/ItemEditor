/****************************************************************************
** Meta object code from reading C++ file 'enhancedfinditems.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../include/ui/dialogs/enhancedfinditems.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'enhancedfinditems.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN2UI7Dialogs17EnhancedFindItemsE_t {};
} // unnamed namespace

template <> constexpr inline auto UI::Dialogs::EnhancedFindItems::qt_create_metaobjectdata<qt_meta_tag_ZN2UI7Dialogs17EnhancedFindItemsE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "UI::Dialogs::EnhancedFindItems",
        "itemSelected",
        "",
        "OTB::ServerItem*",
        "item",
        "itemDoubleClicked",
        "onSearchTextChanged",
        "onFilterChanged",
        "onAdvancedToggled",
        "enabled",
        "onSearchClicked",
        "onClearClicked",
        "onItemSelectionChanged",
        "onItemDoubleClicked"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'itemSelected'
        QtMocHelpers::SignalData<void(OTB::ServerItem *)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'itemDoubleClicked'
        QtMocHelpers::SignalData<void(OTB::ServerItem *)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Slot 'onSearchTextChanged'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onFilterChanged'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAdvancedToggled'
        QtMocHelpers::SlotData<void(bool)>(8, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 9 },
        }}),
        // Slot 'onSearchClicked'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onClearClicked'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onItemSelectionChanged'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onItemDoubleClicked'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<EnhancedFindItems, qt_meta_tag_ZN2UI7Dialogs17EnhancedFindItemsE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject UI::Dialogs::EnhancedFindItems::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN2UI7Dialogs17EnhancedFindItemsE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN2UI7Dialogs17EnhancedFindItemsE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN2UI7Dialogs17EnhancedFindItemsE_t>.metaTypes,
    nullptr
} };

void UI::Dialogs::EnhancedFindItems::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<EnhancedFindItems *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->itemSelected((*reinterpret_cast< std::add_pointer_t<OTB::ServerItem*>>(_a[1]))); break;
        case 1: _t->itemDoubleClicked((*reinterpret_cast< std::add_pointer_t<OTB::ServerItem*>>(_a[1]))); break;
        case 2: _t->onSearchTextChanged(); break;
        case 3: _t->onFilterChanged(); break;
        case 4: _t->onAdvancedToggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 5: _t->onSearchClicked(); break;
        case 6: _t->onClearClicked(); break;
        case 7: _t->onItemSelectionChanged(); break;
        case 8: _t->onItemDoubleClicked(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (EnhancedFindItems::*)(OTB::ServerItem * )>(_a, &EnhancedFindItems::itemSelected, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (EnhancedFindItems::*)(OTB::ServerItem * )>(_a, &EnhancedFindItems::itemDoubleClicked, 1))
            return;
    }
}

const QMetaObject *UI::Dialogs::EnhancedFindItems::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *UI::Dialogs::EnhancedFindItems::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN2UI7Dialogs17EnhancedFindItemsE_t>.strings))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int UI::Dialogs::EnhancedFindItems::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void UI::Dialogs::EnhancedFindItems::itemSelected(OTB::ServerItem * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void UI::Dialogs::EnhancedFindItems::itemDoubleClicked(OTB::ServerItem * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}
QT_WARNING_POP
