/****************************************************************************
** Meta object code from reading C++ file 'CasterPlayer.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../CasterSoundboard/CasterPlayer.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CasterPlayer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CasterPlayerWidget_t {
    QByteArrayData data[12];
    char stringdata0[163];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CasterPlayerWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CasterPlayerWidget_t qt_meta_stringdata_CasterPlayerWidget = {
    {
QT_MOC_LITERAL(0, 0, 18), // "CasterPlayerWidget"
QT_MOC_LITERAL(1, 19, 12), // "playerToggle"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 13), // "volumeChanged"
QT_MOC_LITERAL(4, 47, 5), // "value"
QT_MOC_LITERAL(5, 53, 11), // "openSubMenu"
QT_MOC_LITERAL(6, 65, 21), // "playerPositionChanged"
QT_MOC_LITERAL(7, 87, 8), // "position"
QT_MOC_LITERAL(8, 96, 18), // "playerStateChanged"
QT_MOC_LITERAL(9, 115, 19), // "QMediaPlayer::State"
QT_MOC_LITERAL(10, 135, 5), // "state"
QT_MOC_LITERAL(11, 141, 21) // "playerMetaDataChanged"

    },
    "CasterPlayerWidget\0playerToggle\0\0"
    "volumeChanged\0value\0openSubMenu\0"
    "playerPositionChanged\0position\0"
    "playerStateChanged\0QMediaPlayer::State\0"
    "state\0playerMetaDataChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CasterPlayerWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x0a /* Public */,
       3,    1,   45,    2, 0x0a /* Public */,
       5,    0,   48,    2, 0x0a /* Public */,
       6,    1,   49,    2, 0x0a /* Public */,
       8,    1,   52,    2, 0x0a /* Public */,
      11,    0,   55,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void,
    QMetaType::Void, QMetaType::LongLong,    7,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void,

       0        // eod
};

void CasterPlayerWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CasterPlayerWidget *_t = static_cast<CasterPlayerWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->playerToggle(); break;
        case 1: _t->volumeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->openSubMenu(); break;
        case 3: _t->playerPositionChanged((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 4: _t->playerStateChanged((*reinterpret_cast< QMediaPlayer::State(*)>(_a[1]))); break;
        case 5: _t->playerMetaDataChanged(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QMediaPlayer::State >(); break;
            }
            break;
        }
    }
}

const QMetaObject CasterPlayerWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_CasterPlayerWidget.data,
      qt_meta_data_CasterPlayerWidget,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *CasterPlayerWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CasterPlayerWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_CasterPlayerWidget.stringdata0))
        return static_cast<void*>(const_cast< CasterPlayerWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int CasterPlayerWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
