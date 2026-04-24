/****************************************************************************
** Meta object code from reading C++ file 'corewrapper.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.18)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../JGE/include/qt/corewrapper.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'corewrapper.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.18. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_WagicCore_t {
    QByteArrayData data[18];
    char stringdata0[143];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_WagicCore_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_WagicCore_t qt_meta_stringdata_WagicCore = {
    {
QT_MOC_LITERAL(0, 0, 9), // "WagicCore"
QT_MOC_LITERAL(1, 10, 13), // "activeChanged"
QT_MOC_LITERAL(2, 24, 0), // ""
QT_MOC_LITERAL(3, 25, 5), // "start"
QT_MOC_LITERAL(4, 31, 4), // "doOK"
QT_MOC_LITERAL(5, 36, 6), // "doNext"
QT_MOC_LITERAL(6, 43, 8), // "doCancel"
QT_MOC_LITERAL(7, 52, 6), // "doMenu"
QT_MOC_LITERAL(8, 59, 4), // "done"
QT_MOC_LITERAL(9, 64, 10), // "pixelInput"
QT_MOC_LITERAL(10, 75, 1), // "x"
QT_MOC_LITERAL(11, 77, 1), // "y"
QT_MOC_LITERAL(12, 79, 7), // "getTick"
QT_MOC_LITERAL(13, 87, 8), // "doScroll"
QT_MOC_LITERAL(14, 96, 12), // "nominalWidth"
QT_MOC_LITERAL(15, 109, 13), // "nominalHeight"
QT_MOC_LITERAL(16, 123, 12), // "nominalRatio"
QT_MOC_LITERAL(17, 136, 6) // "active"

    },
    "WagicCore\0activeChanged\0\0start\0doOK\0"
    "doNext\0doCancel\0doMenu\0done\0pixelInput\0"
    "x\0y\0getTick\0doScroll\0nominalWidth\0"
    "nominalHeight\0nominalRatio\0active"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_WagicCore[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       4,   86, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    1,   65,    2, 0x0a /* Public */,

 // methods: name, argc, parameters, tag, flags
       4,    0,   68,    2, 0x02 /* Public */,
       5,    0,   69,    2, 0x02 /* Public */,
       6,    0,   70,    2, 0x02 /* Public */,
       7,    0,   71,    2, 0x02 /* Public */,
       8,    0,   72,    2, 0x02 /* Public */,
       9,    2,   73,    2, 0x02 /* Public */,
      12,    0,   78,    2, 0x02 /* Public */,
      13,    3,   79,    2, 0x02 /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   10,   11,
    QMetaType::LongLong,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int,   10,   11,    2,

 // properties: name, type, flags
      14, QMetaType::Int, 0x00095401,
      15, QMetaType::Int, 0x00095401,
      16, QMetaType::Float, 0x00095401,
      17, QMetaType::Bool, 0x00495103,

 // properties: notify_signal_id
       0,
       0,
       0,
       0,

       0        // eod
};

void WagicCore::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<WagicCore *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->activeChanged(); break;
        case 1: _t->start((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->doOK(); break;
        case 3: _t->doNext(); break;
        case 4: _t->doCancel(); break;
        case 5: _t->doMenu(); break;
        case 6: _t->done(); break;
        case 7: _t->pixelInput((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 8: { qint64 _r = _t->getTick();
            if (_a[0]) *reinterpret_cast< qint64*>(_a[0]) = std::move(_r); }  break;
        case 9: _t->doScroll((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (WagicCore::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&WagicCore::activeChanged)) {
                *result = 0;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<WagicCore *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = _t->getNominalWidth(); break;
        case 1: *reinterpret_cast< int*>(_v) = _t->getNominalHeight(); break;
        case 2: *reinterpret_cast< float*>(_v) = _t->getNominalRatio(); break;
        case 3: *reinterpret_cast< bool*>(_v) = _t->getActive(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<WagicCore *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 3: _t->setActive(*reinterpret_cast< bool*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject WagicCore::staticMetaObject = { {
    QMetaObject::SuperData::link<QGLWidget::staticMetaObject>(),
    qt_meta_stringdata_WagicCore.data,
    qt_meta_data_WagicCore,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *WagicCore::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WagicCore::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_WagicCore.stringdata0))
        return static_cast<void*>(this);
    return QGLWidget::qt_metacast(_clname);
}

int WagicCore::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 4;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void WagicCore::activeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
