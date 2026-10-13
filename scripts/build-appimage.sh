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

# 3. Bundle Qt 6, Wayland, and Multimedia plugins into AppImage
export PATH="${TOOLS_DIR}:${PATH}"
export APPIMAGE_EXTRACT_AND_RUN=1
export NO_STRIP=1
export EXTRA_QT_PLUGINS="multimedia;wayland-decoration-client;wayland-graphics-integration-client;wayland-shell-integration"

pushd "${REPO_ROOT}" >/dev/null
"${LINUXDEPLOY}" \
    --appdir "${APPDIR}" \
    --desktop-file "${APPDIR}/usr/share/applications/CasterSoundboard.desktop" \
    --icon-file "${APPDIR}/usr/share/icons/hicolor/256x256/apps/CasterSoundboard.png" \
    --plugin qt \
    --output appimage
popd >/dev/null
