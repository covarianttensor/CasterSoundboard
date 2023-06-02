#-------------------------------------------------
#
# Project created by QtCreator 2013-05-27T05:20:34
#
#-------------------------------------------------

linux {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    BINDIR = $$PREFIX/bin
    DATADIR = $$PREFIX/share
}

macx {
    isEmpty(PREFIX) {
        PREFIX = ~
    }
    BINDIR = $$PREFIX/Applications
    DATADIR = $$PREFIX/Library/Application\ Support/CasterSoundboard
}

CONFIG += c++11
QT += qml quick core gui multimedia network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CasterSoundboard
TEMPLATE = app

SOURCES += main.cpp \
    casterplayer_controller.cpp \
    casterplayer_model.cpp

HEADERS  += \
    casterplayer_controller.h \
    casterplayer_model.h

RESOURCES += \
    qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

linux {
    appdata.files = ../dist/linux/CasterSoundboard.appdata.xml
    appdata.path = $$DATADIR/appdata/

    desktop.files = ../dist/linux/CasterSoundboard.desktop
    desktop.path = $$DATADIR/applications/

    pixmap.files = ../dist/linux/CasterSoundboard.png
    pixmap.path = $$DATADIR/pixmaps/

    INSTALLS += appdata desktop pixmap
}

macx {
    ICON = ../dist/macos/CasterSoundboard.icns
}

target.path = $$BINDIR
INSTALLS += target
