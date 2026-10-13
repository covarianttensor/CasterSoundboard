#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${REPO_ROOT}/build-appimage"
APPDIR="${BUILD_DIR}/AppDir"
TOOLS_DIR="${BUILD_DIR}/tools"

QMAKE_BIN="${QMAKE:-$(command -v qmake6 || command -v qmake)}"
export QMAKE="${QMAKE_BIN}"

rm -rf "${APPDIR}" "${BUILD_DIR}/build"
mkdir -p "${BUILD_DIR}/build" "${APPDIR}" "${TOOLS_DIR}"

# 1. Configure, compile, and install into AppDir
pushd "${BUILD_DIR}/build" >/dev/null
"${QMAKE_BIN}" "${REPO_ROOT}/CasterSoundboard/CasterSoundboard.pro" CONFIG+=release PREFIX=/usr
make -j"$(nproc)"
make INSTALL_ROOT="${APPDIR}" install
strip "${APPDIR}/usr/bin/CasterSoundboard" || true
popd >/dev/null

# 2. Download linuxdeploy and linuxdeploy-plugin-qt if not cached
LINUXDEPLOY="${TOOLS_DIR}/linuxdeploy-x86_64.AppImage"
LINUXDEPLOY_QT="${TOOLS_DIR}/linuxdeploy-plugin-qt-x86_64.AppImage"

if [ ! -x "${LINUXDEPLOY}" ]; then
    curl -fL -o "${LINUXDEPLOY}" \
        "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
    chmod +x "${LINUXDEPLOY}"
fi

if [ ! -x "${LINUXDEPLOY_QT}" ]; then
    curl -fL -o "${LINUXDEPLOY_QT}" \
        "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"
    chmod +x "${LINUXDEPLOY_QT}"
fi

# 3. Copy Wayland platform plugins into AppDir if available
QT_PLUGINS_DIR="$(${QMAKE_BIN} -query QT_INSTALL_PLUGINS 2>/dev/null || echo /usr/lib/x86_64-linux-gnu/qt6/plugins)"
if [ -d "${QT_PLUGINS_DIR}/platforms" ]; then
    mkdir -p "${APPDIR}/usr/plugins/platforms"
    for wp in "${QT_PLUGINS_DIR}/platforms"/*wayland*.so; do
        if [ -f "$wp" ]; then
            cp -n "$wp" "${APPDIR}/usr/plugins/platforms/" || true
        fi
    done
fi

export PATH="${TOOLS_DIR}:${PATH}"
export APPIMAGE_EXTRACT_AND_RUN=1
export NO_STRIP=1
export EXTRA_QT_PLUGINS="multimedia;wayland-decoration-client;wayland-graphics-integration-client;wayland-shell-integration"

pushd "${REPO_ROOT}" >/dev/null
# Deploy Qt plugins and libraries
"${LINUXDEPLOY}" \
    --appdir "${APPDIR}" \
    --desktop-file "${APPDIR}/usr/share/applications/io.github.covarianttensor.CasterSoundboard.desktop" \
    --icon-file "${APPDIR}/usr/share/icons/hicolor/256x256/apps/io.github.covarianttensor.CasterSoundboard.png" \
    --plugin qt

# Clean up broken GStreamer plugin so QtMultimedia reliably uses bundled FFmpeg
rm -f "${APPDIR}/usr/plugins/multimedia/libgstreamermediaplugin.so"
rm -f "${APPDIR}/usr/lib/libgst"*

# Generate final AppImage
"${LINUXDEPLOY}" \
    --appdir "${APPDIR}" \
    --output appimage
popd >/dev/null
