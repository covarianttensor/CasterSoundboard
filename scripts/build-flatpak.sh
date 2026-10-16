#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${REPO_ROOT}/build-flatpak"
REPO_DIR="${BUILD_DIR}/repo"
APP_ID="io.github.covarianttensor.CasterSoundboard"

# Extract version from CASTER_VERSION, git tag, or fallback to 2.0.0
VERSION="${CASTER_VERSION:-}"
if [ -z "${VERSION}" ]; then
    if [ -n "${GITHUB_REF_NAME:-}" ]; then
        VERSION="${GITHUB_REF_NAME#v}"
    else
        VERSION="$(git describe --tags --always 2>/dev/null | sed 's/^v//' || echo "2.0.0")"
    fi
fi
export VERSION

BUNDLE_FILE="${REPO_ROOT}/CasterSoundboard-${VERSION}-x86_64.flatpak"
DEFAULT_BUNDLE="${REPO_ROOT}/CasterSoundboard.flatpak"

mkdir -p "${BUILD_DIR}" "${REPO_DIR}"

flatpak-builder --force-clean --repo="${REPO_DIR}" "${BUILD_DIR}/build" "${REPO_ROOT}/${APP_ID}.yml"
flatpak build-bundle "${REPO_DIR}" "${BUNDLE_FILE}" "${APP_ID}"
cp -f "${BUNDLE_FILE}" "${DEFAULT_BUNDLE}"

echo "Flatpak bundle successfully generated at ${BUNDLE_FILE}"
