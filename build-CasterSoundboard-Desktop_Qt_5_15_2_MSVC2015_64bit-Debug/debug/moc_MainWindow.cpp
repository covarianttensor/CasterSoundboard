/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../CasterSoundboard/MainWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[20];
    char stringdata0[302];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 8), // "aboutBox"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 9), // "addNewTab"
QT_MOC_LITERAL(4, 31, 34), // "mainTabContainerTabClosedRequ..."
QT_MOC_LITERAL(5, 66, 8), // "tabIndex"
QT_MOC_LITERAL(6, 75, 7), // "saveTab"
QT_MOC_LITERAL(7, 83, 9), // "saveAsTab"
QT_MOC_LITERAL(8, 93, 11), // "openProfile"
QT_MOC_LITERAL(9, 105, 13), // "stopAllSounds"
QT_MOC_LITERAL(10, 119, 16), // "renameCurrentTab"
QT_MOC_LITERAL(11, 136, 18), // "toggleAudioDucking"
QT_MOC_LITERAL(12, 155, 15), // "switchToNextTab"
QT_MOC_LITERAL(13, 171, 15), // "switchToPrevTab"
QT_MOC_LITERAL(14, 187, 20), // "currentTabWasChanged"
QT_MOC_LITERAL(15, 208, 19), // "updateCurrentOSCTab"
QT_MOC_LITERAL(16, 228, 15), // "hotKeyExecution"
QT_MOC_LITERAL(17, 244, 10), // "QKeyEvent*"
QT_MOC_LITERAL(18, 255, 5), // "event"
QT_MOC_LITERAL(19, 261, 40) // "handleGlobalHotKeyEventFromCu..."

    },
    "MainWindow\0aboutBox\0\0addNewTab\0"
    "mainTabContainerTabClosedRequested\0"
    "tabIndex\0saveTab\0saveAsTab\0openProfile\0"
    "stopAllSounds\0renameCurrentTab\0"
    "toggleAudioDucking\0switchToNextTab\0"
    "switchToPrevTab\0currentTabWasChanged\0"
    "updateCurrentOSCTab\0hotKeyExecution\0"
    "QKeyEvent*\0event\0"
    "handleGlobalHotKeyEventFromCurrentWidget"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   89,    2, 0x0a /* Public */,
       3,    0,   90,    2, 0x0a /* Public */,
       4,    1,   91,    2, 0x0a /* Public */,
       6,    0,   94,    2, 0x0a /* Public */,
       7,    0,   95,    2, 0x0a /* Public */,
       8,    0,   96,    2, 0x0a /* Public */,
       9,    0,   97,    2, 0x0a /* Public */,
      10,    0,   98,    2, 0x0a /* Public */,
      11,    0,   99,    2, 0x0a /* Public */,
      12,    0,  100,    2, 0x0a /* Public */,
      13,    0,  101,    2, 0x0a /* Public */,
      14,    1,  102,    2, 0x0a /* Public */,
      15,    1,  105,    2, 0x0a /* Public */,
      16,    1,  108,    2, 0x0a /* Public */,
      19,    1,  111,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, 0x80000000 | 17,   18,
    QMetaType::Void, 0x80000000 | 17,   18,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->aboutBox(); break;
        case 1: _t->addNewTab(); break;
        case 2: _t->mainTabContainerTabClosedRequested((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->saveTab(); break;
        case 4: _t->saveAsTab(); break;
        case 5: _t->openProfile(); break;
        case 6: _t->stopAllSounds(); break;
        case 7: _t->renameCurrentTab(); break;
        case 8: _t->toggleAudioDucking(); break;
        case 9: _t->switchToNextTab(); break;
        case 10: _t->switchToPrevTab(); break;
        case 11: _t->currentTabWasChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->updateCurrentOSCTab((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->hotKeyExecution((*reinterpret_cast< QKeyEvent*(*)>(_a[1]))); break;
        case 14: _t->handleGlobalHotKeyEventFromCurrentWidget((*reinterpret_cast< QKeyEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 15;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
