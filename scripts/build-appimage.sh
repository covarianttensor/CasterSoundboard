#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${REPO_ROOT}/build-appimage"
APPDIR="${BUILD_DIR}/AppDir"
TOOLS_DIR="${BUILD_DIR}/tools"

# Extract version from CASTER_VERSION, git tag, or fallback to 2.1.2
VERSION="${CASTER_VERSION:-}"
if [ -z "${VERSION}" ]; then
    if [ "${GITHUB_REF_TYPE:-}" = "tag" ] && [ -n "${GITHUB_REF_NAME:-}" ]; then
        VERSION="${GITHUB_REF_NAME#v}"
    else
        TAG_DESC="$(git describe --tags --always --dirty 2>/dev/null || echo "")"
        if [ -n "${TAG_DESC}" ]; then
            VERSION="${TAG_DESC#v}"
        else
            VERSION="2.1.2-dev"
        fi
    fi
fi
export VERSION

echo "Building CasterSoundboard AppImage version: ${VERSION}"

rm -rf "${APPDIR}" "${BUILD_DIR}/build"
mkdir -p "${BUILD_DIR}/build" "${APPDIR}" "${TOOLS_DIR}"

# 1. Configure, compile, and install into AppDir using CMake
pushd "${BUILD_DIR}/build" >/dev/null
cmake -B . -S "${REPO_ROOT}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCASTER_VERSION="${VERSION}"
cmake --build . -j"$(nproc)"
DESTDIR="${APPDIR}" cmake --install .
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

# 3. Optional: Bundle host ffmpeg CLI binary into AppDir for self-contained waveform extraction
if command -v ffmpeg >/dev/null 2>&1; then
    mkdir -p "${APPDIR}/usr/bin"
    cp -L "$(command -v ffmpeg)" "${APPDIR}/usr/bin/ffmpeg" 2>/dev/null || true
fi

# 4. Environment setup for linuxdeploy
export PATH="${TOOLS_DIR}:${PATH}"
export APPIMAGE_EXTRACT_AND_RUN=1
export NO_STRIP=1
export QML_SOURCES_PATHS="${REPO_ROOT}/src/qml"
export EXTRA_QT_PLUGINS="multimedia;wayland-decoration-client;wayland-graphics-integration-client;wayland-shell-integration"

pushd "${REPO_ROOT}" >/dev/null
# Deploy Qt plugins, QML imports, and shared libraries
"${LINUXDEPLOY}" \
    --appdir "${APPDIR}" \
    --desktop-file "${APPDIR}/usr/share/applications/io.github.covarianttensor.CasterSoundboard.desktop" \
    --icon-file "${APPDIR}/usr/share/icons/hicolor/256x256/apps/io.github.covarianttensor.CasterSoundboard.png" \
    --plugin qt

# Clean up broken GStreamer plugin so QtMultimedia reliably uses bundled FFmpeg
rm -f "${APPDIR}/usr/plugins/multimedia/libgstreamermediaplugin.so"
rm -f "${APPDIR}/usr/lib/libgst"*

# Generate final AppImage (ignore non-fatal appstream warning exit code 2 from linuxdeploy)
"${LINUXDEPLOY}" \
    --appdir "${APPDIR}" \
    --output appimage || true

# Normalize output filename
GENERATED_APPIMAGE=$(ls -t Caster*Soundboard*.AppImage CasterSoundboard*.AppImage 2>/dev/null | head -n 1)
if [ -n "${GENERATED_APPIMAGE}" ]; then
    TARGET_APPIMAGE="CasterSoundboard-${VERSION}-x86_64.AppImage"
    if [ "${GENERATED_APPIMAGE}" != "${TARGET_APPIMAGE}" ]; then
        mv -f "${GENERATED_APPIMAGE}" "${TARGET_APPIMAGE}"
    fi
    echo "AppImage created: ${TARGET_APPIMAGE}"
else
    echo "Error: AppImage file was not generated" >&2
    exit 1
fi
popd >/dev/null
