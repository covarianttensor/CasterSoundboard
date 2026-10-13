#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${REPO_ROOT}/build-flatpak"
REPO_DIR="${BUILD_DIR}/repo"
APP_ID="io.github.covarianttensor.CasterSoundboard"
BUNDLE_FILE="${REPO_ROOT}/CasterSoundboard.flatpak"

mkdir -p "${BUILD_DIR}" "${REPO_DIR}"

flatpak-builder --force-clean --repo="${REPO_DIR}" "${BUILD_DIR}/build" "${REPO_ROOT}/${APP_ID}.yml"
flatpak build-bundle "${REPO_DIR}" "${BUNDLE_FILE}" "${APP_ID}"

echo "Flatpak bundle successfully generated at ${BUNDLE_FILE}"
