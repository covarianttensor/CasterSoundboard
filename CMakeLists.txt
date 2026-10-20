cmake_minimum_required(VERSION 3.16)

# -------------------------------------------------------------
# Dynamic Git & Release Version Extraction
# -------------------------------------------------------------
set(CASTER_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" CASTER_DEFAULT_VERSION)
    string(STRIP "${CASTER_DEFAULT_VERSION}" CASTER_DEFAULT_VERSION)
else()
    set(CASTER_DEFAULT_VERSION "0.0.0")
endif()

execute_process(
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -DHEADER_OUTPUT=${CASTER_GENERATED_DIR}/CasterVersion.h
        -DEXPLICIT_VERSION=${CASTER_VERSION}
        -DFALLBACK_VERSION=${CASTER_DEFAULT_VERSION}
        -DSEMVER_OUTPUT=${CASTER_GENERATED_DIR}/semver.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateVersionHeader.cmake
)

if(EXISTS "${CASTER_GENERATED_DIR}/semver.txt")
    file(READ "${CASTER_GENERATED_DIR}/semver.txt" PROJECT_VERSION_STRING)
    string(STRIP "${PROJECT_VERSION_STRING}" PROJECT_VERSION_STRING)
else()
    set(PROJECT_VERSION_STRING "${CASTER_DEFAULT_VERSION}")
endif()

project(CasterSoundboard VERSION ${PROJECT_VERSION_STRING} LANGUAGES CXX)

# Target to re-check git version on every build without causing unnecessary rebuilds
add_custom_target(generate_version_header
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -DHEADER_OUTPUT=${CASTER_GENERATED_DIR}/CasterVersion.h
        -DEXPLICIT_VERSION=${CASTER_VERSION}
        -DFALLBACK_VERSION=${CASTER_DEFAULT_VERSION}
        -DSEMVER_OUTPUT=${CASTER_GENERATED_DIR}/semver.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateVersionHeader.cmake
    BYPRODUCTS ${CASTER_GENERATED_DIR}/CasterVersion.h
    COMMENT "Verifying git commit and updating CasterVersion.h..."
)

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
    src/core/McpServerController.h
    src/core/McpServerController.cpp
    src/core/PttProfile.h
    src/core/UInputChannel.h
    src/core/UInputChannel.cpp
    src/core/PttController.h
    src/core/PttController.cpp
    src/core/LegacyProjectImporter.h
    src/core/LegacyProjectImporter.cpp
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
    ${CASTER_GENERATED_DIR}
)
add_dependencies(CasterCore generate_version_header)

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
    ${CASTER_GENERATED_DIR}
)
add_dependencies(CasterSoundboard generate_version_header)

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
        src/qml/PttDialog.qml
        src/qml/LegacyImportDialog.qml
        src/qml/ImportDirectoryDialog.qml
        src/qml/views/SoundboardsView.qml
        src/qml/views/AudioRoutingView.qml
        src/qml/views/OscView.qml
        src/qml/views/PreferencesView.qml
        src/qml/views/DocumentationView.qml
        src/qml/views/AboutView.qml
    RESOURCES
        src/qml/icons/33_percent.png
        src/qml/icons/about.png
        src/qml/icons/actions.png
        src/qml/icons/app.png
        src/qml/icons/audio_devices.png
        src/qml/icons/book.png
        src/qml/icons/cog.png
        src/qml/icons/colorPicker.png
        src/qml/icons/cue.png
        src/qml/icons/down.png
        src/qml/icons/duck.png
        src/qml/icons/Key.png
        src/qml/icons/loop.png
        src/qml/icons/newTab.png
        src/qml/icons/import_directory.png
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
    add_dependencies(${test_name} generate_version_header)
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

add_caster_test(test_sound_tile tests/core/test_sound_tile.cpp)
add_caster_test(test_grid_model tests/core/test_grid_model.cpp)
add_caster_test(test_tab_model tests/core/test_tab_model.cpp)
add_caster_test(test_project_serialization tests/core/test_project_serialization.cpp)
add_caster_test(test_hotkey_manager tests/core/test_hotkey_manager.cpp)
add_caster_test(test_osc_server tests/core/test_osc_server.cpp)
add_caster_test(test_audio_routing tests/core/test_audio_routing.cpp)
add_caster_test(test_audio_backend tests/audio/tst_AudioBackend.cpp)
set_tests_properties(test_audio_backend PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tests/audio")
add_caster_test(test_web_remote tests/core/test_web_remote.cpp)
add_caster_test(test_ptt tests/core/test_ptt.cpp)
add_caster_test(test_mcp_server tests/core/test_mcp_server.cpp)

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

# -------------------------------------------------------------
# CPack Packaging (.deb & .rpm)
# -------------------------------------------------------------
set(CPACK_PACKAGE_NAME "castersoundboard")
set(CPACK_PACKAGE_VENDOR "Covariant Tensor")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Touch-first live soundboard and audio routing tool for Linux")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/covarianttensor/CasterSoundboard")
set(CPACK_PACKAGE_CONTACT "https://github.com/covarianttensor/CasterSoundboard/issues")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")

# Debian generator settings (.deb)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Oscar <https://github.com/covarianttensor>")
set(CPACK_DEBIAN_PACKAGE_SECTION "sound")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")

# Ensure proper Debian architecture even when dpkg is not present on the host
if(NOT CPACK_DEBIAN_PACKAGE_ARCHITECTURE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7|armhf")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "armhf")
    endif()
endif()

# Automatically detect shared library dependencies for the C++ binary when dpkg-shlibdeps is available
find_program(DPKG_SHLIBDEPS_EXECUTABLE dpkg-shlibdeps)
if(DPKG_SHLIBDEPS_EXECUTABLE)
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
endif()

# Explicitly declare runtime QML modules and CLI utilities that shlibdeps cannot detect
set(CPACK_DEBIAN_PACKAGE_DEPENDS "qml6-module-qtquick, qml6-module-qtquick-controls, qml6-module-qtquick-layouts, qml6-module-qtquick-templates, qml6-module-qtmultimedia, ffmpeg, pulseaudio-utils | pipewire-pulse")

# RPM generator settings (.rpm for Fedora, RHEL & openSUSE)
set(CPACK_RPM_PACKAGE_LICENSE "LGPL-3.0-or-later")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Multimedia")
set(CPACK_RPM_PACKAGE_DESCRIPTION "Touch-first live soundboard and audio routing tool for Linux")
set(CPACK_RPM_PACKAGE_AUTOREQPROV ON)
set(CPACK_RPM_PACKAGE_REQUIRES "qt6-qtdeclarative, qt6-qtmultimedia, ffmpeg, (pulseaudio-utils or pipewire-pulseaudio)")
set(CPACK_RPM_PACKAGE_RELEASE "1")
set(CPACK_RPM_FILE_NAME "RPM-DEFAULT")
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
    "/usr/share/applications"
    "/usr/share/metainfo"
    "/usr/share/icons"
    "/usr/share/icons/hicolor"
    "/usr/share/icons/hicolor/256x256"
    "/usr/share/icons/hicolor/256x256/apps"
    "/usr/share/icons/hicolor/scalable"
    "/usr/share/icons/hicolor/scalable/apps"
)

include(CPack)


