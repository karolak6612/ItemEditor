/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../include/ui/mainwindow.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MainWindow",
        "newFile",
        "",
        "openFile",
        "saveFile",
        "saveFileAs",
        "showPreferences",
        "createNewItem",
        "duplicateCurrentItem",
        "reloadCurrentItem",
        "findItem",
        "createMissingItems",
        "onShowMismatchedToggled",
        "checked",
        "onShowDeprecatedToggled",
        "buildFilteredItemsList",
        "reloadAllItemAttributes",
        "compareOtbFiles",
        "updateOtbVersion",
        "about",
        "onServerItemSelectionChanged",
        "QListWidgetItem*",
        "current",
        "previous",
        "updateItemDetailsView",
        "OTB::ServerItem*",
        "item",
        "onClientIdChanged",
        "value",
        "onItemNameChanged",
        "text",
        "onItemTypeChanged",
        "index",
        "onStackOrderChanged",
        "onUnpassableChanged",
        "onBlockMissilesChanged",
        "onBlockPathfinderChanged",
        "onHasElevationChanged",
        "onForceUseChanged",
        "onMultiUseChanged",
        "onPickupableChanged",
        "onMovableChanged",
        "onStackableChanged",
        "onReadableChanged",
        "onRotatableChanged",
        "onHangableChanged",
        "onHookSouthChanged",
        "onHookEastChanged",
        "onIgnoreLookChanged",
        "onFullGroundChanged",
        "onGroundSpeedChanged",
        "onLightLevelChanged",
        "onLightColorChanged",
        "onMinimapColorChanged",
        "onMaxReadCharsChanged",
        "onMaxReadWriteCharsChanged",
        "onWareIdChanged",
        "showSpriteCandidates",
        "showServerListContextMenu",
        "pos",
        "copyServerId",
        "copyClientId",
        "copyItemName",
        "showToolBarContextMenu",
        "customizeToolBar",
        "resetToolBar",
        "toggleToolBarVisibility"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'newFile'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'openFile'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'saveFile'
        QtMocHelpers::SlotData<bool()>(4, 2, QMC::AccessPrivate, QMetaType::Bool),
        // Slot 'saveFileAs'
        QtMocHelpers::SlotData<bool()>(5, 2, QMC::AccessPrivate, QMetaType::Bool),
        // Slot 'showPreferences'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'createNewItem'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'duplicateCurrentItem'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'reloadCurrentItem'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'findItem'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'createMissingItems'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onShowMismatchedToggled'
        QtMocHelpers::SlotData<void(bool)>(12, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'onShowDeprecatedToggled'
        QtMocHelpers::SlotData<void(bool)>(14, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'buildFilteredItemsList'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'reloadAllItemAttributes'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'compareOtbFiles'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'updateOtbVersion'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'about'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onServerItemSelectionChanged'
        QtMocHelpers::SlotData<void(QListWidgetItem *, QListWidgetItem *)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 21, 22 }, { 0x80000000 | 21, 23 },
        }}),
        // Slot 'updateItemDetailsView'
        QtMocHelpers::SlotData<void(OTB::ServerItem *)>(24, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 25, 26 },
        }}),
        // Slot 'onClientIdChanged'
        QtMocHelpers::SlotData<void(int)>(27, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 28 },
        }}),
        // Slot 'onItemNameChanged'
        QtMocHelpers::SlotData<void(const QString &)>(29, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 30 },
        }}),
        // Slot 'onItemTypeChanged'
        QtMocHelpers::SlotData<void(int)>(31, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 32 },
        }}),
        // Slot 'onStackOrderChanged'
        QtMocHelpers::SlotData<void(int)>(33, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 32 },
        }}),
        // Slot 'onUnpassableChanged'
        QtMocHelpers::SlotData<void(bool)>(34, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'onBlockMissilesChanged'
        QtMocHelpers::SlotData<void(bool)>(35, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'onBlockPathfinderChanged'
        QtMocHelpers::SlotData<void(bool)>(36, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'onHasElevationChanged'
        QtMocHelpers::SlotData<void(bool)>(37, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'onForceUseChanged'
        QtMocHelpers::SlotData<void(bool)>(38, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'onMultiUseChanged'
        QtMocHelpers::SlotData<void(bool)>(39, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'onPickupableChanged'
        QtMocHelpers::SlotData<void(bool)>(40, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'onMovableChanged'
        QtMocHelpers::SlotData<void(bool)>(41, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'onStackableChanged'
        QtMocHelpers::SlotData<void(bool)>(42, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'onReadableChanged'
        QtMocHelpers::SlotData<void(bool)>(43, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'onRotatableChanged'
        QtMocHelpers::SlotData<void(bool)>(44, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'onHangableChanged'
        QtMocHelpers::SlotData<void(bool)>(45, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'onHookSouthChanged'
        QtMocHelpers::SlotData<void(bool)>(46, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'onHookEastChanged'
        QtMocHelpers::SlotData<void(bool)>(47, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'onIgnoreLookChanged'
        QtMocHelpers::SlotData<void(bool)>(48, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'onFullGroundChanged'
        QtMocHelpers::SlotData<void(bool)>(49, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'onGroundSpeedChanged'
        QtMocHelpers::SlotData<void(const QString &)>(50, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 30 },
        }}),
        // Slot 'onLightLevelChanged'
        QtMocHelpers::SlotData<void(const QString &)>(51, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 30 },
        }}),
        // Slot 'onLightColorChanged'
        QtMocHelpers::SlotData<void(const QString &)>(52, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 30 },
        }}),
        // Slot 'onMinimapColorChanged'
        QtMocHelpers::SlotData<void(const QString &)>(53, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 30 },
        }}),
        // Slot 'onMaxReadCharsChanged'
        QtMocHelpers::SlotData<void(const QString &)>(54, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 30 },
        }}),
        // Slot 'onMaxReadWriteCharsChanged'
        QtMocHelpers::SlotData<void(const QString &)>(55, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 30 },
        }}),
        // Slot 'onWareIdChanged'
        QtMocHelpers::SlotData<void(const QString &)>(56, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 30 },
        }}),
        // Slot 'showSpriteCandidates'
        QtMocHelpers::SlotData<void()>(57, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showServerListContextMenu'
        QtMocHelpers::SlotData<void(const QPoint &)>(58, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QPoint, 59 },
        }}),
        // Slot 'copyServerId'
        QtMocHelpers::SlotData<void()>(60, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'copyClientId'
        QtMocHelpers::SlotData<void()>(61, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'copyItemName'
        QtMocHelpers::SlotData<void()>(62, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showToolBarContextMenu'
        QtMocHelpers::SlotData<void(const QPoint &)>(63, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QPoint, 59 },
        }}),
        // Slot 'customizeToolBar'
        QtMocHelpers::SlotData<void()>(64, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'resetToolBar'
        QtMocHelpers::SlotData<void()>(65, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'toggleToolBarVisibility'
        QtMocHelpers::SlotData<void()>(66, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10MainWindowE_t>.metaTypes,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->newFile(); break;
        case 1: _t->openFile(); break;
        case 2: { bool _r = _t->saveFile();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 3: { bool _r = _t->saveFileAs();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 4: _t->showPreferences(); break;
        case 5: _t->createNewItem(); break;
        case 6: _t->duplicateCurrentItem(); break;
        case 7: _t->reloadCurrentItem(); break;
        case 8: _t->findItem(); break;
        case 9: _t->createMissingItems(); break;
        case 10: _t->onShowMismatchedToggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 11: _t->onShowDeprecatedToggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 12: _t->buildFilteredItemsList(); break;
        case 13: _t->reloadAllItemAttributes(); break;
        case 14: _t->compareOtbFiles(); break;
        case 15: _t->updateOtbVersion(); break;
        case 16: _t->about(); break;
        case 17: _t->onServerItemSelectionChanged((*reinterpret_cast< std::add_pointer_t<QListWidgetItem*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QListWidgetItem*>>(_a[2]))); break;
        case 18: _t->updateItemDetailsView((*reinterpret_cast< std::add_pointer_t<OTB::ServerItem*>>(_a[1]))); break;
        case 19: _t->onClientIdChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 20: _t->onItemNameChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 21: _t->onItemTypeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 22: _t->onStackOrderChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 23: _t->onUnpassableChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 24: _t->onBlockMissilesChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 25: _t->onBlockPathfinderChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 26: _t->onHasElevationChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 27: _t->onForceUseChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 28: _t->onMultiUseChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 29: _t->onPickupableChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 30: _t->onMovableChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 31: _t->onStackableChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 32: _t->onReadableChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 33: _t->onRotatableChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 34: _t->onHangableChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 35: _t->onHookSouthChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 36: _t->onHookEastChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 37: _t->onIgnoreLookChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 38: _t->onFullGroundChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 39: _t->onGroundSpeedChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 40: _t->onLightLevelChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 41: _t->onLightColorChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 42: _t->onMinimapColorChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 43: _t->onMaxReadCharsChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 44: _t->onMaxReadWriteCharsChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 45: _t->onWareIdChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 46: _t->showSpriteCandidates(); break;
        case 47: _t->showServerListContextMenu((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 48: _t->copyServerId(); break;
        case 49: _t->copyClientId(); break;
        case 50: _t->copyItemName(); break;
        case 51: _t->showToolBarContextMenu((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 52: _t->customizeToolBar(); break;
        case 53: _t->resetToolBar(); break;
        case 54: _t->toggleToolBarVisibility(); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 55)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 55;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 55)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 55;
    }
    return _id;
}
QT_WARNING_POP
