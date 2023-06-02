/****************************************************************************
** Meta object code from reading C++ file 'casterplayer_model.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../CasterSoundboard/casterplayer_model.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'casterplayer_model.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CasterPlayerModel_t {
    QByteArrayData data[16];
    char stringdata0[285];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CasterPlayerModel_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CasterPlayerModel_t qt_meta_stringdata_CasterPlayerModel = {
    {
QT_MOC_LITERAL(0, 0, 17), // "CasterPlayerModel"
QT_MOC_LITERAL(1, 18, 4), // "list"
QT_MOC_LITERAL(2, 23, 23), // "CasterPlayerController*"
QT_MOC_LITERAL(3, 47, 10), // "ModelRoles"
QT_MOC_LITERAL(4, 58, 18), // "IsInPlayerModeRole"
QT_MOC_LITERAL(5, 77, 12), // "IsLoopedRole"
QT_MOC_LITERAL(6, 90, 10), // "VolumeRole"
QT_MOC_LITERAL(7, 101, 26), // "IsPlayingRegionEnabledRole"
QT_MOC_LITERAL(8, 128, 19), // "PlayRegionBeginRole"
QT_MOC_LITERAL(9, 148, 17), // "PlayRegionEndRole"
QT_MOC_LITERAL(10, 166, 16), // "TriggerStyleRole"
QT_MOC_LITERAL(11, 183, 12), // "TriggerStyle"
QT_MOC_LITERAL(12, 196, 21), // "PlayPauseTriggerStyle"
QT_MOC_LITERAL(13, 218, 20), // "PlayStopTriggerStyle"
QT_MOC_LITERAL(14, 239, 21), // "PlayAgainTriggerStyle"
QT_MOC_LITERAL(15, 261, 23) // "SoundEffectTriggerStyle"

    },
    "CasterPlayerModel\0list\0CasterPlayerController*\0"
    "ModelRoles\0IsInPlayerModeRole\0"
    "IsLoopedRole\0VolumeRole\0"
    "IsPlayingRegionEnabledRole\0"
    "PlayRegionBeginRole\0PlayRegionEndRole\0"
    "TriggerStyleRole\0TriggerStyle\0"
    "PlayPauseTriggerStyle\0PlayStopTriggerStyle\0"
    "PlayAgainTriggerStyle\0SoundEffectTriggerStyle"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CasterPlayerModel[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       1,   14, // properties
       2,   17, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
       1, 0x80000000 | 2, 0x0009510b,

 // enums: name, alias, flags, count, data
       3,    3, 0x0,    7,   27,
      11,   11, 0x0,    4,   41,

 // enum data: key, value
       4, uint(CasterPlayerModel::IsInPlayerModeRole),
       5, uint(CasterPlayerModel::IsLoopedRole),
       6, uint(CasterPlayerModel::VolumeRole),
       7, uint(CasterPlayerModel::IsPlayingRegionEnabledRole),
       8, uint(CasterPlayerModel::PlayRegionBeginRole),
       9, uint(CasterPlayerModel::PlayRegionEndRole),
      10, uint(CasterPlayerModel::TriggerStyleRole),
      12, uint(CasterPlayerModel::PlayPauseTriggerStyle),
      13, uint(CasterPlayerModel::PlayStopTriggerStyle),
      14, uint(CasterPlayerModel::PlayAgainTriggerStyle),
      15, uint(CasterPlayerModel::SoundEffectTriggerStyle),

       0        // eod
};

void CasterPlayerModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{

#ifndef QT_NO_PROPERTIES
    if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<CasterPlayerModel *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< CasterPlayerController**>(_v) = _t->list(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<CasterPlayerModel *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setList(*reinterpret_cast< CasterPlayerController**>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject CasterPlayerModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_meta_stringdata_CasterPlayerModel.data,
    qt_meta_data_CasterPlayerModel,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CasterPlayerModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CasterPlayerModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CasterPlayerModel.stringdata0))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int CasterPlayerModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 1;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
