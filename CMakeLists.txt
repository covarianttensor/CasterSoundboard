cmake_minimum_required(VERSION 3.16)

if(DEFINED CASTER_VERSION AND CASTER_VERSION MATCHES "^[0-9]+(\\.[0-9]+)*")
    set(PROJECT_VERSION_STRING "${CMAKE_MATCH_0}")
else()
    set(PROJECT_VERSION_STRING "2.0.0")
endif()

project(CasterSoundboard VERSION ${PROJECT_VERSION_STRING} LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Multimedia Network Test)

# -------------------------------------------------------------
# Core C++ Soundboard Engine Library
# -------------------------------------------------------------
set(CORE_SOURCES
    src/core/SoundTileController.h
    src/core/SoundTileController.cpp
    src/core/SoundboardGridModel.h
    src/core/SoundboardGridModel.cpp
    src/core/SoundboardTabModel.h
    src/core/SoundboardTabModel.cpp
    src/core/SoundboardProjectController.h
    src/core/SoundboardProjectController.cpp
    src/core/HotkeyManager.h
    src/core/HotkeyManager.cpp
    src/core/OscServerController.h
    src/core/OscServerController.cpp
    src/core/AudioRoutingController.h
    src/core/AudioRoutingController.cpp
    src/core/WebRemoteController.h
    src/core/WebRemoteController.cpp
    src/audio/AudioBackend.h
    src/audio/AudioDeviceController.h
    src/audio/AudioDeviceController.cpp
    src/audio/PulseCompatBackend.h
    src/audio/PulseCompatBackend.cpp
    src/audio/CasterPlaybackRouter.h
    src/audio/CasterPlaybackRouter.cpp
)

add_library(CasterCore STATIC ${CORE_SOURCES})

target_include_directories(CasterCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio
)

target_link_libraries(CasterCore PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Multimedia
    Qt6::Network
)

# -------------------------------------------------------------
# QML Application Executable
# -------------------------------------------------------------
qt_add_executable(CasterSoundboard
    src/main.cpp
)

target_include_directories(CasterSoundboard PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
)

target_link_libraries(CasterSoundboard PRIVATE
    CasterCore
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Multimedia
    Qt6::Network
)

if(COMMAND qt_policy)
    qt_policy(SET QTP0004 OLD)
endif()

qt_add_qml_module(CasterSoundboard
    URI CasterSoundboard
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    QML_FILES
        src/qml/Main.qml
        src/qml/NavSidebar.qml
        src/qml/TopActionBar.qml
        src/qml/SoundboardTabBar.qml
        src/qml/SoundTile.qml
        src/qml/SoundTileDrawer.qml
        src/qml/views/SoundboardsView.qml
        src/qml/views/AudioRoutingView.qml
        src/qml/views/OscView.qml
        src/qml/views/PreferencesView.qml
        src/qml/views/DocumentationView.qml
    RESOURCES
        src/qml/icons/33_percent.png
        src/qml/icons/about.png
        src/qml/icons/app.png
        src/qml/icons/audio_devices.png
        src/qml/icons/cog.png
        src/qml/icons/colorPicker.png
        src/qml/icons/cue.png
        src/qml/icons/down.png
        src/qml/icons/duck.png
        src/qml/icons/Key.png
        src/qml/icons/loop.png
        src/qml/icons/newTab.png
        src/qml/icons/no_loop.png
        src/qml/icons/notes.png
        src/qml/icons/openMusic.png
        src/qml/icons/open.png
        src/qml/icons/open_sound_control.png
        src/qml/icons/pause.png
        src/qml/icons/play.png
        src/qml/icons/playState_paused.png
        src/qml/icons/playState_playing.png
        src/qml/icons/playState_stopped.png
        src/qml/icons/rename.png
        src/qml/icons/save_as.png
        src/qml/icons/save.png
        src/qml/icons/stopAll.png
        src/qml/icons/stop.png
        src/qml/icons/unduck.png
        src/qml/icons/fullscreen.png
        src/qml/icons/exit_fullscreen.png
        src/qml/icons/soundboard_tab.png
        src/qml/icons/add_player.png
        src/qml/img/player_gradient.png
        src/qml/img/player_outline.png
        src/web/index.html
)

# -------------------------------------------------------------
# Unit Tests
# -------------------------------------------------------------
enable_testing()

function(add_caster_test test_name test_source)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE CasterCore Qt6::Test)
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

add_caster_test(test_sound_tile tests/core/test_sound_tile.cpp)
add_caster_test(test_grid_model tests/core/test_grid_model.cpp)
add_caster_test(test_tab_model tests/core/test_tab_model.cpp)
add_caster_test(test_project_serialization tests/core/test_project_serialization.cpp)
add_caster_test(test_hotkey_manager tests/core/test_hotkey_manager.cpp)
add_caster_test(test_osc_server tests/core/test_osc_server.cpp)
add_caster_test(test_audio_routing tests/core/test_audio_routing.cpp)
add_caster_test(test_web_remote tests/core/test_web_remote.cpp)

# -------------------------------------------------------------
# Installation
# -------------------------------------------------------------
install(TARGETS CasterSoundboard
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES dist/linux/io.github.covarianttensor.CasterSoundboard.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(FILES dist/linux/io.github.covarianttensor.CasterSoundboard.metainfo.xml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo
)

install(FILES dist/linux/io.github.covarianttensor.CasterSoundboard.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
)

install(FILES dist/linux/io.github.covarianttensor.CasterSoundboard.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)

