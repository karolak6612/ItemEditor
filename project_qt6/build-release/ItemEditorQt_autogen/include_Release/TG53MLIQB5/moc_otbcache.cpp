/****************************************************************************
** Meta object code from reading C++ file 'otbcache.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../include/otb/otbcache.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'otbcache.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN3OTB8OtbCacheE_t {};
} // unnamed namespace

template <> constexpr inline auto OTB::OtbCache::qt_create_metaobjectdata<qt_meta_tag_ZN3OTB8OtbCacheE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "OTB::OtbCache",
        "cacheHit",
        "",
        "CacheLevel",
        "level",
        "itemId",
        "cacheMiss",
        "memoryLimitReached",
        "currentUsage",
        "limit",
        "evictionOccurred",
        "performCleanup",
        "performOptimization",
        "onCleanupTimer"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'cacheHit'
        QtMocHelpers::SignalData<void(CacheLevel, quint16)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { QMetaType::UShort, 5 },
        }}),
        // Signal 'cacheMiss'
        QtMocHelpers::SignalData<void(CacheLevel, quint16)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { QMetaType::UShort, 5 },
        }}),
        // Signal 'memoryLimitReached'
        QtMocHelpers::SignalData<void(qint64, qint64)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 8 }, { QMetaType::LongLong, 9 },
        }}),
        // Signal 'evictionOccurred'
        QtMocHelpers::SignalData<void(CacheLevel, quint16)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { QMetaType::UShort, 5 },
        }}),
        // Slot 'performCleanup'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'performOptimization'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onCleanupTimer'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<OtbCache, qt_meta_tag_ZN3OTB8OtbCacheE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject OTB::OtbCache::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3OTB8OtbCacheE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3OTB8OtbCacheE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN3OTB8OtbCacheE_t>.metaTypes,
    nullptr
} };

void OTB::OtbCache::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<OtbCache *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->cacheHit((*reinterpret_cast< std::add_pointer_t<CacheLevel>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[2]))); break;
        case 1: _t->cacheMiss((*reinterpret_cast< std::add_pointer_t<CacheLevel>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[2]))); break;
        case 2: _t->memoryLimitReached((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[2]))); break;
        case 3: _t->evictionOccurred((*reinterpret_cast< std::add_pointer_t<CacheLevel>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[2]))); break;
        case 4: _t->performCleanup(); break;
        case 5: _t->performOptimization(); break;
        case 6: _t->onCleanupTimer(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (OtbCache::*)(CacheLevel , quint16 )>(_a, &OtbCache::cacheHit, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (OtbCache::*)(CacheLevel , quint16 )>(_a, &OtbCache::cacheMiss, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (OtbCache::*)(qint64 , qint64 )>(_a, &OtbCache::memoryLimitReached, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (OtbCache::*)(CacheLevel , quint16 )>(_a, &OtbCache::evictionOccurred, 3))
            return;
    }
}

const QMetaObject *OTB::OtbCache::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OTB::OtbCache::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3OTB8OtbCacheE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int OTB::OtbCache::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void OTB::OtbCache::cacheHit(CacheLevel _t1, quint16 _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void OTB::OtbCache::cacheMiss(CacheLevel _t1, quint16 _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void OTB::OtbCache::memoryLimitReached(qint64 _t1, qint64 _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void OTB::OtbCache::evictionOccurred(CacheLevel _t1, quint16 _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}
QT_WARNING_POP
