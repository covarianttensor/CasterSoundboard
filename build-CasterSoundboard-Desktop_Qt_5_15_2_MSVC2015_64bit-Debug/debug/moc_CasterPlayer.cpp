/****************************************************************************
** Meta object code from reading C++ file 'CasterPlayer.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../CasterSoundboard/CasterPlayer.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CasterPlayer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CasterPlayerWidget_t {
    QByteArrayData data[18];
    char stringdata0[256];
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
QT_MOC_LITERAL(5, 53, 15), // "trackBarChanged"
QT_MOC_LITERAL(6, 69, 12), // "openFileDiag"
QT_MOC_LITERAL(7, 82, 15), // "openColorPicker"
QT_MOC_LITERAL(8, 98, 13), // "openCuePicker"
QT_MOC_LITERAL(9, 112, 13), // "toggleLooping"
QT_MOC_LITERAL(10, 126, 21), // "playerPositionChanged"
QT_MOC_LITERAL(11, 148, 8), // "position"
QT_MOC_LITERAL(12, 157, 18), // "playerStateChanged"
QT_MOC_LITERAL(13, 176, 19), // "QMediaPlayer::State"
QT_MOC_LITERAL(14, 196, 5), // "state"
QT_MOC_LITERAL(15, 202, 21), // "playerMetaDataChanged"
QT_MOC_LITERAL(16, 224, 21), // "playerDurationChanged"
QT_MOC_LITERAL(17, 246, 9) // "_duration"

    },
    "CasterPlayerWidget\0playerToggle\0\0"
    "volumeChanged\0value\0trackBarChanged\0"
    "openFileDiag\0openColorPicker\0openCuePicker\0"
    "toggleLooping\0playerPositionChanged\0"
    "position\0playerStateChanged\0"
    "QMediaPlayer::State\0state\0"
    "playerMetaDataChanged\0playerDurationChanged\0"
    "_duration"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CasterPlayerWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x0a /* Public */,
       3,    1,   70,    2, 0x0a /* Public */,
       5,    1,   73,    2, 0x0a /* Public */,
       6,    0,   76,    2, 0x0a /* Public */,
       7,    0,   77,    2, 0x0a /* Public */,
       8,    0,   78,    2, 0x0a /* Public */,
       9,    0,   79,    2, 0x0a /* Public */,
      10,    1,   80,    2, 0x0a /* Public */,
      12,    1,   83,    2, 0x0a /* Public */,
      15,    0,   86,    2, 0x0a /* Public */,
      16,    1,   87,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::LongLong,   11,
    QMetaType::Void, 0x80000000 | 13,   14,
    QMetaType::Void,
    QMetaType::Void, QMetaType::LongLong,   17,

       0        // eod
};

void CasterPlayerWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CasterPlayerWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->playerToggle(); break;
        case 1: _t->volumeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->trackBarChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->openFileDiag(); break;
        case 4: _t->openColorPicker(); break;
        case 5: _t->openCuePicker(); break;
        case 6: _t->toggleLooping(); break;
        case 7: _t->playerPositionChanged((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 8: _t->playerStateChanged((*reinterpret_cast< QMediaPlayer::State(*)>(_a[1]))); break;
        case 9: _t->playerMetaDataChanged(); break;
        case 10: _t->playerDurationChanged((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 8:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QMediaPlayer::State >(); break;
            }
            break;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CasterPlayerWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CasterPlayerWidget.data,
    qt_meta_data_CasterPlayerWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CasterPlayerWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CasterPlayerWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CasterPlayerWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CasterPlayerWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
