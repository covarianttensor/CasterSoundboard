cmake_minimum_required(VERSION 3.16)

project(CasterSoundboard VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Multimedia Network Test)

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
)

add_library(CasterCore STATIC ${CORE_SOURCES})

target_include_directories(CasterCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
)

target_link_libraries(CasterCore PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Multimedia
    Qt6::Network
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
