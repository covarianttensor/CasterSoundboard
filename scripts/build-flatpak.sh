#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${REPO_ROOT}/build-flatpak"
REPO_DIR="${BUILD_DIR}/repo"
APP_ID="io.github.covarianttensor.CasterSoundboard"

# Extract version from CASTER_VERSION, git tag, or VERSION file fallback
FALLBACK_VER="$(cat "${REPO_ROOT}/VERSION" 2>/dev/null || echo "0.0.0")"
VERSION="${CASTER_VERSION:-}"
if [ -z "${VERSION}" ]; then
    if [ -n "${GITHUB_REF_NAME:-}" ]; then
        VERSION="${GITHUB_REF_NAME#v}"
    else
        VERSION="$(git describe --tags --always --dirty 2>/dev/null | sed 's/^v//' || echo "${FALLBACK_VER}-dev")"
    fi
fi
export VERSION

BUNDLE_FILE="${REPO_ROOT}/CasterSoundboard-${VERSION}-x86_64.flatpak"
DEFAULT_BUNDLE="${REPO_ROOT}/CasterSoundboard.flatpak"

mkdir -p "${BUILD_DIR}" "${REPO_DIR}"

# Provide VERSION to flatpak sandbox builder if not already present
CLEANUP_VERSION=false
if [ ! -f "${REPO_ROOT}/VERSION" ]; then
    echo "${VERSION}" > "${REPO_ROOT}/VERSION"
    CLEANUP_VERSION=true
fi

cleanup() {
    if [ "${CLEANUP_VERSION}" = "true" ]; then
        rm -f "${REPO_ROOT}/VERSION"
    fi
}
trap cleanup EXIT

MANIFEST="${REPO_ROOT}/${APP_ID}.yml"
LOCAL_MANIFEST="${BUILD_DIR}/${APP_ID}.local.yml"

# Allow building directly from the local working directory when --local is provided
if [ "${1:-}" = "--local" ] || [ "${CASTER_FLATPAK_LOCAL:-0}" = "1" ]; then
    sed -e 's|type: git|type: dir|g' \
        -e 's|url: .*|path: '"${REPO_ROOT}"'|g' \
        -e '/tag: .*/d' \
        -e '/commit: .*/d' \
        "${MANIFEST}" > "${LOCAL_MANIFEST}"
    MANIFEST="${LOCAL_MANIFEST}"
fi

BUILDER_BIN="flatpak-builder"
if ! command -v flatpak-builder >/dev/null 2>&1; then
    if flatpak info org.flatpak.Builder >/dev/null 2>&1; then
        BUILDER_BIN="flatpak run org.flatpak.Builder"
    else
        echo "Error: flatpak-builder is not installed. Install via: sudo pacman -S flatpak-builder" >&2
        exit 1
    fi
fi

${BUILDER_BIN} --force-clean --repo="${REPO_DIR}" "${BUILD_DIR}/build" "${MANIFEST}"
flatpak build-bundle "${REPO_DIR}" "${BUNDLE_FILE}" "${APP_ID}"
cp -f "${BUNDLE_FILE}" "${DEFAULT_BUNDLE}"

echo "Flatpak bundle successfully generated at ${BUNDLE_FILE}"
