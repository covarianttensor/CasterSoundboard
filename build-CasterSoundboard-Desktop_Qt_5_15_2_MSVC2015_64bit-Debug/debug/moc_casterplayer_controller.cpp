/****************************************************************************
** Meta object code from reading C++ file 'casterplayer_controller.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../CasterSoundboard/casterplayer_controller.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'casterplayer_controller.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CasterPlayerController_t {
    QByteArrayData data[10];
    char stringdata0[129];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CasterPlayerController_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CasterPlayerController_t qt_meta_stringdata_CasterPlayerController = {
    {
QT_MOC_LITERAL(0, 0, 22), // "CasterPlayerController"
QT_MOC_LITERAL(1, 23, 13), // "preItemAppend"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 14), // "postItemAppend"
QT_MOC_LITERAL(4, 53, 14), // "preItemRemoved"
QT_MOC_LITERAL(5, 68, 5), // "index"
QT_MOC_LITERAL(6, 74, 15), // "postItemRemoved"
QT_MOC_LITERAL(7, 90, 10), // "appendItem"
QT_MOC_LITERAL(8, 101, 12), // "removeItemAt"
QT_MOC_LITERAL(9, 114, 14) // "removeAllItems"

    },
    "CasterPlayerController\0preItemAppend\0"
    "\0postItemAppend\0preItemRemoved\0index\0"
    "postItemRemoved\0appendItem\0removeItemAt\0"
    "removeAllItems"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CasterPlayerController[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x06 /* Public */,
       3,    0,   50,    2, 0x06 /* Public */,
       4,    1,   51,    2, 0x06 /* Public */,
       6,    0,   54,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    0,   55,    2, 0x0a /* Public */,
       8,    1,   56,    2, 0x0a /* Public */,
       9,    0,   59,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void,

       0        // eod
};

void CasterPlayerController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CasterPlayerController *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->preItemAppend(); break;
        case 1: _t->postItemAppend(); break;
        case 2: _t->preItemRemoved((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->postItemRemoved(); break;
        case 4: _t->appendItem(); break;
        case 5: _t->removeItemAt((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->removeAllItems(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CasterPlayerController::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CasterPlayerController::preItemAppend)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CasterPlayerController::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CasterPlayerController::postItemAppend)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CasterPlayerController::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CasterPlayerController::preItemRemoved)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (CasterPlayerController::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CasterPlayerController::postItemRemoved)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CasterPlayerController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CasterPlayerController.data,
    qt_meta_data_CasterPlayerController,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CasterPlayerController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CasterPlayerController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CasterPlayerController.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CasterPlayerController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void CasterPlayerController::preItemAppend()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void CasterPlayerController::postItemAppend()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void CasterPlayerController::preItemRemoved(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void CasterPlayerController::postItemRemoved()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
