CASTER = $$PWD/../../CasterSoundboard
include($$CASTER/CasterSoundboard.pro)
SOURCES -= main.cpp
for(source, SOURCES): ABS_SOURCES += $$CASTER/$$source
for(header, HEADERS): ABS_HEADERS += $$CASTER/$$header
SOURCES = $$ABS_SOURCES $$PWD/test_actions.cpp
HEADERS = $$ABS_HEADERS
RESOURCES = $$CASTER/res.qrc
INCLUDEPATH += $$CASTER
TARGET = actions-tests
QT += testlib
INSTALLS =
