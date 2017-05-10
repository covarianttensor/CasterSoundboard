#-------------------------------------------------
#
# Project created by QtCreator 2013-05-27T05:20:34
#
#-------------------------------------------------


QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CasterSoundboard
TEMPLATE = app


SOURCES += main.cpp\
    CasterPlayer.cpp \
    CasterBoard.cpp \
    MainWindow.cpp \
    CasterLabelColorPicker.cpp \
    CSS.cpp \
    CasterPlayerState.cpp \
    CasterCuePicker.cpp

HEADERS  += \
    CasterPlayer.h \
    CasterBoard.h \
    CSS.h \
    MainWindow.h \
    CasterLabelColorPicker.h \
    CasterPlayerState.h \
    CasterCuePicker.h

FORMS    +=

RESOURCES += \
    res.qrc

target.path = $${PREFIX}/bin
INSTALLS += target
