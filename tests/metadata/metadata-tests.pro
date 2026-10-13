QT += core gui widgets multimedia network testlib
CONFIG += c++17 console
CONFIG -= app_bundle
TEMPLATE = app
TARGET = metadata-tests
CASTER = $$PWD/../../CasterSoundboard
INCLUDEPATH += $$CASTER
SOURCES += $$PWD/test_metadata.cpp \
    $$CASTER/CasterBoard.cpp $$CASTER/CasterPlayer.cpp \
    $$CASTER/CasterPlayerState.cpp $$CASTER/CasterLabelColorPicker.cpp \
    $$CASTER/CasterCuePicker.cpp $$CASTER/CSS.cpp
SOURCES += $$files($$CASTER/libs/*.cpp, true)
HEADERS += $$CASTER/CasterBoard.h $$CASTER/CasterPlayer.h \
    $$CASTER/CasterLabelColorPicker.h $$CASTER/CasterCuePicker.h
RESOURCES += $$CASTER/res.qrc
SOURCES += $$CASTER/actions/ActionProfile.cpp $$CASTER/actions/ActionEngine.cpp
HEADERS += $$CASTER/actions/ActionProfile.h $$CASTER/actions/ActionEngine.h
