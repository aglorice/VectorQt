/****************************************************************************
** Meta object code from reading C++ file 'drawing-tool-base.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../drawing-tool-base.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'drawing-tool-base.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN15DrawingToolBaseE_t {};
} // unnamed namespace

template <> constexpr inline auto DrawingToolBase::qt_create_metaobjectdata<qt_meta_tag_ZN15DrawingToolBaseE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "DrawingToolBase"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DrawingToolBase, qt_meta_tag_ZN15DrawingToolBaseE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject DrawingToolBase::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15DrawingToolBaseE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15DrawingToolBaseE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15DrawingToolBaseE_t>.metaTypes,
    nullptr
} };

void DrawingToolBase::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DrawingToolBase *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *DrawingToolBase::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DrawingToolBase::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15DrawingToolBaseE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DrawingToolBase::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_tag_ZN13RectangleToolE_t {};
} // unnamed namespace

template <> constexpr inline auto RectangleTool::qt_create_metaobjectdata<qt_meta_tag_ZN13RectangleToolE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "RectangleTool"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<RectangleTool, qt_meta_tag_ZN13RectangleToolE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject RectangleTool::staticMetaObject = { {
    QMetaObject::SuperData::link<DrawingToolBase::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13RectangleToolE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13RectangleToolE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13RectangleToolE_t>.metaTypes,
    nullptr
} };

void RectangleTool::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<RectangleTool *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *RectangleTool::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RectangleTool::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13RectangleToolE_t>.strings))
        return static_cast<void*>(this);
    return DrawingToolBase::qt_metacast(_clname);
}

int RectangleTool::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DrawingToolBase::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_tag_ZN11EllipseToolE_t {};
} // unnamed namespace

template <> constexpr inline auto EllipseTool::qt_create_metaobjectdata<qt_meta_tag_ZN11EllipseToolE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "EllipseTool"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<EllipseTool, qt_meta_tag_ZN11EllipseToolE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject EllipseTool::staticMetaObject = { {
    QMetaObject::SuperData::link<DrawingToolBase::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11EllipseToolE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11EllipseToolE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11EllipseToolE_t>.metaTypes,
    nullptr
} };

void EllipseTool::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<EllipseTool *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *EllipseTool::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *EllipseTool::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11EllipseToolE_t>.strings))
        return static_cast<void*>(this);
    return DrawingToolBase::qt_metacast(_clname);
}

int EllipseTool::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DrawingToolBase::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_tag_ZN10SelectToolE_t {};
} // unnamed namespace

template <> constexpr inline auto SelectTool::qt_create_metaobjectdata<qt_meta_tag_ZN10SelectToolE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SelectTool"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SelectTool, qt_meta_tag_ZN10SelectToolE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SelectTool::staticMetaObject = { {
    QMetaObject::SuperData::link<DrawingToolBase::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10SelectToolE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10SelectToolE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10SelectToolE_t>.metaTypes,
    nullptr
} };

void SelectTool::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SelectTool *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *SelectTool::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SelectTool::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10SelectToolE_t>.strings))
        return static_cast<void*>(this);
    return DrawingToolBase::qt_metacast(_clname);
}

int SelectTool::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DrawingToolBase::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
