/****************************************************************************
** Meta object code from reading C++ file 'spritebrowser.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../include/ui/widgets/spritebrowser.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'spritebrowser.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN2UI7Widgets13SpriteBrowserE_t {};
} // unnamed namespace

template <> constexpr inline auto UI::Widgets::SpriteBrowser::qt_create_metaobjectdata<qt_meta_tag_ZN2UI7Widgets13SpriteBrowserE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "UI::Widgets::SpriteBrowser",
        "spriteSelected",
        "",
        "spriteId",
        "spriteDoubleClicked",
        "spriteAssignmentRequested",
        "ItemEditor::ClientItem*",
        "item",
        "candidateSelected",
        "const ItemEditor::ClientItem*",
        "onSearchTextChanged",
        "onFilterChanged",
        "onZoomChanged",
        "value",
        "onViewModeChanged",
        "onSpriteClicked",
        "onSpriteDoubleClicked",
        "onShowCandidatesClicked",
        "onAssignSpriteClicked",
        "onAnalyzeSimilarityClicked"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'spriteSelected'
        QtMocHelpers::SignalData<void(quint32)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::UInt, 3 },
        }}),
        // Signal 'spriteDoubleClicked'
        QtMocHelpers::SignalData<void(quint32)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::UInt, 3 },
        }}),
        // Signal 'spriteAssignmentRequested'
        QtMocHelpers::SignalData<void(quint32, ItemEditor::ClientItem *)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::UInt, 3 }, { 0x80000000 | 6, 7 },
        }}),
        // Signal 'candidateSelected'
        QtMocHelpers::SignalData<void(const ItemEditor::ClientItem *)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 9, 7 },
        }}),
        // Slot 'onSearchTextChanged'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onFilterChanged'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onZoomChanged'
        QtMocHelpers::SlotData<void(int)>(12, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 13 },
        }}),
        // Slot 'onViewModeChanged'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSpriteClicked'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSpriteDoubleClicked'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onShowCandidatesClicked'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAssignSpriteClicked'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAnalyzeSimilarityClicked'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SpriteBrowser, qt_meta_tag_ZN2UI7Widgets13SpriteBrowserE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject UI::Widgets::SpriteBrowser::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN2UI7Widgets13SpriteBrowserE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN2UI7Widgets13SpriteBrowserE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN2UI7Widgets13SpriteBrowserE_t>.metaTypes,
    nullptr
} };

void UI::Widgets::SpriteBrowser::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SpriteBrowser *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->spriteSelected((*reinterpret_cast< std::add_pointer_t<quint32>>(_a[1]))); break;
        case 1: _t->spriteDoubleClicked((*reinterpret_cast< std::add_pointer_t<quint32>>(_a[1]))); break;
        case 2: _t->spriteAssignmentRequested((*reinterpret_cast< std::add_pointer_t<quint32>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<ItemEditor::ClientItem*>>(_a[2]))); break;
        case 3: _t->candidateSelected((*reinterpret_cast< std::add_pointer_t<const ItemEditor::ClientItem*>>(_a[1]))); break;
        case 4: _t->onSearchTextChanged(); break;
        case 5: _t->onFilterChanged(); break;
        case 6: _t->onZoomChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->onViewModeChanged(); break;
        case 8: _t->onSpriteClicked(); break;
        case 9: _t->onSpriteDoubleClicked(); break;
        case 10: _t->onShowCandidatesClicked(); break;
        case 11: _t->onAssignSpriteClicked(); break;
        case 12: _t->onAnalyzeSimilarityClicked(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SpriteBrowser::*)(quint32 )>(_a, &SpriteBrowser::spriteSelected, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (SpriteBrowser::*)(quint32 )>(_a, &SpriteBrowser::spriteDoubleClicked, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (SpriteBrowser::*)(quint32 , ItemEditor::ClientItem * )>(_a, &SpriteBrowser::spriteAssignmentRequested, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (SpriteBrowser::*)(const ItemEditor::ClientItem * )>(_a, &SpriteBrowser::candidateSelected, 3))
            return;
    }
}

const QMetaObject *UI::Widgets::SpriteBrowser::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *UI::Widgets::SpriteBrowser::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN2UI7Widgets13SpriteBrowserE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int UI::Widgets::SpriteBrowser::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void UI::Widgets::SpriteBrowser::spriteSelected(quint32 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void UI::Widgets::SpriteBrowser::spriteDoubleClicked(quint32 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void UI::Widgets::SpriteBrowser::spriteAssignmentRequested(quint32 _t1, ItemEditor::ClientItem * _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void UI::Widgets::SpriteBrowser::candidateSelected(const ItemEditor::ClientItem * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}
QT_WARNING_POP
