QT += core testlib
QT -= gui
CONFIG += console c++11 testcase
TARGET = audio-tests
SOURCES += tst_AudioBackend.cpp ../../src/audio/PulseCompatBackend.cpp
HEADERS += ../../src/audio/AudioBackend.h ../../src/audio/PulseCompatBackend.h
INCLUDEPATH += ../../src/audio

SOURCES += ../../src/audio/CasterPlaybackRouter.cpp
HEADERS += ../../src/audio/CasterPlaybackRouter.h
