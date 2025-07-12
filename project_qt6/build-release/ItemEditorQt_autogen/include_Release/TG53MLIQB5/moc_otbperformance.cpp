/****************************************************************************
** Meta object code from reading C++ file 'otbperformance.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../include/otb/otbperformance.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'otbperformance.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN3OTB18PerformanceMonitorE_t {};
} // unnamed namespace

template <> constexpr inline auto OTB::PerformanceMonitor::qt_create_metaobjectdata<qt_meta_tag_ZN3OTB18PerformanceMonitorE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "OTB::PerformanceMonitor",
        "memoryThresholdExceeded",
        "",
        "current",
        "threshold",
        "performanceThresholdExceeded",
        "cacheHitRatioLow",
        "metricsUpdated",
        "PerformanceMetrics",
        "metrics",
        "resetMetrics",
        "saveMetricsToFile",
        "filePath",
        "updateMetrics"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'memoryThresholdExceeded'
        QtMocHelpers::SignalData<void(qint64, qint64)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 3 }, { QMetaType::LongLong, 4 },
        }}),
        // Signal 'performanceThresholdExceeded'
        QtMocHelpers::SignalData<void(double, double)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 3 }, { QMetaType::Double, 4 },
        }}),
        // Signal 'cacheHitRatioLow'
        QtMocHelpers::SignalData<void(double, double)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 3 }, { QMetaType::Double, 4 },
        }}),
        // Signal 'metricsUpdated'
        QtMocHelpers::SignalData<void(const PerformanceMetrics &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 9 },
        }}),
        // Slot 'resetMetrics'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'saveMetricsToFile'
        QtMocHelpers::SlotData<void(const QString &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 12 },
        }}),
        // Slot 'updateMetrics'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<PerformanceMonitor, qt_meta_tag_ZN3OTB18PerformanceMonitorE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject OTB::PerformanceMonitor::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3OTB18PerformanceMonitorE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3OTB18PerformanceMonitorE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN3OTB18PerformanceMonitorE_t>.metaTypes,
    nullptr
} };

void OTB::PerformanceMonitor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<PerformanceMonitor *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->memoryThresholdExceeded((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[2]))); break;
        case 1: _t->performanceThresholdExceeded((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 2: _t->cacheHitRatioLow((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 3: _t->metricsUpdated((*reinterpret_cast< std::add_pointer_t<PerformanceMetrics>>(_a[1]))); break;
        case 4: _t->resetMetrics(); break;
        case 5: _t->saveMetricsToFile((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->updateMetrics(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (PerformanceMonitor::*)(qint64 , qint64 )>(_a, &PerformanceMonitor::memoryThresholdExceeded, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (PerformanceMonitor::*)(double , double )>(_a, &PerformanceMonitor::performanceThresholdExceeded, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (PerformanceMonitor::*)(double , double )>(_a, &PerformanceMonitor::cacheHitRatioLow, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (PerformanceMonitor::*)(const PerformanceMetrics & )>(_a, &PerformanceMonitor::metricsUpdated, 3))
            return;
    }
}

const QMetaObject *OTB::PerformanceMonitor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OTB::PerformanceMonitor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3OTB18PerformanceMonitorE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int OTB::PerformanceMonitor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void OTB::PerformanceMonitor::memoryThresholdExceeded(qint64 _t1, qint64 _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void OTB::PerformanceMonitor::performanceThresholdExceeded(double _t1, double _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void OTB::PerformanceMonitor::cacheHitRatioLow(double _t1, double _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void OTB::PerformanceMonitor::metricsUpdated(const PerformanceMetrics & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}
namespace {
struct qt_meta_tag_ZN3OTB20PerformanceOptimizerE_t {};
} // unnamed namespace

template <> constexpr inline auto OTB::PerformanceOptimizer::qt_create_metaobjectdata<qt_meta_tag_ZN3OTB20PerformanceOptimizerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "OTB::PerformanceOptimizer",
        "optimizationCompleted",
        "",
        "optimizationRecommendation",
        "recommendation",
        "performOptimization",
        "analyzeAndOptimize"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'optimizationCompleted'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'optimizationRecommendation'
        QtMocHelpers::SignalData<void(const QString &)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 4 },
        }}),
        // Slot 'performOptimization'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'analyzeAndOptimize'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<PerformanceOptimizer, qt_meta_tag_ZN3OTB20PerformanceOptimizerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject OTB::PerformanceOptimizer::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3OTB20PerformanceOptimizerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3OTB20PerformanceOptimizerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN3OTB20PerformanceOptimizerE_t>.metaTypes,
    nullptr
} };

void OTB::PerformanceOptimizer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<PerformanceOptimizer *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->optimizationCompleted(); break;
        case 1: _t->optimizationRecommendation((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->performOptimization(); break;
        case 3: _t->analyzeAndOptimize(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (PerformanceOptimizer::*)()>(_a, &PerformanceOptimizer::optimizationCompleted, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (PerformanceOptimizer::*)(const QString & )>(_a, &PerformanceOptimizer::optimizationRecommendation, 1))
            return;
    }
}

const QMetaObject *OTB::PerformanceOptimizer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OTB::PerformanceOptimizer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3OTB20PerformanceOptimizerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int OTB::PerformanceOptimizer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void OTB::PerformanceOptimizer::optimizationCompleted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void OTB::PerformanceOptimizer::optimizationRecommendation(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}
QT_WARNING_POP
