QT += core testlib
QT -= gui
CONFIG += console c++11 testcase
TARGET = audio-tests
SOURCES += tst_AudioBackend.cpp ../../CasterSoundboard/audio/PulseCompatBackend.cpp
HEADERS += ../../CasterSoundboard/audio/AudioBackend.h ../../CasterSoundboard/audio/PulseCompatBackend.h
INCLUDEPATH += ../../CasterSoundboard/audio

SOURCES += ../../CasterSoundboard/audio/CasterPlaybackRouter.cpp
HEADERS += ../../CasterSoundboard/audio/CasterPlaybackRouter.h
