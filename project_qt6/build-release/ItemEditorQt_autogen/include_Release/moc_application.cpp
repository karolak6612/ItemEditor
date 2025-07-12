/****************************************************************************
** Meta object code from reading C++ file 'application.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../include/core/application.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'application.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN10ItemEditor4Core11ApplicationE_t {};
} // unnamed namespace

template <> constexpr inline auto ItemEditor::Core::Application::qt_create_metaobjectdata<qt_meta_tag_ZN10ItemEditor4Core11ApplicationE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ItemEditor::Core::Application",
        "stateChanged",
        "",
        "State",
        "newState",
        "oldState",
        "mainWindowCreated",
        "MainWindow*",
        "window",
        "applicationReady",
        "autoSaveTriggered",
        "updateAvailable",
        "version",
        "url",
        "onAutoSaveTimer",
        "onFileChanged",
        "path",
        "onDirectoryChanged",
        "onSystemTrayActivated",
        "QSystemTrayIcon::ActivationReason",
        "reason",
        "onLanguageChanged"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'stateChanged'
        QtMocHelpers::SignalData<void(State, State)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { 0x80000000 | 3, 5 },
        }}),
        // Signal 'mainWindowCreated'
        QtMocHelpers::SignalData<void(MainWindow *)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 7, 8 },
        }}),
        // Signal 'applicationReady'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'autoSaveTriggered'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'updateAvailable'
        QtMocHelpers::SignalData<void(const QString &, const QString &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 12 }, { QMetaType::QString, 13 },
        }}),
        // Slot 'onAutoSaveTimer'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onFileChanged'
        QtMocHelpers::SlotData<void(const QString &)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 16 },
        }}),
        // Slot 'onDirectoryChanged'
        QtMocHelpers::SlotData<void(const QString &)>(17, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 16 },
        }}),
        // Slot 'onSystemTrayActivated'
        QtMocHelpers::SlotData<void(QSystemTrayIcon::ActivationReason)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 19, 20 },
        }}),
        // Slot 'onLanguageChanged'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<Application, qt_meta_tag_ZN10ItemEditor4Core11ApplicationE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ItemEditor::Core::Application::staticMetaObject = { {
    QMetaObject::SuperData::link<::Core::ApplicationBase::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10ItemEditor4Core11ApplicationE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10ItemEditor4Core11ApplicationE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10ItemEditor4Core11ApplicationE_t>.metaTypes,
    nullptr
} };

void ItemEditor::Core::Application::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Application *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->stateChanged((*reinterpret_cast< std::add_pointer_t<State>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<State>>(_a[2]))); break;
        case 1: _t->mainWindowCreated((*reinterpret_cast< std::add_pointer_t<MainWindow*>>(_a[1]))); break;
        case 2: _t->applicationReady(); break;
        case 3: _t->autoSaveTriggered(); break;
        case 4: _t->updateAvailable((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 5: _t->onAutoSaveTimer(); break;
        case 6: _t->onFileChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->onDirectoryChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->onSystemTrayActivated((*reinterpret_cast< std::add_pointer_t<QSystemTrayIcon::ActivationReason>>(_a[1]))); break;
        case 9: _t->onLanguageChanged(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (Application::*)(State , State )>(_a, &Application::stateChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (Application::*)(MainWindow * )>(_a, &Application::mainWindowCreated, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (Application::*)()>(_a, &Application::applicationReady, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (Application::*)()>(_a, &Application::autoSaveTriggered, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (Application::*)(const QString & , const QString & )>(_a, &Application::updateAvailable, 4))
            return;
    }
}

const QMetaObject *ItemEditor::Core::Application::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ItemEditor::Core::Application::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10ItemEditor4Core11ApplicationE_t>.strings))
        return static_cast<void*>(this);
    return ::Core::ApplicationBase::qt_metacast(_clname);
}

int ItemEditor::Core::Application::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ::Core::ApplicationBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void ItemEditor::Core::Application::stateChanged(State _t1, State _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void ItemEditor::Core::Application::mainWindowCreated(MainWindow * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void ItemEditor::Core::Application::applicationReady()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void ItemEditor::Core::Application::autoSaveTriggered()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void ItemEditor::Core::Application::updateAvailable(const QString & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}
QT_WARNING_POP
