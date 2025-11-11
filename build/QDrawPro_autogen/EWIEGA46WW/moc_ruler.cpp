/****************************************************************************
** Meta object code from reading C++ file 'ruler.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../ruler.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ruler.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.2. It"
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
struct qt_meta_tag_ZN5RulerE_t {};
} // unnamed namespace

template <> constexpr inline auto Ruler::qt_create_metaobjectdata<qt_meta_tag_ZN5RulerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Ruler",
        "unitChanged",
        "",
        "Unit",
        "unit",
        "guideRequested",
        "position",
        "Qt::Orientation",
        "orientation",
        "unitChangedForAll",
        "setUnitPixels",
        "setUnitMillimeters",
        "setUnitCentimeters",
        "setUnitInches",
        "setUnitPoints"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'unitChanged'
        QtMocHelpers::SignalData<void(enum Unit)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'guideRequested'
        QtMocHelpers::SignalData<void(const QPointF &, Qt::Orientation)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QPointF, 6 }, { 0x80000000 | 7, 8 },
        }}),
        // Signal 'unitChangedForAll'
        QtMocHelpers::SignalData<void(enum Unit)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Slot 'setUnitPixels'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'setUnitMillimeters'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'setUnitCentimeters'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'setUnitInches'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'setUnitPoints'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<Ruler, qt_meta_tag_ZN5RulerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Ruler::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN5RulerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN5RulerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN5RulerE_t>.metaTypes,
    nullptr
} };

void Ruler::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Ruler *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->unitChanged((*reinterpret_cast< std::add_pointer_t<enum Unit>>(_a[1]))); break;
        case 1: _t->guideRequested((*reinterpret_cast< std::add_pointer_t<QPointF>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<Qt::Orientation>>(_a[2]))); break;
        case 2: _t->unitChangedForAll((*reinterpret_cast< std::add_pointer_t<enum Unit>>(_a[1]))); break;
        case 3: _t->setUnitPixels(); break;
        case 4: _t->setUnitMillimeters(); break;
        case 5: _t->setUnitCentimeters(); break;
        case 6: _t->setUnitInches(); break;
        case 7: _t->setUnitPoints(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (Ruler::*)(Unit )>(_a, &Ruler::unitChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (Ruler::*)(const QPointF & , Qt::Orientation )>(_a, &Ruler::guideRequested, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (Ruler::*)(Unit )>(_a, &Ruler::unitChangedForAll, 2))
            return;
    }
}

const QMetaObject *Ruler::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Ruler::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN5RulerE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Ruler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void Ruler::unitChanged(Unit _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void Ruler::guideRequested(const QPointF & _t1, Qt::Orientation _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void Ruler::unitChangedForAll(Unit _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
QT_WARNING_POP
